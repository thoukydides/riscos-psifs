/*
    File        : claim.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Support for the device claim protocol for the PsiFS filer.

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
#ifndef CLAIM_H
#define CLAIM_H

// Include oslib header files
#include "oslib/toolbox.h"

// Include cathlibcpp header files
#include "string.h"

/*
    Parameters  : void
    Returns     : void
    Description : Start the remote link after claiming the serial port.
*/
void claim_start_link();

/*
    Parameters  : device    - An optional device for the printer mirror.
    Returns     : void
    Description : Start the printer mirror after claiming the serial port.
*/
void claim_start_printer(const string device);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DeviceClaim wimp message events.
*/
int claim_device_claim(wimp_message *message, void *handle);

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DeviceInUse wimp message events.
*/
int claim_device_in_use(wimp_message *message, void *handle);

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle unacknowledged messages.
*/
bool claim_acknowledge(wimp_event_no event_code, wimp_block *block,
                       toolbox_block *id_block, void *handle);

#endif
