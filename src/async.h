/*
    File        : async.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Asynchronous remote operations for the PsiFS module.

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
#ifndef ASYNC_H
#define ASYNC_H

// Include project header files
#include "backtree.h"
#include "fs.h"
#include "tar.h"
#include "psifs.h"

#ifdef __cplusplus
    extern "C" {
#endif

// An asynchronous remote operation
typedef struct async_op
{
    psifs_async_op op;
    union
    {
        struct
        {
            fs_pathname pattern;
            fs_pathname path;
            bool append;
        } shutdown;
        struct
        {
            fs_pathname path;
            bool remove;
        } restart;
        struct
        {
            fs_pathname src;
            fs_pathname dest;
        } read;
        struct
        {
            fs_pathname src;
            fs_pathname dest;
            bool remove;
        } write;
        struct
        {
            fs_pathname src;
            fs_pathname dest;
            fs_pathname prev;
            fs_pathname scrap;
            fs_pathname temp;
        } backup;
        struct
        {
            fs_pathname src;
            fs_pathname dest;
            fs_pathname exe;
            bool remove;
        } write_start;
        struct
        {
            fs_pathname inst_exe;
            fs_pathname inst_src;
            fs_pathname inst_dest;
            bool inst_remove;
            fs_pathname pckg_src;
            fs_pathname pckg_dest;
            bool pckg_remove;
        } install;
        struct
        {
            backtree_handle tree;
            fs_pathname src;
            fs_pathname sub;
        } backup_list;
        struct
        {
            backtree_handle tree;
            fs_pathname src;
            fs_pathname dest;
            fs_pathname prev;
            fs_pathname scrap;
        } backup_prev;
        struct
        {
            backtree_handle tree;
            fs_pathname src;
            fs_pathname dest;
            fs_pathname temp;
        } backup_copy;
        struct
        {
            tar_handle handle;
            psifs_async_status status;
            fs_pathname detail;
        } tar_complete;
        struct
        {
            fs_pathname path;
            psifs_drive drive;
        } find;
    } data;
} async_op;

/*
    Parameters  : op            - Details of the operation to perform.
                                  A temporary structure may be used since the
                                  contents will be copied.
                  handle        - Variable to receive a handle for this
                                  operation.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start an asynchronous operation.
*/
os_error *async_start(const async_op *op, psifs_async_handle *handle);

/*
    Parameters  : handle        - The previously allocated handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End an asynchronous operation.
*/
os_error *async_end(psifs_async_handle handle);

/*
    Parameters  : handle        - The previously allocated handle.
                  status        - Variable to receive the status.
                  desc          - Variable to receive a pointer to the
                                  description of the status.
                  detail        - Variable to receive a pointer to the
                                  details of the status.
                  error         - Variable to receive a pointer to any
                                  error text.
                  taken         - Variable to receive the number of
                                  centi-seconds taken so far.
                  remain        - Variable to receive an estimate of the number
                                  of centi-seconds remaining.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Poll the status of an asynchronous operation.
*/
os_error *async_poll(psifs_async_handle handle, psifs_async_status *status,
                     const char **desc, const char **detail,
                     const char **error, bits *taken, bits *remain);

/*
    Parameters  : handle        - The previously allocated handle.
                  response      - The reponse.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Process a response to a query.
*/
os_error *async_response(psifs_async_handle handle,
                         psifs_async_response response);

/*
    Parameters  : handle        - The previously allocated handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Pause an operation.
*/
os_error *async_pause(psifs_async_handle handle);

/*
    Parameters  : handle        - The previously allocated handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Resume a previously paused operation.
*/
os_error *async_resume(psifs_async_handle handle);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the asynchronous remote
                  operations layer.
*/
os_error *async_status(void);

#ifdef __cplusplus
    }
#endif

#endif
