/*
    File        : mux.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : NCP multiplexor and fragmentation for the link layer of the
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
#include "mux.h"

// Include clib header files
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "oslib/macros.h"

// Include project header files
#include "connect.h"
#include "debug.h"
#include "err.h"
#include "escape.h"
#include "link.h"
#include "lnkchan.h"
#include "mem.h"
#include "ncp.h"
#include "parse.h"
#include "rfsv16.h"
#include "rfsv32.h"
#include "status.h"
#include "unified.h"
#include "util.h"

// Uncomment the next line to force SIBO mode
//#define MUX_SIBO

// Channel numbers
#define MUX_CHANNEL_CTRL (0)
#define MUX_CHANNEL_MAX (256)

// Offsets into frames
#define MUX_OFFSET_DEST (0)
#define MUX_OFFSET_SRC (1)
#define MUX_OFFSET_TYPE (2)
#define MUX_OFFSET_DATA (3)

// Command frames
#define MUX_MSG_DATA_XOFF (0x01)
#define MUX_MSG_DATA_XON (0x02)
#define MUX_MSG_CONNECT_TO_SERVER (0x03)
#define MUX_MSG_CONNECT_RESPONSE (0x04)
#define MUX_MSG_CHANNEL_CLOSED (0x05)
#define MUX_MSG_NCP_INFO (0x06)
#define MUX_MSG_CHANNEL_DISCONNECT (0x07)
#define MUX_MSG_NCP_END (0x08)

// Data frames
#define MUX_MSG_WRITECOMPLETE (0x01)
#define MUX_MSG_WRITEPARTIAL (0x02)

// NCP version numbers
#define MUX_NCP_NO_VERSION (0)
#define MUX_NCP_SIBO_VERSION (2)
#define MUX_NCP_SIBO_NEW_VERSION (3)
#define MUX_NCP_ERA_VERSION (6)
static byte mux_ncp_remote_version;
static bits mux_ncp_remote_id;

// A data frame buffer
typedef struct
{
    byte *data;
    bits size;
    bits used;
    bits offset;
} mux_data_frame;

// The local channels
typedef struct mux_channel_status
{
    const char *name;
    byte chan;
    bool client;
    bool server;
    mux_channel_poll poll;
    byte client_chan;
    byte server_chan;
    mux_data_frame client_rx;
    mux_data_frame server_rx;
    mux_data_frame client_tx;
    mux_data_frame server_tx;
    mux_channel prev;
    mux_channel next;
} mux_channel_status;
static mux_channel mux_channel_list = NULL;
static mux_channel mux_channel_last = NULL;

// The remote channels
static bool mux_blocked[MUX_CHANNEL_MAX];

// Pending control channel transmissions
#define MUX_MAX_CTRL (100)
static frame_data mux_ctrl[MUX_MAX_CTRL];
static bits mux_ctrl_read;
static bits mux_ctrl_write;

// The connection status
static bool mux_active = FALSE;
static bool mux_era = FALSE;

/*
    Parameters  : src           - The source channel number.
                  type          - The type of the frame.
                  frame         - Pointer to the frame data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Prepare the next control channel queue entry.
*/
os_error *mux_ctrl_queue(byte src, byte type, frame_data **frame)
{
    os_error *err = NULL;
    bits next = (mux_ctrl_write + 1) % MUX_MAX_CTRL;

    // Check parameters
    if (!frame) err = &err_bad_parms;
    else if (next == mux_ctrl_read) err = &err_mux_full;
    else
    {
        // Obtain a pointer to the frame
        *frame = &mux_ctrl[mux_ctrl_write];

        // Prepare the frame
        (*frame)->data[MUX_OFFSET_DEST] = MUX_CHANNEL_CTRL;
        (*frame)->data[MUX_OFFSET_SRC] = src;
        (*frame)->data[MUX_OFFSET_TYPE] = type;
        (*frame)->size = MUX_OFFSET_DATA;

        // Add this frame to the queue
        mux_ctrl_write = next;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : chan          - The local channel number to check.
    Returns     : mux_channel   - Pointer to the channel details, or NULL if
                                  not found.
    Description : Attempt to find the details for the specified local channel
                  number.
*/
static mux_channel mux_chan_find(byte chan)
{
    mux_channel ptr = mux_channel_list;

    // Check all of the known channels
    while (ptr && (ptr->chan != chan)) ptr = ptr->next;

    // Return the result
    return ptr;
}

/*
    Parameters  : name          - The name of the server. This is not copied,
                                  so a constant should be specified.
                  chan          - The channel number to use, or 0 to
                                  automatically allocate.
                  client        - Is this a client.
                  server        - Is this a server.
                  poll          - Poll function to call for this function.
                  size          - The maximum frame size. Frames larger than
                                  this value will be discarded.
                  handle        - Variable to receive the handle for this
                                  channel.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create the specified channel. If a client connection is
                  requested then this also attempts to contact a remote server.
*/
os_error *mux_chan_create(const char *name, byte chan,
                          bool client, bool server,
                          mux_channel_poll poll, bits size,
                          mux_channel *handle)
{
    os_error *err = NULL;

    // Check parameters
    if (!name || !handle) err = &err_bad_parms;
    else if (chan && mux_chan_find(chan)) err = &err_chan_exists;

    // Create a status record for this channel
    if (!err)
    {
        *handle = (mux_channel) MEM_MALLOC(sizeof(mux_channel_status));
        if (!*handle) err = &err_buffer;
    }

    // Prepare this channel
    if (!err)
    {
        // Choose a unique channel number
        while ((chan == MUX_CHANNEL_CTRL) || mux_chan_find(chan)) chan++;

        // Copy the channel details
        (*handle)->name = name;
        (*handle)->chan = chan;
        (*handle)->client = client;
        (*handle)->server = server;
        (*handle)->poll = poll;

        // Prepare other details
        (*handle)->client_chan = MUX_CHANNEL_CTRL;
        (*handle)->server_chan = MUX_CHANNEL_CTRL;

        // Allocate buffers
        (*handle)->client_rx.data = server && size
                                    ? (byte *) MEM_MALLOC(size)
                                    : NULL;
        (*handle)->client_rx.size = server ? size : 0;
        (*handle)->client_rx.used = 0;
        (*handle)->client_rx.offset = 0;
        (*handle)->server_rx.data = client && size
                                    ? (byte *) MEM_MALLOC(size)
                                    : NULL;
        (*handle)->server_rx.size = client ? size : 0;
        (*handle)->server_rx.used = 0;
        (*handle)->server_rx.offset = 0;
        (*handle)->client_tx.data = server && size
                                    ? (byte *) MEM_MALLOC(size)
                                    : NULL;
        (*handle)->client_tx.size = server ? size : 0;
        (*handle)->client_tx.used = 0;
        (*handle)->client_tx.offset = 0;
        (*handle)->server_tx.data = client && size
                                    ? (byte *) MEM_MALLOC(size)
                                    : NULL;
        (*handle)->server_tx.size = client ? size : 0;
        (*handle)->server_tx.used = 0;
        (*handle)->server_tx.offset = 0;

        // Inform the channel handler of its construction
        if (poll) err = (*poll)(MUX_EVENT_START, NULL, 0);

        // Attempt to connect to a server if this is a client
        if (!err && client) err = mux_chan_connect(*handle, NULL);

        // Add to the list of channels if successful
        if (!err)
        {
            (*handle)->prev = NULL;
            (*handle)->next = mux_channel_list;
            if (mux_channel_list) mux_channel_list->prev = *handle;
            mux_channel_list = *handle;
        }
        else
        {
            if ((*handle)->client_rx.data) MEM_FREE((*handle)->client_rx.data);
            if ((*handle)->server_rx.data) MEM_FREE((*handle)->server_rx.data);
            if ((*handle)->client_tx.data) MEM_FREE((*handle)->client_tx.data);
            if ((*handle)->server_tx.data) MEM_FREE((*handle)->server_tx.data);
            MEM_FREE(*handle);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle of the channel to destroy.
                  now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the specified channel and any active connections.
*/
os_error *mux_chan_destroy(mux_channel handle, bool now)
{
    os_error *err = NULL;

    // Check parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        frame_data *frame;

        // Disconnect any remote client
        if (handle->client_chan != MUX_CHANNEL_CTRL)
        {
            // Send disconnection frame
            if (!now)
            {
                err = mux_ctrl_queue(handle->client_chan,
                                     MUX_MSG_CHANNEL_CLOSED, &frame);
            }
            if (!err) handle->client_chan = MUX_CHANNEL_CTRL;

            // Inform channel handler
            if (!err && handle->poll)
            {
                err = (*handle->poll)(MUX_EVENT_CLIENT_DISCONNECTED,
                                      NULL, 0);
            }
        }

        // Disconnect any remote server
        if (!err && (handle->server_chan != MUX_CHANNEL_CTRL))
        {
            // Send disconnection frame
            if (!now)
            {
                err = mux_ctrl_queue(handle->chan,
                                     MUX_MSG_CHANNEL_DISCONNECT, &frame);
                if (!err) err = parse_put_start_frame(frame);
                if (!err) err = parse_put_byte(handle->server_chan);
            }
            if (!err) handle->server_chan = MUX_CHANNEL_CTRL;

            // Inform channel handler
            if (!err && handle->poll)
            {
                err = (*handle->poll)(MUX_EVENT_SERVER_DISCONNECTED,
                                      NULL, 0);
            }
        }

        // Inform the channel handler of the imminent destruction
        if (!err && handle->poll)
        {
            err = (*handle->poll)(MUX_EVENT_END, NULL, 0);
        }

        // Destroy this channel
        if (!err)
        {
            // Unlink from the list of channels
            if (handle->prev) handle->prev->next = handle->next;
            else mux_channel_list = handle->next;
            if (handle->next) handle->next->prev = handle->prev;

            // Check any pointers
            if (mux_channel_last == handle) mux_channel_last = handle->next;

            // Deallocate the memory used by the channel status
            if (handle->client_rx.data) MEM_FREE(handle->client_rx.data);
            if (handle->server_rx.data) MEM_FREE(handle->server_rx.data);
            if (handle->client_tx.data) MEM_FREE(handle->client_tx.data);
            if (handle->server_tx.data) MEM_FREE(handle->server_tx.data);
            MEM_FREE(handle);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle of the channel to connect.
                  name          - An optional server name to use, or NULL to use
                                  the name specified when the channel was
                                  created.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Attempt to connect this channel to the corresponding server.
*/
os_error *mux_chan_connect(mux_channel handle, const char *name)
{
    os_error *err = NULL;

    // Check parameters
    if (!handle || !handle->client) err = &err_bad_parms;
    else if (handle->server_chan == MUX_CHANNEL_CTRL)
    {
        frame_data *frame;

        // Attempt to connect to a server
        err = mux_ctrl_queue(handle->chan, MUX_MSG_CONNECT_TO_SERVER, &frame);
        if (!err) err = parse_put_start_frame(frame);
        if (!err) err = parse_put_string(name ? name : handle->name);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle of the channel to send from.
                  data          - Pointer to the data.
                  size          - Size of the data frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Queue a data frame for transmission to this channel's client.
*/
os_error *mux_chan_tx_client(mux_channel handle, const byte *data, bits size)
{
    os_error *err = NULL;

    // Check parameters
    if (!handle || !data || !size || (handle->client_tx.size < size)
        || !handle->client_tx.data)
    {
        err = &err_bad_parms;
    }
    else
    {
        // Store the data
        memcpy(handle->client_tx.data, data, size);
        handle->client_tx.used = size;
        handle->client_tx.offset = 0;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle of the channel to send from.
                  data          - Pointer to the data.
                  size          - Size of the data frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Queue a data frame for transmission to this channel's server.
*/
os_error *mux_chan_tx_server(mux_channel handle, const byte *data, bits size)
{
    os_error *err = NULL;

    // Check parameters
    if (!handle || !data || !size || (handle->server_tx.size < size)
        || !handle->server_tx.data)
    {
        err = &err_bad_parms;
    }
    else
    {
        // Store the data
        memcpy(handle->server_tx.data, data, size);
        handle->server_tx.used = size;
        handle->server_tx.offset = 0;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : chan          - The remote channel to disable.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received XOFF frame.
*/
static os_error *mux_poll_rx_data_xoff(byte chan)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("NCP XOFF channel=%u", chan))

    // Block the specified channel
    mux_blocked[chan] = TRUE;

    // Return any error produced
    return err;
}

/*
    Parameters  : chan          - The remote channel to enable.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received XON frame.
*/
static os_error *mux_poll_rx_data_xon(byte chan)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("NCP XON channel=%u", chan))

    // Unblock the specified channel
    mux_blocked[chan] = FALSE;

    // Return any error produced
    return err;
}

/*
    Parameters  : chan          - The remote client channel.
                  name          - The name of the server to connect to.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received connect frame.
*/
static os_error *mux_poll_rx_connect_to_server(byte chan, const char *name)
{
    os_error *err = NULL;
    mux_channel ptr = mux_channel_list;
    frame_data *frame;

    DEBUG_PRINTF(("NCP connect channel=%u, server='%s'", chan, name))

    // Search for a matching link
    while (ptr
           && (strcmp(ptr->name, name)
               || !ptr->server
               || (ptr->client_chan != MUX_CHANNEL_CTRL)))
    {
        ptr = ptr->next;
    }

    // Handle the connection if successful
    if (ptr)
    {
        // Store the remote channel number
        ptr->client_chan = chan;

        // Inform the channel handler
        if (ptr->poll)
        {
            err = (*ptr->poll)(MUX_EVENT_CLIENT_CONNECTED, NULL, 0);
        }
    }

    // Build and send a reply
    err = mux_ctrl_queue(ptr ? ptr->chan : MUX_CHANNEL_CTRL,
                         MUX_MSG_CONNECT_RESPONSE, &frame);
    if (!err) err = parse_put_start_frame(frame);
    if (!err) err = parse_put_byte(chan);
    if (!err)
    {
        err = parse_put_byte(ptr ? STATUS_SIBO_NONE : STATUS_SIBO_FILE_NXIST);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : client        - The local client channel.
                  server        - The remote server channel.
                  result        - The result code.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received connect response frame.
*/
static os_error *mux_poll_rx_connect_response(byte client, byte server,
                                              status_code result)
{
    os_error *err = NULL;
    mux_channel ptr;

    DEBUG_PRINTF(("NCP connect response channel=%u, remote=%u, result=%i", client, server, result))

    // Attempt to find the local channel details
    ptr = mux_chan_find(client);
    if (ptr)
    {
        // Action depends on whether the connection was successful
        if (result == STATUS_SIBO_NONE)
        {
            // Store the channel details
            ptr->server_chan = server;

            // Inform the channel handler
            if (ptr->poll)
            {
                err = (*ptr->poll)(MUX_EVENT_SERVER_CONNECTED, NULL, 0);
            }
        }
        else
        {
            // Inform the channel handler
            if (ptr->poll)
            {
                err = (*ptr->poll)(MUX_EVENT_SERVER_FAILED, NULL, 0);
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : chan          - The channel that has been closed.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received connection termination frame.
*/
static os_error *mux_poll_rx_channel_closed(byte chan)
{
    os_error *err = NULL;
    mux_channel ptr;

    DEBUG_PRINTF(("NCP connection termination channel=%u", chan))

    // Attempt to find the local channel details
    ptr = mux_chan_find(chan);
    if (ptr && (ptr->client_chan != MUX_CHANNEL_CTRL))
    {
        // Clear the channel details
        ptr->client_chan = MUX_CHANNEL_CTRL;
        ptr->client_rx.used = 0;
        ptr->client_tx.used = 0;

        // Inform the channel handler
        if (ptr->poll)
        {
            err = (*ptr->poll)(MUX_EVENT_CLIENT_DISCONNECTED, NULL, 0);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : version       - The NCP version number.
                  id            - The ID.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received NCP info frame.
*/
static os_error *mux_poll_rx_ncp_info(byte version, bits id)
{
    os_error *err = NULL;
    bool era;

    DEBUG_PRINTF(("NCP info version=%u, id=%u", version, id))

    // Store the details
    mux_ncp_remote_version = version;
    mux_ncp_remote_id = id;

#ifndef MUX_SIBO
    // Update the connection status
    era = MUX_NCP_ERA_VERSION <= version;
    if (era != mux_era)
    {
        // Shutdown and restart higher layers
        err = unified_end(TRUE);
        if (!err)
        {
            mux_era = era;
            err = unified_start(mux_era);
        }
    }
#endif

    // Return any error produced
    return err;
}

/*
    Parameters  : server        - The local client channel.
                  client        - The remote server channel that has been
                                  closed.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received disconnection frame.
*/
static os_error *mux_poll_rx_channel_disconnect(byte client, byte server)
{
    os_error *err = NULL;
    mux_channel ptr;

    DEBUG_PRINTF(("NCP disconnection channel=%u, remote=%u", client, server))

    // Attempt to find the local channel details
    ptr = mux_chan_find(client);
    if (ptr && (ptr->server_chan == server))
    {
        // Clear the channel details
        ptr->server_chan = MUX_CHANNEL_CTRL;
        ptr->server_rx.used = 0;
        ptr->server_tx.used = 0;

        // Inform the channel handler
        if (ptr->poll)
        {
            err = (*ptr->poll)(MUX_EVENT_SERVER_DISCONNECTED, NULL, 0);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received NCP termination frame.
*/
static os_error *mux_poll_rx_ncp_end(void)
{
    os_error *err = NULL;
    mux_channel ptr = mux_channel_list;

    DEBUG_PRINTF(("NCP termination"))

    // Close any remaining open channels
    while (!err && ptr)
    {
        // Close active channels
        if (ptr->client_chan != MUX_CHANNEL_CTRL)
        {
            // Clear the channel number
            ptr->client_chan = MUX_CHANNEL_CTRL;
            ptr->client_rx.used = 0;
            ptr->client_tx.used = 0;

            // Inform channel handler
            if (ptr->poll)
            {
                err = (*ptr->poll)(MUX_EVENT_CLIENT_DISCONNECTED, NULL, 0);
            }
        }
        if (!err && (ptr->server_chan != MUX_CHANNEL_CTRL))
        {
            // Clear the channel number
            ptr->server_chan = MUX_CHANNEL_CTRL;
            ptr->server_rx.used = 0;
            ptr->server_tx.used = 0;

            // Inform channel handler
            if (ptr->poll)
            {
                err = (*ptr->poll)(MUX_EVENT_SERVER_DISCONNECTED, NULL, 0);
            }
        }

        // Advance to the next channel
        ptr = ptr->next;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - Pointer to a received control frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received control frame.
*/
static os_error *mux_poll_rx_ctrl(const frame_data *frame)
{
    os_error *err = NULL;
    byte dest;
    byte src;
    byte type;
    bits offset;

    // Process the standard part of the frame
    dest = frame->data[MUX_OFFSET_DEST];
    src = frame->data[MUX_OFFSET_SRC];
    type = frame->data[MUX_OFFSET_TYPE];

    // Start parsing the rest of the frame
    offset = MUX_OFFSET_DATA;
    err = parse_get_start_frame(frame, &offset);
    if (!err)
    {
        // Action depends on the frame type
        switch (type)
        {
            case MUX_MSG_DATA_XOFF:
                // XOFF frame
                err = mux_poll_rx_data_xoff(src);
                break;

            case MUX_MSG_DATA_XON:
                // XON frame
                err = mux_poll_rx_data_xon(src);
                break;

            case MUX_MSG_CONNECT_TO_SERVER:
                // Connect frame
                {
                    const char *name;

                    if (!parse_get_string(&name))
                    {
                        err = mux_poll_rx_connect_to_server(src, name);
                    }
                }
                break;

            case MUX_MSG_CONNECT_RESPONSE:
                // Connect response frame
                {
                    byte client;
                    status_code result;

                    if (!parse_get_byte(&client)
                        && !parse_get_byte((byte *) &result))
                    {
                        err = mux_poll_rx_connect_response(client, src, result);
                    }
                }
                break;

            case MUX_MSG_CHANNEL_CLOSED:
                // Connection termination frame
                err = mux_poll_rx_channel_closed(src);
                break;

            case MUX_MSG_NCP_INFO:
                // NCP info
                {
                    byte version;
                    bits id;

                    if (!parse_get_byte(&version)
                        && !parse_get_bits(&id))
                    {
                        err = mux_poll_rx_ncp_info(version, id);
                    }
                }
                break;

            case MUX_MSG_CHANNEL_DISCONNECT:
                // Disconnection frame
                {
                    byte client;

                    if (!parse_get_byte(&client))
                    {
                        err = mux_poll_rx_channel_disconnect(client, src);
                    }
                }
                break;

            case MUX_MSG_NCP_END:
                // NCP termination
                err = mux_poll_rx_ncp_end();
                break;

            default:
                // Ignore unknown frame types
                DEBUG_PRINTF(("NCP control frame unrecognised dest=%u, src=%u, type=%u", dest, src, type))
                break;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The source channel number.
                  dest          - The destination channel number.
                  complete      - Is this a complete data frame.
                  data          - Pointer to the start of the data.
                  size          - Size of the data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received and decoded data frame.
*/
os_error *mux_poll_rx_data_decoded(byte src, byte dest, bool complete,
                                   const byte *data, bits size)
{
    os_error *err = NULL;
    mux_channel ptr;
    mux_events event;
    mux_data_frame *frame;

    // Find the channel details
    ptr = mux_chan_find(dest);
    if (ptr)
    {
        // Decide whether the data is from the client or server
        if ((ptr->server_chan == src) && !ptr->server_tx.used)
        {
            event = MUX_EVENT_SERVER_DATA;
            frame = &ptr->server_rx;
        }
        else if ((ptr->client_chan == src) && !ptr->client_tx.used)
        {
            event = MUX_EVENT_CLIENT_DATA;
            frame = &ptr->client_rx;
        }
        else ptr = NULL;

        // No action if no buffer
        if (!frame->data) ptr = NULL;
    }

    // Add the data to the appropriate buffer
    if (ptr)
    {
        if (frame->used + size <= frame->size)
        {
            memcpy(&frame->data[frame->used], data, size);
        }
        frame->used += size;
    }

    // Handle complete frames
    if (ptr && complete)
    {
        // Call the channel poll function
        if (ptr->poll && frame->used && (frame->used <= frame->size))
        {
            err = (*ptr->poll)(event, frame->data, frame->used);
        }

        // Clear the received message
        frame->used = 0;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - Pointer to a received data frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received data frame.
*/
static os_error *mux_poll_rx_data(const frame_data *frame)
{
    os_error *err = NULL;
    byte dest;
    byte src;
    byte type;

    // Process the standard part of the frame
    dest = frame->data[MUX_OFFSET_DEST];
    src = frame->data[MUX_OFFSET_SRC];
    type = frame->data[MUX_OFFSET_TYPE];

    // Action depends on the frame type
    switch (type)
    {
        case MUX_MSG_WRITECOMPLETE:
            // Complete frame
            DEBUG_PRINTF(("NCP complete frame remote=%u, local=%u", src, dest))
            err = mux_poll_rx_data_decoded(src, dest, TRUE,
                                           &frame->data[MUX_OFFSET_DATA],
                                           frame->size - MUX_OFFSET_DATA);
            break;

        case MUX_MSG_WRITEPARTIAL:
            // Partial frame
            DEBUG_PRINTF(("NCP partial frame remote=%u, local=%u", src, dest))
            err = mux_poll_rx_data_decoded(src, dest, FALSE,
                                           &frame->data[MUX_OFFSET_DATA],
                                           frame->size - MUX_OFFSET_DATA);
            break;

        default:
            // Ignore unknown frame types
            DEBUG_PRINTF(("NCP data frame unrecognised dest=%u, src=%u, type=%u", dest, src, type))
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The source channel.
                  dest          - The destination channel.
                  data          - The data to transmit.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Transmit a pending data frame.
*/
os_error *mux_poll_tx_data(byte src, byte dest, mux_data_frame *data)
{
    os_error *err = NULL;
    static frame_data frame;
    bits size;

    // Calculate the frame size
    frame.size = MIN(FRAME_MAX_DATA_TX,
                     data->used - data->offset + MUX_OFFSET_DATA);
    size = frame.size - MUX_OFFSET_DATA;

    // Copy the data
    memcpy(&frame.data[MUX_OFFSET_DATA], &data->data[data->offset], size);
    data->offset += size;

    // Is this a complete write
    if (data->offset == data->used)
    {
        frame.data[MUX_OFFSET_TYPE] = MUX_MSG_WRITECOMPLETE;
        data->used = 0;
    }
    else frame.data[MUX_OFFSET_TYPE] = MUX_MSG_WRITEPARTIAL;

    // Complete the frame header
    frame.data[MUX_OFFSET_DEST] = dest;
    frame.data[MUX_OFFSET_SRC] = src;

    // Send this frame
    err = connect_tx(&frame);

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Transmit any pending frames.
*/
static os_error *mux_poll_tx(void)
{
    os_error *err = NULL;

    // Control frames take priority
    if (mux_ctrl_read != mux_ctrl_write)
    {
        // Send the next control frame
        err = connect_tx(&mux_ctrl[mux_ctrl_read]);
        if (!err) mux_ctrl_read = (mux_ctrl_read + 1) % MUX_MAX_CTRL;
    }
    else
    {
        // Find the next channel with pending data
        do
        {
            // Advance to the next channel to check
            if (mux_channel_last) mux_channel_last = mux_channel_last->next;
            else mux_channel_last = mux_channel_list;
        }
        while (mux_channel_last
               && (!mux_channel_last->client_tx.used
                   || mux_blocked[mux_channel_last->client_chan])
               && (!mux_channel_last->server_tx.used
                   || mux_blocked[mux_channel_last->server_chan]));

        // Transmit any pending data
        if (mux_channel_last)
        {
            // Choose the data to transmit
            if (mux_channel_last->server_tx.used
                && !(mux_channel_last->client_tx.used
                     && mux_channel_last->client_tx.offset))
            {
                err = mux_poll_tx_data(mux_channel_last->chan,
                                       mux_channel_last->server_chan,
                                       &mux_channel_last->server_tx);
            }
            else if (mux_channel_last->client_tx.used)
            {
                err = mux_poll_tx_data(mux_channel_last->chan,
                                       mux_channel_last->client_chan,
                                       &mux_channel_last->client_tx);
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : rx_frame      - Pointer to a received and validated data
                                  frame, or NULL if none.
                  tx_idle       - Can a data frame be transmitted.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle any received data and perform any other polled actions
                  required.
*/
os_error *mux_poll(const frame_data *rx_frame, bool tx_idle)
{
    os_error *err = NULL;

    // No action unless active
    if (mux_active)
    {
        mux_channel ptr;

        // Handle any received data
        if (rx_frame && (MUX_OFFSET_DATA <= rx_frame->size))
        {
            // Special case for the control channel
            if (rx_frame->data[MUX_OFFSET_DEST] == MUX_CHANNEL_CTRL)
            {
                err = mux_poll_rx_ctrl(rx_frame);
            }
            else
            {
                err = mux_poll_rx_data(rx_frame);
            }
        }

        // Poll idle channels
        ptr = mux_channel_list;
        while (!err && ptr)
        {
            // Call the poll function if idle
            if (ptr->poll
                && ((ptr->client_chan != MUX_CHANNEL_CTRL)
                    || (ptr->server_chan != MUX_CHANNEL_CTRL))
                && !ptr->client_tx.used && !ptr->server_tx.used)
            {
                err = (*ptr->poll)(MUX_EVENT_IDLE, NULL, 0);
            }

            // Try the next channel
            ptr = ptr->next;
        }

        // Perform any polled action at higher layers
        if (!err) err = unified_poll();

        // Transmit any new data if possible
        if (!err && tx_idle) err = mux_poll_tx();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for any current frame being transmitted to complete.
*/
static os_error *mux_wait_idle(void)
{
    os_error *err = NULL;
    escape_config esc;

    DEBUG_PRINTF(("Waiting for muliplexor layer to become idle"))

    // Enable escape conditions
    err = escape_store(&esc);
    if (!err) err = escape_enable();
    if (!err)
    {
        // Keep polling until idle
        while (!err && (mux_ctrl_read != mux_ctrl_write))
        {
            err = link_poll(TRUE);
        }

        // Restore the previous escape state
        escape_restore(&esc);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the multiplexor and fragmentation layer after a link
                  has been established.
*/
os_error *mux_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting multiplexor layer"))

    // No action if already active
    if (!mux_active)
    {
        bits i;
        frame_data *frame;

        // Start by assuming the connection state
#ifdef MUX_SIBO
        mux_era = FALSE;
#else
        mux_era = connect_era;
#endif

        // Reset the pending control frames list
        mux_ctrl_read = 0;
        mux_ctrl_write = 0;

        // Unblock all channels
        for (i = 0; i < MUX_CHANNEL_MAX; i++) mux_blocked[i] = FALSE;

        // Send the NCP info
        err = mux_ctrl_queue(MUX_CHANNEL_CTRL, MUX_MSG_NCP_INFO, &frame);
        if (!err) err = parse_put_start_frame(frame);
        if (!err)
        {
            err = parse_put_byte(mux_era
                                 ? MUX_NCP_ERA_VERSION
                                 : MUX_NCP_SIBO_VERSION);
        }
        if (!err) err = parse_put_bits(util_time());

        // Start higher layers
        if (!err) err = lnkchan_start();
        if (!err) err = unified_start(mux_era);

        // Set the active flag if successful
        if (!err) mux_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the multiplexor and fragmentation layer after a link
                  has been closed. No more data can be exchanged at this stage.
*/
os_error *mux_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending multiplexor layer now=%u", now))

    // No action if unless active
    if (mux_active)
    {
        // End higher layers
        err = unified_end(now);
        if (!err) err = lnkchan_end(now);

        // Destroy any remaining active channels
        while (!err && mux_channel_list)
        {
            err = mux_chan_destroy(mux_channel_list, now);
        }

        // Send a termination frame
        if (!err && !now)
        {
            frame_data *frame;

            err = mux_ctrl_queue(MUX_CHANNEL_CTRL, MUX_MSG_NCP_END, &frame);
        }

        // Wait for any pending control frames to be sent
        if (!err && !now) err = mux_wait_idle();

        // Clear the active flag if successful
        if (!err)
        {
            mux_ncp_remote_version = MUX_NCP_NO_VERSION;
            mux_active = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the multiplexor and
                  fragmentation layer.
*/
os_error *mux_status(void)
{
    os_error *err = NULL;
    mux_channel ptr = mux_channel_list;

    DEBUG_PRINTF(("Displaying multiplexor layer status"))

    // Display the current connection status
    if (mux_ncp_remote_version != MUX_NCP_NO_VERSION)
    {
        printf("Remote NCP version %u (", mux_ncp_remote_version);
        switch (mux_ncp_remote_version)
        {
            case MUX_NCP_SIBO_VERSION:
                // Old EPOC16 version
                printf("Old SIBO");
                break;

            case MUX_NCP_SIBO_NEW_VERSION:
                // New EPOC16 version
                printf("New SIBO");
                break;

            case MUX_NCP_ERA_VERSION:
                // EPOC32 version
                printf("EPOC");
                break;

            default:
                // Unrecognised NCP version
                printf("unrecognised");
                break;
        }
        printf(").\n");
    }

    // List the channels
    while (ptr)
    {
        // Display the channel details
        printf("%s on channel %u ", ptr->name, ptr->chan);
        if (ptr->client)
        {
            if (ptr->server_chan == MUX_CHANNEL_CTRL)
            {
                printf("not connected to server");
            }
            else printf("connected to server on %u", ptr->server_chan);
        }
        if (ptr->client && ptr->server) printf(", ");
        if (ptr->server)
        {
            if (ptr->client_chan == MUX_CHANNEL_CTRL)
            {
                printf("not connected to client");
            }
            else printf("connected to client on %u", ptr->client_chan);
        }
        printf(".\n");

        // Advance to the next channel
        ptr = ptr->next;
    }

    // Display the status of the next layer
    if (mux_active) err = unified_status();

    // Return any error produced
    return err;
}
