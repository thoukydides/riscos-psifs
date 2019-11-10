/*
    File        : link.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Serial link handler for the PsiFS module.

    License     : PsiFS is free software: you can redistribute it and/or
                  modify it under the terms of the GNU General Public License
                  as published by the Free Software Foundation, either
                  version 3 of the License, or (at your option) any later
                  version.
    
                  PsiFS is distributed in the hope that it will be useful,
                  but WITHOUT ANY WARRANTY; without even the implied warranty
                  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
                  the GNU General Public License for more details.
    
                  You should have received a copy of the GNU General Public
                  License along with PsiFS. If not, see
                  <http://www.gnu.org/licenses/>.
*/

// Include header file for this module
#include "link.h"

// Include clib header files
#include <limits.h>
#include <stdio.h>

// Include project header files
#include "async.h"
#include "baud.h"
#include "blackhole.h"
#include "blockdrive.h"
#include "ctrl.h"
#include "debug.h"
#include "err.h"
#include "escape.h"
#include "fs.h"
#include "mem.h"
#include "mirror.h"
#include "pollword.h"
#include "sema.h"
#include "sleep.h"
#include "stats.h"
#include "sysvar.h"
#include "user.h"
#include "util.h"
#include "wimpfilt.h"

// System variables to hold the current settings
#define LINK_VAR_PREFIX FS_NAME "$BlockDriver"
#define LINK_VAR_DRIVER_NAME LINK_VAR_PREFIX "Name"
#define LINK_VAR_DRIVER_PORT LINK_VAR_PREFIX "Port"
#define LINK_VAR_DRIVER_BAUD LINK_VAR_PREFIX "BaudRate"
#define LINK_VAR_DRIVER_OPTIONS LINK_VAR_PREFIX "Options"
#define LINK_VAR_DRIVER_AUTOBAUD LINK_VAR_PREFIX "AutoBaud"

// Default settings
#define LINK_DEFAULT_DRIVER_NAME "InternalPC"
#define LINK_DEFAULT_DRIVER_PORT (0)
#define LINK_DEFAULT_DRIVER_BAUD (115200)
#define LINK_DEFAULT_DRIVER_OPTIONS ""
#define LINK_DEFAULT_DRIVER_AUTOBAUD (TRUE)

// Other serial link configuration
#define LINK_WORD_FORMAT (BLOCKDRIVE_FORMAT_BITS_8 \
                          | BLOCKDRIVE_FORMAT_STOP_1 \
                          | BLOCKDRIVE_FORMAT_PARITY_DISABLED \
                          | BLOCKDRIVE_FORMAT_PARITY_TYPE_ODD)
#define LINK_FLOW_CONTROL (BLOCKDRIVE_FLOW_HARDWARE)
#define LINK_CONTROL_INACTIVE (0)
#define LINK_CONTROL_ACTIVE (BLOCKDRIVE_CONTROL_DTR \
                             | BLOCKDRIVE_CONTROL_RTS)

// Maximum number of bits transmitted per byte
#define LINK_MAX_BITS (8 + 1 + 2)

// Timeout in centiseconds for the link to become available
#define LINK_TIMEOUT_CLAIM (100)

// Timeout in centisecond for polled operations
#define LINK_TIMEOUT_POLL (20)

// The current configuration
char *link_driver_name = NULL;
bits link_driver_port = 0;
bits link_driver_baud = 0;
char *link_driver_options = NULL;
bool link_driver_autobaud = TRUE;

// The active baud rate
extern bits link_driver_active_baud = 0;

// Is a serial driver active
bool link_active = FALSE;
static blockdrive_driver link_driver = BLOCKDRIVE_DRIVER_NONE;

// Baud rates
static blockdrive_speeds link_speeds;
static int link_speeds_index;

// The current data rate if active
static bits link_cps;

// Semaphore to prevent reentrant use of the serial link
static semaphore link_sema = SEMA_RELEASED;

/*
    Parameters  : bytes - Number of bytes to transmit.
    Returns     : bits  - Number of centi-seconds required, rounded up.
    Description : Calculate the number of centi-seconds required to transmit or
                  receive the specified number of bytes.
*/
bits link_time(bits bytes)
{
    // Return the result
    return link_active ? ((bytes * 100) / link_cps) + 1 : 0;
}

