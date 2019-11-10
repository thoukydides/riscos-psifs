/*
    File        : rfsv16.h
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
#ifndef RFSV16_H
#define RFSV16_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "epoc16.h"
#include "share.h"
#include "status.h"

// Supported operations
typedef bits rfsv16_op;
#define RFSV16_RF_FOPEN ((rfsv16_op) 0x00)
#define RFSV16_RF_FCLOSE ((rfsv16_op) 0x02)
#define RFSV16_RF_FREAD ((rfsv16_op) 0x04)
#define RFSV16_RF_FDIRREAD ((rfsv16_op) 0x06)
#define RFSV16_RF_FDEVICEREAD ((rfsv16_op) 0x08)
#define RFSV16_RF_FWRITE ((rfsv16_op) 0x0a)
#define RFSV16_RF_FSEEK ((rfsv16_op) 0x0c)
#define RFSV16_RF_FFLUSH ((rfsv16_op) 0x0e)
#define RFSV16_RF_FSETEOF ((rfsv16_op) 0x10)
#define RFSV16_RF_RENAME ((rfsv16_op) 0x12)
#define RFSV16_RF_DELETE ((rfsv16_op) 0x14)
#define RFSV16_RF_FINFO ((rfsv16_op) 0x16)
#define RFSV16_RF_SFSTAT ((rfsv16_op) 0x18)
#define RFSV16_RF_PARSE ((rfsv16_op) 0x1a)
#define RFSV16_RF_MKDIR ((rfsv16_op) 0x1c)
#define RFSV16_RF_OPENUNIQUE ((rfsv16_op) 0x1e)
#define RFSV16_RF_STATUSDEVICE ((rfsv16_op) 0x20)
#define RFSV16_RF_PATHTEST ((rfsv16_op) 0x22)
#define RFSV16_RF_STATUSSYSTEM ((rfsv16_op) 0x24)
#define RFSV16_RF_CHANGEDIR ((rfsv16_op) 0x26)
#define RFSV16_RF_SFDATE ((rfsv16_op) 0x28)

// Special operation for all responses
#define RFSV16_RESPONSE ((rfsv16_op) 0x2a)

// Maximum transfer size for reading and writing
#define RFSV16_MAX_READ (640)
#define RFSV16_MAX_WRITE (640)

// Data types for operations
typedef struct
{
    rfsv16_op op;
    union
    {
        struct
        {
            epoc16_mode mode;
            epoc16_path name;
        } rf_fopen;
        struct
        {
            epoc16_handle handle;
        } rf_fclose;
        struct
        {
            epoc16_handle handle;
            bits length;
            void *buffer;
        } rf_fread;
        struct
        {
            epoc16_handle handle;
            epoc16_p_info *buffer;
            bits size;
        } rf_fdirread;
        struct
        {
            epoc16_handle handle;
        } rf_fdeviceread;
        struct
        {
            epoc16_handle handle;
            bits length;
            const void *buffer;
        } rf_fwrite;
        struct
        {

            epoc16_handle handle;
            int offset;
            epoc16_sense sense;
        } rf_fseek;
        struct
        {
            epoc16_handle handle;
        } rf_fflush;
        struct
        {
            epoc16_handle handle;
            bits size;
        } rf_fseteof;
        struct
        {
            epoc16_path src;
            epoc16_path dest;
        } rf_rename;
        struct
        {
            epoc16_path name;
        } rf_delete;
        struct
        {
            epoc16_path name;
        } rf_finfo;
        struct
        {
            epoc16_file_attributes set;
            epoc16_file_attributes mask;
            epoc16_path name;
        } rf_sfstat;
        struct
        {
            epoc16_path name;
        } rf_parse;
        struct
        {
            epoc16_path name;
        } rf_mkdir;
        struct
        {
            epoc16_mode mode;
            epoc16_path name;
        } rf_openunique;
        struct
        {
            epoc16_path name;
        } rf_statusdevice;
        struct
        {
            epoc16_path name;
        } rf_pathtest;
        struct
        {
            epoc16_path name;
        } rf_statussystem;
        struct
        {
            epoc16_path name;
        } rf_changedir;
        struct
        {
            epoc16_file_time modified;
            epoc16_path name;
        } rf_sfdate;
    } data;
} rfsv16_cmd;
typedef union
{
    struct
    {
        epoc16_handle handle;
    } rf_fopen;
    struct
    {
        bits length;
    } rf_fread;
    struct
    {
        epoc16_p_info *next;
        bits used;
        bits remain;
    } rf_fdirread;
    struct
    {
        epoc16_p_dinfo dinfo;
    } rf_fdeviceread;
    struct
    {
        bits offset;
    } rf_fseek;
    struct
    {
        epoc16_p_info entry;
    } rf_finfo;
    struct
    {
        epoc16_path name;
    } rf_parse;
    struct
    {
        epoc16_handle handle;
        epoc16_path name;
    } rf_openunique;
    struct
    {
        epoc16_p_dinfo dinfo;
    } rf_statusdevice;
    struct
    {
        bits version;
        bits type;
        bool formattable;
    } rf_statussystem;
} rfsv16_reply;

// Shared access handler
extern share_handle rfsv16_share_handle;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : cmd           - The data for the command to perform.
                  reply         - Pointer to block to receive response data.
                  escape        - Should escape conditions be checked for.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for RFSV16 channel to become idle and then perform the
                  specified operation. Control is only returned when the
                  operation has completed.
*/
os_error *rfsv16_fore(const rfsv16_cmd *cmd, rfsv16_reply *reply, bool escape);

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
os_error *rfsv16_back(const rfsv16_cmd *cmd, rfsv16_reply *reply,
                      void *user, share_callback callback);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a client channel for the remote file services.
*/
os_error *rfsv16_start(void);

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy the remote file services client channel.
*/
os_error *rfsv16_end(bool now);

#ifdef __cplusplus
    }
#endif

#endif
