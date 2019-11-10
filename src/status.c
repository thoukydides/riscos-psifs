/*
    File        : status.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Psion status and error mapping for the PsiFS module.

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
#include "status.h"

// Include project header files
#include "err.h"

/*
    Parameters  : status        - The SIBO status code.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a SIBO status to a RISC OS error.
*/
os_error *status_sibo_err(status_code status)
{
    os_error *err;

    // Perform the mapping
    switch (status)
    {
    case STATUS_SIBO_NONE:                   err = NULL;                  break;
    case STATUS_SIBO_GEN_FAIL:               err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_ARG:                err = &err_bad_parms;        break;
    case STATUS_SIBO_GEN_OS:                 err = &err_remote_os;        break;
    case STATUS_SIBO_GEN_NSUP:               err = &err_remote_not_sup;   break;
    case STATUS_SIBO_GEN_UNDER:              err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_OVER:               err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_RANGE:              err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_DIVIDE:             err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_INUSE:              err = &err_open;             break;
    case STATUS_SIBO_GEN_NOMEMORY:           err = &err_remote_no_memory; break;
    case STATUS_SIBO_GEN_NOSEGMENTS:         err = &err_remote_no_memory; break;
    case STATUS_SIBO_GEN_NOSEM:              err = &err_remote_no_memory; break;
    case STATUS_SIBO_GEN_NOPROC:             err = &err_remote_no_memory; break;
    case STATUS_SIBO_GEN_OPEN:               err = &err_exists;           break;
    case STATUS_SIBO_GEN_NOTOPEN:            err = &err_not_found;        break;
    case STATUS_SIBO_GEN_IMAGE:              err = &err_bad_disc;         break;
    case STATUS_SIBO_GEN_RECEIVER:           err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_DEVICE:             err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_FSYS:               err = &err_remote_fs;        break;
    case STATUS_SIBO_GEN_START:              err = &err_remote_not_ready; break;
    case STATUS_SIBO_GEN_NOFONT:             err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_TOOWIDE:            err = &err_remote_general;   break;
    case STATUS_SIBO_GEN_TOOMANY:            err = &err_remote_general;   break;
    case STATUS_SIBO_FILE_EXIST:             err = &err_exists;           break;
    case STATUS_SIBO_FILE_NXIST:             err = &err_not_found;        break;
    case STATUS_SIBO_FILE_WRITE:             err = &err_write_prot;       break;
    case STATUS_SIBO_FILE_READ:              err = &err_access;           break;
    case STATUS_SIBO_FILE_EOF:               err = &err_eof;              break;
    case STATUS_SIBO_FILE_FULL:              err = &err_disc_full;        break;
    case STATUS_SIBO_FILE_NAME:              err = &err_bad_name;         break;
    case STATUS_SIBO_FILE_ACCESS:            err = &err_access;           break;
    case STATUS_SIBO_FILE_LOCKED:            err = &err_locked;           break;
    case STATUS_SIBO_FILE_DEVICE:            err = &err_remote_not_ready; break;
    case STATUS_SIBO_FILE_DIR:               err = &err_not_found;        break;
    case STATUS_SIBO_FILE_RECORD:            err = &err_bad_name;         break;
    case STATUS_SIBO_FILE_RDONLY:            err = &err_access;           break;
    case STATUS_SIBO_FILE_INV:               err = &err_bad_disc;         break;
    case STATUS_SIBO_FILE_PENDING:           err = &err_remote_not_ready; break;
    case STATUS_SIBO_FILE_VOLUME:            err = &err_bad_name;         break;
    case STATUS_SIBO_FILE_CANCEL:            err = &err_remote_cancel;    break;
    case STATUS_SIBO_FILE_DISC:              err = &err_remote_discon;    break;
    case STATUS_SIBO_FILE_CONNECT:           err = &err_remote_no_con;    break;
    case STATUS_SIBO_FILE_RETRAN:            err = &err_timeout;          break;
    case STATUS_SIBO_FILE_LINE:              err = &err_comms;            break;
    case STATUS_SIBO_FILE_INACT:             err = &err_timeout;          break;
    case STATUS_SIBO_FILE_PARITY:            err = &err_comms;            break;
    case STATUS_SIBO_FILE_FRAME:             err = &err_comms;            break;
    case STATUS_SIBO_FILE_OVERRUN:           err = &err_comms;            break;
    case STATUS_SIBO_MDM_CONFAIL:            err = &err_remote_general;   break;
    case STATUS_SIBO_MDM_BUSY:               err = &err_remote_general;   break;
    case STATUS_SIBO_MDM_NOANS:              err = &err_remote_general;   break;
    case STATUS_SIBO_MDM_BLACKLIST:          err = &err_remote_general;   break;
    case STATUS_SIBO_FILE_NOTREADY:          err = &err_drive_empty;      break;
    case STATUS_SIBO_FILE_UNKNOWN:           err = &err_not_found;        break;
    case STATUS_SIBO_FILE_DIRFULL:           err = &err_dir_full;         break;
    case STATUS_SIBO_FILE_PROTECT:           err = &err_write_prot;       break;
    case STATUS_SIBO_FILE_CORRUPT:           err = &err_bad_disc;         break;
    case STATUS_SIBO_FILE_ABORT:             err = &err_remote_abort;     break;
    case STATUS_SIBO_FILE_ERASE:             err = &err_remote_general;   break;
    case STATUS_SIBO_FILE_INVALID:           err = &err_bad_disc;         break;
    default:                                 err = &err_remote_unknown;   break;
    }

    // Return the result
    return err;
}

