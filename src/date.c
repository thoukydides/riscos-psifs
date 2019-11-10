/*
    File        : date.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Date and time stamp conversions for the PsiFS module.

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
#include "date.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/territory.h"

// 1 hour in centi-seconds
#define DATA_OFFSET_HOUR (0x00057e40)

// 1900 years in centi-seconds
#define DATE_ERA_OFFSET_LOW (0x09150e00)
#define DATE_ERA_OFFSET_HIGH (0x574)

// 70 years less 11 hours in centi-seconds
#define DATE_SIBO_OFFSET_LOW (0x6e9ee840)
#define DATE_SIBO_OFFSET_HIGH (0x33)

// 70 years less 12 hours in centi-seconds
#define DATE_UNIX_OFFSET_LOW (0x6e996a00)
#define DATE_UNIX_OFFSET_HIGH (0x33)

/*
    Parameters  : value     - The date and time to modify.
                  low       - The low word of the offset.
                  high      - The high word of the offset.
    Returns     : void
    Description : Add an offset to the specified date and time. No overflow
                  check is performed.
*/
static void date_add(date_riscos *value, bits low, bits high)
{
    // Perform the addition
    value->words.low += low;
    if (value->words.low < low) value->words.high++;
    value->words.high += high;
}

/*
    Parameters  : value     - The date and time to modify.
                  low       - The low word of the offset.
                  high      - The high word of the offset.
    Returns     : void
    Description : Subtract an offset from the specified date and time. Underflow
                  checking is performed.
*/
static void date_sub(date_riscos *value, bits low, bits high)
{
    // Check for overflow
    if ((value->words.high < high)
        || ((value->words.high == high) && (value->words.low < low)))
    {
        // Clip the result
        value->words.low = value->words.high = 0;
    }
    else
    {
        // Perform the subtraction
        value->words.high -= high;
        if (value->words.low < low) value->words.high--;
        value->words.low -= low;
    }
}

/*
    Parameters  : value     - The date and time to modify.
                  factor    - The factor to divide by.
    Returns     : void
    Description : Divide the specified date and time by a factor.
*/
static void date_div(date_riscos *value, bits factor)
{
    // Perform the division
    bits mid = ((value->words.high % factor) << 16) + (value->words.low >> 16);
    value->words.high /= factor;
    value->words.low = ((value->words.low & 0xffff)
                        + ((mid % factor) << 16)) / factor
                       + ((mid / factor) << 16);
}

/*
    Parameters  : value     - The date and time to modify.
                  factor    - The factor to multiply by.
    Returns     : void
    Description : Multiply the specified date and time by a factor. No overflow
                  check is performed.
*/
static void date_mul(date_riscos *value, bits factor)
{
    // Perform the multiplication
    bits mid = (value->words.low >> 16) * factor;
    value->words.low = (value->words.low & 0xffff) * factor;
    value->words.high = value->words.high * factor;
    date_add(value, mid << 16, mid >> 16);
}

/*
    Parameters  : value - The date and time to correct.
                  add   - Should the correction be added.
    Returns     : void
    Description : Add or subtract an offset for the current time zone.
*/
static void date_timezone(date_riscos *value, bool add)
{
    int offset;

    // Read the time zone offset
    if (xterritory_read_current_time_zone(NULL, &offset)) offset = 0;

    // Negate the offset if it should be subtracted
    if (!add) offset = -offset;

    // Add or subtract the offset as appropriate
    if (offset < 0) date_sub(value, -offset, 0);
    else date_add(value, offset, 0);
}

/*
    Parameters  : date          - The ERA format date to convert.
    Returns     : date_riscos   - The date in 5 byte RISC OS format.
    Description : Convert a date and time from ERA to RISC OS format.
*/
const date_riscos *date_from_era(const epoc32_file_time *date)
{
    static date_riscos value;

//    printf("ERA date: 0x%08x%08x\n", date->high, date->low);

    // Copy the date and time
    value.words.low = date->low;
    value.words.high = date->high;

    // Convert from micro-seconds to centi-seconds
    date_div(&value, 10000);

    // Change the base year from 1 to 1900
    date_sub(&value, DATE_ERA_OFFSET_LOW, DATE_ERA_OFFSET_HIGH);

    // Range check the result
    if (value.words.high & ~0xff)
    {
        value.words.low = 0xffffffff;
        value.words.high = 0xff;
    }

    // Include an extra hour if summer time
    date_timezone(&value, FALSE);

//    printf("-> RISC OS date: 0x%08x%08x\n\n", value.words.high, value.words.low);

    // Return the result
    return &value;
}

