/*
    File        : lnkchan.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Link client and server for the PsiFS module.

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
#include "lnkchan.h"

// Include clib header files
#include <ctype.h>
#include <string.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "mux.h"
#include "parse.h"
#include "status.h"

// Maximum message size
#define LNKCHAN_MAX_FRAME (300)

// The channel details
#define LNKCHAN_CHANNEL_NAME "LINK.*"
#define LNKCHAN_CHANNEL_MIN (4)
#define LNKCHAN_CHANNEL_SUFFIX ".*"
static mux_channel lnkchan_channel;
#define LNKCHAN_CHANNEL (1)

// Message types
typedef bits lnkchan_op;
#define LNKCHAN_LOAD_PROCESS ((lnkchan_op) 0x00)
#define LNKCHAN_RESPONSE ((lnkchan_op) 0x01)

// Is the channel active
static bool lnkchan_active = FALSE;

// Shared access handler
share_handle lnkchan_share_handle = SHARE_NONE;

// Current operation ID
#define LNKCHAN_MAX_ID (0xffff)
static bits lnkchan_id = 0;

/*
    Parameters  : cmd           - Name of the server to register.
                  reply         - Variable to receive the server name to
                                  connect to.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Trigger a background registration for the specified server.
*/
os_error *lnkchan_register(const lnkchan_cmd cmd, lnkchan_reply reply,
                           void *user, share_callback callback)
{
    os_error *err = NULL;

    // Check parameters
    if (!cmd || !reply || !callback) err = &err_bad_parms;
    else
    {
        // Queue the operation
        err = share_back(lnkchan_share_handle, cmd, reply, user, callback);
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
static os_error *lnkchan_send(const void *cmd, void *reply)
{
    os_error *err = NULL;
    byte buffer[18];

    // Check parameters
    if (!cmd || !reply) err = &err_bad_parms;
    else
    {
        bits offset = 0;

        // Choose a new operation ID
        if (lnkchan_id < LNKCHAN_MAX_ID) lnkchan_id++;
        else lnkchan_id = 0;

        // Clear the buffer first
        memset(buffer, 0, sizeof(buffer));

        // Prepare the command
        err = parse_put_start(buffer, sizeof(buffer), &offset);
        if (!err) err = parse_put_byte(LNKCHAN_LOAD_PROCESS);
        if (!err) err = parse_put_word(lnkchan_id);
        if (!err) err = parse_put_string(cmd);

        // Send the command
        if (!err) err = mux_chan_tx_server(lnkchan_channel, buffer, sizeof(buffer));
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : server        - The name of the server to check.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Validate the specified server name.
*/
static os_error *lnkchan_validate(const char *server)
{
    os_error *err = NULL;

    // Check parameters
    if (!server) err = &err_bad_parms;
    else
    {
        const char *ptr;

        // First check that all characters are printable
        ptr = server;
        while (*ptr) if (!isprint(*ptr++)) err = &err_bad_name;

        // Check that the prefix is sensible
        if (!err && (strlen(server) < LNKCHAN_CHANNEL_MIN)) err = &err_bad_name;
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
static os_error *lnkchan_receive(const void *cmd, void *reply,
                                 const byte *data, bits size)
{
    os_error *err = NULL;
    char *ptr = (char *) reply;

    // Check parameters
    if (!cmd || !reply || !data) err = &err_bad_parms;
    else
    {
        bits offset = 0;
        byte response;
        unsigned short value;

        // Parse the reply
        err = parse_get_start(data, size, &offset);
        if (!err) err = parse_get_byte(&response);
        if (!err && (response != LNKCHAN_RESPONSE)) err = &err_not_link_reply;
        if (!err) err = parse_get_word(&value);
        if (!err && (value != lnkchan_id)) err = &err_bad_link_reply;
        if (!err) err = parse_get_word(&value);
        if (!err)
        {
            err = status_sibo_err(value);
            if (err) { DEBUG_PRINTF(("LINK status=%i, error='%s'", (short) value, err->errmess)) }
        }
        if (!err) err = parse_get_word(&value);
        if (!err)
        {
            const char *str;

            if (!parse_get_string(&str) && !lnkchan_validate(str)
                && (strlen(str) < sizeof(lnkchan_reply)))
            {
                // The returned server name is valid
                strcpy(ptr, str);
            }
            else if (strlen((char *) cmd) + strlen(LNKCHAN_CHANNEL_SUFFIX)
                     <= sizeof(lnkchan_reply))
            {
                // No valid name returned so just add the standard extension
                strcpy(ptr, (char *) cmd);
                strcat(ptr, LNKCHAN_CHANNEL_SUFFIX);
            }
            else err = &err_link_len;
        }
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
static os_error *lnkchan_poll(mux_events event, const byte *data, bits size)
{
    os_error *err = NULL;

    // Action depends on the event
    switch (event)
    {
        case MUX_EVENT_SERVER_CONNECTED:
            // Connected to remote server
            err = share_create(&lnkchan_share_handle,
                               lnkchan_send, lnkchan_receive);
            break;

        case MUX_EVENT_SERVER_DISCONNECTED:
            // Disconnected from remote server
            err = share_destroy(&lnkchan_share_handle);
            break;

        case MUX_EVENT_SERVER_DATA:
            // Data received from remote server
            err = share_poll_data(lnkchan_share_handle, data, size);
            break;

        case MUX_EVENT_IDLE:
            // Multiplexor is idle for this channel
            if (lnkchan_share_handle != SHARE_NONE)
            {
                err = share_poll_idle(lnkchan_share_handle);
            }
            break;

        case MUX_EVENT_START:
        case MUX_EVENT_END:
        case MUX_EVENT_CLIENT_CONNECTED:
        case MUX_EVENT_CLIENT_DISCONNECTED:
            // Not interested in other events
            break;

        case MUX_EVENT_SERVER_FAILED:
        case MUX_EVENT_CLIENT_DATA:
        default:
            // Other event types should not occur
            DEBUG_PRINTF(("Unexpected LINK event %u", event))
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
    Description : Create both a client and server link channel.
*/
os_error *lnkchan_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting LINK client and server"))

    // No action if already active
    if (!lnkchan_active)
    {
        err = mux_chan_create(LNKCHAN_CHANNEL_NAME, LNKCHAN_CHANNEL,
                              TRUE, TRUE, lnkchan_poll, LNKCHAN_MAX_FRAME,
                              &lnkchan_channel);

        // Set the active flag if successful
        if (!err) lnkchan_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the client and server link channel.
*/
os_error *lnkchan_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending LINK client and server now=%u", now))

    // No action unless active
    if (lnkchan_active)
    {
        // Destroy the channel
        if (!err) err = mux_chan_destroy(lnkchan_channel, now);

        // Clear the active flag if successful
        if (!err) lnkchan_active = FALSE;
    }

    // Return any error produced
    return err;
}
