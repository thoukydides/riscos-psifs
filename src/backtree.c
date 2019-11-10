/*
    File        : backtree.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Backup tree manipulation for the PsiFS module.

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
#include "backtree.h"

// Include clib header files
#include <stdio.h>

// Include project header files
#include "ctrl.h"
#include "err.h"
#include "mem.h"
#include "wildcard.h"

// A backup tree node or leaf
typedef struct backtree_record
{
    struct backtree_record *next;
    struct backtree_record *parent;
    struct backtree_record *child;
    fs_info info;
    bool ignore;
} backtree_record;

// Handle for a backup tree
struct backtree_handle
{
    bits reference;
    backtree_record *root;
    backtree_record *next;
};

// Function prototypes
static os_error *backtree_find(backtree_handle handle, const char *name,
                               backtree_record **ptr);

/*
    Parameters  : handle        - The backup tree handle.
                  info          - The details of the file, including the
                                  sub-directory part of the path.
                  add           - Should dummy parent directory entries be
                                  added if they do not exist.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the backup tree record for the parent of the specified
                  file. If necessary, dummy parent directory entries are added.
*/
static os_error *backtree_find_parent(backtree_handle handle, const char *name,
                                      backtree_record **parent, bool add)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !name || !parent) err = &err_bad_parms;
    else
    {
        static fs_info info;
        char *dot = NULL;

        // Copy the filename if necessary
        if (info.name != name)
        {
            if (sizeof(info.name) <= ctrl_strlen(name)) err = &err_bad_name;
            else ctrl_strcpy(info.name, name);
        }

        // Attempt to remove the leafname
        if (!err) dot = ctrl_strrchr(info.name, FS_CHAR_SEPARATOR);
        if (dot)
        {
            // Remove the leafname
            *dot = '\0';

            // Attempt to find the parent directory
            err = backtree_find(handle, info.name, parent);

            // Create a dummy parent directory entry if not found
            if (!err && !*parent && add)
            {
                info.load_addr = 0;
                info.exec_addr = 0;
                info.size = 0;
                info.attr = 0;
                info.obj_type = fileswitch_IS_DIR;
                err = backtree_add(handle, &info);
            }

            // Attempt to find the parent directory again
            if (!err && !*parent && add)
            {
                err = backtree_find(handle, info.name, parent);
            }
            if (!err && !*parent) err = &err_not_found;

            // The parent must be a directory
            if (!err && ((*parent)->info.obj_type != fileswitch_IS_DIR))
            {
                err = &err_types;
            }

            // Restore the directory separator for recursive calls
            if (!err) *dot = FS_CHAR_SEPARATOR;
        }
        else *parent = NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The backup tree handle.
                  parent        - Pointer to the parent record, or NULL if
                                  the root directory should be searched.
                  name          - The name of the file, including the
                                  sub-directory part of the path.
                  ptr           - Variable to receive a pointer to the requested
                                  record, or NULL if it does not exist.
                  prev          - Variable to receive a pointer to the previous
                                  record, or NULL if it should be the first
                                  entry in the directory.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the record associated with the specified file.
*/
static os_error *backtree_find_raw(backtree_handle handle,
                                   backtree_record *parent, const char *name,
                                   backtree_record **ptr,
                                   backtree_record **prev)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !name || !ptr || !prev) err = &err_bad_parms;
    else
    {
        // Start from the first directory entry
        *prev = NULL;
        *ptr = parent ? parent->child : handle->root;

        // Attempt to find the required entry
        while (*ptr && (0 < wildcard_cmp(name, (*ptr)->info.name)))
        {
            *prev = *ptr;
            *ptr = (*ptr)->next;
        }

        // Clear the current pointer if not found
        if (*ptr && wildcard_cmp(name, (*ptr)->info.name)) *ptr = NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The backup tree handle.
                  name          - The name of the file, including the
                                  sub-directory part of the path.
                  ptr           - Variable to receive a pointer to the requested
                                  record, or NULL if it does not exist.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the record associated with the specified file.
*/
static os_error *backtree_find(backtree_handle handle, const char *name,
                               backtree_record **ptr)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !name || !ptr) err = &err_bad_parms;
    else
    {
        backtree_record *parent;
        backtree_record *prev;

        // Find the parent directory
        err = backtree_find_parent(handle, name, &parent, FALSE);

        // Find the required entry
        if (!err) err = backtree_find_raw(handle, parent, name, ptr, &prev);

        // Special case for not found errors
        if (ERR_EQ(*err, err_not_found))
        {
            *ptr = NULL;
            err = NULL;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : ptr           - The current record.
                  next          - Variable to receive a pointer to the next
                                  record, or NULL if no more.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Obtain a pointer to the next backup tree record in a linear
                  traversal of the tree.
*/
static os_error *backtree_next(backtree_record *ptr, backtree_record **next)
{
    os_error *err = NULL;

    // Check function parameters
    if (!ptr || !next) err = &err_bad_parms;
    else
    {
        // Move in the appropriate direction
        if (ptr->child)
        {
            // Traverse children before siblings
            *next = ptr->child;
        }
        else
        {
            // Find the first parent with untraversed siblings
            while (!ptr->next && ptr->parent)
            {
                ptr = ptr->parent;
            }

            // Traverse siblings from the first possible parent
            *next = ptr->next;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : ptr           - The current record.
                  next          - Variable to receive a pointer to the next
                                  record, or NULL if no more.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Free the memory associated with the specified backup tree
                  record, including any siblings or children.
*/
static os_error *backtree_free(backtree_record *ptr)
{
    os_error *err = NULL;

    // Check function parameters
    if (!ptr) err = &err_bad_parms;
    else
    {
        // Loop through all siblings
        while (!err && ptr)
        {
            backtree_record *this = ptr;

            // Advance to the next sibling
            ptr = ptr->next;

            // Free the memory used by any children
            if (this->child) err = backtree_free(this->child);

            // Free the memory used by this file record
            if (!err) MEM_FREE(this);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The backup tree handle.
                  info          - The details of the file, including the
                                  sub-directory part of the path.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Add the specified file to the backup tree. If necessary, this
                  also adds dummy entries for any parent directories.
*/
os_error *backtree_add(backtree_handle handle, const fs_info *info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !info) err = &err_bad_parms;
    else
    {
        backtree_record *ptr;
        backtree_record *parent;
        backtree_record *prev;

        // Find the parent entry
        err = backtree_find_parent(handle, info->name, &parent, TRUE);

        // Attempt to find this entry
        if (!err)
        {
            err = backtree_find_raw(handle, parent, info->name, &ptr, &prev);
        }

        // If already exists then verify not changing type
        if (!err && ptr && (info->obj_type != ptr->info.obj_type))
        {
            err = &err_types;
        }

        // Create a new record if necessary
        if (!err && !ptr)
        {
            // Allocate memory for the new record
            ptr = (backtree_record *) MEM_MALLOC(sizeof(backtree_record));
            if (!ptr) err = &err_buffer;

            // Link the new record in
            if (!err)
            {
                ptr->parent = parent;
                ptr->child = NULL;
                if (prev)
                {
                    // Not first file in directory
                    ptr->next = prev->next;
                    prev->next = ptr;
                }
                else if (parent)
                {
                    // First file in sub-directory
                    ptr->next = parent->child;
                    parent->child = ptr;
                }
                else
                {
                    // First file in root directory
                    ptr->next = handle->root;
                    handle->root = ptr;
                }
            }
        }

        // Set the file details if successful
        if (!err)
        {
            ptr->info = *info;
            ptr->ignore = FALSE;
        }

        // Reset the next pointer for enumeration
        if (!err) handle->next = handle->root;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : file1 - The first control character terminated file name.
                  file2 - The second control character terminated file name.
    Returns     : int   - The result of the comparison:
                            < 0 if file1 < file2
                             0  if file1 == file2
                            > 0 if file1 > file2
    Description : Perform a case sensitive comparison of the leaf part of the
                  two filenames.
*/
static int backtree_leaf_cmp(const char *file1, const char *file2)
{
    const char *ptr;

    // Obtain pointers to the leafnames
    ptr = ctrl_strrchr(file1, FS_CHAR_SEPARATOR);
    if (ptr) file1 = ptr + 1;
    ptr = ctrl_strrchr(file2, FS_CHAR_SEPARATOR);
    if (ptr) file2 = ptr + 1;

    // Return the result of the comparison
    return ctrl_strcmp(file1, file2);
}

/*
    Parameters  : handle        - The backup tree handle.
                  info          - The details of the file to compare, including
                                  the sub-directory part of the path.
                  result        - Variable to receive the result of the
                                  comparison.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Compare the specified file details to the corresponding file
                  to backup.
*/
os_error *backtree_check(backtree_handle handle, const fs_info *info,
                         backtree_result *result)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !info || !result) err = &err_bad_parms;
    else
    {
        backtree_record *ptr;

        // Attempt to find the appropriate record
        err = backtree_find(handle, info->name, &ptr);

        // Set the result code
        if (err || !ptr || (info->obj_type != ptr->info.obj_type))
        {
            // Error or file not found in backup tree
            *result = BACKTREE_NOT_FOUND;
        }
        else if ((info->load_addr == ptr->info.load_addr)
                 && (info->exec_addr == ptr->info.exec_addr)
                 && ((info->size == ptr->info.size)
                     || (info->obj_type == fileswitch_IS_DIR))
                 && (info->attr == ptr->info.attr)
                 && !backtree_leaf_cmp(info->name, ptr->info.name))
        {
            // File details are identical
            *result = BACKTREE_SAME;
        }
        else if (((info->load_addr & 0xfff00000) == 0xfff00000)
                 && ((ptr->info.load_addr & 0xfff00000) == 0xfff00000)
                 && (((ptr->info.load_addr & 0xff)
                      < (info->load_addr & 0xff))
                     || (((ptr->info.load_addr & 0xff)
                          == (info->load_addr & 0xff))
                         && (ptr->info.exec_addr < info->exec_addr))))
        {
            // Supplied details are more recent than the backup tree version
            *result = BACKTREE_NEWER;
        }
        else
        {
            // Version in backup tree is more recent
            *result = BACKTREE_OLDER;
        }
/*
printf("\nload=%08x exec=%08x size=%08x attr=%02x type=%01x %s\n", info->load_addr, info->exec_addr, info->size, info->attr, info->obj_type, info->name);
if (err || !ptr) printf("Not found\n");
else printf("load=%08x exec=%08x size=%08x attr=%02x type=%01x %s\n", ptr->info.load_addr, ptr->info.exec_addr, ptr->info.size, ptr->info.attr, ptr->info.obj_type, ptr->info.name);
printf("result = %u\n", *result);
*/
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The backup tree handle.
                  info          - The details of the file to remove, including
                                  the sub-directory part of the path.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Mark the specified file in the backup tree to be skipped
                  during the copy stage of the backup. This does not remove the
                  file, leaving it available for subsequent calls to
                  backtree_check.
*/
os_error *backtree_ignore(backtree_handle handle, const fs_info *info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !info) err = &err_bad_parms;
    else
    {
        backtree_record *ptr;

        // Attempt to find the appropriate record
        err = backtree_find(handle, info->name, &ptr);
        if (!err && !ptr) err = &err_not_found;

        // Mark the file to be ignored
        if (!err) ptr->ignore = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The backup tree handle.
                  info          - Variable to receive a pointer to the file
                                  details, including the sub-directory part of
                                  the path, or NULL if no more.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Return a pointer to the details of a file that has not been
                  marked to be ignored. To enumerate successive files,
                  backtree_ignore should be called with the returned pointer.
*/
os_error *backtree_enumerate(backtree_handle handle, const fs_info **info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !info) err = &err_bad_parms;
    else
    {
        // Find the first record not marked to be ignored
        while (!err && handle->next && handle->next->ignore)
        {
            err = backtree_next(handle->next, &handle->next);
        }

        // Set the return value appropriately
        *info = err || !handle->next ? NULL : &handle->next->info;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The backup tree handle.
                  files         - Variable to receive the number of files.
                  size          - Variable to receive the total size of the
                                  files.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Count the number of files that have not been marked to ignore.
*/
os_error *backtree_count(backtree_handle handle, bits *files, bits *size)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !files || !size) err = &err_bad_parms;
    else
    {
        backtree_record *ptr = handle->root;

        // Initialise the counters
        *files = *size = 0;

        // Loop through all tree records
        while (!err && ptr)
        {
            // No action if marked to be ignored
            if (!ptr->ignore)
            {
                // Update the counts
                (*files)++;
                if (ptr->info.obj_type != fileswitch_IS_DIR)
                {
                    (*size) += ptr->info.size;
                }
            }

            // Advance to the next record
            err = backtree_next(ptr, &ptr);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Variable to receive the backup tree handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a new backup tree.
*/
os_error *backtree_create(backtree_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        // Create a data structure for this tree
        *handle = (backtree_handle) MEM_MALLOC(sizeof(struct backtree_handle));
        if (!*handle) err = &err_buffer;

        // Prepare the data structure
        if (!err)
        {
            (*handle)->reference = 1;
            (*handle)->root = NULL;
            (*handle)->next = NULL;
        }
    }

    // Return any error produced
    return err;
}


/*
    Parameters  : src           - The backup tree handle to clone.
                  dest          - Variable to receive the cloned handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone a backup tree handle. The cloned handle references the
                  same backup tree.
*/
os_error *backtree_clone(backtree_handle src, backtree_handle *dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        // Increment the reference count
        src->reference++;

        // Copy the handle
        *dest = src;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Variable containing the backup tree handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy a previously created backup tree.
*/
os_error *backtree_destroy(backtree_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !*handle) err = &err_bad_parms;
    else
    {
        // Decrement the reference count
        if (!--(*handle)->reference)
        {
            // Free the file records
            backtree_free((*handle)->root);

            // Free the memory used by the handle
            MEM_FREE(*handle);
        }

        // Clear the handle
        if (!err) *handle = BACKTREE_NONE;
    }

    // Return any error produced
    return err;
}
