/*
    File        : clipwin.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Clipboard window handling for the PsiFS filer.

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
#ifndef CLIPWIN_H
#define CLIPWIN_H

// Include oslib header files
#include "oslib/toolbox.h"

// Include project header files
#include "convobj.h"

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_ClaimEntity wimp message events.
*/
int clipwin_claim_entity(wimp_message *message, void *handle);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataRequest wimp message events.
*/
int clipwin_data_request(wimp_message *message, void *handle);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSave wimp message events.
*/
int clipwin_data_save(wimp_message *message, void *handle);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSaveAck wimp message events.
*/
int clipwin_data_save_ack(wimp_message *message, void *handle);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoad wimp message events.
*/
int clipwin_data_load(wimp_message *message, void *handle);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoadAck wimp message events.
*/
int clipwin_data_load_ack(wimp_message *message, void *handle);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_TaskCloseDown wimp message events.
*/
int clipwin_task_close_down(wimp_message *message, void *handle);

/*
    Parameters  : void
    Returns     : void
    Description : Perform any clipboard updates required.
*/
void clipwin_update(void);

#endif
