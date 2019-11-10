/*
    File        : parse.c
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

// Include header file for this module
#include "parse.h"

// Include clib header files
#include <string.h>

// Include project header files
#include "err.h"

// Details of the buffer being written
static byte *parse_write_buffer = NULL;
static bits parse_write_size = 0;
static bits *parse_write_offset = 0;

// Details of the buffer being read
static const byte *parse_read_buffer = NULL;
static bits parse_read_size = 0;
static bits *parse_read_offset = 0;
static bits parse_read_offset_value = 0;

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
os_error *parse_put_start(byte *buffer, bits size, bits *offset)
{
    os_error *err = NULL;

    // Check parameters
    if (!buffer || !offset || (size < *offset)) err = &err_bad_parms;
    else
    {
        // Store the buffer details
        parse_write_buffer = buffer;
        parse_write_size = size;
        parse_write_offset = offset;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - Frame to receive the data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start writing data to a frame. The details are stored
                  internally within this module, so only a single buffer can be
                  processed at a time.
*/
os_error *parse_put_start_frame(frame_data *frame)
{
    os_error *err = NULL;

    // Check parameters
    if (!frame) err = &err_bad_parms;
    else
    {
        err = parse_put_start(frame->data, FRAME_MAX_DATA_TX,
                              (bits *) &frame->size);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - The byte value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_byte(byte value)
{
    os_error *err = NULL;

    // Check buffer details and size
    if (!parse_write_buffer) err = &err_bad_buffer;
    else if (parse_write_size < *parse_write_offset + 1) err = &err_buffer_full;
    else parse_write_buffer[(*parse_write_offset)++] = value;

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - The word value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_word(unsigned short value)
{
    os_error *err = NULL;

    // Check buffer details and size
    if (!parse_write_buffer) err = &err_bad_buffer;
    else if (parse_write_size < *parse_write_offset + sizeof(unsigned short))
    {
        err = &err_buffer_full;
    }
    else
    {
        parse_write_buffer[(*parse_write_offset)++] = value;
        parse_write_buffer[(*parse_write_offset)++] = value >> 8;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - The bits value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_bits(bits value)
{
    os_error *err = NULL;

    // Check buffer details and size
    if (!parse_write_buffer) err = &err_bad_buffer;
    else if (parse_write_size < *parse_write_offset + sizeof(bits))
    {
        err = &err_buffer_full;
    }
    else
    {
        parse_write_buffer[(*parse_write_offset)++] = value;
        parse_write_buffer[(*parse_write_offset)++] = value >> 8;
        parse_write_buffer[(*parse_write_offset)++] = value >> 16;
        parse_write_buffer[(*parse_write_offset)++] = value >> 24;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - The string value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_string(const char *value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_write_buffer) err = &err_bad_buffer;
    else
    {
        bits len = strlen(value) + 1;

        // Add the string to the frame
        if (parse_write_size < *parse_write_offset + len)
        {
            err = &err_buffer_full;
        }
        else
        {
            strcpy((char *) &parse_write_buffer[*parse_write_offset], value);
            *parse_write_offset += len;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : len           - The number of characters to append.
                  value         - The string value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_string_len(bits len, const char *value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_write_buffer) err = &err_bad_buffer;
    else if (parse_write_size < *parse_write_offset + len)
    {
        err = &err_buffer_full;
    }
    else
    {
        strncpy((char *) &parse_write_buffer[*parse_write_offset], value, len);
        *parse_write_offset += len;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : len           - The number of bytes to append.
                  value         - The data value to append.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Append the specified value to the current buffer.
*/
os_error *parse_put_bytes(bits len, const void *value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_write_buffer) err = &err_bad_buffer;
    else if (parse_write_size < *parse_write_offset + len)
    {
        err = &err_buffer_full;
    }
    else
    {
        memcpy(&parse_write_buffer[*parse_write_offset], value, len);
        *parse_write_offset += len;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : boundary      - The alignment in bytes (usually 2 or 4).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Advance the buffer pointer to the next boundary.
*/
os_error *parse_put_align(bits boundary)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!boundary) err = &err_bad_parms;
    else if (!parse_write_buffer) err = &err_bad_buffer;
    else
    {
        // Pad to the required boundary
        while (!err && (*parse_write_offset % boundary))
        {
            err = parse_put_byte(0);
        }
    }

    // Return any error produced
    return err;
}

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
os_error *parse_get_start(const byte *buffer, bits size, bits *offset)
{
    os_error *err = NULL;

    // Check parameters
    if (!buffer || (offset && (size < *offset))) err = &err_bad_parms;
    else
    {
        // Store the buffer details
        parse_read_buffer = buffer;
        parse_read_size = size;
        parse_read_offset = offset ?  offset : &parse_read_offset_value;

        // Initialise the internal buffer pointer
        parse_read_offset_value = 0;
    }

    // Return any error produced
    return err;
}

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
os_error *parse_get_start_frame(const frame_data *frame, bits *offset)
{
    os_error *err = NULL;

    // Check parameters
    if (!frame) err = &err_bad_parms;
    else err = parse_get_start(frame->data, FRAME_MAX_DATA_RX, offset);

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - Variable to receive the next byte value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next byte value from the current buffer.
*/
os_error *parse_get_byte(byte *value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_read_buffer) err = &err_bad_buffer;
    else if (parse_read_size < *parse_read_offset + 1) err = &err_buffer_end;
    else *value = parse_read_buffer[(*parse_read_offset)++];

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - Variable to receive the next word value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next word value from the current buffer.
*/
os_error *parse_get_word(unsigned short *value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_read_buffer) err = &err_bad_buffer;
    else if (parse_read_size < *parse_read_offset + sizeof(unsigned short))
    {
        err = &err_buffer_end;
    }
    else
    {
        *value = (unsigned short) parse_read_buffer[(*parse_read_offset)++];
        *value |= (unsigned short) parse_read_buffer[(*parse_read_offset)++] << 8;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - Variable to receive the next bits value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read the next bits value from the current buffer.
*/
os_error *parse_get_bits(bits *value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_read_buffer) err = &err_bad_buffer;
    else if (parse_read_size < *parse_read_offset + sizeof(bits))
    {
        err = &err_buffer_end;
    }
    else
    {
        *value = (bits) parse_read_buffer[(*parse_read_offset)++];
        *value |= (bits) parse_read_buffer[(*parse_read_offset)++] << 8;
        *value |= (bits) parse_read_buffer[(*parse_read_offset)++] << 16;
        *value |= (bits) parse_read_buffer[(*parse_read_offset)++] << 24;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - Variable to receive a pointer to the next
                                  string value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Obtain a pointer to the next string value from the current
                  buffer. The string is not copied from the buffer.
*/
os_error *parse_get_string(const char **value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_read_buffer) err = &err_bad_buffer;
    else
    {
        bits ptr = *parse_read_offset;

        // Attempt to find the end of the string
        while ((ptr < parse_read_size) && parse_read_buffer[ptr]) ptr++;

        // Check the string and destination buffer
        if (parse_read_size <= ptr) err = &err_bad_buffer;
        else
        {
            *value = (char *) &parse_read_buffer[*parse_read_offset];
            *parse_read_offset = ptr + 1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : len           - Number of characters to copy from the buffer.
                  value         - Variable to receive the string value,
                                  including a terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Copy the specified number of characters from the buffer and
                  add a terminator.
*/
os_error *parse_get_string_len(bits len, char *value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_read_buffer) err = &err_bad_buffer;
    else if (parse_read_size < *parse_read_offset + len)
    {
        err = &err_buffer_end;
    }
    else
    {
        strncpy(value, (char *) &parse_read_buffer[*parse_read_offset], len);
        value[len] = '\0';
        *parse_read_offset += len;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : len           - Number of bytes to copy from the buffer.
                  value         - Variable to receive the data value.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Copy the specified number of bytes from the buffer.
*/
os_error *parse_get_bytes(bits len, void *value)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!value) err = &err_bad_parms;
    else if (!parse_read_buffer) err = &err_bad_buffer;
    else if (parse_read_size < *parse_read_offset + len)
    {
        err = &err_buffer_end;
    }
    else
    {
        memcpy(value, &parse_read_buffer[*parse_read_offset], len);
        *parse_read_offset += len;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : boundary      - The alignment in bytes (usually 2 or 4).
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Advance the buffer pointer to the next boundary.
*/
os_error *parse_get_align(bits boundary)
{
    os_error *err = NULL;

    // Check parameters and buffer details
    if (!boundary) err = &err_bad_parms;
    else if (!parse_read_buffer) err = &err_bad_buffer;
    else
    {
        bits ptr = ((*parse_read_offset + boundary - 1) / boundary) * boundary;

        // Update the buffer pointer
        if (parse_read_size < ptr) err = &err_buffer_full;
        else *parse_read_offset = ptr;
    }

    // Return any error produced
    return err;
}
