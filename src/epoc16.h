/*
    File        : epoc16.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Definition of EPOC16 types for the PsiFS module.

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
#ifndef EPOC16_H
#define EPOC16_H

// Maximum filename lengths
#define EPOC16_MAX_DISC_NAME (11)
#define EPOC16_MAX_LEAF_STUB (8)
#define EPOC16_MAX_LEAF_EXTENSION (3)
#define EPOC16_MAX_LEAF_NAME (EPOC16_MAX_LEAF_STUB + 1 \
                              + EPOC16_MAX_LEAF_EXTENSION)
#define EPOC16_MAX_PATH (127)

// Filenames
typedef char epoc16_disc_name[EPOC16_MAX_DISC_NAME + 1];
typedef char epoc16_leaf_name[EPOC16_MAX_LEAF_NAME + 1];
typedef char epoc16_path[EPOC16_MAX_PATH + 1];

// File or directory handle
typedef bits epoc16_handle;

// File attributes
typedef bits epoc16_file_attributes;
#define EPOC16_FILE_WRITEABLE ((epoc16_file_attributes) 0x0001)
#define EPOC16_FILE_HIDDEN ((epoc16_file_attributes) 0x0002)
#define EPOC16_FILE_SYSTEM ((epoc16_file_attributes) 0x0004)
#define EPOC16_FILE_VOLUME ((epoc16_file_attributes) 0x0008)
#define EPOC16_FILE_DIRECTORY ((epoc16_file_attributes) 0x0010)
#define EPOC16_FILE_MODIFIED ((epoc16_file_attributes) 0x0020)
#define EPOC16_FILE_READABLE ((epoc16_file_attributes) 0x0100)
#define EPOC16_FILE_EXECUTABLE ((epoc16_file_attributes) 0x0200)
#define EPOC16_FILE_STREAM ((epoc16_file_attributes) 0x0400)
#define EPOC16_FILE_TEXT ((epoc16_file_attributes) 0x0800)

// File modification date
typedef bits epoc16_file_time;

// Media types
typedef bits epoc16_media_type;
#define EPOC16_MEDIA_TYPE_MASK ((epoc16_media_type) 0x00ff)
#define EPOC16_MEDIA_NOT_KNOWN ((epoc16_media_type) 0x0000)
#define EPOC16_MEDIA_FLOPPY ((epoc16_media_type) 0x0001)
#define EPOC16_MEDIA_HARD_DISK ((epoc16_media_type) 0x0002)
#define EPOC16_MEDIA_FLASH ((epoc16_media_type) 0x0003)
#define EPOC16_MEDIA_RAM ((epoc16_media_type) 0x0004)
#define EPOC16_MEDIA_ROM ((epoc16_media_type) 0x0005)
#define EPOC16_MEDIA_WRITE_PROTECTED ((epoc16_media_type) 0x0006)
#define EPOC16_MEDIA_COMPRESSIBLE ((epoc16_media_type) 0x8000)
#define EPOC16_MEDIA_DYNAMIC ((epoc16_media_type) 0x4000)
#define EPOC16_MEDIA_INTERNAL ((epoc16_media_type) 0x2000)
#define EPOC16_MEDIA_DUAL_DENSITY ((epoc16_media_type) 0x1000)
#define EPOC16_MEDIA_FORMATTABLE ((epoc16_media_type) 0x0800)

// Battery status
typedef bits epoc16_battery_status;
/*
#define EPOC16_BATTERY_DEAD ((epoc16_battery_status) 0)
#define EPOC16_BATTERY_VERYLOW ((epoc16_battery_status) 1)
#define EPOC16_BATTERY_LOW ((epoc16_battery_status) 2)
#define EPOC16_BATTERY_GOOD ((epoc16_battery_status) 3)
*/

// Drive attributes
typedef bits epoc16_drive_attributes;
/*
#define EPOC16_DRIVE_LOCAL ((epoc16_drive_attributes) 0x01)
#define EPOC16_DRIVE_ROM ((epoc16_drive_attributes) 0x02)
#define EPOC16_DRIVE_REDIRECTED ((epoc16_drive_attributes) 0x04)
#define EPOC16_DRIVE_SUBSTED ((epoc16_drive_attributes) 0x08)
#define EPOC16_DRIVE_INTERNAL ((epoc16_drive_attributes) 0x10)
#define EPOC16_DRIVE_REMOVABLE ((epoc16_drive_attributes) 0x20)
*/

// Media attributes
typedef bits epoc16_media_attributes;
/*
#define EPOC16_MEDIA_VARIABLE_SIZE ((epoc16_media_attributes) 0x01)
#define EPOC16_MEDIA_DUAL_DENSITY ((epoc16_media_attributes) 0x02)
#define EPOC16_MEDIA_FORMATTABLE ((epoc16_media_attributes) 0x04)
#define EPOC16_MEDIA_WRITE_PROTECTED ((epoc16_media_attributes) 0x08)
*/

// Modes for opening files
typedef bits epoc16_mode;
#define EPOC16_MODE_OPEN_EXISTING ((epoc16_mode) 0x0000)
#define EPOC16_MODE_CREATE_NEW ((epoc16_mode) 0x0001)
#define EPOC16_MODE_OVERWRITE ((epoc16_mode) 0x0002)
#define EPOC16_MODE_APPEND ((epoc16_mode) 0x0003)
#define EPOC16_MODE_CREATE_UNIQUE ((epoc16_mode) 0x0004)
#define EPOC16_MODE_BINARY_STREAM ((epoc16_mode) 0x0000)
#define EPOC16_MODE_TEXT_STREAM ((epoc16_mode) 0x0010)
#define EPOC16_MODE_BINARY_RECORD ((epoc16_mode) 0x0020)
#define EPOC16_MODE_DIRECTORY_RECORD ((epoc16_mode) 0x0030)
#define EPOC16_MODE_FORMAT ((epoc16_mode) 0x0040)
#define EPOC16_MODE_DEVICE_LIST ((epoc16_mode) 0x0050)
#define EPOC16_MODE_NODE_LIST ((epoc16_mode) 0x0060)
#define EPOC16_MODE_READ_WRITE ((epoc16_mode) 0x0100)
#define EPOC16_MODE_RANDOM_ACCESS ((epoc16_mode) 0x0200)
#define EPOC16_MODE_SHARE ((epoc16_mode) 0x0400)

// Modes for seeking
typedef bits epoc16_sense;
#define EPOC16_SENSE_ABSOLUTE ((epoc16_sense) 0x0001)
#define EPOC16_SENSE_CURRENT ((epoc16_sense) 0x0002)
#define EPOC16_SENSE_END ((epoc16_sense) 0x0003)
#define EPOC16_SENSE_SENSE ((epoc16_sense) 0x0004)
#define EPOC16_SENSE_SET ((epoc16_sense) 0x0005)
#define EPOC16_SENSE_REWIND ((epoc16_sense) 0x0006)

// A P_INFO to describe a directory entry
typedef struct
{
    bits version;
    epoc16_file_attributes attributes;
    bits size;
    epoc16_file_time modified;
    bits reserved;
    epoc16_leaf_name name;
} epoc16_p_info;

// A P_DINFO to describe a disc
typedef struct
{
    bits version;
    epoc16_media_type type;
    bool removable;
    bits size;
    bits free;
    epoc16_disc_name name;
    epoc16_battery_status battery;
    byte reserved[16];
} epoc16_p_dinfo;

#endif
