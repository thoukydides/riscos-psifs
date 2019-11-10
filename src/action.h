/*
    File        : action.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Application level event handling for the PsiFS filer.

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
#ifndef ACTION_H
#define ACTION_H

// Include oslib header files
#include "oslib/toolbox.h"

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Perform a disconnect.
*/
bool action_disconnect(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the remote link.
*/
bool action_link(bits event_code, toolbox_action *action,
                 toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the printer mirror with data sent to the default
                  destination.
*/
bool action_printer_default(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the printer mirror with data sent to the parallel port.
*/
bool action_printer_parallel(bits event_code, toolbox_action *action,
                             toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the printer mirror with data sent to the serial port.
*/
bool action_printer_serial(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the printer mirror with data sent to a specified file.
*/
bool action_printer_file(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Open a filer window.
*/
bool action_filer(bits event_code, toolbox_action *action,
                  toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Prepare the name disk window.
*/
bool action_name_init(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Rename a disk.
*/
bool action_name_action(bits event_code, toolbox_action *action,
                        toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Open the backup window.
*/
bool action_backup(bits event_code, toolbox_action *action,
                   toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Open the free space viewer.
*/
bool action_free(bits event_code, toolbox_action *action,
                 toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Open the online help.
*/
bool action_help(bits event_code, toolbox_action *action,
                 toolbox_block *id_block, void *handle);

#endif
