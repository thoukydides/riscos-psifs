/*
    File        : epoc32.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Definition of EPOC32 types for the PsiFS module.

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
#ifndef EPOC32_H
#define EPOC32_H

// Include project header files
#include "psifs.h"

// Maximum filename and string lengths
#define EPOC32_MAX_DISC_NAME (11)
#define EPOC32_MAX_LEAF_NAME (253)
#define EPOC32_MAX_SHORT_NAME (12)
#define EPOC32_MAX_PATH (268)
#define EPOC32_MAX_MACHINE_NAME (15)

// Filenames and strings
typedef char epoc32_disc_name[EPOC32_MAX_DISC_NAME + 1];
typedef char epoc32_leaf_name[EPOC32_MAX_LEAF_NAME + 1];
typedef char epoc32_short_name[EPOC32_MAX_SHORT_NAME + 1];
typedef char epoc32_path[EPOC32_MAX_PATH + 1];
typedef char epoc32_machine_name[EPOC32_MAX_MACHINE_NAME + 1];

// File or directory handle
typedef bits epoc32_handle;

// Resource handle
typedef bits epoc32_resource_handle;

// A longer integer
typedef struct
{
    bits low;
    bits high;
} epoc32_double_bits;

// File attributes
typedef bits epoc32_file_attributes;
#define EPOC32_FILE_READ_ONLY ((epoc32_file_attributes) 0x0001)
#define EPOC32_FILE_HIDDEN ((epoc32_file_attributes) 0x0002)
#define EPOC32_FILE_SYSTEM ((epoc32_file_attributes) 0x0004)
#define EPOC32_FILE_DIRECTORY ((epoc32_file_attributes) 0x0010)
#define EPOC32_FILE_ARCHIVE ((epoc32_file_attributes) 0x0020)
#define EPOC32_FILE_VOLUME ((epoc32_file_attributes) 0x0040)
#define EPOC32_FILE_NORMAL ((epoc32_file_attributes) 0x0080)
#define EPOC32_FILE_TEMPORARY ((epoc32_file_attributes) 0x0100)
#define EPOC32_FILE_COMPRESSED ((epoc32_file_attributes) 0x0800)
// match mask
// mask supported
// match exclusive
// match exclude
#define EPOC32_FILE_UID ((epoc32_file_attributes) 0x10000000)
// share mask

// File modification date
typedef epoc32_double_bits epoc32_file_time;

// File user IDs
typedef struct
{
    bits uid1;
    bits uid2;
    bits uid3;
} epoc32_file_uid;

// Media types (CF disc appears as a hard disk)
typedef bits epoc32_media_type;
#define EPOC32_MEDIA_NOT_PRESENT ((epoc32_media_type) 0)
#define EPOC32_MEDIA_NOT_KNOWN ((epoc32_media_type) 1)
#define EPOC32_MEDIA_FLOPPY ((epoc32_media_type) 2)
#define EPOC32_MEDIA_HARD_DISK ((epoc32_media_type) 3)
#define EPOC32_MEDIA_CDROM ((epoc32_media_type) 4)
#define EPOC32_MEDIA_RAM ((epoc32_media_type) 5)
#define EPOC32_MEDIA_FLASH ((epoc32_media_type) 6)
#define EPOC32_MEDIA_ROM ((epoc32_media_type) 7)
#define EPOC32_MEDIA_REMOTE ((epoc32_media_type) 8)

// Battery status
typedef bits epoc32_battery_status;
#define EPOC32_BATTERY_DEAD ((epoc32_battery_status) 0)
#define EPOC32_BATTERY_VERYLOW ((epoc32_battery_status) 1)
#define EPOC32_BATTERY_LOW ((epoc32_battery_status) 2)
#define EPOC32_BATTERY_GOOD ((epoc32_battery_status) 3)

// Drive attributes
typedef bits epoc32_drive_attributes;
#define EPOC32_DRIVE_LOCAL ((epoc32_drive_attributes) 0x01)
#define EPOC32_DRIVE_ROM ((epoc32_drive_attributes) 0x02)
#define EPOC32_DRIVE_REDIRECTED ((epoc32_drive_attributes) 0x04)
#define EPOC32_DRIVE_SUBSTED ((epoc32_drive_attributes) 0x08)
#define EPOC32_DRIVE_INTERNAL ((epoc32_drive_attributes) 0x10)
#define EPOC32_DRIVE_REMOVABLE ((epoc32_drive_attributes) 0x20)

// Media attributes
typedef bits epoc32_media_attributes;
#define EPOC32_MEDIA_VARIABLE_SIZE ((epoc32_media_attributes) 0x01)
#define EPOC32_MEDIA_DUAL_DENSITY ((epoc32_media_attributes) 0x02)
#define EPOC32_MEDIA_FORMATTABLE ((epoc32_media_attributes) 0x04)
#define EPOC32_MEDIA_WRITE_PROTECTED ((epoc32_media_attributes) 0x08)

// Modes for opening files
typedef bits epoc32_mode;
#define EPOC32_MODE_SHARE_EXCLUSIVE ((epoc32_mode) 0x0000)
#define EPOC32_MODE_SHARE_READERS ((epoc32_mode) 0x0001)
#define EPOC32_MODE_SHARE_ANY ((epoc32_mode) 0x0002)
#define EPOC32_MODE_BINARY ((epoc32_mode) 0x0000)
#define EPOC32_MODE_TEXT ((epoc32_mode) 0x0020)
#define EPOC32_MODE_READ_WRITE ((epoc32_mode) 0x0200)

// Modes for seeking
typedef bits epoc32_sense;
#define EPOC32_SENSE_ABSOLUTE ((epoc32_sense) 0x0001)
#define EPOC32_SENSE_CURRENT ((epoc32_sense) 0x0002)
#define EPOC32_SENSE_END ((epoc32_sense) 0x0003)
#define EPOC32_SENSE_SENSE ((epoc32_sense) 0x0004)
#define EPOC32_SENSE_SET ((epoc32_sense) 0x0005)
#define EPOC32_SENSE_REWIND ((epoc32_sense) 0x0006)

// A TRemoteEntry to describe a directory entry
typedef struct
{
    epoc32_file_attributes attributes;
    bits size;
    epoc32_file_time modified;
    epoc32_file_uid uid;
    epoc32_leaf_name name;
    epoc32_short_name short_name;
} epoc32_remote_entry;

// A TVolumeInfo to describe a disc
typedef struct
{
    epoc32_media_type type;
    epoc32_battery_status battery;
    epoc32_drive_attributes drive;
    epoc32_media_attributes media;
    bits uid;
    epoc32_double_bits size;
    epoc32_double_bits free;
    epoc32_disc_name name;
} epoc32_volume_info;

// An array of drives
#define EPOC32_DRIVES (26)
typedef byte epoc32_drives[EPOC32_DRIVES];

// Daylight saving zone
typedef bits epoc32_daylight;
#define EPOC32_DAYLIGHT_HOME ((epoc32_daylight) 0x40000000)
#define EPOC32_DAYLIGHT_NONE ((epoc32_daylight) 0x00000000)
#define EPOC32_DAYLIGHT_EUROPEAN ((epoc32_daylight) 0x00000001)
#define EPOC32_DAYLIGHT_NORTHERN ((epoc32_daylight) 0x00000002)
#define EPOC32_DAYLIGHT_SOUTHERN ((epoc32_daylight) 0x00000004)

// Information about a machine
typedef struct
{
    byte major;
    byte minor;
    unsigned short build;
    byte reserved[8];
} epoc32_rom_version;

// Display size in pixels
typedef struct
{
    bits width;
    bits height;
} epoc32_display_size;

// Time intervals
typedef int epoc32_inverval_seconds;
typedef epoc32_double_bits epoc32_interval_microseconds;

// Current time
typedef struct
{
    epoc32_file_time home_time;
    bits country_code;
    epoc32_inverval_seconds utc_offset_seconds;
    epoc32_daylight dst;
    epoc32_daylight dst_zone;
} epoc32_time;

// Supply information
typedef struct
{
    epoc32_file_time main_inserted;
    epoc32_battery_status main_status;
    epoc32_interval_microseconds main_used;
    bits main_current_ma;
    bits main_used_ma_sec;
    bits main_mv;
    bits main_mv_max;
    epoc32_battery_status backup_status;
    bits backup_mv;
    bits backup_mv_max;
    bits external;
    epoc32_interval_microseconds backup_used;
    bits flags;
} epoc32_supply_info;

// Machine memory information
typedef struct
{
    bits ram_size;
    bits rom_size;
    bits ram_free_max;
    bits ram_free;
    bits ram_disc_size;
    bits ram_registry_size;
    bool reprogrammable;
} epoc32_memory_info;

// Machine information
typedef struct
{
    psifs_machine_type machine_type;
    epoc32_rom_version rom_version;
    epoc32_machine_name machine_name;
    epoc32_display_size display_size;
    epoc32_double_bits machine_uid;
    epoc32_time time;
    epoc32_supply_info supply;
    epoc32_memory_info memory;
    psifs_language language;
    char reserved[88];
} epoc32_machine_info;

#endif
