/*
    File        : uid.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : UID and extension conversion for the PsiFS module.

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
#ifndef UID_H
#define UID_H

// Include oslib header files
#include "oslib/osfile.h"

// Include project header files
#include "epoc32.h"

// Special file type value used to clear a mapping
#define UID_CLEAR_TYPE (osfile_TYPE_UNTYPED)

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : uid           - The UID to check.
                  uid4          - Variable to receive the checksum.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Calculate the checksum for a UID.
*/
os_error *uid_checksum_uid(const epoc32_file_uid *uid, bits *uid4);

/*
    Parameters  : name  - The filename in RISC OS format.
                  uid   - The UID to map.
    Returns     : bits  - The corresponding file type.
    Description : Convert the specified filename and file UID into a RISC OS
                  file type. If a valid UID is specified then that is used,
                  otherwise the extension is processed.
*/
bits uid_map_type(const char *name, const epoc32_file_uid *uid);

/*
    Parameters  : str           - The UID string to parse.
                  uid           - Variable to receive the corresponding UID.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Parse the specified UID string.
*/
os_error *uid_parse_uid(const char *str, epoc32_file_uid *uid);

/*
    Parameters  : str           - The file type string to parse.
                  type          - Variable to receive the corresponding file
                                  type.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Parse the specified file type string.
*/
os_error *uid_parse_type(const char *str, bits *type);

/*
    Parameters  : ext           - The file extension to change.
                  type          - The corresponding file type, or
                                  UID_CLEAR_TYPE to clear the mapping.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Change or clear the mapping for the specified extension.
*/
os_error *uid_ext_mapping(const char *ext, bits type);

/*
    Parameters  : uid           - The file UID to change.
                  type          - The corresponding file type, or
                                  UID_CLEAR_TYPE to clear the mapping.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Change or clear the mapping for the specified UID.
*/
os_error *uid_uid_mapping(const epoc32_file_uid *uid, bits type);

/*
    Parameters  : type          - The corresponding file type, or
                                  UID_CLEAR_TYPE to clear the mapping.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Change or clear the mapping for untyped or unrecognised
                  files.
*/
os_error *uid_other_mapping(bits type);

/*
    Parameters  : enable        - Should the MimeMap module be used.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enable or disable use of the MimeMap module to convert
                  filename extensions to file types.
*/
os_error *uid_mime_map(bool enable);

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : List the current file type mappings.
*/
os_error *uid_list_mapping(void);

#ifdef __cplusplus
    }
#endif

#endif
