/*
    File        : unified.c
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

// Include header file for this module
#include "unified.h"

// Include clib header files
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "oslib/macros.h"

// Include project header files
#include "attr.h"
#include "cache.h"
#include "code.h"
#include "debug.h"
#include "err.h"
#include "link.h"
#include "mem.h"
#include "name.h"
#include "ncp.h"
#include "rfsv16.h"
#include "rfsv32.h"
#include "timer.h"
#include "uid.h"
#include "upload.h"
#include "util.h"
#include "wildcard.h"

// Private data for each operation
typedef struct unified_private
{
    struct unified_private *next;
    struct unified_private *prev;
    const unified_cmd *cmd;
    unified_reply *reply;
    void *user;
    share_callback callback;
    bool era;
    os_error *err;
    bits length;
    bits index;
    bool delay;
    os_t timeout;
    union
    {
        struct
        {
            rfsv32_cmd cmd;
            rfsv32_reply reply;
            epoc32_handle handle;
            fs_info *entry;
        } rfsv32;
        struct
        {
            rfsv16_cmd cmd;
            rfsv16_reply reply;
            epoc16_handle handle;
        } rfsv16;
        struct
        {
            ncp_cmd cmd;
            ncp_reply reply;
        } ncp;
    } data;
} unified_private;
static unified_private *unified_free_list = NULL;
static unified_private *unified_active_list = NULL;

// A general purpose shared buffer
#define UNIFIED_MIN_BUFFER (4096)
typedef struct unified_buffer_record
{
    struct unified_buffer_record *next;
    void *buffer;
    size_t size;
} unified_buffer_record;
static unified_buffer_record *unified_buffer_head;
static void *unified_buffer = NULL;

// Status for foreground operations
static bool unified_fore_done;
static os_error *unified_fore_err;

// The connection status
static bool unified_active = FALSE;
static bool unified_partial = FALSE;
static bool unified_connected = FALSE;
static bool unified_era = FALSE;

// Special dates for interactive filer copies
#define UNIFIED_DEAD16_HIGH (0x4f)
#define UNIFIED_DEAD16_LOW (0xd2a7f450)
#define UNIFIED_DEAD32_HIGH (0x8b)
#define UNIFIED_DEAD32_LOW (0x87b26778)
#define UNIFIED_DEAD_LOAD (0xdeaddead)
#define UNIFIED_DEAD_EXEC (0xdeaddead)

// Maximum length of time to allow for a task to stop or start (centi-seconds)
#define UNIFIED_STOP_TIMEOUT (2000)
#define UNIFIED_STOP_DELAY (50)
#define UNIFIED_START_DELAY (500)

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
os_error *unified_validate(const char *src, char *dest, size_t size)
{
    os_error *err = NULL;

    // Check function parameters
    if (!src || !dest || !size) err = &err_bad_parms;
    else
    {
        static fs_pathname path;

        // Use the appropriate conversions
        if (unified_era)
        {
            // Convert to ERA format
            err = name_riscos_to_era(src, path, sizeof(path));

            // Convert back to RISC OS format
            if (!err) err = name_era_to_riscos(path, dest, size);
        }
        else
        {
            // Convert to SIBO format
            err = name_riscos_to_sibo(src, path, sizeof(path));

            // Convert back to RISC OS format
            if (!err) err = name_sibo_to_riscos(path, dest, size);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : all           - Should all higher levels be started, or just
                                  those that only require the remote file
                                  server.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start higher level layers.
*/
static os_error *unified_connect(bool all)
{
    os_error *err = NULL;

    // Start higher layers
    err = upload_start(unified_era);
    if (!err && all) err = cache_start(unified_era);

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End higher level layers.
*/
static os_error *unified_disconnect(bool now)
{
    os_error *err = NULL;

    // End higher layers
    err = cache_end(now);
    if (!err) err = upload_end(now);

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check the current connection status.
*/
static os_error *unified_update(void)
{
    os_error *err = NULL;
    bool partial;
    bool connected;

    // Check the remote file server connection status
    if (unified_active && (rfsv32_share_handle != SHARE_NONE))
    {
        unified_era = TRUE;
        partial = TRUE;
    }
    else if (unified_active && (rfsv16_share_handle != SHARE_NONE))
    {
        unified_era = FALSE;
        partial = TRUE;
    }
    else partial = FALSE;

    // Check the remote command server connection status
    if (partial && (ncp_share_handle != SHARE_NONE))
    {
        connected = TRUE;
    }
    else connected = FALSE;

    // Start or end higher layers if necessary
    if ((unified_partial != partial) || (unified_connected != connected))
    {
        // Store the updated status
        unified_partial = partial;
        unified_connected = connected;

        // Start or end higher layers
        if (partial) err = unified_connect(connected);
        else err = unified_disconnect(TRUE);
        if (!err && partial && !connected) err = ncp_start();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : size          - The required buffer size.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Attempt to ensure that the buffer is at least the specified
                  size.
*/
static os_error *unified_buffer_size(size_t size)
{
    os_error *err = NULL;

    // Check if the current buffer if large enough
    if (!unified_buffer_head || (unified_buffer_head->size < size))
    {
        unified_buffer_record *ptr = NULL;
        size_t requested = size;

        // Round the requested buffer size
        size = UNIFIED_MIN_BUFFER;
        while (size < requested) size <<= 1;

        // Allocate a new buffer record
        ptr = (unified_buffer_record *) MEM_MALLOC(sizeof(unified_buffer_record));
        if (!ptr) err = &err_buffer;

        // Allocate a new buffer
        if (!err)
        {
            ptr->buffer = MEM_MALLOC(size);
            if (!ptr->buffer) err = &err_buffer;
        }

        // Complete the details and link in the new record
        if (!err)
        {
            ptr->next = unified_buffer_head;
            unified_buffer_head = ptr;
            ptr->size = size;
            unified_buffer = ptr->buffer;
        }

        // Release the buffer if failed
        if (err && ptr) MEM_FREE(ptr);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Free any allocated memory.
*/
static os_error *unified_free(void)
{
    os_error *err = NULL;

    // Loop through all of the buffers
    while (unified_buffer_head)
    {
        unified_buffer_record *ptr = unified_buffer_head;

        // Update the buffer list head pointer
        unified_buffer_head = ptr->next;

        // Free the memory
        MEM_FREE(ptr->buffer);
        MEM_FREE(ptr);
    }

    // Free operation buffers also
    while (unified_free_list)
    {
        unified_private *ptr = unified_free_list;

        // Update the free list head pointer
        unified_free_list = ptr->next;

        // Free the memory
        MEM_FREE(ptr);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : type          - The file type.
                  date          - The date stamp.
                  load          - Variable to receive the load address.
                  exec          - Variable to receive the execution address.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Construct the load and execution addresses based on a file
                  type and date stamp.
*/
os_error *unified_load_exec(bits type, const date_riscos *date,
                            bits *load, bits *exec)
{
    os_error *err = NULL;

    // Check function parameters
    if (!date || !load || !exec) err = &err_bad_parms;
    else if (((date->words.high == UNIFIED_DEAD16_HIGH)
              && (date->words.low == UNIFIED_DEAD16_LOW))
             || ((date->words.high == UNIFIED_DEAD32_HIGH)
                 && (date->words.low == UNIFIED_DEAD32_LOW)))
    {
        // Special case if the date is due to an interactive filer copy
        *load = UNIFIED_DEAD_LOAD;
        *exec = UNIFIED_DEAD_EXEC;
    }
    else
    {
        // A normal date stamped file
        *load = 0xfff00000 | (type << 8) | date->words.high;
        *exec = date->words.low;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : era           - The ERA format structure.
                  riscos        - Variable to receive the RISC OS equivalent.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Convert an ERA file information structure to RISC OS format.
*/
static os_error *unified_era_info_to_riscos(const epoc32_remote_entry *era,
                                            fs_info *riscos)
{
    os_error *err = NULL;

    // Check function parameters
    if (!era || !riscos) err = &err_bad_parms;
    else
    {
        // Complete the easy fields
        riscos->size = era->size;
        riscos->attr = attr_from_era(era->attributes);
        riscos->obj_type = era->attributes & EPOC32_FILE_DIRECTORY
                           ? fileswitch_IS_DIR
                           : fileswitch_IS_FILE;

        // Translate the filename
        err = code_era_to_riscos(era->name, riscos->name, sizeof(riscos->name));

        // Build the load and execution addresses
        if (!err)
        {
            err = unified_load_exec(uid_map_type(riscos->name, &era->uid),
                                    date_from_era(&era->modified),
                                    &riscos->load_addr, &riscos->exec_addr);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : sibo          - The SIBO format structure.
                  riscos        - Variable to receive the RISC OS equivalent.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Convert a SIBO file information structure to RISC OS format.
*/
static os_error *unified_sibo_info_to_riscos(const epoc16_p_info *sibo,
                                             fs_info *riscos)
{
    os_error *err = NULL;

    // Check function parameters
    if (!sibo || !riscos) err = &err_bad_parms;
    else
    {
        // Complete the easy fields
        riscos->size = sibo->size;
        riscos->attr = attr_from_sibo(sibo->attributes);
        riscos->obj_type = sibo->attributes & EPOC16_FILE_DIRECTORY
                           ? fileswitch_IS_DIR
                           : fileswitch_IS_FILE;

        // Translate the filename
        err = code_sibo_to_riscos(sibo->name, riscos->name, sizeof(riscos->name));

        // Build the load and execution addresses
        if (!err)
        {
            err = unified_load_exec(uid_map_type(riscos->name, NULL),
                                    date_from_sibo(sibo->modified),
                                    &riscos->load_addr, &riscos->exec_addr);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : info1 - The first information structure.
                  info2 - The second information structure.
    Returns     : int   - The result of the comparison:
                            < 0 if info1 < info2
                             0  if info1 == info2
                            > 0 if info1 > info2
    Description :
*/
static int unified_info_cmp(const void *info1, const void *info2)
{
    const fs_info *ptr1 = (const fs_info *) info1;
    const fs_info *ptr2 = (const fs_info *) info2;

    // Perform a case insensitive comparison of the names
    return wildcard_cmp(ptr1->name, ptr2->name);
}

/*
    Parameters  : op            - The operation data.
                  err           - Any error to return.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End processing an operation.
*/
static os_error *unified_done(unified_private *op, os_error *err)
{
    // Check function parameters
    if (!op) err = &err_bad_parms;
    else
    {
        // Unlink from the active list
        if (op->next) op->next->prev = op->prev;
        if (op->prev) op->prev->next = op->next;
        else unified_active_list = op->next;

        // Add to the free list
        op->prev = NULL;
        op->next = unified_free_list;
        if (unified_free_list) unified_free_list->prev = op;
        unified_free_list = op;

        // Call the callback function
        err = (*op->callback)(op->user, err, op->reply);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a background operation.
*/
static os_error *unified_callback(void *user, os_error *err, const void *reply)
{
    // Check function parameters
    if (!user || (!err && !reply)) err = &err_bad_parms;
    else
    {
        unified_private *op = (unified_private *) user;
        bool done = TRUE;
        static fs_pathname path;
        rfsv32_cmd *cmd32 = &op->data.rfsv32.cmd;
        rfsv16_cmd *cmd16 = &op->data.rfsv16.cmd;
        ncp_cmd *cmdncp = &op->data.ncp.cmd;
        rfsv32_reply *reply32 = &op->data.rfsv32.reply;
        rfsv16_reply *reply16 = &op->data.rfsv16.reply;
        ncp_reply *replyncp = &op->data.ncp.reply;

        // Decode the result of the operation
        switch (op->cmd->op)
        {
            case UNIFIED_DRIVE:
                // Read the details for a drive
                if (!err)
                {
                    if (!op->index)
                    {
                        if (op->era)
                        {
                            op->reply->drive.drive.present = TRUE;
                            op->reply->drive.drive.rom = reply32->req_volume.volume.type == EPOC32_MEDIA_ROM;
                            op->reply->drive.drive.free_low = reply32->req_volume.volume.free.low;
                            op->reply->drive.drive.free_high = reply32->req_volume.volume.free.high;
                            op->reply->drive.drive.size_low = reply32->req_volume.volume.size.low;
                            op->reply->drive.drive.size_high = reply32->req_volume.volume.size.high;
                            err = code_era_to_riscos(reply32->req_volume.volume.name, op->reply->drive.drive.name, sizeof(op->reply->drive.drive.name));
                        }
                        else
                        {
                            op->reply->drive.drive.present = TRUE;
                            op->reply->drive.drive.rom = (reply16->rf_fdeviceread.dinfo.type & EPOC16_MEDIA_TYPE_MASK) == EPOC16_MEDIA_ROM;
                            op->reply->drive.drive.free_low = reply16->rf_fdeviceread.dinfo.free;
                            op->reply->drive.drive.free_high = 0;
                            op->reply->drive.drive.size_low = reply16->rf_fdeviceread.dinfo.size;
                            op->reply->drive.drive.size_high = 0;
                            err = code_sibo_to_riscos(reply16->rf_fdeviceread.dinfo.name, op->reply->drive.drive.name, sizeof(op->reply->drive.drive.name));
                        }
                        if (!err)
                        {
                            done = FALSE;
                            op->index = 1;
                            cmdncp->op = NCP_GET_UNIQUE_ID;
                            sprintf(cmdncp->data.get_unique_id.name, "%c:",
                                    toupper(op->cmd->data.drive.drive));
                            err = ncp_back(cmdncp, replyncp, op, unified_callback);
                        }
                    }
                    else
                    {
                        op->reply->drive.drive.id = replyncp->get_unique_id.id;
                    }
                }
                else if (ERR_EQ(*err, err_remote_not_sup) && op->index)
                {
                    // Valid drive but unable to read unique identifier
                    err = NULL;
                    op->reply->drive.drive.id = 0;
                }
                else if (ERR_EQ(*err, err_drive_empty)
                         || ERR_EQ(*err, err_remote_not_ready)
                         || ERR_EQ(*err, err_remote_not_sup))
                {
                    // Fill the structure with defaults
                    err = NULL;
                    op->reply->drive.drive.present = FALSE;
                    op->reply->drive.drive.rom = FALSE;
                    *op->reply->drive.drive.name = '\0';
                    op->reply->drive.drive.free_low = 0;
                    op->reply->drive.drive.free_high = 0;
                    op->reply->drive.drive.size_low = 0;
                    op->reply->drive.drive.size_high = 0;
                    op->reply->drive.drive.id = 0;
                }
                break;

            case UNIFIED_LIST:
                // Obtain a directory listing
                if (op->era)
                {
                    // Action depends on the last operation
                    if (!err && (cmd32->op == RFSV32_REQ_OPEN_DIR))
                    {
                        // Attempt to read the directory contents
                        done = FALSE;
                        op->data.rfsv32.handle = reply32->req_open_dir.handle;
                        cmd32->op = RFSV32_REQ_READ_DIR;
                        cmd32->data.req_read_dir.handle = op->data.rfsv32.handle;
                        cmd32->data.req_read_dir.buffer = (epoc32_remote_entry *) unified_buffer;
                        cmd32->data.req_read_dir.size = op->cmd->data.list.size;
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                    else if (cmd32->op == RFSV32_REQ_READ_DIR)
                    {
                        // Decode the reply
                        while (!err && (cmd32->data.req_read_dir.buffer
                                        != reply32->req_read_dir.next))
                        {
                            // Process the next entry
                            err = unified_era_info_to_riscos(cmd32->data.req_read_dir.buffer, op->reply->list.next);
                            if (!err)
                            {
                                cmd32->data.req_read_dir.buffer++;
                                op->reply->list.next++;
                                op->reply->list.used++;
                                op->reply->list.remain--;
                            }
                        }

                        // Try again if no error
                        if (!err)
                        {
                            // Read the next block of entries
                            done = FALSE;
                            cmd32->data.req_read_dir.size = op->reply->list.remain;
                            err = rfsv32_back(cmd32, reply32, op, unified_callback);
                        }
                        else
                        {
                            // Close the directory
                            done = FALSE;
                            op->err = err && !ERR_EQ(*err, err_eof)
                                      ? err : NULL;
                            cmd32->op = RFSV32_REQ_CLOSE_HANDLE;
                            cmd32->data.req_close_handle.handle = op->data.rfsv32.handle;
                            err = rfsv32_back(cmd32, reply32, op, unified_callback);
                        }
                    }
                    else if (!err && (cmd32->op == RFSV32_REQ_CLOSE_HANDLE))
                    {
                        // Restore the previous error
                        err = op->err;
                    }
                }
                else
                {
                    // Action depends on the last operation
                    if (!err && (cmd16->op == RFSV16_RF_FOPEN))
                    {
                        // Attempt to read the directory contents
                        done = FALSE;
                        op->data.rfsv16.handle = reply16->rf_fopen.handle;
                        cmd16->op = RFSV16_RF_FDIRREAD;
                        cmd16->data.rf_fdirread.handle = op->data.rfsv16.handle;
                        cmd16->data.rf_fdirread.buffer = (epoc16_p_info *) unified_buffer;
                        cmd16->data.rf_fdirread.size = op->cmd->data.list.size;
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                    else if (cmd16->op == RFSV16_RF_FDIRREAD)
                    {
                        // Decode the reply
                        while (!err && (cmd16->data.rf_fdirread.buffer
                                        != reply16->rf_fdirread.next))
                        {
                            // Process the next entry
                            err = unified_sibo_info_to_riscos(cmd16->data.rf_fdirread.buffer, op->reply->list.next);
                            if (!err)
                            {
                                cmd16->data.rf_fdirread.buffer++;
                                op->reply->list.next++;
                                op->reply->list.used++;
                                op->reply->list.remain--;
                            }
                        }

                        // Try again if no error
                        if (!err)
                        {
                            // Read the next block of entries
                            done = FALSE;
                            cmd16->data.rf_fdirread.size = op->reply->list.remain;
                            err = rfsv16_back(cmd16, reply16, op, unified_callback);
                        }
                        else
                        {
                            // Close the directory
                            done = FALSE;
                            op->err = err && !ERR_EQ(*err, err_eof)
                                      ? err : NULL;
                            cmd16->op = RFSV16_RF_FCLOSE;
                            cmd16->data.rf_fclose.handle = op->data.rfsv16.handle;
                            err = rfsv16_back(cmd16, reply16, op, unified_callback);
                        }
                    }
                    else if (!err && (cmd16->op == RFSV16_RF_FCLOSE))
                    {
                        // Restore any previous error
                        err = op->err;
                    }
                }
                if (!err && done && op->reply->list.used)
                {
                    // Sort the directory entries alphabetically
                    qsort(op->cmd->data.list.buffer,
                          op->reply->list.used,
                          sizeof(*op->cmd->data.list.buffer),
                          unified_info_cmp);
                }
                break;

            case UNIFIED_INFO:
                // Read the information about a single file
                if (!err)
                {
                    if (op->era) err = unified_era_info_to_riscos(&reply32->req_remote_entry.entry, &op->reply->info.info);
                    else
                    {
                        err = unified_sibo_info_to_riscos(&reply16->rf_finfo.entry, &op->reply->info.info);
                        if (!err)
                        {
                            // Preserve the existing leaf name
                            const char *ptr = strrchr(op->cmd->data.info.path, FS_CHAR_SEPARATOR);
                            if (ptr)
                            {
                                if (sizeof(op->reply->info.info.name)
                                    <= strlen(ptr))
                                {
                                    err = &err_bad_name;
                                }
                                else strcpy(op->reply->info.info.name, ptr + 1);
                            }
                        }
                    }
                }
                break;

            case UNIFIED_OPEN:
                // Open a file
                if (!err)
                {
                    op->reply->open.handle = op->era
                                             ? reply32->req_open_file.handle
                                             : reply16->rf_fopen.handle;
                }
                break;

            case UNIFIED_READ:
                // Read from a file
                if (!err)
                {
                    if (op->era)
                    {
                        op->length += reply32->req_read_file.length;
                        if (reply32->req_read_file.length
                            && (op->length < op->cmd->data.read.length))
                        {
                            done = FALSE;
                            cmd32->data.req_read_file.length = MIN(RFSV32_MAX_READ, op->cmd->data.read.length - op->length);
                            cmd32->data.req_read_file.buffer = (byte *) op->cmd->data.read.buffer + op->length;
                            err = rfsv32_back(cmd32, reply32, op, unified_callback);
                        }
                        else op->reply->read.length = op->length;
                    }
                    else
                    {
                        op->length += reply16->rf_fread.length;
                        if (reply16->rf_fread.length
                            && (op->length < op->cmd->data.read.length))
                        {
                            done = FALSE;
                            cmd16->data.rf_fread.length = MIN(RFSV16_MAX_READ, op->cmd->data.read.length - op->length);
                            cmd16->data.rf_fread.buffer = (byte *) op->cmd->data.read.buffer + op->length;
                            err = rfsv16_back(cmd16, reply16, op, unified_callback);
                        }
                        else op->reply->read.length = op->length;
                    }
                }
                break;

            case UNIFIED_WRITE:
                // Write to a file
                if (!err)
                {
                    if (op->era)
                    {
                        op->length += cmd32->data.req_write_file.length;
                        if (op->length < op->cmd->data.write.length)
                        {
                            done = FALSE;
                            cmd32->data.req_write_file.length = MIN(RFSV32_MAX_WRITE, op->cmd->data.write.length - op->length);
                            cmd32->data.req_write_file.buffer = (byte *) op->cmd->data.write.buffer + op->length;
                            err = rfsv32_back(cmd32, reply32, op, unified_callback);
                        }
                    }
                    else
                    {
                        op->length += cmd16->data.rf_fwrite.length;
                        if (op->length < op->cmd->data.write.length)
                        {
                            done = FALSE;
                            cmd16->data.rf_fwrite.length = MIN(RFSV16_MAX_WRITE, op->cmd->data.write.length - op->length);
                            cmd16->data.rf_fwrite.buffer = (byte *) op->cmd->data.write.buffer + op->length;
                            err = rfsv16_back(cmd16, reply16, op, unified_callback);
                        }
                    }
                }
                break;

            case UNIFIED_ZERO:
                // Write zeros to a file
                if (!err)
                {
                    if (op->era)
                    {
                        op->length += cmd32->data.req_write_file.length;
                        if (op->length < op->cmd->data.write.length)
                        {
                            done = FALSE;
                            cmd32->data.req_write_file.length = MIN(RFSV32_MAX_WRITE, op->cmd->data.write.length - op->length);
                            err = rfsv32_back(cmd32, reply32, op, unified_callback);
                        }
                    }
                    else
                    {
                        op->length += cmd16->data.rf_fwrite.length;
                        if (op->length < op->cmd->data.write.length)
                        {
                            done = FALSE;
                            cmd16->data.rf_fwrite.length = MIN(RFSV16_MAX_WRITE, op->cmd->data.write.length - op->length);
                            err = rfsv16_back(cmd16, reply16, op, unified_callback);
                        }
                    }
                }
                break;

            case UNIFIED_MACHINE:
                // Read the remote machine type
                if (!err)
                {
                    if (cmdncp->op == NCP_GET_MACHINE_TYPE)
                    {
                        op->reply->machine.type = replyncp->get_machine_type.type;
                        op->reply->machine.id.low = 0;
                        op->reply->machine.id.high = 0;
                        op->reply->machine.language = psifs_LANGUAGE_UNKNOWN;
                        op->reply->machine.version.major = 0;
                        op->reply->machine.version.minor = 0;
                        op->reply->machine.version.build = 0;
                        switch (replyncp->get_machine_type.type)
                        {
                            case psifs_MACHINE_TYPE_S3:
                                // Psion Series 3
                                strcpy(op->reply->machine.name, "Psion Series 3");
                                break;

                            case psifs_MACHINE_TYPE_S3A:
                                // Psion Series 3a/3c/mx
                                strcpy(op->reply->machine.name, "Psion Series 3a, 3c or 3mx");
                                break;

                            case psifs_MACHINE_TYPE_SIENNA:
                                // Psion Sienna
                                strcpy(op->reply->machine.name, "Psion Sienna");
                                break;

                            case psifs_MACHINE_TYPE_S3C:
                                // Psion Series 3c
                                strcpy(op->reply->machine.name, "Psion Series 3c");
                                break;

                            case psifs_MACHINE_TYPE_S5:
                                // Psion Series 5 or Geofox One
                                strcpy(op->reply->machine.name, "Psion Series 5 or Geofox-One");
                                break;

                            default:
                                // Unrecognised machine type
                                sprintf(op->reply->machine.name,
                                        "unrecognised machine type (%u)",
                                        replyncp->get_machine_type.type);
                                break;
                        }
                        if (op->era)
                        {
                            done = FALSE;
                            cmdncp->op = NCP_GET_MACHINE_INFO;
                            err = ncp_back(cmdncp, replyncp, op, unified_callback);
                        }
                    }
                    else
                    {
                        op->reply->machine.id.low = replyncp->get_machine_info.info.machine_uid.low;
                        op->reply->machine.id.high = replyncp->get_machine_info.info.machine_uid.high;
                        op->reply->machine.language = replyncp->get_machine_info.info.language;
                        op->reply->machine.version.major = replyncp->get_machine_info.info.rom_version.major;
                        op->reply->machine.version.minor = replyncp->get_machine_info.info.rom_version.minor;
                        op->reply->machine.version.build = replyncp->get_machine_info.info.rom_version.build;
                        if (strlen(replyncp->get_machine_info.info.machine_name) < sizeof(op->reply->machine.name))
                        {
                            err = code_ansi_to_latin1(replyncp->get_machine_info.info.machine_name, op->reply->machine.name);
                        }
                    }
                }
                break;

            case UNIFIED_TASKS:
                // Read the tasks with files open on a single drive
                if (!err)
                {
                    if (!op->era && (cmdncp->data.query_drive.drive < 'Z'))
                    {
                        // Try the next drive letter
                        done = FALSE;
                        cmdncp->data.query_drive.drive++;
                        cmdncp->data.query_drive.buffer = replyncp->query_drive.next;
                        cmdncp->data.query_drive.size = replyncp->query_drive.remain;
                        err = ncp_back(cmdncp, replyncp, op, unified_callback);
                    }
                    else
                    {
                        // Decode the reply
                        op->reply->tasks.next = op->cmd->data.tasks.buffer;
                        op->reply->tasks.used = 0;
                        op->reply->tasks.remain = op->cmd->data.tasks.size;
                        while (!err && (op->reply->tasks.next
                                        != replyncp->query_drive.next))
                        {
                            // Process the next entry
                            if (!op->era) err = code_850_to_ansi(op->reply->tasks.next->name, op->reply->tasks.next->name);
                            if (!err)
                            {
                                if (op->era) err = name_era_to_riscos(op->reply->tasks.next->args, path, sizeof(path));
                                else err = name_sibo_to_riscos(op->reply->tasks.next->args, path, sizeof(path));
                                if (!err && (strlen(path) < sizeof(op->reply->tasks.next->args)))
                                {
                                    strcpy(op->reply->tasks.next->args, path);
                                }
                                else
                                {
                                    if (op->era) err = NULL;
                                    else err = code_850_to_ansi(op->reply->tasks.next->args, op->reply->tasks.next->args);
                                }
                            }
                            if (!err)
                            {
                                op->reply->tasks.next++;
                                op->reply->tasks.used++;
                                op->reply->tasks.remain--;
                            }
                        }
                    }
                }
                break;

            case UNIFIED_DETAIL:
                // Read the command line for a task
                if (!err)
                {
                    if (op->era) err = name_era_to_riscos(replyncp->get_cmd_line.name, op->reply->detail.name, sizeof(op->reply->detail.name));
                    else err = name_sibo_to_riscos(replyncp->get_cmd_line.name, op->reply->detail.name, sizeof(op->reply->detail.name));
                    if (err && (strlen(replyncp->get_cmd_line.name)) < sizeof(op->reply->detail.name))
                    {
                        if (op->era) err = NULL;
                        else err = code_850_to_ansi(replyncp->get_cmd_line.name, replyncp->get_cmd_line.name);
                        strcpy(op->reply->detail.name, replyncp->get_cmd_line.name);
                    }
                }
                if (!err)
                {
                    if (op->era) err = name_era_to_riscos(replyncp->get_cmd_line.args, op->reply->detail.args, sizeof(op->reply->detail.args));
                    else err = name_sibo_to_riscos(replyncp->get_cmd_line.args, op->reply->detail.args, sizeof(op->reply->detail.args));
                    if (err && (strlen(replyncp->get_cmd_line.args)) < sizeof(op->reply->detail.args))
                    {
                        if (op->era) err = NULL;
                        else err = code_850_to_ansi(replyncp->get_cmd_line.args, replyncp->get_cmd_line.args);
                        strcpy(op->reply->detail.args, replyncp->get_cmd_line.args);
                    }
                }
                break;

            case UNIFIED_STOP:
                // Stop a task
                if (!err && ((util_time() - op->timeout) < 0))
                {
                    op->index++;
                    if (op->delay)
                    {
                        // Check whether the task has stopped
                        done = FALSE;
                        op->delay = FALSE;
                        cmdncp->op = NCP_PROG_RUNNING;
                        if ((sizeof(cmdncp->data.prog_running.name)
                             <= strlen(op->cmd->data.stop.name)))
                        {
                            err = &err_bad_name;
                        }
                        else
                        {
                            strcpy(cmdncp->data.prog_running.name, op->cmd->data.stop.name);
                            if (!op->era) err = code_ansi_to_850(cmdncp->data.prog_running.name, cmdncp->data.prog_running.name);
                        }

                        if (!err) err = ncp_back(cmdncp, replyncp, op, unified_callback);
                    }
                    else if (replyncp->prog_running.running)
                    {
                        // Set a timer
                        done = FALSE;
                        op->delay = TRUE;
                        err = timer_back(util_time() + UNIFIED_STOP_DELAY,
                                         op, unified_callback);
                    }
                    op->index--;
                }
                break;

            case UNIFIED_START:
                // Start a task
                if (!err && !op->delay)
                {
                    // Allow the task to start
                    done = FALSE;
                    op->delay = TRUE;
                    err = timer_back(util_time() + UNIFIED_START_DELAY,
                                     op, unified_callback);
                }
                /*
                else if (err && ERR_EQ(*err, err_not_found))
                {
                    // Ignore not found errors
                    err = NULL;
                }
                */
                break;

            case UNIFIED_POWER:
                // Read power supply details
                if (!err)
                {
                    op->reply->power.main.status = replyncp->get_machine_info.info.supply.main_status;
                    op->reply->power.main.mv = replyncp->get_machine_info.info.supply.main_mv;
                    op->reply->power.main.mv_max = replyncp->get_machine_info.info.supply.main_mv_max;
                    op->reply->power.backup.status = replyncp->get_machine_info.info.supply.backup_status;
                    op->reply->power.backup.mv = replyncp->get_machine_info.info.supply.backup_mv;
                    op->reply->power.backup.mv_max = replyncp->get_machine_info.info.supply.backup_mv_max;
                    op->reply->power.external = replyncp->get_machine_info.info.supply.external;
                }
                break;

            case UNIFIED_RTIME:
                // Read system time
                if (!err)
                {
                    op->reply->rtime.date = *date_from_era(&replyncp->get_machine_info.info.time.home_time);
                }
                break;

            case UNIFIED_WTIME:
                // Write system time
                if (!err && (cmdncp->op == NCP_GET_MACHINE_INFO))
                {
                    // Set the displayed time to be the same on both machines
                    done = FALSE;
                    cmdncp->op = NCP_SET_TIME;
                    cmdncp->data.set_time.time = replyncp->get_machine_info.info.time;
                    cmdncp->data.set_time.time.home_time = *date_to_era(&op->cmd->data.wtime.date);
                    err = ncp_back(cmdncp, replyncp, op, unified_callback);
                }
                break;

            case UNIFIED_OWNER:
                // Read owner information
                if (!err)
                {
                    if (op->era) err = code_ansi_to_latin1(replyncp->get_owner_info.info, op->reply->owner.info);
                    else err = code_850_to_latin1(replyncp->get_owner_info.info, op->reply->owner.info);
                }
                break;

            case UNIFIED_NAME:
            case UNIFIED_MKDIR:
            case UNIFIED_REMOVE:
            case UNIFIED_RMDIR:
            case UNIFIED_RENAME:
            case UNIFIED_ACCESS:
            case UNIFIED_STAMP:
            case UNIFIED_CLOSE:
            case UNIFIED_SEEK:
            case UNIFIED_SIZE:
            case UNIFIED_FLUSH:
                // No action required
                break;

            default:
                // Not a supported command
                if (!err) err = &err_bad_unified_op;
                break;
        }

        // Call the callback function if appropriate
        if (err) err = unified_done(op, err);
        else if (done) err = unified_done(op, NULL);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - The operation data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start processing an operation.
*/
static os_error *unified_begin(unified_private *op)
{
    os_error *err = NULL;

    // Check function parameters
    if (!op) err = &err_bad_parms;
    else
    {
        bool done = FALSE;
        static fs_pathname path;
        rfsv32_cmd *cmd32 = &op->data.rfsv32.cmd;
        rfsv16_cmd *cmd16 = &op->data.rfsv16.cmd;
        ncp_cmd *cmdncp = &op->data.ncp.cmd;
        rfsv32_reply *reply32 = &op->data.rfsv32.reply;
        rfsv16_reply *reply16 = &op->data.rfsv16.reply;
        ncp_reply *replyncp = &op->data.ncp.reply;

        // Choose remote device type
        err = unified_update();
        if (!err && !unified_partial) err = &err_svr_none;
        if (!err) op->era = unified_era;

        // Action depends on the specified operation
        switch (op->cmd->op)
        {
            case UNIFIED_DRIVE:
                // Read the details for a drive
                if (op->era)
                {
                    if (!err)
                    {
                        op->index = 0;
                        cmd32->op = RFSV32_REQ_VOLUME;
                        cmd32->data.req_volume.drive = toupper(op->cmd->data.drive.drive) - 'A';
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                }
                else
                {
                    if (!err)
                    {
                        op->index = 0;
                        cmd16->op = RFSV16_RF_STATUSDEVICE;
                        sprintf(cmd16->data.rf_statusdevice.name, "%c:",
                                toupper(op->cmd->data.drive.drive));
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_NAME:
                // Change the name of a disc
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_SET_VOLUME_LABEL;
                        cmd32->data.req_set_volume_label.drive = toupper(op->cmd->data.name.drive) - 'A';
                        err = code_riscos_to_era(op->cmd->data.name.name,
                                                 cmd32->data.req_set_volume_label.name,
                                                 sizeof(cmd32->data.req_set_volume_label.name));
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_SFSTAT;
                        cmd16->data.rf_sfstat.set = EPOC16_FILE_VOLUME;
                        cmd16->data.rf_sfstat.mask = EPOC16_FILE_VOLUME;
                        sprintf(path, ":%c.$.%s",
                                toupper(op->cmd->data.name.drive),
                                op->cmd->data.name.name);
                        err = name_riscos_to_sibo(path,
                                                  cmd16->data.rf_sfstat.name,
                                                  sizeof(cmd16->data.rf_sfstat.name));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_LIST:
                // Obtain a directory listing
                if (op->era)
                {
                    if (!err)
                    {
                        err = unified_buffer_size(op->cmd->data.list.size
                                                  * sizeof(epoc32_remote_entry));
                    }
                    if (!err)
                    {
                        op->reply->list.next = op->cmd->data.list.buffer;
                        op->reply->list.used = 0;
                        op->reply->list.remain = op->cmd->data.list.size;
                        cmd32->op = RFSV32_REQ_OPEN_DIR;
                        cmd32->data.req_open_dir.attributes = EPOC32_FILE_HIDDEN
                                                              | EPOC32_FILE_SYSTEM
                                                              | EPOC32_FILE_DIRECTORY
                                                              | EPOC32_FILE_UID;
                        if (sizeof(path) < strlen(op->cmd->data.list.path) + 3)
                        {
                            err = &err_bad_name;
                        }
                    }
                    if (!err)
                    {
                        sprintf(path, "%s.*", op->cmd->data.list.path);
                        err = name_riscos_to_era(path,
                                                 cmd32->data.req_open_dir.match,
                                                 sizeof(cmd32->data.req_open_dir.match));
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        err = unified_buffer_size(op->cmd->data.list.size
                                                  * sizeof(epoc16_p_info));
                    }
                    if (!err)
                    {
                        op->reply->list.next = op->cmd->data.list.buffer;
                        op->reply->list.used = 0;
                        op->reply->list.remain = op->cmd->data.list.size;
                        cmd16->op = RFSV16_RF_FOPEN;
                        cmd16->data.rf_fopen.mode = EPOC16_MODE_DIRECTORY_RECORD;
                        err = name_riscos_to_sibo(op->cmd->data.list.path,
                                                  cmd16->data.rf_fopen.name,
                                                  sizeof(cmd16->data.rf_fopen.name));
                    }
                    if (!err && (sizeof(cmd16->data.rf_fopen.name) < strlen(cmd16->data.rf_fopen.name) + 2))
                    {
                        err = &err_bad_name;
                    }
                    if (!err)
                    {
                        char *ptr = strchr(cmd16->data.rf_fopen.name, '\0');
                        if (ptr[-1] != NAME_CHAR_SEPARATOR)
                        {
                            *ptr++ = NAME_CHAR_SEPARATOR;
                            *ptr = '\0';
                        }
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_INFO:
                // Read the information about a single file
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_REMOTE_ENTRY;
                        err = name_riscos_to_era(op->cmd->data.info.path,
                                                 cmd32->data.req_remote_entry.name,
                                                 sizeof(cmd32->data.req_remote_entry.name));
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_FINFO;
                        err = name_riscos_to_sibo(op->cmd->data.info.path,
                                                  cmd16->data.rf_finfo.name,
                                                  sizeof(cmd16->data.rf_finfo.name));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_MKDIR:
                // Create a directory
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_MK_DIR_ALL;
                        err = name_riscos_to_era(op->cmd->data.mkdir.path,
                                                 cmd32->data.req_mk_dir_all.name,
                                                 sizeof(cmd32->data.req_mk_dir_all.name));
                    }
                    if (!err && (sizeof(cmd32->data.req_mk_dir_all.name)
                                 <= (strlen(cmd32->data.req_mk_dir_all.name) + 1)))
                    {
                        err = &err_bad_name;
                    }
                    if (!err)
                    {
                        char *ptr = strchr(cmd32->data.req_mk_dir_all.name, '\0');
                        *ptr++ = NAME_CHAR_SEPARATOR;
                        *ptr = '\0';
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_MKDIR;
                        err = name_riscos_to_sibo(op->cmd->data.mkdir.path,
                                                  cmd16->data.rf_mkdir.name,
                                                  sizeof(cmd16->data.rf_mkdir.name));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_REMOVE:
                // Delete a file
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_DELETE;
                        err = name_riscos_to_era(op->cmd->data.remove.path,
                                                 cmd32->data.req_delete.name,
                                                 sizeof(cmd32->data.req_delete.name));
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_DELETE;
                        err = name_riscos_to_sibo(op->cmd->data.remove.path,
                                                  cmd16->data.rf_delete.name,
                                                  sizeof(cmd16->data.rf_delete.name));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_RMDIR:
                // Delete a directory
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_RM_DIR;
                        err = name_riscos_to_era(op->cmd->data.rmdir.path,
                                                 cmd32->data.req_rm_dir.name,
                                                 sizeof(cmd32->data.req_rm_dir.name));
                    }
                    if (!err && (sizeof(cmd32->data.req_rm_dir.name)
                                 <= (strlen(cmd32->data.req_rm_dir.name) + 1)))
                    {
                        err = &err_bad_name;
                    }
                    if (!err)
                    {
                        char *ptr = strchr(cmd32->data.req_rm_dir.name, '\0');
                        *ptr++ = NAME_CHAR_SEPARATOR;
                        *ptr = '\0';
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_DELETE;
                        err = name_riscos_to_sibo(op->cmd->data.rmdir.path,
                                                  cmd16->data.rf_delete.name,
                                                  sizeof(cmd16->data.rf_delete.name));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_RENAME:
                // Rename a file
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_RENAME;
                        err = name_riscos_to_era(op->cmd->data.rename.src,
                                                 cmd32->data.req_rename.src,
                                                 sizeof(cmd32->data.req_rename.src));
                    }
                    if (!err)
                    {
                        err = name_riscos_to_era(op->cmd->data.rename.dest,
                                                 cmd32->data.req_rename.dest,
                                                 sizeof(cmd32->data.req_rename.dest));
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_RENAME;
                        err = name_riscos_to_sibo(op->cmd->data.rename.src,
                                                  cmd16->data.rf_rename.src,
                                                  sizeof(cmd16->data.rf_rename.src));
                    }
                    if (!err)
                    {
                        err = name_riscos_to_sibo(op->cmd->data.rename.dest,
                                                  cmd16->data.rf_rename.dest,
                                                  sizeof(cmd16->data.rf_rename.dest));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_ACCESS:
                // Change the attributes
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_SET_ATT;
                        cmd32->data.req_set_att.set = attr_to_era(op->cmd->data.access.attr);
                        cmd32->data.req_set_att.clear = ATTR_ERA_MASK & ~cmd32->data.req_set_att.set;
                        err = name_riscos_to_era(op->cmd->data.access.path,
                                                 cmd32->data.req_set_att.name,
                                                 sizeof(cmd32->data.req_set_att.name));
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_SFSTAT;
                        cmd16->data.rf_sfstat.set = attr_to_sibo(op->cmd->data.access.attr);
                        cmd16->data.rf_sfstat.mask = ATTR_SIBO_MASK;
                        err = name_riscos_to_sibo(op->cmd->data.access.path,
                                                  cmd16->data.rf_sfstat.name,
                                                  sizeof(cmd16->data.rf_sfstat.name));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_STAMP:
                // Set the modification date
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_SET_MODIFIED;
                        cmd32->data.req_set_modified.modified = *date_to_era(&op->cmd->data.stamp.date);
                        err = name_riscos_to_era(op->cmd->data.stamp.path,
                                                 cmd32->data.req_set_modified.name,
                                                 sizeof(cmd32->data.req_set_modified.name));
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_SFDATE;
                        cmd16->data.rf_sfdate.modified = date_to_sibo(&op->cmd->data.stamp.date);
                        err = name_riscos_to_sibo(op->cmd->data.stamp.path,
                                                  cmd16->data.rf_sfdate.name,
                                                  sizeof(cmd16->data.rf_sfdate.name));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_OPEN:
                // Open a file
                if (op->era)
                {
                    if (!err)
                    {
                        if (op->cmd->data.open.mode == FS_MODE_OUT)
                        {
                            cmd32->op = RFSV32_REQ_REPLACE_FILE;
                            cmd32->data.req_replace_file.mode = EPOC32_MODE_SHARE_EXCLUSIVE | EPOC32_MODE_BINARY | EPOC32_MODE_READ_WRITE;
                            err = name_riscos_to_era(op->cmd->data.open.path,
                                                     cmd32->data.req_replace_file.name,
                                                     sizeof(cmd32->data.req_replace_file.name));
                        }
                        else if (op->cmd->data.open.mode == FS_MODE_IN)
                        {
                            cmd32->op = RFSV32_REQ_OPEN_FILE;
                            cmd32->data.req_open_file.mode = EPOC32_MODE_SHARE_ANY | EPOC32_MODE_BINARY;
                            err = name_riscos_to_era(op->cmd->data.open.path,
                                                     cmd32->data.req_open_file.name,
                                                     sizeof(cmd32->data.req_open_file.name));
                        }
                        else if (op->cmd->data.open.mode == FS_MODE_UP)
                        {
                            cmd32->op = RFSV32_REQ_OPEN_FILE;
                            cmd32->data.req_open_file.mode = EPOC32_MODE_SHARE_EXCLUSIVE | EPOC32_MODE_BINARY | EPOC32_MODE_READ_WRITE;
                            err = name_riscos_to_era(op->cmd->data.open.path,
                                                     cmd32->data.req_open_file.name,
                                                     sizeof(cmd32->data.req_open_file.name));
                        }
                        else err = &err_bad_parms;
                    }
                    if (!err) err = rfsv32_back(cmd32, reply32, op, unified_callback);
                }
                else
                {
                    if (!err)
                    {
                        const epoc16_mode mode[] =
                        {
                            EPOC16_MODE_OVERWRITE | EPOC16_MODE_BINARY_STREAM | EPOC16_MODE_READ_WRITE | EPOC16_MODE_RANDOM_ACCESS,
                            EPOC16_MODE_OPEN_EXISTING | EPOC16_MODE_BINARY_STREAM | EPOC16_MODE_RANDOM_ACCESS | EPOC16_MODE_SHARE,
                            EPOC16_MODE_OPEN_EXISTING | EPOC16_MODE_BINARY_STREAM | EPOC16_MODE_READ_WRITE | EPOC16_MODE_RANDOM_ACCESS
                        };
                        cmd16->op = RFSV16_RF_FOPEN;
                        cmd16->data.rf_fopen.mode = mode[op->cmd->data.open.mode];
                        err = name_riscos_to_sibo(op->cmd->data.open.path,
                                                  cmd16->data.rf_fopen.name,
                                                  sizeof(cmd16->data.rf_fopen.name));
                    }
                    if (!err) err = rfsv16_back(cmd16, reply16, op, unified_callback);
                }
                break;

            case UNIFIED_CLOSE:
                // Close a file
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_CLOSE_HANDLE;
                        cmd32->data.req_close_handle.handle = op->cmd->data.close.handle;
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_FCLOSE;
                        cmd16->data.rf_fclose.handle = op->cmd->data.close.handle;
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_SEEK:
                // Set the file pointer
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_SEEK_FILE;
                        cmd32->data.req_seek_file.handle = op->cmd->data.seek.handle;
                        cmd32->data.req_seek_file.offset = op->cmd->data.seek.offset;
                        cmd32->data.req_seek_file.sense = EPOC32_SENSE_ABSOLUTE;
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_FSEEK;
                        cmd16->data.rf_fseek.handle = op->cmd->data.seek.handle;
                        cmd16->data.rf_fseek.offset = op->cmd->data.seek.offset;
                        cmd16->data.rf_fseek.sense = EPOC16_SENSE_ABSOLUTE;
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_READ:
                // Read from a file
                op->length = 0;
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_READ_FILE;
                        cmd32->data.req_read_file.handle = op->cmd->data.read.handle;
                        cmd32->data.req_read_file.length = MIN(RFSV32_MAX_READ, op->cmd->data.read.length);
                        cmd32->data.req_read_file.buffer = op->cmd->data.read.buffer;
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_FREAD;
                        cmd16->data.rf_fread.handle = op->cmd->data.read.handle;
                        cmd16->data.rf_fread.length = MIN(RFSV16_MAX_READ, op->cmd->data.read.length);
                        cmd16->data.rf_fread.buffer = op->cmd->data.read.buffer;
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_WRITE:
                // Write to a file
                op->length = 0;
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_WRITE_FILE;
                        cmd32->data.req_write_file.handle = op->cmd->data.write.handle;
                        cmd32->data.req_write_file.length = MIN(RFSV32_MAX_WRITE, op->cmd->data.write.length);
                        cmd32->data.req_write_file.buffer = op->cmd->data.write.buffer;
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_FWRITE;
                        cmd16->data.rf_fwrite.handle = op->cmd->data.write.handle;
                        cmd16->data.rf_fwrite.length = MIN(RFSV16_MAX_WRITE, op->cmd->data.write.length);
                        cmd16->data.rf_fwrite.buffer = op->cmd->data.write.buffer;
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_ZERO:
                // Write zeros to a file
                op->length = 0;
                if (op->era)
                {
                    static byte buffer[RFSV32_MAX_WRITE];

                    if (!err)
                    {
                        memset(buffer, 0, sizeof(buffer));
                        cmd32->op = RFSV32_REQ_WRITE_FILE;
                        cmd32->data.req_write_file.handle = op->cmd->data.write.handle;
                        cmd32->data.req_write_file.length = MIN(RFSV32_MAX_WRITE, op->cmd->data.write.length);
                        cmd32->data.req_write_file.buffer = buffer;
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                }
                else
                {
                    static byte buffer[RFSV16_MAX_WRITE];

                    if (!err)
                    {
                        memset(buffer, 0, sizeof(buffer));
                        cmd16->op = RFSV16_RF_FWRITE;
                        cmd16->data.rf_fwrite.handle = op->cmd->data.write.handle;
                        cmd16->data.rf_fwrite.length = MIN(RFSV16_MAX_WRITE, op->cmd->data.write.length);
                        cmd16->data.rf_fwrite.buffer = buffer;
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_SIZE:
                // Set the size of a file
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_SET_SIZE;
                        cmd32->data.req_set_size.handle = op->cmd->data.size.handle;
                        cmd32->data.req_set_size.size = op->cmd->data.size.size;
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_FSETEOF;
                        cmd16->data.rf_fseteof.handle = op->cmd->data.size.handle;
                        cmd16->data.rf_fseteof.size = op->cmd->data.size.size;
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_FLUSH:
                // Flush a file
                if (op->era)
                {
                    if (!err)
                    {
                        cmd32->op = RFSV32_REQ_FLUSH;
                        cmd32->data.req_flush.handle = op->cmd->data.flush.handle;
                        err = rfsv32_back(cmd32, reply32, op, unified_callback);
                    }
                }
                else
                {
                    if (!err)
                    {
                        cmd16->op = RFSV16_RF_FFLUSH;
                        cmd16->data.rf_fflush.handle = op->cmd->data.flush.handle;
                        err = rfsv16_back(cmd16, reply16, op, unified_callback);
                    }
                }
                break;

            case UNIFIED_MACHINE:
                // Read the remote machine type
                if (!err)
                {
                    cmdncp->op = NCP_GET_MACHINE_TYPE;
                    err = ncp_back(cmdncp, replyncp, op, unified_callback);
                }
                break;

            case UNIFIED_TASKS:
                // Read the tasks with files open on a single drive
                if (!err)
                {
                    cmdncp->op = NCP_QUERY_DRIVE;
                    cmdncp->data.query_drive.drive = 'A';
                    cmdncp->data.query_drive.buffer = op->cmd->data.tasks.buffer;
                    cmdncp->data.query_drive.size = op->cmd->data.tasks.size;
                    err = ncp_back(cmdncp, replyncp, op, unified_callback);
                }
                break;

            case UNIFIED_DETAIL:
                // Read the command line for a task
                if (!err)
                {
                    cmdncp->op = NCP_GET_CMD_LINE;
                    if (sizeof(cmdncp->data.get_cmd_line.name)
                        <= strlen(op->cmd->data.detail.name))
                    {
                        err = &err_bad_name;
                    }
                    else
                    {
                        strcpy(cmdncp->data.get_cmd_line.name,
                               op->cmd->data.detail.name);
                    }
                }
                if (!err && !op->era) err = code_ansi_to_850(cmdncp->data.get_cmd_line.name, cmdncp->data.get_cmd_line.name);
                if (!err) err = ncp_back(cmdncp, replyncp, op, unified_callback);
                break;

            case UNIFIED_STOP:
                // Stop a task
                if (!err)
                {
op->index = 0;
                    op->delay = TRUE;
                    op->timeout = util_time() + UNIFIED_STOP_TIMEOUT;
                    cmdncp->op = NCP_STOP_PROGRAM;
                    if (sizeof(cmdncp->data.stop_program.name)
                        <= strlen(op->cmd->data.stop.name))
                    {
                        err = &err_bad_name;
                    }
                    else
                    {
                        strcpy(cmdncp->data.stop_program.name, op->cmd->data.stop.name);
                    }
                }
                if (!err && !op->era) err = code_ansi_to_850(cmdncp->data.stop_program.name, cmdncp->data.stop_program.name);
                if (!err) err = ncp_back(cmdncp, replyncp, op, unified_callback);
                break;

            case UNIFIED_START:
                // Start a task
                if (!err)
                {
                    op->delay = FALSE;
                    cmdncp->op = NCP_EXEC_PROGRAM;
                    if (op->era) err = name_riscos_to_era(op->cmd->data.start.name, cmdncp->data.exec_program.name, sizeof(cmdncp->data.exec_program.name));
                    else err = name_riscos_to_sibo(op->cmd->data.start.name, cmdncp->data.exec_program.name, sizeof(cmdncp->data.exec_program.name));
                    if (err && (strlen(op->cmd->data.start.name) < sizeof(cmdncp->data.exec_program.name)))
                    {
                        strcpy(cmdncp->data.exec_program.name, op->cmd->data.start.name);
                        if (op->era) err = NULL;
                        else err = code_ansi_to_850(cmdncp->data.exec_program.name, cmdncp->data.exec_program.name);
                    }
                }
                if (!err)
                {
                    if (op->era) err = name_riscos_to_era(op->cmd->data.start.args, cmdncp->data.exec_program.args, sizeof(cmdncp->data.exec_program.args));
                    else err = name_riscos_to_sibo(op->cmd->data.start.args, cmdncp->data.exec_program.args, sizeof(cmdncp->data.exec_program.args));
                    if (err && (strlen(op->cmd->data.start.args) < sizeof(cmdncp->data.exec_program.args)))
                    {
                        strcpy(cmdncp->data.exec_program.args, op->cmd->data.start.args);
                        if (op->era) err = NULL;
                        else err = code_ansi_to_850(cmdncp->data.exec_program.args, cmdncp->data.exec_program.args);
                    }
                }
                if (!err && op->cmd->data.start.action)
                {
                    if (sizeof(cmdncp->data.exec_program.args) <= strlen(cmdncp->data.exec_program.args) + 3)
                    {
                        err = &err_bad_name;
                    }
                    else
                    {
                        memmove(cmdncp->data.exec_program.args + 2, cmdncp->data.exec_program.args, strlen(cmdncp->data.exec_program.args) + 1);
                        cmdncp->data.exec_program.args[0] = op->cmd->data.start.action;
                        cmdncp->data.exec_program.args[1] = '\"';
                        strcat(cmdncp->data.exec_program.args, "\"");
                    }
                }
                if (!err) err = ncp_back(cmdncp, replyncp, op, unified_callback);
                break;

            case UNIFIED_POWER:
                // Read power supply details
                if (!err)
                {
                    if (op->era)
                    {
                        cmdncp->op = NCP_GET_MACHINE_INFO;
                        err = ncp_back(cmdncp, replyncp, op, unified_callback);
                    }
                    else err = &err_bad_unified_op;
                }
                break;

            case UNIFIED_RTIME:
                // Read system time
                if (!err)
                {
                    if (op->era)
                    {
                        cmdncp->op = NCP_GET_MACHINE_INFO;
                        err = ncp_back(cmdncp, replyncp, op, unified_callback);
                    }
                    else err = &err_bad_unified_op;
                }
                break;

            case UNIFIED_WTIME:
                // Write system time
                if (!err)
                {
                    if (op->era)
                    {
                        cmdncp->op = NCP_GET_MACHINE_INFO;
                        err = ncp_back(cmdncp, replyncp, op, unified_callback);
                    }
                    else err = &err_bad_unified_op;
                }
                break;

            case UNIFIED_OWNER:
                // Read owner information
                if (!err)
                {
                    cmdncp->op = NCP_GET_OWNER_INFO;
                    err = ncp_back(cmdncp, replyncp, op, unified_callback);
                }
                break;

            default:
                // Not a supported command
                err = &err_bad_unified_op;
                break;
        }

        // Pass any error to the callback function
        if (err) err = unified_done(op, err);
        else if (done) err = unified_done(op, NULL);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a link operation.
*/
static os_error *unified_fore_callback(void *user, os_error *err,
                                       const void *reply)
{
    // Store the status
    unified_fore_done = TRUE;
    unified_fore_err = err;

    // Clear any error status
    err = NULL;

    // Return any error produced
    return err;
}

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
                       bool escape)
{
    os_error *err = NULL;

    // Clear the pending done flag
    unified_fore_done = FALSE;

    // Start as a background operation
    err = unified_back(cmd, reply, NULL, unified_fore_callback);

    // Process the operation in the foreground
    while (!err && !unified_fore_done)
    {
        // Poll the various layers
        err = link_poll(escape);
    }

    // Preserve any returned error
    if (!err && unified_fore_done) err = unified_fore_err;

    // Return any error produced
    return err;
}

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
                       void *user, share_callback callback)
{
    os_error *err = NULL;

    // Check function parameters
    if (!cmd || !reply || !callback) err = &err_bad_parms;
    else
    {
        unified_private *ptr;

        // Obtain an operation record pointer
        if (unified_free_list)
        {
            // A suitable record already exists
            ptr = unified_free_list;
            if (ptr->next) ptr->next->prev = NULL;
            unified_free_list = ptr->next;
        }
        else
        {
            // Allocate a new structure
            ptr = (unified_private *) MEM_MALLOC(sizeof(unified_private));
            if (!ptr) err = &err_buffer;
        }

        // Complete the details
        if (!err)
        {
            // Add to the active list
            ptr->prev = NULL;
            ptr->next = unified_active_list;
            if (unified_active_list) unified_active_list->prev = ptr;
            unified_active_list = ptr;

            // Copy the command details
            ptr->cmd = cmd;
            ptr->reply = reply;
            ptr->user = user;
            ptr->callback = callback;

            // Start the operation
            err = unified_begin(ptr);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required.
*/
os_error *unified_poll(void)
{
    os_error *err = NULL;

    // No action unless active
    if (unified_active)
    {
        // Update the connection status
        err = unified_update();

        // Poll the timer layer
        if (!err) err = timer_poll();

        // Poll higher layers if connected
        if (!err && unified_connected) err = cache_poll();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : era           - Is an ERA device connected.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the unified server layer after the multiplexor layer
                  has been started.
*/
os_error *unified_start(bool era)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting unified server layer"))

    // No action if already active
    if (!unified_active)
    {
        // Start servers
        err = timer_start();
        if (!err) err = era ? rfsv32_start() : rfsv16_start();

        // Set the active flag if successful
        if (!err) unified_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the unified server layer before the multiplexor layer has
                  been closed.
*/
os_error *unified_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending unified server layer now=%u", now))

    // No action if unless active
    if (unified_active)
    {
        // End higher levels
        err = unified_disconnect(now);

        // End servers
        if (!err) err = ncp_end(now);
        if (!err) err = rfsv32_end(now);
        if (!err) err = rfsv16_end(now);
        if (!err) err = timer_end(now);

        // Free any allocated memory
        if (!err) err = unified_free();

        // Clear the connected flags if successful
        if (!err)
        {
            unified_partial = FALSE;
            unified_connected = FALSE;
        }

        // Clear the active flag if successful
        if (!err) unified_active = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the unified server layer.
*/
os_error *unified_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying unified server layer status"))

    // Display the current connection status
    if (unified_connected)
    {
        printf("Connected to all necessary %s servers.\n",
               unified_era ? "EPOC" : "SIBO");
        err = cache_status();
        if (!err) err = upload_status();
    }
    else if (unified_partial)
    {
        printf("Connected to the %s remote file server only.\n",
               unified_era ? "EPOC" : "SIBO");
        err = upload_status();
    }
    else if (unified_active)
    {
        printf("Not connected to all necessary remote servers.\n");
    }
    else printf("High level link layers not active.\n");

    // Return any error produced
    return err;
}
