/*
    File        : fs.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Core filing system implementation for the PsiFS module.

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
#ifndef FS_H
#define FS_H

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/fileswitch.h"
#include "oslib/os.h"
#include "oslib/osgbpb.h"

// Include project header files
#include "date.h"
#include "psifs.h"

// Filing system name
#define FS_NAME "PsiFS"

// Maximum open files (0 for no fixed limit)
#define FS_FILES (0)

// Filing system information word
#define FS_INFO ((fileswitch_fs_info) (fileswitch_NEEDS_FLUSH \
                                       | fileswitch_SUPPORTS_IMAGE \
                                       | fileswitch_NO_LOAD_ENTRY \
                                       | fileswitch_NO_SAVE_ENTRY \
                                       | fileswitch_HAS_EXTRA_FLAGS \
                                       | (FS_FILES << 8) \
                                       | psifs_FS_NUMBER_PSIFS))

// Extra filing system information word
#define FS_EXTRA_INFO ((fileswitch_fs_extra_info) 0)

// Maximum filename lengths
#define FS_MAX_DISCNAME (11)
#define FS_MAX_LEAFNAME (250)
#define FS_MAX_PATHNAME (255)
typedef char fs_discname[FS_MAX_DISCNAME + 1];
typedef char fs_leafname[FS_MAX_LEAFNAME + 1];
typedef char fs_pathname[FS_MAX_PATHNAME + 1];

// A filing system file handle
typedef struct cache_file *fs_handle;
#define FS_NONE ((fs_handle) NULL)

// File information word
typedef bits fs_file_info_word;
#define FS_FILE_INFO_WRITE_PERMITTED ((fs_file_info_word) 0x80000000)
#define FS_FILE_INFO_READ_PERMITTED ((fs_file_info_word) 0x40000000)
#define FS_FILE_INFO_IS_DIRECTORY ((fs_file_info_word) 0x20000000)
#define FS_FILE_INFO_UNBUFFERED_GBPB ((fs_file_info_word) 0x10000000)
#define FS_FILE_INFO_INTERACTIVE ((fs_file_info_word) 0x08000000)

// End of file indicator
typedef int fs_is_eof;
#define FS_NOT_EOF ((fs_is_eof) 0)
#define FS_EOF ((fs_is_eof) -1)

// Details for a drive
typedef struct
{
    bool present;
    bool rom;
    psifs_drive_id id;
    fs_discname name;
    bits free_low;
    bits free_high;
    bits size_low;
    bits size_high;
} fs_drive;

// Details for a single file
typedef osgbpb_INFO(FS_MAX_LEAFNAME + 1) fs_info;

// Details for an open file
typedef struct
{
    os_fw handle;
    fs_file_info_word info;
    bits extent;
    bits allocated;
    bits sequential;
} fs_open_info;

// Buffer size for all objects
#define FS_BUFFER_SIZE (1024)

// File open modes
typedef bits fs_mode;
#define FS_MODE_OUT ((fs_mode) 0x00)
#define FS_MODE_IN ((fs_mode) 0x01)
#define FS_MODE_UP ((fs_mode) 0x02)

// Special characters in filenames
#define FS_CHAR_SEPARATOR '.'
#define FS_CHAR_EXTENSION '/'
#define FS_CHAR_DISC ':'
#define FS_CHAR_ROOT '$'
#define FS_CHAR_URD '&'
#define FS_CHAR_CSD '@'
#define FS_CHAR_PARENT '^'
#define FS_CHAR_CSL '%'
#define FS_CHAR_PSD '\\'
#define FS_CHAR_WILD_SINGLE '#'
#define FS_CHAR_WILD_ANY '*'

// Grouped drive letter and name
#define FS_CHAR_DRIVE_ALL '@'
#define FS_NAME_DRIVE_ALL "AllDrives"

// The default drive
extern char fs_default_drive;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : src           - The external or internal path to convert.
                  dest          - Variable to receive a pointer to the result.
                                  This may be set to NULL if the operation
                                  fails.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert the specified path to the internal representation.
                  It is an error if either the disc or directory do not
                  exist. This does not remove any wildcards from the leaf name,
                  or check for existence of the specified object. The result
                  should be copied if required; the value is stored in a
                  temporary buffer.
*/
os_error *fs_internal_name(const char *src, const char **dest);

