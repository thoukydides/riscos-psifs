/*
    File        : tar.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Tar file handling for the PsiFS module.

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
#ifndef TAR_H
#define TAR_H

// Include oslib header files
#include "oslib/os.h"
#include "oslib/osgbpb.h"

// Include project header files
#include "fs.h"

// A tar file handle
typedef struct tar_handle *tar_handle;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : name          - The name of the tar file to open.
                  handle        - Variable to receive a pointer to the handle.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Open an existing tar file for reading.
*/
os_error *tar_open_in(const char *name, tar_handle *handle);

/*
    Parameters  : name          - The name of the tar file to create.
                  handle        - Variable to receive a pointer to the handle.
                  append        - Should data be appended if the file already
                                  exists.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create and open a new tar file for writing.
*/
os_error *tar_open_out(const char *name, tar_handle *handle, bool append);

/*
    Parameters  : src           - The tar handle to clone.
                  dest          - Variable to receive the cloned handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone a tar file handle. The cloned handle references the
                  same tar file.
*/
os_error *tar_clone(tar_handle src, tar_handle *dest);

/*
    Parameters  : handle        - Handle of the tar file to close.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Close a previously opened tar file. This should be called the
                  same number of times as tar_open_in, tar_open_out, and
                  tar_clone.
*/
os_error *tar_close(tar_handle *handle);

/*
    Parameters  : handle        - Handle of the tar file to read.
                  info          - Variable to receive a pointer to the details
                                  of the next file, or NULL if no more.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details of the next file from a tar file.
*/
os_error *tar_info(tar_handle handle, const fs_info **info);

/*
    Parameters  : src           - The name of the source file.
                  name          - The name to store for the file.
                  dest          - Handle of the tar file to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start adding the specified file to the tar file. The operation
                  should be completed using tar_continue or tar_complete.
*/
os_error *tar_add(const char *src, const char *name, tar_handle dest);

/*
    Parameters  : handle        - Handle of the tar file to read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start skipping the next file from the tar file. The operation
                  should be completed using tar_continue or tar_complete.
*/
os_error *tar_skip(tar_handle handle);

/*
    Parameters  : src           - Handle of the tar file to read.
                  dest          - Tye name of the destination file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start extracting the next file from the tar file. The
                  operation should be completed using tar_continue or
                  tar_complete.
*/
os_error *tar_extract(tar_handle src, const char *dest);

/*
    Parameters  : src           - Handle of the tar file to read.
                  dest          - Handle of the tar file to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start copying the next file to another tar file. The
                  operation should be completed using tar_continue or
                  tar_complete.
*/
os_error *tar_copy(tar_handle src, tar_handle dest);

/*
    Parameters  : handle        - Handle of the tar file to process.
                  done          - Variable to receive the units of operation
                                  completed.
                  remain        - Variable to receive the units of operation
                                  still to be completed, or 0 if no operation
                                  in progress.
                  step          - Variable to receive the units to be completed
                                  during the next step.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Continue an operation that is in progress. Note that for a
                  copy operation either the source or destination handle may
                  be specified.
*/
os_error *tar_continue(tar_handle handle, bits *done, bits *remain,
                       bits *step);

/*
    Parameters  : handle        - Handle of the tar file to process.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Complete an operation that is in progress. Note that for a
                  copy operation either the source or destination handle may
                  be specified.
*/
os_error *tar_complete(tar_handle handle);

/*
    Parameters  : handle        - Handle of the tar file to read.
                  done          - Variable to receive the current offset within
                                  the tar file.
                  remain        - Variable to receive the size of the file that
                                  has not been processed.
                  step          - Variable to receive the size of the next file
                                  to read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Estimate the progress reading from the specified tar file.
*/
os_error *tar_position(tar_handle handle, bits *done, bits *remain, bits *step);

#ifdef __cplusplus
    }
#endif

#endif
