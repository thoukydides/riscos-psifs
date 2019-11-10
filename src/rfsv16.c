/*
    File        : rfsv16.c
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
#include "rfsv16.h"

// Include clib header files
#include <string.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "lnkchan.h"
#include "mux.h"
#include "parse.h"

// Maximum message size
#define RFSV16_MAX_FRAME (858)

// The channel details
#define RFSV16_CHANNEL_NAME "SYS$RFSV.*"
static mux_channel rfsv16_channel;

// Link details for this channel
#define RFSV16_LNKCHAN_NAME "SYS$RFSV"
static lnkchan_reply rfsv16_lnkchan_reply;

// Is the channel active
static bool rfsv16_active = FALSE;

// Shared access handler
share_handle rfsv16_share_handle = SHARE_NONE;

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for RFSV16 channel to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *rfsv16_fore(const rfsv16_cmd *cmd, rfsv16_reply *reply, bool escape)
{
    os_error *err = NULL;

    // Check parameters
    if (!cmd || !reply) err = &err_bad_parms;
    else
    {
        // Perform the operation
        err = share_fore(rfsv16_share_handle, cmd, reply, escape);
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
os_error *rfsv16_back(const rfsv16_cmd *cmd, rfsv16_reply *reply,
                      void *user, share_callback callback)
{
    os_error *err = NULL;

    // Check parameters
    if (!cmd || !reply || !callback) err = &err_bad_parms;
    else
    {
        // Perform the operation
        err = share_back(rfsv16_share_handle, cmd, reply, user, callback);
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
static os_error *rfsv16_send(const void *cmd, void *reply)
{
    os_error *err = NULL;
    static byte buffer[RFSV16_MAX_FRAME];
    rfsv16_cmd *in = (rfsv16_cmd *) cmd;

    // Check parameters
    if (!cmd || !reply) err = &err_bad_parms;
    else
    {
        bits offset = 0;
        bits size;

        // Write the standard header
        err = parse_put_start(buffer, RFSV16_MAX_FRAME, &offset);
        if (!err) err = parse_put_word(in->op);
        if (!err) err = parse_put_word(0);

        // Add any command specific data
        switch (in->op)
        {
            case RFSV16_RF_FOPEN:
                // Open
                if (!err) err = parse_put_word(in->data.rf_fopen.mode);
                if (!err) err = parse_put_string(in->data.rf_fopen.name);
                break;

            case RFSV16_RF_FCLOSE:
                // Close
                if (!err) err = parse_put_word(in->data.rf_fclose.handle);
                break;

            case RFSV16_RF_FREAD:
                // Read
                if (!err) err = parse_put_word(in->data.rf_fread.handle);
                if (!err) err = parse_put_word(in->data.rf_fread.length);
                break;

            case RFSV16_RF_FDIRREAD:
                // Read directory
                if (!err) err = parse_put_word(in->data.rf_fdirread.handle);
                break;

            case RFSV16_RF_FDEVICEREAD:
                // Read device info
                if (!err) err = parse_put_word(in->data.rf_fdeviceread.handle);
                break;

            case RFSV16_RF_FWRITE:
                // Write
                {
                    bits i;
                    const byte *ptr = (const byte *) in->data.rf_fwrite.buffer;

                    if (!err) err = parse_put_word(in->data.rf_fwrite.handle);
                    for (i = 0; !err && (i < in->data.rf_fwrite.length); i++)
                    {
                        err = parse_put_byte(*ptr++);
                    }
                }
                break;

            case RFSV16_RF_FSEEK:
                // Seek
                if (!err) err = parse_put_word(in->data.rf_fseek.handle);
                if (!err) err = parse_put_bits((bits) in->data.rf_fseek.offset);
                if (!err) err = parse_put_word(in->data.rf_fseek.sense);
                break;

            case RFSV16_RF_FFLUSH:
                // Flush
                if (!err) err = parse_put_word(in->data.rf_fflush.handle);
                break;

            case RFSV16_RF_FSETEOF:
                // Set file length
                if (!err) err = parse_put_word(in->data.rf_fseteof.handle);
                if (!err) err = parse_put_bits(in->data.rf_fseteof.size);
                break;

            case RFSV16_RF_RENAME:
                // Rename file
                if (!err) err = parse_put_string(in->data.rf_rename.src);
                if (!err) err = parse_put_string(in->data.rf_rename.dest);
                break;

            case RFSV16_RF_DELETE:
                // Delete file
                if (!err) err = parse_put_string(in->data.rf_delete.name);
                break;

            case RFSV16_RF_FINFO:
                // File info
                if (!err) err = parse_put_string(in->data.rf_finfo.name);
                break;

            case RFSV16_RF_SFSTAT:
                // Set file attributes
                if (!err) err = parse_put_word(in->data.rf_sfstat.set);
                if (!err) err = parse_put_word(in->data.rf_sfstat.mask);
                if (!err) err = parse_put_string(in->data.rf_sfstat.name);
                break;

            case RFSV16_RF_PARSE:
                // Parse file name
                if (!err) err = parse_put_string(in->data.rf_parse.name);
                break;

            case RFSV16_RF_MKDIR:
                // Create directory
                if (!err) err = parse_put_string(in->data.rf_mkdir.name);
                break;

            case RFSV16_RF_OPENUNIQUE:
                // Open file and read name
                if (!err) err = parse_put_word(in->data.rf_openunique.mode);
                if (!err) err = parse_put_string(in->data.rf_openunique.name);
                break;

            case RFSV16_RF_STATUSDEVICE:
                // Get device status
                if (!err) err = parse_put_string(in->data.rf_statusdevice.name);
                break;

            case RFSV16_RF_PATHTEST:
                // Test path
                if (!err) err = parse_put_string(in->data.rf_pathtest.name);
                break;

            case RFSV16_RF_STATUSSYSTEM:
                // Get system node info
                if (!err) err = parse_put_string(in->data.rf_statussystem.name);
                break;

            case RFSV16_RF_CHANGEDIR:
                // Change current directory
                if (!err) err = parse_put_string(in->data.rf_changedir.name);
                break;

            case RFSV16_RF_SFDATE:
                // Set date
                if (!err) err = parse_put_bits(in->data.rf_sfdate.modified);
                if (!err) err = parse_put_string(in->data.rf_sfdate.name);
                break;

            default:
                // Not a supported command
                if (!err) err = &err_bad_rfsv_op;
                break;
        }

        // Complete the frame header
        if (!err)
        {
            size = offset;
            offset = 2;
            err = parse_put_word(size - 4);
        }

        // Send the command
        if (!err) err = mux_chan_tx_server(rfsv16_channel, buffer, size);
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
static os_error *rfsv16_get_string(char *value, bits size)
{
    os_error *err = NULL;

    // Check parameters
    if (!value || !size) err = &err_bad_parms;
    else
    {
        const char *str;

        // Read the string
        err = parse_get_string(&str);

        // Check the string length
        if (!err && (size <= strlen(str))) err = &err_rfsv_len;

        // Copy the string
        if (!err) strcpy(value, str);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : entry         - Variable to receive the directory entry.
                  name          - Should the filename be read.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next directory entry value from the current buffer.
*/
static os_error *rfsv16_get_p_info(epoc16_p_info *entry, bool name)
{
    os_error *err = NULL;

    // Check parameters
    if (!entry) err = &err_bad_parms;
    else
    {
        unsigned short value;

        // Read the standard fields
        err = parse_get_word(&value);
        if (!err) entry->version = value;
        if (!err) err = parse_get_word(&value);
        if (!err) entry->attributes = value;
        if (!err) err = parse_get_bits(&entry->size);
        if (!err) err = parse_get_bits(&entry->modified);
        if (!err) err = parse_get_bits(&entry->reserved);

        // Read the name also if required
        if (!err)
        {
            if (name) err = rfsv16_get_string(entry->name, sizeof(entry->name));
            else *entry->name = '\0';
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : entry         - Variable to receive the device info.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next device info value from the current buffer.
*/
static os_error *rfsv16_get_p_dinfo(epoc16_p_dinfo *entry)
{
    os_error *err = NULL;

    // Check parameters
    if (!entry) err = &err_bad_parms;
    else
    {
        unsigned short value;
        int i;

        // Read the fields
        err = parse_get_word(&value);
        if (!err) entry->version = value;
        if (!err) err = parse_get_word(&value);
        if (!err) entry->type = value;
        if (!err) err = parse_get_word(&value);
        if (!err) entry->removable = value;
        if (!err) err = parse_get_bits(&entry->size);
        if (!err) err = parse_get_bits(&entry->free);
        if (!err) err = rfsv16_get_string(entry->name, sizeof(entry->name));
        if (!err) err = parse_get_word(&value);
        if (!err) entry->battery = value;
        for (i = 0; !err && (i < sizeof(entry->reserved)); i++)
        {
            err = parse_get_byte(&entry->reserved[i]);
        }
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
static os_error *rfsv16_receive(const void *cmd, void *reply,
                                const byte *data, bits size)
{
    os_error *err = NULL;
    rfsv16_cmd *in = (rfsv16_cmd *) cmd;
    rfsv16_reply *out = (rfsv16_reply *) reply;

    // Check parameters
    if (!cmd || !reply || !data) err = &err_bad_parms;
    else
    {
        bits offset = 0;
        unsigned short value;

        // Start parsing the data
        err = parse_get_start(data, size, &offset);
        if (!err) err = parse_get_word(&value);
        if (!err && (value != RFSV16_RESPONSE)) err = &err_not_rfsv_reply;
        if (!err) err = parse_get_word(&value);
        if (!err && (value != (size - 4))) err = &err_bad_rfsv_reply;
        if (!err) err = parse_get_word(&value);
        if (!err)
        {
            err = status_sibo_err(value);
            if (err) { DEBUG_PRINTF(("RFSV16 status=%i, error='%s'", (short) value, err->errmess)) }
        }

        // Decode any operation specific data
        switch (in->op)
        {
            case RFSV16_RF_FOPEN:
                // Open
                if (!err) err = parse_get_word(&value);
                if (!err) out->rf_fopen.handle = value;
                break;

            case RFSV16_RF_FREAD:
                // Read
                {
                    byte *ptr = in->data.rf_fread.buffer;
                    byte data;

                    out->rf_fread.length = 0;
                    while (!err && !parse_get_byte(&data))
                    {
                        if (out->rf_fread.length
                            < in->data.rf_fread.length)
                        {
                            *ptr++ = data;
                            out->rf_fread.length++;
                        }
                        else err = &err_buffer_full;
                    }
                }
                break;

            case RFSV16_RF_FDIRREAD:
                // Read directory
                {
                    epoc16_p_info entry;

                    out->rf_fdirread.next = in->data.rf_fdirread.buffer;
                    out->rf_fdirread.used = 0;
                    out->rf_fdirread.remain = in->data.rf_fdirread.size;
                    if (!err) err = parse_get_word(&value);
                    while (!err && !rfsv16_get_p_info(&entry, TRUE))
                    {
                        if (out->rf_fdirread.remain)
                        {
                            *out->rf_fdirread.next++ = entry;
                            out->rf_fdirread.used++;
                            out->rf_fdirread.remain--;
                        }
                        else err = &err_rfsv_too_many;
                    }
                }
                break;

            case RFSV16_RF_FDEVICEREAD:
                // Read device info
                if (!err) err = rfsv16_get_p_dinfo(&out->rf_fdeviceread.dinfo);
                break;

            case RFSV16_RF_FSEEK:
                // Seek
                if (!err) err = parse_get_bits(&out->rf_fseek.offset);
                break;

            case RFSV16_RF_FINFO:
                // File info
                if (!err) err = rfsv16_get_p_info(&out->rf_finfo.entry, FALSE);
                break;

            case RFSV16_RF_PARSE:
                // Parse file name
                if (!err) err = rfsv16_get_string(out->rf_parse.name, sizeof(out->rf_parse.name));
                break;

            case RFSV16_RF_OPENUNIQUE:
                // Open file and read name
                if (!err) err = parse_get_word(&value);
                if (!err) out->rf_openunique.handle = value;
                if (!err) err = rfsv16_get_string(out->rf_openunique.name, sizeof(out->rf_openunique.name));
                break;

            case RFSV16_RF_STATUSDEVICE:
                // Get device status
                if (!err) err = rfsv16_get_p_dinfo(&out->rf_statusdevice.dinfo);
                break;

            case RFSV16_RF_STATUSSYSTEM:
                // Get system node info
                if (!err) err = parse_get_word(&value);
                if (!err) out->rf_statussystem.version = value;
                if (!err) err = parse_get_word(&value);
                if (!err) out->rf_statussystem.type = value;
                if (!err) err = parse_get_word(&value);
                if (!err) out->rf_statussystem.formattable = value;
                break;

            case RFSV16_RF_FCLOSE:
            case RFSV16_RF_FWRITE:
            case RFSV16_RF_FFLUSH:
            case RFSV16_RF_FSETEOF:
            case RFSV16_RF_RENAME:
            case RFSV16_RF_DELETE:
            case RFSV16_RF_SFSTAT:
            case RFSV16_RF_MKDIR:
            case RFSV16_RF_PATHTEST:
            case RFSV16_RF_CHANGEDIR:
            case RFSV16_RF_SFDATE:
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
static os_error *rfsv16_lnkchan_callback(void *user, os_error *err,
                                         const void *reply)
{
    // Check if an error was produced
    if (err)
    {
        // Close this channel
        err = rfsv16_end(FALSE);
    }
    else
    {
        // Retry the connection
        err = mux_chan_connect(rfsv16_channel, reply);
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
static os_error *rfsv16_poll(mux_events event, const byte *data, bits size)
{
    os_error *err = NULL;

    // Action depends on the event
    switch (event)
    {
        case MUX_EVENT_SERVER_FAILED:
            // Failed to connect to remote server
            err = lnkchan_register(RFSV16_LNKCHAN_NAME, rfsv16_lnkchan_reply,
                                   NULL, rfsv16_lnkchan_callback);
            break;

        case MUX_EVENT_SERVER_CONNECTED:
            // Connected to remote server
            err = share_create(&rfsv16_share_handle,
                               rfsv16_send, rfsv16_receive);
            break;

        case MUX_EVENT_SERVER_DISCONNECTED:
            // Disconnected from remote server
            err = share_destroy(&rfsv16_share_handle);
            break;

        case MUX_EVENT_SERVER_DATA:
            // Data received from remote server
            err = share_poll_data(rfsv16_share_handle, data, size);
            break;

        case MUX_EVENT_IDLE:
            // Multiplexor is idle for this channel
            if (rfsv16_share_handle != SHARE_NONE)
            {
                err = share_poll_idle(rfsv16_share_handle);
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
            DEBUG_PRINTF(("Unexpected RFSV16 event %u", event))
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
os_error *rfsv16_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting RFSV16 client"))

    // No action if already active
    if (!rfsv16_active)
    {
        // Create the channel
        err = mux_chan_create(RFSV16_CHANNEL_NAME, 0, TRUE, FALSE, rfsv16_poll,
                              RFSV16_MAX_FRAME, &rfsv16_channel);

        // Set the active flag if successful
        if (!err) rfsv16_active = TRUE;
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
os_error *rfsv16_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending RFSV16 client now=%u", now))

    // No action unless active
    if (rfsv16_active)
    {
        // Destroy the channel
        if (!err) err = mux_chan_destroy(rfsv16_channel, now);

        // Clear the active flag if successful
        if (!err) rfsv16_active = FALSE;
    }

    // Return any error produced
    return err;
}
