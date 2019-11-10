/*
    File        : sis.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : SIS file handling for the PsiFS module.

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
#include "sis.h"

// Include system header files
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

// Include oslib header files
#include "oslib/osargs.h"
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "cache.h"
#include "code.h"
#include "crc.h"
#include "ctrl.h"
#include "debug.h"
#include "err.h"
#include "fs.h"
#include "mem.h"
#include "name.h"
#include "uid.h"

// Referenced counted SIS file common data
typedef struct
{
    bits reference;
    os_fw file;
} sis_common;

// SIS file version information
typedef struct
{
    bits major;
    bits minor;
    bits variant;
} sis_version;

// SIS file languages information
#define SIS_MAX_LANGUAGES (32)
typedef struct
{
    bits num;
    psifs_language data[SIS_MAX_LANGUAGES];
} sis_languages;

// SIS file component name information
#define SIS_MAX_COMPONENT (255)
typedef char sis_component[SIS_MAX_COMPONENT + 1];
typedef struct
{
    sis_component data[SIS_MAX_LANGUAGES];
} sis_components;

// SIS file files information
#define SIS_FILE_MULTILINGUAL (0x00000001)
typedef struct
{
    bits offset;
    bits length;
} sis_file_data;
typedef struct
{
    bool multilingual;
    sis_file_type type;
    bits detail;
    fs_pathname dest;
    sis_file_data data[SIS_MAX_LANGUAGES];
} sis_file;
typedef struct
{
    bits num;
    sis_file *data;
} sis_files;

// SIS file requisites information
typedef struct
{
    bits uid;
    sis_version version;
    sis_components components;
} sis_requisite;
typedef struct
{
    bits num;
    sis_requisite *data;
} sis_requisites;

// SIS file installation information
typedef struct
{
    psifs_language language;
    psifs_drive drive;
    bits files;
} sis_installed;

// SIS file selected options
typedef struct
{
    bits language;
    psifs_drive drive;
} sis_selected;

// Residual SIS file information
typedef struct
{
    bits reference;
    fs_pathname name;
    bits size;
    epoc32_file_uid uid;
    sis_version version;
    sis_languages languages;
    sis_files files;
    sis_requisites requisites;
    sis_components components;
    psifs_drive drive;
    sis_installed installed;
    sis_selected selected;
} sis_residual;

// A SIS file handle
struct sis_handle
{
    bits reference;
    sis_common *common;
    sis_residual *residual;
    bits start;
    bits end;
};

// SIS file UID words
#define SIS_UID1_NONE ((bits) 0x10000000)
#define SIS_UID2 ((bits) 0x1000006d)
#define SIS_UID3 ((bits) 0x10000419)

// Offsets to fields within the file header
#define SIS_HEADER_UID1 (0x00)
#define SIS_HEADER_UID2 (0x04)
#define SIS_HEADER_UID3 (0x08)
#define SIS_HEADER_UID4 (0x0c)
#define SIS_HEADER_CHECKSUM (0x10)
#define SIS_HEADER_LANGUAGES (0x12)
#define SIS_HEADER_FILES (0x14)
#define SIS_HEADER_REQUISITES (0x16)
#define SIS_HEADER_INSTALL_LANGUAGE (0x18)
#define SIS_HEADER_INSTALL_FILES (0x1a)
#define SIS_HEADER_INSTALL_DRIVE (0x1c)
#define SIS_HEADER_DEFAULT_DRIVE (0x20)
#define SIS_HEADER_VERSION_MAJOR (0x28)
#define SIS_HEADER_VERSION_MINOR (0x2a)
#define SIS_HEADER_VERSION_VARIANT (0x2c)
#define SIS_HEADER_LANGUAGES_OFFSET (0x30)
#define SIS_HEADER_FILES_OFFSET (0x34)
#define SIS_HEADER_REQUISITES_OFFSET (0x38)
#define SIS_HEADER_COMPONENT_OFFSET (0x40)
#define SIS_HEADER_SIZE (0x44)

// Offsets to fields within languages
#define SIS_LANGUAGE_OFFSET (0x02)

// Offsets to fields within file records
#define SIS_FILE_FLAGS (0x00)
#define SIS_FILE_TYPE (0x04)
#define SIS_FILE_DETAILS (0x08)
#define SIS_FILE_SRC_LENGTH (0x0c)
#define SIS_FILE_SRC_OFFSET (0x10)
#define SIS_FILE_DEST_LENGTH (0x14)
#define SIS_FILE_DEST_OFFSET (0x18)
#define SIS_FILE_LENGTH (0x1c)
#define SIS_FILE_OFFSET (0x04)

// Offsets to fields within requisite records
#define SIS_REQUISITE_UID (0x00)
#define SIS_REQUISITE_VERSION_MAJOR (0x04)
#define SIS_REQUISITE_VERSION_MINOR (0x06)
#define SIS_REQUISITE_VERSION_VARIANT (0x08)
#define SIS_REQUISITE_LENGTH (0x0c)
#define SIS_REQUISITE_OFFSET (0x04)

// Offsets to fields within component record
#define SIS_COMPONENT_OFFSET (0x04)

// Size of buffer for copying between files
#define SIS_BUFFER_SIZE (1024)

// Directory for residual SIS files
#define SIS_RESIDUAL_DIR ":C.$.System.Install."

/*
    Parameters  : handle        - Handle of the file to write to.
                  offset        - Offset from the start of the file at which to
                                  write.
                  value         - The byte value to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write the specified value to the SIS file.
*/
static os_error *sis_put_byte(os_fw handle, bits offset, bits value)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value) err = &err_bad_parms;
    else
    {
        int unwritten;

        // Attempt to write the value
        err = xosgbpb_write_atw(handle, (byte *) &value, sizeof(byte), offset,
                                &unwritten);
        if (!err && unwritten) err = &err_sis_write_outside;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the file to write to.
                  offset        - Offset from the start of the file at which to
                                  write.
                  value         - The word value to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write the specified value to the SIS file.
*/
static os_error *sis_put_word(os_fw handle, bits offset, bits value)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value) err = &err_bad_parms;
    else
    {
        int unwritten;

        // Attempt to write the value
        err = xosgbpb_write_atw(handle, (byte *) &value, sizeof(unsigned short),
                                offset, &unwritten);
        if (!err && unwritten) err = &err_sis_write_outside;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the file to write to.
                  offset        - Offset from the start of the file at which to
                                  write.
                  value         - The bits value to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write the specified value to the SIS file.
*/
static os_error *sis_put_bits(os_fw handle, bits offset, bits value)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value) err = &err_bad_parms;
    else
    {
        int unwritten;

        // Attempt to write the value
        err = xosgbpb_write_atw(handle, (byte *) &value, sizeof(bits), offset,
                                &unwritten);
        if (!err && unwritten) err = &err_sis_write_outside;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to read from.
                  offset        - Offset from the start of the file at which to
                                  read.
                  value         - Variable to receive the value read.
                  size          - The number of bytes to read.
                  residual      - Should the residual file size be updated.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the specified value from the SIS file and optionally
                  update the residual file size.
*/
static os_error *sis_get(sis_handle handle, bits offset, void *value, bits size,
                         bool residual)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value) err = &err_bad_parms;
    else
    {
        bits ptr = handle->start + offset;

        // Check the file pointer
        if (handle->end < ptr + size) err = &err_sis_read_outside;

        // Attempt to read the value
        if (!err && size)
        {
            int unread;

            err = xosgbpb_read_atw(handle->common->file, (byte *) value, size,
                                   ptr, &unread);
            if (!err && unread) err = &err_sis_read_outside;
        }

        // Update the residual file size
        if (!err && residual && (handle->residual->size < offset + size))
        {
            handle->residual->size = offset + size;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to read from.
                  offset        - Offset from the start of the file at which to
                                  read.
                  value         - Variable to receive the byte value read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the specified value from the SIS file.
*/
static os_error *sis_get_byte(sis_handle handle, bits offset, bits *value)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value) err = &err_bad_parms;
    else
    {
        byte buffer;

        DEBUG_PRINTF(("SIS get byte %p at %u", handle, offset))

        // Attempt to read the value
        err = sis_get(handle, offset, &buffer, sizeof(buffer), TRUE);

        // Set the return value
        *value = err ? 0 : buffer;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to read from.
                  offset        - Offset from the start of the file at which to
                                  read.
                  value         - Variable to receive the word value read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the specified value from the SIS file.
*/
static os_error *sis_get_word(sis_handle handle, bits offset, bits *value)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value) err = &err_bad_parms;
    else
    {
        unsigned short buffer;

        DEBUG_PRINTF(("SIS get word %p at %u", handle, offset))

        // Attempt to read the value
        err = sis_get(handle, offset, &buffer, sizeof(buffer), TRUE);

        // Set the return value
        *value = err ? 0 : buffer;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to read from.
                  offset        - Offset from the start of the file at which to
                                  read.
                  value         - Variable to receive the bits value read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the specified value from the SIS file.
*/
static os_error *sis_get_bits(sis_handle handle, bits offset, bits *value)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value) err = &err_bad_parms;
    else
    {
        bits buffer;

        DEBUG_PRINTF(("SIS get bits %p at %u", handle, offset))

        // Attempt to read the value
        err = sis_get(handle, offset, &buffer, sizeof(buffer), TRUE);

        // Set the return value
        *value = err ? 0 : buffer;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to read from.
                  length        - Offset from the start of the file at which to
                                  read the string length.
                  offset        - Offset from the start of the file at which to
                                  read the string offset.
                  value         - Buffer to receive the string value read.
                  size          - The size of the buffer.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the specified value from the SIS file.
*/
static os_error *sis_get_string(sis_handle handle, bits length, bits offset,
                                char *value, bits size)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value || !size) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get string %p at %u/%u", handle, length, offset))

        // Read the string size and offset
        err = sis_get_bits(handle, length, &length);
        if (!err) err = sis_get_bits(handle, offset, &offset);

        // Check the buffer size
        if (!err && (size <= length)) err = &err_bad_name;

        // Attempt to read the value
        if (!err) err = sis_get(handle, offset, (byte *) value, length, TRUE);
        if (!err) value[length] = '\0';
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to read from.
                  length        - Offset from the start of the file at which to
                                  read the string length.
                  offset        - Offset from the start of the file at which to
                                  read the string offset.
                  value         - Buffer to receive the path value read.
                  size          - The size of the buffer.
                  special       - Only keep the leaf name.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the specified value from the SIS file.
*/
static os_error *sis_get_path(sis_handle handle, bits length, bits offset,
                              char *value, bits size, bool special)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !value || !size) err = &err_bad_parms;
    else
    {
        static fs_pathname path;

        DEBUG_PRINTF(("SIS get path %p at %u/%u", handle, length, offset))

        // Attempt to read the raw path
        err = sis_get_string(handle, length, offset, path, sizeof(path));

        // Convert the path to RISC OS format
        if (!err) err = name_era_to_riscos(path, value, size);

        // Resort to simple character translations if necessary
        if (err && ERR_EQ(*err, err_bad_name))
        {
            err = code_era_to_riscos(path, value, size);
        }

        // Skip directories if special processing required
        if (!err && special)
        {
            const char *ptr = ctrl_strrchr(value, FS_CHAR_SEPARATOR);
            if (ptr) ctrl_strcpy(value, ptr + 1);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to copy from.
                  offset        - Offset from the start of the file at which to
                                  copy from.
                  size          - The number of bytes to copy.
                  file          - The destination file to copy to.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Copy from the SIS file to the current position in the
                  destination file.
*/
static os_error *sis_get_copy(sis_handle handle, bits offset, bits size,
                              os_fw file)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !file) err = &err_bad_parms;
    else
    {
        bits ptr = handle->start + offset;

        // Check the file pointer
        if (handle->end < ptr + size) err = &err_sis_read_outside;

        // Loop until all copied
        while (!err && size)
        {
            static byte buffer[SIS_BUFFER_SIZE];
            int length;
            int unread;
            int unwritten;

            // Choose the number of bytes to copy
            length = MIN(SIS_BUFFER_SIZE, size);

            // Read from the SIS file
            err = xosgbpb_read_atw(handle->common->file, buffer, length, ptr,
                                   &unread);
            if (!err && unread) err = &err_sis_read_outside;

            // Write to the destination file
            if (!err)
            {
                err = xosgbpb_writew(file, buffer, length, &unwritten);
                if (!err && unwritten) err = &err_sis_write_outside;
            }

            // Update the status
            if (!err)
            {
                ptr += length;
                size -= length;
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : name          - The filename of the SIS file.
                  common        - Variable to receive a pointer to the common
                                  SIS file details.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create and initialise the common SIS file details.
*/
static os_error *sis_common_new(const char *name, sis_common **common)
{
    os_error *err = NULL;

    // Check function parameters
    if (!name || !common) err = &err_bad_parms;
    else
    {
        sis_common *ptr;

        DEBUG_PRINTF(("SIS common new '%s'", name))

        // Attempt to allocate memory for the new details
        ptr = (sis_common *) MEM_MALLOC(sizeof(sis_common));
        if (!ptr) err = &err_buffer;

        // Set the initial values
        if (!err) ptr->reference = 1;

        // Attempt to open the file
        if (!err)
        {
            err = xosfind_openinw(osfind_NO_PATH
                                  | osfind_ERROR_IF_ABSENT
                                  | osfind_ERROR_IF_DIR,
                                  name, NULL, &ptr->file);
        }
        if (!err && !ptr->file) err = &err_not_found;

        // Free the memory if an error
        if (err && ptr) MEM_FREE(ptr);

        // Set the return value
        *common = err ? NULL : ptr;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : common        - Pointer to the common SIS file details to
                                  delete.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Destroy the common SIS file details.
*/
static os_error *sis_common_delete(sis_common **common)
{
    os_error *err = NULL;

    // Check function parameters
    if (!common || !*common) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS common delete %p", common))

        // Decrement the reference count
        if (!--(*common)->reference)
        {
            // Close the file
            xosfind_closew((*common)->file);

            // Free the memory
            MEM_FREE(*common);
        }

        // Clear the handle
        if (!err) *common = NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The common SIS file details to clone.
                  dest          - Variable to receive a pointer to the cloned
                                  details.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone the common SIS file details. The cloned details
                  reference the same data.
*/
static os_error *sis_common_clone(sis_common *src, sis_common **dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS common clone %p", src))

        // Increment the reference count
        src->reference++;

        // Copy the handle
        *dest = src;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to calculate the
                                  checksum of.
                  checksum      - Variable to receive the checksum.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Calculate the checksum for the SIS file.
*/
static os_error *sis_checksum(sis_handle handle, bits *checksum)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !checksum) err = &err_bad_parms;
    else
    {
        crc_state crc;
        bits offset;

        // Reset the CRC state
        crc_reset(&crc);

        // Calculate the CRC over the whole file excluding the checksum field
        for (offset = 0;
             !err && (offset < (handle->end - handle->start));
             offset++)
        {
            // Exclude the checksum field
            if ((offset < SIS_HEADER_CHECKSUM)
                || (SIS_HEADER_LANGUAGES <= offset))
            {
                bits buffer;
                err = sis_get_byte(handle, offset, &buffer);
                if (!err) crc_update(&crc, buffer);
            }
        }

        // Return the resulting checksum
        *checksum = err ? 0 : (((bits) crc_msb(&crc)) << 8) | ((bits) crc_lsb(&crc));
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : name          - The filename of the SIS file.
                  handle        - Variable to receive a pointer to the residual
                                  SIS file details.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create and initialise the residual SIS file details.
                  This does not load or process any existing file.
*/
static os_error *sis_residual_new(const char *name, sis_residual **residual)
{
    os_error *err = NULL;

    // Check function parameters
    if (!residual) err = &err_bad_parms;
    else
    {
        sis_residual *ptr;

        DEBUG_PRINTF(("SIS residual new '%s'", name))

        // Attempt to allocate memory for the new handle
        ptr = (sis_residual *) MEM_MALLOC(sizeof(sis_residual));
        if (!ptr) err = &err_buffer;

        // Set the initial values
        if (!err)
        {
            ptr->reference = 1;
            ptr->files.data = NULL;
            ptr->requisites.data = NULL;
        }

        // Store the filename
        if (!err)
        {
            const char *start = ctrl_strrchr(name, FS_CHAR_SEPARATOR);
            start = start ? start + 1 : name;
            if (sizeof(ptr->name) <= (ctrl_strlen(SIS_RESIDUAL_DIR) + ctrl_strlen(start))) err = &err_bad_name;
            else
            {
                ctrl_strcpy(ptr->name, SIS_RESIDUAL_DIR);
                ctrl_strcat(ptr->name, start);
            }
        }

        // Free the memory if an error
        if (err && ptr) MEM_FREE(ptr);

        // Set the return value
        *residual = err ? NULL : ptr;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : residual      - Pointer to the residual SIS file details to
                                  delete.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Destroy the residual SIS file details.
*/
static os_error *sis_residual_delete(sis_residual **residual)
{
    os_error *err = NULL;

    // Check function parameters
    if (!residual || !*residual) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS residual delete %p", residual))

        // Decrement the reference count
        if (!--(*residual)->reference)
        {
            // Free the memory
            if ((*residual)->files.data) MEM_FREE((*residual)->files.data);
            if ((*residual)->requisites.data) MEM_FREE((*residual)->requisites.data);
            MEM_FREE(*residual);
        }

        // Clear the handle
        if (!err) *residual = NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The residual SIS file details to clone.
                  dest          - Variable to receive a pointer to the cloned
                                  details.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone the residual SIS file details. The cloned details
                  reference the same data.
*/
static os_error *sis_residual_clone(sis_residual *src, sis_residual **dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS residual clone %p", src))

        // Increment the reference count
        src->reference++;

        // Copy the handle
        *dest = src;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to load.
                  offset        - Offset to the languages data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Load the languages from the residual portion of a SIS file.
*/
static os_error *sis_residual_load_languages(sis_handle handle, bits offset)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        bits language;

        DEBUG_PRINTF(("SIS residual load languages %p at %u", handle, offset))

        // Read the languages data
        for (language = 0;
             !err && (language < handle->residual->languages.num);
             language++)
        {
            err = sis_get_word(handle, offset, &handle->residual->languages.data[language]);
            if (!err) offset += SIS_LANGUAGE_OFFSET;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to load.
                  offset        - Offset to the files data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Load the files from the residual portion of a SIS file.
*/
static os_error *sis_residual_load_files(sis_handle handle, bits offset)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        bits file;

        DEBUG_PRINTF(("SIS residual load files %p at %u", handle, offset))

        // Allocate memory for the files data
        handle->residual->files.data = (sis_file *) MEM_MALLOC(sizeof(sis_file) * handle->residual->files.num);
        if (!handle->residual->files.data) err = &err_buffer;

        // Read the files data in reverse order
        for (file = handle->residual->files.num; !err && file; )
        {
            bool special = FALSE;
            bits flags;
            bits language;

            // Decrement the file number
            file--;

            // Read the common file details
            err = sis_get_bits(handle, offset + SIS_FILE_FLAGS, &flags);
            if (!err) handle->residual->files.data[file].multilingual = flags & SIS_FILE_MULTILINGUAL;
            if (!err) err = sis_get_bits(handle, offset + SIS_FILE_TYPE, &handle->residual->files.data[file].type);
            if (!err) err = sis_get_bits(handle, offset + SIS_FILE_DETAILS, &handle->residual->files.data[file].detail);
            if (!err)
            {
                switch (handle->residual->files.data[file].type)
                {
                    case SIS_FILE_STANDARD:
                    case SIS_FILE_NONE:
                        // A standard file or one that does not exist yet
                        break;

                    case SIS_FILE_TEXT:
                        // A text file to be displayed
                        if (!err && (handle->residual->files.data[file].detail != SIS_BUTTONS_CONTINUE) && (handle->residual->files.data[file].detail != SIS_BUTTONS_SKIP) && (handle->residual->files.data[file].detail != SIS_BUTTONS_ABORT)) err = &err_bad_sis_type;
                        special = TRUE;
                        break;

                    case SIS_FILE_SIS:
                        // Component SIS file to install
                        special = TRUE;
                        break;

                    case SIS_FILE_RUN:
                        // File to run on installation or removal
                        if (!err && (handle->residual->files.data[file].detail != SIS_RUN_INSTALL) && (handle->residual->files.data[file].detail != SIS_RUN_REMOVE) && (handle->residual->files.data[file].detail != SIS_RUN_BOTH)) err = &err_bad_sis_type;
                        break;

                    default:
                        // Not a recognised file type
                        err = &err_bad_sis_type;
                        break;
                }
            }
            if (!err) err = sis_get_path(handle, offset + (special ? SIS_FILE_SRC_LENGTH : SIS_FILE_DEST_LENGTH), offset + (special ? SIS_FILE_SRC_OFFSET : SIS_FILE_DEST_OFFSET), handle->residual->files.data[file].dest, sizeof(handle->residual->files.data[file].dest), special);
            if (!err) offset += SIS_FILE_LENGTH;

            // Process the language specific variants
            for (language = 0;
                 !err && (language < handle->residual->languages.num);
                 language++)
            {
                err = sis_get_bits(handle, offset, &handle->residual->files.data[file].data[language].length);
                if (!err) err = sis_get_bits(handle, offset + SIS_FILE_OFFSET * (handle->residual->files.data[file].multilingual ? handle->residual->languages.num : 1), &handle->residual->files.data[file].data[language].offset);
                if (handle->residual->files.data[file].multilingual) offset += SIS_FILE_OFFSET;
            }
            if (!err) offset += SIS_FILE_OFFSET * (handle->residual->files.data[file].multilingual ? handle->residual->languages.num : 2);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to load.
                  offset        - Offset to the requisites data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Load the requisites from the residual portion of a SIS file.
*/
static os_error *sis_residual_load_requisites(sis_handle handle, bits offset)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        bits requisite;

        DEBUG_PRINTF(("SIS residual load requisites %p at %u", handle, offset))

        // Allocate memory for the requisites data
        handle->residual->requisites.data = (sis_requisite *) MEM_MALLOC(sizeof(sis_requisite) * handle->residual->requisites.num);
        if (!handle->residual->requisites.data) err = &err_buffer;

        // Read the requisite data in reverse order
        for (requisite = handle->residual->requisites.num; !err && requisite; )
        {
            bits language;

            // Decrement the requisite number
            requisite--;

            // Read the requisite details
            err = sis_get_bits(handle, offset + SIS_REQUISITE_UID, &handle->residual->requisites.data[requisite].uid);
            if (!err) err = sis_get_word(handle, offset + SIS_REQUISITE_VERSION_MAJOR, &handle->residual->requisites.data[requisite].version.major);
            if (!err) err = sis_get_word(handle, offset + SIS_REQUISITE_VERSION_MINOR, &handle->residual->requisites.data[requisite].version.minor);
            if (!err) err = sis_get_bits(handle, offset + SIS_REQUISITE_VERSION_VARIANT, &handle->residual->requisites.data[requisite].version.variant);
            if (!err) offset += SIS_REQUISITE_LENGTH;

            // Process language specific variants
            for (language = 0;
                 !err && (language < handle->residual->languages.num);
                 language++)
            {
                if (!err) err = sis_get_path(handle, offset, offset + SIS_REQUISITE_OFFSET * handle->residual->languages.num, handle->residual->requisites.data[requisite].components.data[language], sizeof(handle->residual->requisites.data[requisite].components.data[language]), FALSE);
                if (!err) offset += SIS_REQUISITE_OFFSET;
            }
            if (!err) offset += SIS_REQUISITE_OFFSET * handle->residual->languages.num;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to load.
                  offset        - Offset to the component data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Load the component from the residual portion of a SIS file.
*/
static os_error *sis_residual_load_component(sis_handle handle, bits offset)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        bits language;

        DEBUG_PRINTF(("SIS residual load component %p at %u", handle, offset))

        // Read the component data
        for (language = 0;
             !err && (language < handle->residual->languages.num);
             language++)
        {
            err = sis_get_string(handle, offset, offset + SIS_COMPONENT_OFFSET * handle->residual->languages.num, handle->residual->components.data[language], sizeof(handle->residual->components.data[language]));
            if (!err) err = code_ansi_to_latin1(handle->residual->components.data[language], handle->residual->components.data[language]);
            if (!err) offset += SIS_COMPONENT_OFFSET;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to load.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Load the residual portion of a SIS file.
*/
static os_error *sis_residual_load(sis_handle handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        bits uid4;
        bits uid4_expected;
        bits checksum;
        bits checksum_expected;
        bits offset_languages;
        bits offset_files;
        bits offset_requisites;
        bits offset_component;
        bits value;

        DEBUG_PRINTF(("SIS residual load %p", handle))

        // Start by assuming no residual file
        handle->residual->size = 0;

        // Read the header
        err = sis_get_bits(handle, SIS_HEADER_UID1, &handle->residual->uid.uid1);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_UID2, &handle->residual->uid.uid2);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_UID3, &handle->residual->uid.uid3);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_UID4, &uid4);
        if (!err) err = uid_checksum_uid(&handle->residual->uid, &uid4_expected);
        if (!err && ((handle->residual->uid.uid2 != SIS_UID2) || (handle->residual->uid.uid3 != SIS_UID3) || (uid4 != uid4_expected))) err = &err_bad_sis_header;
        if (!err) err = sis_get_word(handle, SIS_HEADER_CHECKSUM, &checksum);
        if (!err) err = sis_checksum(handle, &checksum_expected);
        if (!err && (checksum != checksum_expected)) err = &err_bad_sis_checksum;
        if (!err) err = sis_get_word(handle, SIS_HEADER_LANGUAGES, &handle->residual->languages.num);
        if (!err && (SIS_MAX_LANGUAGES < handle->residual->languages.num)) err = &err_too_many_lang;
        if (!err) err = sis_get_word(handle, SIS_HEADER_FILES, &handle->residual->files.num);
        if (!err) err = sis_get_word(handle, SIS_HEADER_REQUISITES, &handle->residual->requisites.num);
        if (!err) err = sis_get_word(handle, SIS_HEADER_INSTALL_LANGUAGE, &handle->residual->installed.language);
        if (!err) err = sis_get_word(handle, SIS_HEADER_INSTALL_FILES, &handle->residual->installed.files);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_INSTALL_DRIVE, &value);
        if (!err) handle->residual->installed.drive = value;
        if (!err) err = sis_get_bits(handle, SIS_HEADER_DEFAULT_DRIVE, &value);
        if (!err) handle->residual->drive = value;
        if (!err) err = sis_get_word(handle, SIS_HEADER_VERSION_MAJOR, &handle->residual->version.major);
        if (!err) err = sis_get_word(handle, SIS_HEADER_VERSION_MINOR, &handle->residual->version.minor);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_VERSION_VARIANT, &handle->residual->version.variant);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_LANGUAGES_OFFSET, &offset_languages);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_FILES_OFFSET, &offset_files);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_REQUISITES_OFFSET, &offset_requisites);
        if (!err) err = sis_get_bits(handle, SIS_HEADER_COMPONENT_OFFSET, &offset_component);

        // Read the languages
        if (!err) err = sis_residual_load_languages(handle, offset_languages);

        // Read the files
        if (!err) err = sis_residual_load_files(handle, offset_files);

        // Read the requisites
        if (!err) err = sis_residual_load_requisites(handle, offset_requisites);

        // Read the component
        if (!err) err = sis_residual_load_component(handle, offset_component);

        // Set the default language
        if (!err)
        {
            // Try the current selection first
            if (sis_set_language(handle, handle->residual->installed.language))
            {
                psifs_language language;

                // Try the language of the connected machine next
                err = cache_machine_status(NULL, NULL, NULL, &language, NULL);
                if (!err && sis_set_language(handle, language))
                {
                    // Try to use English next
                    if (sis_set_language(handle, psifs_LANGUAGE_UK_ENGLISH))
                    {
                        // Finally resort to using the first language
                        err = sis_set_language(handle, handle->residual->languages.data[0]);
                    }
                }
            }
        }

        // Set the default drive
        if (!err)
        {
            // Try the current selection first
            if (sis_set_drive(handle, handle->residual->installed.drive))
            {
                // Try the default drive next
                //if (sis_set_drive(handle, handle->residual->drive))
                {
                    // Finally resort to drive C
                    err = sis_set_drive(handle, 'C');
                }
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : common        - Pointer to the common details for this handle.
                  start         - The offset to the start of the SIS file.
                  end           - The offset to the end of the SIS file.
                  name          - The name of the SIS file to open.
                  handle        - Variable to receive a pointer to the handle.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create a new SIS file handle.
*/
static os_error *sis_handle_new(sis_common *common, bits start, bits end,
                                const char *name, sis_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!common || (end < start) || !name || !handle) err = &err_bad_parms;
    else
    {
        sis_handle ptr;

        DEBUG_PRINTF(("SIS handle new %p from %u to %u '%s'", common, start, end, name))

        // Attempt to allocate memory for the new handle
        ptr = (sis_handle) MEM_MALLOC(sizeof(struct sis_handle));
        if (!ptr) err = &err_buffer;

        // Set the initial values
        if (!err)
        {
            ptr->reference = 1;
            ptr->common = common;
            ptr->start = start;
            ptr->end = end;
        }

        // Attempt to create the residual details
        if (!err) err = sis_residual_new(name, &ptr->residual);

        // Attempt to load the residual details
        if (!err)
        {
            // Load the residual details
            err = sis_residual_load(ptr);

            // Delete the residual details if an error
            if (err) sis_residual_delete(&ptr->residual);
        }

        // Free the memory if an error
        if (err && ptr) MEM_FREE(ptr);

        // Set the return value
        *handle = err ? NULL : ptr;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to delete.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Destroy a SIS file handle.
*/
static os_error *sis_handle_delete(sis_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !*handle) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS handle delete %p", handle))

        // Decrement the reference count
        if (!--(*handle)->reference)
        {
            // Delete the common details
            if (!err) err = sis_common_delete(&(*handle)->common);

            // Delete the residual details
            if (!err) err = sis_residual_delete(&(*handle)->residual);

            // Free the memory
            if (!err) MEM_FREE(*handle);
        }

        // Clear the handle
        if (!err) *handle = NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The SIS file handle to clone.
                  dest          - Variable to receive the cloned handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone a SIS file handle. The cloned handle references the
                  same SIS file data.
*/
static os_error *sis_handle_clone(sis_handle src, sis_handle *dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS handle clone %p", src))

        // Increment the reference count
        src->reference++;

        // Copy the handle
        *dest = src;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The name of the SIS file to open.
                  name          - The original name of the SIS file.
                  handle        - Variable to receive a pointer to the handle.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Open a SIS file.
*/
os_error *sis_open(const char *src, const char *name, sis_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !name || !handle) err = &err_bad_parms;
    else
    {
        sis_common *common = NULL;
        int size;

        DEBUG_PRINTF(("SIS open '%s' as '%s'", src, name))

        // Attempt to open the file
        err = sis_common_new(src, &common);

        // Read the file extent
        if (!err) err = xosargs_read_extw(common->file, &size);

        // Create a new handle
        if (!err) err = sis_handle_new(common, 0, size, name, handle);

        // Delete the common details if an error
        if (err && common) sis_common_delete(&common);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The SIS file handle to clone.
                  dest          - Variable to receive the cloned handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone a SIS file handle. The cloned handle references the
                  same SIS file.
*/
os_error *sis_clone(sis_handle src, sis_handle *dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS clone %p", src))

        // Clone the handle
        err = sis_handle_clone(src, dest);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to close.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Close a previously opened SIS file. This should be called the
                  same number of times as sis_open and sis_clone.
*/
os_error *sis_close(sis_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !*handle) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS close %p", *handle))

        // Close the handle
        err = sis_handle_delete(handle);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  num           - Variable to receive the number of languages.
                  languages     - Variable to receive a pointer to the list of
                                  languages.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the languages supported by the SIS file.
*/
os_error *sis_get_languages(sis_handle handle, bits *num,
                            const psifs_language **languages)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !num || !languages) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get languages %p", handle))

        // Return the languages details
        *num = handle->residual->languages.num;
        *languages = handle->residual->languages.data;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  language      - Variable to receive the selected language.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the selected language for this SIS file.
*/
os_error *sis_get_language(sis_handle handle, psifs_language *language)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !language) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get language %p", handle))

        // Return the currently selected language
        *language = handle->residual->languages.data[handle->residual->selected.language];
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  language      - The language to use.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write the selected language for this SIS file.
*/
os_error *sis_set_language(sis_handle handle, psifs_language language)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        bits index = 0;

        DEBUG_PRINTF(("SIS set language %p to %u", handle, language))

        // Attempt to find the specified language
        while ((index < handle->residual->languages.num)
               && (handle->residual->languages.data[index] != language))
        {
            index++;
        }

        // Store the selected language if found
        if (index < handle->residual->languages.num)
        {
            handle->residual->selected.language = index;
        }
        else err = &err_lang_unknown;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  component     - Variable to receive a pointer to the component
                                  name.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the component name for this SIS file.
*/
os_error *sis_get_component(sis_handle handle, const char **component)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !component) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get component %p", handle))

        // Set the return value
        *component = handle->residual->components.data[handle->residual->selected.language];
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  major         - Variable to receive the major version number.
                  minor         - Variable to receive the minor version number.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the version number for this SIS file.
*/
os_error *sis_get_version(sis_handle handle, bits *major, bits *minor)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !major || !minor) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get version %p", handle))

        // Set the return values
        *major = handle->residual->version.major;
        *minor = handle->residual->version.minor;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  variant       - Variable to receive the variant number.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the variant number for this SIS file.
*/
os_error *sis_get_variant(sis_handle handle, bits *variant)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !variant) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get variant %p", handle))

        // Set the return value
        *variant = handle->residual->version.variant;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  uid           - Variable to receive the UID.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the UID for this SIS file.
*/
os_error *sis_get_uid(sis_handle handle, epoc32_file_uid *uid)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !uid) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get UID %p", handle))

        // Set the return value
        *uid = handle->residual->uid;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  drive         - Variable to receive the current installation
                                  drive.
                  selectable    - Variable to receive whether the installation
                                  drive is selectable.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the installation drive for this SIS file.
*/
os_error *sis_get_drive(sis_handle handle, psifs_drive *drive,
                        bool *selectable)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || (!drive && !selectable)) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get drive %p", handle))

        // Set the return values
        if (drive) *drive = toupper(handle->residual->selected.drive);
        if (selectable)
        {
            bits index;

            // Check if the installation drive is selectable
            *selectable = FALSE;
            for (index = 0;
                 !*selectable && (index < handle->residual->files.num);
                 index++)
            {
                const char *dest = handle->residual->files.data[index].dest;
                if ((dest[0] == FS_CHAR_DISC)
                    && (dest[1] == SIS_SELECTABLE_DRIVE))
                {
                    *selectable = TRUE;
                }
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  drive         - The required installation drive.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write the installation drive for this SIS file.
*/
os_error *sis_set_drive(sis_handle handle, psifs_drive drive)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else if (!isalpha(drive)) err = &err_bad_drive;
    else
    {
        DEBUG_PRINTF(("SIS set drive %p to %c", handle, drive))

        // Set the drive
        handle->residual->selected.drive = tolower(drive);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  num           - Variable to receive the number of files.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the number of files in this SIS file.
*/
os_error *sis_get_files(sis_handle handle, bits *num)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !num) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get files %p", handle))

        // Set the return value
        *num = handle->residual->files.num;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  index         - Index of the file to manipulate.
                  name          - Variable to receive the name of the file.
                  type          - Variable to receive the type of the file.
                  detail        - Variable to receive any associated detail
                                  for the file.
                  size          - Variable to receive the size of the file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read about a file from this SIS file.
*/
os_error *sis_get_file(sis_handle handle, bits index, const char **name,
                       sis_file_type *type, bits *detail, bits *size)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || (handle->residual->files.num <= index) || (!name && !type && !detail && !size)) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get file %p index %u", handle, index))

        // Set the return values
        if (name)
        {
            static fs_pathname path;

            // Set the drive letter if appropriate
            ctrl_strcpy(path, handle->residual->files.data[index].dest);
            if ((path[0] == FS_CHAR_DISC) && (path[1] == SIS_SELECTABLE_DRIVE))
            {
                path[1] = toupper(handle->residual->selected.drive);
            }
            *name = path;
        }
        if (type) *type = handle->residual->files.data[index].type;
        if (detail) *detail = handle->residual->files.data[index].detail;
        if (size) *size = handle->residual->files.data[index].data[handle->residual->selected.language].length;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  index         - Index of the file to manipulate.
                  name          - Name of the file to create.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Extract a file from this SIS file.
*/
os_error *sis_save_file(sis_handle handle, bits index, const char *name)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || (handle->residual->files.num <= index) || !name || (handle->residual->files.data[index].type == SIS_FILE_NONE)) err = &err_bad_parms;
    else
    {
        const sis_file_data *data = &handle->residual->files.data[index].data[handle->residual->selected.language];
        os_fw file = 0;
        epoc32_file_uid uid;

        DEBUG_PRINTF(("SIS save file %p index %u as '%s'", handle, index, name))

        // Create the file
        err = xosfind_openoutw(osfind_NO_PATH | osfind_ERROR_IF_DIR, name, NULL,
                               &file);
        if (!err && !file) err = &err_not_found;

        // Set the file extent
        if (!err) err = xosargs_set_extw(file, data->length);

        // Save the data from the SIS file
        if (!err) err = sis_get_copy(handle, data->offset, data->length, file);

        // Close the file
        if (file) xosfind_closew(file);

        // Set the file type appropriately
        if (!err && (sizeof(uid) <= data->length))
        {
            err = sis_get(handle, data->offset, &uid, sizeof(uid), FALSE);
        }
        if (!err)
        {
            bits type = uid_map_type(handle->residual->files.data[index].dest, data->length < sizeof(uid) ? NULL : &uid);
            err = xosfile_set_type(name, type);
        }

        // Delete the file if an error
        if (err && file) xosfscontrol_wipe(name, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - Handle of the SIS file to manipulate.
                  index         - Index of the file to manipulate.
                  dest          - Variable to receive the new handle.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Obtain a handle for a component SIS file.
*/
os_error *sis_get_sis(sis_handle src, bits index, sis_handle *dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest || (src->residual->files.num <= index) || (src->residual->files.data[index].type != SIS_FILE_SIS)) err = &err_bad_parms;
    else
    {
        sis_common *common;
        const sis_file_data *data = &src->residual->files.data[index].data[src->residual->selected.language];

        DEBUG_PRINTF(("SIS get SIS %p index %u", src, index))

        // Clone the common handle
        err = sis_common_clone(src->common, &common);

        // Create the new handle
        if (!err) err = sis_handle_new(common, src->start + data->offset, src->start + data->offset + data->length, src->residual->files.data[index].dest, dest);

        // Delete the common handle if an error
        if (err && common) sis_common_delete(&common);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  num           - Variable to receive the number of requisites.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the number of requisites in this SIS file.
*/
os_error *sis_get_requisites(sis_handle handle, bits *num)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !num) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get requisites %p", handle))

        // Set the return value
        *num = handle->residual->requisites.num;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  index         - Index of the requisite to manipulate.
                  name          - Variable to receive the name of the requisite.
                  uid           - Variable to receive the UID of the requisite.
                  major         - Variable to receive the major version number
                                  of the variant.
                  minor         - Variable to receive the minor version number
                                  of the variant.
                  variant       - Variable to receive the variant of the
                                  requisite.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the number of requisites in this SIS file.
*/
os_error *sis_get_requisite(sis_handle handle, bits index, const char **name,
                            bits *uid, bits *major, bits *minor, bits *variant)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || (handle->residual->requisites.num <= index) || (!name && !uid && !major && !minor && !variant)) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get requisite %p index %u", handle, index))

        // Set the return values
        if (name) *name = handle->residual->requisites.data[index].components.data[handle->residual->selected.language];
        if (uid) *uid = handle->residual->requisites.data[index].uid;
        if (major) *major = handle->residual->requisites.data[index].version.major;
        if (minor) *minor = handle->residual->requisites.data[index].version.minor;
        if (variant) *variant = handle->residual->requisites.data[index].version.variant;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  name          - Variable to receive the residual SIS file
                                  name.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the name for the residual version of this SIS file.
*/
os_error *sis_get_residual(sis_handle handle, const char **name)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !name) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SIS get residual %p", handle))

        // Set the return value
        *name = handle->residual->name;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  name          - Name of the file to create.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create the residual version of this SIS file.
*/
os_error *sis_save_residual(sis_handle handle, const char *name)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !name) err = &err_bad_parms;
    else
    {
        os_fw file = 0;

        DEBUG_PRINTF(("SIS save residual %p as '%s'", handle, name))

        // Create the residual file
        err = xosfind_openoutw(osfind_NO_PATH | osfind_ERROR_IF_DIR, name, NULL,
                               &file);
        if (!err && !file) err = &err_not_found;

        // Set the file extent
        if (!err) err = xosargs_set_extw(file, handle->residual->size);

        // Copy the initial portion of the original SIS file
        if (!err) err = sis_get_copy(handle, 0, handle->residual->size, file);

        // Set the installation details
        if (!err)
        {
            handle->residual->installed.language = handle->residual->languages.data[handle->residual->selected.language];
            handle->residual->installed.drive = handle->residual->selected.drive;
            handle->residual->installed.files = handle->residual->files.num;
        }
        if (!err) err = sis_put_word(file, SIS_HEADER_INSTALL_LANGUAGE, handle->residual->installed.language);
        if (!err) err = sis_put_word(file, SIS_HEADER_INSTALL_FILES, handle->residual->installed.files);
        if (!err) err = sis_put_bits(file, SIS_HEADER_INSTALL_DRIVE, handle->residual->installed.drive);

        // Close the file
        if (file) xosfind_closew(file);

        // Set the file type appropriately
        if (!err)
        {
            bits type = uid_map_type(handle->residual->name, &handle->residual->uid);
            err = xosfile_set_type(name, type);
        }

        // Delete the file if an error
        if (err && file) xosfscontrol_wipe(name, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
    }

    // Return any error produced
    return err;
}