/*
    Parameters  : date              - The date in 5 byte RISC OS format.
    Returns     : epoc32_file_time  - The ERA format result.
    Description : Convert a date and time from RISC OS to ERA format.
*/
const epoc32_file_time *date_to_era(const date_riscos *date)
{
    static epoc32_file_time value;
    date_riscos work;

//    printf("RISC OS date: 0x%08x%08x\n", date->words.high, date->words.low);

    // Ensure that the unused bytes are all zero
    work.words.low = date->words.low;
    work.words.high = date->words.high & 0xff;

    // Include an extra hour if summer time
    date_timezone(&work, TRUE);

    // Change the base year from 1900 to 1
    date_add(&work, DATE_ERA_OFFSET_LOW, DATE_ERA_OFFSET_HIGH);

    // Convert from centi-seconds to micro-seconds
    date_mul(&work, 10000);

    // Copy the result
    value.low = work.words.low;
    value.high = work.words.high;

//    printf("-> ERA date: 0x%08x%08x\n\n", value.high, value.low);

    // Return the result
    return &value;
}

/*
    Parameters  : date          - The SIBO format date to convert.
    Returns     : date_riscos   - The date in 5 byte RISC OS format.
    Description : Convert a date and time from SIBO to RISC OS format.
*/
const date_riscos *date_from_sibo(epoc16_file_time date)
{
    static date_riscos value;

    // Copy the date and time
    value.words.low = date;
    value.words.high = 0;

    // Convert from seconds to centi-seconds
    date_mul(&value, 100);

    // Change the base year from 1970 to 1900
    date_add(&value, DATE_SIBO_OFFSET_LOW, DATE_SIBO_OFFSET_HIGH);

    // Include an extra hour if summer time
    date_timezone(&value, FALSE);

    // Return the result
    return &value;
}

/*
    Parameters  : date              - The date in 5 byte RISC OS format.
    Returns     : epoc16_file_time  - The SIBO format result.
    Description : Convert a date and time from RISC OS to SIBO format.
*/
epoc16_file_time date_to_sibo(const date_riscos *date)
{
    date_riscos work = *date;

    // Ensure that the unused bytes are all zero
    work.words.low = date->words.low;
    work.words.high = date->words.high & 0xff;

    // Include an extra hour if summer time
    date_timezone(&work, TRUE);

    // Change the base year from 1900 to 1970
    date_sub(&work, DATE_SIBO_OFFSET_LOW, DATE_SIBO_OFFSET_HIGH);

    // Convert from centi-seconds to seconds
    date_div(&work, 100);

    // Return the result
    return work.words.high ? 0xffffffff : work.words.low;
}

/*
    Parameters  : date          - The UNIX format date to convert.
    Returns     : date_riscos   - The date in 5 byte RISC OS format.
    Description : Convert a date and time from UNIX to RISC OS format.
*/
const date_riscos *date_from_unix(date_unix date)
{
    static date_riscos value;

    // Copy the date and time
    value.words.low = date;
    value.words.high = 0;

    // Convert from seconds to centi-seconds
    date_mul(&value, 100);

    // Change the base year from 1970 to 1900
    date_add(&value, DATE_UNIX_OFFSET_LOW, DATE_UNIX_OFFSET_HIGH);

    // Include an extra hour if summer time
    date_timezone(&value, FALSE);

    // Return the result
    return &value;
}

/*
    Parameters  : date          - The date in 5 byte RISC OS format.
    Returns     : date_unix     - The UNIX format result.
    Description : Convert a date and time from RISC OS to UNIX format.
*/
date_unix date_to_unix(const date_riscos *date)
{
    date_riscos work = *date;

    // Ensure that the unused bytes are all zero
    work.words.low = date->words.low;
    work.words.high = date->words.high & 0xff;

    // Include an extra hour if summer time
    date_timezone(&work, TRUE);

    // Change the base year from 1900 to 1970
    date_sub(&work, DATE_UNIX_OFFSET_LOW, DATE_UNIX_OFFSET_HIGH);

    // Convert from centi-seconds to seconds
    date_div(&work, 100);

    // Return the result
    return work.words.high ? 0xffffffff : work.words.low;
}
