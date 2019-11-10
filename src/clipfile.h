/*
    File        : clipfile.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Upload the remote command services server to a remote device
                  for the PsiFS module.

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
#ifndef CLIPFILE_H
#define CLIPFILE_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "share.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Trigger a background copy of the remote clipboard server.
*/
os_error *clipfile_upload(void *user, share_callback callback);

#ifdef __cplusplus
    }
#endif

#endif
