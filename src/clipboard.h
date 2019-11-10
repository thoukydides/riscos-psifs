/*
    File        : clipboard.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Local clipboard handling for the PsiFS module.

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
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "psifs.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : flags         - Variable to receive the current clipboard
                                  status flags.
                  timestamp     - Variable to receive the timestamp of the
                                  last change to the clipboard.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Poll the status of the remote clipboard.
*/
os_error *clipboard_poll(psifs_clipboard_flags *flags, os_t *timestamp);

/*
    Parameters  : name          - The name of the file to copy to the clipboard.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Copy to the local clipboard.
*/
os_error *clipboard_copy(const char *name);

/*
    Parameters  : name          - The name of the file to paste the clipboard
                                  contents to.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Paste from the local clipboard.
*/
os_error *clipboard_paste(const char *name);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the remote clipboard to the local clipboard file.
*/
os_error *clipboard_remote_copy(void);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the local clipboard services layer after the
                  multiplexor layer has been started.
*/
os_error *clipboard_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the local clipboard services layer before the multiplexor
                  layer has been closed.
*/
os_error *clipboard_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the local clipboard services
                  layer.
*/
os_error *clipboard_status(void);

#ifdef __cplusplus
    }
#endif

#endif
