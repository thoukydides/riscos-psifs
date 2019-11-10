/*
    File        : rclip.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Remote clipboard handling for the PsiFS module.

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
#include "rclip.h"

// Include clib header files
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// Include project header files
#include "clipboard.h"
#include "clipfile.h"
#include "debug.h"
#include "err.h"
#include "lnkchan.h"
#include "mux.h"
#include "parse.h"

// Maximum message size
#define RCLIP_MAX_FRAME (16)

// The channel details
#define RCLIP_CHANNEL_NAME "ClipBdServer"
static mux_channel rclip_channel;

// Link details for this channel
#define RCLIP_LNKCHAN_NAME "CLIPSVR.RSY"
static lnkchan_reply rclip_lnkchan_reply;

// Allow a connection retry after copying the server
static bool rclip_retry = FALSE;

// Current version of the clipboard server
#define RCLIP_VERSION (0x0100)

// Is the channel active
static bool rclip_active = FALSE;

// Message types
typedef bits rclip_op;
#define RCLIP_INIT ((rclip_op) 0x00)
#define RCLIP_LISTEN ((rclip_op) 0x04)
#define RCLIP_NOTIFY ((rclip_op) 0x08)

// Clipboard command status
static bool rclip_init_pending = FALSE;
static bool rclip_init_done = FALSE;
static bool rclip_listen_pending = FALSE;
static bool rclip_notify_pending = FALSE;

// Notification status
static bool rclip_notify_active = FALSE;
static bool rclip_notify_required = FALSE;

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Notify the remote device that the clipboard is being changed.
*/
os_error *rclip_notify_start(void)
{
    os_error *err = NULL;

    // Ignore any listen replies until notify complete
    rclip_notify_active = TRUE;

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Notify the remote device that the clipboard has changed.
*/
os_error *rclip_notify_end(void)
{
    os_error *err = NULL;

    // Set the notify pending flag and re-enable listen commands
    rclip_notify_required = rclip_notify_active;
    rclip_notify_active = FALSE;

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Process data received from the remote clipboard server.
*/
static os_error *rclip_receive(const byte *data, bits size)
{
    os_error *err = NULL;
    byte reason;

    // Start parsing the data
    err = parse_get_start(data, size, NULL);
    if (!err) err = parse_get_byte(&reason);

    // Process the received packet
    if (reason == RCLIP_INIT)
    {
        // Initialise and listen requests return the same reason code
        if (rclip_init_pending)
        {
            // Server initialised
            DEBUG_PRINTF(("Remote clipboard server initialised"))
            rclip_init_done = TRUE;
            rclip_init_pending = FALSE;
            err = clipboard_start();
        }
        else if (rclip_listen_pending)
        {
            // Clipboard has changed
            DEBUG_PRINTF(("Remote clipboard has changed"))
            rclip_listen_pending = FALSE;
            if (!rclip_notify_active) clipboard_remote_copy();
        }
    }
    else if (reason == RCLIP_NOTIFY)
    {
        if (rclip_notify_pending)
        {
            // Notification accepted
            DEBUG_PRINTF(("Remote clipboard notify acknowledged"))
            rclip_notify_pending = FALSE;
            rclip_notify_required = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Send any pending commands when the multiplexor is idle.
*/
static os_error *rclip_idle(void)
{
    os_error *err = NULL;
    static byte buffer[RCLIP_MAX_FRAME];
    bits offset = 0;

    // Prepare the buffer
    err = parse_put_start(buffer, RCLIP_MAX_FRAME, &offset);

    // Check for pending commands
    if (!rclip_init_done)
    {
        // Initialise the server
        if (!rclip_init_pending)
        {
            DEBUG_PRINTF(("Initialising remote clipboard server"))
            if (!err) err = parse_put_byte(RCLIP_INIT);
            if (!err) err = parse_put_word(RCLIP_VERSION);
            if (!err) rclip_init_pending = TRUE;
        }
    }
    else if (!rclip_notify_pending && rclip_notify_required)
    {
        // Notify a change to the clipboard
        DEBUG_PRINTF(("Notifying remote clipboard"))
        if (!err) err = parse_put_byte(RCLIP_NOTIFY);
        if (!err) err = parse_put_byte(0);
        if (!err) rclip_notify_pending = TRUE;
    }
    else if (!rclip_listen_pending && !rclip_notify_active)
    {
        // Listen for changes to the clipboard
        DEBUG_PRINTF(("Listening to remote clipboard"))
        if (!err) err = parse_put_byte(RCLIP_LISTEN);
        if (!err) rclip_listen_pending = TRUE;
    }

    // Send any pending command
    if (!err && offset) err = mux_chan_tx_server(rclip_channel, buffer, offset);

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
static os_error *rclip_clipfile_callback(void *user, os_error *err,
                                         const void *reply)
{
    // Check if an error was produced
    if (err)
    {
        // Close this channel if an error
        err = rclip_end(FALSE);
    }
    else
    {
        // Retry the connection
        err = mux_chan_connect(rclip_channel, NULL);
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
static os_error *rclip_lnkchan_callback(void *user, os_error *err,
                                        const void *reply)
{
    // Check if an error was produced
    if (err)
    {
        // Try to upload the server if an error
        if (!rclip_retry)
        {
            // Set the retry flag
            rclip_retry = TRUE;

            // Start an upload
            err = clipfile_upload(NULL, rclip_clipfile_callback);
        }

        // Close this channel if an error
        if (err) err = rclip_end(FALSE);
    }
    else
    {
        // Retry the connection
        err = mux_chan_connect(rclip_channel, reply);
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
static os_error *rclip_poll(mux_events event, const byte *data, bits size)
{
    os_error *err = NULL;

    // Action depends on the event
    switch (event)
    {
        case MUX_EVENT_SERVER_FAILED:
            // Failed to connect to remote server
            err = lnkchan_register(RCLIP_LNKCHAN_NAME, rclip_lnkchan_reply,
                                   NULL, rclip_lnkchan_callback);
            break;

        case MUX_EVENT_SERVER_DATA:
            // Data received from remote server
            err = rclip_receive(data, size);
            break;

        case MUX_EVENT_IDLE:
            // Multiplexor is idle for this channel
            err = rclip_idle();
            break;

        case MUX_EVENT_SERVER_CONNECTED:
            // Reset the status on connection
            rclip_init_pending = FALSE;
            rclip_init_done = FALSE;
            rclip_listen_pending = FALSE;
            rclip_notify_pending = FALSE;
            break;

        case MUX_EVENT_SERVER_DISCONNECTED:
            // Attempt to reconnect
            rclip_init_done = FALSE;
            err = mux_chan_connect(rclip_channel, NULL);
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
            DEBUG_PRINTF(("Unexpected ClipBdServer event %u", event))
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
    Description : Create a client channel for remote clipboard services.
*/
os_error *rclip_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting ClipBdServer client"))

    // No action if already active
    if (!rclip_active)
    {
        // Initially no retries
        rclip_retry = FALSE;

        // Create the channel
        err = mux_chan_create(RCLIP_CHANNEL_NAME, 0, TRUE, FALSE, rclip_poll,
                              RCLIP_MAX_FRAME, &rclip_channel);

        // Set the active flag if successful
        if (!err) rclip_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the remote clipboard services client channel.
*/
os_error *rclip_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending ClipBdServer client now=%u", now))

    // No action unless active
    if (rclip_active)
    {
        // End higher levels
        err = clipboard_end(now);

        // Destroy the channel
        if (!err) err = mux_chan_destroy(rclip_channel, now);

        // Clear the active flag if successful
        if (!err)
        {
            rclip_active = FALSE;
            rclip_notify_active = FALSE;
            rclip_notify_required = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the remote clipboard services.
*/
os_error *rclip_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying remote clipboard server status"))

    // Display the current connection status
    if (rclip_init_done)
    {
        err = clipboard_status();
    }
    else if (rclip_init_pending)
    {
        printf("Initialising remote clipboard server.\n");
    }
    else if (rclip_active)
    {
        printf("Not connected to remote clipboard server.\n");
    }
    else printf("Remote clipboard server not active.\n");

    // Return any error produced
    return err;
}
