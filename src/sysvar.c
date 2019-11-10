/*
    File        : sysvar.c
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

// Include header file for this module
#include "sysvar.h"

// Include clib header files
#include <string.h>
#include "kernel.h"

// Include project header files
#include "ctrl.h"
#include "err.h"
#include "mem.h"

// Text for boolean values
#define SYSVAR_TRUE "True"
#define SYSVAR_FALSE "False"

/*
    Parameters  : var           - The name of the variable to read.
                  value         - Variable to receive the value of the
                                  variable.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the specified system variable as an integer. If the
                  variable is a string or a macro then it is evaluated.
*/
os_error *sysvar_read_int(const char *var, int *value)
{
    os_error *err = NULL;
    union
    {
        int num;
        char str[12];
    } buffer;
    int used;
    os_var_type type;

    // Check the parameters
    if (!var || !value) err = &err_bad_parms;
    else
    {
        // Read the value of the variable
        err =  xos_read_var_val(var, (char *) &buffer, sizeof(buffer) - 1, 0,
                                os_VARTYPE_NUMBER, &used, NULL, &type);

        // Convert the type if required
        if (err) *value = 0;
        else if (type == os_VARTYPE_NUMBER) *value = buffer.num;
        else
        {
            buffer.str[used] = '\0';
            *value = atoi(buffer.str);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : var           - The name of the variable to set.
                  value         - The value to set.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set the specified system variable as an integer.
*/
os_error *sysvar_write_int(const char *var, int value)
{
    os_error *err = NULL;

    // Check the parameters
    if (!var) err = &err_bad_parms;
    else
    {
        // Set the value of the variable
        err = xos_set_var_val(var, (const byte *) &value, sizeof(value), 0,
                              os_VARTYPE_NUMBER, NULL, NULL);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : var           - The name of the variable to read.
                  value         - Variable to receive the value of the
                                  variable.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the specified system variable as a boolean. Conversions
                  are performed as appropriate.
*/
os_error *sysvar_read_bool(const char *var, bool *value)
{
    os_error *err = NULL;
    char buffer[6];

    // Check the parameters
    if (!var || !value) err = &err_bad_parms;
    else
    {
        // Read the value as a string
        err = sysvar_read_string(var, buffer, sizeof(buffer));
        *value = !err && !ctrl_strcmp(buffer, SYSVAR_TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : var           - The name of the variable to set.
                  value         - The value to set.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set the specified system variable as a boolean.
*/
os_error *sysvar_write_bool(const char *var, bool value)
{
    os_error *err = NULL;

    // Check the parameters
    if (!var) err = &err_bad_parms;
    else
    {
        // Write the value as a string
        err = sysvar_write_string(var, value ? SYSVAR_TRUE : SYSVAR_FALSE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : var           - The name of the variable to read.
                  buffer        - Buffer to receive the value of the variable.
                  size          - Size of the buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the specified system variable as a string. If the
                  variable is an integer or a macro then it is converted.
*/
os_error *sysvar_read_string(const char *var, char *buffer, size_t size)
{
    os_error *err = NULL;
    int used;

    // Check the parameters
    if (!var || !buffer || !size) err = &err_bad_parms;
    else
    {
        // Read the value of the variable
        err =  xos_read_var_val(var, buffer, size - 1, 0, os_VARTYPE_EXPANDED,
                                &used, NULL, NULL);

        // Terminate the string
        buffer[err ? 0 : used] = '\0';
    }

    // Return any error produced
    return err;
}

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
os_error *sysvar_read_string_alloc(const char *var, char **buffer)
{
    os_error *err = NULL;
    int size;
    os_var_type type;

    // Check the parameters
    if (!var || !buffer) err = &err_bad_parms;
    else
    {
        _kernel_swi_regs regs;

        // No buffer initially
        *buffer = NULL;

        // Read the type of variable and size of buffer required
        regs.r[0] = (int) var;
        regs.r[1] = NULL;
        regs.r[2] = -1;
        regs.r[3] = 0;
        regs.r[4] = os_VARTYPE_STRING;
        err = (os_error *) _kernel_swi(OS_ReadVarVal, &regs, &regs);
        size = regs.r[2];
        type = regs.r[4];
        /*
        err = xos_read_var_val(var, NULL, -1, 0, os_VARTYPE_STRING,
                               &size, NULL, &type);
        */

        // No further action unless the variable exists
        if (size < 0)
        {
            // Initial buffer size depends on the variable type
            switch (type)
            {
                case os_VARTYPE_STRING:
                    // Buffer length just needs the terminator added
                    size = 1 + ~size;
                    break;

                case os_VARTYPE_NUMBER:
                    // Assume the largest possible integer
                    size = 12;
                    break;

                case os_VARTYPE_MACRO:
                default:
                    // The returned size is not accurate
                    size = 256;
                    break;
            }

            // Attempt to read the variable
            do
            {
                // Attempt to allocate a buffer of the selected size
                char *ptr = (char *) MEM_REALLOC(*buffer, size);
                if (ptr)
                {
                    // Copy the buffer pointer
                    *buffer = ptr;

                    // Attempt to read the variable
                    err = sysvar_read_string(var, *buffer, size);
                }
                else err = &err_buffer;

                // Release the memory if failed or increase the required size
                if (*buffer && err && (err->errnum != error_BUFF_OVERFLOW))
                {
                    MEM_FREE(*buffer);
                    *buffer = NULL;
                }
                else size <<= 1;
            } while (*buffer && err);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : var           - The name of the variable to set.
                  buffer        - The value to set.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set the specified system variable as a string.
*/
os_error *sysvar_write_string(const char *var, const char *buffer)
{
    os_error *err = NULL;

    // Check the parameters
    if (!var || !buffer) err = &err_bad_parms;
    else
    {
        // Set the value of the variable
        err = xos_set_var_val(var, (const byte *) buffer, strlen(buffer), 0,
                              os_VARTYPE_LITERAL_STRING, NULL, NULL);
    }

    // Return any error produced
    return err;
}
