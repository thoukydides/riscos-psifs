/*
    File        : share.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Shared access to multiplexed servers channels for the
                  PsiFS module.

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
#include "share.h"

// Include clib header files
#include <stdlib.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "link.h"
#include "mem.h"
#include "util.h"

// Data for an operation
typedef struct share_op
{
    const void *cmd;
    void *reply;
    void *user;
    share_callback callback;
    struct share_op *next;
} share_op;

// Handle for a shared channel
struct share_handle
{
    share_send send;
    share_receive receive;
    share_op *free;
    share_op *active;
    share_op *pending;
    bits threads;
    bool timeout_started;
    int timeout;
    share_handle next;
    share_handle prev;
};
static share_handle share_handle_list = SHARE_NONE;
#define SHARE_TIMEOUT (30 * 100)

// Status for foreground operations
static bool share_fore_done;
static os_error *share_fore_err;

/*
    Parameters  : handle        - The handle for the shared channel.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Call the active callback function for a channel operation.
*/
static os_error *share_call_callback(share_handle handle, os_error *err,
                                     const void *reply)
{
    // Check function parameters
    if ((handle == SHARE_NONE) || (!err && !reply)) err = &err_bad_parms;
    else if (handle->active)
    {
        share_op *op;

        // Increment the threading count
        handle->threads++;

        // Remove the operation from the active list
        op = handle->active;
        handle->active = NULL;

        // Call the callback function
        err = (*op->callback)(op->user, err, reply);

        // Add the operation to the free list
        op->next = handle->free;
        handle->free = op;

        // Decrement the threading count
        handle->threads--;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle for the shared channel.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Validate the specified channel handle.
*/
static os_error *share_validate(share_handle handle)
{
    os_error *err = NULL;

    // Check the supplied handle
    if (handle == SHARE_NONE) err = &err_svr_none;
    else
    {
        share_handle ptr = share_handle_list;

        // Check for any matches
        while ((ptr != SHARE_NONE) && (ptr != handle)) ptr = ptr->next;

        // Set an error if not found
        if (ptr == SHARE_NONE) err = &err_svr_closed;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle for the shared channel.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Free the memory associated with the specified channel handle.
*/
static os_error *share_tidy(share_handle handle)
{
    os_error *err = NULL;

    // No action unless not valid and not threaded
    if (share_validate(handle) && !handle->threads)
    {
        // Release all blocks of memory
        while (handle->free)
        {
            share_op *ptr = handle->free;
            handle->free = ptr->next;
            MEM_FREE(ptr);
        }
        MEM_FREE(handle);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle for the shared channel.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any operations required when an idle poll is
                  received.
*/
os_error *share_poll_idle(share_handle handle)
{
    os_error *err = NULL;

    // Check function parameters
    err = share_validate(handle);
    if (!err)
    {
        // Action depends on current state
        if (handle->active)
        {
            // Process timeout
            if (handle->timeout_started)
            {
                // Check whether timeout exceeded
                if (0 < (util_time() - handle->timeout))
                {
                    // Call the callback function
                    err = share_call_callback(handle, &err_svr_time, NULL);
                }
            }
            else
            {
                // Start the timeout counter
                handle->timeout_started = TRUE;
                handle->timeout = util_time() + SHARE_TIMEOUT;
            }
        }
        else if (handle->pending)
        {
            // Make the next pending operation active
            handle->active = handle->pending;
            handle->pending = handle->active->next;
            handle->active->next = NULL;
            handle->timeout_started = FALSE;

            // Attempt to start the next operation
            err = (*handle->send)(handle->active->cmd, handle->active->reply);

            // Handle failure to start the operation
            if (err)
            {
                // Call the callback function
                err = share_call_callback(handle, err, NULL);
            }
        }

        // Delete the structure if no longer required
        if (!err) err = share_tidy(handle);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle for the shared channel.
                  data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any operations required when a data poll is received.
*/
os_error *share_poll_data(share_handle handle, const byte *data, bits size)
{
    os_error *err = NULL;

    // Check function parameters
    if (!data) err = &err_bad_parms;
    else err = share_validate(handle);
    if (!err)
    {
        // No action unless there is an active operation
        if (handle->active)
        {
            // Call the data handler
            err = (*handle->receive)(handle->active->cmd, handle->active->reply,
                                     data, size);

            // Call the callback function
            err = share_call_callback(handle, err, handle->active->reply);
        }

        // Delete the structure if no longer required
        if (!err) err = share_tidy(handle);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle for the shared channel.
                  cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
                  ptr           - Variable to receive a pointer to an operation
                                  structure completed with the specified
                                  details.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Return a pointer to a suitable operation structure.
*/
static os_error *share_op_ptr(share_handle handle, const void *cmd, void *reply,
                              void *user, share_callback callback,
                              share_op **ptr)
{
    os_error *err = NULL;

    // Check function parameters
    if (!ptr || !callback) err = &err_bad_parms;
    else err = share_validate(handle);
    if (!err)
    {
        // Obtain a suitable operation pointer
        if (handle->free)
        {
            // A free structure is already available
            *ptr = handle->free;
            handle->free = handle->free->next;
        }
        else
        {
            // Allocate a new structure
            *ptr = (share_op *) MEM_MALLOC(sizeof(share_op));
            if (!*ptr) err = &err_buffer;
        }

        // Complete the details
        if (!err)
        {
            (*ptr)->cmd = cmd;
            (*ptr)->reply = reply;
            (*ptr)->user = user;
            (*ptr)->callback = callback;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a channel operation.
*/
static os_error *share_fore_callback(void *user, os_error *err,
                                     const void *reply)
{
    // Store the status
    share_fore_done = TRUE;
    share_fore_err = err;

    // Clear any error status
    err = NULL;

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle for the shared channel.
                  cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the channel to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *share_fore(share_handle handle, const void *cmd, void *reply,
                     bool escape)
{
    os_error *err = NULL;

    // Check function parameters
    err = share_validate(handle);
    if (!err)
    {
        share_op *ptr;

        // Clear the done flag
        share_fore_done = FALSE;

        // Obtain an operation record pointer
        err = share_op_ptr(handle, cmd, reply, NULL, share_fore_callback, &ptr);

        // Add to the front of the pending queue
        if (!err)
        {
            ptr->next = handle->pending;
            handle->pending = ptr;
        }

        // Process operation in foreground
        while (!err && !share_fore_done)
        {
            // Poll the various layers
            err = link_poll(escape);
        }

        // Preserve any returned error
        if (!err && share_fore_done) err = share_fore_err;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle for the shared channel.
                  cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the specified operation when the channel becomes idle.
                  Control is returned immediately. If the channel is not valid
                  or the operation fails then no error is returned, but instead
                  the callback function is notified.
*/
os_error *share_back(share_handle handle, const void *cmd, void *reply,
                     void *user, share_callback callback)
{
    os_error *err = NULL;

    // Check function parameters
    err = share_validate(handle);
    if (!err)
    {
        share_op *ptr;

        // Obtain an operation record pointer
        err = share_op_ptr(handle, cmd, reply, user, callback, &ptr);

        // Add to the end of the pending queue
        if (!err)
        {
            share_op **prev = &handle->pending;
            while (*prev) prev = &(*prev)->next;
            ptr->next = NULL;
            *prev = ptr;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Variable to receive the shared channel handle.
                  send          - Function to send a message.
                  receive       - Function to handle a reply.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a new shared channel. This allocates a unique handle
                  for a connected server channel.
*/
os_error *share_create(share_handle *handle, share_send send,
                       share_receive receive)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !send || !receive) err = &err_bad_parms;
    else
    {
        // Create a data structure for this channel
        *handle = (share_handle) MEM_MALLOC(sizeof(struct share_handle));
        if (!*handle) err = &err_buffer;

        // Prepare the data structure
        if (!err)
        {
            (*handle)->send = send;
            (*handle)->receive = receive;
            (*handle)->free = NULL;
            (*handle)->active = NULL;
            (*handle)->pending = NULL;
            (*handle)->threads = 0;
            (*handle)->next = share_handle_list;
            (*handle)->prev = NULL;
        }

        // Link the new data structure in
        if (!err)
        {
            if (share_handle_list) share_handle_list->prev = *handle;
            share_handle_list = *handle;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Variable containing the shared channel handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy a previously created shared channel.
*/
os_error *share_destroy(share_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else err = share_validate(*handle);
    if (!err)
    {
        // Unlink from the list of shared channels
        if ((*handle)->next) (*handle)->next->prev = (*handle)->prev;
        if ((*handle)->prev) (*handle)->prev->next = (*handle)->next;
        else share_handle_list = (*handle)->next;

        // Call callback functions for any pending operations
        while (!err && ((*handle)->active || (*handle)->pending))
        {
            // Make the next operation active
            if (!(*handle)->active)
            {
                (*handle)->active = (*handle)->pending;
                (*handle)->pending = (*handle)->active->next;
            }

            // Callback with a failure error code, ignoring any error returned
            share_call_callback(*handle, &err_svr_closed, NULL);
        }

        // Delete the structure if no longer required
        if (!err) err = share_tidy(*handle);

        // Clear the handle
        if (!err) *handle = SHARE_NONE;
    }

    // Return any error produced
    return err;
}
