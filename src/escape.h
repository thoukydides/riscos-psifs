/*
    File        : escape.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Escape configuration and condition handling for the PsiFS
                  module.

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
#ifndef ESCAPE_H
#define ESCAPE_H

// Include oslib header files
#include "oslib/os.h"

// Structure to store an escape configuration
typedef struct
{
    int esc_char;
    int break_action;
    int esc_state;
    int reset_effects;
} escape_config;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : config        - Variable to receive the current escape
                                  configuration.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read and store the current escape configuration.
*/
os_error *escape_store(escape_config *config);

/*
    Parameters  : config        - Variable to receive the current escape
                                  configuration.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Restore a previously saved escape configuration.
*/
os_error *escape_restore(const escape_config *config);

/*
    Parameters  : void
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Enable normal escape handling.
*/
os_error *escape_enable(void);

/*
    Parameters  : void
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Check for an active escape condition. An error is returned
                  and the condition cleared if necessary.
*/
os_error *escape_check(void);

#ifdef __cplusplus
    }
#endif

#endif
