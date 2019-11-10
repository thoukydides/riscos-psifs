/*
    File        : cache.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Cached remote file and directory access for the PsiFS
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

// Include header file for this module
#include "cache.h"

// Include clib header files
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "oslib/osfile.h"
#include "oslib/osword.h"

// Include project header files
#include "debug.h"
#include "err.h"
#include "idle.h"
#include "link.h"
#include "mem.h"
#include "pollword.h"
#include "rclip.h"
#include "unified.h"
#include "upcall.h"
#include "util.h"
#include "wildcard.h"
#include "wprt.h"

// Uncomment the next line to disable background updates
//#define CACHE_FOREGROUND

// Priorities for pending operations
typedef enum
{
    CACHE_PRIORITY_NONE,
    CACHE_PRIORITY_REFRESH,
    CACHE_PRIORITY_INVALID,
    CACHE_PRIORITY_REQUIRED
} cache_priority;
#define CACHE_REFRESH_DELAY (100)
#define CACHE_INVALID_DELAY (10)

// The next operation to perform
static cache_priority cache_next_priority;
static os_t cache_next_time;
static os_t cache_next_busy = 0;
static int cache_next_refresh;
static unified_cmd cache_next_cmd;
static unified_reply cache_next_reply;
static bool cache_next_active = FALSE;

// Delays between successive operations
static os_t cache_delay_time = 0;
#define CACHE_BACK_DELAY (50)
#define CACHE_FORE_DELAY (15)

// Cached machine type
static bool cache_machine_required = FALSE;
static bool cache_machine_valid = FALSE;
static os_error *cache_machine_err = NULL;
static psifs_machine_type cache_machine_type;
static unified_machine_type cache_machine_name;
static unified_machine_id cache_machine_id;
static psifs_language cache_machine_language;
static unified_version cache_machine_version;

// Cached owner information
static bool cache_owner_required = FALSE;
static bool cache_owner_valid = FALSE;
static os_error *cache_owner_err = NULL;
static unified_owner cache_owner_info;

// Cached power status
#define CACHE_POWER_TIMEOUT (30 * 100)
static bool cache_power_required = FALSE;
static bool cache_power_valid = FALSE;
static os_error *cache_power_err = NULL;
static os_t cache_power_refresh;
static unified_battery cache_power_main;
static unified_battery cache_power_backup;
static bool cache_power_external;

// Clock synchronization
bool cache_sync = FALSE;
static bool cache_sync_done = FALSE;
static os_error *cache_sync_err = NULL;

// Cached directory details
#define CACHE_DIR_TIMEOUT (10 * 100)
typedef struct cache_dir
{
    bool required;
    bool valid;
    os_error *err;
    struct cache_dir *parent;
    struct cache_dir *next;
    struct cache_dir *prev;
    fs_info info;
    fs_handle open;
    struct
    {
        bool active;
        bool required;
        bool valid;
        os_error *err;
        os_t refresh;
        struct cache_dir *children;
    } dir;
} cache_dir;
//static cache_dir *cache_dir_free = NULL;

// Cached drive details
#define CACHE_DRIVE_TIMEOUT_ACTIVE (20 * 100)
#define CACHE_DRIVE_TIMEOUT_INACTIVE (60 * 100)
typedef struct
{
    os_t refresh;
    fs_drive info;
    cache_dir root;
} cache_drive;
static cache_drive cache_drive_array[26];

// Pending operations
typedef enum
{
    CACHE_PENDING_STATE_INITIAL,
    CACHE_PENDING_STATE_DELETE,
    CACHE_PENDING_STATE_OPEN,
    CACHE_PENDING_STATE_CLOSE,
    CACHE_PENDING_STATE_STAMP,
    CACHE_PENDING_STATE_ACCESS,
    CACHE_PENDING_STATE_RESIZE,
    CACHE_PENDING_STATE_READ,
    CACHE_PENDING_STATE_WRITE,
    CACHE_PENDING_STATE_DONE
} cache_pending_state;
typedef struct cache_pending
{
    struct cache_pending *next;
    struct cache_pending *prev;
    const cache_cmd *cmd;
    cache_reply *reply;
    void *user;
    share_callback callback;
    cache_pending_state state;
} cache_pending;
//static cache_pending *cache_pending_free = NULL;
static cache_pending *cache_pending_head = NULL;
static cache_pending *cache_pending_tail = NULL;
static bool cache_pending_cmd = FALSE;
static os_error *cache_pending_err = FALSE;
static bool cache_pending_reply = FALSE;

// Buffer for reading file details
#define CACHE_BUFFER_SIZE (64)
static fs_info *cache_buffer = NULL;
static bits cache_buffer_size = CACHE_BUFFER_SIZE;

// A cached file handle
typedef struct cache_file
{
    fs_handle next;
    fs_handle prev;
    cache_dir *dir;
    fs_open_info info;
    bool stamp;
    bits load;
    bits exec;
    bool access;
    fileswitch_attr attr;
    bits sequential;
    unified_handle handle;
} cache_file;
//static fs_handle cache_handle_free = NULL;
static fs_handle cache_handle_active = NULL;

// Status for foreground operations
static bool cache_fore_done;
static os_error *cache_fore_err;

// The connection status
static bool cache_era = FALSE;
static bool cache_active = FALSE;
static bool cache_connected = FALSE;

// Counter of background operation disable
bits cache_disable = 0;

// Function prototypes
static os_error *cache_find_dir(const char *path, bool required, bool *valid,
                                cache_dir **dir);
static os_error *cache_process(void);

/*
    Parameters  : allocated     - The value to round.
    Returns     : bits          - The rounded allocated size.
    Description : Round the allocated size of a file to a multiple of the
                  buffer size.
*/
static bits cache_round_allocated(bits allocated)
{
    // Return the rounded size
    return ((allocated + FS_BUFFER_SIZE - 1) / FS_BUFFER_SIZE) * FS_BUFFER_SIZE;
}

