/*
    File        : fs.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Core filing system implementation for the PsiFS module.

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
#include "fs.h"

// Include clib header files
#include <ctype.h>
#include <string.h>

// Include oslib header files
#include "oslib/macros.h"

// Include project header files
#include "cache.h"
#include "ctrl.h"
#include "debug.h"
#include "err.h"
#include "unified.h"
#include "wildcard.h"

// Shared foreground cache operation
static cache_cmd fs_cmd;
static cache_reply fs_reply;

// The default drive
char fs_default_drive = 'C';

/*
    Parameters  : drive - The drive letter to check.
    Returns     : bool  - Is it a valid drive letter.
    Description : Check whether the specified drive letter is valid.
*/
static bool fs_is_drive(char drive)
{
    // Return whether the specified character is a valid drive letter
    return isalpha(drive) || (drive == FS_CHAR_DRIVE_ALL);
}

/*
    Parameters  : src           - The external or internal path to convert.
                  dest          - Variable to receive a pointer to the result.
                                  This may be set to NULL if the operation
                                  fails.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert the specified path to the internal representation.
                  It is an error if either the disc or directory do not
                  exist. This does not remove any wildcards from the leaf name,
                  or check for existence of the specified object. The result
                  should be copied if required; the value is stored in a
                  temporary buffer.
*/
os_error *fs_internal_name(const char *src, const char **dest)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest) err = &err_bad_parms;
    else if (!*src
             || ctrl_strchr(src, FS_CHAR_URD)
             || ctrl_strchr(src, FS_CHAR_CSD)
             || ctrl_strchr(src, FS_CHAR_PARENT)
             || ctrl_strchr(src, FS_CHAR_CSL)
             || ctrl_strchr(src, FS_CHAR_PSD))
    {
        err = &err_bad_name;
    }
    else
    {
        static fs_pathname path;
        const char *ptr;
        char *leaf;

        DEBUG_PRINTF(("fs_internal_name src='%s'", src))

        // Start with the standard path
        path[0] = FS_CHAR_DISC;
        path[1] = fs_default_drive;
        path[2] = FS_CHAR_SEPARATOR;
        path[3] = FS_CHAR_ROOT;
        path[4] = '\0';

        // Use the specified drive or root if possible
        if (src[0] == FS_CHAR_DISC)
        {
            static fs_discname disc;

            // Find the end of the drive name
            src++;
            ptr = ctrl_strchr(src, FS_CHAR_SEPARATOR);
            if (!ptr) ptr = ctrl_strchr(src, '\0');

            // Attempt to convert the drive name
            if (sizeof(disc) <= (ptr - src)) err = &err_bad_name;
            else
            {
                strncpy(disc, src, ptr - src);
                disc[ptr - src] = '\0';
                err = fs_internal_drive(disc, &path[1]);
            }

            // Skip over the drive name and any root specifier
            if (!err)
            {
                src = *ptr ? ptr + 1 : ptr;
                if ((src[0] == FS_CHAR_ROOT)
                    && (!src[1] || (src[1] == FS_CHAR_SEPARATOR)))
                {
                    src = src[1] ? src + 2 : src + 1;
                }
            }

            // Special case if grouped drive
            if (!err && (path[1] == FS_CHAR_DRIVE_ALL) && *src)
            {
                // Find the end of the drive name
                ptr = ctrl_strchr(src, FS_CHAR_SEPARATOR);
                if (!ptr) ptr = ctrl_strchr(src, '\0');

                // Attempt to convert the drive name
                if (sizeof(disc) <= (ptr - src)) err = &err_bad_name;
                else
                {
                    strncpy(disc, src, ptr - src);
                    disc[ptr - src] = '\0';
                    err = fs_internal_drive(disc, &path[1]);
                }

                // Skip over the drive name
                if (!err) src = *ptr ? ptr + 1 : ptr;
            }
        }
        else
        {
            const fs_drive *info;

            // Use all or part of the CSD
            if ((src[0] == FS_CHAR_ROOT)
                && (!src[1] || (src[1] == FS_CHAR_SEPARATOR)))
            {
                // Skip the root directory specifier
                src = src[1] ? src + 2 : src + 1;
            }

            // Verify that the drive is available
            err = fs_drive_info(fs_default_drive, FALSE, &info);
            if (!err && !info->present) err = &err_drive_empty;
        }

        // Canonicalise the rest of the path (excluding the leaf name)
        ptr = ctrl_strchr(src, FS_CHAR_SEPARATOR);
        while (!err && ptr)
        {
            const fs_info *info;

            // Prepare to add a new leaf name
            leaf = strchr(path, '\0');
            *leaf++ = FS_CHAR_SEPARATOR;

            // Check whether the path is valid
            if (!ptr[1] || (ptr == src)) err = &err_bad_name;

            // Attempt to read the object details
            if (!err && ((path + sizeof(path) - leaf) <= (ptr - src)))
            {
                err = &err_bad_name;
            }
            if (!err)
            {
                ctrl_strncpy(leaf, src, ptr - src);
                leaf[ptr - src] = '\0';
                err = fs_file_info(path, &info);
            }

            // Substitute the canonicalised leaf if possible
            if (!err)
            {
                if (info->obj_type != fileswitch_IS_DIR)
                {
                    err = &err_not_found;
                }
                else if ((path + sizeof(path) - leaf) <= strlen(info->name))
                {
                    err = &err_bad_name;
                }
                else strcpy(leaf, info->name);
            }

            // Update the pointers
            if (!err)
            {
                src = ptr + 1;
                ptr = ctrl_strchr(src, FS_CHAR_SEPARATOR);
            }
        }

        // Append the leaf name
        if (!err && *src)
        {
            leaf = strchr(path, '\0');
            *leaf++ = FS_CHAR_SEPARATOR;
            if ((path + sizeof(path) - leaf) <= ctrl_strlen(src))
            {
                err = &err_bad_name;
            }
            else ctrl_strcpy(leaf, src);
        }

        // Validate the result
        if (!err && (path[1] != FS_CHAR_DRIVE_ALL))
        {
            err = unified_validate(path, path, sizeof(path));
        }

        // Return a pointer to the result
        *dest = !err ? path : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The external or internal path to convert.
                  dest          - Variable to receive a pointer to the result.
                                  This may be set to NULL if the operation
                                  fails.
                  info          - Variable to receive a pointer to the details
                                  for the item. The object type is set to
                                  fileswitch_NOT_FOUND if it does not exist.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert the specified path to the internal representation.
                  It is an error if either the disc or directory do not exist,
                  but it is not an error if the object does not exist. This
                  removes all wildcards from the path. The results should be
                  copied if required; the values are stored in temporary
                  buffers.
*/
os_error *fs_internal_name_resolve(const char *src, const char **dest,
                                   const fs_info **info)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || (!dest && !info)) err = &err_bad_parms;
    else
    {
        static fs_pathname path;
        const char *ptr;
        const fs_info *details;

        DEBUG_PRINTF(("fs_internal_name_resolve src='%s'", src))

        // Start by canonicalising the path (except the leaf)
        err = fs_internal_name(src, &ptr);
        if (!err) strcpy(path, ptr);

        // Attempt to read the object details
        if (!err) err = fs_file_info(path, &details);

        // Replace the leaf name if the object was found
        if (!err && (details->obj_type != fileswitch_NOT_FOUND) && path[4])
        {
            char *leaf = strrchr(path, FS_CHAR_SEPARATOR) + 1;
            if ((path + sizeof(path) - leaf) <= strlen(details->name))
            {
                err = &err_bad_name;
            }
            else strcpy(leaf, details->name);
        }

        // Return pointers to the result
        if (dest) *dest = !err ? path : NULL;
        if (info) *info = !err ? details : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The internal path to convert.
                  dest          - Variable to receive a pointer to the result.
                                  This may be set to NULL if the operation
                                  fails.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert the specified path to the canonical external
                  representation. This does not remove any wildcards from the
                  path, or check for existence of the specified object. The
                  result should be copied if required; the value is stored in
                  a temporary buffer.
*/
os_error *fs_external_name(const char *src, const char **dest)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest) err = &err_bad_parms;
    else if ((src[0] != FS_CHAR_DISC) || !fs_is_drive(src[1]))
    {
        err = &err_bad_name;
    }
    else
    {
        static fs_pathname path;
        const fs_drive *info;

        DEBUG_PRINTF(("fs_external_name src='%s'", src))

        // Only the drive letter needs converting
        err = fs_drive_info(src[1], TRUE, &info);
        if (!err && info->present)
        {
            // Use the actual disc name if it fits
            if (sizeof(path) < strlen(src) + strlen(info->name))
            {
                err = &err_bad_name;
            }
            else
            {
                path[0] = FS_CHAR_DISC;
                strcpy(path + 1, info->name);
                strcat(path, src + 2);
            }
        }
        else
        {
            // Keep the existing name if it fits
            if (sizeof(path) <= strlen(src)) err = &err_bad_name;
            else
            {
                err = NULL;
                strcpy(path, src);
            }
        }

        // Return a pointer to the result
        *dest = !err ? path : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The path to check.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Check whether the specified path contains wildcards, and
                  return an error if it does.
*/
os_error *fs_wildcards(const char *path)
{
    os_error *err = NULL;

    // Check parameters
    if (!path) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("fs_wildcards path='%s'", path))

        // Check for wildcards
        if (strchr(path, FS_CHAR_WILD_SINGLE) || strchr(path, FS_CHAR_WILD_ANY))
        {
            err = &err_wild_cards;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : name          - The name of the disc or drive.
                  drive         - Variable to receive the corresponding drive
                                  letter.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert the specified disc or drive name to a drive letter.
                  It is an error if the disc is not present.
*/
os_error *fs_internal_drive(const char *name, char *drive)
{
    os_error *err = NULL;

    // Check parameters
    if (!name || !drive) err = &err_bad_parms;
    else
    {
        const fs_drive *info;

        DEBUG_PRINTF(("fs_internal_drive name='%s'", name))

        // Special case if drive letter already specified
        if (fs_is_drive(name[0]) && !name[1])
        {
            // Just check that the drive is available
            err = fs_drive_info(name[0], FALSE, &info);
            if (!err)
            {
                if (info->present) *drive = toupper(name[0]);
                else err = &err_drive_empty;
            }
        }
        else
        {
            char i;
            int count = 0;

            // Check all possible drives for a matching name
            for (i = psifs_DRIVE_FIRST_UPPER; i <= psifs_DRIVE_LAST_UPPER; i++)
            {
                if (!fs_drive_info(i, FALSE, &info) && info->present
                    && !wildcard_cmp(name, info->name))
                {
                    *drive = i;
                    count++;
                }
            }

            // Also check the grouped drive
            if (!fs_drive_info(FS_CHAR_DRIVE_ALL, FALSE, &info) && info->present
                && !wildcard_cmp(name, info->name))
            {
                *drive = FS_CHAR_DRIVE_ALL;
                count++;
            }

            // Ensure that exactly one match was found
            if (!err)
            {
                if (!count) err = &err_disc_not_found;
                else if (1 < count) err = &err_ambig_disc;
            }
        }

        // Clear the result if not found
        if (err) *drive = ' ';
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : drive         - The drive to read the details for.
                  mangle        - Should the drive name be mangled if invalid.
                  info          - Variable to receive a pointer to the details
                                  for the drive.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details for the specified drive. It is not an
                  error for the drive to be empty. The results should be
                  copied if required; the values are stored in a temporary
                  buffer.
*/
os_error *fs_drive_info(char drive, bool mangle, const fs_drive **info)
{
    os_error *err = NULL;

    // Check parameters
    if (!fs_is_drive(drive) || !info) err = &err_bad_parms;
    else
    {
        static fs_drive details;

        DEBUG_PRINTF(("fs_drive_info drive=%c", drive))

        if (drive == FS_CHAR_DRIVE_ALL)
        {
            char i;

            // Make up the drive details
            details.present = TRUE;
            details.rom = FALSE;
            strcpy(details.name, FS_NAME_DRIVE_ALL);
            details.free_low = 0;
            details.free_high = 0;
            details.size_low = 0;
            details.size_high = 0;

            // Calculate the cumulative disc size and free space
            for (i = psifs_DRIVE_FIRST_UPPER; i <= psifs_DRIVE_LAST_UPPER; i++)
            {
                // Read the drive details
                fs_cmd.op = CACHE_DRIVE;
                fs_cmd.data.drive.drive = i;
                if (!cache_fore(&fs_cmd, &fs_reply, TRUE))
                {
                    // Update the free space
                    details.free_low += fs_reply.drive.drive.free_low;
                    if (details.free_low < fs_reply.drive.drive.free_low)
                    {
                        details.free_high++;
                    }
                    details.free_high += fs_reply.drive.drive.free_high;

                    // Update the disc size
                    details.size_low += fs_reply.drive.drive.size_low;
                    if (details.size_low < fs_reply.drive.drive.size_low)
                    {
                        details.size_high++;
                    }
                    details.size_high += fs_reply.drive.drive.size_high;
                }
            }
        }
        else
        {
            // Read the drive details
            fs_cmd.op = CACHE_DRIVE;
            fs_cmd.data.drive.drive = drive;
            err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            if (!err)
            {
                details = fs_reply.drive.drive;
                if (mangle && (!details.present || (strlen(details.name) <= 1)))
                {
                    details.name[0] = drive;
                    details.name[1] = '\0';
                }
            }
        }

        // Return a pointer to the result
        *info = !err ? &details : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : drive         - The drive to change the name of.
                  info          - The new name for the disc.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Change the name of the specified disc.
*/
os_error *fs_drive_name(char drive, const char *name)
{
    os_error *err = NULL;

    // Check parameters
    if (!fs_is_drive(drive) || !name) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("fs_drive_name drive=%c, name='%s'", drive, name))

        // Not possible to rename the grouped drive
        if (drive == FS_CHAR_DRIVE_ALL) err = &err_write_prot;
        else
        {
            // Check the length of the name
            if (sizeof(fs_cmd.data.name.name) <= strlen(name))
            {
                err = &err_bad_parms;
            }

            // Perform the rename
            if (!err)
            {
                fs_cmd.op = CACHE_NAME;
                fs_cmd.data.name.drive = drive;
                strcpy(fs_cmd.data.name.name, name);
                err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The object to read the details for.
                  info          - Variable to receive a pointer to the details
                                  for the item. The object type is set to
                                  fileswitch_NOT_FOUND if it does not exist.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details for the specified object. It is not an
                  error if the object does not exist. The results should be
                  copied if required; the values are stored in a temporary
                  buffer.
*/
os_error *fs_file_info(const char *path, const fs_info **info)
{
    os_error *err = NULL;

    // Check parameters
    if (!path || !info) err = &err_bad_parms;
    else
    {
        static fs_info details;

        DEBUG_PRINTF(("fs_file_info path='%s'", path))

        // Special case if the grouped drive
        if ((path[0] == FS_CHAR_DISC) && (path[1] == FS_CHAR_DRIVE_ALL)
            && (path[2] == FS_CHAR_SEPARATOR) && (path[3] == FS_CHAR_ROOT))
        {
            // Start with defaults
            details.load_addr = 0;
            details.exec_addr = 0;
            details.size = 0;
            details.attr = fileswitch_ATTR_OWNER_LOCKED;
            details.obj_type = fileswitch_IS_DIR;

            // Handle the special cases
            if (!path[4])
            {
                // Root of the grouped drive
                details.name[0] = FS_CHAR_ROOT;
                details.name[1] = '\0';
            }
            else if (path[4] == FS_CHAR_SEPARATOR)
            {
                char drive;
                const fs_drive *info;

                // Subdirectory (another drive)
                if (!fs_internal_drive(path + 5, &drive)
                    && !fs_drive_info(drive, TRUE, &info) && info->present)
                {
                    strcpy(details.name, info->name);
                }
                else
                {
                    // Drive not found
                    details.obj_type = fileswitch_NOT_FOUND;
                }
            }
            else err = &err_bad_name;
        }
        else
        {
            // Read the file details
            fs_cmd.op = CACHE_INFO;
            strcpy(fs_cmd.data.info.path, path);
            err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            if (!err) details = fs_reply.info.info;
        }

        // Return a pointer to the result
        *info = !err ? &details : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - The directory to search.
                  match         - Wildcarded leaf name to match.
                  offset        - Variable containing the offset of the first
                                  item to read (0 for first call). This is
                                  updated ready for the next call (-1 if end).
                  buffer        - Pointer to buffer to receive the entries.
                  size          - The number of entries to read.
                  read          - The number of entries read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Enumerate items matching the supplied specification.
*/
os_error *fs_enumerate(const char *path, const char *match, int *offset,
                       fs_info *buffer, bits size, bits *read)
{
    os_error *err = NULL;

    // Check parameters
    if (!path || !match || !offset || !buffer || !size || !read)
    {
        err = &err_bad_parms;
    }
    else
    {
        DEBUG_PRINTF(("fs_enumerate path='%s', match='%s', offset=%i, size=%u", path, match, *offset, size))

        // Special case if the grouped drive
        if ((path[0] == FS_CHAR_DISC) && (path[1] == FS_CHAR_DRIVE_ALL)
            && (path[2] == FS_CHAR_SEPARATOR) && (path[3] == FS_CHAR_ROOT)
            && !path[4])
        {
            char i;
            const fs_drive *info;

            // Check all possible drives for a matching name
            *read = 0;
            for (i = psifs_DRIVE_FIRST_UPPER; i <= psifs_DRIVE_LAST_UPPER; i++)
            {
                if (size && (*offset < i) && !fs_drive_info(i, TRUE, &info)
                    && info->present && !wildcard_cmp(match, info->name))
                {
                    // Add this entry to the buffer
                    buffer->load_addr = 0;
                    buffer->exec_addr = 0;
                    buffer->size = 0;
                    buffer->attr = fileswitch_ATTR_OWNER_LOCKED;
                    buffer->obj_type = fileswitch_IS_DIR;
                    strcpy(buffer->name, info->name);

                    // Update the status
                    buffer++;
                    *offset = i;
                    size--;
                    (*read)++;
                }
            }

            // Change the offset if all done
            if (size) *offset = -1;
        }
        else
        {
            // Check the length of the names
            if ((sizeof(fs_cmd.data.enumerate.path) <= strlen(path))
                || (sizeof(fs_cmd.data.enumerate.match) <= strlen(match)))
            {
                err = &err_bad_parms;
            }

            // Perform the enumeration
            if (!err)
            {
                fs_cmd.op = CACHE_ENUMERATE;
                strcpy(fs_cmd.data.enumerate.path, path);
                strcpy(fs_cmd.data.enumerate.match, match);
                fs_cmd.data.enumerate.offset = *offset;
                fs_cmd.data.enumerate.buffer = buffer;
                fs_cmd.data.enumerate.size = size;
                err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            }
            if (!err)
            {
                *offset = fs_reply.enumerate.offset;
                *read = fs_reply.enumerate.read;
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - Non-wildcarded path of the directory.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create a directory.
*/
os_error *fs_mkdir(const char *path)
{
    os_error *err = NULL;

    // Check parameters
    if (!path) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("fs_mkdir path='%s'", path))

        // Not possible to modify the grouped drive
        if (path[1] == FS_CHAR_DRIVE_ALL) err = &err_write_prot;
        else
        {
            // Check the length of the name
            if (sizeof(fs_cmd.data.mkdir.path) <= strlen(path))
            {
                err = &err_bad_parms;
            }

            // Create the directory
            if (!err)
            {
                fs_cmd.op = CACHE_MKDIR;
                strcpy(fs_cmd.data.mkdir.path, path);
                err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - Non-wildcarded path of object to delete.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Delete the specified object. No error will be returned if the
                  object does not exist.
*/
os_error *fs_remove(const char *path)
{
    os_error *err = NULL;

    // Check parameters
    if (!path) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("fs_remove path='%s'", path))

        // Not possible to modify the grouped drive
        if (path[1] == FS_CHAR_DRIVE_ALL) err = &err_write_prot;
        else
        {
            // Check the length of the name
            if (sizeof(fs_cmd.data.remove.path) <= strlen(path))
            {
                err = &err_bad_parms;
            }

            // Perform the deletion
            if (!err)
            {
                fs_cmd.op = CACHE_REMOVE;
                strcpy(fs_cmd.data.remove.path, path);
                err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - Current non-wildcarded path of the file.
                  dest          - New non-wildcarded path of the file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Rename the specified object. An error may be returned if
                  the rename is not simple, i.e. not just a case of changing
                  the object's catalogue entry.
*/
os_error *fs_rename(const char *src, const char *dest)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("fs_rename src='%s', dest='%s'", src, dest))

        // Renaming a grouped drive object should change a disc name
        if (src[1] == FS_CHAR_DRIVE_ALL) err = &err_write_prot;
        else
        {
            // Check the length of the names
            if ((sizeof(fs_cmd.data.rename.src) <= strlen(src))
                || (sizeof(fs_cmd.data.rename.dest) <= strlen(dest)))
            {
                err = &err_bad_parms;
            }

            // Perform the rename
            if (!err)
            {
                fs_cmd.op = CACHE_RENAME;
                strcpy(fs_cmd.data.rename.src, src);
                strcpy(fs_cmd.data.rename.dest, dest);
                err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - Non-wildcarded path of the object.
                  attr          - The required attributes for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the attributes for the specified object.
*/
os_error *fs_access(const char *path, fileswitch_attr attr)
{
    os_error *err = NULL;

    // Check parameters
    if (!path) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("fs_access path='%s', att=0x%02X", path, attr))

        // Not possible to modify the grouped drive
        if (path[1] == FS_CHAR_DRIVE_ALL) err = &err_write_prot;
        else
        {
            // Check the length of the name
            if (sizeof(fs_cmd.data.access.path) <= strlen(path))
            {
                err = &err_bad_parms;
            }

            // Perform the access
            if (!err)
            {
                fs_cmd.op = CACHE_ACCESS;
                strcpy(fs_cmd.data.access.path, path);
                fs_cmd.data.access.attr = attr;
                err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - Non-wildcarded path of the object.
                  load          - The load address for the specified object.
                  exec          - The execute address for the specified object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the load and execute addresses for the specified object.
*/
os_error *fs_stamp(const char *path, bits load, bits exec)
{
    os_error *err = NULL;

    // Check parameters
    if (!path) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("fs_stamp path='%s', load=0x%08x, exec=0x%08x", path, load, exec))

        // Not possible to modify the grouped drive
        if (path[1] == FS_CHAR_DRIVE_ALL) err = &err_write_prot;
        else
        {
            // Check the length of the name
            if (sizeof(fs_cmd.data.stamp.path) <= strlen(path))
            {
                err = &err_bad_parms;
            }

            // Perform the stamp
            if (!err)
            {
                fs_cmd.op = CACHE_STAMP;
                strcpy(fs_cmd.data.stamp.path, path);
                fs_cmd.data.stamp.load = load;
                fs_cmd.data.stamp.exec = exec;
                err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : path          - Non-wildcarded path of the file.
                  mode          - The mode in which the object should be opened.
                  handle        - The FileSwitch file handle for the object,
                                  or 0 for none.
                  object        - Variable to receive the filing system handle
                                  for the object, or FS_NONE if failed to open.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Open the specified object. This may optionally create a new
                  object or overwrite an existing one. Failure to allocate
                  memory is not an error
*/
os_error *fs_open(const char *path, fs_mode mode, os_fw handle,
                  fs_handle *object)
{
    os_error *err = NULL;

    // Check parameters
    if (!path || !object) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("fs_open path='%s', mode=%u, handle=%u", path, mode, handle))

        // Not possible to open objects on the grouped drive
        if (path[1] == FS_CHAR_DRIVE_ALL) err = &err_write_prot;
        else
        {
            // Check the length of the name
            if (sizeof(fs_cmd.data.open.path) <= strlen(path))
            {
                err = &err_bad_parms;
            }

            // Perform the stamp
            if (!err)
            {
                fs_cmd.op = CACHE_OPEN;
                strcpy(fs_cmd.data.open.path, path);
                fs_cmd.data.open.mode = mode;
                fs_cmd.data.open.handle = handle;
                err = cache_fore(&fs_cmd, &fs_reply, TRUE);
            }
        }
        *object = !err ? fs_reply.open.handle : FS_NONE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : object        - Filing system handle for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Close the specified object.
*/
os_error *fs_close(fs_handle object)
{
    os_error *err = NULL;

    // Check parameters
    if (object == FS_NONE) err = &err_channel;
    else
    {
        DEBUG_PRINTF(("fs_close object=%p", object))

        // Close the object
        fs_cmd.op = CACHE_CLOSE;
        fs_cmd.data.close.handle = object;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : object        - Filing system handle for the object.
                  info          - Variable to receive a pointer to the details
                                  for the item.
                  path          - Vairable to receive a pointer to the name.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details for the specified open file. It is an error
                  if either the handle or file is not valid. The results should
                  be copied if required; the values are stored in temporary
                  buffers.
*/
os_error *fs_args(fs_handle object, const fs_open_info **info,
                  const char **path)
{
    os_error *err = NULL;

    // Check parameters
    if (!info && !path) err = &err_bad_parms;
    else if (object == FS_NONE) err = &err_channel;
    else
    {
        static fs_open_info details;
        static fs_pathname name;

        DEBUG_PRINTF(("fs_args object=%p", object))

        // Read the object details
        fs_cmd.op = CACHE_ARGS;
        fs_cmd.data.args.handle = object;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
        if (!err && (sizeof(name) <= strlen(fs_reply.args.path)))
        {
            err = &err_bad_name;
        }
        if (!err)
        {
            details = fs_reply.args.info;
            strcpy(name, fs_reply.args.path);
        }

        // Return pointers to the results
        if (info) *info = !err ? &details : NULL;
        if (path) *path = !err ? name : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : file          - Filing system handle for the file.
                  buffer        - The buffer to write the data into.
                  offset        - The initial file offset to read from.
                  bytes         - The number of bytes to read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read bytes from the specified object. Bytes past the end of
                  the file are returned as zeros. It is an error to read from
                  a directory.
*/
os_error *fs_read(fs_handle file, void *buffer, bits offset, bits bytes)
{
    os_error *err = NULL;

    // Check parameters
    if (!buffer) err = &err_bad_parms;
    else if (file == FS_NONE) err = &err_channel;
    else
    {
        DEBUG_PRINTF(("fs_read file=%p, buffer=%p, offset=%u, bytes=%u", file, buffer, offset, bytes))

        // Attempt the read
        fs_cmd.op = CACHE_READ;
        fs_cmd.data.read.handle = file;
        fs_cmd.data.read.offset = offset;
        fs_cmd.data.read.length = bytes;
        fs_cmd.data.read.buffer = buffer;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : file          - Filing system handle for the file.
                  buffer        - The buffer to read the data from.
                  offset        - The initial file offset to write at.
                  bytes         - The number of bytes to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write bytes to the specified object, increasing the file
                  extent if required. If the specified offset is beyond the
                  end of the file then the gap is padded with zeros. It is
                  an error to write to either a directory or a file without
                  write access.
*/
os_error *fs_write(fs_handle file, const void *buffer, bits offset, bits bytes)
{
    os_error *err = NULL;

    // Check parameters
    if (!buffer) err = &err_bad_parms;
    else if (file == FS_NONE) err = &err_channel;
    else
    {
        DEBUG_PRINTF(("fs_write file=%p, buffer=%p, offset=%u, bytes=%u", file, buffer, offset, bytes))

        // Attempt the write
        fs_cmd.op = CACHE_WRITE;
        fs_cmd.data.write.handle = file;
        fs_cmd.data.write.offset = offset;
        fs_cmd.data.write.length = bytes;
        fs_cmd.data.write.buffer = buffer;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : file          - Filing system handle for the file.
                  offset        - The initial file offset to write at.
                  bytes         - The number of bytes to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write zeros to the specified object, increasing the file
                  extent if required. It is an error to write to either a
                  directory or a file without write access.
*/
os_error *fs_write_zeros(fs_handle file, bits offset, bits bytes)
{
    os_error *err = NULL;

    // Check parameters
    if (file == FS_NONE) err = &err_channel;
    else
    {
        DEBUG_PRINTF(("fs_write_zeros file=%p, offset=%u, bytes=%u", file, offset, bytes))

        // Attempt the write
        fs_cmd.op = CACHE_ZERO;
        fs_cmd.data.zero.handle = file;
        fs_cmd.data.zero.offset = offset;
        fs_cmd.data.zero.length = bytes;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : object        - Filing system handle for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Flush any modified data for the specified object being held
                  in buffers.
*/
os_error *fs_flush(fs_handle object)
{
    os_error *err = NULL;

    // Check parameters
    if (object == FS_NONE) err = &err_channel;
    else
    {
        DEBUG_PRINTF(("fs_flush object=%p", object))

        // Attempt the flush
        fs_cmd.op = CACHE_FLUSH;
        fs_cmd.data.flush.handle = object;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : file          - Filing system handle for the file.
                  allocated     - The allocated size for the specified file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the allocated size of the specified file to at least the
                  specified size. If the file is extended then the value of the
                  new data is undefined. It is an error to set the allocated
                  size of a directory or a file without write access.
*/
os_error *fs_allocated(fs_handle file, bits allocated)
{
    os_error *err = NULL;

    // Check parameters
    if (file == FS_NONE) err = &err_channel;
    else
    {
        DEBUG_PRINTF(("fs_allocated file=%p, allocated=%u", file, allocated))

        // Attempt to set the allocated size
        fs_cmd.op = CACHE_ALLOCATED;
        fs_cmd.data.allocated.handle = file;
        fs_cmd.data.allocated.size = allocated;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : file          - Filing system handle for the file.
                  extent        - The extent for the specified file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the extent of the specified file. The file is either
                  truncated or padded with zeros as appropriate. It is an error
                  to set the extent of a directory or a file without write
                  access.
*/
os_error *fs_extent(fs_handle file, bits extent)
{
    os_error *err = NULL;

    // Check parameters
    if (file == FS_NONE) err = &err_channel;
    else
    {
        DEBUG_PRINTF(("fs_extent file=%p, extent=%u", file, extent))

        // Attempt to set the extent
        fs_cmd.op = CACHE_EXTENT;
        fs_cmd.data.extent.handle = file;
        fs_cmd.data.extent.size = extent;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : file          - Filing system handle for the file.
                  sequential    - The sequential pointer.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the sequential pointer for the specified file. If the
                  new pointer is greater than the current file extent then if
                  possible the file is extended with zeros and the extent is
                  set to the new sequential pointer, otherwise an error is
                  returned. It is an error to set the sequential pointer for
                  a directory.
*/
os_error *fs_sequential(fs_handle file, bits sequential)
{
    os_error *err = NULL;

    // Check parameters
    if (file == FS_NONE) err = &err_channel;
    else
    {
        DEBUG_PRINTF(("fs_sequential file=%p, sequential=%u", file, sequential))

        // Attempt to set the extent
        fs_cmd.op = CACHE_SEQUENTIAL;
        fs_cmd.data.sequential.handle = file;
        fs_cmd.data.sequential.offset = sequential;
        err = cache_fore(&fs_cmd, &fs_reply, TRUE);
    }

    // Return any error produced
    return err;
}
