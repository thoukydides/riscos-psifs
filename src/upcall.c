/*
    File        : upcall.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Utility functions for the PsiFS module.

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
#include "upcall.h"

// Include clib header files
#include <string.h>

// Include oslib header files
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"

// Inclue project header files
#include "debug.h"
#include "err.h"
#include "fs.h"

// Search timeout in centiseconds
#define UPCALL_TIMEOUT (100)

// Search status
static bits upcall_count = 0;

/*
    Parameters  : name          - The name of the disc.
                  retry         - Variable to receive whether a reply should be
                                  performed.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to request that a remote device with the
                  specified disc is connected.
*/
os_error *upcall_search(const char *name, bool *retry)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("upcall_search '%s'", name))

    // Check parameters
    if (!name || !retry) err = &err_bad_parms;
    else
    {
        bool cancel;

        // Perform the UpCall
        err = xupcall_media_not_present(psifs_FS_NUMBER_PSIFS, name, -1,
                                        upcall_count, UPCALL_TIMEOUT,
                                        "remote system or device", &cancel);

        // Increment the iteration count
        if (!err) upcall_count++;

        // Set the retry flag
        if (!err) *retry = !cancel;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to end a search for a disc.
*/
os_error *upcall_search_end(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("upcall_search_end"))

    // Reset the iteration count
    upcall_count = 0;

    // Perform the UpCall
    err = xupcall_media_search_end();

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The filename in external format.
                  info          - The details for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to indicate that the specified object has
                  been added.
*/
os_error *upcall_added(const char *path, const fs_info *info)
{
    os_error *err = NULL;

    // Check parameters
    if (!path || !info) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("upcall_added '%s'", path))

        // Perform the UpCall
        if (info->obj_type == fileswitch_IS_DIR)
        {
            // A directory has been created
            err = xupcallfile_create_dir(path,
                                         info->load_addr, info->exec_addr,
                                         0, NULL, FS_INFO);
        }
        else
        {
            // A file has been created
            err = xupcallfile_create(path,
                                     info->load_addr, info->exec_addr,
                                     (const byte *) 0,
                                     (const byte *) info->size, NULL,
                                     FS_INFO);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The filename in external format.
                  info          - The details for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to indicate that the specified object has
                  been removed.
*/
os_error *upcall_removed(const char *path, const fs_info *info)
{
    os_error *err = NULL;

    // Check parameters
    if (!path || !info) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("upcall_removed '%s'", path))

        // Perform the UpCall
        err = xupcallfile_delete(path, NULL, FS_INFO);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The filename in external format.
                  info          - The details for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an UpCall to indicate that the specified object has
                  been changed.
*/
os_error *upcall_changed(const char *path, const fs_info *info)
{
    os_error *err = NULL;

    // Check parameters
    if (!path || !info) err = &err_bad_parms;
    else
    {
        fs_pathname dest;
        const char *ptr;

        DEBUG_PRINTF(("upcall_changed '%s'", path))

        // Construct the new pathname
        ptr = strrchr(path, FS_CHAR_SEPARATOR);
        ptr = ptr ? ptr + 1 : path;
        if (sizeof(dest) <= ptr - path + strlen(info->name))
        {
            err = &err_bad_name;
        }
        else
        {
            strncpy(dest, path, ptr - path);
            strcpy(dest + (ptr - path), info->name);
        }

        // Perform the UpCall
        if (!err)
        {
            err = xupcallfile_set_args(path, info->load_addr, info->exec_addr,
                                       info->attr, NULL, FS_INFO);
        }
        if (!err && strcmp(path, dest))
        {
            err = xupcallfscontrol_rename(path, dest, NULL, NULL, FS_INFO);
        }
    }

    // Return any error produced
    return err;
}
