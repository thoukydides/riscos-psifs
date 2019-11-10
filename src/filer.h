/*
    File        : filer.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : The main module for the desktop filer.

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
#ifndef FILER_H
#define FILER_H

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/types.h"
#include "oslib/wimp.h"

// Include cathlibcpp header files
#include "string.h"

// Task handle for this task
extern wimp_t filer_task_handle;

// Poll word returned by PsiFS module
extern int *filer_poll_word;

// A variable to increment around error generating routines
extern bits filer_error_allowed;

/*
    Parameters  : token - The token to lookup.
                  arg0  - The optional first parameter.
                  arg1  - The optional second parameter.
                  arg2  - The optional third parameter.
                  arg3  - The optional fourth parameter.
    Returns     : char  - Pointer to the result.
    Description : Lookup the specified token in the message file opened by
                  the toolbox, substituting up to four parameters.
*/
string filer_msgtrans(const char *token, const char *arg0 = NULL,
                      const char *arg1 = NULL, const char *arg2 = NULL,
                      const char *arg3 = NULL);

/*
    Parameters  : token - The token to lookup.
                  arg0  - The optional first parameter.
                  arg1  - The optional second parameter.
                  arg2  - The optional third parameter.
                  arg3  - The optional fourth parameter.
    Returns     : char  - Pointer to the supplied buffer.
    Description : Lookup the specified token in the message file opened by
                  the toolbox, substituting up to four parameters, and placing
                  the result in the supplied buffer.
*/
char *filer_msgtrans(char *buffer, size_t size, const char *token,
                     const char *arg0 = NULL, const char *arg1 = NULL,
                     const char *arg2 = NULL, const char *arg3 = NULL);

/*
    Parameters  : task  - The task handle.
    Returns     : bool  - Are the tasks the same.
    Description : Check if the specified task handle refers to this task.
*/
bool filer_eq_task(wimp_t task);

#endif
