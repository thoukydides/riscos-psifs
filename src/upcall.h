/*
    File        : upcall.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Utility functions for the PsiFS module.

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
#ifndef UPCALL_H
#define UPCALL_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "fs.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : name          - The name of the disc.
                  retry         - Variable to receive whether a reply should be
                                  performed.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to request that a remote device with the
                  specified disc is connected.
*/
os_error *upcall_search(const char *name, bool *retry);

/*
    Parameters  : void
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to end a search for a disc.
*/
os_error *upcall_search_end(void);

/*
    Parameters  : path          - The filename in external format.
                  info          - The details for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to indicate that the specified object has
                  been added.
*/
os_error *upcall_added(const char *path, const fs_info *info);

/*
    Parameters  : path          - The filename in external format.
                  info          - The details for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to indicate that the specified object has
                  been removed.
*/
os_error *upcall_removed(const char *path, const fs_info *info);

/*
    Parameters  : path          - The filename in external format.
                  info          - The details for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to indicate that the specified object has
                  been changed.
*/
os_error *upcall_changed(const char *path, const fs_info *info);

#ifdef __cplusplus
    }
#endif

#endif
