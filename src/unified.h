/*
    File        : unified.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Unified remote file access for the PsiFS module.

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
#ifndef UNIFIED_H
#define UNIFIED_H

// Include project header files
#include "date.h"
#include "epoc.h"
#include "fs.h"
#include "ncp.h"
#include "share.h"

// Available operations
typedef bits unified_op;
#define UNIFIED_DRIVE ((unified_op) 0x00)
#define UNIFIED_NAME ((unified_op) 0x01)
#define UNIFIED_LIST ((unified_op) 0x02)
#define UNIFIED_INFO ((unified_op) 0x03)
#define UNIFIED_MKDIR ((unified_op) 0x04)
#define UNIFIED_REMOVE ((unified_op) 0x05)
#define UNIFIED_RMDIR ((unified_op) 0x06)
#define UNIFIED_RENAME ((unified_op) 0x07)
#define UNIFIED_ACCESS ((unified_op) 0x08)
#define UNIFIED_STAMP ((unified_op) 0x09)
#define UNIFIED_OPEN ((unified_op) 0x0a)
#define UNIFIED_CLOSE ((unified_op) 0x0b)
#define UNIFIED_SEEK ((unified_op) 0x0c)
#define UNIFIED_READ ((unified_op) 0x0d)
#define UNIFIED_WRITE ((unified_op) 0x0e)
#define UNIFIED_ZERO ((unified_op) 0x0f)
#define UNIFIED_SIZE ((unified_op) 0x10)
#define UNIFIED_FLUSH ((unified_op) 0x11)
#define UNIFIED_MACHINE ((unified_op) 0x12)
#define UNIFIED_TASKS ((unified_op) 0x13)
#define UNIFIED_DETAIL ((unified_op) 0x14)
#define UNIFIED_STOP ((unified_op) 0x15)
#define UNIFIED_START ((unified_op) 0x16)
#define UNIFIED_POWER ((unified_op) 0x17)
#define UNIFIED_RTIME ((unified_op) 0x18)
#define UNIFIED_WTIME ((unified_op) 0x19)
#define UNIFIED_OWNER ((unified_op) 0x1a)

// A unified file handle
typedef bits unified_handle;

// A uniied battery status
typedef struct
{
    psifs_battery_status status;
    bits mv;
    bits mv_max;
} unified_battery;

// An open application
typedef struct
{
    fs_pathname name;
    fs_pathname args;
} unified_program;

// Software version
typedef struct
{
    bits major;
    bits minor;
    bits build;
} unified_version;

// Owner information
typedef ncp_string unified_owner;

// Possible actions when starting a task
typedef char unified_start_action;
#define UNIFIED_START_DEFAULT ('\0')
#define UNIFIED_START_CREATE ('C')
#define UNIFIED_START_OPEN ('O')
#define UNIFIED_START_RUN ('R')

// A textual machine type
#define UNIFIED_MAX_MACHINE_TYPE (32)
typedef char unified_machine_type[UNIFIED_MAX_MACHINE_TYPE + 1];

// A machine unique identifier
typedef struct
{
    bits low;
    bits high;
} unified_machine_id;

// Data types for operations
typedef struct
{
    unified_op op;
    union
    {
        struct
        {
            char drive;
        } drive;
        struct
        {
            char drive;
            fs_discname name;
        } name;
        struct
        {
            fs_pathname path;
            fs_info *buffer;
            bits size;
        } list;
        struct
        {
            fs_pathname path;
        } info;
        struct
        {
            fs_pathname path;
        } mkdir;
        struct
        {
            fs_pathname path;
        } remove;
        struct
        {
            fs_pathname path;
        } rmdir;
        struct
        {
            fs_pathname src;
            fs_pathname dest;
        } rename;
        struct
        {
            fs_pathname path;
            fileswitch_attr attr;
        } access;
        struct
        {
            fs_pathname path;
            date_riscos date;
        } stamp;
        struct
        {
            fs_pathname path;
            fs_mode mode;
        } open;
        struct
        {
            unified_handle handle;
        } close;
        struct
        {
            unified_handle handle;
            bits offset;
        } seek;
        struct
        {
            unified_handle handle;
            bits length;
            void *buffer;
        } read;
        struct
        {
            unified_handle handle;
            bits length;
            const void *buffer;
        } write;
        struct
        {
            unified_handle handle;
            bits length;
        } zero;
        struct
        {
            unified_handle handle;
            bits size;
        } size;
        struct
        {
            unified_handle handle;
        } flush;
        struct
        {
            ncp_app *buffer;
            bits size;
        } tasks;
        struct
        {
            epoc_process name;
        } detail;
        struct
        {
            epoc_process name;
        } stop;
        struct
        {
            epoc_program name;
            epoc_args args;
            unified_start_action action;
        } start;
        struct
        {
            date_riscos date;
        } wtime;
    } data;
} unified_cmd;
typedef union
{
    struct
    {
        fs_drive drive;
    } drive;
    struct
    {
        fs_info *next;
        bits used;
        bits remain;
    } list;
    struct
    {
        fs_info info;
    } info;
    struct
    {
        unified_handle handle;
    } open;
    struct
    {
        bits length;
    } read;
    struct
    {
        psifs_machine_type type;
        unified_machine_type name;
        unified_machine_id id;
        psifs_language language;
        unified_version version;
    } machine;
    struct
    {
        ncp_app *next;
        bits used;
        bits remain;
    } tasks;
    struct
    {
        epoc_program name;
        epoc_args args;
    } detail;
    struct
    {
        unified_battery main;
        unified_battery backup;
        bool external;
    } power;
    struct
    {
        date_riscos date;
    } rtime;
    struct
    {
        unified_owner info;
    } owner;
} unified_reply;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - The size of the destination buffer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Validate the specified file or directory name for the
                  connected device type. The source and destination strings may
                  be the same to perform an in-place conversion.
*/
os_error *unified_validate(const char *src, char *dest, size_t size);

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for the link to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *unified_fore(const unified_cmd *cmd, unified_reply *reply,
                       bool escape);

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  user          - User defined handle for this operation.
                  callback      - Callback function to call when the operation
                                  has completed (both for success and failure).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform the specified operation when the link becomes idle.
                  Control is returned immediately. If the link is not valid or
                  the operation fails then no error is returned, but instead
                  the callback function is notified.
*/
os_error *unified_back(const unified_cmd *cmd, unified_reply *reply,
                       void *user, share_callback callback);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required.
*/
os_error *unified_poll(void);

/*
    Parameters  : era           - Is an ERA device connected.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the unified server layer after the multiplexor layer
                  has been started.
*/
os_error *unified_start(bool era);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the unified server layer before the multiplexor layer has
                  been closed.
*/
os_error *unified_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the unified server layer.
*/
os_error *unified_status(void);

#ifdef __cplusplus
    }
#endif

#endif
