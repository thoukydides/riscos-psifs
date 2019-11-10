/*
    File        : rfsv32.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Remote file services client for the PsiFS module.

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
#include "rfsv32.h"

// Include clib header files
#include <string.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "lnkchan.h"
#include "mux.h"
#include "parse.h"

// Maximum message size
#define RFSV32_MAX_FRAME (2079)

// The channel details
#define RFSV32_CHANNEL_NAME "SYS$RFSV.*"
static mux_channel rfsv32_channel;

// Link details for this channel
#define RFSV32_LNKCHAN_NAME "SYS$RFSV"
static lnkchan_reply rfsv32_lnkchan_reply;

// Is the channel active
static bool rfsv32_active = FALSE;

// Shared access handler
share_handle rfsv32_share_handle = SHARE_NONE;

// Current operation ID
#define RFSV32_MAX_ID (0xffff)
static bits rfsv32_id = 0;

// Mask for sting lengths
#define RFSV32_LEN_MASK_WORD (0x8000)
#define RFSV32_LEN_MASK_BITS (0xf0000000)

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for RFSV32 channel to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *rfsv32_fore(const rfsv32_cmd *cmd, rfsv32_reply *reply, bool escape)
{
    os_error *err = NULL;

    // Check parameters
    if (!cmd || !reply) err = &err_bad_parms;
    else
    {
        // Perform the operation
        err = share_fore(rfsv32_share_handle, cmd, reply, escape);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the specified operation when the RFSV channel becomes
                  idle. Control is returned immediately. If the channel is not
                  valid or the operation fails then no error is returned, but
                  instead the callback function is notified.
*/
os_error *rfsv32_back(const rfsv32_cmd *cmd, rfsv32_reply *reply,
                      void *user, share_callback callback)
{
    os_error *err = NULL;

    // Check parameters
    if (!cmd || !reply || !callback) err = &err_bad_parms;
    else
    {
        // Perform the operation
        err = share_back(rfsv32_share_handle, cmd, reply, user, callback);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - The string value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
static os_error *rfsv32_put_des(const char *value)
{
    os_error *err = NULL;

    // Check parameters
    if (!value) err = &err_bad_parms;
    else
    {
        bits len = strlen(value);

        // Add the string prefixed by its length
        err = parse_put_word(len);
        if (!err) err = parse_put_string_len(len, value);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Function to start an operation.
*/
static os_error *rfsv32_send(const void *cmd, void *reply)
{
    os_error *err = NULL;
    static byte buffer[RFSV32_MAX_FRAME];
    rfsv32_cmd *in = (rfsv32_cmd *) cmd;

    // Check parameters
    if (!cmd || !reply) err = &err_bad_parms;
    else
    {
        bits offset = 0;

        // Choose a new operation ID
        if (rfsv32_id < RFSV32_MAX_ID) rfsv32_id++;
        else rfsv32_id = 0;

        // Write the standard header
        err = parse_put_start(buffer, RFSV32_MAX_FRAME, &offset);
        if (!err) err = parse_put_word(in->op);
        if (!err) err = parse_put_word(rfsv32_id);

        // Add any command specific data
        switch (in->op)
        {
            case RFSV32_REQ_CLOSE_HANDLE:
                // Close a directory or file handle
                if (!err) err = parse_put_bits(in->data.req_close_handle.handle);
                break;

            case RFSV32_REQ_OPEN_DIR:
                // Open remote directory
                if (!err) err = parse_put_bits(in->data.req_open_dir.attributes);
                if (!err) err = rfsv32_put_des(in->data.req_open_dir.match);
                break;

            case RFSV32_REQ_READ_DIR:
                // Request a directory listing
                if (!err) err = parse_put_bits(in->data.req_read_dir.handle);
                break;

            case RFSV32_REQ_VOLUME:
                // Get volume name
                if (!err) err = parse_put_bits(in->data.req_volume.drive);
                break;

            case RFSV32_REQ_SET_VOLUME_LABEL:
                // Set volume name
                if (!err) err = parse_put_bits(in->data.req_set_volume_label.drive);
                if (!err) err = rfsv32_put_des(in->data.req_set_volume_label.name);
                break;

            case RFSV32_REQ_OPEN_FILE:
                // Open remote file
                if (!err) err = parse_put_bits(in->data.req_open_file.mode);
                if (!err) err = rfsv32_put_des(in->data.req_open_file.name);
                break;

            case RFSV32_REQ_TEMP_FILE:
                // Open temporary remote file
                if (!err) err = parse_put_bits(in->data.req_temp_file.mode);
                if (!err) err = rfsv32_put_des(in->data.req_temp_file.name);
                break;

            case RFSV32_REQ_READ_FILE:
                // Read from a remote file
                if (!err) err = parse_put_bits(in->data.req_read_file.handle);
                if (!err) err = parse_put_bits(in->data.req_read_file.length);
                break;

            case RFSV32_REQ_WRITE_FILE:
                // Write to a remote file
                {
                    bits i;
                    const byte *ptr = (const byte *) in->data.req_write_file.buffer;

                    if (!err) err = parse_put_bits(in->data.req_write_file.handle);
                    for (i = 0; !err && (i < in->data.req_write_file.length); i++)
                    {
                        err = parse_put_byte(*ptr++);
                    }
                }
                break;

            case RFSV32_REQ_SEEK_FILE:
                // Seek to offset in remote file
                if (!err) err = parse_put_bits((bits) in->data.req_seek_file.offset);
                if (!err) err = parse_put_bits(in->data.req_seek_file.handle);
                if (!err) err = parse_put_bits(in->data.req_seek_file.sense);
                break;

            case RFSV32_REQ_DELETE:
                // Delete named remote file
                if (!err) err = rfsv32_put_des(in->data.req_delete.name);
                break;

            case RFSV32_REQ_REMOTE_ENTRY:
                //  Get entry details for a remote file
                if (!err) err = rfsv32_put_des(in->data.req_remote_entry.name);
                break;

            case RFSV32_REQ_FLUSH:
                // Flush remote file to disk
                if (!err) err = parse_put_bits(in->data.req_flush.handle);
                break;

            case RFSV32_REQ_SET_SIZE:
                // Set the size of a remote file
                // !!!
                if (!err) err = parse_put_bits(in->data.req_set_size.handle);
                if (!err) err = parse_put_bits(in->data.req_set_size.size);
                break;

            case RFSV32_REQ_RENAME:
                // Rename a remote file
                if (!err) err = rfsv32_put_des(in->data.req_rename.src);
                if (!err) err = rfsv32_put_des(in->data.req_rename.dest);
                break;

            case RFSV32_REQ_MK_DIR_ALL:
                // Make remote directory
                if (!err) err = rfsv32_put_des(in->data.req_mk_dir_all.name);
                break;

            case RFSV32_REQ_RM_DIR:
                // Remote remote directory
                if (!err) err = rfsv32_put_des(in->data.req_rm_dir.name);
                break;

            case RFSV32_REQ_SET_ATT:
                // Set remote file attributes
                if (!err) err = parse_put_bits(in->data.req_set_att.set);
                if (!err) err = parse_put_bits(in->data.req_set_att.clear);
                if (!err) err = rfsv32_put_des(in->data.req_set_att.name);
                break;

            case RFSV32_REQ_ATT:
                // Get remote file attributes
                if (!err) err = rfsv32_put_des(in->data.req_att.name);
                break;

            case RFSV32_REQ_SET_MODIFIED:
                // Set modified time
                if (!err) err = parse_put_bits(in->data.req_set_modified.modified.low);
                if (!err) err = parse_put_bits(in->data.req_set_modified.modified.high);
                if (!err) err = rfsv32_put_des(in->data.req_set_modified.name);
                break;

            case RFSV32_REQ_MODIFIED:
                // Get modified time
                if (!err) err = rfsv32_put_des(in->data.req_modified.name);
                break;

            case RFSV32_REQ_SET_SESSION_PATH:
                // Set session path
                if (!err) err = rfsv32_put_des(in->data.req_set_session_path.name);
                break;

            case RFSV32_REQ_READ_WRITE_FILE:
                // Read from one file and write to another
                if (!err) err = parse_put_bits(in->data.req_read_write_file.length);
                if (!err) err = parse_put_bits(in->data.req_read_write_file.dest);
                if (!err) err = parse_put_bits(in->data.req_read_write_file.src);
                break;

            case RFSV32_REQ_CREATE_FILE:
                // Create file
                if (!err) err = parse_put_bits(in->data.req_create_file.mode);
                if (!err) err = rfsv32_put_des(in->data.req_create_file.name);
                break;

            case RFSV32_REQ_REPLACE_FILE:
                // Replace file
                if (!err) err = parse_put_bits(in->data.req_replace_file.mode);
                if (!err) err = rfsv32_put_des(in->data.req_replace_file.name);
                break;

            case RFSV32_REQ_PATH_TEST:
                // Test a path exists
                if (!err) err = rfsv32_put_des(in->data.req_path_test.name);
                break;

            case RFSV32_REQ_LOCK:
                // Lock a file
                if (!err) err = parse_put_bits(in->data.req_lock.length);
                if (!err) err = parse_put_bits(in->data.req_lock.offset);
                if (!err) err = parse_put_bits(in->data.req_lock.handle);
                break;

            case RFSV32_REQ_UNLOCK:
                // Unlock a file
                if (!err) err = parse_put_bits(in->data.req_unlock.length);
                if (!err) err = parse_put_bits(in->data.req_unlock.offset);
                if (!err) err = parse_put_bits(in->data.req_unlock.handle);
                break;

            case RFSV32_REQ_OPEN_DIR_UID:
                // Open remote directory with UID filter
                if (!err) err = parse_put_bits(in->data.req_open_dir_uid.uid.uid1);
                if (!err) err = parse_put_bits(in->data.req_open_dir_uid.uid.uid2);
                if (!err) err = parse_put_bits(in->data.req_open_dir_uid.uid.uid3);
                if (!err) err = rfsv32_put_des(in->data.req_open_dir_uid.match);
                break;

            case RFSV32_REQ_DRIVE_NAME:
                // Request drive name
                if (!err) err = parse_put_bits(in->data.req_drive_name.drive);
                break;

            case RFSV32_REQ_SET_DRIVE_NAME:
                // Set drive name
                if (!err) err = parse_put_bits(in->data.req_set_drive_name.drive);
                if (!err) err = rfsv32_put_des(in->data.req_set_drive_name.name);
                break;

            case RFSV32_REQ_REPLACE:
                // Replace remote file
                if (!err) err = rfsv32_put_des(in->data.req_replace.src);
                if (!err) err = rfsv32_put_des(in->data.req_replace.dest);
                break;

            case RFSV32_REQ_GET_DRIVE_LIST:
            case RFSV32_REQ_SESSION_PATH:
                // No additional information
                break;

            default:
                // Not a supported command
                if (!err) err = &err_bad_rfsv_op;
                break;
        }

        // Send the command
        if (!err) err = mux_chan_tx_server(rfsv32_channel, buffer, offset);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - Variable to receive the string.
                  size          - Size of the buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next string from the current buffer.
*/
static os_error *rfsv32_get_des(char *value, bits size)
{
    os_error *err = NULL;

    // Check parameters
    if (!value || !size) err = &err_bad_parms;
    else
    {
        unsigned short len;

        // Read the string length
        err = parse_get_word(&len);
        if (!err && (size <= len)) err = &err_rfsv_len;

        // Read the string
        if (!err) err = parse_get_string_len(len, value);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : entry         - Variable to receive the remote entry.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next remote entry value from the current buffer.
*/
static os_error *rfsv32_get_remote_entry(epoc32_remote_entry *entry)
{
    os_error *err = NULL;

    // Check parameters
    if (!entry) err = &err_bad_parms;
    else
    {
        bits short_len;
        bits long_len;

        // Always start from a 4 byte boundary
        err = parse_get_align(4);

        // Read the fixed fields
        if (!err) err = parse_get_bits(&short_len);
        if (!err) err = parse_get_bits(&entry->attributes);
        if (!err) err = parse_get_bits(&entry->size);
        if (!err) err = parse_get_bits(&entry->modified.low);
        if (!err) err = parse_get_bits(&entry->modified.high);
        if (!err) err = parse_get_bits(&entry->uid.uid1);
        if (!err) err = parse_get_bits(&entry->uid.uid2);
        if (!err) err = parse_get_bits(&entry->uid.uid3);
        if (!err) err = parse_get_bits(&long_len);
        if (!err && (long_len & RFSV32_LEN_MASK_BITS)) err = &err_rfsv_str;
        if (!err && (EPOC32_MAX_LEAF_NAME < long_len)) err = &err_rfsv_len;
        if (!err) err = parse_get_string_len(long_len, entry->name);

        // Read the short version of the filename if present
        if (short_len)
        {
            // Read the short filename
            if (!err && (EPOC32_MAX_SHORT_NAME < short_len)) err = &err_rfsv_len;
            if (!err) err = parse_get_align(4);
            if (!err) err = parse_get_string_len(short_len, entry->short_name);
        }
        else
        {
            // No short filename so try to use the long filename
            if (!err && (EPOC32_MAX_SHORT_NAME < long_len)) err = &err_rfsv_len;
            if (!err) strcpy(entry->short_name, entry->name);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : volume        - Variable to receive the volume info.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next volume info value from the current buffer.
*/
static os_error *rfsv32_get_volume_info(epoc32_volume_info *volume)
{
    os_error *err = NULL;

    // Check parameters
    if (!volume) err = &err_bad_parms;
    else
    {
        bits len;

        // Read all of the fields
        err = parse_get_bits(&volume->type);
        if (!err) err = parse_get_bits(&volume->battery);
        if (!err) err = parse_get_bits(&volume->drive);
        if (!err) err = parse_get_bits(&volume->media);
        if (!err) err = parse_get_bits(&volume->uid);
        if (!err) err = parse_get_bits(&volume->size.low);
        if (!err) err = parse_get_bits(&volume->size.high);
        if (!err) err = parse_get_bits(&volume->free.low);
        if (!err) err = parse_get_bits(&volume->free.high);
        if (!err) err = parse_get_bits(&len);
        if (!err && (len & RFSV32_LEN_MASK_BITS)) err = &err_rfsv_str;
        if (!err && (EPOC32_MAX_DISC_NAME < len)) err = &err_rfsv_len;
        if (!err) err = parse_get_string_len(len, volume->name);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Function to end an operation.
*/
static os_error *rfsv32_receive(const void *cmd, void *reply,
                                const byte *data, bits size)
{
    os_error *err = NULL;
    rfsv32_cmd *in = (rfsv32_cmd *) cmd;
    rfsv32_reply *out = (rfsv32_reply *) reply;

    // Check parameters
    if (!cmd || !reply || !data) err = &err_bad_parms;
    else
    {
        bits offset = 0;
        unsigned short value;
        bits status;

        // Start parsing the data
        err = parse_get_start(data, size, &offset);
        if (!err) err = parse_get_word(&value);
        if (!err && (value != RFSV32_RESPONSE)) err = &err_not_rfsv_reply;
        if (!err) err = parse_get_word(&value);
        if (!err && (value != rfsv32_id)) err = &err_bad_rfsv_reply;
        if (!err) err = parse_get_bits(&status);
        if (!err)
        {
            err = status_era_err(status);
            if (err) { DEBUG_PRINTF(("RFSV32 status=%i, error='%s'", status, err->errmess)) }
        }

        // Decode any operation specific data
        switch (in->op)
        {
            case RFSV32_REQ_OPEN_DIR:
                // Open remote directory
                if (!err) err = parse_get_bits(&out->req_open_dir.handle);
                break;

            case RFSV32_REQ_READ_DIR:
                // Request a directory listing
                {
                    epoc32_remote_entry entry;

                    out->req_read_dir.next = in->data.req_read_dir.buffer;
                    out->req_read_dir.used = 0;
                    out->req_read_dir.remain = in->data.req_read_dir.size;
                    while (!err && !rfsv32_get_remote_entry(&entry))
                    {
                        if (out->req_read_dir.remain)
                        {
                            *out->req_read_dir.next++ = entry;
                            out->req_read_dir.used++;
                            out->req_read_dir.remain--;
                        }
                        else err = &err_rfsv_too_many;
                    }
                }
                break;

            case RFSV32_REQ_GET_DRIVE_LIST:
                // Get a 26 byte list of drive states
                {
                    int i;

                    for (i = 0; !err && (i < EPOC32_DRIVES); i++)
                    {
                        err = parse_get_byte(&out->req_get_drive_list.drives[i]);
                    }
                }
                break;

            case RFSV32_REQ_VOLUME:
                // Get volume name
                if (!err) err = rfsv32_get_volume_info(&out->req_volume.volume);
                break;

            case RFSV32_REQ_OPEN_FILE:
                // Open remote file
                if (err && ERR_EQ(*err, err_bad_parms)) err = &err_open;
                if (!err) err = parse_get_bits(&out->req_open_file.handle);
                break;

            case RFSV32_REQ_TEMP_FILE:
                // Open temporary remote file
                if (!err) err = parse_get_bits(&out->req_temp_file.handle);
                if (!err) err = rfsv32_get_des(out->req_temp_file.name, sizeof(out->req_temp_file.name));
                break;

            case RFSV32_REQ_READ_FILE:
                // Read from a remote file
                {
                    byte *ptr = in->data.req_read_file.buffer;
                    byte data;

                    out->req_read_file.length = 0;
                    while (!err && !parse_get_byte(&data))
                    {
                        if (out->req_read_file.length
                            < in->data.req_read_file.length)
                        {
                            *ptr++ = data;
                            out->req_read_file.length++;
                        }
                        else err = &err_buffer_full;
                    }
                }
                break;

            case RFSV32_REQ_SEEK_FILE:
                // Seek to offset in remote file
                if (!err) err = parse_get_bits((bits *) &out->req_seek_file.offset);
                break;

            case RFSV32_REQ_REMOTE_ENTRY:
                // Get entry details for a remote file
                if (!err) err = rfsv32_get_remote_entry(&out->req_remote_entry.entry);
                break;

            case RFSV32_REQ_ATT:
                // Get remote file attributes
                if (!err) err = parse_get_bits(&out->req_att.attributes);
                break;

            case RFSV32_REQ_MODIFIED:
                // Get modified time
                if (!err) err = parse_get_bits(&out->req_modified.modified.low);
                if (!err) err = parse_get_bits(&out->req_modified.modified.high);
                break;

            case RFSV32_REQ_SESSION_PATH:
                // Get session path
                if (!err) err = rfsv32_get_des(out->req_session_path.name, sizeof(out->req_session_path.name));
                break;

            case RFSV32_REQ_READ_WRITE_FILE:
                // Read from one file and write to another
                if (!err) err = parse_get_bits(&out->req_read_write_file.length);
                break;

            case RFSV32_REQ_CREATE_FILE:
                // Create file
                if (!err) err = parse_get_bits(&out->req_create_file.handle);
                break;

            case RFSV32_REQ_REPLACE_FILE:
                // Replace file
                if (!err) err = parse_get_bits(&out->req_replace_file.handle);
                break;

            case RFSV32_REQ_OPEN_DIR_UID:
                // Open remote directory with UID filter
                if (!err) err = parse_get_bits(&out->req_open_dir_uid.handle);
                break;

            case RFSV32_REQ_DRIVE_NAME:
                // Request drive name
                if (!err) err = rfsv32_get_des(out->req_drive_name.name, sizeof(out->req_drive_name.name));
                break;

            case RFSV32_REQ_CLOSE_HANDLE:
            case RFSV32_REQ_SET_VOLUME_LABEL:
            case RFSV32_REQ_WRITE_FILE:
            case RFSV32_REQ_DELETE:
            case RFSV32_REQ_FLUSH:
            case RFSV32_REQ_SET_SIZE:
            case RFSV32_REQ_RENAME:
            case RFSV32_REQ_MK_DIR_ALL:
            case RFSV32_REQ_RM_DIR:
            case RFSV32_REQ_SET_ATT:
            case RFSV32_REQ_SET_MODIFIED:
            case RFSV32_REQ_SET_SESSION_PATH:
            case RFSV32_REQ_PATH_TEST:
            case RFSV32_REQ_LOCK:
            case RFSV32_REQ_UNLOCK:
            case RFSV32_REQ_SET_DRIVE_NAME:
            case RFSV32_REQ_REPLACE:
                // No additional information
                break;

            default:
                // Not a supported command
                if (!err) err = &err_bad_rfsv_op;
                break;
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
    Description : Callback function for a link channel register operation.
*/
static os_error *rfsv32_lnkchan_callback(void *user, os_error *err,
                                         const void *reply)
{
    // Check if an error was produced
    if (err)
    {
        // Close this channel
        err = rfsv32_end(FALSE);
    }
    else
    {
        // Retry the connection
        err = mux_chan_connect(rfsv32_channel, reply);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : event         - The event to process.
                  data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Poll routine for an individual channel.
*/
static os_error *rfsv32_poll(mux_events event, const byte *data, bits size)
{
    os_error *err = NULL;

    // Action depends on the event
    switch (event)
    {
        case MUX_EVENT_SERVER_FAILED:
            // Failed to connect to remote server
            err = lnkchan_register(RFSV32_LNKCHAN_NAME, rfsv32_lnkchan_reply,
                                   NULL, rfsv32_lnkchan_callback);
            break;

        case MUX_EVENT_SERVER_CONNECTED:
            // Connected to remote server
            err = share_create(&rfsv32_share_handle,
                               rfsv32_send, rfsv32_receive);
            break;

        case MUX_EVENT_SERVER_DISCONNECTED:
            // Disconnected from remote server
            err = share_destroy(&rfsv32_share_handle);
            break;

        case MUX_EVENT_SERVER_DATA:
            // Data received from remote server
            err = share_poll_data(rfsv32_share_handle, data, size);
            break;

        case MUX_EVENT_IDLE:
            // Multiplexor is idle for this channel
            if (rfsv32_share_handle != SHARE_NONE)
            {
                err = share_poll_idle(rfsv32_share_handle);
            }
            break;

        case MUX_EVENT_START:
        case MUX_EVENT_END:
            // Not interested in other events
            break;

        case MUX_EVENT_CLIENT_CONNECTED:
        case MUX_EVENT_CLIENT_DISCONNECTED:
        case MUX_EVENT_CLIENT_DATA:
        default:
            // Other event types should not occur
            DEBUG_PRINTF(("Unexpected RFSV32 event %u", event))
            err = &err_bad_ncp_event;
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a client channel for the remote file services.
*/
os_error *rfsv32_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting RFSV32 client"))

    // No action if already active
    if (!rfsv32_active)
    {
        // Create the channel
        err = mux_chan_create(RFSV32_CHANNEL_NAME, 0, TRUE, FALSE, rfsv32_poll,
                              RFSV32_MAX_FRAME, &rfsv32_channel);

        // Set the active flag if successful
        if (!err) rfsv32_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the remote file services client channel.
*/
os_error *rfsv32_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending RFSV32 client now=%u", now))

    // No action unless active
    if (rfsv32_active)
    {
        // Destroy the channel
        if (!err) err = mux_chan_destroy(rfsv32_channel, now);

        // Clear the active flag if successful
        if (!err) rfsv32_active = FALSE;
    }

    // Return any error produced
    return err;
}
