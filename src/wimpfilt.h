/*
    File        : wimpfilt.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2000-2002, 2019
    Description : WIMP filter handlers for the PsiFS module.

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
#ifndef WIMPFILT_H
#define WIMPFILT_H

// Include project header files
#include "psifs.h"

// Include oslib header files
#include "oslib/wimp.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : mask          - Pointer to the event mask as passed to
                                  Wimp_Poll. This may be modified to provide
                                  a new event mask.
                  block         - Pointer to the event block as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new event.
                  task          - The task handle of the task that is being
                                  returned to.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handler for WIMP post-filter calls.
*/
os_error *wimpfilt_pre(wimp_poll_flags *mask, wimp_block *block, wimp_t task);

/*
    Parameters  : event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  block         - Pointer to the event block as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new event.
                  task          - The task handle of the task that is being
                                  returned to.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handler for WIMP post-filter calls.
*/
os_error *wimpfilt_post(wimp_event_no *event, wimp_block *block, wimp_t task);

/*
    Parameters  : pollword      - The previously allocated pollword pointer.
                  type          - The file type.
                  mask          - The masks of intercept types to handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Claim an intercepted file type for the specified client.
                  If 0 is passed for the mask then the intercept is released.
*/
os_error *wimpfilt_claim(int *pollword, bits type, psifs_intercept_type mask);

/*
    Parameters  : pollword      - The previously allocated pollword pointer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Release any intercepts associated with the specified client.
*/
os_error *wimpfilt_release(int *pollword);

/*
    Parameters  : pollword      - The previously allocated pollword pointer.
                  handle        - Variable to receive the handle for the
                                  intercept, or 0 for none.
                  type          - Variable to receive a the file type.
                  mask          - Variable to receive the intercept type.
                  orig          - Variable to receive a pointer to the original
                                  file name.
                  temp          - Variable to receive a pointer to the name of
                                  the temporary copy of the file.
                  sender        - Variable to receive the task handle of the
                                  sending task.
                  receiver      - Variable to receive the task handle of the
                                  receiving task.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check for any new intercepted operations for the specified
                  client. This should be called until no more operations are
                  returned.
*/
os_error *wimpfilt_poll(int *pollword, psifs_intercept_handle *handle,
                        bits *type, psifs_intercept_type *mask,
                        const char **orig, const char **temp,
                        wimp_t *sender, wimp_t *receiver);

/*
    Parameters  : handle        - The handle of the intercept to restart.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Restart an intercepted operation. This must be called from
                  a WIMP task.
*/
os_error *wimpfilt_restart(psifs_intercept_handle handle);

/*
    Parameters  : handle        - The handle of the intercept to replace.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Replace an intercepted operation. This must be called from
                  a WIMP task.
*/
os_error *wimpfilt_replace(psifs_intercept_handle handle);

/*
    Parameters  : handle        - The handle of the intercept to cancel.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Cancel an intercepted operation. This must be called from
                  a WIMP task.
*/
os_error *wimpfilt_cancel(psifs_intercept_handle handle);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the WIMP post-filters.
*/
os_error *wimpfilt_status(void);

#ifdef __cplusplus
    }
#endif

#endif
