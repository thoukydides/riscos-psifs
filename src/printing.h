/*
    File        : printing.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Remote print job handling for the PsiFS module.

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
#ifndef PRINTING_H
#define PRINTING_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "psifs.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : handle        - Variable to receive the handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Obtain the handle of the next unclaimed print job.
*/
os_error *printing_next(psifs_print_job_handle *handle);

/*
    Parameters  : handle        - Handle of the print job.
                  status        - Variable to receive the status.
                  received      - Variable to receive the number of complete
                                  pages received.
                  read          - Variable to receive the number of pages read.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Poll the status of a print job.
*/
os_error *printing_poll(psifs_print_job_handle handle,
                        psifs_print_job_status *status,
                        bits *received, bits *read);

/*
    Parameters  : handle        - Handle of the print job.
                  name          - Name of the file to write the page data to.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read a page from a print job.
*/
os_error *printing_read(psifs_print_job_handle handle, const char *name);

/*
    Parameters  : handle        - Handle of the print job.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Cancel a print job.
*/
os_error *printing_cancel(psifs_print_job_handle handle);

/*
    Parameters  : data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
                  last_packet   - Is this the end of the current page.
                  last_page     - Is this part of the last page in the job.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Process data received from the remote printing server.
*/
os_error *printing_data(const byte *data, bits size,
                        bool last_packet, bool last_page);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : The print job was cancelled before completion.
*/
os_error *printing_cancelled(void);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the print job services layer after the multiplexor
                  layer has been started.
*/
os_error *printing_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the print job services layer before the multiplexor
                  layer has been closed.
*/
os_error *printing_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the print job services layer.
*/
os_error *printing_status(void);

#ifdef __cplusplus
    }
#endif

#endif
