/*
    File        : args.c
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

// Include header file for this module
#include "args.h"

// Include clib header files
#include <stdlib.h>

// Include project header files
#include "err.h"
#include "mem.h"

// Buffer to use for all calls
static size_t args_buffer_size = 1;
static char **args_buffer = NULL;

/*
    Parameters  : void
    Returns     : bool  - Could the buffer be resized.
    Description : Attempt to enlarge the output buffer size.
*/
static bool args_resize(void)
{
    size_t size = args_buffer_size << 1;
    char **ptr = (char **) MEM_REALLOC(args_buffer, size);

    // Update the status if successful
    if (ptr)
    {
        args_buffer_size = size;
        args_buffer = ptr;
    }

    // Return the status
    return ptr ? TRUE : FALSE;
}

/*
    Parameters  : keywords      - The keywords definition.
                  input         - The input string to parse.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Call OS_ReadArgs using an internal output buffer.
*/
os_error *args_parse(const char *keywords, const char *input)
{
    os_error *err = NULL;

    // Check the parameters
    if (!keywords || !input) err = &err_bad_parms;
    else
    {
        // Perform the parsing
        err = xos_read_args(keywords, input,
                            (char *) args_buffer, args_buffer_size, NULL);

        // Repeat the parsing until the buffer is large enough
        while (err && (err->errnum == error_BUFF_OVERFLOW) && args_resize())
        {
            // Try to perform the parsing again
            err = xos_read_args(keywords, input,
                                (char *) args_buffer, args_buffer_size, NULL);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : arg   - The index of the argument to check (starting from 0).
    Returns     : bool  - Was the switch specified.
    Description : Check for the presence of a switch in the output buffer.
*/
bool args_read_switch(bits arg)
{
    // Return whether the switch was specified
    return args_buffer[arg] ? TRUE : FALSE;
}

/*
    Parameters  : arg   - The index of the argument to check (starting from 0).
                  value - Variable to receive a pointer to the resulting
                          string, or NULL if not present.
    Returns     : bool  - Was the value specified.
    Description : Return a pointer to the specified string.
*/
bool args_read_string(bits arg, const char **value)
{
    // Set the pointer to the string
    if (value) *value = args_buffer[arg];

    // Return whether the value was specified
    return args_buffer[arg] ? TRUE : FALSE;
}

/*
    Parameters  : arg   - The index of the argument to check (starting from 0).
                  value - Variable to receive a pointer to the resulting
                          string, or NULL if not present.
    Returns     : bool  - Was the value specified.
    Description : Return a pointer to the specified string.
*/
bool args_read_gstrans_string(bits arg, const char **value)
{
    // Set the pointer to the string
    if (value) *value = args_buffer[arg] ? args_buffer[arg] + 2 : NULL;

    // Return whether the value was specified
    return args_buffer[arg] ? TRUE : FALSE;
}

/*
    Parameters  : ptr   - Pointer to the integer.
    Returns     : int   - The value pointed to.
    Description : Read a non-word aligned integer value.
*/
static int args_int(char *ptr)
{
    // Return the value pointed to
    return ((bits) ptr[0])
           + (((bits) ptr[1]) << 8)
           + (((bits) ptr[2]) << 16)
           + (((bits) ptr[3]) << 24);
}

/*
    Parameters  : arg   - The index of the argument to check (starting from 0).
                  value - Variable to receive the value.
    Returns     : bool  - Was the value specified.
    Description : Return the requested value.
*/
bool args_read_evaluated(bits arg, int *value)
{
    // Set the value
    if (value) *value = args_buffer[arg] ? args_int(args_buffer[arg] + 1) : 0;

    // Return whether the value was specified
    return args_buffer[arg] ? TRUE : FALSE;
}
