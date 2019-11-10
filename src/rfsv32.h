/*
    File        : rfsv32.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Remote file services client for the PsiFS module.

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
#ifndef RFSV32_H
#define RFSV32_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "epoc32.h"
#include "share.h"
#include "status.h"

// Supported operations
typedef bits rfsv32_op;
#define RFSV32_REQ_CLOSE_HANDLE ((rfsv32_op) 0x01)
#define RFSV32_REQ_OPEN_DIR ((rfsv32_op) 0x10)
#define RFSV32_REQ_READ_DIR ((rfsv32_op) 0x12)
#define RFSV32_REQ_GET_DRIVE_LIST ((rfsv32_op) 0x13)
#define RFSV32_REQ_VOLUME ((rfsv32_op) 0x14)
#define RFSV32_REQ_SET_VOLUME_LABEL ((rfsv32_op) 0x15)
#define RFSV32_REQ_OPEN_FILE ((rfsv32_op) 0x16)
#define RFSV32_REQ_TEMP_FILE ((rfsv32_op) 0x17)
#define RFSV32_REQ_READ_FILE ((rfsv32_op) 0x18)
#define RFSV32_REQ_WRITE_FILE ((rfsv32_op) 0x19)
#define RFSV32_REQ_SEEK_FILE ((rfsv32_op) 0x1a)
#define RFSV32_REQ_DELETE ((rfsv32_op) 0x1b)
#define RFSV32_REQ_REMOTE_ENTRY ((rfsv32_op) 0x1c)
#define RFSV32_REQ_FLUSH ((rfsv32_op) 0x1d)
#define RFSV32_REQ_SET_SIZE ((rfsv32_op) 0x1e)
#define RFSV32_REQ_RENAME ((rfsv32_op) 0x1f)
#define RFSV32_REQ_MK_DIR_ALL ((rfsv32_op) 0x20)
#define RFSV32_REQ_RM_DIR ((rfsv32_op) 0x21)
#define RFSV32_REQ_SET_ATT ((rfsv32_op) 0x22)
#define RFSV32_REQ_ATT ((rfsv32_op) 0x23)
#define RFSV32_REQ_SET_MODIFIED ((rfsv32_op) 0x24)
#define RFSV32_REQ_MODIFIED ((rfsv32_op) 0x25)
#define RFSV32_REQ_SET_SESSION_PATH ((rfsv32_op) 0x26)
#define RFSV32_REQ_SESSION_PATH ((rfsv32_op) 0x27)
#define RFSV32_REQ_READ_WRITE_FILE ((rfsv32_op) 0x28)
#define RFSV32_REQ_CREATE_FILE ((rfsv32_op) 0x29)
#define RFSV32_REQ_REPLACE_FILE ((rfsv32_op) 0x2a)
#define RFSV32_REQ_PATH_TEST ((rfsv32_op) 0x2b)
#define RFSV32_REQ_LOCK ((rfsv32_op) 0x2d)
#define RFSV32_REQ_UNLOCK ((rfsv32_op) 0x2e)
#define RFSV32_REQ_OPEN_DIR_UID ((rfsv32_op) 0x2f)
#define RFSV32_REQ_DRIVE_NAME ((rfsv32_op) 0x30)
#define RFSV32_REQ_SET_DRIVE_NAME ((rfsv32_op) 0x31)
#define RFSV32_REQ_REPLACE ((rfsv32_op) 0x32)

// Special operation for all responses
#define RFSV32_RESPONSE ((rfsv32_op) 0x11)

// Maximum transfer size for reading and writing
#define RFSV32_MAX_READ (2048)
#define RFSV32_MAX_WRITE (2048)

// Data types for operations
typedef struct
{
    rfsv32_op op;
    union
    {
        struct
        {
            epoc32_handle handle;
        } req_close_handle;
        struct
        {
            epoc32_file_attributes attributes;
            epoc32_path match;
        } req_open_dir;
        struct
        {
            epoc32_handle handle;
            epoc32_remote_entry *buffer;
            bits size;
        } req_read_dir;
        struct
        {
            bits drive;
        } req_volume;
        struct
        {
            bits drive;
            epoc32_disc_name name;
        } req_set_volume_label;
        struct
        {
            epoc32_mode mode;
            epoc32_path name;
        } req_open_file;
        struct
        {
            epoc32_mode mode;
            epoc32_path name;
        } req_temp_file;
        struct
        {
            epoc32_handle handle;
            bits length;
            void *buffer;
        } req_read_file;
        struct
        {
            epoc32_handle handle;
            bits length;
            const void *buffer;
        } req_write_file;
        struct
        {
            int offset;
            epoc32_handle handle;
            epoc32_sense sense;
        } req_seek_file;
        struct
        {
            epoc32_path name;
        } req_delete;
        struct
        {
            epoc32_path name;
        } req_remote_entry;
        struct
        {
            epoc32_handle handle;
        } req_flush;
        struct
        {
            bits size;
            epoc32_handle handle;
        } req_set_size;
        struct
        {
            epoc32_path src;
            epoc32_path dest;
        } req_rename;
        struct
        {
            epoc32_path name;
        } req_mk_dir_all;
        struct
        {
            epoc32_path name;
        } req_rm_dir;
        struct
        {
            epoc32_file_attributes set;
            epoc32_file_attributes clear;
            epoc32_path name;
        } req_set_att;
        struct
        {
            epoc32_path name;
        } req_att;
        struct
        {
            epoc32_file_time modified;
            epoc32_path name;
        } req_set_modified;
        struct
        {
            epoc32_path name;
        } req_modified;
        struct
        {
            epoc32_path name;
        } req_set_session_path;
        struct
        {
            bits length;
            epoc32_handle dest;
            epoc32_handle src;
        } req_read_write_file;
        struct
        {
            epoc32_mode mode;
            epoc32_path name;
        } req_create_file;
        struct
        {
            epoc32_mode mode;
            epoc32_path name;
        } req_replace_file;
        struct
        {
            epoc32_path name;
        } req_path_test;
        struct
        {
            bits length;
            bits offset;
            epoc32_handle handle;
        } req_lock;
        struct
        {
            bits length;
            bits offset;
            epoc32_handle handle;
        } req_unlock;
        struct
        {
            epoc32_file_uid uid;
            epoc32_path match;
        } req_open_dir_uid;
        struct
        {
            bits drive;
        } req_drive_name;
        struct
        {
            bits drive;
            epoc32_disc_name name;
        } req_set_drive_name;
        struct
        {
            epoc32_path src;
            epoc32_path dest;
        } req_replace;
    } data;
} rfsv32_cmd;
typedef union
{
    struct
    {
        epoc32_handle handle;
    } req_open_dir;
    struct
    {
        epoc32_remote_entry *next;
        bits used;
        bits remain;
    } req_read_dir;
    struct
    {
        epoc32_drives drives;
    } req_get_drive_list;
    struct
    {
        epoc32_volume_info volume;
    } req_volume;
    struct
    {
        epoc32_handle handle;
    } req_open_file;
    struct
    {
        epoc32_handle handle;
        epoc32_path name;
    } req_temp_file;
    struct
    {
        bits length;
    } req_read_file;
    struct
    {
        int offset;
    } req_seek_file;
    struct
    {
        epoc32_file_attributes attributes;
    } req_att;
    struct
    {
        epoc32_file_time modified;
    } req_modified;
    struct
    {
        epoc32_remote_entry entry;
    } req_remote_entry;
    struct
    {
        epoc32_disc_name name;
    } req_session_path;
    struct
    {
        bits length;
    } req_read_write_file;
    struct
    {
        epoc32_handle handle;
    } req_create_file;
    struct
    {
        epoc32_handle handle;
    } req_replace_file;
    struct
    {
        epoc32_handle handle;
    } req_open_dir_uid;
    struct
    {
        epoc32_disc_name name;
    } req_drive_name;
} rfsv32_reply;

// Shared access handler
extern share_handle rfsv32_share_handle;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for RFSV32 channel to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *rfsv32_fore(const rfsv32_cmd *cmd, rfsv32_reply *reply, bool escape);

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the specified operation when the RFSV channel becomes
                  idle. Control is returned immediately. If the channel is not
                  valid or the operation fails then no error is returned, but
                  instead the callback function is notified.
*/
os_error *rfsv32_back(const rfsv32_cmd *cmd, rfsv32_reply *reply,
                      void *user, share_callback callback);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a client channel for the remote file services.
*/
os_error *rfsv32_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the remote file services client channel.
*/
os_error *rfsv32_end(bool now);

#ifdef __cplusplus
    }
#endif

#endif
