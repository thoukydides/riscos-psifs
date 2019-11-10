/*
    File        : idle.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Idle detection and timing for the PsiFS module.

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
#ifndef IDLE_H
#define IDLE_H

// Include oslib header files
#include "oslib/os.h"

#ifdef __cplusplus
    extern "C" {
#endif

// Idle configuration
extern bits idle_disconnect_link;
extern bits idle_disconnect_printer;
extern bool idle_background_throttle;

/*
    Parameters  : void
    Returns     : void
    Description : Mark the start of an operation using the remote link. The
                  idle timeout is disabled for the duration of the operation.
                  A count of the number of active operations is maintained,
                  so calls to idle_start and idle_end must be matched.
*/
void idle_start(void);

/*
    Parameters  : void
    Returns     : void
    Description : Mark the end of an operation using the remote link. The
                  idle timeout is re-enabled if there are no other active
                  operations.
*/
void idle_end(void);

/*
    Parameters  : void
    Returns     : void
    Description : Reset the idle timeout. This is equivalent to calling
                  idle_start followed immediately by idle_end.
*/
void idle_kick(void);

/*
    Parameters  : time  - Required idle timeout in seconds.
    Returns     : bool  - Should a disconnection be performed.
    Description : Check if an automatic idle disconnection should occur.
*/
bool idle_check_disconnect(bits time);

/*
    Parameters  : void
    Returns     : bool  - Should background operations be throttled back.
    Description : Check if background operations should be reduced.
*/
bool idle_check_throttle(void);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the idle timeout.
*/
os_error *idle_status(void);

#ifdef __cplusplus
    }
#endif

#endif
