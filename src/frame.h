/*
    File        : frame.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Frame manipulation for the link layer of the PsiFS
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
#ifndef FRAME_H
#define FRAME_H

// Include oslib header files
#include "oslib/os.h"

// Frame sizes
#define FRAME_MAX_DATA_RX (2048)
#define FRAME_MAX_DATA_TX (300)
#define FRAME_MAX_DATA (FRAME_MAX_DATA_RX)

// Status of connection to EPOC mobile phone
extern bool frame_phone;

// A frame with data
typedef struct
{
    byte cont;
    bits seq;
    byte data[FRAME_MAX_DATA];
    int size;
} frame_data;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : all           - Should all status be reset, or just the
                                  transmit status.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Reset the frame handler status. This should be called if the
                  connection handler times out, especially if the baud rate is
                  changed.
*/
os_error *frame_reset(bool all);

/*
    Parameters  : active        - Is the remote device present and active.
                  rx            - The next received character, or negative if
                                  none.
                  tx            - Variable to receive a character to transmit,
                                  or NULL if transmit buffer is full.
                  idle          - Should idle polling of higher layers be
                                  performed.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required. This must only be called
                  when a block driver is active and usable.
*/
os_error *frame_poll(bool active, int rx, int *tx, bool idle);

/*
    Parameters  : frame         - The message to send.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Abort transmission of any active frame, and transmit the
                  specified message. The message is copied, so the original
                  may be modified or transient.
*/
os_error *frame_send(const frame_data *frame);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the frame handler and any clients for the link.
*/
os_error *frame_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the frame handler and any clients for the link. This will
                  normally attempt a tidy shutdown, but may be forced to
                  terminate immediately.
*/
os_error *frame_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the frame handler and any
                  clients.
*/
os_error *frame_status(void);

#ifdef __cplusplus
    }
#endif

#endif
