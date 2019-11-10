/*
    File        : escape.c
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

// Include header file for this module
#include "escape.h"

// Inlcude oslib header files
#include "oslib/osbyte.h"

// Include project header files
#include "err.h"

// Default escape configuration
#define ESCAPE_ESCAPE_CHAR (27)
#define ESCAPE_BREAK_ACTION (1)
#define ESCAPE_ESCAPE_STATE (0)
#define ESCAPE_RESET_EFFECTS (0)

/*
    Parameters  : config        - Variable to receive the current escape
                                  configuration.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read and store the current escape configuration.
*/
os_error *escape_store(escape_config *config)
{
    os_error *err = NULL;

    // Check the parameters
    if (!config) err = &err_bad_parms;

    // Read the current escape configuration
    if (!err) err = xosbyte_read(osbyte_VAR_ESCAPE_CHAR, &config->esc_char);
    if (!err) err = xosbyte_read(osbyte_VAR_INTERPRETATION_BREAK, &config->break_action);
    if (!err) err = xosbyte_read(osbyte_VAR_ESCAPE_STATE, &config->esc_state);
    if (!err) err = xosbyte_read(osbyte_VAR_RESET_EFFECTS, &config->reset_effects);

    // Return any error produced
    return err;
}

/*
    Parameters  : config        - Variable to receive the current escape
                                  configuration.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Restore a previously saved escape configuration.
*/
os_error *escape_restore(const escape_config *config)
{
    os_error *err = NULL;

    // Check the parameters
    if (!config) err = &err_bad_parms;

    // Set the specified escape configuration
    if (!err) err = xosbyte_write(osbyte_VAR_ESCAPE_CHAR, config->esc_char);
    if (!err) err = xosbyte_write(osbyte_VAR_INTERPRETATION_BREAK, config->break_action);
    if (!err) err = xosbyte_write(osbyte_VAR_ESCAPE_STATE, config->esc_state);
    if (!err) err = xosbyte_write(osbyte_VAR_RESET_EFFECTS, config->reset_effects);

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Enable normal escape handling.
*/
os_error *escape_enable(void)
{
    os_error *err = NULL;

    // Set the defaut escape configuration
    err = xosbyte_write(osbyte_VAR_ESCAPE_CHAR, ESCAPE_ESCAPE_CHAR);
    if (!err) err = xosbyte_write(osbyte_VAR_INTERPRETATION_BREAK, ESCAPE_BREAK_ACTION);
    if (!err) err = xosbyte_write(osbyte_VAR_ESCAPE_STATE, ESCAPE_ESCAPE_STATE);
    if (!err) err = xosbyte_write(osbyte_VAR_RESET_EFFECTS, ESCAPE_RESET_EFFECTS);

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Check for an active escape condition. An error is returned
                  and the condition cleared if necessary.
*/
os_error *escape_check(void)
{
    os_error *err = NULL;
    bits psr;

    // Read the escape state
    err = xos_read_escape_state(&psr);

    // Check for an escape condition
    if (!err && (psr & _C))
    {
        // Clear the escape condition
        err = xosbyte(osbyte_CLEAR_ESCAPE, 0, 0);

        // Set an escape error
        if (!err) err = &err_ext_escape;
    }

    // Return any error produced
    return err;
}
