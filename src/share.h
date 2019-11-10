/*
    File        : share.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Shared access to multiplexed servers channels for the
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
#ifndef SHARE_H
#define SHARE_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "mux.h"

// Handle for a shared channel
typedef struct share_handle *share_handle;
#define SHARE_NONE ((share_handle) NULL)

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a channel operation.
*/
typedef os_error *(* share_callback)(void *user, os_error *err,
                                     const void *reply);

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Function to start an operation.
*/
typedef os_error *(* share_send)(const void *cmd, void *reply);

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
typedef os_error *(* share_receive)(const void *cmd, void *reply,
                                    const byte *data, bits size);

/*
    Parameters  : handle        - The handle for the shared channel.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any operations required when an idle poll is
                  received.
*/
os_error *share_poll_idle(share_handle handle);

/*
    Parameters  : handle        - The handle for the shared channel.
                  data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any operations required when a data poll is received.
*/
os_error *share_poll_data(share_handle handle, const byte *data, bits size);

/*
    Parameters  : handle        - The handle for the shared channel.
                  cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the channel to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *share_fore(share_handle handle, const void *cmd, void *reply,
                     bool escape);

/*
    Parameters  : handle        - The handle for the shared channel.
                  cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the specified operation when the channel becomes idle.
                  Control is returned immediately. If the channel is not valid
                  or the operation fails then no error is returned, but instead
                  the callback function is notified.
*/
os_error *share_back(share_handle handle, const void *cmd, void *reply,
                     void *user, share_callback callback);

/*
    Parameters  : handle        - Variable to receive the shared channel handle.
                  send          - Function to send a message.
                  receive       - Function to handle a reply.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a new shared channel. This allocates a unique handle
                  for a connected server channel.
*/
os_error *share_create(share_handle *handle, share_send send,
                       share_receive receive);

/*
    Parameters  : handle        - Variable containing the shared channel handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy a previously created shared channel.
*/
os_error *share_destroy(share_handle *handle);

#ifdef __cplusplus
    }
#endif

#endif
