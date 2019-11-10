/*
    File        : timer.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Timer handling for the PsiFS module.

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
#ifndef TIMER_H
#define TIMER_H

// Include project header files
#include "share.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : timeout       - The time at which the timer should expire.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the specified length of time before returning
                  control.
*/
os_error *timer_fore(os_t timeout, bool escape);

/*
    Parameters  : timeout       - The time at which the timer should expire.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start a timer. Control is returned immediately. If the link
                  is not valid then no error is returned, but instead the
                  callback function is notified.
*/
os_error *timer_back(os_t timeout, void *user, share_callback callback);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required.
*/
os_error *timer_poll(void);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the timer server layer after the multiplexor layer
                  has been started.
*/
os_error *timer_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the timer server layer before the multiplexor layer has
                  been closed.
*/
os_error *timer_end(bool now);

#ifdef __cplusplus
    }
#endif

#endif
