/*
    File        : attr.c
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

// Include header file for this module
#include "attr.h"

/*
    Parameters  : attr              - The ERA format file attributes.
    Returns     : fileswitch_attr   - The RISC OS format attributes.
    Description : Convert file or directory attributes from ERA to RISC OS.
*/
fileswitch_attr attr_from_era(epoc32_file_attributes attr)
{
    fileswitch_attr value = 0;

    // Map the owner attributes
    if ((attr & EPOC32_FILE_SYSTEM)
        || ((attr & EPOC32_FILE_DIRECTORY) && (attr & EPOC32_FILE_READ_ONLY)))
    {
        value |= fileswitch_ATTR_OWNER_LOCKED;
    }
    if (!(attr & EPOC32_FILE_DIRECTORY) && !(attr & EPOC32_FILE_READ_ONLY))
    {
        value |= fileswitch_ATTR_OWNER_WRITE;
    }
    if (!(attr & EPOC32_FILE_DIRECTORY))
    {
        value |= fileswitch_ATTR_OWNER_READ;
    }

    // Map the public attributes
    if (attr & EPOC32_FILE_ARCHIVE)
    {
        value |= fileswitch_ATTR_WORLD_WRITE;
    }
    if (!(attr & EPOC32_FILE_ARCHIVE) != !(attr & EPOC32_FILE_HIDDEN))
    {
        value |= fileswitch_ATTR_WORLD_READ;
    }

    // Return the result
    return value;
}

/*
    Parameters  : attr                      - The RISC OS format file
                                              attributes.
    Returns     : epoc32_file_attributes    - The ERA format attributes.
    Description : Convert file or directory attributes from RISC OS to ERA.
*/
epoc32_file_attributes attr_to_era(fileswitch_attr attr)
{
    epoc32_file_attributes value = 0;

    // Map the owner attributes
    if (!(attr & fileswitch_ATTR_OWNER_WRITE)
        && (attr & (fileswitch_ATTR_OWNER_LOCKED
                    | fileswitch_ATTR_OWNER_READ)))
    {
        value |= EPOC32_FILE_READ_ONLY;
    }
    if ((attr & fileswitch_ATTR_OWNER_LOCKED)
        && (attr & (fileswitch_ATTR_OWNER_WRITE
                    | fileswitch_ATTR_OWNER_READ)))
    {
        value |= EPOC32_FILE_SYSTEM;
    }

    // Map the public attributes
    if (attr & fileswitch_ATTR_WORLD_WRITE)
    {
        value |= EPOC32_FILE_ARCHIVE;
    }
    if (!(attr & fileswitch_ATTR_WORLD_WRITE)
        != !(attr & fileswitch_ATTR_WORLD_READ))
    {
        value |= EPOC32_FILE_HIDDEN;
    }

    // Return the result
    return value;
}

/*
    Parameters  : attr              - The SIBO format file attributes.
    Returns     : fileswitch_attr   - The RISC OS format attributes.
    Description : Convert file or directory attributes from SIBO to RISC OS.
*/
fileswitch_attr attr_from_sibo(epoc16_file_attributes attr)
{
    epoc32_file_attributes era = 0;

    // Convert to ERA attributes first
    if (!(attr & EPOC16_FILE_WRITEABLE)) era |= EPOC32_FILE_READ_ONLY;
    if (attr & EPOC16_FILE_SYSTEM) era |= EPOC32_FILE_SYSTEM;
    if (attr & EPOC16_FILE_MODIFIED) era |= EPOC32_FILE_ARCHIVE;
    if (attr & EPOC16_FILE_HIDDEN) era |= EPOC32_FILE_HIDDEN;

    // Return the result compared to
    return attr_from_era(era);
}

/*
    Parameters  : attr                      - The RISC OS format file
                                              attributes.
    Returns     : epoc16_file_attributes    - The SIBO format attributes.
    Description : Convert file or directory attributes from RISC OS to SIBO.
*/
epoc16_file_attributes attr_to_sibo(fileswitch_attr attr)
{
    epoc32_file_attributes era = attr_to_era(attr);
    epoc16_file_attributes value = 0;

    // Map the attributes
    if (!(era & EPOC32_FILE_READ_ONLY)) value |= EPOC16_FILE_WRITEABLE;
    if (era & EPOC32_FILE_SYSTEM) value |= EPOC16_FILE_SYSTEM;
    if (era & EPOC32_FILE_ARCHIVE) value |= EPOC16_FILE_MODIFIED;
    if (era & EPOC32_FILE_HIDDEN) value |= EPOC16_FILE_HIDDEN;

    // Return the result
    return value;
}

/*
    Parameters  : attr              - The UNIX format file attributes.
    Returns     : fileswitch_attr   - The RISC OS format attributes.
    Description : Convert file or directory attributes from UNIX to RISC OS.
*/
fileswitch_attr attr_from_unix(attr_unix attr)
{
    attr_unix value = 0;

    // Map the owner attributes
    if (attr & (ATTR_UNIX_USER_EXECUTE | ATTR_UNIX_GROUP_EXECUTE))
    {
        value |= fileswitch_ATTR_OWNER_READ;
    }
    if (attr & (ATTR_UNIX_USER_WRITE | ATTR_UNIX_GROUP_WRITE))
    {
        value |= fileswitch_ATTR_OWNER_WRITE;
    }

    // Map the public attributes
    if (attr & ATTR_UNIX_OTHER_EXECUTE) value |= fileswitch_ATTR_WORLD_READ;
    if (attr & ATTR_UNIX_OTHER_WRITE) value |= fileswitch_ATTR_WORLD_WRITE;

    // Return the result
    return value;
}

/*
    Parameters  : attr          - The RISC OS format file attributes.
    Returns     : attr_unix     - The UNIX format attributes.
    Description : Convert file or directory attributes from RISC OS to UNIX
*/
attr_unix attr_to_unix(fileswitch_attr attr)
{
    attr_unix value = 0;

    // Map the owner attributes
    value |= ATTR_UNIX_USER_READ | ATTR_UNIX_GROUP_READ;
    if (attr & fileswitch_ATTR_OWNER_READ)
    {
        value |= ATTR_UNIX_USER_EXECUTE | ATTR_UNIX_GROUP_EXECUTE;
    }
    if (attr & fileswitch_ATTR_OWNER_WRITE)
    {
        value |= ATTR_UNIX_USER_WRITE | ATTR_UNIX_GROUP_WRITE;
    }

    // Map the public attributes
    value |= ATTR_UNIX_OTHER_READ;
    if (attr & fileswitch_ATTR_WORLD_READ) value |= ATTR_UNIX_OTHER_EXECUTE;
    if (attr & fileswitch_ATTR_WORLD_WRITE) value |= ATTR_UNIX_OTHER_WRITE;

    // Return the result
    return value;
}
