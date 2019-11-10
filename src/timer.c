/*
    File        : timer.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Timer handling for the PsiFS module.

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
#include "timer.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/macros.h"

// Include project header files
#include "debug.h"
#include "err.h"
#include "link.h"
#include "mem.h"
#include "util.h"

// Private data for each operation
typedef struct timer_private
{
    struct timer_private *next;
    os_t timeout;
    void *user;
    share_callback callback;
} timer_private;
static timer_private *timer_free_list = NULL;
static timer_private *timer_active_list = NULL;

// Status for foreground operations
static bool timer_fore_done;
static os_error *timer_fore_err;

// The connection status
static bool timer_active = FALSE;

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a timer operation.
*/
static os_error *timer_fore_callback(void *user, os_error *err,
                                     const void *reply)
{
    // Store the status
    timer_fore_done = TRUE;
    timer_fore_err = err;

    // Clear any error status
    err = NULL;

    // Return any error produced
    return err;
}

/*
    Parameters  : timeout       - The time at which the timer should expire.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the specified length of time before returning
                  control.
*/
os_error *timer_fore(os_t timeout, bool escape)
{
    os_error *err = NULL;

    // Clear the pending done flag
    timer_fore_done = FALSE;

    // Start as a background operation
    err = timer_back(timeout, NULL, timer_fore_callback);

    // Process the operation in the foreground
    while (!err && !timer_fore_done)
    {
        // Poll the various layers
        err = link_poll(escape);
    }

    // Preserve any returned error
    if (!err && timer_fore_done) err = timer_fore_err;

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Expire the next pending timer.
*/
static os_error *timer_expire(void)
{
    os_error *err = NULL;
    timer_private *ptr;

    // Unlink from the active list
    ptr = timer_active_list;
    timer_active_list = ptr->next;

    // Call the callback function
    err = (*ptr->callback)(ptr->user, NULL, &ptr->timeout);

    // Add to the free list
    ptr->next = timer_free_list;
    timer_free_list = ptr;

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required.
*/
os_error *timer_poll(void)
{
    os_error *err = NULL;

    // Expire any pending timers
    while (!err && timer_active_list
           && (!timer_active
               || ((timer_active_list->timeout - util_time()) < 0)))
    {
        err = timer_expire();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : timeout       - The time at which the timer should expire.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start a timer. Control is returned immediately. If the link
                  is not valid then no error is returned, but instead the
                  callback function is notified.
*/
os_error *timer_back(os_t timeout, void *user, share_callback callback)
{
    os_error *err = NULL;

    // Check function parameters
    if (!callback) err = &err_bad_parms;
    else
    {
        timer_private *ptr;

        // Obtain an operation record pointer
        if (timer_free_list)
        {
            // A suitable record already exists
            ptr = timer_free_list;
            timer_free_list = ptr->next;
        }
        else
        {
            // Allocate a new structure
            ptr = (timer_private *) MEM_MALLOC(sizeof(timer_private));
            if (!ptr) err = &err_buffer;
        }

        // Complete the details
        if (!err)
        {
            // Add to the active list
            ptr->next = timer_active_list;
            timer_active_list = ptr;

            // Copy the command details
            ptr->timeout = timeout;
            ptr->user = user;
            ptr->callback = callback;

            // Check for expiry immediately
            err = timer_poll();
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Free any allocated memory.
*/
static os_error *timer_free(void)
{
    os_error *err = NULL;

    // Free operation buffers
    while (timer_free_list)
    {
        timer_private *ptr = timer_free_list;

        // Update the free list head pointer
        timer_free_list = ptr->next;

        // Free the memory
        MEM_FREE(ptr);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the timer server layer after the multiplexor layer
                  has been started.
*/
os_error *timer_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting timer server layer"))

    // No action if already active
    if (!timer_active)
    {
        // Set the active flag
        timer_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the timer server layer before the multiplexor layer has
                  been closed.
*/
os_error *timer_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending timer server layer now=%u", now))

    // No action if unless active
    if (timer_active)
    {
        // Clear the active flag
        timer_active = FALSE;

        // End any pending timers
        err = timer_poll();

        // Free any allocated memory
        if (!err) err = timer_free();
    }

    // Return any error produced
    return err;
}
