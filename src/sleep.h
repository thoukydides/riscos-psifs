/*
    File        : sleep.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Sleeping in a taskwindow for the PsiFS module.

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
#ifndef SLEEP_H
#define SLEEP_H

// Include oslib header files
#include "oslib/os.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : task          - Variable to receive whether the code is being
                                  run in a taskwindow.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Check if a taskwindow is being used.
*/
os_error *sleep_taskwindow(bool *task);

/*
    Parameters  : poll          - Pointer to a poll word, or NULL for none.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : If running in a taskwindow then sleep until either the next
                  poll or the poll word becomes non-zero.
*/
os_error *sleep_snooze(int *poll);

#ifdef __cplusplus
    }
#endif

#endif
