/*
    File        : fsentry.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Filing system entry points for the PsiFS module.

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
#include "fsentry.h"

// Include clib header files
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "oslib/fileswitch.h"
#include "oslib/macros.h"
#include "oslib/os.h"

// Include project header files
#include "debug.h"
#include "err.h"
#include "idle.h"
#include "link.h"
#include "util.h"

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_Open.
*/
os_error *fsentry_open_handler(fsentry_open_params *params)
{
    os_error *err = NULL;
    fs_mode mode;
    const char *path;
    fs_handle handle;
    const fs_open_info *info;

    DEBUG_PRINTF(("FSEntry_Open %u, path='%s', handle=%u", params->in.reason, params->in.path, params->in.handle))

    // Disable the idle timeout
    idle_start();

    // Ensure that the name is canonicalised
    err = fs_internal_name_resolve(params->in.path, &path, NULL);
    if (!err) err = fs_wildcards(path);

    // Action depends on the reason code
    switch (params->in.reason)
    {
        case FSENTRY_OPEN_OPEN_READ:
            // Open for read
            DEBUG_PRINTF(("FSEntry_Open - open for read"))
            mode = FS_MODE_IN;
            break;

        case FSENTRY_OPEN_CREATE_UPDATE:
            // Create and open for update
            DEBUG_PRINTF(("FSEntry_Open - create and open for update"))
            mode = FS_MODE_OUT;
            break;

        case FSENTRY_OPEN_UPDATE:
            // Open for update
            DEBUG_PRINTF(("FSEntry_Open - open for update"))
            mode = FS_MODE_UP;
            break;

        default:
            // Unrecognised reason code
            err = &err_bad_parms;
            break;
    }

    // Attempt to open the object and read the details
    if (!err) err = fs_open(path, mode, params->in.handle, &handle);
    if (!err && (handle != FS_NONE)) err = fs_args(handle, &info, NULL);

    // Fill in the other details
    if (!err && (handle != FS_NONE))
    {
        // Set the details for the file
        params->out.information_word = info->info;
        params->out.handle = handle;
        params->out.buffer_size = FS_BUFFER_SIZE;
        params->out.extent = info->extent;
        params->out.allocated = info->allocated;
    }
    else
    {
        // Use defaults otherwise
        params->out.information_word = 0;
        params->out.handle = FS_NONE;
        params->out.buffer_size = 0;
        params->out.extent = 0;
        params->out.allocated = 0;
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_GetBytes.
*/
os_error *fsentry_getbytes_handler(fsentry_getbytes_params *params)
{
    os_error *err = NULL;
    fs_handle handle = params->in.handle;
    const fs_open_info *info;

    DEBUG_PRINTF(("FSEntry_GetBytes handle=%p", params->in.handle))

    // Disable the idle timeout
    idle_start();

    // Read the details for the file
    err = fs_args(handle, &info, NULL);
    if (!err)
    {
        // Action depends on whether the file is buffered
        if (info->info & FS_FILE_INFO_UNBUFFERED_GBPB)
        {
            DEBUG_PRINTF(("FSEntry_GetBytes - unbuffered pointer=%u", info->sequential))

            // Check if at end of file
            if (info->sequential < info->extent)
            {
                // Read a single byte and increment the sequential pointer
                err = fs_read(handle, &params->out.value, info->sequential, 1);
                if (!err) err = fs_sequential(handle, info->sequential + 1);
                params->out.success = !err;
            }
            else params->out.success = FALSE;
        }
        else
        {
            DEBUG_PRINTF(("FSEntry_GetBytes - buffered buffer=%p, offset=%u, bytes=%u", params->in.buffer, params->in.offset, params->in.bytes))

            // Get bytes from a buffered file
            err = fs_read(handle, params->in.buffer, params->in.offset,
                          params->in.bytes);
        }
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_PutBytes.
*/
os_error *fsentry_putbytes_handler(fsentry_putbytes_params *params)
{
    os_error *err = NULL;
    fs_handle handle = params->in.handle;
    const fs_open_info *info;

    DEBUG_PRINTF(("FSEntry_PutBytes handle=%p", params->in.handle))

    // Disable the idle timeout
    idle_start();

    // Read the details for the file
    err = fs_args(handle, &info, NULL);
    if (!err)
    {
        // Action depends on whether the file is buffered
        if (info->info & FS_FILE_INFO_UNBUFFERED_GBPB)
        {
            DEBUG_PRINTF(("FSEntry_PutBytes - unbuffered pointer=%u, byte=%u", info->sequential, params->in.value))

            // Write a single byte and increment the sequential pointer
            err = fs_write(handle, &params->in.value, info->sequential, 1);
            if (!err) err = fs_sequential(handle, info->sequential + 1);
        }
        else
        {
            DEBUG_PRINTF(("FSEntry_PutBytes - buffered buffer=%p, offset=%u, bytes=%u", params->in.buffer, params->in.offset, params->in.bytes))

            // Write bytes to a buffered file
            err = fs_write(handle, params->in.buffer, params->in.offset,
                           params->in.bytes);
        }
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_Args.
*/
os_error *fsentry_args_handler(fsentry_args_params *params)
{
    os_error *err = NULL;
    fs_handle handle = params->in.handle;
    const fs_open_info *info;
    const char *path;
    const fs_info *details;

    DEBUG_PRINTF(("FSEntry_Args %u, handle=%p", params->in.reason, params->in.handle))

    // Disable the idle timeout
    idle_start();

    // Read the details for the file
    err = fs_args(handle, &info, &path);
    if (!err)
    {
        // Action depends on the reason code
        switch (params->in.reason)
        {
            case FSENTRY_ARGS_READ_SEQUENTIAL:
                // Read sequential file pointer
                DEBUG_PRINTF(("FSEntry_Args - read sequential"))
                params->out_read_sequential.sequential = info->sequential;
                break;

            case FSENTRY_ARGS_WRITE_SEQUENTIAL:
                // Write sequential file pointer
                DEBUG_PRINTF(("FSEntry_Args - write sequential =%u", params->in_write_sequential.sequential))
                err = fs_sequential(handle, params->in_write_sequential.sequential);
                break;

            case FSENTRY_ARGS_READ_EXTENT:
                // Read file extent
                DEBUG_PRINTF(("FSEntry_Args - read extent"))
                params->out_read_extent.extent = info->extent;
                break;

            case FSENTRY_ARGS_WRITE_EXTENT:
                // Write file extent
                DEBUG_PRINTF(("FSEntry_Args - write extent =%u", params->in_write_extent.extent))
                err = fs_extent(handle, params->in_write_extent.extent);
                break;

            case FSENTRY_ARGS_READ_ALLOCATED:
                // Read size allocated to file
                DEBUG_PRINTF(("FSEntry_Args - read allocated"))
                params->out_read_allocated.allocated = info->allocated;
                break;

            case FSENTRY_ARGS_EOF_CHECK:
                // EOF check
                DEBUG_PRINTF(("FSEntry_Args - EOF check"))
                params->out_eof_check.eof = info->sequential < info->extent
                                            ? 0 : FS_EOF;
                break;

            case FSENTRY_ARGS_FLUSH:
                // Notify of a flush
                DEBUG_PRINTF(("FSEntry_Args - notify of a flush"))
                err = fs_flush(handle);
                if (!err) err = fs_internal_name_resolve(path, NULL, &details);
                if (!err)
                {
                    params->out_flush.load_addr = details->load_addr;
                    params->out_flush.exec_addr = details->exec_addr;
                }
                break;

            case FSENTRY_ARGS_ENSURE:
                // Ensure file size
                DEBUG_PRINTF(("FSEntry_Args - ensure file size =%u", params->in_ensure.extent))
                err = fs_extent(handle, params->in_ensure.extent);
                if (!err) err = fs_args(handle, &info, NULL);
                if (!err) params->out_ensure.extent = info->extent;
                break;

            case FSENTRY_ARGS_ZERO:
                // Write zeros to file
                DEBUG_PRINTF(("FSEntry_Args - write zeros ptr=%u, bytes=%u", params->in_zero.offset, params->in_zero.bytes))
                err = fs_write_zeros(handle, params->in_zero.offset,
                                     params->in_zero.bytes);
                break;

            case FSENTRY_ARGS_READ_STAMP:
                // Read file datestamp
                DEBUG_PRINTF(("FSEntry_Args - read file datestamp"))
                if (!err) err = fs_internal_name_resolve(path, NULL, &details);
                if (!err)
                {
                    params->out_read_stamp.load_addr = details->load_addr;
                    params->out_read_stamp.exec_addr = details->exec_addr;
                }
                break;

            case FSENTRY_ARGS_IMAGE_STAMP:
                // Inform of new image stamp (ignored)
                DEBUG_PRINTF(("FSEntry_Args - new image stamp"))
                break;

            default:
                // Unrecognised reason code
                err = &err_bad_parms;
                break;
        }
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_Close.
*/
os_error *fsentry_close_handler(fsentry_close_params *params)
{
    os_error *err = NULL;
    fs_handle handle = params->in.handle;
    const char *path;

    DEBUG_PRINTF(("FSEntry_Close handle=%p, load=0x%08X, exec=0x%08X", params->in.handle, params->in.load_addr, params->in.exec_addr))

    // Disable the idle timeout
    idle_start();

    // Read the details for the file
    err = fs_args(handle, NULL, &path);

    // Set the load and execution address if required
    if (!err && (params->in.load_addr || params->in.exec_addr))
    {
        err = fs_stamp(path, params->in.load_addr, params->in.exec_addr);
    }

    // Close the file
    if (!err) err = fs_close(handle);

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_File.
*/
os_error *fsentry_file_handler(fsentry_file_params *params)
{
    os_error *err = NULL;
    const char *path;
    const fs_info *details;

    DEBUG_PRINTF(("FSEntry_File %u, path='%s'", params->in.reason, params->in.path))

    // Disable the idle timeout
    idle_start();

    // Read the details for the file
    err = fs_internal_name_resolve(params->in.path, &path, &details);

    // Action depends on the reason code
    switch (params->in.reason)
    {
        case FSENTRY_FILE_SAVE:
            // Save file (not used)
            DEBUG_PRINTF(("FSEntry_File - save file load=0x%08X, exec=0x%08X, buffer=%p, end=%p", params->in_save.load_addr, params->in_save.exec_addr, params->in_save.buffer, params->in_save.buffer_end))
            err = &err_not_supported;
            break;

        case FSENTRY_FILE_WRITE_CAT:
            // Write catalogue information
            DEBUG_PRINTF(("FSEntry_File - write catalogue information load=0x%08X, exec=0x%08X, attr=0x%X", params->in_write_cat.load_addr, params->in_write_cat.exec_addr, params->in_write_cat.attr))
            if (!err && (details->obj_type != fileswitch_NOT_FOUND))
            {
                err = fs_access(path, params->in_write_cat.attr);
                if (!err) err = fs_stamp(path, params->in_write_cat.load_addr, params->in_write_cat.exec_addr);
            }
            break;

        case FSENTRY_FILE_WRITE_LOAD:
            // Write load address
            DEBUG_PRINTF(("FSEntry_File - write load address =0x%08X", params->in_write_load.load_addr))
            if (!err && (details->obj_type != fileswitch_NOT_FOUND))
            {
                err = fs_stamp(path, params->in_write_load.load_addr, details->exec_addr);
            }
            break;

        case FSENTRY_FILE_WRITE_EXEC:
            // Write execution address
            DEBUG_PRINTF(("FSEntry_File - write execution address =0x%08X", params->in_write_exec.exec_addr))
            if (!err && (details->obj_type != fileswitch_NOT_FOUND))
            {
                err = fs_stamp(path, details->load_addr, params->in_write_exec.exec_addr);
            }
            break;

        case FSENTRY_FILE_WRITE_ATTR:
            // Write attributes
            DEBUG_PRINTF(("FSEntry_File - write attributes =0x%X", params->in_write_cat.attr))
            if (!err && (details->obj_type != fileswitch_NOT_FOUND))
            {
                err = fs_access(path, params->in_write_attr.attr);
            }
            break;

        case FSENTRY_FILE_READ_CAT:
            // Read catalogue information
            DEBUG_PRINTF(("FSEntry_File - read catalogue information"))
            if (!err && (details->obj_type != fileswitch_NOT_FOUND))
            {
                params->out_read_cat.type = details->obj_type;
                params->out_read_cat.load_addr = details->load_addr;
                params->out_read_cat.exec_addr = details->exec_addr;
                params->out_read_cat.length = details->size;
                params->out_read_cat.attr = details->attr;
            }
            else if (!err || ERR_EQ(*err, err_not_found))
            {
                params->out_read_cat.type = fileswitch_NOT_FOUND;
                err = NULL;
            }
            break;

        case FSENTRY_FILE_DELETE:
            // Delete object
            DEBUG_PRINTF(("FSEntry_File - delete object"))
            if (!err && (details->obj_type != fileswitch_NOT_FOUND))
            {
                params->out_delete.type = details->obj_type;
                params->out_delete.load_addr = details->load_addr;
                params->out_delete.exec_addr = details->exec_addr;
                params->out_delete.length = details->size;
                params->out_delete.attr = details->attr;
                err = fs_remove(path);
            }
            else if (!err || ERR_EQ(*err, err_not_found))
            {
                params->out_delete.type = fileswitch_NOT_FOUND;
                err = NULL;
            }
            break;

        case FSENTRY_FILE_CREATE_FILE:
            // Create file
            DEBUG_PRINTF(("FSEntry_File - create file load=0x%08X, exec=0x%08X, start=0%p, end=%p", params->in_create_file.load_addr, params->in_create_file.exec_addr, params->in_create_file.buffer, params->in_create_file.buffer_end))
            if (!err && (details->obj_type != fileswitch_NOT_FOUND))
            {
                err = fs_remove(path);
            }
            if (!err)
            {
                fs_handle handle;

                err = fs_open(path, FS_MODE_OUT, 0, &handle);
                if (!err && (handle == FS_NONE)) err = &err_not_found;
                if (!err)
                {
                    err = fs_stamp(path, params->in_create_file.load_addr,
                                   params->in_create_file.exec_addr);
                    if (!err)
                    {
                        err = fs_extent(handle,
                                        params->in_create_file.buffer_end
                                        - params->in_create_file.buffer);
                    }
                    if (!err && (details->obj_type != fileswitch_NOT_FOUND))
                    {
                        err = fs_access(path, details->attr);
                    }
                    fs_close(handle);
                }
            }
            if (!err) params->out_create_file.leaf = params->in_create_file.path;
            break;

        case FSENTRY_FILE_CREATE_DIR:
            // Create directory
            DEBUG_PRINTF(("FSEntry_File - create directory load=0x%08X, exec=0x%08X, entries=%u", params->in_create_dir.load_addr, params->in_create_dir.exec_addr, params->in_create_dir.entries))
            if (!err) err = fs_mkdir(path);
            if (!err) fs_stamp(path, params->in_create_dir.load_addr, params->in_create_dir.exec_addr);
            break;

        case FSENTRY_FILE_READ_CAT_NOT_LENGTH:
            // Read catalogue information except length (not used)
            DEBUG_PRINTF(("FSEntry_File - read catalogue information (no length)"))
            err = &err_not_supported;
            break;

        case FSENTRY_FILE_READ_BLOCK_SIZE:
            // Read block size
            DEBUG_PRINTF(("FSEntry_File - read block size"))
            params->out_read_block_size.block_size = FS_BUFFER_SIZE;
            break;

        case FSENTRY_FILE_LOAD:
            // Load file (not used)
            DEBUG_PRINTF(("FSEntry_File - load file"))
            err = &err_not_supported;
            break;

        default:
            // Unrecognised reason code
            err = &err_bad_parms;
            break;
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : reason        - The FSEntry_Func reason code to perform.
                  path          - The path to the directory to list, or NULL to
                                  list the CSD.
                  buffer        - The destination buffer.
                  number        - The number of items to read.
                  offset        - Offset of the first item to read.
                  buffer_size   - The size of the destination buffer.
                  out_number    - Variable to receive the number of items read.
                  out_offset    - Variable to receive the offset of the next
                                  item in the directory, or -1 if the end.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Perform an FSEntry_Func directory read.
*/
static os_error *fsentry_read_dir(fsentry_file_reason reason, const char *path,
                                  byte *buffer, bits number, int offset,
                                  bits buffer_size,
                                  bits *out_number, int *out_offset)
{
    os_error *err = NULL;
    const fs_info *details;

    DEBUG_PRINTF(("fsentry_read_dir %u, path='%s', buffer=%p, number=%u, offset=%i, buffer size=%u", reason, path, buffer, number, offset, buffer_size))

    // Disable the idle timeout
    idle_start();

    // Initially no entries read
    *out_offset = offset;
    *out_number = 0;

    // If directory name is null then use the CSD
    if (!path || !*path)
    {
        static fs_pathname csd;
        int spare;

        path = csd;
        err = xosfscontrol_read_dir(csd, osfscontrol_DIR_CSD, FS_NAME, sizeof(csd), &spare, NULL);
        if (!err && (spare < 0)) err = &err_bad_name;
    }

    // Verify that the path specifies a directory
    if (!err) err = fs_internal_name_resolve(path, &path, &details);
    if (!err && (details->obj_type != fileswitch_IS_DIR)) err = &err_not_found;

    // Keep trying until finished
    while (!err && number && (*out_offset != -1))
    {
        fs_info info;
        int offset = *out_offset;
        bits read;

        // Read the next entry
        err = fs_enumerate(path, "*", &offset, &info, 1, &read);
        if (!err)
        {
            if (read)
            {
                bits req;
                byte *src;
                osgbpb_SYSTEM_INFO(FS_MAX_LEAFNAME + 1) internal;

                // Construct the appropriate description block
                switch (reason)
                {
                    case FSENTRY_FUNC_READ_DIR:
                        // Read directory entries
                        src = (byte *) info.name;
                        req = strlen(info.name) + 1;
                        break;

                    case FSENTRY_FUNC_READ_DIR_INFO:
                        // Read directory entries and information
                        src = (byte *) &info;
                        req = ALIGN(osgbpb_SIZEOF_INFO(strlen(info.name) + 1));
                        break;

                    case FSENTRY_FUNC_READ_DIR_INFO_INTERNAL:
                        // Read directory entries and information
                        src = (byte *) &internal;
                        internal.load_addr = info.load_addr;
                        internal.exec_addr = info.exec_addr;
                        internal.size = info.size;
                        internal.attr = info.attr;
                        internal.obj_type = info.obj_type;
                        internal.sin = 0;
                        ((date_riscos *) &internal.stamp)->words.low = info.exec_addr;
                        ((date_riscos *) &internal.stamp)->words.high = info.load_addr & 0xff;
                        strcpy(internal.name, info.name);
                        req = ALIGN(osgbpb_SIZEOF_SYSTEM_INFO(strlen(internal.name) + 1));
                        break;

                    default:
                        // Unrecognised reason code
                        err = &err_bad_parms;
                        break;
                }

                // Copy the details if the buffer is large enough
                if (req <= buffer_size)
                {
                    memcpy(buffer, src, req);
                    buffer += req;
                    buffer_size -= req;
                    *out_offset = offset;
                    (*out_number)++;
                    number--;
                }
                else number = 0;
            }
            else if (offset == -1) *out_offset = offset;
        }
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_Func.
*/
os_error *fsentry_func_handler(fsentry_func_params *params)
{
    os_error *err = NULL;
    const char *path;
    const fs_info *details;

    DEBUG_PRINTF(("FSEntry_Func %u", params->in.reason))

    // Disable the idle timeout
    idle_start();

    // Action depends on the reason code
    switch (params->in.reason)
    {
        case FSENTRY_FUNC_SET_CSD:
            // Set current directory (not used)
            DEBUG_PRINTF(("FSEntry_Func - set current directory"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_SET_LIB:
            // Set library directory (not used)
            DEBUG_PRINTF(("FSEntry_Func - set library directory"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_CAT_CSD:
            // Catalogue directory (not used)
            DEBUG_PRINTF(("FSEntry_Func - catalogue directory"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_EX_CSD:
            // Examine directory (not used)
            DEBUG_PRINTF(("FSEntry_Func - examine directory"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_CAT_LIB:
            // Catalogue library directory (not used)
            DEBUG_PRINTF(("FSEntry_Func - catalogue library directory"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_EX_LIB:
            // Examine library directory (not used)
            DEBUG_PRINTF(("FSEntry_Func - examine library directory"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_INFO:
            // Examine object(s) (not used)
            DEBUG_PRINTF(("FSEntry_Func - examine object(s)"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_OPT:
            // Set filing system options
            DEBUG_PRINTF(("FSEntry_Func - set filing system options"))
            err = &err_bad_parms;
            break;

        case FSENTRY_FUNC_RENAME:
            // Rename object
            DEBUG_PRINTF(("FSEntry_Func - rename object source='%s', dest='%s'", params->in_rename.source, params->in_rename.dest))
            err = fs_internal_name_resolve(params->in_rename.source, &path, &details);
            if (!err && (details->obj_type == fileswitch_NOT_FOUND))
            {
                err = &err_not_found;
            }
            if (!err)
            {
                static fs_pathname src;

                strcpy(src, path);
                err = fs_internal_name(params->in_rename.dest, &path);
                if (!err) err = fs_wildcards(path);
                if (!err) err = fs_rename(src, path);
            }
            params->out_rename.invalid = BOOL(err);
            break;

        case FSENTRY_FUNC_ACCESS:
            // Access object(s) (not used)
            DEBUG_PRINTF(("FSEntry_Func - access object(s)"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_BOOT:
            // Boot filing system (not used)
            DEBUG_PRINTF(("FSEntry_Func - boot filing system"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_READ_BOOT_NAME:
            // Read name and boot (*OPT 4) option of disc
            DEBUG_PRINTF(("FSEntry_Func - read name and boot (*OPT 4) option of disc buffer=%p", params->in_read_boot_name.buffer))
            {
                char *buffer = (char *) params->in_read_boot_name.buffer;
                bits psr;
                int spare;

                err = xosfscontrol_read_dir(NULL, osfscontrol_DIR_CSD, FS_NAME, 0, &spare, NULL);
                if (!err) err = xos_validate_address((byte *) buffer, (byte *) buffer - spare, &psr);
                if (!err && (psr & _C)) err = &err_bad_parms;
                if (!err) err = xosfscontrol_read_dir(buffer, osfscontrol_DIR_CSD, FS_NAME, -spare, NULL, NULL);
                if (!err)
                {
                    char *ptr = strchr(buffer, FS_CHAR_SEPARATOR);

                    if (!ptr) ptr = strchr(buffer, '\0');
                    buffer[0] = ptr - buffer - 1;
                    *ptr = 0;
                }
            }
            break;

        case FSENTRY_FUNC_READ_CSD:
            // Read current directory name and privilege byte (not used)
            DEBUG_PRINTF(("FSEntry_Func - read current directory name and privilege byte"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_READ_LIB:
            // Read library directory name and privilege byte (not used)
            DEBUG_PRINTF(("FSEntry_Func - read library directory name and privilege byte"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_READ_DIR:
            // Read directory entries
            DEBUG_PRINTF(("FSEntry_Func - read directory entries path='%s', buffer=%p, number=%u, offset=%i, buffer size=%u", params->in_read_dir.path, params->in_read_dir.buffer, params->in_read_dir.number, params->in_read_dir.offset, params->in_read_dir.buffer_size))
            {
                bits psr;

                err = xos_validate_address(params->in_read_dir.buffer, params->in_read_dir.buffer + params->in_read_dir.buffer_size, &psr);
                if (!err && (psr & _C)) err = &err_bad_parms;
                if (!err) err = fsentry_read_dir(params->in_read_dir.reason, params->in_read_dir.path, params->in_read_dir.buffer, params->in_read_dir.number, params->in_read_dir.offset, params->in_read_dir.buffer_size, &params->out_read_dir.number, &params->out_read_dir.offset);
            }
            break;

        case FSENTRY_FUNC_READ_DIR_INFO:
            // Read directory entries and information
            DEBUG_PRINTF(("FSEntry_Func - read directory entries and information path='%s', buffer=%p, number=%u, offset=%i, buffer size=%u", params->in_read_dir_info.path, params->in_read_dir_info.buffer, params->in_read_dir_info.number, params->in_read_dir_info.offset, params->in_read_dir_info.buffer_size))
            err = fsentry_read_dir(params->in_read_dir_info.reason, params->in_read_dir_info.path, params->in_read_dir_info.buffer, params->in_read_dir_info.number, params->in_read_dir_info.offset, params->in_read_dir_info.buffer_size, &params->out_read_dir_info.number, &params->out_read_dir_info.offset);
            break;

        case FSENTRY_FUNC_SHUTDOWN:
            // Shut down
            DEBUG_PRINTF(("FSEntry_Func - shut down"))
            err = link_disable(FALSE);
            break;

        case FSENTRY_FUNC_BANNER:
            // Print start up banner
            DEBUG_PRINTF(("FSEntry_Func - print start up banner"))
            xos_write0(FS_NAME);
            xos_new_line();
            break;

        case FSENTRY_FUNC_READ_DIR_INFO_INTERNAL:
            // Read directory entries and information
            DEBUG_PRINTF(("FSEntry_Func - read directory entries and information path='%s', buffer=%p, number=%u, offset=%i, buffer size=%u", params->in_read_dir_info_internal.path, params->in_read_dir_info_internal.buffer, params->in_read_dir_info_internal.number, params->in_read_dir_info_internal.offset, params->in_read_dir_info_internal.buffer_size))
            err = fsentry_read_dir(params->in_read_dir_info_internal.reason, params->in_read_dir_info_internal.path, params->in_read_dir_info_internal.buffer, params->in_read_dir_info_internal.number, params->in_read_dir_info_internal.offset, params->in_read_dir_info_internal.buffer_size, &params->out_read_dir_info_internal.number, &params->out_read_dir_info_internal.offset);
            break;

        case FSENTRY_FUNC_FILE_INFO:
            // Output full information on object(s) (not used)
            DEBUG_PRINTF(("FSEntry_Func - output full information on object(s)"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_CANONICALISE:
            // Canonicalise special field and disc name
            DEBUG_PRINTF(("FSEntry_Func - canonicalise special field ='%s' and disc name ='%s', buffers=%p,%p, sizes=%u,%u", params->in_canonicalise.special_field ? params->in_canonicalise.special_field : "", params->in_canonicalise.disc_name ? params->in_canonicalise.disc_name : "", params->in_canonicalise.buffer_special_field, params->in_canonicalise.buffer_disc_name, params->in_canonicalise.buffer_special_field_size, params->in_canonicalise.buffer_disc_name_size))
            if (!params->in_canonicalise.buffer_special_field) params->in_canonicalise.buffer_special_field_size = 0;
            if (!params->in_canonicalise.buffer_disc_name) params->in_canonicalise.buffer_disc_name_size = 0;
            if (params->in_canonicalise.disc_name)
            {
                char drive;
                const fs_drive *details;

                err = fs_internal_drive(params->in_canonicalise.disc_name, &drive);
                if (!err) err = fs_drive_info(drive, TRUE, &details);
                if (!err && !details->present) err = &err_disc_not_found;
                if (!err)
                {
                    params->out_canonicalise.disc_name = params->in_canonicalise.buffer_disc_name;
                    params->out_canonicalise.disc_name_overflow = strlen(details->name) + 1 - params->in_canonicalise.buffer_disc_name_size;
                    if (params->out_canonicalise.disc_name) strncpy(params->out_canonicalise.disc_name, (char *) details->name, params->in_canonicalise.buffer_disc_name_size);
                }
                else
                {
                    params->out_canonicalise.disc_name = NULL;
                    params->out_canonicalise.disc_name_overflow = 0;
                }
            }
            else
            {
                params->out_canonicalise.disc_name = NULL;
                params->out_canonicalise.disc_name_overflow = 0;
            }
            params->out_canonicalise.special_field = NULL;
            params->out_canonicalise.special_field_overflow = 0;
            break;

        case FSENTRY_FUNC_RESOLVE_WILDCARD:
            // Resolve wildcard
            DEBUG_PRINTF(("FSEntry_Func - resolve wildcard path='%s', buffer=%p, name='%s', buffer size=%u", params->in_resolve_wildcard.path, params->in_resolve_wildcard.buffer_name, params->in_resolve_wildcard.name, params->in_resolve_wildcard.buffer_name_size))
            params->out_resolve_wildcard.overflow = -1;
            break;

        case FSENTRY_FUNC_READ_DEFECTS:
            // Read defect list (not used)
            DEBUG_PRINTF(("FSEntry_Func - read defect list"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_ADD_DEFECT:
            // Add a defect (not used)
            DEBUG_PRINTF(("FSEntry_Func - add a defect"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_READ_BOOT:
            // Read boot option
            DEBUG_PRINTF(("FSEntry_Func - read boot option path='%s'", params->in_read_boot.path))
            params->out_read_boot.option = 0;
            break;

        case FSENTRY_FUNC_WRITE_BOOT:
            // Write boot option (not used)
            DEBUG_PRINTF(("FSEntry_Func - write boot option"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_READ_MAP:
            // Read used space map (not used)
            DEBUG_PRINTF(("FSEntry_Func - read used space map"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_READ_FREE:
            // Read free space
            DEBUG_PRINTF(("FSEntry_Func - read free space name='%s'", params->in_read_free.path))
            err = fs_internal_name_resolve(params->in_read_free.path, &path, NULL);
            if (!err)
            {
                const fs_drive *info;

                err = fs_drive_info(path[1], FALSE, &info);
                if (!err)
                {
                    params->out_read_free.free = info->free_high ? -1 : info->free_low;
                    params->out_read_free.biggest = params->out_read_free.free;
                    params->out_read_free.size = info->size_high ? -1 : info->size_low;
                }
            }
            break;

        case FSENTRY_FUNC_NAME_IMAGE:
            // Name image
            DEBUG_PRINTF(("FSEntry_Func - name image path='%s', name='%s'", params->in_name_image.path, params->in_name_image.name))
            err = fs_internal_name_resolve(params->in_name_image.path, &path, NULL);
            if (!err) err = fs_drive_name(path[1], params->in_name_image.name);
            break;

        case FSENTRY_FUNC_STAMP_IMAGE:
            // Stamp image (not used)
            DEBUG_PRINTF(("FSEntry_Func - stamp image"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_GET_USAGE:
            // Get usage of offset (not used)
            DEBUG_PRINTF(("FSEntry_Func - get usage of offset"))
            err = &err_not_supported;
            break;

        case FSENTRY_FUNC_CHANGED_DIR:
            // Notification of changed directory (not used)
            DEBUG_PRINTF(("FSEntry_Func - notification of changed directory"))
            err = &err_not_supported;
            break;

        default:
            // Unrecognised reason code
            err = &err_bad_parms;
            break;
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_GBPB.
*/
os_error *fsentry_gbpb_handler(fsentry_gbpb_params *params)
{
    os_error *err = NULL;
    fs_handle handle = params->in.handle;
    bits ptr;
    bits bytes;
    const fs_open_info *info;

    DEBUG_PRINTF(("FSEntry_GBPB %u", params->in.reason))

    // Disable the idle timeout
    idle_start();

    // Read the details for the file
    err = fs_args(handle, &info, NULL);
    if (!err)
    {
        // Action depends on the reason code
        switch (params->in.reason)
        {
            case FSENTRY_GBPB_PUT_HERE:
                // Put multiple bytes to an unbuffered file
                DEBUG_PRINTF(("FSEntry_GBPB - put multiple bytes to an unbuffered file handle=%p, buffer=%p, pointer=%u, bytes=%u", params->in.handle, params->in.buffer, params->in.sequential, params->in.bytes))
                ptr = params->in.sequential;
                bytes = params->in.bytes;
                err = fs_write(handle, params->in.buffer, ptr, bytes);
                if (!err)
                {
                    params->out.buffer_end = params->in.buffer + bytes;
                    params->out.bytes_left = 0;
                    params->out.sequential = ptr + bytes;
                    err = fs_sequential(handle, params->out.sequential);
                }
                break;

            case FSENTRY_GBPB_PUT:
                // Put multiple bytes to an unbuffered file
                DEBUG_PRINTF(("FSEntry_GBPB - put multiple bytes to an unbuffered file handle=%p, buffer=%p, bytes=%u", params->in.handle, params->in.buffer, params->in.bytes))
                ptr = info->sequential;
                bytes = params->in.bytes;
                err = fs_write(handle, params->in.buffer, ptr, bytes);
                if (!err)
                {
                    params->out.buffer_end = params->in.buffer + bytes;
                    params->out.bytes_left = 0;
                    params->out.sequential = ptr + bytes;
                    err = fs_sequential(handle, params->out.sequential);
                }
                break;

            case FSENTRY_GBPB_GET_HERE:
                // Read bytes from an open file
                DEBUG_PRINTF(("FSEntry_GBPB - read bytes from an open file handle=%p, buffer=%p, pointer=%u, bytes=%u", params->in.handle, params->in.buffer, params->in.sequential, params->in.bytes))
                ptr = params->in.sequential;
                if (info->extent <= ptr) bytes = 0;
                else bytes = MIN(params->in.bytes, info->extent - ptr);
                err = fs_read(handle, params->in.buffer, ptr, bytes);
                if (!err)
                {
                    params->out.buffer_end = params->in.buffer + bytes;
                    params->out.bytes_left = params->in.bytes - bytes;
                    params->out.sequential = ptr + bytes;
                    if (params->out.sequential <= info->extent)
                    {
                        err = fs_sequential(handle, params->out.sequential);
                    }
                }
                break;

            case FSENTRY_GBPB_GET:
                // Read bytes from an open file
                DEBUG_PRINTF(("FSEntry_GBPB - read bytes from an open file handle=%p, buffer=%p, bytes=%u", params->in.handle, params->in.buffer, params->in.bytes))
                ptr = info->sequential;
                bytes = MIN(params->in.bytes, info->extent - ptr);
                err = fs_read(handle, params->in.buffer, ptr, bytes);
                if (!err)
                {
                    params->out.buffer_end = params->in.buffer + bytes;
                    params->out.bytes_left = params->in.bytes - bytes;
                    params->out.sequential = ptr + bytes;
                    err = fs_sequential(handle, params->out.sequential);
                }
                break;

            default:
                // Unrecognised reason code
                err = &err_bad_parms;
                break;
        }
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for interactive free space display.
*/
os_error *fsentry_free_handler(fsentry_free_params *params)
{
    os_error *err = NULL;
    const char *path;

    DEBUG_PRINTF(("FSEntry_Free %u", params->in.reason))

    // Disable the idle timeout
    idle_start();

    // Action depends on the reason code
    switch (params->in.reason)
    {
        case FSENTRY_FREE_NO_OP:
            // No operation
            break;

        case FSENTRY_FREE_GET_DEVICE_NAME:
            // Get device name
            DEBUG_PRINTF(("FSEntry_Free - get device name fs=%u, buffer=%p, id='%s'", params->in_get_device_name.number, params->in_get_device_name.buffer, params->in_get_device_name.id))
            err = util_disc_spec(params->in_get_device_name.id, &path);
            if (!err)
            {
                fs_pathname str;
                int spare;

                err = xosfscontrol_canonicalise_path(path, str, NULL, NULL, sizeof(str), &spare);
                if (!err && (spare < 1)) err = &err_bad_name;
                if (!err)
                {
                    char *ptr;

                    ptr = strchr(str, FS_CHAR_SEPARATOR);
                    if (ptr) *ptr = '\0';
                    ptr = strrchr(str, FS_CHAR_DISC);
                    if (!ptr) err = &err_bad_name;
                    else
                    {
                        strcpy(params->in_get_device_name.buffer, ptr + 1);
                        params->out_get_device_name.length = strlen(ptr);
                    }
                }
            }
            break;

        case FSENTRY_FREE_GET_FREE_SPACE:
            // Get free space for drive
            DEBUG_PRINTF(("FSEntry_Free - get free space fs=%u, buffer=%p, id='%s'", params->in_get_free_space.number, params->in_get_free_space.buffer, params->in_get_free_space.id))
            err = util_disc_spec(params->in_get_free_space.id, &path);
            if (!err)
            {
                int free;
                int size;

                err = xosfscontrol_free_space(path, &free, NULL, &size);
                if (!err)
                {
                    params->in_get_free_space.buffer[0] = size;
                    params->in_get_free_space.buffer[1] = free;
                    params->in_get_free_space.buffer[2] = size - free;
                }
            }
            break;

        case FSENTRY_FREE_COMPARE_DEVICE:
            // Compare device
            DEBUG_PRINTF(("FSEntry_Free - compare device fs=%u, path='%s', id='%s'", params->in_compare_device.number, params->in_compare_device.path, params->in_compare_device.id))
            err = util_disc_spec(params->in_compare_device.id, &path);
            if (!err)
            {
                path = strchr(path, FS_CHAR_DISC);
                if (!path || (path[1] != FS_CHAR_DISC)) err = &err_bad_name;
                else path++;
            }
            if (!err)
            {
                const char *ptr;

                ptr = strchr(path, FS_CHAR_SEPARATOR);
                if (!ptr) ptr = strchr(path, '\0');
                if (memcmp(params->in_compare_device.path, path, ptr - path))
                {
                    err = &err_not_found;
                }
            }
            params->out_compare_device.match = !err;
            err = NULL;
            break;

        default:
            // Unrecognised reason code
            err = &err_bad_parms;
            break;
    }

    // Re-enable the idle timeout
    idle_end();

    // Return any error produced
    DEBUG_ERR(err)
    return err;
}
