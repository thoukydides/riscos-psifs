/*
    File        : attr.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : File and directory attribute conversions for the PsiFS
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
#ifndef ATTR_H
#define ATTR_H

// Include oslib header files
#include "oslib/fileswitch.h"

// Include project header files
#include "epoc16.h"
#include "epoc32.h"

// Masks for modifiable attributes
#define ATTR_ERA_MASK (EPOC32_FILE_READ_ONLY | EPOC32_FILE_HIDDEN | EPOC32_FILE_SYSTEM | EPOC32_FILE_ARCHIVE)
#define ATTR_SIBO_MASK (EPOC16_FILE_WRITEABLE | EPOC16_FILE_HIDDEN | EPOC16_FILE_SYSTEM | EPOC16_FILE_MODIFIED)

// UNIX attributes
typedef bits attr_unix;
#define ATTR_UNIX_USER_READ ((attr_unix) 0100)
#define ATTR_UNIX_USER_WRITE ((attr_unix) 0200)
#define ATTR_UNIX_USER_EXECUTE ((attr_unix) 0400)
#define ATTR_UNIX_GROUP_READ ((attr_unix) 0010)
#define ATTR_UNIX_GROUP_WRITE ((attr_unix) 0020)
#define ATTR_UNIX_GROUP_EXECUTE ((attr_unix) 0040)
#define ATTR_UNIX_OTHER_READ ((attr_unix) 0001)
#define ATTR_UNIX_OTHER_WRITE ((attr_unix) 0002)
#define ATTR_UNIX_OTHER_EXECUTE ((attr_unix) 0004)

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : attr              - The ERA format file attributes.
    Returns     : fileswitch_attr   - The RISC OS format attributes.
    Description : Convert file or directory attributes from ERA to RISC OS.
*/
fileswitch_attr attr_from_era(epoc32_file_attributes attr);

/*
    Parameters  : attr                      - The RISC OS format file
                                              attributes.
    Returns     : epoc32_file_attributes    - The ERA format attributes.
    Description : Convert file or directory attributes from RISC OS to ERA.
*/
epoc32_file_attributes attr_to_era(fileswitch_attr attr);

/*
    Parameters  : attr              - The SIBO format file attributes.
    Returns     : fileswitch_attr   - The RISC OS format attributes.
    Description : Convert file or directory attributes from SIBO to RISC OS.
*/
fileswitch_attr attr_from_sibo(epoc16_file_attributes attr);

/*
    Parameters  : attr                      - The RISC OS format file
                                              attributes.
    Returns     : epoc16_file_attributes    - The SIBO format attributes.
    Description : Convert file or directory attributes from RISC OS to SIBO.
*/
epoc16_file_attributes attr_to_sibo(fileswitch_attr attr);

/*
    Parameters  : attr              - The UNIX format file attributes.
    Returns     : fileswitch_attr   - The RISC OS format attributes.
    Description : Convert file or directory attributes from UNIX to RISC OS.
*/
fileswitch_attr attr_from_unix(attr_unix attr);

/*
    Parameters  : attr          - The RISC OS format file attributes.
    Returns     : attr_unix     - The UNIX format attributes.
    Description : Convert file or directory attributes from RISC OS to UNIX
*/
attr_unix attr_to_unix(fileswitch_attr attr);

#ifdef __cplusplus
    }
#endif

#endif
