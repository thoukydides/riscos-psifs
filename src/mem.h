/*
    File        : mem.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Memory management for the PsiFS module.

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
#ifndef MEM_H
#define MEM_H

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/os.h"

// Uncomment the next line to enable checking
//#define MEM_CHECK

// Extend the supplied arguments with debugging information
#define MEM_MALLOC(s) MEM_RAW_MALLOC((s), __FILE__, __LINE__)
#define MEM_CALLOC(s) MEM_RAW_CALLOC((s), __FILE__, __LINE__)
#define MEM_REALLOC(p, s) MEM_RAW_REALLOC((p), (s), __FILE__, __LINE__)
#define MEM_FREE(p) MEM_RAW_FREE((p), __FILE__, __LINE__)

// Map to the appropriate function prototypes
#ifdef MEM_CHECK
#define MEM_RAW_MALLOC(s, f, l) mem_malloc(s, f, l)
#define MEM_RAW_CALLOC(s, f, l) mem_calloc(s, f, l)
#define MEM_RAW_REALLOC(p, s, f, l) mem_realloc(p, s, f, l)
#define MEM_RAW_FREE(p, f, l) mem_free(p, f, l)
#else
#define MEM_RAW_MALLOC(s, f, l) mem_malloc(s)
#define MEM_RAW_CALLOC(s, f, l) mem_calloc(s)
#define MEM_RAW_REALLOC(p, s, f, l) mem_realloc(p, s)
#define MEM_RAW_FREE(p, f, l) mem_free(p)
#endif

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Initialise the memory manager.
*/
os_error *mem_initialise(void);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Finalise the memory manager.
*/
os_error *mem_finalise(void);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Tidy the memory manager.
*/
os_error *mem_tidy(void);

/*
    Parameters  : size      - The amount of memory to allocate.
                  file      - The name of the source code file.
                  line      - The line number in the source file.
    Returns     : void *    - Pointer to the allocated memory or NULL if failed.
    Description : Allocate a block of memory with optional checking.
*/
#ifdef MEM_CHECK
void *mem_malloc(size_t size, const char *file, bits line);
#else
void *mem_malloc(size_t size);
#endif

/*
    Parameters  : n         - The number of objects
                  size      - The amount of memory to allocate for each object.
                  file      - The name of the source code file.
                  line      - The line number in the source file.
    Returns     : void *    - Pointer to the allocated memory or NULL if failed.
    Description : Allocate and initialise a block of memory with optional
                  checking.
*/
#ifdef MEM_CHECK
void *mem_calloc(size_t n, size_t size, const char *file, bits line);
#else
void *mem_calloc(size_t n, size_t size);
#endif

/*
    Parameters  : ptr       - Pointer to a previously allocated block, or NULL
                              for none.
                  size      - The new amount of memory to allocate.
                  file      - The name of the source code file.
                  line      - The line number in the source file.
    Returns     : void *    - Pointer to the allocated memory or NULL if failed.
    Description : Reallocate a block of memory with optional checking.
*/
#ifdef MEM_CHECK
void *mem_realloc(void *ptr, size_t size, const char *file, bits line);
#else
void *mem_realloc(void *ptr, size_t size);
#endif

/*
    Parameters  : ptr       - Pointer to a previously allocated block.
                  file      - The name of the source code file.
                  line      - The line number in the source file.
    Returns     : void
    Description : Free a block of memory with optional checking.
*/
#ifdef MEM_CHECK
void mem_free(void *ptr, const char *file, bits line);
#else
void mem_free(void *ptr);
#endif

#ifdef __cplusplus
    }
#endif

#endif
