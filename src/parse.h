/*
    File        : parse.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Frame and buffer passing functions for the PsiFS module.

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
#ifndef PARSE_H
#define PARSE_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "frame.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : buffer        - Buffer to receive the data.
                  size          - Size of the buffer.
                  offset        - Pointer to the variable containing the
                                  current buffer offset. This will be updated
                                  when the buffer is modified.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start writing data to a buffer. The details are stored
                  internally within this module, so only a single buffer can be
                  processed at a time.
*/
os_error *parse_put_start(byte *buffer, bits size, bits *offset);

/*
    Parameters  : frame         - Frame to receive the data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start writing data to a frame. The details are stored
                  internally within this module, so only a single buffer can be
                  processed at a time.
*/
os_error *parse_put_start_frame(frame_data *frame);

/*
    Parameters  : value         - The byte value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_byte(byte value);

/*
    Parameters  : value         - The word value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_word(unsigned short value);

/*
    Parameters  : value         - The bits value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_bits(bits value);

/*
    Parameters  : value         - The string value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_string(const char *value);

/*
    Parameters  : len           - The number of characters to append.
                  value         - The string value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_string_len(bits len, const char *value);

/*
    Parameters  : len           - The number of bytes to append.
                  value         - The data value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_bytes(bits len, const void *value);

/*
    Parameters  : boundary      - The alignment in bytes (usually 2 or 4).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Advance the buffer pointer to the next boundary.
*/
os_error *parse_put_align(bits boundary);

/*
    Parameters  : buffer        - Buffer containing the data.
                  size          - Size of the used portion of the buffer.
                  offset        - Pointer to the variable containing the
                                  current buffer offset. This will be updated
                                  when values are extracted from the buffer.
                                  NULL may be specified to use an internal
                                  variable that is initialised to zero.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start reading data from a buffer. The details are stored
                  internally within this module, so only a single buffer can be
                  processed at a time.
*/
os_error *parse_get_start(const byte *buffer, bits size, bits *offset);

/*
    Parameters  : frame         - Frame containing the data.
                  offset        - Pointer to the variable containing the
                                  current frame offset. This will be updated
                                  when values are extracted from the frame.
                                  NULL may be specified to use an internal
                                  variable that is initialised to zero.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start reading data from a frame. The details are stored
                  internally within this module, so only a single buffer can be
                  processed at a time.
*/
os_error *parse_get_start_frame(const frame_data *frame, bits *offset);

/*
    Parameters  : value         - Variable to receive the next byte value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next byte value from the current buffer.
*/
os_error *parse_get_byte(byte *value);

/*
    Parameters  : value         - Variable to receive the next word value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next word value from the current buffer.
*/
os_error *parse_get_word(unsigned short *value);

/*
    Parameters  : value         - Variable to receive the next bits value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next bits value from the current buffer.
*/
os_error *parse_get_bits(bits *value);

/*
    Parameters  : value         - Variable to receive a pointer to the next
                                  string value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Obtain a pointer to the next string value from the current
                  buffer. The string is not copied from the buffer.
*/
os_error *parse_get_string(const char **value);

/*
    Parameters  : len           - Number of characters to copy from the buffer.
                  value         - Variable to receive the string value,
                                  including a terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Copy the specified number of characters from the buffer and
                  add a terminator.
*/
os_error *parse_get_string_len(bits len, char *value);

/*
    Parameters  : len           - Number of bytes to copy from the buffer.
                  value         - Variable to receive the data value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Copy the specified number of bytes from the buffer.
*/
os_error *parse_get_bytes(bits len, void *value);

/*
    Parameters  : boundary      - The alignment in bytes (usually 2 or 4).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Advance the buffer pointer to the next boundary.
*/
os_error *parse_get_align(bits boundary);

#ifdef __cplusplus
    }
#endif

#endif
