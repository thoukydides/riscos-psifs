/*
    File        : upload.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Upload files to a remote device for the PsiFS module.

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
#include "upload.h"

// Include project header files
#include <stdio.h>
#include <string.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "idle.h"
#include "mem.h"
#include "unified.h"

// Private data for each operation
typedef enum
{
    UPLOAD_STATE_INITIAL,
    UPLOAD_STATE_CREATED
} upload_state;
typedef struct upload_private
{
    struct upload_private *next;
    struct upload_private *prev;
    const upload_cmd *cmd;
    void *user;
    share_callback callback;
    upload_state state;
    unified_handle handle;
    os_error *err;
    unified_cmd uni_cmd;
    unified_reply uni_reply;
} upload_private;
static upload_private *upload_free_list = NULL;
static upload_private *upload_active_list = NULL;

// Is the remote file uploader active
static bool upload_era = FALSE;
static bool upload_active = FALSE;

/*
    Parameters  : op            - The operation data.
                  err           - Any error to return.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End processing an operation.
*/
static os_error *upload_done(upload_private *op, os_error *err)
{
    // Check function parameters
    if (!op) err = &err_bad_parms;
    else
    {
        // Re-enable idle timeouts
        idle_end();

        // Unlink from the active list
        if (op->next) op->next->prev = op->prev;
        if (op->prev) op->prev->next = op->next;
        else upload_active_list = op->next;

        // Add to the free list
        op->prev = NULL;
        op->next = upload_free_list;
        if (upload_free_list) upload_free_list->prev = op;
        upload_free_list = op;

        // Call the callback function
        err = (*op->callback)(op->user, err, NULL);
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
    Description : Callback function for a background operation.
*/
static os_error *upload_callback(void *user, os_error *err, const void *reply)
{
    // Check function parameters
    if (!user || (!err && !reply)) err = &err_bad_parms;
    else
    {
        upload_private *op = (upload_private *) user;
        bool done = TRUE;

        // Decode the result of the operation
        switch (op->cmd->op)
        {
            case UPLOAD_COPY:
                // Copy the specified file
                if (!err && (op->uni_cmd.op == UNIFIED_ACCESS)
                    && (op->state == UPLOAD_STATE_INITIAL))
                {
                    done = FALSE;
                    op->uni_cmd.op = UNIFIED_REMOVE;
                    if (sizeof(op->uni_cmd.data.remove.path) <= strlen(op->cmd->data.copy.path)) err = &err_bad_name;
                    if (!err)
                    {
                        strcpy(op->uni_cmd.data.remove.path, op->cmd->data.copy.path);
                        err = unified_back(&op->uni_cmd, &op->uni_reply, op, upload_callback);
                    }
                }
                else if (op->state == UPLOAD_STATE_INITIAL)
                {
                    // Ignore any error
                    err = NULL;
                    op->state = UPLOAD_STATE_CREATED;

                    // Open the file
                    done = FALSE;
                    op->uni_cmd.op = UNIFIED_OPEN;
                    op->uni_cmd.data.open.mode = FS_MODE_OUT;
                    if (sizeof(op->uni_cmd.data.open.path) <= strlen(op->cmd->data.copy.path)) err = &err_bad_name;
                    if (!err)
                    {
                        strcpy(op->uni_cmd.data.open.path, op->cmd->data.copy.path);
                        err = unified_back(&op->uni_cmd, &op->uni_reply, op, upload_callback);
                    }
                }
                else if (!err && (op->uni_cmd.op == UNIFIED_OPEN))
                {
                    // Store the file handle
                    op->handle = op->uni_reply.open.handle;

                    // Set the required file size
                    done = FALSE;
                    op->uni_cmd.op = UNIFIED_SIZE;
                    op->uni_cmd.data.size.handle = op->handle;
                    op->uni_cmd.data.size.size = op->cmd->data.copy.size;
                    err = unified_back(&op->uni_cmd, &op->uni_reply, op, upload_callback);
                }
                else if (!err && (op->uni_cmd.op == UNIFIED_SIZE))
                {
                    // Write the file contents
                    done = FALSE;
                    op->uni_cmd.op = UNIFIED_WRITE;
                    op->uni_cmd.data.write.handle = op->handle;
                    op->uni_cmd.data.write.length = op->cmd->data.copy.size;
                    op->uni_cmd.data.write.buffer = op->cmd->data.copy.buffer;
                    err = unified_back(&op->uni_cmd, &op->uni_reply, op, upload_callback);
                }
                else if ((op->uni_cmd.op == UNIFIED_SIZE)
                         ||(op->uni_cmd.op == UNIFIED_WRITE))
                {
                    // Close the file
                    done = FALSE;
                    op->err = err;
                    op->uni_cmd.op = UNIFIED_CLOSE;
                    op->uni_cmd.data.close.handle = op->handle;
                    err = unified_back(&op->uni_cmd, &op->uni_reply, op, upload_callback);
                }
                else if (!err && (op->uni_cmd.op == UNIFIED_CLOSE))
                {
                    // Restore the previous error
                    err = op->err;

                    // Set the file attributes if no error
                    if (!err)
                    {
                        done = FALSE;
                        op->uni_cmd.op = UNIFIED_ACCESS;
                        op->uni_cmd.data.access.attr = op->cmd->data.copy.attr;
                        if (sizeof(op->uni_cmd.data.access.path) <= strlen(op->cmd->data.copy.path)) err = &err_bad_name;
                        if (!err)
                        {
                            strcpy(op->uni_cmd.data.access.path, op->cmd->data.copy.path);
                            err = unified_back(&op->uni_cmd, &op->uni_reply, op, upload_callback);
                        }
                    }
                }
                else if (!err && (op->uni_cmd.op == UNIFIED_ACCESS)
                         && (op->state == UPLOAD_STATE_CREATED))
                {
                    // Set the date and time stamp
                    done = FALSE;
                    op->uni_cmd.op = UNIFIED_STAMP;
                    op->uni_cmd.data.stamp.date = op->cmd->data.copy.date;
                    if (sizeof(op->uni_cmd.data.stamp.path) <= strlen(op->cmd->data.copy.path)) err = &err_bad_name;
                    if (!err)
                    {
                        strcpy(op->uni_cmd.data.stamp.path, op->cmd->data.copy.path);
                        err = unified_back(&op->uni_cmd, &op->uni_reply, op, upload_callback);
                    }
                }
                break;

            default:
                // Not a supported command
                if (!err) err = &err_bad_upload_op;
                break;
        }

        // Call the callback function if appropriate
        if (err) err = upload_done(op, err);
        else if (done) err = upload_done(op, NULL);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start processing an operation.
*/
static os_error *upload_begin(upload_private *op)
{
    os_error *err = NULL;

    // Check function parameters
    if (!op) err = &err_bad_parms;
    else
    {
        bool done = FALSE;

        // Disable idle timeouts
        idle_start();

        // Action depends on the specified operation
        switch (op->cmd->op)
        {
            case UPLOAD_COPY:
                // Copy the specified file
                op->uni_cmd.op = UNIFIED_ACCESS;
                op->uni_cmd.data.access.attr = fileswitch_ATTR_OWNER_READ
                                               | fileswitch_ATTR_OWNER_WRITE;
                if (sizeof(op->uni_cmd.data.access.path) <= strlen(op->cmd->data.copy.path)) err = &err_bad_name;
                if (!err)
                {
                    strcpy(op->uni_cmd.data.access.path, op->cmd->data.copy.path);
                    err = unified_back(&op->uni_cmd, &op->uni_reply, op, upload_callback);
                }
                break;

            default:
                // Not a supported command
                err = &err_bad_upload_op;
                break;
        }

        // Pass any error to the callback function
        if (err) err = upload_done(op, err);
        else if (done) err = upload_done(op, NULL);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : cmd           - Details of the file to upload.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Trigger a background remote file upload operation.
*/
os_error *upload_file(const upload_cmd *cmd, void *user,
                      share_callback callback)
{
    os_error *err = NULL;

    // Check function parameters
    if (!cmd || !callback) err = &err_bad_parms;
    else
    {
        upload_private *ptr;

        // Obtain an operation record pointer
        if (upload_free_list)
        {
            // A suitable record already exists
            ptr = upload_free_list;
            if (ptr->next) ptr->next->prev = NULL;
            upload_free_list = ptr->next;
        }
        else
        {
            // Allocate a new structure
            ptr = (upload_private *) MEM_MALLOC(sizeof(upload_private));
            if (!ptr) err = &err_buffer;
        }

        // Complete the details
        if (!err)
        {
            // Add to the active list
            ptr->prev = NULL;
            ptr->next = upload_active_list;
            if (upload_active_list) upload_active_list->prev = ptr;
            upload_active_list = ptr;

            // Copy the command details
            ptr->cmd = cmd;
            ptr->user = user;
            ptr->callback = callback;

            // Always start in the initial state
            ptr->state = UPLOAD_STATE_INITIAL;

            // Start the operation
            err = upload_begin(ptr);
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
static os_error *upload_free(void)
{
    os_error *err = NULL;

    // Free operation buffers
    while (upload_free_list)
    {
        upload_private *ptr = upload_free_list;

        // Update the free list head pointer
        upload_free_list = ptr->next;

        // Free the memory
        MEM_FREE(ptr);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : era           - Is an ERA device connected.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the remote file uploader.
*/
os_error *upload_start(bool era)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting remote file uploader era=%u", era))

    // No action if already active
    if (!upload_active)
    {
        // Store the remote device type
        upload_era = era;

        // Set the active flag if successful
        if (!err) upload_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the remote file uploader.
*/
os_error *upload_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending remote file uploader now=%u", now))

    // No action unless active
    if (upload_active)
    {
        // Free any allocated memory
        err = upload_free();

        // Clear the active flag if successful
        if (!err) upload_active = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the remote file uploader.
*/
os_error *upload_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying remote file uploader status"))

    // Display the current transfer status
    if (upload_active_list)
    {
        printf("Uploading files to the remote device.\n");
    }

    // Return any error produced
    return err;
}
