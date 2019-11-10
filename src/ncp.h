/*
    File        : ncp.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Remote command services client for the PsiFS module.

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
#ifndef NCP_H
#define NCP_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "epoc.h"
#include "share.h"
#include "status.h"

// Supported operations
typedef bits ncp_op;
#define NCP_QUERY_SUPPORT ((ncp_op) 0x00)
#define NCP_EXEC_PROGRAM ((ncp_op) 0x01)
#define NCP_QUERY_DRIVE ((ncp_op) 0x02)
#define NCP_STOP_PROGRAM ((ncp_op) 0x03)
#define NCP_PROG_RUNNING ((ncp_op) 0x04)
#define NCP_FORMAT_OPEN ((ncp_op) 0x05)
#define NCP_FORMAT_READ ((ncp_op) 0x06)
#define NCP_GET_UNIQUE_ID ((ncp_op) 0x07)
#define NCP_GET_OWNER_INFO ((ncp_op) 0x08)
#define NCP_GET_MACHINE_TYPE ((ncp_op) 0x09)
#define NCP_GET_CMD_LINE ((ncp_op) 0x0a)
#define NCP_STOP_FILE ((ncp_op) 0x0b)
#define NCP_GET_MACHINE_INFO ((ncp_op) 0x64)
#define NCP_CLOSE_HANDLE ((ncp_op) 0x65)
// #define NCP_REG_OPEN_ITER ((ncp_op) 0x66)
// #define NCP_REG_READ_ITER ((ncp_op) 0x67)
// #define NCP_REG_WRITE ((ncp_op) 0x68)
// #define NCP_REG_READ ((ncp_op) 0x69)
// #define NCP_REG_DELETE ((ncp_op) 0x6a)
#define NCP_SET_TIME ((ncp_op) 0x6b)
// #define NCP_CONFIG_OPEN ((ncp_op) 0x6c)
// #define NCP_CONFIG_READ ((ncp_op) 0x6d)
// #define NCP_CONFIG_WRITE ((ncp_op) 0x6e)
#define NCP_QUERY_OPEN ((ncp_op) 0x6f)
#define NCP_QUERY_READ ((ncp_op) 0x70)
#define NCP_QUIT_SERVER ((ncp_op) 0xff)

// Supported versions
#define NCP_MAJOR_VER_MIN (1)
#define NCP_MAJOR_VER_MAX (1)
#define NCP_MINOR_VER_MIN (1)
#define NCP_MINOR_VER_MAX (30)

// A general string buffer
#define NCP_MAX_STRING (2047)
typedef char ncp_string[NCP_MAX_STRING + 1];

// A list of process/argument pairs
typedef struct
{
    epoc_process name;
    epoc_args args;
} ncp_app;

// Data types for operations
typedef struct
{
    ncp_op op;
    union
    {
        struct
        {
            bits major;
            bits minor;
        } query_support;
        struct
        {
            epoc_program name;
            epoc_args args;
        } exec_program;
        struct
        {
            char drive;
            ncp_app *buffer;
            bits size;
        } query_drive;
        struct
        {
            epoc_process name;
        } stop_program;
        struct
        {
            epoc_program name;
        } prog_running;
        struct
        {
            epoc_path name;
        } format_open;
        struct
        {
            epoc_format_handle handle;
        } format_read;
        struct
        {
            epoc_path name;
        } get_unique_id;
        struct
        {
            epoc_process name;
        } get_cmd_line;
        struct
        {
            epoc_path name;
        } stop_file;
        struct
        {
            epoc32_resource_handle handle;
        } close_handle;
        /*
        struct
        {
        } reg_open_iter;
        struct
        {
        } reg_read_iter;
        struct
        {
        } reg_write;
        struct
        {
        } reg_read;
        struct
        {
        } reg_delete;
        */
        struct
        {
            epoc32_time time;
        } set_time;
        /*
        struct
        {
        } config_open;
        struct
        {
        } config_read;
        struct
        {
        } config_write;
        */
        struct
        {
            char drive;
        } query_open;
        struct
        {
            epoc32_resource_handle handle;
            ncp_app *buffer;
            bits size;
        } query_read;
    } data;
} ncp_cmd;
typedef union
{
    struct
    {
        bits major;
        bits minor;
    } query_support;
    struct
    {
        ncp_app *next;
        bits used;
        bits remain;
    } query_drive;
    struct
    {
        bool running;
    } prog_running;
    struct
    {
        epoc_format_handle handle;
        bits count;
    } format_open;
    struct
    {
        epoc_media_id id;
    } get_unique_id;
    struct
    {
        ncp_string info;
    } get_owner_info;
    struct
    {
        psifs_machine_type type;
    } get_machine_type;
    struct
    {
        epoc_program name;
        epoc_args args;
    } get_cmd_line;
    struct
    {
        epoc_program name;
    } stop_file;
    struct
    {
        epoc32_machine_info info;
    } get_machine_info;
    /*
    struct
    {
    } reg_open_iter;
    struct
    {
    } reg_read_iter;
    struct
    {
    } reg_write;
    struct
    {
    } reg_read;
    struct
    {
    } reg_delete;
    struct
    {
    } config_open;
    struct
    {
    } config_read;
    struct
    {
    } config_write;
    */
    struct
    {
        epoc32_resource_handle handle;
    } query_open;
    struct
    {
        ncp_app *next;
        bits used;
        bits remain;
    } query_read;
} ncp_reply;

// Shared access handler
extern share_handle ncp_share_handle;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the NCP channel to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *ncp_fore(const ncp_cmd *cmd, ncp_reply *reply, bool escape);

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the specified operation when the NCP channel becomes
                  idle. Control is returned immediately. If the channel is not
                  valid or the operation fails then no error is returned, but
                  instead the callback function is notified.
*/
os_error *ncp_back(const ncp_cmd *cmd, ncp_reply *reply,
                   void *user, share_callback callback);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a client channel for the remote file services.
*/
os_error *ncp_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the remote file services client channel.
*/
os_error *ncp_end(bool now);

#ifdef __cplusplus
    }
#endif

#endif
