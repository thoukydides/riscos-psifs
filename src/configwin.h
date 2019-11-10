/*
    File        : configwin.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Configuration event handling for the PsiFS filer.

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
#ifndef CONFIGWIN_H
#define CONFIGWIN_H

// Include oslib header files
#include "oslib/toolbox.h"

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Initialise when the configuration window is shown.
*/
bool configwin_show(bits event_code, toolbox_action *action,
                    toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Tidy up when the configuration window is closed.
*/
bool configwin_hide(bits event_code, toolbox_action *action,
                    toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the OK action button.
*/
bool configwin_action(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the cancel action button.
*/
bool configwin_cancel(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the save action button.
*/
bool configwin_save(bits event_code, toolbox_action *action,
                    toolbox_block *id_block, void *handle);

#endif
