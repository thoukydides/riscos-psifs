/*
    File        : sysvar.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : System variable handling for the PsiFS module.

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
#ifndef SYSVAR_H
#define SYSVAR_H

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/os.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : var           - The name of the variable to read.
                  value         - Variable to receive the value of the
                                  variable.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the specified system variable as an integer. If the
                  variable is a string or a macro then it is evaluated.
*/
os_error *sysvar_read_int(const char *var, int *value);

/*
    Parameters  : var           - The name of the variable to set.
                  value         - The value to set.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set the specified system variable as an integer.
*/
os_error *sysvar_write_int(const char *var, int value);

/*
    Parameters  : var           - The name of the variable to read.
                  value         - Variable to receive the value of the
                                  variable.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the specified system variable as a boolean. Conversions
                  are performed as appropriate.
*/
os_error *sysvar_read_bool(const char *var, bool *value);

/*
    Parameters  : var           - The name of the variable to set.
                  value         - The value to set.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set the specified system variable as a boolean.
*/
os_error *sysvar_write_bool(const char *var, bool value);

/*
    Parameters  : var           - The name of the variable to read.
                  buffer        - Buffer to receive the value of the variable.
                  size          - Size of the buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the specified system variable as a string. If the
                  variable is an integer or a macro then it is converted.
*/
os_error *sysvar_read_string(const char *var, char *buffer, size_t size);

/*
    Parameters  : var           - The name of the variable to read.
                  buffer        - Variable to receive a pointer to the buffer
                                  containing the string.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the specified system variable as a string. If the
                  variable is an integer or a macro then it is converted.
                  A suitable sized block of memory is allocated to hold the
                  result. This should be released when finished with by
                  calling free.
*/
os_error *sysvar_read_string_alloc(const char *var, char **buffer);

/*
    Parameters  : var           - The name of the variable to set.
                  buffer        - The value to set.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set the specified system variable as a string.
*/
os_error *sysvar_write_string(const char *var, const char *buffer);

#ifdef __cplusplus
    }
#endif

#endif
