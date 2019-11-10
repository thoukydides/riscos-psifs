/*
    File        : mux.h
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

// Only include header file once
#ifndef MUX_H
#define MUX_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "frame.h"

// Channel events
typedef enum
{
    MUX_EVENT_START,
    MUX_EVENT_END,
    MUX_EVENT_SERVER_FAILED,
    MUX_EVENT_SERVER_CONNECTED,
    MUX_EVENT_SERVER_DISCONNECTED,
    MUX_EVENT_SERVER_DATA,
    MUX_EVENT_CLIENT_CONNECTED,
    MUX_EVENT_CLIENT_DISCONNECTED,
    MUX_EVENT_CLIENT_DATA,
    MUX_EVENT_IDLE
} mux_events;

// Opaque type for a channel
typedef struct mux_channel_status *mux_channel;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : event         - The event to process.
                  data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Poll routine for an individual channel.
*/
typedef os_error *(* mux_channel_poll)(mux_events event, const byte *data,
                                       bits size);

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
                          mux_channel *handle);

/*
    Parameters  : handle        - The handle of the channel to destroy.
                  now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the specified channel and any active connections.
*/
os_error *mux_chan_destroy(mux_channel handle, bool now);

/*
    Parameters  : handle        - The handle of the channel to connect.
                  name          - An optional server name to use, or NULL to use
                                  the name specified when the channel was
                                  created.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Attempt to connect this channel to the corresponding server.
*/
os_error *mux_chan_connect(mux_channel handle, const char *name);

/*
    Parameters  : handle        - The handle of the channel to send from.
                  data          - Pointer to the data.
                  size          - Size of the data frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Queue a data frame for transmission to this channel's client.
*/
os_error *mux_chan_tx_client(mux_channel handle, const byte *data, bits size);

/*
    Parameters  : handle        - The handle of the channel to send from.
                  data          - Pointer to the data.
                  size          - Size of the data frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Queue a data frame for transmission to this channel's server.
*/
os_error *mux_chan_tx_server(mux_channel handle, const byte *data, bits size);

/*
    Parameters  : rx_frame      - Pointer to a received and validated data
                                  frame, or NULL if none.
                  tx_idle       - Can a data frame be transmitted.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle any received data and perform any other polled actions
                  required.
*/
os_error *mux_poll(const frame_data *rx_frame, bool tx_idle);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the multiplexor and fragmentation layer after a link
                  has been established.
*/
os_error *mux_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the multiplexor and fragmentation layer after a link
                  has been closed. No more data can be exchanged at this stage.
*/
os_error *mux_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the multiplexor and
                  fragmentation layer.
*/
os_error *mux_status(void);

#ifdef __cplusplus
    }
#endif

#endif