/*
    Parameters  : timeout       - The maximum number of centiseconds to wait
                                  for the link to become available. A value of
                                  0 results in an immediate return, even if the
                                  link is not available.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Attempt to claim access to the active block driver. This
                  waits up to the specified length of time for the link to
                  become available.
*/
static os_error *link_claim(int timeout)
{
    os_error *err = NULL;
    bool task;

    // Attempt to claim the link
    if (!sema_claim(&link_sema)) err = &err_in_use;

    // Keep trying until timeout occurs if in a taskwindow
    if (err && timeout && !sleep_taskwindow(&task) && task)
    {
        // Calculate the time to give up
        timeout += util_time();

        // Keep trying until successful or timeout
        while (err && ERR_EQ(*err, err_in_use) && ((util_time() - timeout) < 0))
        {
            // Sleep until the next poll
            err = sleep_snooze(NULL);

            // Try again to claim the link
            if (!err) if (!sema_claim(&link_sema)) err = &err_in_use;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : void
    Description : Release the link following a successful call to link_claim.
*/
static void link_release(void)
{
    // Release the link
    sema_release(&link_sema);
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the serial transmitter to become idle.
*/
static os_error *link_wait_idle(void)
{
    os_error *err = NULL;
    escape_config esc;

    DEBUG_PRINTF(("Waiting for link to become idle"))

    // Enable escape conditions
    err = escape_store(&esc);
    if (!err) err = escape_enable();
    if (!err)
    {
        bits free = UINT_MAX - 1;
        bits prev = UINT_MAX;

        // Wait until the buffer status does not change
        while (!err && (free != prev))
        {
            int end;

            // Delay for another character to be transmitted
            end = util_time() + link_time(1);
            while (!err && ((util_time() - end) < 0)) err = link_poll(TRUE);

            // Check the number of free buffer entries
            prev = free;
            if (!link_claim(LINK_TIMEOUT_CLAIM))
            {
                free = blockdrive_check_tx(link_driver);
                link_release();
            }
        }

        // Restore the previous escape state
        escape_restore(&esc);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Intialise the serial link during module initialisation. This
                  attempts to restore the last used block driver and settings.
*/
os_error *link_initialise(void)
{
    os_error *err = NULL;
    char *name = NULL;
    int port;
    int baud;
    char *options = NULL;
    int autobaud;

    DEBUG_PRINTF(("Initialising block driver"))

    // Reset the statistics
    stats_reset();

    // Read the initial configuration from system variables
    if (sysvar_read_string_alloc(LINK_VAR_DRIVER_NAME, &name))
    {
        name = ctrl_strdup(LINK_DEFAULT_DRIVER_NAME);
    }
    if (sysvar_read_int(LINK_VAR_DRIVER_PORT, &port) || (port < 0))
    {
        port = LINK_DEFAULT_DRIVER_PORT;
    }
    if (sysvar_read_int(LINK_VAR_DRIVER_BAUD, &baud) || (baud < 0))
    {
        baud = LINK_DEFAULT_DRIVER_BAUD;
    }
    if (sysvar_read_string_alloc(LINK_VAR_DRIVER_OPTIONS, &options))
    {
        options = ctrl_strdup(LINK_DEFAULT_DRIVER_OPTIONS);
    }
    if (sysvar_read_bool(LINK_VAR_DRIVER_AUTOBAUD, &autobaud))
    {
        autobaud = LINK_DEFAULT_DRIVER_AUTOBAUD;
    }

    // Set the configuration, ignoring any error produced
    link_configure(name, port, baud, options, autobaud);

    // Deallocate any memory that is no longer required
    if (name) MEM_FREE(name);
    if (options) MEM_FREE(options);

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Shut down any active serial link during module finalisation.
*/
os_error *link_finalise(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Finalising block driver"))

    // Unload any active blockdriver
    err = link_disable(FALSE);

    // Release any claimed memory
    if (!err)
    {
        if (link_driver_name) MEM_FREE(link_driver_name);
        if (link_driver_options) MEM_FREE(link_driver_options);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : changed       - Variable to receive whether the baud rate
                                  was changed.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Try the next available baud rate if automatic baud rate
                  identification selected.
*/
os_error *link_next_baud(bool *changed)
{
    os_error *err = NULL;
    int next;

    // Choose the next baud rate to use
    next = link_speeds_index + 1;
    if (!link_speeds[next]) next = 0;

    // No action unless the baud rate is different
    if (changed) *changed = next != link_speeds_index;
    if (next != link_speeds_index)
    {
        link_driver_active_baud = link_speeds[next];

        DEBUG_PRINTF(("New baud rate %u", link_driver_active_baud))

        // Store the new index
        link_speeds_index = next;

        // Lower DTR and RTS
        blockdrive_control_write(link_driver, LINK_CONTROL_INACTIVE);

        // Set the new baud rate
        blockdrive_tx_speed_write(link_driver, link_driver_active_baud);
        blockdrive_rx_speed_write(link_driver, link_driver_active_baud);

        // Raise DTR and RTS
        blockdrive_control_write(link_driver, LINK_CONTROL_ACTIVE);

        // Calculate and store the data rate
        link_cps = link_driver_active_baud / LINK_MAX_BITS;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : auto_baud     - Can automatic baud rate identification be
                                  used.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Attempt to load and initialise the configured block driver.
*/
static os_error *link_load(bool auto_baud)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Loading block driver"))

    // Prevent block driver contentions
    err = link_claim(LINK_TIMEOUT_CLAIM);
    if (!err)
    {
        bits ports;

        // Patch BlackHole if necessary
        err = blackhole_check();

        // Patch the Mirror support module if necessary
        if (!err) err = mirror_check();

        // Attempt to load the configured driver
        if (!err) err = blockdrive_load(link_driver_name, &link_driver, &ports);

        // Attempt to initialise the configured port
        if (!err && (ports <= link_driver_port)) err = &err_block_driver;
        if (!err)
        {
            err = blockdrive_initialise(link_driver, link_driver_port,
                                        link_driver_options);
        }

        // Configure and flush the block driver if successful
        if (!err)
        {
            // Configure the block driver
            blockdrive_word_format_write(link_driver, LINK_WORD_FORMAT);
            blockdrive_flow_control_write(link_driver, LINK_FLOW_CONTROL);

            // Prepare the list of baud rates
            if (!auto_baud || !link_driver_autobaud
                || !baud_list(link_driver, baud_recommended, 0, link_speeds))
            {
                // No restriction if no baud rates left or automatic disabled
                if (!baud_list(link_driver, NULL, link_driver_baud,
                               link_speeds))
                {
                    // Remove target speed also if still problems
                    baud_list(link_driver, NULL, 0, link_speeds);
                }
            }

            // Set the initial baud rate
            link_speeds_index = -1;
            link_next_baud(NULL);

            // Flush any pending input and output
            blockdrive_flush_tx(link_driver);
            blockdrive_flush_rx(link_driver);
        }

        // Set the status if successul or unload the block driver otherwise
        if (!err) link_active = TRUE;
        else if (link_driver != BLOCKDRIVE_DRIVER_NONE)
        {
            blockdrive_unload(link_driver);
            link_driver = BLOCKDRIVE_DRIVER_NONE;
        }

        // Reset the statistics if successful
        if (!err) stats_reset();

        // Update any relevant pollwords
        if (!err) err = pollword_update(psifs_MASK_STATS_BYTES);

        // Allow the block driver to be used
        link_release();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Attempt to unload any loaded block driver.
*/
static os_error *link_unload(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Unloading block driver"))

    // No action unless block driver loaded
    if (link_active)
    {
        // Prevent block driver contentions
        err = link_claim(LINK_TIMEOUT_CLAIM);
        if (!err)
        {
            // Lower DTR and RTS
            blockdrive_control_write(link_driver, LINK_CONTROL_INACTIVE);

            // Close down the active port
            blockdrive_close_down(link_driver);

            // Unload the block driver
            blockdrive_unload(link_driver);

            // Set the status
            link_active = FALSE;
            link_driver = BLOCKDRIVE_DRIVER_NONE;

            // Allow the block driver to be used
            link_release();
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enable the block driver for a remote link.
*/
os_error *link_enable_link(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Enabling block driver for remote link"))

    // Special case if link already active
    if (link_active)
    {
        // Just ensure that the user is correct
        err = user_start_link();
    }
    else
    {
        // Load and configure the block driver
        err = link_load(TRUE);

        // Start the remote link user
        if (!err)
        {
            err = user_start_link();
            if (err) link_unload();
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : print         - The name of the device to write to, or NULL
                                  to use the default.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enable the block driver for a printer mirror.
*/
os_error *link_enable_print(const char *print)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Enabling block driver for printer mirror device='%s'", print))

    // Special case if link already active
    if (link_active)
    {
        // Just ensure that the user is correct
        err = user_start_print(print);
    }
    else
    {
        // Load and configure the block driver
        err = link_load(FALSE);

        // Start the printer mirror user
        if (!err)
        {
            err = user_start_print(print);
            if (err) link_unload();
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should any active block driver be unloaded
                                  immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Disable the block driver if already active.
                  This will normally attempt a tidy shutdown, but may be forced
                  to terminate immediately.
*/
os_error *link_disable(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Disabling block driver now=%u", now))

    // No action required unless the block driver is active
    if (link_active)
    {
        // Terminate the user of the block driver
        err = user_end(now);

        // If necessary wait for transmit to complete
        if (!err && !now) err = link_wait_idle();

        // Unload the block driver
        if (!err) err = link_unload();

        // Clear the active baud rate value if successful
        if (!err) link_driver_active_baud = 0;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : name          - The name of the block driver to use.
                  port          - The port number.
                  baud          - The baud rate.
                  options       - Any other options.
                  autobaud      - Automatic baud rate identification mode.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set new serial link settings. It is safe to pass the existing
                  settings for any values to preserve.
*/
os_error *link_configure(const char *name, bits port, bits baud,
                         const char *options, bool autobaud)

{
    os_error *err = NULL;

    DEBUG_PRINTF(("Configuring block driver name='%s', port=%u, baud=%u, options='%s', autobaud=%u", name, port, baud, options, autobaud))

    // It is an error if a block driver is active
    if (link_active) err = &err_link_busy;

    // Update any relevant pollwords
    if (!err) err = pollword_update(psifs_MASK_BLOCK_DRIVER);

    // Set the internal copy of the settings (careful if preserving)
    if (!err)
    {
        if (name != link_driver_name)
        {
            if (link_driver_name) MEM_FREE(link_driver_name);
            link_driver_name = ctrl_strdup(name);
        }
        link_driver_port = port;
        link_driver_baud = baud;
        if (options != link_driver_options)
        {
            if (link_driver_options) MEM_FREE(link_driver_options);
            link_driver_options = ctrl_strdup(options);
        }
        link_driver_autobaud = autobaud;
    }

    // Set all of the system variables
    if (!err) err = sysvar_write_string(LINK_VAR_DRIVER_NAME, link_driver_name);
    if (!err) err = sysvar_write_int(LINK_VAR_DRIVER_PORT, link_driver_port);
    if (!err) err = sysvar_write_int(LINK_VAR_DRIVER_BAUD, link_driver_baud);
    if (!err) err = sysvar_write_string(LINK_VAR_DRIVER_OPTIONS, link_driver_options);
    if (!err) err = sysvar_write_bool(LINK_VAR_DRIVER_AUTOBAUD, link_driver_autobaud);

    // Return any error produced
    return err;
}

/*
    Parameters  : escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled operations required with respect to the
                  link.
*/
os_error *link_poll(bool escape)
{
    os_error *err = NULL;

    // Check for an escape condition
    err = escape_check();

    // Attempt to claim the link
    if (!err && !link_claim(0))
    {
        bool first = TRUE;
        bool more = TRUE;
        int timeout = util_time() + LINK_TIMEOUT_POLL;

        // Keep polling until no more actions
        while (!err && link_active && more && ((util_time() - timeout) < 0))
        {
            byte value;
            int rx = -1;
            int tx = -1;

            // Start by assuming no more actions
            more = FALSE;

            // Poll the block driver
            blockdrive_poll(link_driver);

            // Check the receive buffer
            if (blockdrive_get_byte(link_driver, &value))
            {
                rx = value;
                stats_rx_bytes++;
                more = TRUE;
            }

            // Poll the user of the block driver
            err = user_poll(blockdrive_modem_read(link_driver)
                            & BLOCKDRIVE_MODEM_DSR,
                            rx,
                            blockdrive_check_tx(link_driver) ? &tx : NULL,
                            first);
            first = FALSE;

            // Transmit any returned character
            if (!err && (0 <= tx))
            {
                if (!blockdrive_put_byte(link_driver, tx))
                {
                    err = &err_driver_full;
                }
                stats_tx_bytes++;
                more = TRUE;
            }

            // Update any relevant pollwords
            if (!err && more) err = pollword_update(psifs_MASK_STATS_BYTES);
        }

        // Release the link
        link_release();
    }

    // Disconnect the link if required
    if (!err && !escape && link_active && user_check_disconnect())
    {
        err = link_disable(FALSE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : bool          - Should verbose information be output.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : List the current settings and status.
*/
os_error *link_list_settings(bool verbose)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("List link settings verbose=%u", verbose))

    // Action depends on the verbosity level required
    if (verbose)
    {
        printf("Status       : ");
        if (link_active) printf("Active\n");
        else printf("Disabled\n");
        printf("Block driver : %s\n",
               ctrl_strlen(link_driver_name) ? link_driver_name : "None");
        if (link_active)
        {
            printf("Information  : %s\n", link_driver->information);
            printf("Manufacturer : %s\n", link_driver->manufacturer);
            printf("Driver number: %u\n",
                   link_driver->number >> BLOCKDRIVE_NUMBER_SHIFT);
            printf("Version      : %X.%02X\n",
                   link_driver->version >> 16, link_driver->version & 0xffff);
        }
        printf("Port         : %u\n", link_driver_port);
        printf("Baud rate    : ");
        if (link_driver_autobaud) printf("Automatic/");
        printf("%u", link_driver_baud);
        if (link_active)
        {
            bits tx_baud = blockdrive_tx_speed_read(link_driver);
            bits rx_baud = blockdrive_rx_speed_read(link_driver);
            if ((tx_baud != link_driver_baud)
                || (rx_baud != link_driver_baud))
            {
                if (tx_baud == rx_baud) printf(" (using %u)", tx_baud);
                else printf(" (using Tx:%u/Rx:%u)", tx_baud, rx_baud);
            }
        }
        printf("\n");
        printf("Options      : %s\n",
               ctrl_strlen(link_driver_options)
               ? link_driver_options
               : "None");
    }
    else
    {
        // Summarise the activity status
        if (link_active)
        {
            printf("Block driver '%s' active on port %i.\n",
                   link_driver_name, link_driver_port);
            printf("%u bytes received, %u bytes transmitted.\n",
                   stats_rx_bytes, stats_tx_bytes);
            err = user_status();
            if (!err) err = blackhole_status();
            if (!err) err = mirror_status();
        }
        else printf("Block driver disabled.\n");
        if (!err) err = async_status();
        if (!err) err = wimpfilt_status();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : bool          - Should verbose information be output.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : List the available block drivers.
*/
os_error *link_list_drivers(bool verbose)
{
    os_error *err = NULL;
    bool any = FALSE;
    int context = 0;
    fs_leafname name;
    bool found;

    DEBUG_PRINTF(("List block drivers verbose=%u", verbose))

    // Enumerate the available drivers
    err = blockdrive_enumerate(&context, name, sizeof(name), &found);
    while (!err && found)
    {
        bits ports;
        blockdrive_driver driver;

        // Attempt to load this driver
        if (!blockdrive_load(name, &driver, &ports))
        {
            // Should a short list or verbose output be produced
            if (verbose)
            {
                bits baud;

                // Extra newline between drivers
                if (any) printf("\n");

                // Display the driver details
                printf("%s:\n", name);
                printf("    Information  : %s\n", driver->information);
                printf("    Manufacturer : %s\n", driver->manufacturer);
                printf("    Driver number: %u\n",
                       driver->number >> BLOCKDRIVE_NUMBER_SHIFT);
                printf("    Version      : %X.%02X\n",
                       driver->version >> 16, driver->version & 0xffff);
                printf("    Ports        : %u\n", ports);

                // List the available baud rates
                printf("    Baud rates   : ");
                for (baud = 0; driver->speeds[baud]; baud++)
                {
                    if (baud) printf(driver->speeds[baud + 1] ? ", " : " and ");
                    printf("%u", driver->speeds[baud]);
                }
                if (!baud) printf("None");
                printf("\n");
            }
            else
            {
                // Display a heading if appropriate
                if (!any)
                {
                    printf("Driver     Information                     Manufacturer\n");
                    printf("---------- ------------------------------- -------------------------------\n");
                }

                // Display a summary of the driver details
                printf("%-10.10s %-31.31s %-31.31s\n",
                       name, driver->information, driver->manufacturer);
            }

            // Unload the driver
            blockdrive_unload(driver);
        }

        // Find the next block driver
        err = blockdrive_enumerate(&context, name, sizeof(name), &found);

        // Set flag to indicate that a block driver was found
        any = TRUE;
    }

    // Display a message if no drivers found
    if (!err && !any) printf("No block drivers found.\n");

    // Return any error produced
    return err;
}
