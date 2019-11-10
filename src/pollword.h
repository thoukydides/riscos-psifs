/*
    File        : pollword.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Client pollword handler for the PsiFS module.

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
#ifndef POLLWORD_H
#define POLLWORD_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "psifs.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : name          - The name of the client.
                  mask          - The mask of changes of interest.
                  pollword      - Variable to receive a pointer to the pollword.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Register a client to be informed of changes.
*/
os_error *pollword_register(const char *name, psifs_mask mask, int **pollword);

/*
    Parameters  : pollword      - The previously allocated pollword pointer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Unregister the specified client.
*/
os_error *pollword_unregister(int *pollword);

/*
    Parameters  : mask          - The mask of changes to notify to clients.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Update any pollwords affected by the specified mask.
*/
os_error *pollword_update(psifs_mask mask);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check if any clients are registered.
*/
os_error *pollword_pre_finalise(void);

#ifdef __cplusplus
    }
#endif

#endif
