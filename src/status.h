/*
    File        : status.h
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

// Only include header file once
#ifndef STATUS_H
#define STATUS_H

// Include oslib header files
#include "oslib/os.h"

// A type for status codes
typedef signed char status_code;

// SIBO status codes
#define STATUS_SIBO_NONE ((status_code) 0)
#define STATUS_SIBO_GEN_FAIL ((status_code) -1)
#define STATUS_SIBO_GEN_ARG ((status_code) -2)
#define STATUS_SIBO_GEN_OS ((status_code) -3)
#define STATUS_SIBO_GEN_NSUP ((status_code) -4)
#define STATUS_SIBO_GEN_UNDER ((status_code) -5)
#define STATUS_SIBO_GEN_OVER ((status_code) -6)
#define STATUS_SIBO_GEN_RANGE ((status_code) -7)
#define STATUS_SIBO_GEN_DIVIDE ((status_code) -8)
#define STATUS_SIBO_GEN_INUSE ((status_code) -9)
#define STATUS_SIBO_GEN_NOMEMORY ((status_code) -10)
#define STATUS_SIBO_GEN_NOSEGMENTS ((status_code) -11)
#define STATUS_SIBO_GEN_NOSEM ((status_code) -12)
#define STATUS_SIBO_GEN_NOPROC ((status_code) -13)
#define STATUS_SIBO_GEN_OPEN ((status_code) -14)
#define STATUS_SIBO_GEN_NOTOPEN ((status_code) -15)
#define STATUS_SIBO_GEN_IMAGE ((status_code) -16)
#define STATUS_SIBO_GEN_RECEIVER ((status_code) -17)
#define STATUS_SIBO_GEN_DEVICE ((status_code) -18)
#define STATUS_SIBO_GEN_FSYS ((status_code) -19)
#define STATUS_SIBO_GEN_START ((status_code) -20)
#define STATUS_SIBO_GEN_NOFONT ((status_code) -21)
#define STATUS_SIBO_GEN_TOOWIDE ((status_code) -22)
#define STATUS_SIBO_GEN_TOOMANY ((status_code) -23)
#define STATUS_SIBO_FILE_EXIST ((status_code) -32)
#define STATUS_SIBO_FILE_NXIST ((status_code) -33)
#define STATUS_SIBO_FILE_WRITE ((status_code) -34)
#define STATUS_SIBO_FILE_READ ((status_code) -35)
#define STATUS_SIBO_FILE_EOF ((status_code) -36)
#define STATUS_SIBO_FILE_FULL ((status_code) -37)
#define STATUS_SIBO_FILE_NAME ((status_code) -38)
#define STATUS_SIBO_FILE_ACCESS ((status_code) -39)
#define STATUS_SIBO_FILE_LOCKED ((status_code) -40)
#define STATUS_SIBO_FILE_DEVICE ((status_code) -41)
#define STATUS_SIBO_FILE_DIR ((status_code) -42)
#define STATUS_SIBO_FILE_RECORD ((status_code) -43)
#define STATUS_SIBO_FILE_RDONLY ((status_code) -44)
#define STATUS_SIBO_FILE_INV ((status_code) -45)
#define STATUS_SIBO_FILE_PENDING ((status_code) -46)
#define STATUS_SIBO_FILE_VOLUME ((status_code) -47)
#define STATUS_SIBO_FILE_CANCEL ((status_code) -48)
#define STATUS_SIBO_FILE_DISC ((status_code) -50)
#define STATUS_SIBO_FILE_CONNECT ((status_code) -51)
#define STATUS_SIBO_FILE_RETRAN ((status_code) -52)
#define STATUS_SIBO_FILE_LINE ((status_code) -53)
#define STATUS_SIBO_FILE_INACT ((status_code) -54)
#define STATUS_SIBO_FILE_PARITY ((status_code) -55)
#define STATUS_SIBO_FILE_FRAME ((status_code) -56)
#define STATUS_SIBO_FILE_OVERRUN ((status_code) -57)
#define STATUS_SIBO_MDM_CONFAIL ((status_code) -58)
#define STATUS_SIBO_MDM_BUSY ((status_code) -59)
#define STATUS_SIBO_MDM_NOANS ((status_code) -60)
#define STATUS_SIBO_MDM_BLACKLIST ((status_code) -61)
#define STATUS_SIBO_FILE_NOTREADY ((status_code) -62)
#define STATUS_SIBO_FILE_UNKNOWN ((status_code) -63)
#define STATUS_SIBO_FILE_DIRFULL ((status_code) -64)
#define STATUS_SIBO_FILE_PROTECT ((status_code) -65)
#define STATUS_SIBO_FILE_CORRUPT ((status_code) -66)
#define STATUS_SIBO_FILE_ABORT ((status_code) -67)
#define STATUS_SIBO_FILE_ERASE ((status_code) -68)
#define STATUS_SIBO_FILE_INVALID ((status_code) -69)

// ERA status codes
#define STATUS_ERA_NONE ((status_code) 0)
#define STATUS_ERA_NOT_FOUND ((status_code) -1)
#define STATUS_ERA_GENERAL ((status_code) -2)
#define STATUS_ERA_CANCEL ((status_code) -3)
#define STATUS_ERA_NO_MEMORY ((status_code) -4)
#define STATUS_ERA_NOT_SUPPORTED ((status_code) -5)
#define STATUS_ERA_ARGUMENT ((status_code) -6)
#define STATUS_ERA_TOTAL_LOSS_OF_PRECISION ((status_code) -7)
#define STATUS_ERA_BAD_HANDLE ((status_code) -8)
#define STATUS_ERA_OVERFLOW ((status_code) -9)
#define STATUS_ERA_UNDERFLOW ((status_code) -10)
#define STATUS_ERA_ALREADY_EXISTS ((status_code) -11)
#define STATUS_ERA_PATH_NOT_FOUND ((status_code) -12)
#define STATUS_ERA_DIED ((status_code) -13)
#define STATUS_ERA_IN_USE ((status_code) -14)
#define STATUS_ERA_SERVER_TERMINATED ((status_code) -15)
#define STATUS_ERA_SERVER_BUSY ((status_code) -16)
#define STATUS_ERA_COMPLETION ((status_code) -17)
#define STATUS_ERA_NOT_READY ((status_code) -18)
#define STATUS_ERA_UNKNOWN ((status_code) -19)
#define STATUS_ERA_CORRUPT ((status_code) -20)
#define STATUS_ERA_ACCESS_DENIED ((status_code) -21)
#define STATUS_ERA_LOCKED ((status_code) -22)
#define STATUS_ERA_WRITE ((status_code) -23)
#define STATUS_ERA_DISMOUNTED ((status_code) -24)
#define STATUS_ERA_EOF ((status_code) -25)
#define STATUS_ERA_DISK_FULL ((status_code) -26)
#define STATUS_ERA_BAD_DRIVER ((status_code) -27)
#define STATUS_ERA_BAD_NAME ((status_code) -28)
#define STATUS_ERA_COMMS_LINE_FAIL ((status_code) -29)
#define STATUS_ERA_COMMS_FRAME ((status_code) -30)
#define STATUS_ERA_COMMS_OVERRUN ((status_code) -31)
#define STATUS_ERA_COMMS_PARITY ((status_code) -32)
#define STATUS_ERA_TIMEOUT ((status_code) -33)
#define STATUS_ERA_COULD_NOT_CONNECT ((status_code) -34)
#define STATUS_ERA_COULD_NOT_DISCONNECT ((status_code) -35)
#define STATUS_ERA_DISCONNECTED ((status_code) -36)
#define STATUS_ERA_BAD_LIBRARY_ENTRY_POINT ((status_code) -37)
#define STATUS_ERA_BAD_DESCRIPTOR ((status_code) -38)
#define STATUS_ERA_ABORT ((status_code) -39)
#define STATUS_ERA_TOO_BIG ((status_code) -40)
#define STATUS_ERA_DIVIDE_BY_ZERO ((status_code) -41)
#define STATUS_ERA_BAD_POWER ((status_code) -42)
#define STATUS_ERA_DIR_FULL ((status_code) -43)

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : status        - The SIBO status code.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a SIBO status to a RISC OS error.
*/
os_error *status_sibo_err(status_code status);

/*
    Parameters  : status        - The ERA status code.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a ERA status to a RISC OS error.
*/
os_error *status_era_err(status_code status);

#ifdef __cplusplus
    }
#endif

#endif
