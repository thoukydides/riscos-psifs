/*
    File        : err.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Errors that may be produced by the PsiFS module.

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
#ifndef ERR_H
#define ERR_H

// Include project header files
#include "psifs.h"

// Macro to test error blocks for equality
#define ERR_EQ(a,b) ((a).errnum == (b).errnum)

// Macro to declare the error blocks
#ifndef ERR_BLOCK
#define ERR_BLOCK(v,n,t) extern os_error v;
#endif

// General error blocks
ERR_BLOCK(err_no_driver,        NO_DRIVER,          "Block driver not found")
ERR_BLOCK(err_block_driver,     BLOCK_DRIVER,       "Block driver error")
ERR_BLOCK(err_driver_size,      DRIVER_SIZE,        "Block driver larger than 4kB - module workspace may be corrupt")
ERR_BLOCK(err_driver_full,      DRIVER_FULL,        "Block driver buffer full")
ERR_BLOCK(err_link_busy,        LINK_BUSY,          "Block driver is in use")
ERR_BLOCK(err_no_link,          NO_LINK,            "Block driver not active")
ERR_BLOCK(err_no_frame,         NO_FRAME,           "Frame handler not active")
ERR_BLOCK(err_no_connect,       NO_CONNECT,         "Connection manager not active")
ERR_BLOCK(err_no_mux,           NO_MUX,             "Multiplexor and fragmentation layer not active")
ERR_BLOCK(err_not_connected,    NOT_CONNECTED,      "Not connected")
ERR_BLOCK(err_not_poll,         NOT_POLL,           "Not within a poll")
ERR_BLOCK(err_connection_busy,  CONNECTION_BUSY,    "Connection is busy")
ERR_BLOCK(err_chan_exists,      CHAN_EXISTS,        "Channel already exists")
ERR_BLOCK(err_mux_full,         MUX_FULL,           "Pending frame queue full")
ERR_BLOCK(err_svr_none,         SVR_NONE,           "Server channel not connected")
ERR_BLOCK(err_svr_closed,       SVR_CLOSED,         "Server channel disconnected")
ERR_BLOCK(err_svr_time,         SVR_TIME,           "Server time out")
ERR_BLOCK(err_bad_buffer,       BAD_BUFFER,         "Bad buffer")
ERR_BLOCK(err_buffer_full,      BUFFER_FULL,        "Buffer full")
ERR_BLOCK(err_buffer_end,       BUFFER_END,         "End of buffer reached")
ERR_BLOCK(err_bad_frame_state,  BAD_FRAME_STATE,    "Invalid frame state")
ERR_BLOCK(err_bad_connect_state,BAD_CONNECT_STATE,  "Invalid connection state")
ERR_BLOCK(err_bad_ncp_event,    BAD_NCP_EVENT,      "Invalid NCP event")
ERR_BLOCK(err_bad_ncp_op,       BAD_NCP_OP,         "Unsupported NCP operation")
ERR_BLOCK(err_ncp_len,          NCP_LEN,            "NCP string too long")
ERR_BLOCK(err_ncp_too_many,     NCP_TOO_MANY,       "Too many applications")
ERR_BLOCK(err_bad_rfsv_op,      BAD_RFSV_OP,        "Unsupported RFSV operation")
ERR_BLOCK(err_not_rfsv_reply,   NOT_RFSV_REPLY,     "Not a RFSV reply")
ERR_BLOCK(err_bad_rfsv_reply,   BAD_RFSV_REPLY,     "Mismatched RFSV reply")
ERR_BLOCK(err_rfsv_len,         RFSV_LEN,           "RFSV string too long")
ERR_BLOCK(err_rfsv_str,         RFSV_STR,           "Unsupported RFSV string type")
ERR_BLOCK(err_rfsv_too_many,    RFSV_TOO_MANY,      "Too many directory entries")
ERR_BLOCK(err_bad_unified_op,   BAD_UNIFIED_OP,     "Unsupported unified file operation")
ERR_BLOCK(err_bad_cache_op,     BAD_CACHE_OP,       "Unsupported cached file operation")
ERR_BLOCK(err_timeout,          TIMEOUT,            "Operation timed out")
ERR_BLOCK(err_comms,            COMMS,              "Communications failure")
ERR_BLOCK(err_cache_inactive,   CACHE_INACTIVE,     "Connection not active")
ERR_BLOCK(err_cache_busy,       CACHE_BUSY,         "Connection is busy")
ERR_BLOCK(err_not_link_reply,   NOT_LINK_REPLY,     "Not a LINK reply")
ERR_BLOCK(err_bad_link_reply,   BAD_LINK_REPLY,     "Mismatched LINK reply")
ERR_BLOCK(err_link_len,         LINK_LEN,           "LINK name too long")
ERR_BLOCK(err_bad_uid,          BAD_UID,            "Invalid file UID")
ERR_BLOCK(err_bad_ext,          BAD_EXT,            "Invalid file extension")
ERR_BLOCK(err_bad_upload_op,    BAD_UPLOAD_OP,      "Unsupported upload operation")
ERR_BLOCK(err_bad_async_handle, BAD_ASYNC_HANDLE,   "Invalid asynchronous operation handle")
ERR_BLOCK(err_bad_async_op,     BAD_ASYNC_OP,       "Unsupported asynchronous operation")
ERR_BLOCK(err_bad_async_state,  BAD_ASYNC_STATE,    "Invalid asynchronous operation state")
ERR_BLOCK(err_bad_tar_checksum, BAD_TAR_CHECKSUM,   "Bad tar header checksum")
ERR_BLOCK(err_bad_tar_header,   BAD_TAR_HEADER,     "Bad format tar header")
ERR_BLOCK(err_tar_eof,          TAR_EOF,            "Unexpected end of tar file")
ERR_BLOCK(err_bad_tar_op,       BAD_TAR_OP,         "Unsupported tar file operation")
ERR_BLOCK(err_remote_unknown,   REMOTE_UNKNOWN,     "Unknown error received from remote system")
ERR_BLOCK(err_remote_general,   REMOTE_GENERAL,     "General remote system error")
ERR_BLOCK(err_remote_os,        REMOTE_OS,          "Remote operating system error")
ERR_BLOCK(err_remote_not_sup,   REMOTE_NOT_SUP,     "Remote system does not support operation")
ERR_BLOCK(err_remote_in_use,    REMOTE_IN_USE,      "Remote system in use")
ERR_BLOCK(err_remote_no_memory, REMOTE_NO_MEMORY,   "Remote system out of memory")
ERR_BLOCK(err_remote_fs,        REMOTE_FS,          "Remote file system error")
ERR_BLOCK(err_remote_not_ready, REMOTE_NOT_READY,   "Remote system not ready")
ERR_BLOCK(err_remote_cancel,    REMOTE_CANCEL,      "Remote system cancelled operation")
ERR_BLOCK(err_remote_discon,    REMOTE_DISCON,      "Remote system disconnected")
ERR_BLOCK(err_remote_no_con,    REMOTE_NO_CON,      "Remote system failed to connect")
ERR_BLOCK(err_remote_abort,     REMOTE_ABORT,       "Remote system aborted operation")
ERR_BLOCK(err_remote_power,     REMOTE_POWER,       "Remote system power failure")
ERR_BLOCK(err_kill_clients,     KILL_CLIENTS,       "PsiFS module cannot be killed while clients are registered")
ERR_BLOCK(err_bad_intercept_handle,BAD_INTERCEPT_HANDLE,"Invalid file transfer intercept handle")
ERR_BLOCK(err_bad_intercept_type,BAD_INTERCEPT_TYPE,"Invalid file transfer intercept type")
ERR_BLOCK(err_bad_intercept_msg,BAD_INTERCEPT_MSG,  "Unexpected message during file transfer intercept")
ERR_BLOCK(err_intercept_died,   INTERCEPT_DIED,     "Receiver died during file transfer intercept")
ERR_BLOCK(err_bad_intercept_state,BAD_INTERCEPT_STATE,"Invalid file transfer intercept state")
ERR_BLOCK(err_intercept_run,    INTERCEPT_RUN,      "No active application can load this type of file. Run a suitable application and try again.")
ERR_BLOCK(err_lang_unknown,     LANG_UNKNOWN,       "Unknown language")
ERR_BLOCK(err_too_many_lang,    TOO_MANY_LANG,      "Too many languages")
ERR_BLOCK(err_sis_write_outside,SIS_WRITE_OUTSIDE,  "Attempt to write outside SIS file")
ERR_BLOCK(err_sis_read_outside, SIS_READ_OUTSIDE,   "Attempt to read outside SIS file")
ERR_BLOCK(err_bad_sis_header,   BAD_SIS_HEADER,     "Bad format SIS file header")
ERR_BLOCK(err_bad_sis_type,     BAD_SIS_TYPE,       "Unsupported SIS file type")
ERR_BLOCK(err_bad_sis_checksum, BAD_SIS_CHECKSUM,   "Incorrect SIS file checksum")
ERR_BLOCK(err_clipboard_not_active,CLIPBOARD_NOT_ACTIVE,"Clipboard server is not active")
ERR_BLOCK(err_clipboard_not_sync,CLIPBOARD_NOT_SYNC,"Clipboard file is not synchronized")
ERR_BLOCK(err_clipboard_state,  CLIPBOARD_STATE,    "Invalid clipboard state")
ERR_BLOCK(err_print_job_no_page,PRINT_JOB_NO_PAGE,  "No page data available for print job")

// File system error blocks
ERR_BLOCK(err_eof,              EOF,                "End of file")
ERR_BLOCK(err_ext_escape,       EXT_ESCAPE,         "Escape")
//ERR_BLOCK(err_cant_del_csd,     CANT_DEL_CSD,       "Can't delete current directory")
//ERR_BLOCK(err_cant_del_lib,     CANT_DEL_LIB,       "Can't delete library")
ERR_BLOCK(err_bad_disc,         BAD_DISC,           "Disc not formatted")
//ERR_BLOCK(err_too_many_discs,   TOO_MANY_DISCS,     "Too many discs")
//ERR_BLOCK(err_bad_up,           BAD_UP,             "Illegal use of ^")
ERR_BLOCK(err_ambig_disc,       AMBIG_DISC,         "Ambiguous disc name")
//ERR_BLOCK(err_not_ref_disc,     NOT_REF_DISC,       "Not same disc")
ERR_BLOCK(err_in_use,           IN_USE,             "PsiFS in use")
ERR_BLOCK(err_bad_parms,        BAD_PARMS,          "Bad parameters")
//ERR_BLOCK(err_cant_del_urd,     CANT_DEL_URD,       "Can't delete user root directory")
ERR_BLOCK(err_buffer,           BUFFER,             "No room for buffer")
//ERR_BLOCK(err_workspace,        WORKSPACE,          "PsiFS workspace corrupt")
//ERR_BLOCK(err_multiple_close,   MULTIPLE_CLOSE,     "Multiple file closing errors")
ERR_BLOCK(err_bad_drive,        BAD_DRIVE,          "Bad drive")
ERR_BLOCK(err_bad_rename,       BAD_RENAME,         "Bad rename")
ERR_BLOCK(err_dir_full,         DIR_FULL,           "Directory full")
ERR_BLOCK(err_dir_not_empty,    DIR_NOT_EMPTY,      "Directory not empty")
ERR_BLOCK(err_outside,          OUTSIDE,            "Outside file")
ERR_BLOCK(err_access,           ACCESS,             "Access violation")
//ERR_BLOCK(err_too_many_open,    TOO_MANY_OPEN,      "Too many open files")
ERR_BLOCK(err_open,             OPEN,               "File open")
ERR_BLOCK(err_locked,           LOCKED,             "Locked")
ERR_BLOCK(err_exists,           EXISTS,             "Already exists")
ERR_BLOCK(err_types,            TYPES,              "Types don't match")
ERR_BLOCK(err_disc_full,        DISC_FULL,          "Disc full")
//ERR_BLOCK(err_disc,             DISC,               "Disc error")
ERR_BLOCK(err_write_prot,       WRITE_PROT,         "Protected disc")
//ERR_BLOCK(err_data_lost,        DATA_LOST,          "Three things are certain: Death, taxes, and lost data. Guess which has occurred.")
ERR_BLOCK(err_bad_name,         BAD_NAME,           "Bad name")
//ERR_BLOCK(err_bad_att,          BAD_ATT,            "Bad attribute")
ERR_BLOCK(err_drive_empty,      DRIVE_EMPTY,        "Drive empty")
ERR_BLOCK(err_disc_not_found,   DISC_NOT_FOUND,     "Disc not found")
//ERR_BLOCK(err_disc_not_present, DISC_NOT_PRESENT,"  Disc not present")
ERR_BLOCK(err_not_found,        NOT_FOUND,          "Not found")
ERR_BLOCK(err_channel,          CHANNEL,            "Channel")
ERR_BLOCK(err_not_supported,    NOT_SUPPORTED,      "Bad operation on PsiFS")
//ERR_BLOCK(err_fs_write_only,    FS_WRITE_ONLY,      "PsiFS is a write only filing system")
//ERR_BLOCK(err_fs_read_only,     FS_READ_ONLY,       "PsiFS is a read only filing system")
ERR_BLOCK(err_wild_cards,       WILD_CARDS,         "Wild cards")
//ERR_BLOCK(err_bad_com,          BAD_COM,            "Bad command")

#endif
