/*
    File        : ncp.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Remote command services client for the PsiFS module.

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
#include "ncp.h"

// Include clib header files
#include <string.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "lnkchan.h"
#include "mux.h"
#include "ncpfile.h"
#include "parse.h"

// Maximum message size
#define NCP_MAX_FRAME (2048)

// The channel details
#define NCP_CHANNEL_NAME "SYS$RPCS.*"
static mux_channel ncp_channel;

// Link details for this channel
#define NCP_LNKCHAN_NAME "SYS$RPCS"
static lnkchan_reply ncp_lnkchan_reply;

// Allow a connection retry after copying the server
static bool ncp_retry = FALSE;

// Is the channel active
static bool ncp_active = FALSE;

// Shared access handler
share_handle ncp_share_handle = SHARE_NONE;

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the NCP channel to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *ncp_fore(const ncp_cmd *cmd, ncp_reply *reply, bool escape)
{
    os_error *err = NULL;

    // Check parameters
    if (!cmd || !reply) err = &err_bad_parms;
    else
    {
        // Perform the operation
        err = share_fore(ncp_share_handle, cmd, reply, escape);
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
    Description : Perform the specified operation when the NCP channel becomes
                  idle. Control is returned immediately. If the channel is not
                  valid or the operation fails then no error is returned, but
                  instead the callback function is notified.
*/
os_error *ncp_back(const ncp_cmd *cmd, ncp_reply *reply,
                   void *user, share_callback callback)
{
    os_error *err = NULL;

    // Check parameters
    if (!cmd || !reply || !callback) err = &err_bad_parms;
    else
    {
        // Perform the operation
        err = share_back(ncp_share_handle, cmd, reply, user, callback);
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
static os_error *ncp_send(const void *cmd, void *reply)
{
    os_error *err = NULL;
    static byte buffer[NCP_MAX_FRAME];
    ncp_cmd *in = (ncp_cmd *) cmd;

    // Check parameters
    if (!cmd || !reply) err = &err_bad_parms;
    else
    {
        bits offset = 0;

        // Write the standard header
        err = parse_put_start(buffer, NCP_MAX_FRAME, &offset);
        if (!err) err = parse_put_byte(in->op);

        // Add any command specific data
        switch (in->op)
        {
            case NCP_QUERY_SUPPORT:
                // Get version number
                if (!err) err = parse_put_byte(in->data.query_support.major);
                if (!err) err = parse_put_byte(in->data.query_support.minor);
                break;

            case NCP_EXEC_PROGRAM:
                // Launch process
                if (!err)
                {
                    if (strlen(in->data.exec_program.name) < 128)
                    {
                        err = parse_put_string_len(128, in->data.exec_program.name);
                    }
                    else err = parse_put_string(in->data.exec_program.name);
                }
                if (!err) err = parse_put_byte(strlen(in->data.exec_program.args));
                if (!err) err = parse_put_string(in->data.exec_program.args);
                break;

            case NCP_QUERY_DRIVE:
                // Get open apps
                if (!err) err = parse_put_byte(in->data.query_drive.drive);
                break;

            case NCP_STOP_PROGRAM:
                // Terminate remote process
                if (!err) err = parse_put_string(in->data.stop_program.name);
                break;

            case NCP_PROG_RUNNING:
                // Program running query
                if (!err) err = parse_put_string(in->data.prog_running.name);
                break;

            case NCP_FORMAT_OPEN:
                // Format drive - initiate format
                if (!err) err = parse_put_string(in->data.format_open.name);
                break;

            case NCP_FORMAT_READ:
                // Format drive - continue format
                if (!err) err = parse_put_word(in->data.format_read.handle);
                break;

            case NCP_GET_UNIQUE_ID:
                // Get unique ID
                if (!err) err = parse_put_string(in->data.get_unique_id.name);
                break;

            case NCP_GET_CMD_LINE:
                // Get command line
                if (!err) err = parse_put_string(in->data.get_cmd_line.name);
                break;

            case NCP_STOP_FILE:
                // Get file owner
                if (!err) err = parse_put_string(in->data.stop_file.name);
                break;

            case NCP_CLOSE_HANDLE:
                // Close a previously opened resource handle
                if (!err) err = parse_put_word(in->data.close_handle.handle);
                break;

                /*
            case NCP_REG_OPEN_ITER:
                // Open a registry iterator
                break;

            case NCP_REG_READ_ITER:
                // Read from the registry using an iterator
                break;

            case NCP_REG_WRITE:
                // Write entries to the registry
                break;

            case NCP_REG_READ:
                // Read a single entry from the registry
                break;

            case NCP_REG_DELETE:
                // Delete items from the registry
                break;
                */

            case NCP_SET_TIME:
                // Set the time
                if (!err) err = parse_put_bytes(sizeof(in->data.set_time.time), &in->data.set_time.time);
                break;

                /*
            case NCP_CONFIG_OPEN:
                // Open configuration
                break;

            case NCP_CONFIG_READ:
                // Read configuration
                break;

            case NCP_CONFIG_WRITE:
                // Write configuration
                break;
                */

            case NCP_QUERY_OPEN:
                // Start a document-application name query
                if (!err) err = parse_put_byte(in->data.query_open.drive);
                break;

            case NCP_QUERY_READ:
                // Read document-application name pairs
                if (!err) err = parse_put_word(in->data.query_read.handle);
                break;

            case NCP_GET_OWNER_INFO:
            case NCP_GET_MACHINE_TYPE:
            case NCP_GET_MACHINE_INFO:
            case NCP_QUIT_SERVER:
                // No additional information
                break;

            default:
                // Not a supported command
                if (!err) err = &err_bad_ncp_op;
                break;
        }

        // Send the command
        if (!err) err = mux_chan_tx_server(ncp_channel, buffer, offset);
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
static os_error *ncp_get_string(char *value, bits size)
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
        if (!err && (size <= strlen(str))) err = &err_ncp_len;

        // Copy the string
        if (!err) strcpy(value, str);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - Variable to receive the string.
                  size          - Size of the buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next unterminated string from the current buffer.
*/
static os_error *ncp_get_string_all(char *value, bits size)
{
    os_error *err = NULL;

    // Check parameters
    if (!value || !size) err = &err_bad_parms;
    else
    {
        bool newline = FALSE;

        // Loop until finished or buffer full
        while (size && !parse_get_byte((byte *) value))
        {
            // Convert sequences of nulls to single newlines
            if (*value || !newline)
            {
                newline = !*value;
                if (newline) *value = '\n';
                value++;
                size--;
            }
        }

        // Add a terminator if successful
        if (size) *value = '\0';
        else err = &err_ncp_len;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : app           - Variable to receive the details.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next process/argument pair from the current buffer.
*/
static os_error *ncp_get_app(ncp_app *app)
{
    os_error *err = NULL;

    // Check parameters
    if (!app) err = &err_bad_parms;
    else
    {
        const char *name;
        const char *args;

        // Read the strings
        err = parse_get_string(&name);
        if (!err) err = parse_get_string(&args);

        // Check the string lengths
        if (!err && ((sizeof(app->name) <= strlen(name))
                     || (sizeof(app->args) <= strlen(args))))
        {
            err = &err_ncp_len;
        }

        // Copy the strings
        if (!err)
        {
            strcpy(app->name, name);
            strcpy(app->args, args);
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
static os_error *ncp_receive(const void *cmd, void *reply,
                             const byte *data, bits size)
{
    os_error *err = NULL;
    ncp_cmd *in = (ncp_cmd *) cmd;
    ncp_reply *out = (ncp_reply *) reply;

    // Check parameters
    if (!cmd || !reply || !data) err = &err_bad_parms;
    else
    {
        bits offset = 0;
        byte value;

        // Start parsing the data
        err = parse_get_start(data, size, &offset);
        if (!err) err = parse_get_byte(&value);
        if (!err)
        {
            err = status_sibo_err(value);
            if (err) { DEBUG_PRINTF(("NCP status=%i, error='%s'", (short) value, err->errmess)) }
        }

        // Decode any operation specific data
        switch (in->op)
        {
            case NCP_QUERY_SUPPORT:
                // Get version number
                if (!err) err = parse_get_byte(&value);
                if (!err) out->query_support.major = value;
                if (!err) err = parse_get_byte(&value);
                if (!err) out->query_support.minor = value;
                break;

            case NCP_QUERY_DRIVE:
                // Get open apps
                {
                    ncp_app app;

                    err = NULL;
                    offset = 0;
                    out->query_drive.next = in->data.query_drive.buffer;
                    out->query_drive.used = 0;
                    out->query_drive.remain = in->data.query_drive.size;
                    while (!err && !ncp_get_app(&app))
                    {
                        if (out->query_drive.remain)
                        {
                            *out->query_drive.next++ = app;
                            out->query_drive.used++;
                            out->query_drive.remain--;
                        }
                        else err = &err_ncp_too_many;
                    }
                }
                break;

            case NCP_PROG_RUNNING:
                // Program running query
                if (!err) out->prog_running.running = TRUE;
                else if (ERR_EQ(*err, err_not_found))
                {
                    out->prog_running.running = FALSE;
                    err = NULL;
                }
                break;

            case NCP_FORMAT_OPEN:
                // Format drive - initiate format
                if (!err)
                {
                    unsigned short value;

                    err = parse_get_word(&value);
                    if (!err) out->format_open.handle = value;
                    if (!err) err = parse_get_word(&value);
                    if (!err) out->format_open.count = value;
                }
                break;

            case NCP_GET_UNIQUE_ID:
                // Get unique ID
                if (!err) err = parse_get_bits(&out->get_unique_id.id);
                break;

            case NCP_GET_OWNER_INFO:
                // Get owner info
                if (!err) err = ncp_get_string_all(out->get_owner_info.info, sizeof(out->get_owner_info.info));
                break;

            case NCP_GET_MACHINE_TYPE:
                // Get machine type
                if (!err)
                {
                    unsigned short value;

                    err = parse_get_word(&value);
                    if (!err) out->get_machine_type.type = value;
                }
                break;

            case NCP_GET_CMD_LINE:
                // Get command line
                if (!err) err = ncp_get_string(out->get_cmd_line.name, sizeof(out->get_cmd_line.name));
                if (!err && !parse_get_byte(&value))
                {
                    err = ncp_get_string_all(out->get_cmd_line.args, sizeof(out->get_cmd_line.args));
                    if (err && ERR_EQ(*err, err_ncp_len) && (256 < size))
                    {
                        // Try to decode SIBO format reply
                        offset = 257;
                        err = ncp_get_string(out->get_cmd_line.args, sizeof(out->get_cmd_line.args));
                    }
                }
                else *out->get_cmd_line.args = '\0';
                break;

            case NCP_STOP_FILE:
                // Get file owner
                if (!err) err = ncp_get_string(out->stop_file.name, sizeof(out->stop_file.name));
                break;

            case NCP_GET_MACHINE_INFO:
                // Read machine information
                if (!err) err = parse_get_bytes(sizeof(out->get_machine_info.info), &out->get_machine_info.info);
                break;

                /*
            case NCP_REG_OPEN_ITER:
                // Open a registry iterator
                break;

            case NCP_REG_READ_ITER:
                // Read from the registry using an iterator
                break;

            case NCP_REG_WRITE:
                // Write entries to the registry
                break;

            case NCP_REG_READ:
                // Read a single entry from the registry
                break;

            case NCP_REG_DELETE:
                // Delete items from the registry
                break;

            case NCP_CONFIG_OPEN:
                // Open configuration
                break;

            case NCP_CONFIG_READ:
                // Read configuration
                break;

            case NCP_CONFIG_WRITE:
                // Write configuration
                break;
                */

            case NCP_QUERY_OPEN:
                // Start a document-application name query
                if (!err)
                {
                    unsigned short value;

                    err = parse_get_word(&value);
                    if (!err) out->query_open.handle = value;
                }
                break;

            case NCP_QUERY_READ:
                // Read document-application name pairs
                {
                    ncp_app app;

                    err = NULL;
                    offset = 0;
                    out->query_read.next = in->data.query_read.buffer;
                    out->query_read.used = 0;
                    out->query_read.remain = in->data.query_read.size;
                    while (!err && !ncp_get_app(&app))
                    {
                        if (out->query_read.remain)
                        {
                            *out->query_read.next++ = app;
                            out->query_read.used++;
                            out->query_read.remain--;
                        }
                        else err = &err_ncp_too_many;
                    }
                }
                break;

            case NCP_EXEC_PROGRAM:
            case NCP_STOP_PROGRAM:
            case NCP_FORMAT_READ:
            case NCP_CLOSE_HANDLE:
            case NCP_QUIT_SERVER:
            case NCP_SET_TIME:
                // No additional information
                break;

            default:
                // Not a supported command
                if (!err) err = &err_bad_ncp_op;
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
    Description : Callback function for a server upload operation.
*/
static os_error *ncp_ncpfile_callback(void *user, os_error *err,
                                      const void *reply)
{
    // Check if an error was produced
    if (err)
    {
        // Close this channel if an error
        err = ncp_end(FALSE);
    }
    else
    {
        // Retry the connection
        err = mux_chan_connect(ncp_channel, NULL);
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
static os_error *ncp_lnkchan_callback(void *user, os_error *err,
                                      const void *reply)
{
    // Check if an error was produced
    if (err)
    {
        // Try to upload the server if an error
        if (!ncp_retry)
        {
            // Set the retry flag
            ncp_retry = TRUE;

            // Start an upload
            err = ncpfile_upload(NULL, ncp_ncpfile_callback);
        }

        // Close this channel if an error
        if (err) err = ncp_end(FALSE);
    }
    else
    {
        // Retry the connection
        err = mux_chan_connect(ncp_channel, reply);
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
static os_error *ncp_poll(mux_events event, const byte *data, bits size)
{
    os_error *err = NULL;

    // Action depends on the event
    switch (event)
    {
        case MUX_EVENT_SERVER_FAILED:
            // Failed to connect to remote server
            err = lnkchan_register(NCP_LNKCHAN_NAME, ncp_lnkchan_reply,
                                   NULL, ncp_lnkchan_callback);
            break;

        case MUX_EVENT_SERVER_CONNECTED:
            // Connected to remote server
            err = share_create(&ncp_share_handle,
                               ncp_send, ncp_receive);
            break;

        case MUX_EVENT_SERVER_DISCONNECTED:
            // Disconnected from remote server
            err = share_destroy(&ncp_share_handle);
            break;

        case MUX_EVENT_SERVER_DATA:
            // Data received from remote server
            err = share_poll_data(ncp_share_handle, data, size);
            break;

        case MUX_EVENT_IDLE:
            // Multiplexor is idle for this channel
            if (ncp_share_handle != SHARE_NONE)
            {
                err = share_poll_idle(ncp_share_handle);
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
            DEBUG_PRINTF(("Unexpected NCP event %u", event))
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
os_error *ncp_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting NCP client"))

    // No action if already active
    if (!ncp_active)
    {
        // Initially no retries
        ncp_retry = FALSE;

        // Create the channel
        err = mux_chan_create(NCP_CHANNEL_NAME, 0, TRUE, FALSE, ncp_poll,
                              NCP_MAX_FRAME, &ncp_channel);

        // Set the active flag if successful
        if (!err) ncp_active = TRUE;
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
os_error *ncp_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending NCP client now=%u", now))

    // No action unless active
    if (ncp_active)
    {
        // Destroy the channel
        err = mux_chan_destroy(ncp_channel, now);

        // Clear the active flag if successful
        if (!err) ncp_active = FALSE;
    }

    // Return any error produced
    return err;
}
