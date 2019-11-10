/*
    File        : wprt.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Remote printing handling for the PsiFS module.

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
#include "wprt.h"

// Include clib header files
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "lnkchan.h"
#include "mux.h"
#include "parse.h"
#include "status.h"
#include "printing.h"

// Maximum message size
#define WPRT_MAX_FRAME (2048)

// The channel details
#define WPRT_CHANNEL_NAME "SYS$WPRT.*"
static mux_channel wprt_channel;

// Link details for this channel
#define WPRT_LNKCHAN_NAME "SYS$WPRT"
static lnkchan_reply wprt_lnkchan_reply;

// Current version of the printing server
#define WPRT_VERSION_MAJOR (0x02)
#define WPRT_VERSION_MINOR (0x00)

// Is the channel active
static bool wprt_active = FALSE;

// Message types
typedef bits wprt_op;
#define WPRT_LEVEL ((wprt_op) 0x00)
#define WPRT_DATA ((wprt_op) 0xf0)
#define WPRT_CANCEL ((wprt_op) 0xf1)
#define WPRT_STOP ((wprt_op) 0xff)

// Reply values
#define WPRT_CONT (0x2a)
#define WPRT_LAST (0xff)
static const byte wprt_reset[] =
{
    0x2a, 0x2a, 0x09, 0x00, 0x00, 0x00, 0x82, 0x2e,
    0x00, 0x00, 0xc6, 0x41, 0x00, 0x00, 0x00
};
#define WPRT_RESET_SIZE (sizeof(wprt_reset))

// Printing command status
static bool wprt_init_pending = FALSE;
static bool wprt_init_done = FALSE;
static bool wprt_data_pending = FALSE;
static bool wprt_data_enabled = FALSE;

// Cancel and stop requests
static bool wprt_cancel_required = FALSE;
static bool wprt_stop_required = FALSE;

// Print job status
static bool wprt_busy = FALSE;
static bool wprt_suppress = FALSE;
static bits wprt_page_remain = 0;
static bool wprt_page_last = FALSE;

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Cancel the current print job.
*/
os_error *wprt_cancel(void)
{
    os_error *err = NULL;

    // No action unless there is an active print job
    if (wprt_busy && !wprt_suppress)
    {
        // Cancel the current print job
        wprt_cancel_required = TRUE;
        wprt_data_enabled = TRUE;
        wprt_suppress = TRUE;

        // Notify that the job has been cancelled
        err = printing_cancelled();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Continue receiving data for the current print job.
*/
os_error *wprt_continue(void)
{
    os_error *err = NULL;

    // Enable a data request
    wprt_data_enabled = TRUE;

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Process data received from the remote printing server.
*/
static os_error *wprt_receive(const byte *data, bits size)
{
    os_error *err = NULL;
    bits offset = 0;

    // Start parsing the data
    err = parse_get_start(data, size, &offset);

    // Process the received packet
    if (wprt_init_pending)
    {
        byte status;
        byte major;
        byte minor;

        // Level packet received
        DEBUG_PRINTF(("Remote printing server level received"))
        wprt_init_pending = FALSE;
        wprt_init_done = TRUE;

        // Decode the receive packet
        if (!err) err = parse_get_byte(&status);
        if (!err) err = status_era_err(status);
        if (!err) err = parse_get_byte(&major);
        if (!err) err = parse_get_byte(&minor);

        // Stop the connection if an error or incompatible major version
        if (err || (major != WPRT_VERSION_MAJOR))
        {
            err = NULL;
            wprt_stop_required = TRUE;
        }
        else err = printing_start();
    }
    else if (wprt_data_pending)
    {
        byte value;
        bool last_packet;
        bits length;

        // Data packet received
        DEBUG_PRINTF(("Remote printing server %i bytes of data received", size))
        wprt_data_pending = FALSE;

        // Start by checking for unexpected start of new print job
        if (!err && (size == WPRT_RESET_SIZE) && wprt_busy)
        {
            bits i = 0;
            while ((i < WPRT_RESET_SIZE) && (data[i] == wprt_reset[i])) i++;
            if (i == WPRT_RESET_SIZE)
            {
                // Pretend that the job was cancelled remotely
                wprt_busy = FALSE;
                wprt_page_remain = 0;
                if (!wprt_suppress) err = printing_cancelled();
            }
        }

        // First byte indicates whether this is the last in the print job
        if (!err) err = parse_get_byte(&value);
        if (!err) last_packet = value == WPRT_LAST;

        // Extra fields in first packet of a page
        if (!wprt_page_remain)
        {
            if (!err) err = parse_get_byte(&value);
            if (!err) wprt_page_last = value == WPRT_LAST;
            if (!err) err = parse_get_bits(&wprt_page_remain);
        }

        // Check the packet length
        length = size - offset;
        if (!err)
        {
            if (wprt_page_remain < length) err = &err_buffer_end;
            else wprt_page_remain -= length;
        }
        if (!err && last_packet && wprt_page_remain) err = &err_buffer_end;

        // Pass the data to higher layers
        if (!err)
        {
            // Check for the start of a new job
            if (!wprt_busy)
            {
                wprt_busy = TRUE;
                wprt_suppress = FALSE;
            }

            // Pass the data to higher layers and suspend data requests
            if (!wprt_suppress)
            {
                wprt_data_enabled = FALSE;
                err = printing_data(data + offset, length,
                                    wprt_page_remain == 0, wprt_page_last);
            }
        }
        else
        {
            // Notify higher layers of the problem
            if (wprt_busy) err = printing_cancelled();
        }

        // Reset ready for the next job if last packet received
        if (err || last_packet)
        {
            err = NULL;
            wprt_busy = FALSE;
            wprt_page_remain = 0;
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
static os_error *wprt_idle(void)
{
    os_error *err = NULL;
    static byte buffer[WPRT_MAX_FRAME];
    bits offset = 0;

    // Prepare the buffer
    err = parse_put_start(buffer, WPRT_MAX_FRAME, &offset);

    // Check for pending commands
    if (wprt_stop_required)
    {
        // Stop the server
        DEBUG_PRINTF(("Requesting remote printing server termination"))
        if (!err) err = parse_put_byte(WPRT_STOP);
        if (!err) wprt_stop_required = FALSE;
    }
    else if (!wprt_init_done)
    {
        // Initialise the server
        if (!wprt_init_pending)
        {
            DEBUG_PRINTF(("Initialising remote printing server"))
            if (!err) err = parse_put_byte(WPRT_LEVEL);
            if (!err) err = parse_put_byte(WPRT_VERSION_MAJOR);
            if (!err) err = parse_put_byte(WPRT_VERSION_MINOR);
            if (!err) wprt_init_pending = TRUE;
        }
    }
    else if (!wprt_data_pending && wprt_data_enabled)
    {
        // Requesting data from remote printing server
        if (wprt_cancel_required)
        {
            DEBUG_PRINTF(("Cancelling remote printing server client"))
            if (!err) err = parse_put_byte(WPRT_CANCEL);
            if (!err) wprt_cancel_required = FALSE;
        }
        else
        {
            DEBUG_PRINTF(("Requesting data from remote printing server"))
            if (!err) err = parse_put_byte(WPRT_DATA);
        }
        if (!err) wprt_data_pending = TRUE;
    }

    // Send any pending command
    if (!err && offset) err = mux_chan_tx_server(wprt_channel, buffer, offset);

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
static os_error *wprt_lnkchan_callback(void *user, os_error *err,
                                       const void *reply)
{
    // Check if an error was produced
    if (err)
    {
        // Close this channel
        err = wprt_end(FALSE);
    }
    else
    {
        // Retry the connection
        err = mux_chan_connect(wprt_channel, reply);
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
static os_error *wprt_poll(mux_events event, const byte *data, bits size)
{
    os_error *err = NULL;

    // Action depends on the event
    switch (event)
    {
        case MUX_EVENT_SERVER_FAILED:
            // Failed to connect to remote server
            err = lnkchan_register(WPRT_LNKCHAN_NAME, wprt_lnkchan_reply,
                                   NULL, wprt_lnkchan_callback);
            break;

        case MUX_EVENT_SERVER_DATA:
            // Data received from remote server
            err = wprt_receive(data, size);
            break;

        case MUX_EVENT_IDLE:
            // Multiplexor is idle for this channel
            err = wprt_idle();
            break;

        case MUX_EVENT_SERVER_CONNECTED:
            // Reset the status on connection
            wprt_init_pending = FALSE;
            wprt_init_done = FALSE;
            wprt_data_pending = FALSE;
            wprt_data_enabled = TRUE;
            wprt_cancel_required = FALSE;
            wprt_stop_required = FALSE;
            wprt_busy = FALSE;
            wprt_page_remain = 0;
            break;

        case MUX_EVENT_SERVER_DISCONNECTED:
            // Disconnected from remote server
            wprt_init_done = FALSE;
            if (wprt_busy && !wprt_suppress) err = printing_cancelled();
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
            DEBUG_PRINTF(("Unexpected WPRT event %u", event))
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
os_error *wprt_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting WPRT client"))

    // No action if already active
    if (!wprt_active)
    {
        // Create the channel
        err = mux_chan_create(WPRT_CHANNEL_NAME, 0, TRUE, FALSE, wprt_poll,
                              WPRT_MAX_FRAME, &wprt_channel);

        // Set the active flag if successful
        if (!err) wprt_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the remote printing services client channel.
*/
os_error *wprt_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending WPRT client now=%u", now))

    // No action unless active
    if (wprt_active)
    {
        // End higher levels
        err = printing_end(now);

        // Destroy the channel
        if (!err) err = mux_chan_destroy(wprt_channel, now);

        // Clear the active flag if successful
        if (!err) wprt_active = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the remote printing services.
*/
os_error *wprt_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying remote printing server status"))

    // Display the current connection status
    if (wprt_init_done)
    {
        err = printing_status();
    }
    else if (wprt_init_pending)
    {
        printf("Initialising remote printing server.\n");
    }
    else if (wprt_active)
    {
        printf("Not connected to remote printing server.\n");
    }
    else printf("Remote printing server not active.\n");

    // Return any error produced
    return err;
}
