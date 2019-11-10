/*
    File        : sema.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Semaphore handling for the PsiFS module.

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
#ifndef SEMA_H
#define SEMA_H

// Include oslib header files
#include "oslib/types.h"

// A semaphore type
typedef bool semaphore;
#define SEMA_CLAIMED ((semaphore) 1)
#define SEMA_RELEASED ((semaphore) 0)

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : sema  - The semaphore to claim.
    Returns     : bool  - Was the semaphore claimed.
    Description : Attempt to claim the specified semaphore. This fails if the
                  semaphore has already been claimed.
*/
bool sema_claim(semaphore *sema);

/*
    Parameters  : sema  - The semaphore to release.
    Returns     : void
    Description : Release the specified semaphore. The semaphore must have been
                  claimed previously.
*/
void sema_release(semaphore *sema);

#ifdef __cplusplus
    }
#endif

#endif
