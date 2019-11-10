/*
    File        : mem.c
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

// Include header file for this module
#include "mem.h"

// Include clib header files
#include <limits.h>
#include <string.h>

// Include oslib header files
#include "oslib/macros.h"
#include "oslib/os.h"
#include "oslib/osheap.h"

// Include project header files
#ifdef MEM_CHECK
//#define DEBUG
#endif
#include "debug.h"
#include "fs.h"

// Dynamic area
#define MEM_DYNAMIC_AREA_NAME FS_NAME " workspace"
#define MEM_DYNAMIC_AREA_INITIAL (1024)
#define MEM_DYNAMIC_AREA_LIMIT (10 * 1024 * 1024)
#define MEM_DYNAMIC_AREA_HEAP_BASE (((bits *) mem_dynamic_area_base)[2])
#define MEM_DYNAMIC_AREA_HEAP_END (((bits *) mem_dynamic_area_base)[3])
static os_dynamic_area_no mem_dynamic_area_number = 0;
static byte *mem_dynamic_area_base = 0;

// Suffix to memory allocations
#ifdef MEM_CHECK
typedef struct
{
    bits guard;
} mem_post;
#define MEM_POST_SIZE (sizeof(mem_post))
#else
#define MEM_POST_SIZE (0)
#endif

// Prefix to memory allocations
typedef struct mem_pre
{
#ifdef MEM_CHECK
    bits guard1;
#endif
    bits size;
#ifdef MEM_CHECK
    bits line;
    char file[100];
    mem_post *post;
    struct mem_pre *next;
    struct mem_pre *prev;
    bits guard2;
#endif
} mem_pre;
#define MEM_PRE_SIZE (sizeof(mem_pre))

// Guard words
#define MEM_GUARD_PRE1 (0X7b697350) // Psi{
#define MEM_GUARD_PRE2 (0X3a697350) // Psi:
#define MEM_GUARD_POST (0X7d697350) // Psi}

// Convert memory pointer types
#define MEM_PRE(p) ((mem_pre *) (((char *) (p)) - sizeof(mem_pre)))
#define MEM_PTR(p) ((void *) (((char *) (p)) + sizeof(mem_pre)))
#define MEM_ROUND(s, d) ((((bits) s) + ((d) - 1)) & ~((d) - 1))

// The start of the list of allocated blocks
static mem_pre *mem_head = NULL;

/*
    Parameters  : void
    Returns     : void
    Description : Tidy up the memory manager on termination.
*/
static void mem_atexit(void)
{
    // Delete any dynamic area created
    if (mem_dynamic_area_number)
    {
        // Delete the dynamic area
        xosdynamicarea_delete(mem_dynamic_area_number);
        mem_dynamic_area_number = 0;
    }
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Initialise the memory manager.
*/
os_error *mem_initialise(void)
{
    os_error *err = NULL;

    // Register an atexit handler to tidy up on termination
    atexit(mem_atexit);

    // Attempt to create a dynamic area
    err = xosdynamicarea_create(-1, MEM_DYNAMIC_AREA_INITIAL, (byte *) -1,
                                0x80, MEM_DYNAMIC_AREA_LIMIT, NULL, NULL,
                                MEM_DYNAMIC_AREA_NAME,
                                &mem_dynamic_area_number,
                                &mem_dynamic_area_base, NULL);
    if (!err)
    {
        int size;

        // Initialise a heap in the dynamic area
        err = xos_read_dynamic_area(mem_dynamic_area_number, NULL, &size, NULL);
        if (!err) err = xosheap_initialise(mem_dynamic_area_base, size);
        if (err) xosdynamicarea_delete(mem_dynamic_area_number);
    }
    if (err)
    {
        // Failure to create a dynamic area is not fatal
        err = NULL;
        mem_dynamic_area_number = 0;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Finalise the memory manager.
*/
os_error *mem_finalise(void)
{
    // Finalisation is performed by an atexit handler
    return NULL;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Tidy the memory manager.
*/
os_error *mem_tidy(void)
{
    os_error *err = NULL;

    // Shrink the heap and dynamic area as far as possible
    if (mem_dynamic_area_number)
    {
        int change = MEM_DYNAMIC_AREA_HEAP_BASE - MEM_DYNAMIC_AREA_HEAP_END;
        int changed;

        // Shrink the heap as far as possible
        err = xosheap_resize(mem_dynamic_area_base, change);

        // Attempt to shrink the dynamic area
        if (!err)
        {
            err = xos_change_dynamic_area(mem_dynamic_area_number,
                                          change, &changed);
        }

        // Grow the heap back to the size of the dynamic area
        if (!err) err = xosheap_resize(mem_dynamic_area_base, changed - change);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : size  - The block size to ensure that there is space for.
    Returns     : void
    Description : Attempt to grow the dynamic area to allow the specified sized
                  block to be allocated.
*/
static void mem_grow(size_t size)
{
    os_error *err = NULL;
    int change;
    int changed;

    // Calculate the required change in size
    change = size + MEM_DYNAMIC_AREA_HEAP_BASE - MEM_DYNAMIC_AREA_HEAP_END;

    // Attempt to grow the dynamic area
    err = xos_change_dynamic_area(mem_dynamic_area_number, change, &changed);

    // Grow the heap to the new size of the dynamic area
    if (!err) err = xosheap_resize(mem_dynamic_area_base, changed);
}

/*
    Parameters  : size      - The amount of memory to allocate.
                  file      - The name of the source code file.
                  line      - The line number in the source file.
    Returns     : void *    - Pointer to the allocated memory or NULL if failed.
    Description : Allocate a block of memory from the current memory pool.
*/
static void *mem_allocate(size_t size)
{
    void *ptr;

    // Default to standard library allocator if no dynamic area
    if (mem_dynamic_area_number)
    {
        // Attempt to allocate the requested memory
        if (xosheap_alloc(mem_dynamic_area_base, size, &ptr))
        {
            // Try again after resizing the dynamic area
            mem_grow(size);
            if (xosheap_alloc(mem_dynamic_area_base, size, &ptr)) ptr = NULL;
        }
    }
    else ptr = malloc(size);

    // Return the result
    return ptr;
}

/*
    Parameters  : ptr       - Pointer to a previously allocated block.
    Returns     : void
    Description : Free a block of memory from the current memory pool.
*/
static void mem_deallocate(void *ptr)
{
    // Default to standard library allocator if no dynamic area
    if (mem_dynamic_area_number)
    {
        // Attempt to free the block
        xosheap_free(mem_dynamic_area_base, ptr);
    }
    else free(ptr);
}

// Only define checking functions if used
#ifdef MEM_CHECK

// Map the function calls
#define MEM_PRINTF(p) DEBUG_PRINTF(p)
#define MEM_CHECK_ALL() mem_check_all()
#define MEM_CHECK_VALIDATE(p) mem_check_validate(p)

/*
    Parameters  : void
    Returns     : bool  - Is the allocated memory valid.
    Description : Check the specified pointer.
*/
static bool mem_check_validate(mem_pre *ptr)
{
    bool ok = FALSE;

    // Check the memory block
    if (!ptr) DEBUG_PRINTF(("Null pointer"))
    else if (ptr->guard1 != MEM_GUARD_PRE1) DEBUG_PRINTF(("First prefix guard word overwritten"))
    else if (ptr->guard2 != MEM_GUARD_PRE2) DEBUG_PRINTF(("Second prefix guard word overwritten"))
    else if (ptr->post->guard != MEM_GUARD_POST) DEBUG_PRINTF(("Suffix guard word overwritten"))
    else ok = TRUE;

    // Display more diagnostics if failed
    if (!ok)
    {
        DEBUG_PRINTF(("Pointer value = %p", ptr))
        DEBUG_PRINTF(("Bad block (%s/%i) %i", ptr->file, ptr->line, ptr->size))
    }

    // Return the status
    return ok;
}

/*
    Parameters  : void
    Returns     : bool  - Is the allocated memory valid.
    Description : Check the consistency of the allocated memory.
*/
static bool mem_check_all(void)
{
    bool ok = TRUE;
    mem_pre *ptr = mem_head;

    // Check the complete list
    while (ok && ptr)
    {
        ok = mem_check_validate(ptr);
        ptr = ptr->next;
    }

    // Return the status
    return ok;
}

#else

// Define dummy
#define MEM_PRINTF(p) {}
#define MEM_CHECK_ALL() {}
#define MEM_CHECK_VALIDATE(p) {}

#endif

/*
    Parameters  : size      - The amount of memory to allocate.
                  file      - The name of the source code file.
                  line      - The line number in the source file.
    Returns     : void *    - Pointer to the allocated memory or NULL if failed.
    Description : Allocate a block of memory with optional checking.
*/
#ifdef MEM_CHECK
void *mem_malloc(size_t size, const char *file, bits line)
#else
void *mem_malloc(size_t size)
#endif
{
    mem_pre *ptr;

    MEM_PRINTF(("malloc(%u) @ (%s/%i)", size, file, line))

    // Check the current memory allocations
    MEM_CHECK_ALL();

    // Round the size of block required
    if (size < 32) size = MEM_ROUND(size, 8);
    else if (size < 128) size = MEM_ROUND(size, 16);
    else if (size < 512) size = MEM_ROUND(size, 32);
    else if (size < 2048) size = MEM_ROUND(size, 64);
    else if (size < 8192) size = MEM_ROUND(size, 128);
    else size = MEM_ROUND(size, 256);

    // Allocate a larger block
    ptr = (mem_pre *) mem_allocate(size + MEM_PRE_SIZE + MEM_POST_SIZE);
    if (ptr)
    {
        // Fill in the extra fields
        ptr->size = size;

#ifdef MEM_CHECK
        // Additional fields if memory checking enabled
        ptr->guard1 = MEM_GUARD_PRE1;
        strncpy(ptr->file, file, sizeof(ptr->file));
        ptr->line = line;
        ptr->post = (mem_post *) (((char *) ptr) + MEM_PRE_SIZE + size);
        ptr->guard2 = MEM_GUARD_PRE2;
        ptr->post->guard = MEM_GUARD_POST;

        // Link into the list
        ptr->next = mem_head;
        if (mem_head) mem_head->prev = ptr;
        ptr->prev = NULL;
        mem_head = ptr;
#endif
    }
    else MEM_PRINTF(("Memory allocation failed"))

    // Return the result
    return ptr ? MEM_PTR(ptr) : NULL;
}

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
void *mem_calloc(size_t n, size_t size, const char *file, bits line)
#else
void *mem_calloc(size_t n, size_t size)
#endif
{
    void *ptr;

    MEM_PRINTF(("calloc(%u, %u) @ (%s/%i)", n, size, file, line))

    // Remap to a malloc style call
    ptr = MEM_RAW_MALLOC(n * size, file, line);
    if (ptr) memset(ptr, 0, n * size);

    // Return the result
    return ptr;
}

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
void *mem_realloc(void *ptr, size_t size, const char *file, bits line)
#else
void *mem_realloc(void *ptr, size_t size)
#endif
{
    void *this = NULL;

    MEM_PRINTF(("realloc(%p, %u) @ (%s/%i)", ptr, size, file, line))

    // Handle the special cases
    if (size)
    {
        // Check if the existing block can be reused
        if (ptr && (size <= MEM_PRE(ptr)->size))
        {
            // Existing block is large enough
            this = ptr;
        }
        else
        {
            // Remap to a malloc followed by a copy and free
            this = MEM_RAW_MALLOC(size, file, line);
            if (this && ptr)
            {
                memcpy(this, ptr, MIN(size, MEM_PRE(ptr)->size));
                MEM_RAW_FREE(ptr, file, line);
            }
        }
    }
    else if (ptr) MEM_RAW_FREE(ptr, file, line);

    // Return the result
    return this;
}

/*
    Parameters  : ptr       - Pointer to a previously allocated block.
                  file      - The name of the source code file.
                  line      - The line number in the source file.
    Returns     : void
    Description : Free a block of memory with optional checking.
*/
#ifdef MEM_CHECK
void mem_free(void *ptr, const char *file, bits line)
#else
void mem_free(void *ptr)
#endif
{
    MEM_PRINTF(("free(%p) @ (%s/%i)", ptr, file, line))

    // Check the current memory allocations
    MEM_CHECK_ALL();

    // No action unless pointer valid
    if (ptr)
    {
        mem_pre *this = MEM_PRE(ptr);

        MEM_PRINTF(("Free block (%s/%i) %i", this->file, this->line, this->size))

        // Check this pointer
        MEM_CHECK_VALIDATE(this);

#ifdef MEM_CHECK
        // Unlink from the list
        if (this->next) this->next->prev = this->prev;
        if (this->prev) this->prev->next = this->next;
        else mem_head = this->next;

        // Scramble the guard words
        this->guard1 = this->guard2 = this->post->guard = 0;
#endif

        // Free the memory
        mem_deallocate(this);
    }
}
