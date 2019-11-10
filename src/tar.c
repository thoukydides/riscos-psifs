/*
    File        : tar.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Tar file handling for the PsiFS module.

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
#include "tar.h"

// Include system header files
#include <stdio.h>
#include <stddef.h>
#include <string.h>

// Include oslib header files
#include "oslib/hourglass.h"
#include "oslib/macros.h"
#include "oslib/osargs.h"
#include "oslib/osfind.h"
#include "oslib/osfile.h"

// Include project header files
#include "attr.h"
#include "date.h"
#include "debug.h"
#include "err.h"
#include "mem.h"
#include "name.h"
#include "util.h"

// Filetype of a tar file
#define TAR_TYPE (0xc46)

// Sze of a tar block
#define TAR_BLOCK (512)

// Maximum number of blocks for different operations
#define TAR_MAX_BLOCKS (32)
#define TAR_ADD_BLOCKS (100)
#define TAR_EXTRACT_BLOCKS (100)
#define TAR_COPY_BLOCKS (100)
static bits tar_step_blocks = TAR_MAX_BLOCKS;
static os_t tar_step_start;

// Ideal time in centiseconds per step
#define TAR_STEP_CENTISECONDS (3)

// A header block
typedef struct
{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char modified[12];
    char checksum[8];
    union
    {
        struct
        {
            char linkflag[2];
            char padding1[2];
            char stamped[1];
            char type[6];
            char datestamp[12];
            char padding2[4];
            char attr[5];
            char load[13];
            char exec[13];
            char padding3[46];
            char magic[8];
            char padding4[244];
        } arctar;
        struct
        {
            char padding1[101];
            char magic[8];
            char load[12];
            char exec[12];
            char attr[12];
            char date[12];
            char zero[12];
            char a[2];
            char padding2[185];
        } fltar;
    } ext;
} tar_header;

// A generic block
typedef union
{
    byte data[TAR_BLOCK];
    tar_header header;
} tar_block;

// An operation
typedef bits tar_op;
#define TAR_IDLE ((tar_op) 0x00)
#define TAR_ADD ((tar_op) 0x01)
#define TAR_EXTRACT ((tar_op) 0x02)
#define TAR_COPY_SRC ((tar_op) 0x03)
#define TAR_COPY_DEST ((tar_op) 0x04)

// A tar file handle
struct tar_handle
{
    bits reference;
    fs_pathname name;
    bool write;
    os_fw file;
    fs_info info;
    tar_block block[TAR_MAX_BLOCKS];
    tar_op op;
    os_fw op_file;
    bits done;
    bits remain;
    tar_handle partner;
};

/*
    Parameters  : handle        - Handle of the tar file to read.
                  num           - The number of blocks to skip.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Skip over the specified number of blocks from the specified
                  tar file.
*/
static os_error *tar_skip_block(tar_handle handle, bits num)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        int ptr;
        int ext;

        // Attempt to skip over the specified number of blocks
        err = xosargs_read_ptrw(handle->file, &ptr);
        if (!err) err = xosargs_read_extw(handle->file, &ext);
        if (!err)
        {
            ptr += TAR_BLOCK * num;
            if (ext < ptr) err = &err_eof;
            else err = xosargs_set_ptrw(handle->file, ptr);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the tar file to read.
                  block         - Buffer to receive the next blocks of data.
                  num           - The number of blocks to read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the specified number of blocks from the specified tar
                  file.
*/
static os_error *tar_read_block(tar_handle handle, tar_block *block,
                                bits num)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !block) err = &err_bad_parms;
    else
    {
        int unread;

        // Attempt to read the specified number of blocks
        err = xosgbpb_readw(handle->file, block->data,
                            TAR_BLOCK * num, &unread);

        // Special case if did not read the specified number of blocks
        if (!err && unread) err = &err_eof;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the tar file to write.
                  block         - Buffer containing the blocks of data.
                  num           - The number of blocks to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write the specified number of blocks to the specified tar
                  file.
*/
static os_error *tar_write_block(tar_handle handle, const tar_block *block,
                                 bits num)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !block) err = &err_bad_parms;
    else
    {
        int unwritten;

        // Attempt to write the specified number of blocks
        err = xosgbpb_writew(handle->file, block->data, TAR_BLOCK * num,
                             &unwritten);

        // Special case if did not write the specified number of blocks
        if (!err && unwritten) err = &err_eof;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : block     - Buffer containing the header block.
    Returns     : bits      - The corresponding checksum.
    Description : Calculate the checksum for the specified header block. This
                  ignores the checksum field, assuming that it contains spaces.
*/
static bits tar_checksum(const tar_block *block)
{
    bits sum = 0;
    int i;

    // Include all bytes in the checksum
    for (i = 0; i < TAR_BLOCK; i++)
    {
        sum += (i < offsetof(tar_header, checksum))
               || (offsetof(tar_header, checksum) + 8 <= i)
               ? block->data[i] : ' ';
    }

    // Return the result
    return sum;
}

/*
    Parameters  : block     - Buffer containing the header block.
    Returns     : bool      - Is the header block blank.
    Description : Check whether the specified header block is blank.
*/
static bool tar_blank(const tar_block *block)
{
    bool blank = TRUE;
    int i;

    // Include all bytes in the checksum
    for (i = 0; blank && (i < TAR_BLOCK); i++)
    {
        if (block->data[i]) blank = FALSE;
    }

    // Return the result
    return blank;
}

/*
    Parameters  : info          - The file information.
                  block         - Buffer to receive the header block.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Construct a tar file header block based on the specified file
                  information.
*/
static os_error *tar_build_header(const fs_info *info, tar_block *block)
{
    os_error *err = NULL;

    // Check function parameters
    if (!info || !block) err = &err_bad_parms;
    else
    {
        bool dir = info->obj_type == fileswitch_IS_DIR;
        //bool typed = (info->load_addr & 0xfff00000) == 0xfff00000;
        date_riscos date;

        // Start by clearing the header
        memset(block->data, 0, TAR_BLOCK);

        // Fill in the standard fields except the checksum
        if (sizeof(block->header.name) <= strlen(info->name) + (dir ? 1 : 0))
        {
            err = &err_bad_name;
        }
        else
        {
            strcpy(block->header.name, info->name);
            if (dir) *strchr(block->header.name, '\0') = FS_CHAR_SEPARATOR;
            sprintf(block->header.mode, "%6o ", attr_to_unix(info->attr));
            sprintf(block->header.uid, "%6o ", 0);
            sprintf(block->header.gid, "%6o ", 0);
            sprintf(block->header.size, "%11o ", dir ? 0 : info->size);
            date.words.high = info->load_addr;
            date.words.low = info->exec_addr;
            sprintf(block->header.modified, "%11o ", date_to_unix(&date));

            // Fill in the arctar fields
            /*
            sprintf(block->header.ext.arctar.linkflag, "%1o", dir ? 5 : 0);
            sprintf(block->header.ext.arctar.stamped, "%1o", typed ? 1 : 0);
            sprintf(block->header.ext.arctar.type, "%4o ",
                    typed ? (info->load_addr >> 8) & 0xfff : 0);
            sprintf(block->header.ext.arctar.datestamp, "%02x%08x ",
                    info->load_addr & 0xff, info->exec_addr);
            sprintf(block->header.ext.arctar.attr, "%3o ", info->attr);
            sprintf(block->header.ext.arctar.load, "%11o ", info->load_addr);
            sprintf(block->header.ext.arctar.exec, "%11o ", info->exec_addr);
            sprintf(block->header.ext.arctar.magic, "arctar ");
            */

            // Fill in the fltar fields
            sprintf(block->header.ext.fltar.magic, "Archie ");
            sprintf(block->header.ext.fltar.load, "%11o", info->load_addr);
            sprintf(block->header.ext.fltar.exec, "%11o", info->exec_addr);
            sprintf(block->header.ext.fltar.attr, "%11o", info->attr);
            sprintf(block->header.ext.fltar.date, "%02x %08x",
                    info->load_addr & 0xff, info->exec_addr);
            sprintf(block->header.ext.fltar.zero, "%11o", 0);
            sprintf(block->header.ext.fltar.a, "A");

            // Finally fill in the checksum
            sprintf(block->header.checksum, "%6o", tar_checksum(block));
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : block         - Buffer containing the header block.
                  info          - Variable to receive the file information.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Decode a tar file header block into the corresponding file
                  information.
*/
static os_error *tar_decode_header(const tar_block *block, fs_info *info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!block || !info) err = &err_bad_parms;
    else
    {
        bits checksum;

        // Start from default details
        info->load_addr = 0;
        info->exec_addr = 0;
        info->size = 0;
        info->attr = 0;
        info->obj_type = fileswitch_NOT_FOUND;
        *info->name = '\0';

        // Check for special conditions
        if (tar_blank(block))
        {
            // End of tar file marker
        }
        else if ((sscanf(block->header.checksum, "%6o", &checksum) != 1)
                 || (checksum != tar_checksum(block)))
        {
            // Incorrect header checksum
            err = &err_bad_tar_checksum;
        }
        else
        {
            char *ptr;
            bool arctar;
            bool fltar;
            bits type = osfile_TYPE_DATA;
            attr_unix mode;
            int size;
            date_unix modified;

            // Copy the filename
            strcpy(info->name, block->header.name);

            // Process the filetype if any
            ptr = strchr(info->name, ',');
            if (ptr)
            {
                bits comma;
                if (sscanf(ptr + 1, "%3x", &comma) == 1) type = comma;
                *ptr = '\0';
            }

            // Attempt to recognise the file format
            arctar = !strncmp(block->header.ext.arctar.magic, "arctar", 6);
            fltar = !strncmp(block->header.ext.fltar.magic, "Archie", 6);

            // Check if character translation is required
            if (!arctar && !fltar)
            {
                // Perform character translations
                ptr = info->name;
                while (*ptr)
                {
                    if (*ptr == FS_CHAR_EXTENSION) *ptr = FS_CHAR_SEPARATOR;
                    else if (*ptr == FS_CHAR_SEPARATOR) *ptr = FS_CHAR_EXTENSION;
                    ptr++;
                }
            }

            // Check if a directory
            ptr = strrchr(info->name, FS_CHAR_SEPARATOR);
            if (ptr && !ptr[1])
            {
                *ptr = '\0';
                info->obj_type = fileswitch_IS_DIR;
            }
            else
            {
                info->obj_type = fileswitch_IS_FILE;
            }

            // Process the main fields
            if ((sscanf(block->header.mode, "%7o", &mode) == 1)
                && (sscanf(block->header.size, "%11o", &size) == 1)
                && (sscanf(block->header.modified, "%11o", &modified) == 1))
            {
                const date_riscos *date;

                // Managed to read common header fields
                info->attr = attr_from_unix(mode);
                info->size = size;
                date = date_from_unix(modified);
                info->load_addr = 0xfff00000 | (type << 8) | date->words.high;
                info->exec_addr = date->words.low;
            }
            else
            {
                // Unable to decode header
                err = &err_bad_tar_header;
            }

            // Attempt to process extended formats
            if (!err && arctar)
            {
                bits linkflag;
                bits stamped;
                fileswitch_attr attr;
                bits load;
                bits exec;

                // Attempt to decode arctar fields
                if (sscanf(block->header.ext.arctar.linkflag, "%1o", &linkflag) == 1)
                {
                    if (linkflag == 0) info->obj_type = fileswitch_IS_FILE;
                    else if (linkflag == 5) info->obj_type = fileswitch_IS_DIR;
                }
                if ((sscanf(block->header.ext.arctar.stamped, "%1o", &stamped) == 1)
                    && stamped)
                {
                    date_riscos date;

                    if ((sscanf(block->header.ext.arctar.type, "%5o", &type) == 1)
                        && (sscanf(block->header.ext.arctar.datestamp, "%2x%8x",
                                   &date.words.high, &date.words.low) == 2))
                    {
                        info->load_addr = 0xfff00000 | (type << 8)
                                          | date.words.high;
                        info->exec_addr = date.words.low;
                    }
                }
                if (sscanf(block->header.ext.arctar.attr, "%4o", &attr) == 1)
                {
                    info->attr = attr;
                }
                if ((sscanf(block->header.ext.arctar.load, "%12o", &load) == 1)
                    && (sscanf(block->header.ext.arctar.exec, "%12o", &exec) == 1))
                {
                    info->load_addr = load;
                    info->exec_addr = exec;
                }
            }
            if (!err && fltar)
            {
                date_riscos date;
                bits load;
                bits exec;
                fileswitch_attr attr;

                // Attempt to decode fltar fields
                if (sscanf(block->header.ext.fltar.date, "%2x %8x",
                           &date.words.high, &date.words.low) == 2)
                {
                    info->load_addr = 0xfff00000 | (type << 8)
                                      | date.words.high;
                    info->exec_addr = date.words.low;
                }
                if (sscanf(block->header.ext.fltar.attr, "%11o", &attr) == 1)
                {
                    info->attr = attr;
                }
                if ((sscanf(block->header.ext.fltar.load, "%11o", &load) == 1)
                    && (sscanf(block->header.ext.fltar.exec, "%11o", &exec) == 1))
                {
                    info->load_addr = load;
                    info->exec_addr = exec;
                }
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the tar file to read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details of the next file from the tar file. This
                  should be called immediately after an operation has completed.
*/
static os_error *tar_next(tar_handle handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar next %p", handle))

        // Read the next header block
        err = tar_read_block(handle, handle->block, 1);

        // Attempt to decode the header
        if (!err) err = tar_decode_header(handle->block, &handle->info);

        // No files remain if an error produced
        if (err) handle->info.obj_type = fileswitch_NOT_FOUND;

        // Reset the operation status
        handle->done = 0;
        handle->remain = (handle->info.size + TAR_BLOCK - 1) / TAR_BLOCK;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : name          - The name of the tar file to open.
                  handle        - Variable to receive a pointer to the handle.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Open an existing tar file for reading.
*/
os_error *tar_open_in(const char *name, tar_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!name || !handle) err = &err_bad_parms;
    else
    {
        tar_handle ptr;

        DEBUG_PRINTF(("Tar open in '%s'", name))

        // Attempt to allocate memory for the tar file handle
        ptr = (tar_handle) MEM_MALLOC(sizeof(struct tar_handle));
        if (!ptr) err = &err_buffer;

        // Set the initial details
        if (!err)
        {
            ptr->reference = 1;
            ptr->write = FALSE;
            ptr->op = TAR_IDLE;
            ptr->op_file = 0;
        }

        // Store the filename
        if (!err && (sizeof(ptr->name) <= strlen(name))) err = &err_bad_name;
        if (!err) strcpy(ptr->name, name);

        // Attempt to open the file
        if (!err)
        {
            err = xosfind_openinw(osfind_NO_PATH
                                  | osfind_ERROR_IF_ABSENT
                                  | osfind_ERROR_IF_DIR,
                                  name, NULL, &ptr->file);
        }
        if (!err && !ptr->file) err = &err_not_found;

        // Read the details of the first file
        if (!err) err = tar_next(ptr);

        // Close the file if error produced
        if (err && ptr && ptr->file)
        {
            xosfind_closew(ptr->file);
            ptr->file = 0;
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
    Parameters  : name          - The name of the tar file to create.
                  handle        - Variable to receive a pointer to the handle.
                  append        - Should data be appended if the file already
                                  exists.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create and open a new tar file for writing.
*/
os_error *tar_open_out(const char *name, tar_handle *handle, bool append)
{
    os_error *err = NULL;

    // Check function parameters
    if (!name || !handle) err = &err_bad_parms;
    else
    {
        tar_handle ptr;

        DEBUG_PRINTF(("Tar open out '%s'", name))

        // Attempt to allocate memory for the tar file handle
        ptr = (tar_handle) MEM_MALLOC(sizeof(struct tar_handle));
        if (!ptr) err = &err_buffer;

        // Set the initial details
        if (!err)
        {
            ptr->reference = 1;
            ptr->write = TRUE;
            ptr->file = 0;
            ptr->op = TAR_IDLE;
            ptr->op_file = 0;
        }

        // Store the filename
        if (!err && (sizeof(ptr->name) <= strlen(name))) err = &err_bad_name;
        if (!err) strcpy(ptr->name, name);

        // Attempt to open an existing file
        if (!err && append)
        {
            err = xosfind_openupw(osfind_NO_PATH
                                  | osfind_ERROR_IF_DIR,
                                  name, NULL, &ptr->file);
            if (err)
            {
                // Suppress any error to allow a new file to be created
                err = NULL;
                ptr->file = 0;
            }
        }
        if (!err && ptr->file)
        {
            int length = 0;

            // Read the current length of the file
            err = xosargs_read_extw(ptr->file, &length);

            // Assume file ends with an end of file marker
            if (!err && ((length % TAR_BLOCK) || (length < TAR_BLOCK)))
            {
                err = &err_tar_eof;
            }
            if (!err) length -= TAR_BLOCK;

            // Set the file pointer to the start of the end of file block
            if (!err) err = xosargs_set_ptrw(ptr->file, length);

            // Verify that it is an end of file block
            if (!err) err = tar_read_block(ptr, ptr->block, 1);
            if (!err && !tar_blank(ptr->block)) err = &err_tar_eof;

            // Reset the file pointer and truncate the file
            if (!err) err = xosargs_set_ptrw(ptr->file, length);
            if (!err) err = xosargs_set_extw(ptr->file, length);
        }

        // Attempt to create the file
        if (!err && !ptr->file)
        {
            err = xosfind_openoutw(osfind_NO_PATH
                                   | osfind_ERROR_IF_DIR,
                                   name, NULL, &ptr->file);
        }
        if (!err && !ptr->file) err = &err_not_found;

        // Close the file if an error
        if (err && ptr && ptr->file) xosfind_closew(ptr->file);

        // Free the memory if an error
        if (err && ptr) MEM_FREE(ptr);

        // Set the return value
        *handle = err ? NULL : ptr;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The tar handle to clone.
                  dest          - Variable to receive the cloned handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone a tar file handle. The cloned handle references the
                  same tar file.
*/
os_error *tar_clone(tar_handle src, tar_handle *dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar clone %p", src))

        // Increment the reference count
        src->reference++;

        // Copy the handle
        *dest = src;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the tar file to close.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Close a previously opened tar file. This should be called the
                  same number of times as tar_open_in, tar_open_out, and
                  tar_clone.
*/
os_error *tar_close(tar_handle *handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !*handle) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar close %p", *handle))

        // Decrement the reference count
        if (!--(*handle)->reference)
        {
            // Ensure that any outstanding operation has been completed
            err = tar_complete(*handle);

            // Add an end of file block to output files
            if (!err && (*handle)->write)
            {
                memset((*handle)->block[0].data, 0, TAR_BLOCK);
                err = tar_write_block(*handle, (*handle)->block, 1);
            }

            // Close the file
            xosfind_closew((*handle)->file);

            // Set the filetype if an output file
            if (!err && (*handle)->write)
            {
                err = xosfile_set_type((*handle)->name, TAR_TYPE);
            }

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
    Parameters  : handle        - Handle of the tar file to read.
                  info          - Variable to receive a pointer to the details
                                  of the next file, or NULL if no more.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details of the next file from a tar file.
*/
os_error *tar_info(tar_handle handle, const fs_info **info)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !info) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar info %p", handle))

        // Ensure that any outstanding operation has been completed
        err = tar_complete(handle);

        // Set the return value
        *info = err || (handle->info.obj_type == fileswitch_NOT_FOUND)
                ? NULL : &handle->info;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : perc  - The percentage of the default step size to use.
                  max   - Maximum acceptable step size.
    Returns     : bits  - Recommended step size.
    Description : Calculate the next step size to use.
*/
static bits tar_step(bits perc, bits max)
{
    bits step;

    // Calculate the next step size
    step = (tar_step_blocks * perc) / 100;

    // Ensure that there is at least one block
    if (!step) step = 1;

    // Enforce the maximum step size
    if (TAR_MAX_BLOCKS < step) step = TAR_MAX_BLOCKS;
    if (max < step) step = max;

    // Return the step size
    return step;
}

/*
    Parameters  : void
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start timing of a step.
*/
static os_error *tar_step_pre(void)
{
    os_error *err = NULL;

    // Record the start time
    tar_step_start = util_time();

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : End timing of a step.
*/
static os_error *tar_step_post(void)
{
    os_error *err = NULL;
    bits time = util_time() - tar_step_start;

    // Update the step size if appropriate
    if (time < TAR_STEP_CENTISECONDS)
    {
        if (tar_step_blocks < TAR_MAX_BLOCKS) tar_step_blocks++;
    }
    else if (TAR_STEP_CENTISECONDS < time)
    {
        if (0 < tar_step_blocks) tar_step_blocks--;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The name of the source file.
                  name          - The name to store for the file.
                  dest          - Handle of the tar file to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start adding the specified file to the tar file. The operation
                  should be completed using tar_continue or tar_complete.
*/
os_error *tar_add(const char *src, const char *name, tar_handle dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !name || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar add '%s' to %p as '%s'", src, dest, name))

        // Ensure that any outstanding operation has been completed
        err = tar_complete(dest);

        // Read the details for the file
        if (!err)
        {
            err = xosfile_read(src, &dest->info.obj_type,
                               &dest->info.load_addr, &dest->info.exec_addr,
                               &dest->info.size, &dest->info.attr);
        }
        if (!err && (dest->info.obj_type == fileswitch_NOT_FOUND))
        {
            err = &err_not_found;
        }

        // Overwrite the filename to store
        if (!err && (sizeof(dest->info.name) <= strlen(name)))
        {
            err = &err_bad_name;
        }
        if (!err) strcpy(dest->info.name, name);

        // Calculate the number of blocks required
        dest->done = 0;
        dest->remain = dest->info.obj_type == fileswitch_IS_DIR
                       ? 0
                       : (dest->info.size + TAR_BLOCK - 1) / TAR_BLOCK;

        // Open source file if any data to read
        if (!err && dest->remain)
        {
            err = xosfind_openinw(osfind_NO_PATH
                                  | osfind_ERROR_IF_ABSENT
                                  | osfind_ERROR_IF_DIR,
                                  src, NULL, &dest->op_file);
            if (!err && !dest->op_file) err = &err_not_found;
        }

        // Construct a header block
        if (!err) err = tar_build_header(&dest->info, dest->block);

        // Write the file header to the destination file
        if (!err) err = tar_write_block(dest, dest->block, 1);

        // Set the operation details or close the source file
        if (!err) dest->op = TAR_ADD;
        else if (dest->op_file)
        {
            // Close the file
            xosfind_closew(dest->op_file);
            dest->op_file = 0;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dest          - Handle of the tar file to write.
                  done          - Variable to receive the units of operation
                                  completed.
                  remain        - Variable to receive the units of operation
                                  still to be completed, or 0 if no operation
                                  in progress.
                  step          - Variable to receive the units to be completed
                                  during the next step.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Continue adding a file to the tar file.
*/
static os_error *tar_continue_add(tar_handle dest, bits *done, bits *remain,
                                  bits *step)
{
    os_error *err = NULL;

    // Check function parameters
    if (!dest || !done || !remain || !step) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar continue add %p", dest))

        // Special case if finished
        if (dest->op_file && dest->remain)
        {
            bits blocks = tar_step(TAR_ADD_BLOCKS, dest->remain);
            int unread;

            // Start timing
            err = tar_step_pre();

            // Attempt to read the next blocks
            if (!err)
            {
                err = xosgbpb_readw(dest->op_file, dest->block[0].data,
                                    TAR_BLOCK * blocks, &unread);
            }
            if (!err) dest->remain -= blocks;

            // Pad with nulls
            if (!err && unread)
            {
                memset(dest->block[0].data + TAR_BLOCK * blocks - unread, 0,
                       unread);
            }

            // Write the blocks to the destination file
            if (!err) err = tar_write_block(dest, dest->block, blocks);
            if (!err) dest->done += blocks;

            // End timing
            if (!err) err = tar_step_post();

            // Set the return values
            if (!err)
            {
                *done = dest->done;
                *remain = dest->remain + 1;
                *step = tar_step(TAR_ADD_BLOCKS, dest->remain);
                if (!*step) *step = 1;
            }
            else
            {
                // Close the source file
                xosfind_closew(dest->op_file);
                dest->op_file = 0;
            }
        }
        else
        {
            // The operation has finished
            dest->op = TAR_IDLE;

            // Close the source file
            if (dest->op_file)
            {
                xosfind_closew(dest->op_file);
                dest->op_file = 0;
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the tar file to read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start skipping the next file from the tar file. The operation
                  should be completed using tar_continue or tar_complete.
*/
os_error *tar_skip(tar_handle handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar skip %p", handle))

        // Ensure that any outstanding operation has been completed
        err = tar_complete(handle);

        // Skip over all the blocks of the current file
        if (!err) err = tar_skip_block(handle, handle->remain);

        // Read the details of the next file
        if (!err) err = tar_next(handle);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - Handle of the tar file to read.
                  dest          - Tye name of the destination file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start extracting the next file from the tar file. The
                  operation should be completed using tar_continue or
                  tar_complete.
*/
os_error *tar_extract(tar_handle src, const char *dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar extract %p to '%s'", src, dest))

        // Ensure that any outstanding operation has been completed
        err = tar_complete(src);

        // Copy the destination file name
        if (!err && (sizeof(src->info.name) <= strlen(dest)))
        {
            err = &err_bad_name;
        }
        if (!err) strcpy(src->info.name, dest);

        // Special action if a directory
        if (!err)
        {
            if (src->info.obj_type == fileswitch_IS_DIR)
            {
                // Just create the directory
                err = xosfile_create_dir(dest, 0);
            }
            else
            {
                // Create the destination file
                err = xosfind_openoutw(osfind_NO_PATH
                                       | osfind_ERROR_IF_DIR,
                                       dest, NULL, &src->op_file);
                if (!err && !src->op_file) err = &err_not_found;
            }
        }

        // Set the operation details or close the destination file
        if (!err) src->op = TAR_EXTRACT;
        else if (src->op_file)
        {
            // Close the file
            xosfind_closew(src->op_file);
            src->op_file = 0;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - Handle of the tar file to read.
                  done          - Variable to receive the units of operation
                                  completed.
                  remain        - Variable to receive the units of operation
                                  still to be completed, or 0 if no operation
                                  in progress.
                  step          - Variable to receive the units to be completed
                                  during the next step.
                  dest          - Tye name of the destination file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Continue extracting the next file from the tar file.
*/
static os_error *tar_continue_extract(tar_handle src, bits *done, bits *remain,
                                      bits *step)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !done || !remain || !step) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar continue extract %p", src))

        // Special case if finished
        if (src->op_file && src->remain)
        {
            bits blocks = tar_step(TAR_EXTRACT_BLOCKS, src->remain);
            int unwritten;

            // Start timing
            err = tar_step_pre();

            // Read the next blocks from the source file
            if (!err) err = tar_read_block(src, src->block, blocks);
            if (!err) src->remain -= blocks;

            // Write the blocks to the destination file
            if (!err)
            {
                err = xosgbpb_writew(src->op_file, src->block[0].data,
                                     TAR_BLOCK * blocks, &unwritten);
            }
            if (!err && unwritten) err = &err_eof;
            if (!err) src->done += blocks;

            // End timing
            if (!err) err = tar_step_post();

            // Set the return values
            if (!err)
            {
                *done = src->done;
                *remain = src->remain + 1;
                *step = tar_step(TAR_EXTRACT_BLOCKS, src->remain);
                if (!*step) *step = 1;
            }
            else
            {
                // Close the file
                xosfind_closew(src->op_file);
                src->op_file = 0;
            }
        }
        else
        {
            // The operation has finished
            src->op = TAR_IDLE;

            // Set the file size and close the file
            if (src->op_file)
            {
                // Set the file size
                err = xosargs_set_extw(src->op_file, src->info.size);

                // Close the file
                xosfind_closew(src->op_file);
                src->op_file = 0;
            }

            // Write the catalogue entry
            if (!err)
            {
                err = xosfile_write(src->info.name, src->info.load_addr,
                                    src->info.exec_addr, src->info.attr);
            }

            // Read the details of the next file
            if (!err) err = tar_next(src);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - Handle of the tar file to read.
                  dest          - Handle of the tar file to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Start copying the next file to another tar file. The
                  operation should be completed using tar_continue or
                  tar_complete.
*/
os_error *tar_copy(tar_handle src, tar_handle dest)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar copy %p to %p", src, dest))

        // Ensure that any outstanding operation has been completed
        err = tar_complete(src);

        // Write the file header to the destination file
        if (!err) err = tar_write_block(dest, src->block, 1);

        // Set the operation details
        if (!err)
        {
            src->op = TAR_COPY_SRC;
            src->partner = dest;
            src->done = 0;
            dest->op = TAR_COPY_DEST;
            dest->partner = src;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - Handle of the tar file to read.
                  dest          - Handle of the tar file to write.
                  done          - Variable to receive the units of operation
                                  completed.
                  remain        - Variable to receive the units of operation
                                  still to be completed, or 0 if no operation
                                  in progress.
                  step          - Variable to receive the units to be completed
                                  during the next step.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Continue copying the next file to another tar file.
*/
static os_error *tar_continue_copy(tar_handle src, tar_handle dest, bits *done,
                                   bits *remain, bits *step)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest || !done || !remain || !step) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar continue copy %p to %p", src, dest))

        // Special case if finished
        if (src->remain)
        {
            bits blocks = tar_step(TAR_COPY_BLOCKS, src->remain);

            // Start timing
            err = tar_step_pre();

            // Read the blocks from the source file
            if (!err) err = tar_read_block(src, dest->block, blocks);
            if (!err) src->remain -= blocks;

            // Write the blocks to the destination file
            if (!err) err = tar_write_block(dest, dest->block, blocks);
            if (!err) src->done += blocks;

            // End timing
            if (!err) err = tar_step_post();

            // Set the return values
            if (!err)
            {
                *done = src->done;
                *remain = src->remain + 1;
                *step = tar_step(TAR_COPY_BLOCKS, src->remain);
                if (!*step) *step = 1;
            }
        }
        else
        {
            // The operation has finished
            src->op = TAR_IDLE;
            dest->op = TAR_IDLE;

            // Read the details of the next file
            if (!err) err = tar_next(src);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the tar file to process.
                  done          - Variable to receive the units of operation
                                  completed.
                  remain        - Variable to receive the units of operation
                                  still to be completed, or 0 if no operation
                                  in progress.
                  step          - Variable to receive the units to be completed
                                  during the next step.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Continue an operation that is in progress. Note that for a
                  copy operation either the source or destination handle may
                  be specified.
*/
os_error *tar_continue(tar_handle handle, bits *done, bits *remain,
                       bits *step)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle || !done || !remain || !step) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Tar continue %p", handle))

        // Set the default return values
        *done = 0;
        *remain = 0;
        *step = 0;

        // Action depends on the current operation
        switch (handle->op)
        {
            case TAR_IDLE:
                // No active operation
                break;

            case TAR_ADD:
                // Adding a file
                err = tar_continue_add(handle, done, remain, step);
                break;

            case TAR_EXTRACT:
                // Extracting a file
                err = tar_continue_extract(handle, done, remain, step);
                break;

            case TAR_COPY_SRC:
                // Copying a file with this as the source
                err = tar_continue_copy(handle, handle->partner,
                                        done, remain, step);
                break;

            case TAR_COPY_DEST:
                // Copying a file with this as the destination
                err = tar_continue_copy(handle->partner, handle,
                                        done, remain, step);
                break;

            default:
                // No other operation types expected
                err = &err_bad_tar_op;
        }

        // Ensure that the return values are sensible
        if (!err && (*remain < *step)) *step = *remain;

        // Abort the operation if an error was produced
        if (err) handle->op = TAR_IDLE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the tar file to process.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Complete an operation that is in progress. Note that for a
                  copy operation either the source or destination handle may
                  be specified.
*/
os_error *tar_complete(tar_handle handle)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        bits done;
        bits remain;
        bits step;

        DEBUG_PRINTF(("Tar complete %p", handle))

        // Check whether an operation is in progress
        err = tar_continue(handle, &done, &remain, &step);
        if (!err && remain)
        {
            // This could be a long operation so enable the hourglass
            err = xhourglass_on();

            // Loop until an error occurs or the operation has been completed
            while (!err && remain)
            {
                // Update the hourglass percentage
                err = xhourglass_percentage(done * 100 / (done + remain));

                // Continue the operation
                if (!err) err = tar_continue(handle, &done, &remain, &step);
            }

            // Switch the hourglass off
            if (err) xhourglass_smash();
            else err = xhourglass_off();
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the tar file to read.
                  done          - Variable to receive the current offset within
                                  the tar file.
                  remain        - Variable to receive the size of the file that
                                  has not been processed.
                  step          - Variable to receive the size of the next file
                                  to read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Estimate the progress reading from the specified tar file.
*/
os_error *tar_position(tar_handle handle, bits *done, bits *remain, bits *step)
{
    os_error *err = NULL;

    // Check function parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        int size;
        int offset;

        DEBUG_PRINTF(("Tar position %p", handle))

        // Read the size of the file
        err = xosargs_read_extw(handle->file, &size);

        // Read the current position within the file
        if (!err) err = xosargs_read_ptrw(handle->file, &offset);

        // Set the return values
        if (done) *done = !err ? offset : 0;
        if (remain) *remain = !err ? size - offset : 0;
        if (step) *step = !err ? (handle->remain + 1) * TAR_BLOCK : 0;
    }

    // Return any error produced
    return err;
}
