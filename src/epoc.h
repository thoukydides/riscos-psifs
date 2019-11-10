/*
    File        : epoc.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Definition of common EPOC16 and EPOC32 types for the
                  PsiFS module.

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
#ifndef EPOC_H
#define EPOC_H

// Include oslib header files
#include "oslib/macros.h"

// Include project header files
#include "epoc16.h"
#include "epoc32.h"

// Maximum string lengths
#define EPOC_MAX_PATH (MAX(EPOC16_MAX_PATH, EPOC32_MAX_PATH))
#define EPOC_MAX_PROGRAM (EPOC_MAX_PATH)
#define EPOC_MAX_PROCESS (256)
#define EPOC_MAX_ARGS (256)

// Strings
typedef char epoc_path[EPOC_MAX_PATH + 1];
typedef char epoc_program[EPOC_MAX_PROGRAM + 1];
typedef char epoc_process[EPOC_MAX_PROCESS + 1];
typedef char epoc_args[EPOC_MAX_ARGS + 1];

// Process ID
typedef bits epoc_process_id;
#define EPOC_PROCESS_ID_NONE ((epoc_process_id) 0)

// Format handle
typedef bits epoc_format_handle;

// Media unique ID
typedef bits epoc_media_id;

#endif
