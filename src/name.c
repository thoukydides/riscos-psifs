/*
    File        : name.c
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

// Inlcude header file for this module
#include "name.h"

// Include clib header files
#include <ctype.h>
#include <string.h>

// Include project header files
#include "code.h"
#include "epoc.h"
#include "err.h"
#include "fs.h"
#include "sis.h"
#include "util.h"

/*
    Parameters  : src           - The source leaf name.
                  dest          - Variable to hold the result.
                  size          - Maximum number of characters to write to the
                                  destination buffer, including the terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory leafname between two formats.
*/
typedef os_error *(* name_leaf_code)(const char *src, char *dest, size_t size);

/*
    Parameters  : src           - The source leaf name.
                  dest          - Variable to hold the result.
                  size          - Maximum number of characters to write to the
                                  destination buffer, including the terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory leafname from the format used by
                  RISC OS to that used by SIBO. This also applies the configured
                  truncate option to either generate an error or truncate long
                  file names.
*/
static os_error *name_leaf_riscos_to_sibo(const char *src, char *dest,
                                          size_t size)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest || !size) err = &err_bad_parms;
    else
    {
        char *ptr;
        bits len;

        // Find the extension separator
        ptr = strchr(src, FS_CHAR_EXTENSION);
        if (ptr) *ptr = '\0';

        // Process the stub
        err = code_riscos_to_sibo(src, dest, size);

        // Check the length of the stub
        if (!err)
        {
            len = strlen(dest);
            if (!len) err = &err_bad_name;
            else if (EPOC16_MAX_LEAF_STUB < len)
            {
                if (util_truncate()) len = EPOC16_MAX_LEAF_STUB;
                else err = &err_bad_name;
            }
        }
        if (!err)
        {
            dest += len;
            size -= len;
        }

        // Process any extension
        if (!err && ptr)
        {
            // Append an extension to the destination
            if (size <= 1) err = &err_bad_name;
            else
            {
                *dest++ = NAME_CHAR_EXTENSION;
                size--;
            }

            // Process the extension
            if (!err) err = code_riscos_to_sibo(ptr + 1, dest, size);

            // Check the length of the extension
            if (!err)
            {
                len = strlen(dest);
                if (!len) err = &err_bad_name;
                else if (EPOC16_MAX_LEAF_EXTENSION < len)
                {
                    if (util_truncate()) len = EPOC16_MAX_LEAF_EXTENSION;
                    else err = &err_bad_name;
                }
            }
            if (!err)
            {
                dest += len;
                size -= len;
            }
        }

        // Ensure that the name is terminated
        if (!err) *dest = NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The source leaf name.
                  from          - The source directory separator.
                  to            - The destination directory separator.
                  dest          - Variable to hold the result.
                  size          - Maximum number of characters to write to the
                                  destination buffer, including the terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory leafname between two formats.
*/
static os_error *name_code(name_leaf_code code, char from, char to,
                           const char *src, char *dest, size_t size)
{
    os_error *err = NULL;

    // Check parameters
    if (!code || !src || !dest || !size) err = &err_bad_parms;
    else
    {
        static fs_pathname name;
        char *ptr;

        // Ensure that the destination string if terminated
        *dest = '\0';

        // Make a temporary copy of the name
        if (sizeof(name) <= strlen(src)) err = &err_bad_name;
        else strcpy(name, src);

        // Loop through all of the components of the name
        ptr = name;
        while (!err && *ptr)
        {
            char *next;

            // Find the next separator
            next = strchr(ptr, from);
            if (next) *next = '\0';

            // Add a separator to the result if necessary
            if (ptr != name)
            {
                if (size <= 1) err = &err_bad_name;
                else
                {
                    *dest++ = to;
                    size--;
                }
            }

            // Map this component
            if (!err) err = (*code)(ptr, dest, size);

            // Update the pointers
            if (!err)
            {
                ptr = next ? next + 1 : strchr(ptr, '\0');
                size -= strlen(dest);
                dest = strchr(dest, '\0');
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Size of the result buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory path from the format used by
                  RISC OS to that used by ERA.
*/
os_error *name_riscos_to_era(const char *src, char *dest, size_t size)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest || (size < 4)) err = &err_bad_parms;
    else if ((src[0] != FS_CHAR_DISC) || !isalpha(src[1])
             || (src[2] != FS_CHAR_SEPARATOR) || (src[3] != FS_CHAR_ROOT)
             || (src[4] && (src[4] != FS_CHAR_SEPARATOR)))
    {
        err = &err_bad_name;
    }
    else
    {
        // Convert the prefix
        dest[0] = toupper(src[1]);
        dest[1] = NAME_CHAR_DISC;
        dest[2] = NAME_CHAR_SEPARATOR;

        // Append any path
        if (src[4])
        {
            err = name_code(code_riscos_to_era,
                            FS_CHAR_SEPARATOR, NAME_CHAR_SEPARATOR,
                            src + 5, dest + 3, size - 3);
        }
        else dest[3] = '\0';
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Size of the result buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory path from the format used by
                  RISC OS to that used by SIBO.
*/
os_error *name_riscos_to_sibo(const char *src, char *dest, size_t size)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest || (size < 4)) err = &err_bad_parms;
    else if ((src[0] != FS_CHAR_DISC) || !isalpha(src[1])
             || (src[2] != FS_CHAR_SEPARATOR) || (src[3] != FS_CHAR_ROOT)
             || (src[4] && (src[4] != FS_CHAR_SEPARATOR)))
    {
        err = &err_bad_name;
    }
    else
    {
        // Convert the prefix
        dest[0] = toupper(src[1]);
        dest[1] = NAME_CHAR_DISC;
        dest[2] = NAME_CHAR_SEPARATOR;

        // Append any path
        if (src[4])
        {
            err = name_code(name_leaf_riscos_to_sibo,
                            FS_CHAR_SEPARATOR, NAME_CHAR_SEPARATOR,
                            src + 5, dest + 3, size - 3);
        }
        else dest[3] = '\0';
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Size of the result buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory path from the format used by
                  ERA to that used by RISC OS.
*/
os_error *name_era_to_riscos(const char *src, char *dest, size_t size)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest || (size < 5)) err = &err_bad_parms;
    else if ((!isalpha(src[0]) && (src[0] != SIS_SELECTABLE_DRIVE))
             || (src[1] != NAME_CHAR_DISC)
             || (src[2] != NAME_CHAR_SEPARATOR))
    {
        err = &err_bad_name;
    }
    else
    {
        // Convert the prefix
        dest[0] = FS_CHAR_DISC;
        dest[1] = toupper(src[0]);
        dest[2] = FS_CHAR_SEPARATOR;
        dest[3] = FS_CHAR_ROOT;

        // Append any path
        if (src[3])
        {
            dest[4] = FS_CHAR_SEPARATOR;
            err = name_code(code_era_to_riscos,
                            NAME_CHAR_SEPARATOR, FS_CHAR_SEPARATOR,
                            src + 3, dest + 5, size - 5);
        }
        else dest[4] = '\0';
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Size of the result buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory path from the format used by
                  SIBO to that used by RISC OS.
*/
os_error *name_sibo_to_riscos(const char *src, char *dest, size_t size)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest || (size < 5)) err = &err_bad_parms;
    else if (!isalpha(src[0]) || (src[1] != NAME_CHAR_DISC)
             || (src[2] != NAME_CHAR_SEPARATOR))
    {
        err = &err_bad_name;
    }
    else
    {
        // Convert the prefix
        dest[0] = FS_CHAR_DISC;
        dest[1] = toupper(src[0]);
        dest[2] = FS_CHAR_SEPARATOR;
        dest[3] = FS_CHAR_ROOT;

        // Append any path
        if (src[3])
        {
            dest[4] = FS_CHAR_SEPARATOR;
            err = name_code(code_sibo_to_riscos,
                            NAME_CHAR_SEPARATOR, FS_CHAR_SEPARATOR,
                            src + 3, dest + 5, size - 5);
        }
        else dest[4] = '\0';
    }

    // Return any error produced
    return err;
}