/*
    Parameters  : handle        - The file handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Update both sequential file pointers for an open file to lie
                  within the appropriate file extent.
*/
static os_error *cache_check_sequential(fs_handle handle)
{
    os_error *err = NULL;

    // Check parameters
    if (handle == FS_NONE) err = &err_bad_parms;
    else
    {
        // Check the real sequential file pointer
        if (handle->info.allocated < handle->sequential)
        {
            handle->sequential = handle->info.allocated;
        }

        // Check the simulated sequential file pointer
        if (handle->info.extent < handle->info.sequential)
        {
            handle->info.sequential = handle->info.extent;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enlarge the size of the buffer used for reading directory
                  entries.
*/
static os_error *cache_buffer_grow(void)
{
    os_error *err = NULL;

    // Free any previously allocated buffer and choose the new buffer size
    if (cache_buffer)
    {
        MEM_FREE(cache_buffer);
        cache_buffer = NULL;
        cache_buffer_size <<= 1;
    }

    // Attempt to allocate a new buffer
    cache_buffer = (fs_info *) MEM_MALLOC(sizeof(fs_info) * cache_buffer_size);
    if (!cache_buffer) err = &err_buffer;

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Free the memory used by any allocated buffer.
*/
static os_error *cache_buffer_free(void)
{
    os_error *err = NULL;

    // Free any previously allocated buffer
    if (cache_buffer)
    {
        MEM_FREE(cache_buffer);
        cache_buffer = NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start higher level layers.
*/
static os_error *cache_connect(void)
{
    os_error *err = NULL;

    // Start higher layers
    if (cache_era)
    {
        err = rclip_start();
        if (!err) err = wprt_start();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End higher level layers.
*/
static os_error *cache_disconnect(bool now)
{
    os_error *err = NULL;

    // End higher layers
    err = wprt_end(now);
    if (!err) err = rclip_end(now);

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The path to split up.
                  parent        - Variable to receive a pointer to the parent
                                  path.
                  leaf          - Variable to receive a pointer to the leaf
                                  name, or NULL if the root directory.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Split the specified path into a parent and leaf name if
                  possible. The results should be copied if required; the values
                  are stored in a temporary buffer.
*/
static os_error *cache_dir_parent(const char *path,
                                  const char **parent, const char **leaf)
{
    os_error *err = NULL;

    // Check function parameters
    if (!path || (!parent && !leaf)) err = &err_bad_parms;
    else
    {
        static fs_pathname str;
        char *ptr;

        // Copy the supplied path
        if ((path[0] != FS_CHAR_DISC) || (!isalpha(path[1]))
            || (path[2] != FS_CHAR_SEPARATOR) || (path[3] != FS_CHAR_ROOT))
        {
            err = &err_bad_name;
        }
        else if (sizeof(str) <= strlen(path)) err = &err_bad_name;
        else strcpy(str, path);

        // Separate the leaf name
        if (!err)
        {
            ptr = strrchr(str, FS_CHAR_SEPARATOR);
            if (!ptr) err = &err_bad_name;
            else if (ptr < str + 4) ptr = NULL;
            else *ptr++ = '\0';
        }

        // Return pointers to the results
        if (parent) *parent = !err ? str : NULL;
        if (leaf) *leaf = !err ? ptr : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dir           - The directory entry to check.
                  path          - Variable to receive a pointer to the result.
                  external      - Should the external name (with a disc name
                                  rather than a drive letter) be generated.
                  virtual       - Should a virtual drive name be generated.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Generate the path corresponding to the specified directory
                  entry. The result should be copied if required; the value is
                  stored in a temporary buffer.
*/
static os_error *cache_dir_name(const cache_dir *dir, const char **path,
                                bool external, bool virtual)
{
    os_error *err = NULL;

    // Check function parameters
    if (!dir || !path) err = &err_bad_parms;
    else
    {
        static fs_pathname str;

        // Special case for a root directory
        if (dir->parent)
        {
            // Recursively generate the required path
            err = cache_dir_name(dir->parent, path, external, virtual);
        }
        else
        {
            cache_drive *drive = (cache_drive *) (((byte *) dir) - offsetof(cache_drive, root));

            // Construct the initial stub
            if (virtual)
            {
                if (external)
                {
                    sprintf(str, "%c%s%c%c%c", FS_CHAR_DISC, FS_NAME_DRIVE_ALL,
                            FS_CHAR_SEPARATOR, FS_CHAR_ROOT, FS_CHAR_SEPARATOR);
                }
                else
                {
                    sprintf(str, "%c%c%c%c%c", FS_CHAR_DISC, FS_CHAR_DRIVE_ALL,
                            FS_CHAR_SEPARATOR, FS_CHAR_ROOT, FS_CHAR_SEPARATOR);
                }
            }
            else sprintf(str, "%c", FS_CHAR_DISC);

            // Add the drive name
            if ((external || virtual) && drive->info.present
                && (1 < strlen(drive->info.name)))
            {
                strcat(str, drive->info.name);
            }
            else
            {
                sprintf(strchr(str, '\0'), "%c",
                        drive - cache_drive_array + 'A');
            }
        }

        // Add the leaf if not the root of a virtual path
        if (!err && (dir->parent || !virtual))
        {
            if (sizeof(str) <= (strlen(str) + strlen(dir->info.name) + 1))
            {
                err = &err_bad_name;
            }
            else
            {
                sprintf(strchr(str, '\0'), "%c%s",
                        FS_CHAR_SEPARATOR, dir->info.name);
            }
        }

        // Set the result pointer
        *path = !err ? str : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : parent        - The parent of the directory to search.
                  name          - The name to search for.
                  prev          - Variable to receive a pointer to the entry
                                  immediately preceding the first one after
                                  the specified name.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Attempt to find the specified directory entry, and return a
                  pointer to the requested entry (or the preceding entry if not
                  found).
*/
static os_error *cache_dir_prev(const cache_dir *parent, const char *name,
                                cache_dir **prev)
{
    os_error *err = NULL;

    // Check function parameters
    if (!parent || !name || !prev) err = &err_bad_parms;
    else
    {
        cache_dir *ptr = parent->dir.children;

        // Search the directory
        *prev = NULL;
        while (!err && ptr && (wildcard_cmp(ptr->info.name, name) <= 0))
        {
            // Advance to the next entry
            *prev = ptr;
            ptr = ptr->next;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : required      - Should the machine type be marked as required
                                  if not valid.
                  valid         - Variable to receive whether the machine type
                                  is valid.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the machine type details.
*/
static os_error *cache_find_machine(bool required, bool *valid)
{
    os_error *err = NULL;

    // Check function parameters
    if (required && !valid) err = &err_bad_parms;
    else
    {
        // Check if the machine type is known
        if (cache_machine_valid)
        {
             if (valid) *valid = TRUE;
             if (required) err = cache_machine_err;
        }
        else
        {
            if (valid) *valid = FALSE;
            if (required) cache_machine_required = TRUE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : required      - Should the owner information be marked as
                                  required if not valid.
                  valid         - Variable to receive whether the owner
                                  information is valid.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the owner information.
*/
static os_error *cache_find_owner(bool required, bool *valid)
{
    os_error *err = NULL;

    // Check function parameters
    if (required && !valid) err = &err_bad_parms;
    else
    {
        // Check if the machine type is known
        if (cache_owner_valid)
        {
             if (valid) *valid = TRUE;
             if (required) err = cache_owner_err;
        }
        else
        {
            if (valid) *valid = FALSE;
            if (required) cache_owner_required = TRUE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : required      - Should the power details be marked as required
                                  if not valid.
                  valid         - Variable to receive whether the power details
                                  are valid.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the power details.
*/
static os_error *cache_find_power(bool required, bool *valid)
{
    os_error *err = NULL;

    // Check function parameters
    if (required && !valid) err = &err_bad_parms;
    else
    {
        // Check if the power details are known
        if (cache_power_valid)
        {
             if (valid) *valid = TRUE;
             if (required) err = cache_power_err;
        }
        else
        {
            if (valid) *valid = FALSE;
            if (required) cache_power_required = TRUE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : drive         - The drive letter.
                  required      - Should the drive details be marked as
                                  required if not valid.
                  valid         - Variable to receive whether the drive details
                                  are currently valid.
                  info          - Variable to receive a pointer to the drive
                                  details.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the details for the specified drive.
*/
static os_error *cache_find_drive(char drive, bool required, bool *valid,
                                  cache_drive **info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!isalpha(drive) || (required && !valid) || !info) err = &err_bad_parms;
    else
    {
        // Set the details pointer
        *info = &cache_drive_array[toupper(drive) - 'A'];

        // Check if the drive details are valid
        if ((*info)->root.valid)
        {
             if (valid) *valid = TRUE;
             if (required) err = (*info)->root.err;
        }
        else
        {
            if (valid) *valid = FALSE;
            if (required) (*info)->root.required = TRUE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dir           - The directory entry details.
                  info          - Variable to receive the modified information.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Modify the entry details for the specified directory entry.
*/
static os_error *cache_dir_info(const cache_dir *dir, fs_info *info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!dir || !info) err = &err_bad_parms;
    else
    {
        // Copy the specified details
        *info = dir->info;

        // Overwrite some details if the file is open for update
        if (dir->open)
        {
            info->size = dir->open->info.allocated;
            info->load_addr = dir->open->load;
            info->exec_addr = dir->open->exec;
            info->attr = dir->open->attr;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The path of the directory entry to find.
                  required      - Should the directory entry details be marked
                                  as required if not valid.
                  valid         - Variable to receive whether the directory
                                  entry details are currently valid.
                  info          - Variable to receive a pointer to the directory
                                  entry details.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the details for the specified directory entry.
*/
static os_error *cache_find_dir_entry(const char *path, bool required,
                                      bool *valid, cache_dir **dir)
{
    os_error *err = NULL;

    // Check function parameters
    if (!path || (required && !valid) || !dir) err = &err_bad_parms;
    else
    {
        const char *parent;
        const char *leaf;
        bool entry;

        // Extract the leafname
        err = cache_dir_parent(path, &parent, &leaf);

        // Attempt to find the requested directory entry
        if (!err)
        {
            // Special case if the root directory required
            if (!leaf)
            {
                cache_drive *drive;

                // Root directory
                err = cache_find_drive(path[1], required, &entry, &drive);
                *dir = drive ? &drive->root : NULL;
                if (!err && required && (!drive || !drive->info.present))
                {
                    err = &err_drive_empty;
                }
            }
            else
            {
                // Attempt to find the parent directory
                err = cache_find_dir(parent, required, &entry, dir);

                // Find the required entry within the directory
                if (!err && *dir)
                {
                    *dir = (*dir)->dir.children;
                    while (*dir && wildcard_cmp((*dir)->info.name, leaf))
                    {
                        (*dir) = (*dir)->next;
                    }
                }
                else *dir = NULL;
            }
        }

        // No further action unless the directory entry has been found
        if (!err)
        {
            // Check if the directory entry details are valid
            if (!entry)
            {
                // Parent directory is not valid
                if (valid) *valid = FALSE;
                if (required) *dir = NULL;
            }
            else if (!*dir)
            {
                // Entry not found
                if (valid) *valid = FALSE;
                if (required) err = &err_not_found;
            }
            else if (!(*dir)->valid || (*dir)->required)
            {
                // Directory entry not valid
                if (valid) *valid = FALSE;
                if (required) (*dir)->required = TRUE;
            }
            else
            {
                // Directory entry is valid
                if (valid) *valid = TRUE;
                if (required) err = (*dir)->err;
            }
        }
        else
        {
            // Directory entry not found
            if (valid) *valid = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The path of the directory to find.
                  required      - Should the directory details be marked as
                                  required if not valid.
                  valid         - Variable to receive whether the directory
                                  details are currently valid.
                  info          - Variable to receive a pointer to the directory
                                  details.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the details for the specified directory.
*/
static os_error *cache_find_dir(const char *path, bool required, bool *valid,
                                cache_dir **dir)
{
    os_error *err = NULL;

    // Check function parameters
    if (!path || (required && !valid) || !dir) err = &err_bad_parms;
    else
    {
        bool entry;

        // Attempt to find the parent directory entry
        err = cache_find_dir_entry(path, required, &entry, dir);

        // No further action unless the entry has been found
        if (!err && *dir)
        {
            // Check if the directory details are valid
            if (!entry || ((*dir)->info.obj_type != fileswitch_IS_DIR))
            {
                // Not a valid directory
                if (valid) *valid = FALSE;
                if (required)
                {
                    *dir = NULL;
                    err = &err_not_found;
                }
            }
            else if (!(*dir)->dir.active)
            {
                // Subdirectory has not been activated
                if (valid) *valid = FALSE;
                if (required)
                {
                    (*dir)->dir.active = TRUE;
                    (*dir)->dir.required = TRUE;
                    (*dir)->dir.valid = FALSE;
                }
            }
            else if (!(*dir)->dir.valid)
            {
                // Subdirectory is not valid
                if (valid) *valid = FALSE;
                if (required) (*dir)->dir.required = TRUE;
            }
            else if ((*dir)->dir.err)
            {
                // Reading the subdirectory returned an error
                if (valid) *valid = TRUE;
                if (required) err = (*dir)->dir.err;
            }
            else
            {
                cache_dir *child = (*dir)->dir.children;

                // Check if the directory entries are valid
                if (valid) *valid = TRUE;
                while (!err && child)
                {
                    // Check this child
                    if (child->valid && !child->required)
                    {
                        if (required)
                        {
                            if (err) err = child->err;
                            else if ((child->info.obj_type == fileswitch_IS_DIR)
                                     && !child->dir.active)
                            {
                                child->dir.active = TRUE;
                                child->dir.required = FALSE;
                                child->dir.valid = FALSE;
                            }
                        }
                    }
                    else
                    {
                        if (valid) *valid = FALSE;
                        if (required) child->required = TRUE;
                    }

                    // Advance to the next child
                    child = child->next;
                }
            }
        }
        else
        {
            // Parent directory entry not found
            if (valid) *valid = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dir           - The directory entry to remove.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Unlink the specified directory entry and add it to the free
                  list. Any children are also added to the free list.
*/
static os_error *cache_dir_remove(cache_dir *dir)
{
    os_error *err = NULL;

    // Check function parameters
    if (!dir) err = &err_bad_parms;
    else
    {
        // Unlink any open file
        if (dir->open)
        {
            dir->open->dir = NULL;
            dir->open = NULL;
        }

        // Recursively remove any children
        while (!err && dir->dir.children)
        {
            // Remove the next child
            err = cache_dir_remove(dir->dir.children);
        }

        // Unlink from any siblings or parent
        if (dir->next) dir->next->prev = dir->prev;
        if (dir->prev) dir->prev->next = dir->next;
        else if (dir->parent) dir->parent->dir.children = dir->next;

        // Free the memory
        MEM_FREE(dir);
        /*
        // Add to the free list
        dir->next = cache_dir_free;
        dir->prev = NULL;
        dir->parent = NULL;
        if (cache_dir_free) cache_dir_free->prev = dir;
        cache_dir_free = dir;
        */
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : parent        - The parent directory.
                  info          - The details for the new entry.
                  dir           - Variable to receive a pointer to the new
                                  entry.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a new directory entry for the specified object and
                  insert it in the appropriate position.
*/
static os_error *cache_dir_add(cache_dir *parent, const fs_info *info,
                               cache_dir **dir)
{
    os_error *err = NULL;

    // Check function parameters
    if (!parent || !info) err = &err_bad_parms;
    else
    {
        cache_dir *ptr;

        // Obtain a directory entry pointer
        /*
        if (cache_dir_free)
        {
            // An entry is already available
            ptr = cache_dir_free;
            if (ptr->next) ptr->next->prev = NULL;
            cache_dir_free = ptr->next;
        }
        else
        */
        {
            // Allocate a new structure
            ptr = (cache_dir *) MEM_MALLOC(sizeof(cache_dir));
            if (!ptr) err = &err_buffer;
        }

        // Complete the details
        if (!err)
        {
            cache_dir *prev;

            // Fill in the details
            ptr->required = FALSE;
            ptr->valid = TRUE;
            ptr->err = NULL;
            ptr->info = *info;
            ptr->open = NULL;
            ptr->dir.active = FALSE;
            ptr->dir.required = FALSE;
            ptr->dir.valid = FALSE;
            ptr->dir.children = NULL;

            // Find the entry immediately before this
            err = cache_dir_prev(parent, info->name, &prev);

            // Link the details to the directory
            if (!err)
            {
                ptr->parent = parent;
                ptr->prev = prev;
                if (prev)
                {
                    ptr->next = prev->next;
                    if (prev->next) prev->next->prev = ptr;
                    prev->next = ptr;
                }
                else
                {
                    ptr->next = parent->dir.children;
                    parent->dir.children = ptr;
                }
            }

            // Add back to the free list if any error
            if (err)
            {
                MEM_FREE(ptr);
                /*
                ptr->next = cache_dir_free;
                ptr->prev = NULL;
                if (cache_dir_free) cache_dir_free->prev = ptr;
                cache_dir_free = ptr;
                */
            }
        }

        // Set the return parameters
        if (dir) *dir = !err ? ptr : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Invalidate all cached details.
*/
static os_error *cache_invalidate_all(void)
{
    os_error *err = NULL;
    int i;

    // Start by updating any relevant pollwords
    err = pollword_update(psifs_MASK_LINK_DRIVES);

    // Invalidate the machine type
    if (!err)
    {
        cache_machine_required = FALSE;
        cache_machine_valid = FALSE;
    }

    // Invalidate the owner information
    if (!err)
    {
        cache_owner_required = FALSE;
        cache_owner_valid = FALSE;
    }

    // Invalidate the power status
    if (!err)
    {
        cache_power_required = FALSE;
        cache_power_valid = FALSE;
    }

    // Invalidate the synchronized time
    if (!err) cache_sync_done = FALSE;

    // Invalidate all drive and directory details
    for (i = 0; !err && (i < 26); i++)
    {
        cache_drive *drive = &cache_drive_array[i];

        // Perform an UpCall if necessary
        if (!err && !drive->root.err && drive->info.present)
        {
            const char *path;

            err = cache_dir_name(&drive->root, &path, TRUE, TRUE);
            if (!err) err = upcall_removed(path, &drive->root.info);
        }

        // Invalidate this drive if successful
        if (!err)
        {
            // Invalidate the drive details
            drive->root.required = FALSE;
            drive->root.valid = FALSE;

            // Invalidate the root directory details
            drive->root.dir.active = FALSE;
            drive->root.dir.required = FALSE;
            drive->root.dir.valid = FALSE;
        }

        // Remove any directory entries
        while (!err && drive->root.dir.children)
        {
            err = cache_dir_remove(drive->root.dir.children);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : drive         - The drive details to update.
                  info          - The details read for the specified drive.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Update the specified drive details in response to a reply.
*/
static os_error *cache_next_drive_update(cache_drive *drive,
                                         const fs_drive *info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!drive || !info) err = &err_bad_parms;
    else
    {
        bool upcall = FALSE;
        const char *path;

        // Start by updating any relevant pollwords
        err = pollword_update(psifs_MASK_LINK_DRIVES);

        // Perform an UpCall if necessary
        if (!err && !drive->root.err && drive->info.present && info->present
            && ((drive->info.free_low != info->free_low)
                || (drive->info.free_high != info->free_high)
                || (drive->info.size_low != info->size_low)
                || (drive->info.size_high != info->size_high)))
        {
            err = cache_dir_name(&drive->root, &path, TRUE, FALSE);
            if (!err) err = upcall_added(path, &drive->root.info);
            if (!err) err = cache_dir_name(&drive->root, &path, TRUE, TRUE);
            if (!err) err = upcall_added(path, &drive->root.info);
        }
        else if (!err && (drive->root.err || !drive->info.present)
                 && info->present)
        {
            // Postpone the UpCall until after the update
            upcall = TRUE;
        }

        // Update the details if successful
        if (!err) drive->info = *info;

        // Perform another UpCall if just added
        if (!err && !drive->root.err && drive->info.present && upcall)
        {
            err = cache_dir_name(&drive->root, &path, TRUE, TRUE);
            if (!err) err = upcall_added(path, &drive->root.info);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dir           - The directory entry to remove.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Remove the specified directory entry in response to a reply.
*/
static os_error *cache_next_dir_remove(cache_dir *dir)
{
    os_error *err = NULL;

    // Check function parameters
    if (!dir) err = &err_bad_parms;
    else
    {
        // Perform an UpCall if necessary
        if (dir->valid && !dir->err)
        {
            const char *path;

            err = cache_dir_name(dir, &path, TRUE, FALSE);
            if (!err) err = upcall_removed(path, &dir->info);
            if (!err) err = cache_dir_name(dir, &path, TRUE, TRUE);
            if (!err) err = upcall_removed(path, &dir->info);
        }

        // Remove the specified entry
        err = cache_dir_remove(dir);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dir           - The directory to contain the new entry.
                  err           - Any error produced by the operation.
                  info          - The details read for the specified directory
                                  entry.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Add the specified directory entry in response to a reply.
*/
static os_error *cache_next_dir_add(cache_dir *dir, const fs_info *info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!dir) err = &err_bad_parms;
    else
    {
        cache_dir *ptr;

        // Add the specified entry
        err = cache_dir_add(dir, info, &ptr);

        // Perform an UpCall if necessary
        if (dir->valid && !dir->err)
        {
            const char *path;

            err = cache_dir_name(ptr, &path, TRUE, FALSE);
            if (!err) err = upcall_added(path, info);
            if (!err) err = cache_dir_name(ptr, &path, TRUE, TRUE);
            if (!err) err = upcall_added(path, info);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dir           - The directory entry to update.
                  err           - Any error produced by the operation.
                  info          - The details read for the specified directory
                                  entry.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Update the specified directory entry in response to a reply.
*/
static os_error *cache_next_dir_update(cache_dir *dir, os_error *err,
                                       const fs_info *info)
{
    // Check function parameters
    if (!dir || !info) err = &err_bad_parms;
    else
    {
        // Check whether the type of object has changed
        if (!err
            && (((dir->dir.active) && (info->obj_type != fileswitch_IS_DIR))
                || (dir->valid && !dir->err
                    && ((dir->info.obj_type) != (info->obj_type)))))
        {
            cache_dir *parent = dir->parent;

            // Delete and recreate
            err = cache_next_dir_remove(dir);
            if (!err) err = cache_next_dir_add(parent, info);
        }
        else
        {
            // Perform an UpCall if necessary
            if (dir->valid && !dir->err && !err
                && ((dir->info.load_addr != info->load_addr)
                    || (dir->info.exec_addr != info->exec_addr)
                    || (dir->info.size != info->size)
                    || (dir->info.attr != info->attr)
                    || strcmp(dir->info.name, info->name)))
            {
                const char *path;

                err = cache_dir_name(dir, &path, TRUE, FALSE);
                if (!err) err = upcall_changed(path, info);
                if (!err) err = cache_dir_name(dir, &path, TRUE, TRUE);
                if (!err) err = upcall_changed(path, info);
            }

            // Update the details
            dir->required = FALSE;
            dir->valid = cache_active;
            dir->err = err;
            dir->info = *info;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a background operation.
*/
static os_error *cache_next_callback(void *user, os_error *err,
                                     const void *reply)
{
    // Check function parameters
    if (!err && (reply != &cache_next_reply)) err = &err_bad_parms;
    else
    {
        cache_dir *dir = NULL;
        cache_drive *drive = NULL;

        // Clear the active flag
        cache_next_active = FALSE;

        // Reset the time of the last operation
        cache_delay_time = util_time();

        // Decode the result of the operation
        switch (cache_next_cmd.op)
        {
            case UNIFIED_DRIVE:
                // Read drive details
                DEBUG_PRINTF(("Drive '%c'", cache_next_cmd.data.drive.drive))
                DEBUG_ERR(err)
                drive = &cache_drive_array[cache_next_cmd.data.drive.drive - 'A'];
                drive->root.required = FALSE;
                drive->root.valid = cache_active;
                drive->root.err = err;
                drive->refresh = util_time()
                                 + (!err && drive->root.valid
                                    && cache_next_reply.drive.drive.present
                                    ? CACHE_DRIVE_TIMEOUT_ACTIVE
                                    : CACHE_DRIVE_TIMEOUT_INACTIVE);
                if (!err)
                {
                    err = cache_next_drive_update(drive, &cache_next_reply.drive.drive);
                    if (drive->root.valid && drive->info.present
                        && !drive->root.dir.active)
                    {
                        // Activate the root directory
                        drive->root.parent = NULL;
                        drive->root.next = NULL;
                        drive->root.prev = NULL;
                        drive->root.info.load_addr = 0;
                        drive->root.info.exec_addr = 0;
                        drive->root.info.size = 0;
                        drive->root.info.attr = 0;
                        drive->root.info.obj_type = fileswitch_IS_DIR;
                        drive->root.info.name[0] = FS_CHAR_ROOT;
                        drive->root.info.name[1] = '\0';
                        drive->root.dir.active = TRUE;
                        drive->root.dir.required = FALSE;
                        drive->root.dir.valid = FALSE;
                    }
                }
                if (!drive->root.valid || drive->root.err
                    || !drive->info.present)
                {
                    // Deactivate the root directory
                    drive->root.info.obj_type = fileswitch_NOT_FOUND;
                    drive->root.dir.active = FALSE;
                    drive->root.dir.required = FALSE;
                    drive->root.dir.valid = FALSE;

                    // Remove any directory entries
                    while (!err && drive->root.dir.children)
                    {
                        err = cache_dir_remove(drive->root.dir.children);
                    }
                }
                break;

            case UNIFIED_LIST:
                // Read directory listing
                DEBUG_PRINTF(("List '%s'", cache_next_cmd.data.list.path))
                DEBUG_ERR(err)
                if (err && ERR_EQ(*err, err_rfsv_too_many))
                {
                    // Enlarge the buffer ready to try again
                    err = cache_buffer_grow();
                }
                else if (!cache_find_dir(cache_next_cmd.data.list.path,
                                         FALSE, NULL, &dir)
                         && dir)
                {
                    // Update the parent entry details
                    dir->dir.required = FALSE;
                    dir->dir.valid = cache_active;
                    dir->dir.err = err;
                    dir->dir.refresh = util_time() + CACHE_DIR_TIMEOUT;

                    // Invalidate the parent entry if an error was returned
                    if (err) dir->valid = FALSE;
                    else
                    {
                        const fs_info *from = cache_next_cmd.data.list.buffer;
                        cache_dir *to = dir->dir.children;

                        // Update the directory listing
                        while (!err
                               && ((from < cache_next_reply.list.next) || to))
                        {
                            int cmp;

                            // Compare the two entries
                            if (!to) cmp = -1;
                            else if (from < cache_next_reply.list.next)
                            {
                                cmp = wildcard_cmp(from->name, to->info.name);
                            }
                            else cmp = 1;

                            // Action depends on the relative position
                            if (cmp < 0)
                            {
                                // Create a new directory entry
                                err = cache_next_dir_add(dir, from);

                                // Increment the source pointer
                                from++;
                            }
                            else if (0 < cmp)
                            {
                                cache_dir *next = to->next;

                                // Delete the current entry
                                err = cache_next_dir_remove(to);

                                // Advance to the next entry
                                to = next;
                            }
                            else
                            {
                                // Check for changes to the existing entry
                                err = cache_next_dir_update(to, err, from);

                                // Increment both pointers
                                from++;
                                to = to->next;
                            }
                        }
                    }
                }
                break;

            case UNIFIED_INFO:
                // Read details for a single object
                DEBUG_PRINTF(("Info '%s'", cache_next_cmd.data.info.path))
                DEBUG_ERR(err)
                if (!cache_find_dir_entry(cache_next_cmd.data.info.path,
                                          FALSE, NULL, &dir)
                    && dir)
                {
                    // Check if the directory entry was found
                    if (err && ERR_EQ(*err, err_not_found))
                    {
                        // Remove the directory entry
                        err = cache_next_dir_remove(dir);
                    }
                    else
                    {
                        // Invalidate the parent if an error returned
                        if (err && dir->parent) dir->parent->valid = FALSE;

                        // Update the directory entry details
                        err = cache_next_dir_update(dir, err, &cache_next_reply.info.info);
                    }
                }
                break;

            case UNIFIED_MACHINE:
                // Read machine type
                cache_machine_required = FALSE;
                cache_machine_valid = cache_active;
                cache_machine_err = err;
                if (!err)
                {
                    cache_machine_type = cache_next_reply.machine.type;
                    strcpy(cache_machine_name, cache_next_reply.machine.name);
                    cache_machine_id = cache_next_reply.machine.id;
                    cache_machine_language = cache_next_reply.machine.language;
                    cache_machine_version = cache_next_reply.machine.version;
                }
                break;

            case UNIFIED_OWNER:
                // Read owner information
                cache_owner_required = FALSE;
                cache_owner_valid = cache_active;
                cache_owner_err = err;
                if (!err) strcpy(cache_owner_info, cache_next_reply.owner.info);
                break;

            case UNIFIED_POWER:
                // Read power details
                cache_power_required = FALSE;
                cache_power_valid = cache_active;
                cache_power_err = err;
                cache_power_refresh = util_time() + CACHE_POWER_TIMEOUT;
                if (!err)
                {
                    cache_power_main = cache_next_reply.power.main;
                    cache_power_backup = cache_next_reply.power.backup;
                    cache_power_external = cache_next_reply.power.external;
                }
                break;

            case UNIFIED_WTIME:
                // Write time
                cache_sync_done = cache_active;
                cache_sync_err = err;
                break;

            default:
                // Not a supported command
                if (!err) err = &err_bad_cache_op;
                break;
        }

        // Perform another operation
        if (!err) err = cache_process();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : priority  - The priority of this operation.
                  refresh   - The time of the next scheduled refresh.
    Returns     : bool      - Is this the highest priority so far.
    Description : Compare the specified priority to the highest so far.
*/
static bool cache_next_compare(cache_priority priority, os_t refresh)
{
    bool higher = FALSE;
    int relative = refresh - cache_next_time;

    // Compare to the highest priority so far
    if ((cache_next_priority < priority)
        || ((cache_next_priority == priority)
            && (relative < cache_next_refresh)))
    {
        // Set this as the highest priority so far
        higher = TRUE;
        cache_next_priority = priority;
        cache_next_refresh = relative;
    }

    // Return the result
    return higher;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check if the machine type needs updating.
*/
static os_error *cache_next_machine(void)
{
    os_error *err = NULL;
    cache_priority priority;

    // Choose the priority
    if (cache_machine_required) priority = CACHE_PRIORITY_REQUIRED;
    else if (!cache_machine_valid) priority = CACHE_PRIORITY_REQUIRED;
    else priority = CACHE_PRIORITY_NONE;

    // Check if highest priority
    if (cache_next_compare(priority, cache_next_time))
    {
        // Build a possible command
        cache_next_cmd.op = UNIFIED_MACHINE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check if the owner information needs updating.
*/
static os_error *cache_next_owner(void)
{
    os_error *err = NULL;
    cache_priority priority;

    // Choose the priority
    if (cache_owner_required) priority = CACHE_PRIORITY_REQUIRED;
    else if (!cache_owner_valid) priority = CACHE_PRIORITY_INVALID;
    else priority = CACHE_PRIORITY_NONE;

    // Check if highest priority
    if (cache_next_compare(priority, cache_next_time))
    {
        // Build a possible command
        cache_next_cmd.op = UNIFIED_OWNER;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check if the power details need updating.
*/
static os_error *cache_next_power(void)
{
    os_error *err = NULL;
    cache_priority priority;

    // Choose the priority
    if (cache_power_required) priority = CACHE_PRIORITY_REQUIRED;
    else if (!cache_power_valid) priority = CACHE_PRIORITY_INVALID;
    else priority = CACHE_PRIORITY_REFRESH;

    // Check if highest priority
    if (cache_next_compare(priority, cache_power_refresh))
    {
        // Build a possible command
        cache_next_cmd.op = UNIFIED_POWER;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check if the time needs synchronizing.
*/
static os_error *cache_next_sync(void)
{
    os_error *err = NULL;
    cache_priority priority;

    // Choose the priority
    if (!cache_sync_done && cache_sync) priority = CACHE_PRIORITY_REQUIRED;
    else priority = CACHE_PRIORITY_NONE;

    // Check if highest priority
    if (cache_next_compare(priority, cache_next_time))
    {
        static oswordreadclock_utc_block now;

        // Build a possible command
        now.op = oswordreadclock_OP_UTC;
        err = xoswordreadclock_utc(&now);
        if (!err)
        {
            cache_next_cmd.op = UNIFIED_WTIME;
            cache_next_cmd.data.wtime.date = *(date_riscos *) &now;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check if any of the drive details need updating.
*/
static os_error *cache_next_drive(void)
{
    os_error *err = NULL;
    int i;

    // Check each drive in sequence
    for (i = 0; !err && (i < 26); i++)
    {
        cache_drive *drive = &cache_drive_array[i];
        cache_priority priority;

        // Choose the priority
        if (drive->root.required) priority = CACHE_PRIORITY_REQUIRED;
        else if (!drive->root.valid) priority = CACHE_PRIORITY_INVALID;
        else priority = CACHE_PRIORITY_REFRESH;

        // Check if highest priority
        if (cache_next_compare(priority, drive->refresh))
        {
            // Build a possible command
            cache_next_cmd.op = UNIFIED_DRIVE;
            cache_next_cmd.data.drive.drive = i + 'A';
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dir           - The directory entry to check.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check whether the specified directory (or any subdirectories)
                  need updating.
*/
static os_error *cache_next_dir_recurse(cache_dir *dir)
{
    os_error *err = NULL;
    cache_priority priority;

    // First check the object if it is not the root directory
    if (dir->parent)
    {
        // Choose the priority
        if (dir->required) priority = CACHE_PRIORITY_REQUIRED;
        else if (!dir->valid) priority = CACHE_PRIORITY_INVALID;
        else priority = CACHE_PRIORITY_NONE;

        // Check if highest priority
        if (cache_next_compare(priority, cache_next_time))
        {
            const char *ptr;

            // Build a possible command
            err = cache_dir_name(dir, &ptr, FALSE, FALSE);
            if (!err && (sizeof(cache_next_cmd.data.info.path) <= strlen(ptr)))
            {
                err = &err_bad_name;
            }
            if (!err)
            {
                cache_next_cmd.op = UNIFIED_INFO;
                strcpy(cache_next_cmd.data.info.path, ptr);
            }
        }
    }

    // Extra checks if the subdirectory is active
    if (!err && (dir->info.obj_type == fileswitch_IS_DIR) && dir->dir.active)
    {
        // Choose the priority
        if (dir->dir.required) priority = CACHE_PRIORITY_REQUIRED;
        else if (!dir->dir.valid) priority = CACHE_PRIORITY_INVALID;
        else priority = CACHE_PRIORITY_REFRESH;

        // Check if highest priority
        if (cache_next_compare(priority, dir->dir.refresh))
        {
            const char *ptr;

            // Build a possible command
            err = cache_dir_name(dir, &ptr, FALSE, FALSE);
            if (!err && (sizeof(cache_next_cmd.data.list.path) <= strlen(ptr)))
            {
                err = &err_bad_name;
            }
            if (!err && !cache_buffer) err = cache_buffer_grow();
            if (!err)
            {
                cache_next_cmd.op = UNIFIED_LIST;
                strcpy(cache_next_cmd.data.list.path, ptr);
                cache_next_cmd.data.list.buffer = cache_buffer;
                cache_next_cmd.data.list.size = cache_buffer_size;
            }
        }

        // Recurse through all children if valid
        if (dir->dir.valid && !dir->dir.err)
        {
            dir = dir->dir.children;
            while (!err && dir)
            {
                // Check this entry
                err = cache_next_dir_recurse(dir);

                // Advance to the next entry
                dir = dir->next;
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check if any of the directory details need updating.
*/
static os_error *cache_next_dir(void)
{
    os_error *err = NULL;
    int i;

    // Check each drive in sequence
    for (i = 0; !err && (i < 26); i++)
    {
        cache_drive *drive = &cache_drive_array[i];

        // Check this directory if the drive is valid
        if (drive->root.valid && !drive->root.err && drive->info.present)
        {
            err = cache_next_dir_recurse(&drive->root);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Choose an operation to perform.
*/
static os_error *cache_next_start(void)
{
    os_error *err = NULL;

    // Reset the highest priority
    cache_next_priority = CACHE_PRIORITY_NONE;
    cache_next_time = util_time();
    cache_next_refresh = 0;

    // Update the last time that the link was busy
    if (cache_pending_head || !cache_next_busy)
    {
        cache_next_busy = cache_next_time;
    }

    // No action if not connected or already active
    if (cache_active && !cache_next_active)
    {
        // Check whether the time needs to be synchronized
        err = cache_next_sync();

        // Check whether the drive details need updating
        if (!err) err = cache_next_drive();

        // Check whether the machine type needs updating
        if (!err) err = cache_next_machine();

        // Check whether the power details need updating
        if (!err) err = cache_next_power();

        // Check whether the owner information needs updating
        if (!err) err = cache_next_owner();

        // Check whether the directory details need updating
        if (!err) err = cache_next_dir();

        // Ignore refresh requests if disabled
        if (cache_disable && (cache_next_priority == CACHE_PRIORITY_REFRESH))
        {
            cache_next_priority = CACHE_PRIORITY_NONE;
        }

#ifdef CACHE_FOREGROUND

        // Also ignore refresh requests during testing
        if (cache_next_priority == CACHE_PRIORITY_REFRESH)
        {
            cache_next_priority = CACHE_PRIORITY_NONE;
        }

#endif

        // Some operations are disabled if the link is busy
        if (((cache_next_priority == CACHE_PRIORITY_REFRESH)
             && ((cache_next_time - cache_next_busy) < CACHE_REFRESH_DELAY))
            || ((cache_next_priority == CACHE_PRIORITY_INVALID)
                && ((cache_next_time - cache_next_busy) < CACHE_INVALID_DELAY)))
        {
            cache_next_priority = CACHE_PRIORITY_NONE;
        }

        // Some operations are also disabled if soon after previous operation
        if (idle_check_throttle()
            && ((((cache_next_priority == CACHE_PRIORITY_REFRESH)
                  ||(cache_next_priority == CACHE_PRIORITY_INVALID))
                 && ((cache_next_time - cache_delay_time)
                     < CACHE_BACK_DELAY))
                || ((cache_next_priority == CACHE_PRIORITY_REQUIRED)
                    && ((cache_next_time - cache_delay_time)
                        < CACHE_FORE_DELAY))))
        {
            cache_next_priority = CACHE_PRIORITY_NONE;
        }

        // Start an operation if any required
        if (!err && (cache_next_priority != CACHE_PRIORITY_NONE))
        {
            // Start the selected operation
            cache_next_active = TRUE;
            err = unified_back(&cache_next_cmd, &cache_next_reply, NULL,
                               cache_next_callback);
            if (err) cache_next_active = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a background operation.
*/
static os_error *cache_op_callback(void *user, os_error *err, const void *reply)
{
    // Check function parameters
    if (!err && (reply != &cache_next_reply)) err = &err_bad_parms;
    else
    {
        // Clear the operation active flag
        cache_next_active = FALSE;

        // Reset the time of the last operation
        cache_delay_time = util_time();

        // Update the reply status
        if (cache_pending_cmd)
        {
            cache_pending_cmd = FALSE;
            cache_pending_err = err;
            cache_pending_reply = BOOL(reply);
        }

        // Perform any necessary processing
        err = cache_process();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation requesting the operation.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start a background operation.
*/
static os_error *cache_op_back(cache_pending *op)
{
    os_error *err = NULL;

    // Check function parameters
    if (!op) err = &err_bad_parms;
    else
    {
        // Ensure that an operation can be performed
        if (!cache_active) err = &err_cache_inactive;
        else if (cache_next_active) err = &err_cache_busy;

        // Attempt to start the operation
        if (!err)
        {
            cache_next_active = TRUE;
            cache_pending_cmd = TRUE;
            cache_pending_err = NULL;
            cache_pending_reply = TRUE;
            err = unified_back(&cache_next_cmd, &cache_next_reply, NULL,
                               cache_op_callback);
            if (err)
            {
                cache_next_active = FALSE;
                cache_pending_cmd = FALSE;
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a drive request.
*/
static os_error *cache_op_drive(cache_pending *op, os_error *err,
                                bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        cache_drive *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Attempt to find the requested details
        if (!err)
        {
            err = cache_find_drive(op->cmd->data.drive.drive, TRUE,
                                   done, &info);
        }
        if (!err && *done) op->reply->drive.drive = info->info;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a name disc request.
*/
static os_error *cache_op_name(cache_pending *op, os_error *err,
                               bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        cache_drive *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Attempt to find the specified details
        if (!err)
        {
            err = cache_find_drive(op->cmd->data.drive.drive, TRUE,
                                   done, &info);
        }

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!info->info.present)
        {
            // Disc not present
            err = &err_drive_empty;
        }
        else if (reply)
        {
            // The name should have been changed
            info->root.valid = FALSE;
        }
        else if (idle)
        {
            // Attempt to change the name
            if (sizeof(cache_next_cmd.data.name.name)
                <= strlen(op->cmd->data.name.name))
            {
                 err = &err_bad_name;
            }
            else
            {
                cache_next_cmd.op = UNIFIED_NAME;
                cache_next_cmd.data.name.drive = op->cmd->data.name.drive;
                strcpy(cache_next_cmd.data.name.name,
                       op->cmd->data.name.name);
                err = cache_op_back(op);
            }
            if (!err) *done = FALSE;
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of an enumerate request.
*/
static os_error *cache_op_enumerate(cache_pending *op, os_error *err,
                                    bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        cache_dir *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Attempt to find the requested details
        if (!err)
        {
            err = cache_find_dir(op->cmd->data.enumerate.path, TRUE,
                                 done, &info);
        }
        if (!err && *done)
        {
            // Start from the first directory entry
            info = info->dir.children;
            op->reply->enumerate.offset = 0;
            op->reply->enumerate.read = 0;

            // Copy the requested number of matching entries
            while (!err && info && (op->reply->enumerate.read
                                    < op->cmd->data.enumerate.size))
            {
                const char any[] = {FS_CHAR_WILD_ANY, '\0'};

                // Check if the next entry matches
                if (!wildcard_cmp(*op->cmd->data.enumerate.match
                                  ? op->cmd->data.enumerate.match : any,
                                  info->info.name))
                {
                    // Copy the entry if required
                    if (op->cmd->data.enumerate.offset
                        <= op->reply->enumerate.offset)
                    {
                        err = cache_dir_info(info, &op->cmd->data.enumerate.buffer[op->reply->enumerate.read++]);
                    }

                    // Increment the offset
                    op->reply->enumerate.offset++;
                }

                // Advance to the next directory entry
                info = info->next;
            }
            if (!info) op->reply->enumerate.offset = -1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of an info request.
*/
static os_error *cache_op_info(cache_pending *op, os_error *err,
                               bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        cache_dir *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Attempt to find the requested details
        if (!err)
        {
            err = cache_find_dir_entry(op->cmd->data.info.path, TRUE,
                                       done, &info);
        }

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else
        {
            // Process the results
            err = cache_dir_info(info, &op->reply->info.info);
        }

        // Special case if not found
        if (err && ERR_EQ(*err, err_not_found))
        {
            err = NULL;
            *done = TRUE;
            op->reply->info.info.load_addr = 0;
            op->reply->info.info.exec_addr = 0;
            op->reply->info.info.size = 0;
            op->reply->info.info.attr = 0;
            op->reply->info.info.obj_type = fileswitch_NOT_FOUND;
            *op->reply->info.info.name = '\0';
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a make directory request.
*/
static os_error *cache_op_mkdir(cache_pending *op, os_error *err,
                                bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        const char *parent;
        const char *leaf;
        cache_dir *dir;
        cache_dir *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Split off the leaf name
        if (!err)
        {
            err = cache_dir_parent(op->cmd->data.mkdir.path, &parent, &leaf);
        }

        // Read the details for the parent directory
        if (!err) err = cache_find_dir(parent, TRUE, done, &dir);

        // Attempt to read details for the specified entry
        if (!err)
        {
            // Special case if root directory
            if (!leaf) info = dir;
            else
            {
                err = cache_find_dir_entry(op->cmd->data.mkdir.path, FALSE,
                                           NULL, &info);
            }
        }

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (info)
        {
            // An object with the specified name already exists
            if (info->info.obj_type != fileswitch_IS_DIR) err = &err_exists;
        }
        else if (reply)
        {
            fs_info detail;

            // The directory should have been created
            if (sizeof(detail.name) <= strlen(leaf)) err = &err_bad_name;
            else
            {
                // Add a new entry to the cache
                strcpy(detail.name, leaf);
                detail.obj_type = fileswitch_IS_DIR;
                err = cache_dir_add(dir, &detail, &info);
            }

            // Invalidate the directory entry
            if (!err)
            {
                info->valid = FALSE;
                *done = FALSE;
            }
        }
        else if (op->state == CACHE_PENDING_STATE_DONE)
        {
            // Created directory not found
            err = &err_not_found;
        }
        else if (idle)
        {
            // Attempt to create the directory
            if (sizeof(cache_next_cmd.data.mkdir.path)
                <= strlen(op->cmd->data.mkdir.path))
            {
                 err = &err_bad_name;
            }
            else
            {
                cache_next_cmd.op = UNIFIED_MKDIR;
                strcpy(cache_next_cmd.data.mkdir.path,
                       op->cmd->data.mkdir.path);
                err = cache_op_back(op);
            }
            if (!err)
            {
                op->state = CACHE_PENDING_STATE_DONE;
                *done = FALSE;
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a remove request.
*/
static os_error *cache_op_remove(cache_pending *op, os_error *err,
                                 bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        cache_dir *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Read the details for the specified object
        if (!err)
        {
            err = cache_find_dir_entry(op->cmd->data.remove.path, TRUE, done,
                                       &info);
        }

        // If a directory then check if any children
        if (!err && *done && (info->info.obj_type == fileswitch_IS_DIR))
        {
            err = cache_find_dir(op->cmd->data.remove.path, TRUE, done, &info);
        }

        // Take appropriate action
        if (err && ERR_EQ(*err, err_not_found) && !reply)
        {
            // Object has either been deleted or does not exist
            err = NULL;
            *done = TRUE;
        }
        else if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (info->dir.children)
        {
            // Directory is not empty
            err = &err_dir_not_empty;
        }
        else if (info->open)
        {
            // Object is open
            err = &err_open;
        }
        else if (op->state == CACHE_PENDING_STATE_DONE)
        {
            // Object not deleted
            err = &err_locked;
        }
        else if (idle)
        {
            // Attempt to delete the object
            if (info->info.obj_type == fileswitch_IS_DIR)
            {
                // Delete a directory
                if (sizeof(cache_next_cmd.data.rmdir.path)
                    <= strlen(op->cmd->data.remove.path))
                {
                     err = &err_bad_name;
                }
                else
                {
                    cache_next_cmd.op = UNIFIED_RMDIR;
                    strcpy(cache_next_cmd.data.rmdir.path,
                           op->cmd->data.remove.path);
                    err = cache_op_back(op);
                }
            }
            else
            {
                // Delete a file
                if (sizeof(cache_next_cmd.data.remove.path)
                    <= strlen(op->cmd->data.remove.path))
                {
                     err = &err_bad_name;
                }
                else
                {
                    cache_next_cmd.op = UNIFIED_REMOVE;
                    strcpy(cache_next_cmd.data.remove.path,
                           op->cmd->data.remove.path);
                    err = cache_op_back(op);
                }
            }
            if (!err)
            {
                info->valid = FALSE;
                op->state = CACHE_PENDING_STATE_DONE;
                *done = FALSE;
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a rename request.
*/
static os_error *cache_op_rename(cache_pending *op, os_error *err,
                                 bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        const char *parent;
        const char *leaf;
        cache_dir *src;
        cache_dir *dir;
        cache_dir *dest;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Source and destination must be on the same drive
        if (!err
            && (op->cmd->data.rename.src[1] != op->cmd->data.rename.dest[1]))
        {
            err = &err_bad_rename;
        }

        // Split off the source leaf name
        if (!err)
        {
            err = cache_dir_parent(op->cmd->data.rename.src, &parent, &leaf);
        }
        if (!err && !leaf) err = &err_bad_rename;

        // Read the details for the source object
        if (!err)
        {
            err = cache_find_dir_entry(op->cmd->data.rename.src, FALSE, NULL,
                                       &src);
        }

        // Split off the destination leaf name
        if (!err)
        {
            err = cache_dir_parent(op->cmd->data.rename.dest, &parent, &leaf);
        }
        if (!err && !leaf) err = &err_bad_rename;

        // Read the details for the destination parent directory
        if (!err) err = cache_find_dir(parent, TRUE, done, &dir);

        // Read the details for the destination object
        if (!err)
        {
            err = cache_find_dir_entry(op->cmd->data.rename.dest, FALSE, NULL,
                                       &dest);
        }

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (reply)
        {
            // Create a new destination directory entry if required
            if (src && !dest)
            {
                fs_info detail;

                // Create a copy of the source entry with the new leaf name
                if (sizeof(detail.name) <= strlen(leaf)) err = &err_bad_name;
                else
                {
                    detail = src->info;
                    strcpy(detail.name, leaf);
                    err = cache_dir_add(dir, &detail, &dest);
                }

                // Move any subdirectory and open details
                if (!err)
                {
                    cache_dir *ptr;

                    // Move and relink open file details
                    dest->open = src->open;
                    src->open = NULL;
                    if (dest->open) dest->open->dir = dest;

                    // Move and relink subdirectory details
                    dest->dir = src->dir;
                    src->dir.active = FALSE;
                    src->dir.required = FALSE;
                    src->dir.valid = FALSE;
                    src->dir.children = NULL;
                    for (ptr = dest->dir.children; ptr; ptr = ptr->next)
                    {
                        ptr->parent = dest;
                    }
                }
            }

            // Invalidate the directory entries
            if (src) src->valid = FALSE;
            if (dest) dest->valid = FALSE;
            *done = FALSE;
        }
        else if (op->state == CACHE_PENDING_STATE_DONE)
        {
            // The rename should have taken place
            if (!dest || (src && (src != dest))) err = &err_bad_rename;
        }
        else if (!src)
        {
            // Source object not found
            err = &err_not_found;
        }
        else if (dest && (dest != src))
        {
            // Destination object already exists
            err = &err_exists;
        }
        else if (idle)
        {
            // Attempt to rename the object
            cache_next_cmd.op = UNIFIED_RENAME;
            strcpy(cache_next_cmd.data.rename.src, op->cmd->data.rename.src);
            strcpy(cache_next_cmd.data.rename.dest, op->cmd->data.rename.dest);
            err = cache_op_back(op);
            if (!err)
            {
                op->state = CACHE_PENDING_STATE_DONE;
                *done = FALSE;
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of an access request.
*/
static os_error *cache_op_access(cache_pending *op, os_error *err,
                                 bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        cache_dir *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Read the details for the specified object
        if (!err)
        {
            err = cache_find_dir_entry(op->cmd->data.access.path, TRUE, done,
                                       &info);
        }

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (info->open)
        {
            // Delay until after the file has been closed
            info->open->access = TRUE;
            info->open->attr = op->cmd->data.access.attr;
        }
        else if (info->info.attr == op->cmd->data.access.attr)
        {
            // No action if the attributes are already correct
        }
        else if (reply)
        {
            // The attributes should have been changed
            info->valid = FALSE;
        }
        else if (idle)
        {
            // Attempt to change the attributes
            if (sizeof(cache_next_cmd.data.access.path)
                <= strlen(op->cmd->data.access.path))
            {
                 err = &err_bad_name;
            }
            else
            {
                cache_next_cmd.op = UNIFIED_ACCESS;
                strcpy(cache_next_cmd.data.access.path,
                       op->cmd->data.access.path);
                cache_next_cmd.data.access.attr = op->cmd->data.access.attr;
                err = cache_op_back(op);
            }
            if (!err) *done = FALSE;
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a stamp request.
*/
static os_error *cache_op_stamp(cache_pending *op, os_error *err,
                                bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        cache_dir *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Read the details for the specified object
        if (!err)
        {
            err = cache_find_dir_entry(op->cmd->data.stamp.path, TRUE, done,
                                       &info);
        }

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (info->open)
        {
            // Delay until after the file has been closed
            info->open->stamp = TRUE;
            info->open->load = op->cmd->data.stamp.load;
            info->open->exec = op->cmd->data.stamp.exec;
        }
        else if (reply)
        {
            // The attributes should have been changed
            info->valid = FALSE;
        }
        else if (idle)
        {
            // Attempt to change the modification date
            if (sizeof(cache_next_cmd.data.stamp.path)
                <= strlen(op->cmd->data.stamp.path))
            {
                 err = &err_bad_name;
            }
            else
            {
                cache_next_cmd.op = UNIFIED_STAMP;
                strcpy(cache_next_cmd.data.stamp.path,
                       op->cmd->data.stamp.path);
                cache_next_cmd.data.stamp.date.words.high = op->cmd->data.stamp.load & 0xff;
                cache_next_cmd.data.stamp.date.words.low = op->cmd->data.stamp.exec;
                err = cache_op_back(op);
            }
            if (!err) *done = FALSE;
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of an open request.
*/
static os_error *cache_op_open(cache_pending *op, os_error *err,
                               bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = FS_NONE;
        const char *parent;
        const char *leaf;
        cache_dir *dir;
        cache_dir *info;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Allocate a file handle if not already done
        if (op->state == CACHE_PENDING_STATE_INITIAL)
        {
            union
            {
                oswordreadclock_utc_block utc;
                date_riscos date;
            } utc;

            // Obtain a suitable file handle
            if (!err)
            {
                /*
                if (cache_handle_free)
                {
                    // Unlink handle from the free list
                    handle = cache_handle_free;
                    if (handle->next) handle->next->prev = NULL;
                    cache_handle_free = handle->next;
                }
                else
                */
                {
                    // Allocate a new structure
                    handle = (fs_handle) MEM_MALLOC(sizeof(cache_file));
                    if (!handle) err = &err_buffer;
                }
            }
            if (!err)
            {
                // Read the current time
                utc.utc.op = oswordreadclock_OP_UTC;
                err = xoswordreadclock_utc(&utc.utc);
            }
            if (!err)
            {
                // Update the status
                op->state = CACHE_PENDING_STATE_DELETE;

                // Initialise the main fields of the handle
                handle->info.handle = op->cmd->data.open.handle;
                handle->info.info = 0;
                handle->info.extent = 0;
                handle->info.allocated = 0;
                handle->info.sequential = 0;
                handle->stamp = FALSE;
                handle->load = (utc.date.words.high & 0xff)
                               | (osfile_TYPE_DATA << osfile_FILE_TYPE_SHIFT)
                               | 0xfff00000;
                handle->exec = utc.date.words.low;
                handle->access = FALSE;
                handle->attr = fileswitch_ATTR_OWNER_READ
                               | fileswitch_ATTR_OWNER_WRITE
                               | fileswitch_ATTR_WORLD_WRITE;
                handle->sequential = 0;
            }
        }
        else handle = op->reply->open.handle;

        // Split off the leaf name
        if (!err)
        {
            err = cache_dir_parent(op->cmd->data.open.path, &parent, &leaf);
        }

        // Read the details for the parent directory
        if (!err) err = cache_find_dir(parent, TRUE, done, &dir);

        // Attempt to read details for the specified entry
        if (!err)
        {
            // Special case if root directory
            if (!leaf) info = dir;
            else
            {
                err = cache_find_dir_entry(op->cmd->data.open.path, FALSE,
                                           NULL, &info);
            }
        }

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!info
                 && ((op->cmd->data.open.mode != FS_MODE_OUT)
                     || (op->state == CACHE_PENDING_STATE_DONE)))
        {
            // The object does not exist
            err = &err_not_found;
        }
        else if ((op->state == CACHE_PENDING_STATE_DELETE)
                 && info && info->open)
        {
            // The object is already open
            err = &err_open;
        }
        else if (info && (info->info.obj_type == fileswitch_IS_DIR))
        {
            // The object is a directory
            if (op->cmd->data.open.mode == FS_MODE_IN)
            {
                // Fake the open directory
                handle->info.info |= FS_FILE_INFO_IS_DIRECTORY;
            }
            else err = &err_not_found;
        }
        else if (idle)
        {
            // Action depends on the current state
            if (op->state == CACHE_PENDING_STATE_DELETE)
            {
                // Special case if file already exists
                if (info)
                {
                    // Copy the currently allocated size of the file
                    handle->info.allocated = cache_round_allocated(info->info.size);

                    // Delete the existing file if creating a new one
                    if (op->cmd->data.open.mode == FS_MODE_OUT)
                    {
                        if (sizeof(cache_next_cmd.data.remove.path)
                            <= strlen(op->cmd->data.open.path))
                        {
                             err = &err_bad_name;
                        }
                        else
                        {
                            cache_next_cmd.op = UNIFIED_REMOVE;
                            strcpy(cache_next_cmd.data.remove.path,
                                   op->cmd->data.open.path);
                            err = cache_op_back(op);
                        }
                        if (!err) *done = FALSE;
                    }
                    else
                    {
                        handle->info.extent = info->info.size;
                        handle->load = info->info.load_addr;
                        handle->exec = info->info.exec_addr;
                        handle->attr = info->info.attr;
                    }
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_OPEN;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_OPEN))
            {
                fs_mode mode = op->cmd->data.open.mode;

                // Choose the file opening mode
                if ((mode == FS_MODE_UP)
                    && !(info->info.attr & fileswitch_ATTR_OWNER_WRITE))
                {
                    mode = FS_MODE_IN;
                }
                handle->info.info |= FS_FILE_INFO_READ_PERMITTED;
                if (mode != FS_MODE_IN)
                {
                    handle->info.info |= FS_FILE_INFO_WRITE_PERMITTED;
                }

                // Attempt to open the file
                if (sizeof(cache_next_cmd.data.open.path)
                    <= strlen(op->cmd->data.open.path))
                {
                     err = &err_bad_name;
                }
                else
                {
                    cache_next_cmd.op = UNIFIED_OPEN;
                    strcpy(cache_next_cmd.data.open.path,
                           op->cmd->data.open.path);
                    cache_next_cmd.data.open.mode = mode;
                    err = cache_op_back(op);
                }
                if (!err)
                {
                    op->state = CACHE_PENDING_STATE_RESIZE;
                    *done = FALSE;
                }
            }
            if (!err && *done && reply
                && (op->state == CACHE_PENDING_STATE_RESIZE))
            {
                // Store the remote file handle
                handle->handle = cache_next_reply.open.handle;

                // Special case if creating a new file
                if (op->cmd->data.open.mode == FS_MODE_OUT)
                {
                    // Create a new entry if required
                    if (!info)
                    {
                        fs_info detail;

                        // The file should have been created
                        if (sizeof(detail.name) <= strlen(leaf))
                        {
                            err = &err_bad_name;
                        }
                        else
                        {
                            // Add a new entry to the cache
                            strcpy(detail.name, leaf);
                            detail.obj_type = 0;
                            err = cache_dir_add(dir, &detail, &info);
                        }
                    }

                    // Set the allocated size if required
                    if (handle->info.allocated)
                    {
                        cache_next_cmd.op = UNIFIED_SIZE;
                        cache_next_cmd.data.size.handle = handle->handle;
                        cache_next_cmd.data.size.size = handle->info.allocated;
                        err = cache_op_back(op);
                    }

                    // Invalidate the directory entry
                    if (!err)
                    {
                        info->valid = FALSE;
                        *done = FALSE;
                    }
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_DONE;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_DONE))
            {
                // The file has been successfully opened
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }

        // Link the handle to either the active or free list if finished
        if (!err && *done)
        {
            // Add to the active list
            handle->next = cache_handle_active;
            handle->prev = NULL;
            if (cache_handle_active) cache_handle_active->prev = handle;
            cache_handle_active = handle;

            // Link to the directory entry
            handle->dir = info;
            info->open = handle;
        }
        else if (err && handle)
        {
            // Return the unused handle to the free list
            MEM_FREE(handle);
            /*
            handle->next = cache_handle_free;
            handle->prev = NULL;
            handle->dir = NULL;
            if (cache_handle_free) cache_handle_free->prev = handle;
            cache_handle_free = handle;
            */

            // Clear the returned handle
            handle = FS_NONE;
        }

        DEBUG_ERR(err)

        // Set the handle in the reply
        op->reply->open.handle = handle;

        // Failure to allocate memory or object not found is not an error
        if (err && (ERR_EQ(*err, err_buffer) || ERR_EQ(*err, err_not_found)))
        {
            err = NULL;
            *done = TRUE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a close request.
*/
static os_error *cache_op_close(cache_pending *op, os_error *err,
                                bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = op->cmd->data.close.handle;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else if (idle)
        {
            const char *path;

            // Generate the path for the file
            err = cache_dir_name(handle->dir, &path, FALSE, FALSE);

            // Action depends on the current state
            if (!err && *done && (op->state == CACHE_PENDING_STATE_INITIAL))
            {
                // Set the actual file size if necessary
                if (handle->info.info & FS_FILE_INFO_WRITE_PERMITTED)
                {
                    cache_next_cmd.op = UNIFIED_SIZE;
                    cache_next_cmd.data.size.handle = handle->handle;
                    cache_next_cmd.data.size.size = handle->info.extent;
                    err = cache_op_back(op);
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_CLOSE;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_CLOSE))
            {
                // Attempt to close the file if necessary
                if (!(handle->info.info & FS_FILE_INFO_IS_DIRECTORY))
                {
                    cache_next_cmd.op = UNIFIED_CLOSE;
                    cache_next_cmd.data.close.handle = handle->handle;
                    err = cache_op_back(op);
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_STAMP;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_STAMP))
            {
                // Set the modification date if necessary
                if (handle->stamp
                    || (handle->info.info & FS_FILE_INFO_WRITE_PERMITTED))
                {
                    if (sizeof(cache_next_cmd.data.stamp.path) <= strlen(path))
                    {
                         err = &err_bad_name;
                    }
                    else
                    {
                        cache_next_cmd.op = UNIFIED_STAMP;
                        strcpy(cache_next_cmd.data.stamp.path, path);
                        cache_next_cmd.data.stamp.date.words.high = handle->load & 0xff;
                        cache_next_cmd.data.stamp.date.words.low = handle->exec;
                        err = cache_op_back(op);
                    }
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_ACCESS;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_ACCESS))
            {
                // Set the attributes if necessary
                if (handle->access && (handle->dir->info.attr != handle->attr))
                {
                    if (sizeof(cache_next_cmd.data.access.path) <= strlen(path))
                    {
                         err = &err_bad_name;
                    }
                    else
                    {
                        cache_next_cmd.op = UNIFIED_ACCESS;
                        strcpy(cache_next_cmd.data.access.path, path);
                        cache_next_cmd.data.access.attr = handle->attr;
                        err = cache_op_back(op);
                    }
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_DONE;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_DONE))
            {
                // Unlink from and invalidate the directory cache
                if (handle->stamp || handle->access
                    || (handle->info.info & FS_FILE_INFO_WRITE_PERMITTED))
                {
                    handle->dir->valid = FALSE;
                }
                handle->dir->open = NULL;
                handle->dir = NULL;

                // Unlink from the active list
                if (handle->next) handle->next->prev = handle->prev;
                if (handle->prev) handle->prev->next = handle->next;
                else cache_handle_active = handle->next;

                // Free the memory
                MEM_FREE(handle);

                /*
                // Add to the free list
                handle->next = cache_handle_free;
                handle->prev = NULL;
                if (cache_handle_free) cache_handle_free->prev = handle;
                cache_handle_free = handle;
                */
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a read details request.
*/
static os_error *cache_op_args(cache_pending *op, os_error *err,
                               bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = op->cmd->data.args.handle;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else
        {
            const char *str;

            // Generate the name of the object
            err = cache_dir_name(handle->dir, &str, FALSE, FALSE);

            // Set the return values
            if (!err && (sizeof(op->reply->args.path) <= strlen(str)))
            {
                err = &err_bad_name;
            }
            if (!err)
            {
                strcpy(op->reply->args.path, str);
                op->reply->args.info = handle->info;
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a read request.
*/
static os_error *cache_op_read(cache_pending *op, os_error *err,
                               bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = op->cmd->data.read.handle;
        bits read = 0;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else if (!(handle->info.info & FS_FILE_INFO_READ_PERMITTED))
        {
            // Not open for reading
            err = &err_access;
        }
        else if ((handle->info.extent <= op->cmd->data.read.offset)
                 || !op->cmd->data.read.length)
        {
            // No data to read
        }
        else if (idle)
        {
            // Action depends on the current state
            if (op->state == CACHE_PENDING_STATE_INITIAL)
            {
                // Set the file pointer if necessary
                if (op->cmd->data.read.offset != handle->sequential)
                {
                    cache_next_cmd.op = UNIFIED_SEEK;
                    cache_next_cmd.data.seek.handle = handle->handle;
                    cache_next_cmd.data.seek.offset = op->cmd->data.read.offset;
                    err = cache_op_back(op);
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_READ;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_READ))
            {
                // Store the current sequential file pointer
                handle->sequential = op->cmd->data.read.offset;

                // Read the requested data
                cache_next_cmd.op = UNIFIED_READ;
                cache_next_cmd.data.read.handle = handle->handle;
                cache_next_cmd.data.read.length = MIN(op->cmd->data.read.length, handle->info.extent - op->cmd->data.read.offset);
                cache_next_cmd.data.read.buffer = op->cmd->data.read.buffer;
                err = cache_op_back(op);
                if (!err)
                {
                    *done = FALSE;
                    op->state = CACHE_PENDING_STATE_DONE;
                }
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_DONE))
            {
                // The read has completed
                read = cache_next_reply.read.length;
                handle->sequential += read;
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }

        // Pad the reply with zeros if necessary
        if (!err && *done && (read < op->cmd->data.read.length))
        {
            memset((byte *) op->cmd->data.read.buffer + read, 0,
                   op->cmd->data.read.length - read);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a write request.
*/
static os_error *cache_op_write(cache_pending *op, os_error *err,
                                bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = op->cmd->data.write.handle;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else if (!(handle->info.info & FS_FILE_INFO_WRITE_PERMITTED))
        {
            // Not open for writing
            err = &err_access;
        }
        else if (idle)
        {
            // Action depends on the current state
            if (op->state == CACHE_PENDING_STATE_INITIAL)
            {
                bits size = op->cmd->data.write.offset
                            + op->cmd->data.write.length;

                // Resize the file if necessary
                if (handle->info.allocated < size)
                {
                    cache_next_cmd.op = UNIFIED_SIZE;
                    cache_next_cmd.data.size.handle = handle->handle;
                    cache_next_cmd.data.size.size = cache_round_allocated(size);
                    err = cache_op_back(op);
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_RESIZE;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_RESIZE))
            {
                // Handle a successful resize operation
                if (reply)
                {
                    handle->info.extent = op->cmd->data.write.offset
                                          + op->cmd->data.write.length;
                    handle->info.allocated = cache_next_cmd.data.size.size;
                    err = cache_check_sequential(handle);
                }

                // Set the file pointer if necessary
                if (!err && (op->cmd->data.write.offset != handle->sequential))
                {
                    cache_next_cmd.op = UNIFIED_SEEK;
                    cache_next_cmd.data.seek.handle = handle->handle;
                    cache_next_cmd.data.seek.offset = op->cmd->data.write.offset;
                    err = cache_op_back(op);
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_WRITE;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_WRITE))
            {
                // Store the current sequential file pointer
                handle->sequential = op->cmd->data.write.offset;

                // Write the requested data
                cache_next_cmd.op = UNIFIED_WRITE;
                cache_next_cmd.data.write.handle = handle->handle;
                cache_next_cmd.data.write.length = op->cmd->data.write.length;
                cache_next_cmd.data.write.buffer = op->cmd->data.write.buffer;
                err = cache_op_back(op);
                if (!err)
                {
                    *done = FALSE;
                    op->state = CACHE_PENDING_STATE_DONE;
                }
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_DONE))
            {
                // The write has completed
                handle->sequential += op->cmd->data.write.length;
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a write zeros request.
*/
static os_error *cache_op_zero(cache_pending *op, os_error *err,
                               bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = op->cmd->data.write.handle;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else if (!(handle->info.info & FS_FILE_INFO_WRITE_PERMITTED))
        {
            // Not open for writing
            err = &err_access;
        }
        else if (idle)
        {
            // Action depends on the current state
            if (op->state == CACHE_PENDING_STATE_INITIAL)
            {
                bits size = op->cmd->data.zero.offset
                            + op->cmd->data.zero.length;

                // Resize the file if necessary
                if (handle->info.allocated < size)
                {
                    cache_next_cmd.op = UNIFIED_SIZE;
                    cache_next_cmd.data.size.handle = handle->handle;
                    cache_next_cmd.data.size.size = cache_round_allocated(size);
                    err = cache_op_back(op);
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_RESIZE;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_RESIZE))
            {
                // Handle a successful resize operation
                if (reply)
                {
                    handle->info.extent = op->cmd->data.zero.offset
                                          + op->cmd->data.zero.length;
                    handle->info.allocated = cache_next_cmd.data.size.size;
                    err = cache_check_sequential(handle);
                }

                // Set the file pointer if necessary
                if (!err && (op->cmd->data.zero.offset != handle->sequential))
                {
                    cache_next_cmd.op = UNIFIED_SEEK;
                    cache_next_cmd.data.seek.handle = handle->handle;
                    cache_next_cmd.data.seek.offset = op->cmd->data.zero.offset;
                    err = cache_op_back(op);
                    if (!err) *done = FALSE;
                }

                // Update the state
                if (!err) op->state = CACHE_PENDING_STATE_WRITE;
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_WRITE))
            {
                // Store the current sequential file pointer
                handle->sequential = op->cmd->data.zero.offset;

                // Write the requested data
                cache_next_cmd.op = UNIFIED_ZERO;
                cache_next_cmd.data.zero.handle = handle->handle;
                cache_next_cmd.data.zero.length = op->cmd->data.zero.length;
                err = cache_op_back(op);
                if (!err)
                {
                    *done = FALSE;
                    op->state = CACHE_PENDING_STATE_DONE;
                }
            }
            if (!err && *done && (op->state == CACHE_PENDING_STATE_DONE))
            {
                // The write has completed
                handle->sequential += op->cmd->data.zero.length;
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of an allocated size request.
*/
static os_error *cache_op_allocated(cache_pending *op, os_error *err,
                                    bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = op->cmd->data.allocated.handle;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else if (!(handle->info.info & FS_FILE_INFO_WRITE_PERMITTED))
        {
            // Not open for writing
            err = &err_access;
        }
        else if (reply)
        {
            // Size successfully set
            handle->info.allocated = cache_next_cmd.data.size.size;
            err = cache_check_sequential(handle);
        }
        else if (idle)
        {
            bits size = cache_round_allocated(op->cmd->data.allocated.size);

            // No action if size is already large enough
            if (handle->info.allocated < size)
            {
                // Attempt to set the size of the file
                cache_next_cmd.op = UNIFIED_SIZE;
                cache_next_cmd.data.size.handle = handle->handle;
                cache_next_cmd.data.size.size = size;
                err = cache_op_back(op);
                if (!err) *done = FALSE;
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of an extent request.
*/
static os_error *cache_op_extent(cache_pending *op, os_error *err,
                                 bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = op->cmd->data.extent.handle;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else if (!(handle->info.info & FS_FILE_INFO_WRITE_PERMITTED))
        {
            // Not open for writing
            err = &err_access;
        }
        else if (reply)
        {
            // Size successfully set
            handle->info.extent = op->cmd->data.extent.size;
            handle->info.allocated = cache_next_cmd.data.size.size;
            err = cache_check_sequential(handle);
        }
        else if (idle)
        {
            bits size = cache_round_allocated(op->cmd->data.extent.size);

            // No action if size is already large enough
            if (handle->info.allocated < size)
            {
                // Attempt to set the size of the file
                cache_next_cmd.op = UNIFIED_SIZE;
                cache_next_cmd.data.size.handle = handle->handle;
                cache_next_cmd.data.size.size = size;
                err = cache_op_back(op);
                if (!err) *done = FALSE;
            }
            else
            {
                // Update the file extent
                handle->info.extent = op->cmd->data.extent.size;
                err = cache_check_sequential(handle);
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a flush request.
*/
static os_error *cache_op_flush(cache_pending *op, os_error *err,
                                bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        // No action is required for a flush request
        *done = TRUE;

        /*
        fs_handle handle = op->cmd->data.flush.handle;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else if (!(handle->info.info & FS_FILE_INFO_WRITE_PERMITTED))
        {
            // Not open for writing, so no action required
        }
        else if (reply)
        {
            // Buffers successfully flushed
        }
        else if (idle)
        {
            // Attempt to flush the file
            cache_next_cmd.op = UNIFIED_FLUSH;
            cache_next_cmd.data.flush.handle = handle->handle;
            err = cache_op_back(op);
            if (!err) *done = FALSE;
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
        */
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation to perform.
                  err           - Any error received with the reply.
                  reply         - Has a reply been received for this operation.
                  idle          - Can a command be performed immediately.
                  done          - Variable to receive whether the operation is
                                  complete.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of a set sequential file pointer
                  request.
*/
static os_error *cache_op_sequential(cache_pending *op, os_error *err,
                                     bool reply, bool idle, bool *done)
{
    // Check function parameters
    if (!op || !done) err = &err_bad_parms;
    else
    {
        fs_handle handle = op->cmd->data.flush.handle;

        // Start by assuming that the operation can complete
        *done = TRUE;

        // Take appropriate action
        if (err || !*done)
        {
            // Either an error occurred or waiting for data
        }
        else if (!handle->dir)
        {
            // Handle no longer valid
            err = &err_channel;
        }
        else if (op->cmd->data.sequential.offset <= handle->info.extent)
        {
            // The file is already large enough
            handle->info.sequential = op->cmd->data.sequential.offset;
        }
        else if (!(handle->info.info & FS_FILE_INFO_WRITE_PERMITTED))
        {
            // Unable to extend the file
            err = &err_outside;
        }
        else if (reply)
        {
            // Extent successfully set
            handle->info.sequential = op->cmd->data.sequential.offset;
            handle->info.extent = op->cmd->data.sequential.offset;
            handle->info.allocated = cache_next_cmd.data.size.size;
            err = cache_check_sequential(handle);
        }
        else if (idle)
        {
            bits size = cache_round_allocated(op->cmd->data.sequential.offset);

            // No action if size is already large enough
            if (handle->info.allocated < size)
            {
                // Attempt to set the size of the file
                cache_next_cmd.op = UNIFIED_SIZE;
                cache_next_cmd.data.size.handle = handle->handle;
                cache_next_cmd.data.size.size = size;
                err = cache_op_back(op);
                if (!err) *done = FALSE;
            }
            else
            {
                // Just set the extent and sequential file pointer
                handle->info.sequential = op->cmd->data.sequential.offset;
                handle->info.extent = op->cmd->data.extent.size;
            }
        }
        else
        {
            // Waiting for the link to become idle
            *done = FALSE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation data.
                  err           - Any error to return.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End processing an operation.
*/
static os_error *cache_op_done(cache_pending *op, os_error *err)
{
    // Check function parameters
    if (!op) err = &err_bad_parms;
    else
    {
        // Cancel any pending reply
        if (cache_pending_head == op)
        {
            cache_pending_cmd = FALSE;
            cache_pending_err = NULL;
            cache_pending_reply = FALSE;
        }

        // Unlink from the active list
        if (op->next) op->next->prev = op->prev;
        else cache_pending_tail = op->prev;
        if (op->prev) op->prev->next = op->next;
        else cache_pending_head = op->next;

        /*
        // Add to the free list
        op->prev = NULL;
        op->next = cache_pending_free;
        if (cache_pending_free) cache_pending_free->prev = op;
        cache_pending_free = op;
        */

        // Call the callback function
        err = (*op->callback)(op->user, err, op->reply);

        // Free the memory
        MEM_FREE(op);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the next stage of an operation.
*/
static os_error *cache_process(void)
{
    os_error *err = NULL;
    static bool threaded = FALSE;
    bool done = TRUE;

    // No action if already threaded
    if (!threaded)
    {
        // Set the threaded status
        threaded = TRUE;

        // Attempt to process any pending operations
        while (!err && done && cache_pending_head)
        {
            bool reply;

            // Restore any pending reply if active
            if (cache_active)
            {
                err = cache_pending_err;
                reply = cache_pending_reply;
            }
            else
            {
                err = &err_cache_inactive;
                reply = FALSE;
            }
            cache_pending_err = NULL;
            cache_pending_reply = FALSE;

            // Can only perform an operation if active
            if (cache_pending_cmd) done = FALSE;
            else
            {
                // Perform an appropriate action
                switch (cache_pending_head->cmd->op)
                {
                    case CACHE_DRIVE:
                        // Read drive details
                        err = cache_op_drive(cache_pending_head, err, reply,
                                             !cache_next_active, &done);
                        break;

                    case CACHE_NAME:
                        // Change a disc name
                        err = cache_op_name(cache_pending_head, err, reply,
                                            !cache_next_active, &done);
                        break;

                    case CACHE_ENUMERATE:
                        // Enumerate objects in a directory
                        err = cache_op_enumerate(cache_pending_head, err, reply,
                                                 !cache_next_active, &done);
                        break;

                    case CACHE_INFO:
                        // Read details for a single object
                        err = cache_op_info(cache_pending_head, err, reply,
                                            !cache_next_active, &done);
                        break;

                    case CACHE_MKDIR:
                        // Create a directory
                        err = cache_op_mkdir(cache_pending_head, err, reply,
                                             !cache_next_active, &done);
                        break;

                    case CACHE_REMOVE:
                        // Delete an object
                        err = cache_op_remove(cache_pending_head, err, reply,
                                              !cache_next_active, &done);
                        break;

                    case CACHE_RENAME:
                        // Rename an object
                        err = cache_op_rename(cache_pending_head, err, reply,
                                              !cache_next_active, &done);
                        break;

                    case CACHE_ACCESS:
                        // Change the attributes of an object
                        err = cache_op_access(cache_pending_head, err, reply,
                                              !cache_next_active, &done);
                        break;

                    case CACHE_STAMP:
                        // Change the date stamp of an object
                        err = cache_op_stamp(cache_pending_head, err, reply,
                                             !cache_next_active, &done);
                        break;

                    case CACHE_OPEN:
                        // Open an object
                        err = cache_op_open(cache_pending_head, err, reply,
                                            !cache_next_active, &done);
                        break;

                    case CACHE_CLOSE:
                        // Close a previously opened object
                        err = cache_op_close(cache_pending_head, err, reply,
                                             !cache_next_active, &done);
                        break;

                    case CACHE_ARGS:
                        // Read the details of a previously opened file
                        err = cache_op_args(cache_pending_head, err, reply,
                                            !cache_next_active, &done);
                        break;

                    case CACHE_READ:
                        // Read from a previously opened file
                        err = cache_op_read(cache_pending_head, err, reply,
                                            !cache_next_active, &done);
                        break;

                    case CACHE_WRITE:
                        // Write to a previously opened file
                        err = cache_op_write(cache_pending_head, err, reply,
                                             !cache_next_active, &done);
                        break;

                    case CACHE_ZERO:
                        // Write zeros to a previously opened file
                        err = cache_op_zero(cache_pending_head, err, reply,
                                            !cache_next_active, &done);
                        break;

                    case CACHE_ALLOCATED:
                        // Set the allocated size of a previously opened file
                        err = cache_op_allocated(cache_pending_head, err, reply,
                                                 !cache_next_active, &done);
                        break;

                    case CACHE_EXTENT:
                        // Set the extent of a previously opened file
                        err = cache_op_extent(cache_pending_head, err, reply,
                                              !cache_next_active, &done);
                        break;

                    case CACHE_FLUSH:
                        // Flush buffers of a previously opened file
                        err = cache_op_flush(cache_pending_head, err, reply,
                                             !cache_next_active, &done);
                        break;

                    case CACHE_SEQUENTIAL:
                        // Set the sequential file pointer of an open file
                        err = cache_op_sequential(cache_pending_head, err,
                                                  reply, !cache_next_active,
                                                  &done);
                        break;

                    default:
                        // Not a supported command
                        if (!err) err = &err_bad_cache_op;
                        break;
                }
            }

            // Call the callback function if appropriate
            if (err || done)
            {
                done = TRUE;
                err = cache_op_done(cache_pending_head, err);
            }
        }

        // Try to start another operation
        if (!err) err = cache_next_start();

        // Clear the threaded status
        threaded = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a link operation.
*/
static os_error *cache_fore_callback(void *user, os_error *err,
                                     const void *reply)
{
    // Store the status
    cache_fore_done = TRUE;
    cache_fore_err = err;

    // Clear any error status
    err = NULL;

    // Return any error produced
    return err;
}

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the link to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *cache_fore(const cache_cmd *cmd, cache_reply *reply, bool escape)
{
    os_error *err = NULL;

    // Clear the pending done flag
    cache_fore_done = FALSE;

    // Start as a background operation
    err = cache_back(cmd, reply, NULL, cache_fore_callback);

    // Process the operation in the foreground
    while (!err && !cache_fore_done)
    {
        // Poll the various layers
        err = link_poll(escape);
    }

    // Preserve any returned error
    if (!err && cache_fore_done) err = cache_fore_err;

    // Return any error produced
    return err;
}

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the specified operation when the link becomes idle.
                  Control is returned immediately. If the link is not valid or
                  the operation fails then no error is returned, but instead
                  the callback function is notified.
*/
os_error *cache_back(const cache_cmd *cmd, cache_reply *reply,
                     void *user, share_callback callback)
{
    os_error *err = NULL;

    // Check function parameters
    if (!cmd || !reply || !callback) err = &err_bad_parms;
    else
    {
        cache_pending *ptr;

        // Obtain an operation record pointer
        /*
        if (cache_pending_free)
        {
            // A suitable record already exists
            ptr = cache_pending_free;
            if (ptr->next) ptr->next->prev = NULL;
            cache_pending_free = ptr->next;
        }
        else
        */
        {
            // Allocate a new structure
            ptr = (cache_pending *) MEM_MALLOC(sizeof(cache_pending));
            if (!ptr) err = &err_buffer;
        }

        // Complete the details
        if (!err)
        {
            // Add to the active list
            ptr->prev = cache_pending_tail;
            ptr->next = NULL;
            if (cache_pending_tail) cache_pending_tail->next = ptr;
            else cache_pending_head = ptr;
            cache_pending_tail = ptr;

            // Copy the command details
            ptr->cmd = cmd;
            ptr->reply = reply;
            ptr->user = user;
            ptr->callback = callback;

            // Always start in the initial state
            ptr->state = CACHE_PENDING_STATE_INITIAL;

            // Start the operation
            err = cache_process();
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : drive         - The drive to read the details of.
                  status        - Variable to receive the status of the drive,
                                  or NULL if the value is not required.
                  name          - Variable to receive a pointer to the drive
                                  name, or NULL if the value is not required.
                  id            - Variable to receive the unique identifier of
                                  the drive, or NULL if the value is not
                                  required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the status for the specified drive.
*/
os_error *cache_drive_status(char drive, psifs_drive_status *status,
                             const char **name, psifs_drive_id *id)
{
    os_error *err = NULL;

    // Check parameters
    if (!isalpha(drive) || (!status && !name && !id)) err = &err_bad_parms;
    else
    {
        bool valid;
        cache_drive *info;

        // Attempt to read the drive details
        err = cache_find_drive(drive, FALSE, &valid, &info);

        // Set the return values if successful
        if (!err)
        {
            // Check if the drive is present
            valid = valid && info && info->info.present;

            // Set the status value
            if (status)
            {
                if (valid)
                {
                    *status = psifs_DRIVE_STATUS_PRESENT;
                    if (info->info.rom) *status |= psifs_DRIVE_STATUS_ROM;
                }
                else *status = 0;
            }

            // Set the drive name
            if (name) *name = valid ? info->info.name : NULL;

            // Set the unique ID
            if (id) *id = valid ? info->info.id : 0;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : type          - Variable to receive the machine type, or NULL
                                  if the value is not required.
                  name          - Variable to receive a pointer to the machine
                                  description, or NULL if the value is not
                                  required.
                  id            - Variable to receive the unique identifier, or
                                  NULL if the value is not required.
                  language      - Variable to receive the language, or NULL if
                                  the value is not required.
                  version       - Variable to receive the operating system
                                  version, or NULL if the value is not required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the machine details.
*/
os_error *cache_machine_status(psifs_machine_type *type, const char **name,
                               unified_machine_id *id, psifs_language *language,
                               unified_version *version)
{
    os_error *err = NULL;

    // Check parameters
    if (!type && !name && !id && !language && !version) err = &err_bad_parms;
    else
    {
        bool valid;

        // Attempt to read the machine details
        err = cache_find_machine(FALSE, &valid);

        // Set the return values if successful
        if (!err)
        {
            if (type) *type = valid ? cache_machine_type : psifs_MACHINE_TYPE_UNKNOWN;
            if (name) *name = valid ? cache_machine_name : NULL;
            if (id)
            {
                id->low = valid ? cache_machine_id.low : 0;
                id->high = valid ? cache_machine_id.high : 0;
            }
            if (language) *language = valid ? cache_machine_language : psifs_LANGUAGE_UNKNOWN;
            if (version)
            {
                version->major = valid ? cache_machine_version.major : 0;
                version->minor = valid ? cache_machine_version.minor : 0;
                version->build = valid ? cache_machine_version.build : 0;
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : owner         - Variable to receive a pointer to the owner
                                  information, or NULL if the value is not
                                  required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the owner information.
*/
os_error *cache_owner_status(const char **owner)
{
    os_error *err = NULL;

    // Check parameters
    if (!owner) err = &err_bad_parms;
    else
    {
        bool valid;

        // Attempt to read the owner information
        err = cache_find_owner(FALSE, &valid);

        // Set the return values if successful
        if (!err)
        {
            if (owner) *owner = valid ? cache_owner_info : NULL;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : main          - Variable to receive the status of the main
                                  battery, or NULL if the value is not required.
                  backup        - Variable to receive the status of the backup
                                  battery, or NULL if the value is not required.
                  external      - Variable to receive the status of external
                                  power, or NULL if the value is not required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the power status.
*/
os_error *cache_power_status(unified_battery *main, unified_battery *backup,
                             bool *external)
{
    os_error *err = NULL;

    // Check parameters
    if (!main && !backup && !external) err = &err_bad_parms;
    else
    {
        bool valid;

        // Attempt to read the power details
        err = cache_find_power(FALSE, &valid);

        // Set an error if details are not available
        if (!err && !valid) err = &err_cache_busy;
        if (!err && cache_power_err) err = cache_power_err;

        // Set the return values if successful
        if (!err)
        {
            if (main) *main = cache_power_main;
            if (backup) *backup = cache_power_backup;
            if (external) *external = cache_power_external;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The path of the object to invalidate.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Re-cache the details for the specified object.
*/
os_error *cache_recache(const char *path)
{
    os_error *err = NULL;

    // Check parameters
    if (!path) err = &err_bad_parms;
    else
    {
        // Attempt to find the specified object
        cache_dir *dir;
        err = cache_find_dir_entry(path, FALSE, NULL, &dir);

        // Request an update of the object and its parent if found
        if (!err && dir)
        {
            dir->required = TRUE;
            if (dir->parent) dir->parent->required = TRUE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required.
*/
os_error *cache_poll(void)
{
    os_error *err = NULL;

    // No action unless active
    if (cache_active)
    {
        // Try to advance any pending operations
        err = cache_process();

        // Check if connection just established
        if (!err && !cache_connected)
        {
            bool connected = TRUE;
            int i;

            // Check if all the drives have been found
            for (i = 0; !err && (i < 26); i++)
            {
                if (!cache_drive_array[i].root.valid) connected = FALSE;
            }

            // Start higher layers if necessary
            if (connected)
            {
                cache_connected = connected;
                err = cache_connect();
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : era           - Is an ERA device connected.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the cache layer after the unified server layer has been
                  started.
*/
os_error *cache_start(bool era)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting cache layer"))

    // No action if already active
    if (!cache_active)
    {
        // Store the remote device type
        cache_era = era;

        // Invalidate all details
        err = cache_invalidate_all();

        // Set the active flag if successful
        if (!err) cache_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the cache layer before the unified server layer has been
                  closed.
*/
os_error *cache_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending cache layer now=%u", now))

    // No action if unless active
    if (cache_active)
    {
        // End higher levels
        err = cache_disconnect(now);

        // Invalidate all details
        if (!err) err = cache_invalidate_all();

        // Clear the conected flag if successful
        if (!err) cache_connected = FALSE;

        // Clear the active flag if successful
        if (!err) cache_active = FALSE;

        // Fail any pending operations
        if (!err) err = cache_process();

        // Deallocate any buffer
        if (!err) err = cache_buffer_free();
    }

    // Return any error produced
    return err;
}

#ifdef DEBUG

/*
    Parameters  : indent        - The level of indentation.
                  dir           - The directory to dump.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Dump the current cache state for the specified directory.
*/
static os_error *cache_dump_dir(bits indent, const cache_dir *dir)
{
    os_error *err = NULL;

    // Display the details for the specified directory entry
    printf("%*s%-*s %-8s %-9s  ",
           indent, "", 50 - indent, dir->info.name,
           dir->required ? "Required" : "", dir->valid ? "Valid" : "Not valid");
    if (dir->valid)
    {
        if (dir->err)
        {
            printf("Error %u '%s'", dir->err->errnum, dir->err->errmess);
        }
        else
        {
            printf("load=0x%08x, exec=0x%08x, size=%010u, attr=0x%02x, type=%u",
                   dir->info.load_addr, dir->info.exec_addr, dir->info.size,
                   dir->info.attr, dir->info.obj_type);
        }
    }
    printf("\n");

    // Display open file details if required
    if (dir->open)
    {
        printf("%*s%-*s os=0x%02x, info=0x%08x, extent=%010u, allocated=%010u, sequential=%010u, load=0x%08x, exec=0x%08x, attr=0x%02x, unified=0x%08x\n",
               indent, "", indent, "> Open", dir->open->info.handle,
               dir->open->info.info, dir->open->info.extent,
               dir->open->info.allocated, dir->open->info.sequential,
               dir->open->load, dir->open->exec, dir->open->attr,
               dir->open->handle);
    }

    // Extra information if a directory
    if (dir->dir.active)
    {
        printf("%*s%-*s r%08x %-8s %-9s  ",
               indent, "", 40 - indent, "> Directory", dir->dir.refresh,
               dir->dir.required ? "Required" : "",
               dir->dir.valid ? "Valid" : "Not valid");
        if (dir->dir.valid)
        {
            if (dir->dir.err)
            {
                printf("Error %u '%s'",
                       dir->dir.err->errnum, dir->dir.err->errmess);
            }
            else printf("No error");
        }
        printf("\n");
    }

    // Display the details for any subdirectories
    dir = dir->dir.children;
    while (!err && dir)
    {
        // Recurse through this entry
        err = cache_dump_dir(indent + 1, dir);

        // Advance to the next subdirectory
        dir = dir->next;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Dump the current cache state.
*/
static os_error *cache_dump(void)
{
    os_error *err = NULL;
    char drive;

    // Display a header for the cache details
    printf("\nCache contents:\n");

    // Loop through all of the drives
    for (drive = 'A'; !err && (drive <= 'Z'); drive++)
    {
        const cache_drive *info = &cache_drive_array[drive - 'A'];

        // Display the details for this drive
        printf("    Drive %c: r%08x ", drive, info->refresh);
        if (info->info.present)
        {
            printf("%-47s free=0x%08x%08x, size=0x%08x%08x, id=0x%08x\n", info->info.name, info->info.free_high, info->info.free_low, info->info.size_high, info->info.size_low, info->info.id);
            err = cache_dump_dir(5, &info->root);
        }
        else printf("(not present)\n");
    }

    // Return any error produced
    return err;
}

#endif

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the cache layer.
*/
os_error *cache_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying cache layer status"))

    // Display the current connection status
    if (cache_active)
    {
        printf("Connected to remote filesystem");
        if (cache_machine_valid && !cache_machine_err)
        {
            printf(" of %s", cache_machine_name);
            if (cache_machine_id.low || cache_machine_id.high)
            {
                printf(" (%04X-%04X-%04X-%04X)",
                       cache_machine_id.high / 0x10000,
                       cache_machine_id.high % 0x10000,
                       cache_machine_id.low / 0x10000,
                       cache_machine_id.low % 0x10000);
            }
        }
        printf(".\n");
        if (cache_power_valid && !cache_power_err)
        {
            static const char *status[4];
            status[0] = "dead";
            status[1] = "very low";
            status[2] = "low";
            status[3] = "good";
            printf("Main batteries are %s (%u.%03uV),"
                   " backup battery is %s (%u.%03uV)",
                   status[cache_power_main.status],
                   cache_power_main.mv / 1000, cache_power_main.mv % 1000,
                   status[cache_power_backup.status],
                   cache_power_backup.mv / 1000, cache_power_backup.mv % 1000);
            if (cache_power_external) printf(", external power");
            printf(".\n");
        }
        if (cache_sync_done && !cache_sync_err)
        {
            printf("Successfully synchronized clock.\n");
        }
        if (!err) err = rclip_status();
        if (!err) err = wprt_status();
    }
    else printf("Not connected to remote filesystem.\n");

#ifdef DEBUG

    // Dump the cache data
    err = cache_dump();

#endif

    // Return any error produced
    return err;
}