/*
    Parameters  : src           - The external or internal path to convert.
                  dest          - Variable to receive a pointer to the result.
                                  This may be set to NULL if the operation
                                  fails.
                  info          - Variable to receive a pointer to the details
                                  for the item. The object type is set to
                                  fileswitch_NOT_FOUND if it does not exist.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert the specified path to the internal representation.
                  It is an error if either the disc or directory do not exist,
                  but it is not an error if the object does not exist. This
                  removes all wildcards from the path. The results should be
                  copied if required; the values are stored in temporary
                  buffers.
*/
os_error *fs_internal_name_resolve(const char *src, const char **dest,
                                   const fs_info **info);

/*
    Parameters  : src           - The internal path to convert.
                  dest          - Variable to receive a pointer to the result.
                                  This may be set to NULL if the operation
                                  fails.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert the specified path to the canonical external
                  representation. This does not remove any wildcards from the
                  path, or check for existence of the specified object. The
                  result should be copied if required; the value is stored in
                  a temporary buffer.
*/
os_error *fs_external_name(const char *src, const char **dest);

/*
    Parameters  : path          - The path to check.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Check whether the specified path contains wildcards, and
                  return an error if it does.
*/
os_error *fs_wildcards(const char *path);

/*
    Parameters  : name          - The name of the disc or drive.
                  drive         - Variable to receive the corresponding drive
                                  letter.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert the specified disc or drive name to a drive letter.
                  It is an error if the disc is not present.
*/
os_error *fs_internal_drive(const char *name, char *drive);

/*
    Parameters  : drive         - The drive to read the details for.
                  mangle        - Should the drive name be mangled if invalid.
                  info          - Variable to receive a pointer to the details
                                  for the drive.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details for the specified drive. It is not an
                  error for the drive to be empty. The results should be
                  copied if required; the values are stored in a temporary
                  buffer.
*/
os_error *fs_drive_info(char drive, bool mangle, const fs_drive **info);

/*
    Parameters  : drive         - The drive to change the name of.
                  info          - The new name for the disc.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Change the name of the specified disc.
*/
os_error *fs_drive_name(char drive, const char *name);

/*
    Parameters  : path          - The object to read the details for.
                  info          - Variable to receive a pointer to the details
                                  for the item. The object type is set to
                                  fileswitch_NOT_FOUND if it does not exist.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details for the specified object. It is not an
                  error if the object does not exist. The results should be
                  copied if required; the values are stored in a temporary
                  buffer.
*/
os_error *fs_file_info(const char *path, const fs_info **info);

/*
    Parameters  : path          - The directory to search.
                  match         - Wildcarded leaf name to match.
                  offset        - Variable containing the offset of the first
                                  item to read (0 for first call). This is
                                  updated ready for the next call (-1 if end).
                  buffer        - Pointer to buffer to receive the entries.
                  size          - The number of entries to read.
                  read          - The number of entries read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Enumerate items matching the supplied specification.
*/
os_error *fs_enumerate(const char *path, const char *match, int *offset,
                       fs_info *buffer, bits size, bits *read);

/*
    Parameters  : path          - Non-wildcarded path of the directory.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create a directory.
*/
os_error *fs_mkdir(const char *path);

/*
    Parameters  : path          - Non-wildcarded path of object to delete.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Delete the specified object. No error will be returned if the
                  object does not exist.
*/
os_error *fs_remove(const char *path);

/*
    Parameters  : src           - Current non-wildcarded path of the file.
                  dest          - New non-wildcarded path of the file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Rename the specified object. An error may be returned if
                  the rename is not simple, i.e. not just a case of changing
                  the object's catalogue entry.
*/
os_error *fs_rename(const char *src, const char *dest);

