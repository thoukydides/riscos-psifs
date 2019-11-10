/*
    File        : debug.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Allow debugging information to be output fom the PsiFS
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
#ifndef DEBUG_H
#define DEBUG_H

// Uncomment the next line to enable debugging output
//#define DEBUG

// Define the appropriate functions
#ifdef DEBUG

// Include clib header files
#include <stdio.h>

// Include project header files
#include "util.h"

// Bind to the special functions if debugging enabled
#define DEBUG_PRINTF(p) { printf("[PsiFS (%s/%i/@%u) - ", __FILE__, __LINE__, util_time()); printf p; printf("]\n"); }
#define DEBUG_ERR(e) { if (e) DEBUG_PRINTF(("Error %u '%s'", (e)->errnum, (e)->errmess)) }

#else

// No action if debugging disabled
#define DEBUG_PRINTF(p) {}
#define DEBUG_ERR(e) {}

#endif

#endif
