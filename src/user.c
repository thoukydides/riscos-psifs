/*
    File        : user.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Block driver user forwarding layer for the PsiFS module.

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
#include "user.h"

// Include clib header files
#include <stdio.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "frame.h"
#include "idle.h"
#include "link.h"
#include "pollword.h"
#include "print.h"

// The type of link
psifs_mode user_mode = psifs_MODE_INACTIVE;

/*
    Parameters  : active        - Is the remote device present and active.
                  rx            - The next received character, or negative if
                                  none.
                  tx            - Variable to receive a character to transmit,
                                  or NULL if transmit buffer is full.
                  idle          - Should idle polling of higher layers be
                                  performed.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required. This must only be called
                  when a block driver is active and usable.
*/
os_error *user_poll(bool active, int rx, int *tx, bool idle)
{
    os_error *err = NULL;

    // Action depends on the current user
    switch (user_mode)
    {
        case psifs_MODE_LINK:
            // The remote link is active
            err = frame_poll(active, rx, tx, idle);
            break;

        case psifs_MODE_PRINTER:
            // The printer mirror is active
            err = print_poll(active, rx, tx);
            break;

        case psifs_MODE_INACTIVE:
        default:
            // No active user of the block driver
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : bool  - Should a disconnection be performed.
    Description : Check if an automatic idle disconnection should occur.
*/
bool user_check_disconnect(void)
{
    bool disconnect = TRUE;

    // Action depends on the current user
    switch (user_mode)
    {
        case psifs_MODE_LINK:
            // The remote link is active
            disconnect = idle_check_disconnect(idle_disconnect_link);
            break;

        case psifs_MODE_PRINTER:
            // The printer mirror is active
            disconnect = idle_check_disconnect(idle_disconnect_printer);
            break;

        case psifs_MODE_INACTIVE:
        default:
            // No active user of the block driver
            disconnect = TRUE;
            break;
    }

    // Return the disconnection status
    return disconnect;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the frame handler and any clients for the link.
*/
os_error *user_start_link(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting frame handler"))

    // Kick the idle timeout
    idle_kick();

    // Action depends on the current user
    switch (user_mode)
    {
        case psifs_MODE_LINK:
            // The remote link is already active
            break;

        case psifs_MODE_PRINTER:
            // The printer mirror is active
            err = &err_link_busy;
            break;

        case psifs_MODE_INACTIVE:
        default:
            // No active user of the block driver
            err = frame_start();
            break;
    }

    // Set the current user if successful
    if (!err)
    {
        user_mode = psifs_MODE_LINK;
        err = pollword_update(psifs_MASK_MODE);
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
os_error *user_start_print(const char *device)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting printer mirror device='%s'", device))

    // Kick the idle timeout
    idle_kick();

    // Action depends on the current user
    switch (user_mode)
    {
        case psifs_MODE_LINK:
            // The remote link is active
            err = &err_link_busy;
            break;

        case psifs_MODE_PRINTER:
            // The printer mirror is already active
            err = print_start(device);
            break;

        case psifs_MODE_INACTIVE:
        default:
            // No active user of the block driver
            err = print_start(device);
            break;
    }

    // Set the current user if successful
    if (!err)
    {
        user_mode = psifs_MODE_PRINTER;
        err = pollword_update(psifs_MASK_MODE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End any user of the block driver. This will normally attempt
                  a tidy shutdown, but may be forced to terminate immediately.
*/
os_error *user_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending user forwarding now=%u", now))

    // Action depends on the current user
    switch (user_mode)
    {
        case psifs_MODE_LINK:
            // The remote link is active
            err = frame_end(now);
            break;

        case psifs_MODE_PRINTER:
            // The printer mirror is active
            err = print_end(now);
            break;

        case psifs_MODE_INACTIVE:
        default:
            // No active user of the block driver
            break;
    }

    // No current user if successful
    if (!err)
    {
        user_mode = psifs_MODE_INACTIVE;
        err = pollword_update(psifs_MASK_MODE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the block driver user, if any.
*/
os_error *user_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying user forwarding status"))

    // Action depends on the current user
    switch (user_mode)
    {
        case psifs_MODE_LINK:
            // The remote link is active
            err = frame_status();
            break;

        case psifs_MODE_PRINTER:
            // The printer mirror is active
            err = print_status();
            break;

        case psifs_MODE_INACTIVE:
        default:
            // No active user of the block driver
            printf("The block driver is not being used.\n");
            break;
    }

    // Display the idle timeout status also
    if (!err) err = idle_status();

    // Return any error produced
    return err;
}
