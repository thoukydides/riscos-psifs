/*
    File        : args.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Parsing of command line arguments for the PsiFS modules.

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
#ifndef ARGS_H
#define ARGS_H

// Include oslib header files
#include "oslib/os.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : keywords      - The keywords definition.
                  input         - The input string to parse.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Call OS_ReadArgs using an internal output buffer.
*/
os_error *args_parse(const char *keywords, const char *input);

/*
    Parameters  : arg   - The index of the argument to check (starting from 0).
    Returns     : bool  - Was the switch specified.
    Description : Check for the presence of a switch in the output buffer.
*/
bool args_read_switch(bits arg);

/*
    Parameters  : arg   - The index of the argument to check (starting from 0).
                  value - Variable to receive a pointer to the resulting
                          string, or NULL if not present.
    Returns     : bool  - Was the value specified.
    Description : Return a pointer to the specified string.
*/
bool args_read_string(bits arg, const char **value);

/*
    Parameters  : arg   - The index of the argument to check (starting from 0).
                  value - Variable to receive a pointer to the resulting
                          string, or NULL if not present.
    Returns     : bool  - Was the value specified.
    Description : Return a pointer to the specified string.
*/
bool args_read_gstrans_string(bits arg, const char **value);

/*
    Parameters  : arg   - The index of the argument to check (starting from 0).
                  value - Variable to receive the value.
    Returns     : bool  - Was the value specified.
    Description : Return the requested value.
*/
bool args_read_evaluated(bits arg, int *value);

#ifdef __cplusplus
    }
#endif

#endif
