/*
    File        : clipboard.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Local clipboard handling for the PsiFS module.

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
#include "clipboard.h"

// Include clib header files
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// Include oslib header files
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "async.h"
#include "cache.h"
#include "debug.h"
#include "err.h"
#include "idle.h"
#include "pollword.h"
#include "psifs.h"
#include "rclip.h"
#include "uid.h"
#include "unified.h"
#include "util.h"

// The clipboard status
static psifs_clipboard_flags clipboard_flags = 0;
static os_t clipboard_modified = 0;

// Local clipboard files
#define CLIPBOARD_TEMP_DIR "<PsiFSScrap$Dir>"
#define CLIPBOARD_TEMP_SUBDIR "<PsiFSScrap$Dir>.Clipboard"
#define CLIPBOARD_FILE_LOCAL CLIPBOARD_TEMP_SUBDIR ".Active"
#define CLIPBOARD_FILE_TRANSFER CLIPBOARD_TEMP_SUBDIR ".Transfer"

// Remote clipboard file
#define CLIPBOARD_FILE_REMOTE ":C.$.System.Data.Clpboard/cbd"

// Remote clipboard transfer status
typedef enum
{
    CLIPBOARD_IDLE,
    CLIPBOARD_READ_OPEN,
    CLIPBOARD_READ_ARGS,
    CLIPBOARD_READ_TRANSFER,
    CLIPBOARD_READ_CLOSE,
    CLIPBOARD_WRITE_OPEN,
    CLIPBOARD_WRITE_EXTENT,
    CLIPBOARD_WRITE_TRANSFER,
    CLIPBOARD_WRITE_ACCESS,
    CLIPBOARD_WRITE_CLOSE,
    CLIPBOARD_CLOSE
} clipboard_state;

// Reading and writing remote clipboard
static bool clipboard_transfer_required = FALSE;
static clipboard_state clipboard_transfer_state = CLIPBOARD_IDLE;
static cache_cmd clipboard_transfer_cmd;
static cache_reply clipboard_transfer_reply;
static bits clipboard_transfer_remain;

// Buffer for copying clipboard data
#define CLIPBOARD_MAX_BUFFER (256)
static byte cache_transfer_buffer[CLIPBOARD_MAX_BUFFER];

// Function prototypes
static os_error *clipboard_transfer(bool required);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set the file type of the local clipboard file.
*/
static os_error *clipboard_settype(void)
{
    os_error *err = NULL;
    os_fw handle;

    // Attempt to open the clipboard file
    err = xosfind_openinw(osfind_NO_PATH | osfind_ERROR_IF_DIR,
                          CLIPBOARD_FILE_LOCAL, NULL, &handle);
    if (!err && (handle != 0))
    {
        struct
        {
            epoc32_file_uid uid;
            bits uid4;
        } uids;
        bits uid4;
        int unread;
        bits type;

        // Attempt to read the UID
        err = xosgbpb_readw(handle, (byte *) &uids, sizeof(uids), &unread);

        // Close the file
        xosfind_closew(handle);

        // Choose the filetype to set
        if (!err && !unread) err = uid_checksum_uid(&uids.uid, &uid4);
        if (!err) type = uid_map_type(NULL, !unread && (uids.uid4 == uid4) ? &uids.uid : NULL);

        // Set the file type
        if (!err) err = xosfile_set_type(CLIPBOARD_FILE_LOCAL, type);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Tidy up if possible.
*/
static os_error *clipboard_tidy(void)
{
    os_error *err = NULL;

    // No action unless inactive without transfers in progress
    if (!(clipboard_flags & psifs_CLIPBOARD_SERVER_ACTIVE)
        && (clipboard_transfer_state == CLIPBOARD_IDLE)
        && !clipboard_transfer_required)
    {
        // Delete the temporary directory
        xosfscontrol_wipe(CLIPBOARD_TEMP_SUBDIR, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
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
static os_error *clipboard_callback(void *user, os_error *err,
                                    const void *reply)
{
    DEBUG_ERR(err);

    // Action depends on the current state
    switch (clipboard_transfer_state)
    {
        case CLIPBOARD_READ_OPEN:
            // Remote clipboard file opened for reading
            DEBUG_PRINTF(("Clipboard callback - read open"))
            if (err)
            {
                // Give up if failed to open the file
                err = NULL;
                clipboard_transfer_state = CLIPBOARD_IDLE;
            }
            else
            {
                // Read the file extent
                clipboard_transfer_state = CLIPBOARD_READ_ARGS;
                clipboard_transfer_cmd.op = CACHE_ARGS;
                clipboard_transfer_cmd.data.args.handle = clipboard_transfer_reply.open.handle;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
            }
            break;

        case CLIPBOARD_READ_ARGS:
            // Read the file extent
            DEBUG_PRINTF(("Clipboard callback - read args"))
            if (err)
            {
                // Close the file and give up if failed to read details
                clipboard_transfer_state = CLIPBOARD_CLOSE;
                clipboard_transfer_cmd.op = CACHE_CLOSE;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
            }
            else
            {
                // Store the file size
                clipboard_transfer_remain = clipboard_transfer_reply.args.info.extent;

                // Start reading from the file
                clipboard_transfer_state = CLIPBOARD_READ_TRANSFER;
                clipboard_transfer_cmd.op = CACHE_READ;
                clipboard_transfer_cmd.data.read.offset = 0;
                clipboard_transfer_cmd.data.read.length = CLIPBOARD_MAX_BUFFER;
                if (clipboard_transfer_remain < CLIPBOARD_MAX_BUFFER) clipboard_transfer_cmd.data.read.length = clipboard_transfer_remain;
                clipboard_transfer_cmd.data.read.buffer = cache_transfer_buffer;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
            }
            break;

        case CLIPBOARD_READ_TRANSFER:
            // Block of data read from clipboard file
            DEBUG_PRINTF(("Clipboard callback - read transfer"))
            if (!err)
            {
                os_fw handle;

                // Write the block of data to the local file
                err = xosfind_openupw(osfind_NO_PATH | osfind_ERROR_IF_ABSENT | osfind_ERROR_IF_DIR, CLIPBOARD_FILE_TRANSFER, NULL, &handle);
                if (!err)
                {
                    err = xosgbpb_write_atw(handle, clipboard_transfer_cmd.data.read.buffer, clipboard_transfer_cmd.data.read.length, clipboard_transfer_cmd.data.read.offset, NULL);
                    xosfind_closew(handle);
                }
            }
            if (err)
            {
                // Close the file and give up if failed to read
                clipboard_transfer_state = CLIPBOARD_CLOSE;
                clipboard_transfer_cmd.op = CACHE_CLOSE;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
            }
            else
            {
                // Check if the whole file has been read
                clipboard_transfer_remain -= clipboard_transfer_cmd.data.read.length;
                if (clipboard_transfer_remain)
                {
                    // Read the next block of data
                    clipboard_transfer_cmd.data.read.offset += clipboard_transfer_cmd.data.read.length;
                    if (clipboard_transfer_remain < CLIPBOARD_MAX_BUFFER) clipboard_transfer_cmd.data.read.length = clipboard_transfer_remain;
                    err = cache_back(&clipboard_transfer_cmd,
                                     &clipboard_transfer_reply, NULL,
                                     clipboard_callback);
                }
                else
                {
                    // Close the file
                    clipboard_transfer_state = CLIPBOARD_READ_CLOSE;
                    clipboard_transfer_cmd.op = CACHE_CLOSE;
                    err = cache_back(&clipboard_transfer_cmd,
                                     &clipboard_transfer_reply, NULL,
                                     clipboard_callback);
                }
            }
            break;

        case CLIPBOARD_READ_CLOSE:
            DEBUG_PRINTF(("Clipboard callback - read close"))
            // Clipboard file closed after reading remote clipboard
            if (err)
            {
                // Ignore any error
                err = NULL;
            }
            else if (!clipboard_transfer_required)
            {
                // Copy the new clipboard contents
                err = xosfscontrol_copy(CLIPBOARD_FILE_TRANSFER,
                                        CLIPBOARD_FILE_LOCAL,
                                        osfscontrol_COPY_RECURSE
                                        | osfscontrol_COPY_FORCE
                                        | osfscontrol_COPY_LOOK,
                                        0, 0, 0, 0, NULL);

                // Update the clipboard status
                if (!err) clipboard_flags |= psifs_CLIPBOARD_REMOTE_SYNC;

                // Update any pollwords
                if (!err) err = pollword_update(psifs_MASK_CLIPBOARD_STATUS);

                // Set the type of the new clipboard file
                if (!err) err = clipboard_settype();
            }
            clipboard_transfer_state = CLIPBOARD_IDLE;
            break;

        case CLIPBOARD_WRITE_OPEN:
            // Remote clipboard file opened for writing
            DEBUG_PRINTF(("Clipboard callback - write open"))
            if (!err)
            {
                // Read the size of the file
                err = xosfile_read_stamped_no_path(CLIPBOARD_FILE_TRANSFER, NULL, NULL, NULL, (int *) &clipboard_transfer_remain, NULL, NULL);
            }
            if (err)
            {
                // Give up if failed to open the file
                rclip_notify_end();
                clipboard_transfer_state = CLIPBOARD_IDLE;
            }
            else
            {
                // Write the file extent
                clipboard_transfer_state = CLIPBOARD_WRITE_EXTENT;
                clipboard_transfer_cmd.op = CACHE_EXTENT;
                clipboard_transfer_cmd.data.extent.handle = clipboard_transfer_reply.open.handle;
                clipboard_transfer_cmd.data.extent.size = clipboard_transfer_remain;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
            }
            break;

        case CLIPBOARD_WRITE_EXTENT:
            // Remote clipboard file extent set
            DEBUG_PRINTF(("Clipboard callback - write extent"))
            if (err)
            {
                // Close the file and give up if failed to write
                rclip_notify_end();
                clipboard_transfer_state = CLIPBOARD_CLOSE;
                clipboard_transfer_cmd.op = CACHE_CLOSE;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
                break;
            }
            else
            {
                // Start writing to the file
                clipboard_transfer_state = CLIPBOARD_WRITE_TRANSFER;
                clipboard_transfer_cmd.op = CACHE_WRITE;
                clipboard_transfer_cmd.data.write.offset = 0;
                clipboard_transfer_cmd.data.write.length = 0;
                clipboard_transfer_cmd.data.write.buffer = cache_transfer_buffer;
            }
            // Fall through!

        case CLIPBOARD_WRITE_TRANSFER:
            // Block of data written to remote clipboard
            DEBUG_PRINTF(("Clipboard callback - write transfer"))
            if (!err)
            {
                // Check if the whole file has been written
                clipboard_transfer_remain -= clipboard_transfer_cmd.data.write.length;
                if (clipboard_transfer_remain)
                {
                    os_fw handle;

                    // Read the next block of data from the file
                    clipboard_transfer_cmd.data.write.offset += clipboard_transfer_cmd.data.write.length;
                    clipboard_transfer_cmd.data.write.length = CLIPBOARD_MAX_BUFFER;
                    if (clipboard_transfer_remain < CLIPBOARD_MAX_BUFFER) clipboard_transfer_cmd.data.write.length = clipboard_transfer_remain;

                    // Write the block of data to the local file
                    err = xosfind_openinw(osfind_NO_PATH | osfind_ERROR_IF_ABSENT | osfind_ERROR_IF_DIR, CLIPBOARD_FILE_TRANSFER, NULL, &handle);
                    if (!err)
                    {
                        err = xosgbpb_read_atw(handle, clipboard_transfer_cmd.data.read.buffer, clipboard_transfer_cmd.data.read.length, clipboard_transfer_cmd.data.read.offset, NULL);
                        xosfind_closew(handle);
                    }
                }
            }
            if (err)
            {
                // Close the file and give up if failed to write
                rclip_notify_end();
                clipboard_transfer_state = CLIPBOARD_CLOSE;
                clipboard_transfer_cmd.op = CACHE_CLOSE;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
            }
            else
            {
                // Check if the whole file has been written
                if (clipboard_transfer_remain)
                {
                    // Write the next block of data
                    err = cache_back(&clipboard_transfer_cmd,
                                     &clipboard_transfer_reply, NULL,
                                     clipboard_callback);
                }
                else
                {
                    // Close the file
                    clipboard_transfer_state = CLIPBOARD_WRITE_CLOSE;
                    clipboard_transfer_cmd.op = CACHE_CLOSE;
                    err = cache_back(&clipboard_transfer_cmd,
                                     &clipboard_transfer_reply, NULL,
                                     clipboard_callback);
                }
            }
            break;

        case CLIPBOARD_WRITE_CLOSE:
            // Clipboard file closed after writing remote clipboard
            DEBUG_PRINTF(("Clipboard callback - write close"))
            if (err)
            {
                // Ignore any error
                err = NULL;
            }
            else if (!clipboard_transfer_required)
            {
                // Update the clipboard status
                clipboard_flags |= psifs_CLIPBOARD_LOCAL_SYNC;

                // Update any pollwords
                err = pollword_update(psifs_MASK_CLIPBOARD_STATUS);
            }
            clipboard_transfer_state = CLIPBOARD_IDLE;
            rclip_notify_end();
            break;

        case CLIPBOARD_CLOSE:
            // File closed after error condition
            DEBUG_PRINTF(("Clipboard callback - close"))
            err = NULL;
            clipboard_transfer_state = CLIPBOARD_IDLE;
            break;

        default:
            // No other states should occur
            err = &err_clipboard_state;
            break;
    }

    // Additional actions if idle
    if (clipboard_transfer_state == CLIPBOARD_IDLE)
    {
        // Re-enable the idle timeout
        idle_end();

        // Attempt to start a new transfer
        if (!err) err = clipboard_transfer(FALSE);

        // Attempt to tidy up
        if (!err) err = clipboard_tidy();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : required      - Is a new transfer required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start a clipboard transfer if required and possible.
*/
static os_error *clipboard_transfer(bool required)
{
    os_error *err = NULL;

    // Set the transfer flag if necessary
    if (required) clipboard_transfer_required = TRUE;

    // Start a new transfer if required and idle
    if (clipboard_transfer_required
        && (clipboard_transfer_state == CLIPBOARD_IDLE))
    {
        // Clear the transfer required flag
        clipboard_transfer_required = FALSE;

        // Start the required transfer
        if ((clipboard_flags & psifs_CLIPBOARD_REMOTE_CHANGED)
            && !(clipboard_flags & psifs_CLIPBOARD_REMOTE_SYNC))
        {
            DEBUG_PRINTF(("Clipboard transfer - read"))

            // Create a zero length transfer file
            err = xosfile_create_stamped(CLIPBOARD_FILE_TRANSFER,
                                        osfile_TYPE_DATA, 0);
            if (!err)
            {
                // Disable the idle timeout
                idle_start();

                // Start the operation
                clipboard_transfer_state = CLIPBOARD_READ_OPEN;
                clipboard_transfer_cmd.op = CACHE_OPEN;
                strcpy(clipboard_transfer_cmd.data.open.path,
                       CLIPBOARD_FILE_REMOTE);
                clipboard_transfer_cmd.data.open.mode = FS_MODE_IN;
                clipboard_transfer_cmd.data.open.handle = 0;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
            }
        }
        else if ((clipboard_flags & psifs_CLIPBOARD_LOCAL_CHANGED)
                 && !(clipboard_flags & psifs_CLIPBOARD_LOCAL_SYNC))
        {
            DEBUG_PRINTF(("Clipboard transfer - write"))

            // Copy the current clipboard file
            err = xosfscontrol_copy(CLIPBOARD_FILE_LOCAL,
                                    CLIPBOARD_FILE_TRANSFER,
                                    osfscontrol_COPY_RECURSE
                                    | osfscontrol_COPY_FORCE
                                    | osfscontrol_COPY_LOOK,
                                    0, 0, 0, 0, NULL);
            if (!err)
            {
                // Disable the idle timeout
                idle_start();

                // Start the operation
                clipboard_transfer_state = CLIPBOARD_WRITE_OPEN;
                clipboard_transfer_cmd.op = CACHE_OPEN;
                strcpy(clipboard_transfer_cmd.data.open.path,
                       CLIPBOARD_FILE_REMOTE);
                clipboard_transfer_cmd.data.open.mode = FS_MODE_OUT;
                clipboard_transfer_cmd.data.open.handle = 0;
                err = cache_back(&clipboard_transfer_cmd,
                                 &clipboard_transfer_reply, NULL,
                                 clipboard_callback);
            }

            // Start the remote clipboard notify
            if (!err) err = rclip_notify_start();
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : flags         - Variable to receive the current clipboard
                                  status flags.
                  timestamp     - Variable to receive the timestamp of the
                                  last change to the clipboard.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Poll the status of the remote clipboard.
*/
os_error *clipboard_poll(psifs_clipboard_flags *flags, os_t *timestamp)
{
    os_error *err = NULL;

    // Check parameters
    if (!flags || !timestamp) err = &err_bad_parms;
    else
    {
        // Set the return values
        *flags = clipboard_flags;
        *timestamp = clipboard_modified;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : name          - The name of the file to copy to the clipboard.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Copy to the local clipboard.
*/
os_error *clipboard_copy(const char *name)
{
    os_error *err = NULL;

    // Check parameters
    if (!name) err = &err_bad_parms;
    else if (!(clipboard_flags & psifs_CLIPBOARD_SERVER_ACTIVE))
    {
        err = &err_clipboard_not_active;
    }
    else
    {
        // Clear the clipboard status flags
        clipboard_flags &= ~(psifs_CLIPBOARD_REMOTE_CHANGED
                             | psifs_CLIPBOARD_REMOTE_SYNC
                             | psifs_CLIPBOARD_LOCAL_CHANGED
                             | psifs_CLIPBOARD_LOCAL_SYNC);
        clipboard_modified = util_time();

        // Update any pollwords
        err = pollword_update(psifs_MASK_CLIPBOARD_STATUS);

        // Copy the file to the local clipboard
        if (!err)
        {
            err = xosfscontrol_copy(name, CLIPBOARD_FILE_LOCAL,
                                    osfscontrol_COPY_RECURSE
                                    | osfscontrol_COPY_FORCE
                                    | osfscontrol_COPY_LOOK,
                                    0, 0, 0, 0, NULL);
        }

        // Set the type of the new clipboard file
        if (!err) err = clipboard_settype();

        // Mark the clipboard as locally modified
        if (!err) clipboard_flags |= psifs_CLIPBOARD_LOCAL_CHANGED;

        // Start a transfer operation if possible
        if (!err) err = clipboard_transfer(TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : name          - The name of the file to paste the clipboard
                                  contents to.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Paste from the local clipboard.
*/
os_error *clipboard_paste(const char *name)
{
    os_error *err = NULL;

    // Check parameters
    if (!name) err = &err_bad_parms;
    else if (!(clipboard_flags & psifs_CLIPBOARD_SERVER_ACTIVE))
    {
        err = &err_clipboard_not_active;
    }
    else if (!(clipboard_flags & (psifs_CLIPBOARD_REMOTE_SYNC
                                  | psifs_CLIPBOARD_LOCAL_CHANGED)))
    {
        err = &err_clipboard_not_sync;
    }
    else
    {
        // Copy the file from the local clipboard
        err = xosfscontrol_copy(CLIPBOARD_FILE_LOCAL, name,
                                osfscontrol_COPY_RECURSE
                                | osfscontrol_COPY_FORCE
                                | osfscontrol_COPY_LOOK,
                                0, 0, 0, 0, NULL);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the remote clipboard to the local clipboard file.
*/
os_error *clipboard_remote_copy(void)
{
    os_error *err = NULL;

    // Ignore unless the clipboard server is active
    if (clipboard_flags & psifs_CLIPBOARD_SERVER_ACTIVE)
    {
        // Flush the file cache
        err = cache_recache(CLIPBOARD_FILE_REMOTE);

        // Mark the clipboard as remotely modified
        if (!err)
        {
            clipboard_flags &= ~(psifs_CLIPBOARD_REMOTE_SYNC
                                 | psifs_CLIPBOARD_LOCAL_CHANGED
                                 | psifs_CLIPBOARD_LOCAL_SYNC);
            clipboard_flags |= psifs_CLIPBOARD_REMOTE_CHANGED;
            clipboard_modified = util_time();
        }

        // Update any pollwords
        if (!err) err = pollword_update(psifs_MASK_CLIPBOARD_STATUS);

        // Start a transfer operation if possible
        if (!err) err = clipboard_transfer(TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the local clipboard services layer after the
                  multiplexor layer has been started.
*/
os_error *clipboard_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting local clipboard layer"))

    // No action if already active
    if (!(clipboard_flags & psifs_CLIPBOARD_SERVER_ACTIVE))
    {
        // Ensure that the temporary directory exists
        err = xosfile_create_dir(CLIPBOARD_TEMP_DIR, 0);
        if (!err) err = xosfile_create_dir(CLIPBOARD_TEMP_SUBDIR, 0);

        // Set the active flag if successful
        if (!err) clipboard_flags = psifs_CLIPBOARD_SERVER_ACTIVE;

        // Update any pollwords
        if (!err) err = pollword_update(psifs_MASK_CLIPBOARD_STATUS);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the local clipboard services layer before the multiplexor
                  layer has been closed.
*/
os_error *clipboard_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending local clipboard layer now=%u", now))

    // No action unless active
    if (clipboard_flags & psifs_CLIPBOARD_SERVER_ACTIVE)
    {
        // Clear the active flag
        clipboard_flags = 0;
        clipboard_transfer_required = FALSE;

        // Update any pollwords
        err = pollword_update(psifs_MASK_CLIPBOARD_STATUS);

        // Attempt to tidy up
        if (!err) err = clipboard_tidy();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the local clipboard.
*/
os_error *clipboard_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying local clipboard layer status"))

    // Display the current clipboard status
    if (clipboard_flags & psifs_CLIPBOARD_SERVER_ACTIVE)
    {
        printf("Clipboard active");
        if (clipboard_flags & psifs_CLIPBOARD_REMOTE_CHANGED)
        {
            printf(", %s remote",
                   clipboard_flags & psifs_CLIPBOARD_REMOTE_SYNC
                   ? "synchronized with" : "modified on");
        }
        if (clipboard_flags & psifs_CLIPBOARD_LOCAL_CHANGED)
        {
            printf(", %s local",
                   clipboard_flags & psifs_CLIPBOARD_LOCAL_SYNC
                   ? "synchronized with" : "modified on");
        }
        printf(".\n");
    }
    else printf("Local clipboard not active.\n");

    // Return any error produced
    return err;
}
