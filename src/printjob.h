/*
    File        : printjob.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print job handling for the PsiFS filer.

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
#ifndef PRINTJOB_H
#define PRINTJOB_H

// Include oslib header files
#include "oslib/wimp.h"

/*
    Parameters  : void
    Returns     : void
    Description : Update the print job status.
*/
void printjob_update(void);

/*
    Parameters  : void
    Returns     : void
    Description : Cancel all print jobs.
*/
void printjob_cancel(void);

/*
    Parameters  : void
    Returns     : bits      - The number of open print job windows.
    Description : Check how many print jobs are active.
*/
bits printjob_active(void);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_SetPrinter wimp message events.
*/
int printjob_set_printer(wimp_message *message, void *handle);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_PsifsPrint wimp message events.
*/
int printjob_print(wimp_message *message, void *handle);

#endif