/*
    Parameters  : path          - Non-wildcarded path of the object.
                  attr          - The required attributes for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the attributes for the specified object.
*/
os_error *fs_access(const char *path, fileswitch_attr attr);

/*
    Parameters  : path          - Non-wildcarded path of the object.
                  load          - The load address for the specified object.
                  exec          - The execute address for the specified object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the load and execute addresses for the specified object.
*/
os_error *fs_stamp(const char *path, bits load, bits exec);

/*
    Parameters  : path          - Non-wildcarded path of the file.
                  mode          - The mode in which the object should be opened.
                  handle        - The FileSwitch file handle for the object,
                                  or 0 for none.
                  object        - Variable to receive the filing system handle
                                  for the object, or FS_NONE if failed to open.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Open the specified object. This may optionally create a new
                  object or overwrite an existing one. Failure to allocate
                  memory is not an error
*/
os_error *fs_open(const char *path, fs_mode mode, os_fw handle,
                  fs_handle *object);

/*
    Parameters  : object        - Filing system handle for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Close the specified object.
*/
os_error *fs_close(fs_handle object);

/*
    Parameters  : object        - Filing system handle for the object.
                  info          - Variable to receive a pointer to the details
                                  for the item.
                  path          - Vairable to receive a pointer to the name.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the details for the specified open file. It is an error
                  if either the handle or file is not valid. The results should
                  be copied if required; the values are stored in temporary
                  buffers.
*/
os_error *fs_args(fs_handle object, const fs_open_info **info,
                  const char **path);

/*
    Parameters  : file          - Filing system handle for the file.
                  buffer        - The buffer to write the data into.
                  offset        - The initial file offset to read from.
                  bytes         - The number of bytes to read.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read bytes from the specified object. Bytes past the end of
                  the file are returned as zeros. It is an error to read from
                  a directory.
*/
os_error *fs_read(fs_handle file, void *buffer, bits offset, bits bytes);

/*
    Parameters  : file          - Filing system handle for the file.
                  buffer        - The buffer to read the data from.
                  offset        - The initial file offset to write at.
                  bytes         - The number of bytes to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write bytes to the specified object, increasing the file
                  extent if required. If the specified offset is beyond the
                  end of the file then the gap is padded with zeros. It is
                  an error to write to either a directory or a file without
                  write access.
*/
os_error *fs_write(fs_handle file, const void *buffer, bits offset, bits bytes);

/*
    Parameters  : file          - Filing system handle for the file.
                  offset        - The initial file offset to write at.
                  bytes         - The number of bytes to write.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write zeros to the specified object, increasing the file
                  extent if required. It is an error to write to either a
                  directory or a file without write access.
*/
os_error *fs_write_zeros(fs_handle file, bits offset, bits bytes);

/*
    Parameters  : object        - Filing system handle for the object.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Flush any modified data for the specified object being held
                  in buffers.
*/
os_error *fs_flush(fs_handle object);

/*
    Parameters  : file          - Filing system handle for the file.
                  allocated     - The allocated size for the specified file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the allocated size of the specified file to at least the
                  specified size. If the file is extended then the value of the
                  new data is undefined. It is an error to set the allocated
                  size of a directory or a file without write access.
*/
os_error *fs_allocated(fs_handle file, bits allocated);

/*
    Parameters  : file          - Filing system handle for the file.
                  extent        - The extent for the specified file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the extent of the specified file. The file is either
                  truncated or padded with zeros as appropriate. It is an error
                  to set the extent of a directory or a file without write
                  access.
*/
os_error *fs_extent(fs_handle file, bits extent);

/*
    Parameters  : file          - Filing system handle for the file.
                  sequential    - The sequential pointer.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the sequential pointer for the specified file. If the
                  new pointer is greater than the current file extent then if
                  possible the file is extended with zeros and the extent is
                  set to the new sequential pointer, otherwise an error is
                  returned. It is an error to set the sequential pointer for
                  a directory.
*/
os_error *fs_sequential(fs_handle file, bits sequential);

#ifdef __cplusplus
    }
#endif

#endif
