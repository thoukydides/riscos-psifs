/*
    File        : connect.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Connection manager for the link layer of the PsiFS
                  module.

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
#ifndef CONNECT_H
#define CONNECT_H

// Include project header files
#include "frame.h"

// The connection state
extern bool connect_active;
extern bool connect_connected;
extern bool connect_era;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : active        - Is the remote device present and active.
                  rx_frame      - Pointer to a received message, or NULL if
                                  none.
                  tx_idle       - Is the frame transmitter idle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle any received frame and perform any other polled
                  actions required.
*/
os_error *connect_poll(bool active, const frame_data *rx_frame, bool tx_idle);

/*
    Parameters  : frame         - The data to send.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Transmit the specified data. An error is returned if either
                  there is no connection or the transmit queue is full.
*/
os_error *connect_tx(const frame_data *frame);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the connection managers and any clients for the link.
*/
os_error *connect_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the connection manager and any clients for the link.
                  This will normally attempt a tidy shutdown, but may be forced
                  to terminate immediately.
*/
os_error *connect_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the connection manager and any
                  clients.
*/
os_error *connect_status(void);

#ifdef __cplusplus
    }
#endif

#endif