/*
    Parameters  : status        - The ERA status code.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a ERA status to a RISC OS error.
*/
os_error *status_era_err(status_code status)
{
    os_error *err;

    // Perform the mapping
    switch (status)
    {
    case STATUS_ERA_NONE:                    err = NULL;                  break;
    case STATUS_ERA_NOT_FOUND:               err = &err_not_found;        break;
    case STATUS_ERA_GENERAL:                 err = &err_remote_general;   break;
    case STATUS_ERA_CANCEL:                  err = &err_remote_cancel;    break;
    case STATUS_ERA_NO_MEMORY:               err = &err_remote_no_memory; break;
    case STATUS_ERA_NOT_SUPPORTED:           err = &err_remote_not_sup;   break;
    case STATUS_ERA_ARGUMENT:                err = &err_bad_parms;        break;
    case STATUS_ERA_TOTAL_LOSS_OF_PRECISION: err = &err_remote_general;   break;
    case STATUS_ERA_BAD_HANDLE:              err = &err_channel;          break;
    case STATUS_ERA_OVERFLOW:                err = &err_remote_general;   break;
    case STATUS_ERA_UNDERFLOW:               err = &err_remote_general;   break;
    case STATUS_ERA_ALREADY_EXISTS:          err = &err_exists;           break;
    case STATUS_ERA_PATH_NOT_FOUND:          err = &err_not_found;        break;
    case STATUS_ERA_DIED:                    err = &err_remote_general;   break;
    case STATUS_ERA_IN_USE:                  err = &err_remote_in_use;    break;
    case STATUS_ERA_SERVER_TERMINATED:       err = &err_remote_general;   break;
    case STATUS_ERA_SERVER_BUSY:             err = &err_remote_general;   break;
    case STATUS_ERA_COMPLETION:              err = &err_remote_general;   break;
    case STATUS_ERA_NOT_READY:               err = &err_drive_empty;      break;
    case STATUS_ERA_UNKNOWN:                 err = &err_not_found;        break;
    case STATUS_ERA_CORRUPT:                 err = &err_bad_disc;         break;
    case STATUS_ERA_ACCESS_DENIED:           err = &err_access;           break;
    case STATUS_ERA_LOCKED:                  err = &err_locked;           break;
    case STATUS_ERA_WRITE:                   err = &err_write_prot;       break;
    case STATUS_ERA_DISMOUNTED:              err = &err_remote_fs;        break;
    case STATUS_ERA_EOF:                     err = &err_eof;              break;
    case STATUS_ERA_DISK_FULL:               err = &err_disc_full;        break;
    case STATUS_ERA_BAD_DRIVER:              err = &err_remote_general;   break;
    case STATUS_ERA_BAD_NAME:                err = &err_bad_name;         break;
    case STATUS_ERA_COMMS_LINE_FAIL:         err = &err_comms;            break;
    case STATUS_ERA_COMMS_FRAME:             err = &err_comms;            break;
    case STATUS_ERA_COMMS_OVERRUN:           err = &err_comms;            break;
    case STATUS_ERA_COMMS_PARITY:            err = &err_comms;            break;
    case STATUS_ERA_TIMEOUT:                 err = &err_timeout;          break;
    case STATUS_ERA_COULD_NOT_CONNECT:       err = &err_remote_no_con;    break;
    case STATUS_ERA_COULD_NOT_DISCONNECT:    err = &err_remote_discon;    break;
    case STATUS_ERA_DISCONNECTED:            err = &err_remote_discon;    break;
    case STATUS_ERA_BAD_LIBRARY_ENTRY_POINT: err = &err_remote_os;        break;
    case STATUS_ERA_BAD_DESCRIPTOR:          err = &err_remote_general;   break;
    case STATUS_ERA_ABORT:                   err = &err_remote_abort;     break;
    case STATUS_ERA_TOO_BIG:                 err = &err_disc_full;        break;
    case STATUS_ERA_DIVIDE_BY_ZERO:          err = &err_remote_general;   break;
    case STATUS_ERA_BAD_POWER:               err = &err_remote_power;     break;
    case STATUS_ERA_DIR_FULL:                err = &err_dir_full;         break;
    default:                                 err = &err_remote_unknown;   break;
    }

    // Return the result
    return err;
}
