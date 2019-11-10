/*
    File        : print.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Printer mirror for the PsiFS module.

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
#ifndef PRINT_H
#define PRINT_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "fs.h"

// The connection state
extern bool print_active;
extern fs_pathname print_device;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : active        - Is the remote device present and active.
                  rx            - The next received character, or negative if
                                  none.
                  tx            - Variable to receive a character to transmit,
                                  or NULL if transmit buffer is full.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required. This must only be called
                  when a block driver is active and usable.
*/
os_error *print_poll(bool active, int rx, int *tx);

/*
    Parameters  : device        - The name of the device to write to, or NULL
                                  to use the default.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the printer mirror or change the destination device.
*/
os_error *print_start(const char *device);

/*
    Parameters  : now           - Should the block driver usage terminate
                                  immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the printer mirror. This will normally attempt a tidy
                  shutdown, but may be forced to terminate immediately.
*/
os_error *print_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the printer mirror.
*/
os_error *print_status(void);

#ifdef __cplusplus
    }
#endif

#endif
