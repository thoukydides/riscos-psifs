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

// Only include header file once
#ifndef LNKCHAN_H
#define LNKCHAN_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "share.h"

// Data types for registering servers
typedef char *lnkchan_cmd;
typedef char lnkchan_reply[16];

// Shared access handler
extern share_handle lnkchan_share_handle;

#ifdef __cplusplus
    extern "C" {
#endif

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
                           void *user, share_callback callback);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create both a client and server link channel.
*/
os_error *lnkchan_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the client and server link channel.
*/
os_error *lnkchan_end(bool now);

#ifdef __cplusplus
    }
#endif

#endif
