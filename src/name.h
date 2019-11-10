/*
    File        : name.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : File name conversions for the PsiFS module.

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
#ifndef NAME_H
#define NAME_H

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/os.h"

// Special characters in Psion filenames
#define NAME_CHAR_SEPARATOR '\\'
#define NAME_CHAR_EXTENSION '.'
#define NAME_CHAR_DISC ':'

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Size of the result buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory path from the format used by
                  RISC OS to that used by ERA.
*/
os_error *name_riscos_to_era(const char *src, char *dest, size_t size);

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Size of the result buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory path from the format used by
                  RISC OS to that used by SIBO.
*/
os_error *name_riscos_to_sibo(const char *src, char *dest, size_t size);

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Size of the result buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory path from the format used by
                  ERA to that used by RISC OS.
*/
os_error *name_era_to_riscos(const char *src, char *dest, size_t size);

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Size of the result buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory path from the format used by
                  SIBO to that used by RISC OS.
*/
os_error *name_sibo_to_riscos(const char *src, char *dest, size_t size);

#ifdef __cplusplus
    }
#endif

#endif
