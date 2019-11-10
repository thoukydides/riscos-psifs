/*
    File        : print.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Printer mirror for the PsiFS module.

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
#include "print.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/osargs.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"

// Include project header files
#include "ctrl.h"
#include "debug.h"
#include "err.h"
#include "idle.h"
#include "pollword.h"

// The default destination
#define PRINT_DEFAULT "printer:"

// The printer output stream
static os_fw print_handle;

// The connection state
bool print_active = FALSE;
fs_pathname print_device = "";

/*
    Parameters  : active        - Is the remote device present and active.
                  rx            - The next received character, or negative if
                                  none.
                  tx            - Variable to receive a character to transmit,
                                  or NULL if transmit buffer is full.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required. This must only be called
                  when a block driver is active and usable.
*/
os_error *print_poll(bool active, int rx, int *tx)
{
    os_error *err = NULL;

    // No action unless stream open and a character received
    if (print_active && (0 <= rx))
    {
        // Kick the idle timeout
        idle_kick();

        // Copy the character to the output stream
        err = xos_bputw(rx, print_handle);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : device        - The name of the device to write to, or NULL
                                  to use the default.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the printer mirror or change the destination device.
*/
os_error *print_start(const char *device)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting printer mirror device='%s'", device))

    // Use the default device name if none specified
    if (!device) device = PRINT_DEFAULT;

    // Start by updating any relevant pollwords
    err = pollword_update(psifs_MASK_PRINTER_CONFIG);

    // If already active then close the current stream
    if (!err && print_active) err = print_end(FALSE);

    // Attempt to open the requested device
    if (!err)
    {
        // Start by attempting to open an existing file
        err = xosfind_openupw(osfind_NO_PATH | osfind_ERROR_IF_DIR,
                              device, NULL, &print_handle);
        if (!err && !print_handle)
        {
            // Try to create a new file
            err = xosfind_openoutw(osfind_NO_PATH
                                   | osfind_ERROR_IF_DIR
                                   | osfind_ERROR_IF_ABSENT,
                                   device, NULL, &print_handle);
        }

        // Attempt to read the device name
        if (!err)
        {
            int spare;

            // Attempt to read the canonicalised name
            if (xosargs_read_pathw(print_handle, print_device,
                                   sizeof(print_device), &spare)
                || (spare < 1))
            {
                // Simply copy the name if failed
                ctrl_strcpy(print_device, device);
            }

            // Attempt to set the filetype
            xosfile_set_type(print_device, osfile_TYPE_PRINTOUT);
        }

        // Set the active flag if successful
        if (!err) print_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the block driver usage terminate
                                  immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the printer mirror. This will normally attempt a tidy
                  shutdown, but may be forced to terminate immediately.
*/
os_error *print_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending printer mirror now=%u", now))

    // No action unless active
    if (print_active)
    {
        // Start by updating any relevant pollwords
        err = pollword_update(psifs_MASK_PRINTER_CONFIG);

        // Close the stream, ignoring any errors produced
        if (!err)
        {
            xosfind_closew(print_handle);
            print_active = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the printer mirror.
*/
os_error *print_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying printer mirror status"))

    // Display the current mirror status
    if (print_active)
    {
        printf("Copying received characters to \"%s\".\n", print_device);
    }
    else printf("Printer mirror disabled.\n");

    // Return any error produced
    return err;
}
