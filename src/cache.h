/*
    File        : cache.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Cached remote file and directory access for the PsiFS
                  module.

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
#ifndef CACHE_H
#define CACHE_H

// Include project header files
#include "date.h"
#include "fs.h"
#include "share.h"
#include "unified.h"

// Available operations
typedef bits cache_op;
#define CACHE_DRIVE ((cache_op) 0x00)
#define CACHE_NAME ((cache_op) 0x01)
#define CACHE_ENUMERATE ((cache_op) 0x02)
#define CACHE_INFO ((cache_op) 0x03)
#define CACHE_MKDIR ((cache_op) 0x04)
#define CACHE_REMOVE ((cache_op) 0x05)
#define CACHE_RENAME ((cache_op) 0x06)
#define CACHE_ACCESS ((cache_op) 0x07)
#define CACHE_STAMP ((cache_op) 0x08)
#define CACHE_OPEN ((cache_op) 0x09)
#define CACHE_CLOSE ((cache_op) 0x0a)
#define CACHE_ARGS ((cache_op) 0x0b)
#define CACHE_READ ((cache_op) 0x0c)
#define CACHE_WRITE ((cache_op) 0x0d)
#define CACHE_ZERO ((cache_op) 0x0e)
#define CACHE_ALLOCATED ((cache_op) 0x0f)
#define CACHE_EXTENT ((cache_op) 0x10)
#define CACHE_FLUSH ((cache_op) 0x11)
#define CACHE_SEQUENTIAL ((cache_op) 0x12)

// Data types for operations
typedef struct
{
    cache_op op;
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
            fs_leafname match;
            int offset;
            fs_info *buffer;
            bits size;
        } enumerate;
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
            bits load;
            bits exec;
        } stamp;
        struct
        {
            fs_pathname path;
            fs_mode mode;
            os_fw handle;
        } open;
        struct
        {
            fs_handle handle;
        } close;
        struct
        {
            fs_handle handle;
        } args;
        struct
        {
            fs_handle handle;
            bits offset;
            bits length;
            void *buffer;
        } read;
        struct
        {
            fs_handle handle;
            bits offset;
            bits length;
            const void *buffer;
        } write;
        struct
        {
            fs_handle handle;
            bits offset;
            bits length;
        } zero;
        struct
        {
            fs_handle handle;
            bits size;
        } allocated;
        struct
        {
            fs_handle handle;
            bits size;
        } extent;
        struct
        {
            fs_handle handle;
        } flush;
        struct
        {
            fs_handle handle;
            bits offset;
        } sequential;
    } data;
} cache_cmd;
typedef union
{
    struct
    {
        fs_drive drive;
    } drive;
    struct
    {
        int offset;
        bits read;
    } enumerate;
    struct
    {
        fs_info info;
    } info;
    struct
    {
        fs_handle handle;
    } open;
    struct
    {
        fs_pathname path;
        fs_open_info info;
    } args;
} cache_reply;

#ifdef __cplusplus
    extern "C" {
#endif

// Counter of background operation disable
extern bits cache_disable;

// Should clocks be synchronized on connection
extern bool cache_sync;

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
os_error *cache_fore(const cache_cmd *cmd, cache_reply *reply, bool escape);

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
os_error *cache_back(const cache_cmd *cmd, cache_reply *reply,
                     void *user, share_callback callback);

/*
    Parameters  : drive         - The drive to read the details of.
                  status        - Variable to receive the status of the drive,
                                  or NULL if the value is not required.
                  name          - Variable to receive a pointer to the drive
                                  name, or NULL if the value is not required.
                  id            - Variable to receive the unique identifier of
                                  the drive, or NULL if the value is not
                                  required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the status for the specified drive.
*/
os_error *cache_drive_status(char drive, psifs_drive_status *status,
                             const char **name, psifs_drive_id *id);

/*
    Parameters  : type          - Variable to receive the machine type, or NULL
                                  if the value is not required.
                  name          - Variable to receive a pointer to the machine
                                  description, or NULL if the value is not
                                  required.
                  id            - Variable to receive the unique identifier, or
                                  NULL if the value is not required.
                  language      - Variable to receive the language, or NULL if
                                  the value is not required.
                  version       - Variable to receive the operating system
                                  version, or NULL if the value is not required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the machine details.
*/
os_error *cache_machine_status(psifs_machine_type *type, const char **name,
                               unified_machine_id *id, psifs_language *language,
                               unified_version *version);

/*
    Parameters  : owner         - Variable to receive a pointer to the owner
                                  information, or NULL if the value is not
                                  required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the owner information.
*/
os_error *cache_owner_status(const char **owner);

/*
    Parameters  : main          - Variable to receive the status of the main
                                  battery, or NULL if the value is not required.
                  backup        - Variable to receive the status of the backup
                                  battery, or NULL if the value is not required.
                  external      - Variable to receive the status of external
                                  power, or NULL if the value is not required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the power status.
*/
os_error *cache_power_status(unified_battery *main, unified_battery *backup,
                             bool *external);

/*
    Parameters  : path          - The path of the object to invalidate.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Re-cache the details for the specified object.
*/
os_error *cache_recache(const char *path);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required.
*/
os_error *cache_poll(void);

/*
    Parameters  : era           - Is an ERA device connected.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the cache layer after the unified server layer has been
                  started.
*/
os_error *cache_start(bool era);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the cache layer before the unified server layer has been
                  closed.
*/
os_error *cache_end(bool now);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the cache layer.
*/
os_error *cache_status(void);

#ifdef __cplusplus
    }
#endif

#endif
