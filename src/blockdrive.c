/*
    File        : blockdrive.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Interface to Hugo Fiennes's Block Drivers for serial access.

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
#include "blockdrive.h"

// Include clib header files
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "oslib/fileswitch.h"
#include "oslib/osfile.h"

// Include project header files
#include "debug.h"
#include "err.h"
#include "fs.h"
#include "mem.h"

// Block driver filenames
#define BLOCKDRIVE_DIR "SerialDev:Modules"
#define BLOCKDRIVE_FILE "Driver"

// Function codes
typedef bits blockdrive_reason;
#define BLOCKDRIVE_PUT_BYTE ((blockdrive_reason) 0)
#define BLOCKDRIVE_GET_BYTE ((blockdrive_reason) 1)
#define BLOCKDRIVE_PUT_BLOCK ((blockdrive_reason) 2)
#define BLOCKDRIVE_GET_BLOCK ((blockdrive_reason) 3)
#define BLOCKDRIVE_CHECK_TX ((blockdrive_reason) 4)
#define BLOCKDRIVE_CHECK_RX ((blockdrive_reason) 5)
#define BLOCKDRIVE_FLUSH_TX ((blockdrive_reason) 6)
#define BLOCKDRIVE_FLUSH_RX ((blockdrive_reason) 7)
#define BLOCKDRIVE_CONTROL_LINES ((blockdrive_reason) 8)
#define BLOCKDRIVE_MODEM_CONTROL ((blockdrive_reason) 9)
#define BLOCKDRIVE_RX_ERRORS ((blockdrive_reason) 10)
#define BLOCKDRIVE_BREAK ((blockdrive_reason) 11)
#define BLOCKDRIVE_EXAMINE ((blockdrive_reason) 12)
#define BLOCKDRIVE_TX_SPEED ((blockdrive_reason) 13)
#define BLOCKDRIVE_RX_SPEED ((blockdrive_reason) 14)
#define BLOCKDRIVE_WORD_FORMAT ((blockdrive_reason) 15)
#define BLOCKDRIVE_FLOW_CONTROL ((blockdrive_reason) 16)
#define BLOCKDRIVE_INITIALISE ((blockdrive_reason) 17)
#define BLOCKDRIVE_CLOSE_DOWN ((blockdrive_reason) 18)
#define BLOCKDRIVE_POLL ((blockdrive_reason) 19)

// Prototype for the block driver entry point
typedef int (*blockdrive_entry)(blockdrive_reason reason, bits port,
                                int r2, int r3);

// Value used to read a setting without changing it
#define BLOCKDRIVE_READ (-1)

// A union to allow casting of function pointers without errors
typedef union
{
    blockdrive_entry entry;
    blockdrive_driver driver;
} blockdrive_cast;

/*
    Parameters  : context       - Set to 0 for the first call and preserve
                                  between successive calls.
                  name          - Buffer to receive the next block driver name.
                  size          - Size of the buffer.
                  found         - Variable to receive whether a block driver
                                  was found.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enumerate the available block drivers.
*/
os_error *blockdrive_enumerate(int *context, char *name, size_t size,
                               bool *found)
{
    // Search the block driver directory
    return xosgbpb_dir_entries(BLOCKDRIVE_DIR, (osgbpb_string_list *) name, 1,
                               *context, size, NULL, found, context);
}

/*
    Parameters  : ptr           - Pointer to the error text, or NULL if none.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Convert an error message pointer to a proper error block.
*/
static os_error *blockdrive_error(int ptr)
{
    static os_error err;

    // Always use the same error number
    err.errnum = err_block_driver.errnum;

    // Copy the error text
    if (ptr)
    {
        strncpy(err.errmess, (const char *) ptr, sizeof(err.errmess));
        err.errmess[sizeof(err.errmess) - 1] = '\0';
    }

    // Return a pointer to the error block
    return ptr ? &err : NULL;
}

/*
    Parameters  : driver    - The handle of the block driver.
                  reason    - Block driver reason code (value to pass in r0).
                  port      - The port number (value to pass in r1).
                  r2        - Value to pass in r2.
                  r3        - Value to pass in r3.
    Returns     : int       - Value returned in r0.
    Description : Call the specified reason code for the currently selected
                  block driver and port.
*/
static int blockdrive_call(blockdrive_driver driver, blockdrive_reason reason,
                           bits port, int r2, int r3)
{
    blockdrive_cast cast;
    int result;

//    DEBUG_PRINTF(("Block driver call driver=0x%p, reason=%u, port=%i, r2=%i, r3=%i", driver, reason, port, r2, r3))

    // Call the specified reason code
    cast.driver = driver;
    result = (*cast.entry)(reason, port, r2, r3);

//    DEBUG_PRINTF(("Block driver call result=%i", result))

    // Return the result
    return result;
}

