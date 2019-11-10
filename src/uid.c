/*
    File        : uid.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : UID and extension conversion for the PsiFS module.

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
#include "uid.h"

// Include clib header files
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "oslib/mimemap.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "crc.h"
#include "ctrl.h"
#include "err.h"
#include "mem.h"

// Extension separator
#define UID_EXT_SEPARATOR '/'

// Invalid characters in extensions
#define UID_EXT_INVALID " \"#$%&*./:?@\\^|"

// Range of valid file types
#define UID_MIN_TYPE (0x000)
#define UID_MAX_TYPE (0xfff)

// Filetype to use if no mappings
#define UID_DEFAULT_TYPE (osfile_TYPE_DATA)
static bits uid_default_type = UID_DEFAULT_TYPE;

// Mappings for extensions
typedef struct uid_ext_map
{
    struct uid_ext_map *next;
    struct uid_ext_map *prev;
    char *ext;
    bits type;
} uid_ext_map;
static uid_ext_map *uid_ext_list = NULL;

// Mappings for UIDs
typedef struct uid_uid_map
{
    struct uid_uid_map *next;
    struct uid_uid_map *prev;
    epoc32_file_uid uid;
    bits type;
} uid_uid_map;
static uid_uid_map *uid_uid_list = NULL;

// Should the MimeMap module be used
static bool uid_use_mime_map = TRUE;

/*
    Parameters  : uid           - The UID to check.
                  uid4          - Variable to receive the checksum.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Calculate the checksum for a UID.
*/
os_error *uid_checksum_uid(const epoc32_file_uid *uid, bits *uid4)
{
    os_error *err = NULL;

    // Check the parameters
    if (!uid || !uid4) err = &err_bad_parms;
    else
    {
        const byte *ptr = (const byte *) &uid->uid1;
        crc_state crc_low;
        crc_state crc_high;
        bits i;

        // Calculate the checksum
        crc_reset(&crc_low);
        crc_reset(&crc_high);
        for (i = 0; i < 6; i++)
        {
            crc_update(&crc_low, *ptr++);
            crc_update(&crc_high, *ptr++);
        }

        // Combine the CRC values to form the checksum
        *uid4 = ((bits) crc_lsb(&crc_low))
                | (((bits) crc_msb(&crc_low)) << 8)
                | (((bits) crc_lsb(&crc_high)) << 16)
                | (((bits) crc_msb(&crc_high)) << 24);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : a     - The first UID to compare.
                  b     - The second UID to compare.
    Returns     : bool  - Do the UIDs match.
    Description : Compare two UIDs.
*/
static bool uid_match_uid(const epoc32_file_uid *a, const epoc32_file_uid *b)
{
    // Return the result of the comparison
    return (a->uid1 == b->uid1) && (a->uid2 == b->uid2) && (a->uid3 == b->uid3);
}

/*
    Parameters  : a     - The first extension to compare.
                  b     - The second extension to compare.
    Returns     : bool  - Do the extensions match.
    Description : Compare two file extensions.
*/
static bool uid_match_ext(const char *a, const char *b)
{
    bool match = TRUE;

    // Perform a case insensitive comparison
    while (match && (*a || *b)) match = toupper(*a++) == toupper(*b++);

    // Return the result of the comparison
    return match;
}

/*
    Parameters  : name  - The filename in RISC OS format.
                  uid   - The UID to map.
    Returns     : bits  - The corresponding file type.
    Description : Convert the specified filename and file UID into a RISC OS
                  file type. If a valid UID is specified then that is used,
                  otherwise the extension is processed.
*/
bits uid_map_type(const char *name, const epoc32_file_uid *uid)
{
    bits type = uid_default_type;
    bits mime_type;
    bool found = FALSE;
    uid_uid_map *uid_ptr = uid_uid_list;
    uid_ext_map *ext_ptr = uid_ext_list;

    // Try to match the UID first
    while (!found && uid && uid_ptr)
    {
        if (uid_match_uid(uid, &uid_ptr->uid))
        {
            type = uid_ptr->type;
            found = TRUE;
        }
        else uid_ptr = uid_ptr->next;
    }

    // Try to match the extension
    if (!found && name)
    {
        const char *ptr = strrchr(name, UID_EXT_SEPARATOR);
        name = ptr ? ptr + 1 : "";
    }
    while (!found && name && ext_ptr)
    {
        if (uid_match_ext(name, ext_ptr->ext))
        {
            type = ext_ptr->type;
            found = TRUE;
        }
        else ext_ptr = ext_ptr->next;
    }

    // As a last resort try using the MimeMap module
    if (!found && name && uid_use_mime_map
        && !xmimemaptranslate_extension_to_filetype(name, &mime_type))
    {
        type = mime_type;
        found = TRUE;
    }

    // Return the resulting type
    return type;
}

/*
    Parameters  : str           - The UID string to parse.
                  uid           - Variable to receive the corresponding UID.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Parse the specified UID string.
*/
os_error *uid_parse_uid(const char *str, epoc32_file_uid *uid)
{
    os_error *err = NULL;

    // Check the parameters
    if (!str || !uid) err = &err_bad_parms;
    else
    {
        const char *ptr = str;

        // Check that all characters are hexadecimal digits
        while (isxdigit(*ptr)) ptr++;
        if (*ptr || ((ptr - str) != 24)) err = &err_bad_uid;

        // Parse the UID if valid
        if (!err) sscanf(str, "%8x%8x%8x", &uid->uid1, &uid->uid2, &uid->uid3);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : str           - The file type string to parse.
                  type          - Variable to receive the corresponding file
                                  type.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Parse the specified file type string.
*/
os_error *uid_parse_type(const char *str, bits *type)
{
    os_error *err = NULL;

    // Check the parameters
    if (!str || !type) err = &err_bad_parms;
    else err = xosfscontrol_file_type_from_string(str, type);

    // Return any error produced
    return err;
}


/*
    Parameters  : ext           - The file extension to change.
                  type          - The corresponding file type, or
                                  UID_CLEAR_TYPE to clear the mapping.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Change or clear the mapping for the specified extension.
*/
os_error *uid_ext_mapping(const char *ext, bits type)
{
    os_error *err = NULL;

    // Check the parameters
    if (!ext) err = &err_bad_parms;
    else if (ctrl_strpbrk(ext, UID_EXT_INVALID)) err = &err_bad_ext;
    else
    {
        uid_ext_map *ptr = uid_ext_list;

        // Attempt to find an existing entry that matches
        while (ptr && !uid_match_ext(ext, ptr->ext)) ptr = ptr->next;

        // Should the mapping be changed or cleared
        if (type == UID_CLEAR_TYPE)
        {
            // Clear the mapping if found
            if (ptr)
            {
                // Unlink the entry
                if (ptr->next) ptr->next->prev = ptr->prev;
                if (ptr->prev) ptr->prev->next = ptr->next;
                else uid_ext_list = ptr->next;

                // Free the memory
                MEM_FREE(ptr->ext);
                MEM_FREE(ptr);
            }
        }
        else
        {
            // Create a new entry if necessary
            if (!ptr)
            {
                // Allocate the required memory
                ptr = (uid_ext_map *) MEM_MALLOC(sizeof(uid_ext_map));
                if (!ptr) err = &err_buffer;

                // Copy the extension string
                if (!err)
                {
                    ptr->ext = ctrl_strdup(ext);
                    if (!ptr->ext) err = &err_buffer;
                }

                // Link in the new entry
                if (!err)
                {
                    ptr->next = uid_ext_list;
                    ptr->prev = NULL;
                    if (uid_ext_list) uid_ext_list->prev = ptr;
                    uid_ext_list = ptr;
                }

                // Release the memory if failed
                if (err && ptr) MEM_FREE(ptr);
            }

            // Set the mapping type
            if (!err) ptr->type = type;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : uid           - The file UID to change.
                  type          - The corresponding file type, or
                                  UID_CLEAR_TYPE to clear the mapping.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Change or clear the mapping for the specified UID.
*/
os_error *uid_uid_mapping(const epoc32_file_uid *uid, bits type)
{
    os_error *err = NULL;

    // Check the parameters
    if (!uid) err = &err_bad_parms;
    else
    {
        uid_uid_map *ptr = uid_uid_list;

        // Attempt to find an existing entry that matches
        while (ptr && !uid_match_uid(uid, &ptr->uid)) ptr = ptr->next;

        // Should the mapping be changed or cleared
        if (type == UID_CLEAR_TYPE)
        {
            // Clear the mapping if found
            if (ptr)
            {
                // Unlink the entry
                if (ptr->next) ptr->next->prev = ptr->prev;
                if (ptr->prev) ptr->prev->next = ptr->next;
                else uid_uid_list = ptr->next;

                // Free the memory
                MEM_FREE(ptr);
            }
        }
        else
        {
            // Create a new entry if necessary
            if (!ptr)
            {
                // Allocate the required memory
                ptr = (uid_uid_map *) MEM_MALLOC(sizeof(uid_uid_map));
                if (!ptr) err = &err_buffer;

                // Fill in the details
                if (!err)
                {
                    // Copy the UID
                    ptr->uid = *uid;

                    // Link in the new entry
                    ptr->next = uid_uid_list;
                    ptr->prev = NULL;
                    if (uid_uid_list) uid_uid_list->prev = ptr;
                    uid_uid_list = ptr;
                }
            }

            // Set the mapping type
            if (!err) ptr->type = type;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : type          - The corresponding file type, or
                                  UID_CLEAR_TYPE to clear the mapping.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Change or clear the mapping for untyped or unrecognised
                  files.
*/
os_error *uid_other_mapping(bits type)
{
    os_error *err = NULL;

    // Clearing the mapping restores the default
    uid_default_type = (type == UID_CLEAR_TYPE) ? UID_DEFAULT_TYPE : type;

    // Return any error produced
    return err;
}

/*
    Parameters  : enable        - Should the MimeMap module be used.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enable or disable use of the MimeMap module to convert
                  filename extensions to file types.
*/
os_error *uid_mime_map(bool enable)
{
    os_error *err = NULL;

    // Store the new setting
    uid_use_mime_map = enable;

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : List the current file type mappings.
*/
os_error *uid_list_mapping(void)
{
    os_error *err = NULL;
    bits type;

    // Display a header
    printf("SIBO extension    EPOC UID                 RISC OS type\n");
    printf("----------------- ------------------------ ------------\n");

    // Loop through all possible filetypes
    for (type = UID_MIN_TYPE; !err && (type <= UID_MAX_TYPE); type++)
    {
        uid_uid_map *uid_ptr = uid_uid_list;
        uid_ext_map *ext_ptr = uid_ext_list;
        char desc[13];

        // Convert the file type number to a string
        err = xosfscontrol_read_file_type(type, (bits *) desc,
                                          (bits *) (desc + 4));
        if (!err)
        {
            // Complete the textual conversion
            sprintf(desc + 8, " %03X", type);

            // Find any matching entries
            while (uid_ptr || ext_ptr)
            {
                // Find the next matching UID
                while (uid_ptr && (uid_ptr->type != type))
                {
                    uid_ptr = uid_ptr->next;
                }

                // Find the next matching extension
                while (ext_ptr && (ext_ptr->type != type))
                {
                    ext_ptr = ext_ptr->next;
                }

                // Display any result
                if (uid_ptr || ext_ptr)
                {
                    printf("       %-10.10s ", ext_ptr ? ext_ptr->ext : "");
                    if (uid_ptr)
                    {
                        printf("%08X%08X%08X", uid_ptr->uid.uid1,
                               uid_ptr->uid.uid2, uid_ptr->uid.uid3);
                    }
                    else printf("%-24s", "");
                    printf(" %s\n", desc);
                }

                // Advance to the next entries
                if (uid_ptr) uid_ptr = uid_ptr->next;
                if (ext_ptr) ext_ptr = ext_ptr->next;
            }

            // Special case for untyped or unmatched files
            if (uid_default_type == type)
            {
                printf("     (other)              (other)          %s\n",
                       desc);
            }
        }
    }

    // Display whether the MimeMap module will be used
    if (uid_use_mime_map)
    {
        printf("\nThe MimeMap module will be used for other extensions.\n");
    }

    // Return any error produced
    return err;
}
