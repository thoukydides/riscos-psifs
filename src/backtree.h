/*
    File        : backtree.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Backup tree manipulation for the PsiFS module.

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
#ifndef BACKTREE_H
#define BACKTREE_H

// Include project header files
#include "fs.h"

// Handle for a backup tree
typedef struct backtree_handle *backtree_handle;
#define BACKTREE_NONE ((backtree_handle) NULL)

// Result of a comparison
typedef int backtree_result;
#define BACKTREE_SAME ((backtree_result) 0)
#define BACKTREE_NOT_FOUND ((backtree_result) 1)
#define BACKTREE_OLDER ((backtree_result) 2)
#define BACKTREE_NEWER ((backtree_result) 3)

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : handle        - The backup tree handle.
                  info          - The details of the file, including the
                                  sub-directory part of the path.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Add the specified file to the backup tree. If necessary, this
                  also adds dummy entries for any parent directories.
*/
os_error *backtree_add(backtree_handle handle, const fs_info *info);

/*
    Parameters  : handle        - The backup tree handle.
                  info          - The details of the file to compare, including
                                  the sub-directory part of the path.
                  result        - Variable to receive the result of the
                                  comparison.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Compare the specified file details to the corresponding file
                  to backup.
*/
os_error *backtree_check(backtree_handle handle, const fs_info *info,
                         backtree_result *result);

/*
    Parameters  : handle        - The backup tree handle.
                  info          - The details of the file to remove, including
                                  the sub-directory part of the path.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Mark the specified file in the backup tree to be skipped
                  during the copy stage of the backup. This does not remove the
                  file, leaving it available for subsequent calls to
                  backtree_check.
*/
os_error *backtree_ignore(backtree_handle handle, const fs_info *info);

/*
    Parameters  : handle        - The backup tree handle.
                  info          - Variable to receive a pointer to the file
                                  details, including the sub-directory part of
                                  the path, or NULL if no more.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Return a pointer to the details of a file that has not been
                  marked to be ignored. To enumerate successive files,
                  backtree_ignore should be called with the returned pointer.
*/
os_error *backtree_enumerate(backtree_handle handle, const fs_info **info);

/*
    Parameters  : handle        - The backup tree handle.
                  files         - Variable to receive the number of files.
                  size          - Variable to receive the total size of the
                                  files.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Count the number of files that have not been marked to ignore.
*/
os_error *backtree_count(backtree_handle handle, bits *files, bits *size);

/*
    Parameters  : handle        - Variable to receive the backup tree handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a new backup tree.
*/
os_error *backtree_create(backtree_handle *handle);

/*
    Parameters  : src           - The backup tree handle to clone.
                  dest          - Variable to receive the cloned handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone a backup tree handle. The cloned handle references the
                  same backup tree.
*/
os_error *backtree_clone(backtree_handle src, backtree_handle *dest);

/*
    Parameters  : handle        - Variable containing the backup tree handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy a previously created backup tree. This should be
                  called the same number of times as backtree_create or
                  backtree_clone.
*/
os_error *backtree_destroy(backtree_handle *handle);

#ifdef __cplusplus
    }
#endif

#endif
