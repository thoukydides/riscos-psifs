/*
    File        : link.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Serial link handler for the PsiFS module.

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
#ifndef LINK_H
#define LINK_H

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/os.h"

// The current configuration
extern char *link_driver_name;
extern bits link_driver_port;
extern bits link_driver_baud;
extern char *link_driver_options;
extern bool link_driver_autobaud;

// The active baud rate
extern bits link_driver_active_baud;

// Is a serial driver active
extern bool link_active;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : bytes - Number of bytes to transmit.
    Returns     : bits  - Number of centi-seconds required, rounded up.
    Description : Calculate the number of centi-seconds required to transmit or
                  receive the specified number of bytes.
*/
bits link_time(bits bytes);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Intialise the serial link during module initialisation. This
                  attempts to restore the last used block driver and settings.
*/
os_error *link_initialise(void);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Shut down any active serial link during module finalisation.
*/
os_error *link_finalise(void);

/*
    Parameters  : changed       - Variable to receive whether the baud rate
                                  was changed.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Try the next available baud rate if automatic baud rate
                  identification selected.
*/
os_error *link_next_baud(bool *changed);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enable the block driver for a remote link.
*/
os_error *link_enable_link(void);

/*
    Parameters  : print         - The name of the device to write to, or NULL
                                  to use the default.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enable the block driver for a printer mirror.
*/
os_error *link_enable_print(const char *print);

/*
    Parameters  : now           - Should any active block driver be unloaded
                                  immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Disable the block driver if already active.
*/
os_error *link_disable(bool now);

/*
    Parameters  : name          - The name of the block driver to use.
                  port          - The port number.
                  baud          - The baud rate.
                  options       - Any other options.
                  autobaud      - Automatic baud rate identification mode.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set new serial link settings. It is safe to pass the existing
                  settings for any values to preserve.
*/
os_error *link_configure(const char *name, bits port, bits baud,
                         const char *options, bool autobaud);

/*
    Parameters  : escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled operations required with respect to the
                  link.
*/
os_error *link_poll(bool escape);

/*
    Parameters  : bool          - Should verbose information be output.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : List the current settings and status.
*/
os_error *link_list_settings(bool verbose);

/*
    Parameters  : bool          - Should verbose information be output.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : List the available block drivers.
*/
os_error *link_list_drivers(bool verbose);

#ifdef __cplusplus
    }
#endif

#endif
