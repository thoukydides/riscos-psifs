/*
    File        : upload.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Upload files to a remote device for the PsiFS module.

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
#ifndef UPLOAD_H
#define UPLOAD_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "fs.h"
#include "share.h"

// Available operations
typedef bits upload_op;
#define UPLOAD_COPY ((upload_op) 0x00)

// Data types for operations
typedef struct
{
    upload_op op;
    union
    {
        struct
        {
            fs_pathname path;
            fileswitch_attr attr;
            date_riscos date;
            bits size;
            const void *buffer;
        } copy;
    } data;
} upload_cmd;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : cmd           - Details of the file to upload.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Trigger a background remote file upload operation.
*/
os_error *upload_file(const upload_cmd *cmd, void *user,
                      share_callback callback);

/*
    Parameters  : era           - Is an ERA device connected.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the remote file uploader.
*/
os_error *upload_start(bool era);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the remote file uploader.
*/
os_error *upload_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the remote file uploader.
*/
os_error *upload_status(void);

#ifdef __cplusplus
    }
#endif

#endif
