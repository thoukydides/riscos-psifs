/*
    File        : fsentry.h
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

// Only include header file once
#ifndef FSENTRY_H
#define FSENTRY_H

// Include oslib header files
#include "oslib/fileswitch.h"
#include "oslib/os.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "fs.h"

// A padding word
#define FSENTRY_PAD bits : 32;

// Parameters for fsentry_open_handler
typedef bits fsentry_open_reason;
#define FSENTRY_OPEN_OPEN_READ ((fsentry_open_reason) 0)
#define FSENTRY_OPEN_CREATE_UPDATE ((fsentry_open_reason) 1)
#define FSENTRY_OPEN_UPDATE ((fsentry_open_reason) 2)
typedef union
{
    struct
    {
        fsentry_open_reason reason;
        const char *path;
        FSENTRY_PAD
        os_fw handle;
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in;
    struct
    {
        fs_file_info_word information_word;
        fs_handle handle;
        bits buffer_size;
        bits extent;
        bits allocated;
    } out;
} fsentry_open_params;

// Parameters for fsentry_getbytes_handler
typedef union
{
    struct
    {
        FSENTRY_PAD
        fs_handle handle;
        byte *buffer;
        bits bytes;
        bits offset;
    } in;
    struct
    {
        byte value;
        bool success;
    } out;
} fsentry_getbytes_params;

// Parameters for fsentry_putbytes_handler
typedef union
{
    struct
    {
        byte value;
        fs_handle handle;
        const byte *buffer;
        bits bytes;
        bits offset;
    } in;
} fsentry_putbytes_params;

// Parameters for fsentry_args_handler
typedef bits fsentry_args_reason;
#define FSENTRY_ARGS_READ_SEQUENTIAL ((fsentry_args_reason) 0)
#define FSENTRY_ARGS_WRITE_SEQUENTIAL ((fsentry_args_reason) 1)
#define FSENTRY_ARGS_READ_EXTENT ((fsentry_args_reason) 2)
#define FSENTRY_ARGS_WRITE_EXTENT ((fsentry_args_reason) 3)
#define FSENTRY_ARGS_READ_ALLOCATED ((fsentry_args_reason) 4)
#define FSENTRY_ARGS_EOF_CHECK ((fsentry_args_reason) 5)
#define FSENTRY_ARGS_FLUSH ((fsentry_args_reason) 6)
#define FSENTRY_ARGS_ENSURE ((fsentry_args_reason) 7)
#define FSENTRY_ARGS_ZERO ((fsentry_args_reason) 8)
#define FSENTRY_ARGS_READ_STAMP ((fsentry_args_reason) 9)
#define FSENTRY_ARGS_IMAGE_STAMP ((fsentry_args_reason) 10)
typedef union
{
    struct
    {
        fsentry_args_reason reason;
        fs_handle handle;
    } in;
    struct
    {
        fsentry_args_reason reason;
        fs_handle handle;
        bits sequential;
    } in_write_sequential;
    struct
    {
        fsentry_args_reason reason;
        fs_handle handle;
        bits extent;
    } in_write_extent;
    struct
    {
        fsentry_args_reason reason;
        fs_handle handle;
        bits extent;
    } in_ensure;
    struct
    {
        fsentry_args_reason reason;
        fs_handle handle;
        bits offset;
        bits bytes;
    } in_zero;
    struct
    {
        fsentry_args_reason reason;
        fs_handle handle;
        bits stamp;
    } in_image_stamp;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits sequential;
    } out_read_sequential;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits extent;
    } out_read_extent;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits allocated;
    } out_read_allocated;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        fs_is_eof eof;
    } out_eof_check;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits load_addr;
        bits exec_addr;
    } out_flush;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits extent;
    } out_ensure;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits load_addr;
        bits exec_addr;
    } out_read_stamp;
} fsentry_args_params;

// Parameters for fsentry_close_handler
typedef union
{
    struct
    {
        FSENTRY_PAD
        fs_handle handle;
        bits load_addr;
        bits exec_addr;
    } in;
} fsentry_close_params;

// Parameters for fs_handler
typedef bits fsentry_file_reason;
#define FSENTRY_FILE_SAVE ((fsentry_file_reason) 0)
#define FSENTRY_FILE_WRITE_CAT ((fsentry_file_reason) 1)
#define FSENTRY_FILE_WRITE_LOAD ((fsentry_file_reason) 2)
#define FSENTRY_FILE_WRITE_EXEC ((fsentry_file_reason) 3)
#define FSENTRY_FILE_WRITE_ATTR ((fsentry_file_reason) 4)
#define FSENTRY_FILE_READ_CAT ((fsentry_file_reason) 5)
#define FSENTRY_FILE_DELETE ((fsentry_file_reason) 6)
#define FSENTRY_FILE_CREATE_FILE ((fsentry_file_reason) 7)
#define FSENTRY_FILE_CREATE_DIR ((fsentry_file_reason) 8)
#define FSENTRY_FILE_READ_CAT_NOT_LENGTH ((fsentry_file_reason) 9)
#define FSENTRY_FILE_READ_BLOCK_SIZE ((fsentry_file_reason) 10)
#define FSENTRY_FILE_LOAD ((fsentry_file_reason) 255)
typedef union
{
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits load_addr;
        bits exec_addr;
        const byte *buffer;
        const byte *buffer_end;
        const char *special_field;
    } in_save;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits load_addr;
        bits exec_addr;
        FSENTRY_PAD
        fileswitch_attr attr;
        const char *special_field;
    } in_write_cat;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits load_addr;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_write_load;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        bits exec_addr;
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_write_exec;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        fileswitch_attr attr;
        const char *special_field;
    } in_write_attr;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits load_addr;
        bits exec_addr;
        byte *buffer;
        byte *buffer_end;
        const char *special_field;
    } in_create_file;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits load_addr;
        bits exec_addr;
        bits entries;
        FSENTRY_PAD
        const char *special_field;
    } in_create_dir;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits buffer;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_load;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *leaf;
    } out_save;
    struct
    {
        fileswitch_object_type type;
        FSENTRY_PAD
        bits load_addr;
        bits exec_addr;
        bits length;
        fileswitch_attr attr;
    } out_read_cat;
    struct
    {
        fileswitch_object_type type;
        FSENTRY_PAD
        bits load_addr;
        bits exec_addr;
        bits length;
        fileswitch_attr attr;
    } out_delete;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *leaf;
    } out_create_file;
    struct
    {
        fileswitch_object_type type;
        FSENTRY_PAD
        bits load_addr;
        bits exec_addr;
        FSENTRY_PAD
        fileswitch_attr attr;
    } out_read_cat_no_length;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits block_size;
    } out_read_block_size;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits load_addr;
        bits exec_addr;
        bits length;
        fileswitch_attr attr;
        const char *leaf;
    } out_load;
} fsentry_file_params;

// Parameters for fsentry_func_handler
typedef bits fsentry_func_reason;
#define FSENTRY_FUNC_SET_CSD ((fsentry_func_reason) 0)
#define FSENTRY_FUNC_SET_LIB ((fsentry_func_reason) 1)
#define FSENTRY_FUNC_CAT_CSD ((fsentry_func_reason) 2)
#define FSENTRY_FUNC_EX_CSD ((fsentry_func_reason) 3)
#define FSENTRY_FUNC_CAT_LIB ((fsentry_func_reason) 4)
#define FSENTRY_FUNC_EX_LIB ((fsentry_func_reason) 5)
#define FSENTRY_FUNC_INFO ((fsentry_func_reason) 6)
#define FSENTRY_FUNC_OPT ((fsentry_func_reason) 7)
#define FSENTRY_FUNC_RENAME ((fsentry_func_reason) 8)
#define FSENTRY_FUNC_ACCESS ((fsentry_func_reason) 9)
#define FSENTRY_FUNC_BOOT ((fsentry_func_reason) 10)
#define FSENTRY_FUNC_READ_BOOT_NAME ((fsentry_func_reason) 11)
#define FSENTRY_FUNC_READ_CSD ((fsentry_func_reason) 12)
#define FSENTRY_FUNC_READ_LIB ((fsentry_func_reason) 13)
#define FSENTRY_FUNC_READ_DIR ((fsentry_func_reason) 14)
#define FSENTRY_FUNC_READ_DIR_INFO ((fsentry_func_reason) 15)
#define FSENTRY_FUNC_SHUTDOWN ((fsentry_func_reason) 16)
#define FSENTRY_FUNC_BANNER ((fsentry_func_reason) 17)
#define FSENTRY_FUNC_READ_DIR_INFO_INTERNAL ((fsentry_func_reason) 19)
#define FSENTRY_FUNC_FILE_INFO ((fsentry_func_reason) 20)
#define FSENTRY_FUNC_CANONICALISE ((fsentry_func_reason) 23)
#define FSENTRY_FUNC_RESOLVE_WILDCARD ((fsentry_func_reason) 24)
#define FSENTRY_FUNC_READ_DEFECTS ((fsentry_func_reason) 25)
#define FSENTRY_FUNC_ADD_DEFECT ((fsentry_func_reason) 26)
#define FSENTRY_FUNC_READ_BOOT ((fsentry_func_reason) 27)
#define FSENTRY_FUNC_WRITE_BOOT ((fsentry_func_reason) 28)
#define FSENTRY_FUNC_READ_MAP ((fsentry_func_reason) 29)
#define FSENTRY_FUNC_READ_FREE ((fsentry_func_reason) 30)
#define FSENTRY_FUNC_NAME_IMAGE ((fsentry_func_reason) 31)
#define FSENTRY_FUNC_STAMP_IMAGE ((fsentry_func_reason) 32)
#define FSENTRY_FUNC_GET_USAGE ((fsentry_func_reason) 33)
#define FSENTRY_FUNC_CHANGED_DIR ((fsentry_func_reason) 34)
typedef union
{
    struct
    {
        fsentry_func_reason reason;
    } in;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_set_csd;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_set_lib;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_cat_csd;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_ex_csd;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_cat_lib;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_ex_lib;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_info;
    struct
    {
        fsentry_file_reason reason;
        bits option;
        bits parameter;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_opt;
    struct
    {
        fsentry_file_reason reason;
        const char *source;
        const char *dest;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *source_special_field;
        const char *dest_special_field;
    } in_rename;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        const char *access;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_access;
    struct
    {
        fsentry_file_reason reason;
        FSENTRY_PAD
        byte *buffer;
    } in_read_boot_name;
    struct
    {
        fsentry_file_reason reason;
        FSENTRY_PAD
        byte *buffer;
    } in_read_csd;
    struct
    {
        fsentry_file_reason reason;
        FSENTRY_PAD
        byte *buffer;
    } in_read_lib;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        byte *buffer;
        bits number;
        int offset;
        bits buffer_size;
        const char *special_field;
    } in_read_dir;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        byte *buffer;
        bits number;
        int offset;
        bits buffer_size;
        const char *special_field;
    } in_read_dir_info;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        byte *buffer;
        bits number;
        int offset;
        bits buffer_size;
        const char *special_field;
    } in_read_dir_info_internal;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_file_info;
    struct
    {
        fsentry_file_reason reason;
        const char *special_field;
        const char *disc_name;
        char *buffer_special_field;
        char *buffer_disc_name;
        bits buffer_special_field_size;
        bits buffer_disc_name_size;
    } in_canonicalise;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        char *buffer_name;
        const char *name;
        FSENTRY_PAD
        bits buffer_name_size;
        const char *special_field;
    } in_resolve_wildcard;
    struct
    {
        fsentry_file_reason reason;
        const char *image;
        bits *buffer;
        FSENTRY_PAD
        FSENTRY_PAD
        bits buffer_size;
        const char *special_field;
    } in_read_defects;
    struct
    {
        fsentry_file_reason reason;
        const char *image;
        bits offset;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_add_defect;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_read_boot;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits option;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_write_boot;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        byte *buffer;
        FSENTRY_PAD
        FSENTRY_PAD
        bits buffer_size;
        const char *special_field;
    } in_read_map;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_read_free;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        const char *name;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_name_image;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits reason_code;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_stamp_image;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits offset;
        char *buffer;
        bits buffer_size;
        FSENTRY_PAD
        const char *special_field;
    } in_get_usage;
    struct
    {
        fsentry_file_reason reason;
        const char *path;
        bits type;
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_changed_dir;
    struct
    {
        FSENTRY_PAD
        bool invalid;
    } out_rename;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        bits number;
        int offset;
    } out_read_dir;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        bits number;
        int offset;
    } out_read_dir_info;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        bits number;
        int offset;
    } out_read_dir_info_internal;
    struct
    {
        FSENTRY_PAD
        char *special_field;
        char *disc_name;
        bits special_field_overflow;
        bits disc_name_overflow;
    } out_canonicalise;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        int found;
        FSENTRY_PAD
        int overflow;
    } out_resolve_wildcard;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        bits option;
    } out_read_boot;
    struct
    {
        bits free;
        bits biggest;
        bits size;
    } out_read_free;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        osfscontrol_object_kind kind;
    } out_get_usage;
} fsentry_func_params;

// Parameters for fsentry_gbpb_handler
typedef bits fsentry_gbpb_reason;
#define FSENTRY_GBPB_PUT_HERE ((fsentry_gbpb_reason) 1)
#define FSENTRY_GBPB_PUT ((fsentry_gbpb_reason) 2)
#define FSENTRY_GBPB_GET_HERE ((fsentry_gbpb_reason) 3)
#define FSENTRY_GBPB_GET ((fsentry_gbpb_reason) 4)
typedef union
{
    struct
    {
        fsentry_gbpb_reason reason;
        fs_handle handle;
        byte *buffer;
        bits bytes;
        bits sequential;
    } in;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        byte *buffer_end;
        bits bytes_left;
        bits sequential;
    } out;
} fsentry_gbpb_params;

// Parameters for the interactive free space display
typedef bits fsentry_free_reason;
#define FSENTRY_FREE_NO_OP ((fsentry_free_reason) 0)
#define FSENTRY_FREE_GET_DEVICE_NAME ((fsentry_free_reason) 1)
#define FSENTRY_FREE_GET_FREE_SPACE ((fsentry_free_reason) 2)
#define FSENTRY_FREE_COMPARE_DEVICE ((fsentry_free_reason) 3)
typedef union
{
    struct
    {
        fsentry_free_reason reason;
    } in;
    struct
    {
        fsentry_free_reason reason;
        fileswitch_fs_no number;
        char *buffer;
        const char *id;
    } in_get_device_name;
    struct
    {
        fsentry_free_reason reason;
        fileswitch_fs_no number;
        bits *buffer;
        const char *id;
    } in_get_free_space;
    struct
    {
        fsentry_free_reason reason;
        fileswitch_fs_no number;
        const char *path;
        const char *id;
        FSENTRY_PAD
        FSENTRY_PAD
        const char *special_field;
    } in_compare_device;
    struct
    {
        bits length;
    } out_get_device_name;
    struct
    {
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        FSENTRY_PAD
        bool match;
    } out_compare_device;
} fsentry_free_params;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_Open.
*/
os_error *fsentry_open_handler(fsentry_open_params *params);

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_GetBytes.
*/
os_error *fsentry_getbytes_handler(fsentry_getbytes_params *params);

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_PutBytes.
*/
os_error *fsentry_putbytes_handler(fsentry_putbytes_params *params);

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_Args.
*/
os_error *fsentry_args_handler(fsentry_args_params *params);

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_Close.
*/
os_error *fsentry_close_handler(fsentry_close_params *params);

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_File.
*/
os_error *fsentry_file_handler(fsentry_file_params *params);

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_Func.
*/
os_error *fsentry_func_handler(fsentry_func_params *params);

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for FSEntry_GBPB.
*/
os_error *fsentry_gbpb_handler(fsentry_gbpb_params *params);

/*
    Parameters  : params        - The input and output registers for this
                                  filing system entry point.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Filing system entry point for interactive free space display.
*/
os_error *fsentry_free_handler(fsentry_free_params *params);

#ifdef __cplusplus
    }
#endif

#endif