/*
    Parameters  : name          - The name of the block driver to load.
                  driver        - Variable to receive the driver handle.
                  ports         - Variable to receive the number of ports
                                  available.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Allocate a block of memory, load the specified block driver,
                  and perform an inquiry initialise (if supported). This also
                  selects the driver for future operations.
*/
os_error *blockdrive_load(const char *name, blockdrive_driver *driver,
                          bits *ports)
{
    os_error *err = NULL;

    // Check the parameters
    if (!name || !driver) err = &err_bad_parms;
    else
    {
        blockdrive_driver ptr = NULL;
        static fs_pathname filename;
        fileswitch_object_type type;
        int size;

        // Generate the block driver filename
        sprintf(filename, "%s.%s.%s", BLOCKDRIVE_DIR, name, BLOCKDRIVE_FILE);

        // Allocate memory for the driver
        if (!err)
        {
            ptr = (blockdrive_driver) malloc(sizeof(blockdrive_layout));
            if (!ptr) err = &err_buffer;
        }

        // Load the driver
        if (!err)
        {
            err = xosfile_load_stamped(filename, (byte *) ptr,
                                       &type, NULL, NULL, &size, NULL);
        }
        if (!err && (type != fileswitch_IS_FILE)) err = &err_no_driver;
        if (!err && (sizeof(blockdrive_layout) < size)) err = &err_driver_size;

        // Call the initialise entry point if possible
        if (!err && (ptr->flags & BLOCKDRIVE_FLAGS_INQUIRY_INITIALISE))
        {
            err = blockdrive_initialise(ptr, BLOCKDRIVE_PORT_NONE, NULL);
        }

        // Initially no serial port initialise
        if (!err) ptr->port = BLOCKDRIVE_PORT_NONE;

        // Deallocate the memory if failed
        if (err && ptr) free(ptr);

        // Set the return values
        *driver = err ? BLOCKDRIVE_DRIVER_NONE : ptr;
        if (ports)
        {
            *ports = err
                     ? 0
                     : (ptr->flags & BLOCKDRIVE_FLAGS_MULTIPLE_PORTS
                        ? (ptr->flags
                           >> BLOCKDRIVE_FLAGS_HIGHEST_PORT_SHIFT) + 1
                        : 1);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : Deallocate the block of memory used by the specified block
                  driver. This does not close down the driver first.
*/
void blockdrive_unload(blockdrive_driver driver)
{
    // Deallocate the memory
    free(driver);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  value     - The byte to send.
    Returns     : bool      - Was the byte inserted into the transmit queue.
    Description : Insert a byte into the transmit queue.
*/
bool blockdrive_put_byte(blockdrive_driver driver, byte value)
{
    return 0 <= blockdrive_call(driver, BLOCKDRIVE_PUT_BYTE, driver->port,
                                value, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  value     - Variable to receive the byte read.
    Returns     : bool      - Was a byte available.
    Description : Remove a byte from the receive queue.
*/
bool blockdrive_get_byte(blockdrive_driver driver, byte *value)
{
    int result = blockdrive_call(driver, BLOCKDRIVE_GET_BYTE, driver->port,
                                 0, 0);
    *value = result < 0 ? 0 : result;
    return 0 <= result;
}

/*
    Parameters  : driver    - The handle of the block driver.
                  block     - Pointer to block containing the bytes to transmit.
                  bytes     - Number of bytes to insert.
    Returns     : bits      - The number of bytes successfully inserted.
    Description : Insert a block of bytes into the transmit queue.
*/
bits blockdrive_put_block(blockdrive_driver driver, const byte *block,
                          size_t bytes)
{
    return blockdrive_call(driver, BLOCKDRIVE_PUT_BLOCK, driver->port,
                           (int) block, bytes);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  block     - Pointer to block to hold the received bytes.
                  bytes     - Maximum number of bytes to place in the buffer.
    Returns     : bits      - Number of bytes placed in the buffer.
    Description : Remove a block of bytes from the receive queue.
*/
bits blockdrive_get_block(blockdrive_driver driver, byte *block, size_t bytes)
{
    return blockdrive_call(driver, BLOCKDRIVE_GET_BLOCK, driver->port,
                           (int) block, bytes);
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : bits      - Number of bytes free in the transmit buffer.
    Description : Check the available space in the transmit buffer.
*/
bits blockdrive_check_tx(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_CHECK_TX, driver->port, 0, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : bits      - Number of bytes used in the receive buffer.
    Description : Check the number of bytes in the receive buffer.
*/
bits blockdrive_check_rx(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_CHECK_RX, driver->port, 0, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : Flush the transmit buffer and hardware FIFO (if applicable).
*/
void blockdrive_flush_tx(blockdrive_driver driver)
{
    blockdrive_call(driver, BLOCKDRIVE_FLUSH_TX, driver->port, 0, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : Flush the receive buffer and hardware FIFO (if applicable).
*/
void blockdrive_flush_rx(blockdrive_driver driver)
{
    blockdrive_call(driver, BLOCKDRIVE_FLUSH_RX, driver->port, 0, 0);
}

/*
    Parameters  : driver                - The handle of the block driver.
    Returns     : blockdrive_control    - The current state of the control
                                          lines.
    Description : Read the control line settings.
*/
blockdrive_control blockdrive_control_read(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_CONTROL_LINES, driver->port,
                           BLOCKDRIVE_READ, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  control   - The new control line settings.
    Returns     : void
    Description : Set the state of the control lines.
*/
void blockdrive_control_write(blockdrive_driver driver,
                              blockdrive_control control)
{
    blockdrive_call(driver, BLOCKDRIVE_CONTROL_LINES, driver->port, control, 0);
}

/*
    Parameters  : driver            - The handle of the block driver.
    Returns     : blockdrive_modem  - The current state of the modem control
                                      lines.
    Description : Read the modem control line status.
*/
blockdrive_modem blockdrive_modem_read(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_MODEM_CONTROL, driver->port,
                           0, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : blockdrive_errors - The receive errors seen.
    Description : Read the receive errors seen since the last call of this
                  function.
*/
blockdrive_errors blockdrive_errors_read(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_RX_ERRORS, driver->port, 0, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  time      - The length of the break in centiseconds.
    Returns     : void
    Description : Send a break. This function does not return until the break
                  has been sent.
*/
void blockdrive_break(blockdrive_driver driver, bits time)
{
    blockdrive_call(driver, BLOCKDRIVE_BREAK, driver->port, time, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  value     - Variable to receive the byte read.
    Returns     : bool  - Was a byte available.
    Description : Read the next byte from the receive queue, leaving it in the
                  buffer.
*/
bool blockdrive_examine_byte(blockdrive_driver driver, byte *value)
{
    int result = blockdrive_call(driver, BLOCKDRIVE_EXAMINE, driver->port,
                                 0, 0);
    *value = result < 0 ? 0 : result;
    return 0 <= result;
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : bits      - The current transmit speed.
    Description : Read the current transmit baud rate.
*/
bits blockdrive_tx_speed_read(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_TX_SPEED, driver->port,
                           BLOCKDRIVE_READ, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  speed     - The new transmit speed.
    Returns     : void
    Description : Set a new transmit baud rate.
*/
void blockdrive_tx_speed_write(blockdrive_driver driver, bits speed)
{
    blockdrive_call(driver, BLOCKDRIVE_TX_SPEED, driver->port, speed, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : bits  - The current receive speed.
    Description : Read the current receive baud rate.
*/
bits blockdrive_rx_speed_read(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_RX_SPEED, driver->port,
                           BLOCKDRIVE_READ, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  speed     - The new receive speed.
    Returns     : void
    Description : Set a new receive baud rate.
*/
void blockdrive_rx_speed_write(blockdrive_driver driver, bits speed)
{
    blockdrive_call(driver, BLOCKDRIVE_RX_SPEED, driver->port, speed, 0);
}

/*
    Parameters  : driver            - The handle of the block driver.
    Returns     : blockdrive_format - The word format.
    Description : Read the current word format.
*/
blockdrive_format blockdrive_word_format_read(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_WORD_FORMAT, driver->port,
                           BLOCKDRIVE_READ, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  format    - The required word format.
    Returns     : void
    Description : Set a new word format.
*/
void blockdrive_word_format_write(blockdrive_driver driver,
                                  blockdrive_format format)
{
    blockdrive_call(driver, BLOCKDRIVE_WORD_FORMAT, driver->port, format, 0);
}

/*
    Parameters  : driver            - The handle of the block driver.
    Returns     : blockdrive_flow   - The current flow control method.
    Description : Read the current flow control method.
*/
blockdrive_flow blockdrive_flow_control_read(blockdrive_driver driver)
{
    return blockdrive_call(driver, BLOCKDRIVE_FLOW_CONTROL, driver->port,
                           BLOCKDRIVE_READ, 0);
}

/*
    Parameters  : driver    - The handle of the block driver.
                  method    - The required flow control method.
    Returns     : void
    Description : Set a new flow control method.
*/
void blockdrive_flow_control_write(blockdrive_driver driver,
                                   blockdrive_flow method)
{
    blockdrive_call(driver, BLOCKDRIVE_FLOW_CONTROL, driver->port, method, 0);
}

/*
    Parameters  : driver        - The handle of the block driver.
                  port          - The port to initialise.
                  options       - Parameter string specifying device options,
                                  or NULL if none.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Initialise the block driver for the specified port. The baud
                  rate tables and driver flags should not be read until this
                  function has been called. This also selects the port for
                  future options.
*/
os_error *blockdrive_initialise(blockdrive_driver driver, bits port,
                                const char *options)
{
    driver->port = port;
    return blockdrive_error(blockdrive_call(driver, BLOCKDRIVE_INITIALISE,
                                            port, (int) options, 0));
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : Close down the block driver.
*/
void blockdrive_close_down(blockdrive_driver driver)
{
    blockdrive_call(driver, BLOCKDRIVE_CLOSE_DOWN, driver->port, 0, 0);
    driver->port = BLOCKDRIVE_PORT_NONE;
}

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : This should be called regularly, such as in a polling loop,
                  for the driver to perform polling tasks. Polls can be as
                  infrequent as three or four times a second.
*/
void blockdrive_poll(blockdrive_driver driver)
{
    // Only poll the driver if it requires polling
    if (driver->flags & BLOCKDRIVE_FLAGS_REQUIRES_POLL)
    {
        blockdrive_call(driver, BLOCKDRIVE_POLL, driver->port, 0, 0);
    }
}
