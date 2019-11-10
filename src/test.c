/*
    File        : test.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Test functions for the PsiFS module.

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
#include "test.h"

// Include clib header files
#include <stdio.h>
#include <string.h>

// Include project header files
#include "module.h"
#include "rclip.h"

// No code if test command disabled
#ifdef CMD_Test

/*
    Parameters  : args          - Arguments to control the test behaviour.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform a test operation.
*/
os_error *test(const char *args)
{
    os_error *err = NULL;

    // ...

    // Return any error produced
    return err;
}

#endif
