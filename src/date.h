/*
    File        : date.h
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

// Only include header file once
#ifndef DATE_H
#define DATE_H

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "epoc16.h"
#include "epoc32.h"

// A RISC OS 5 byte date and time
typedef union
{
    os_date_and_time bytes;
    struct
    {
        bits low;
        bits high;
    } words;
} date_riscos;

// A UNIX date and time
typedef bits date_unix;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : date          - The ERA format date to convert.
    Returns     : date_riscos   - The date in 5 byte RISC OS format.
    Description : Convert a date and time from ERA to RISC OS format.
*/
const date_riscos *date_from_era(const epoc32_file_time *date);

/*
    Parameters  : date              - The date in 5 byte RISC OS format.
    Returns     : epoc32_file_time  - The ERA format result.
    Description : Convert a date and time from RISC OS to ERA format.
*/
const epoc32_file_time *date_to_era(const date_riscos *date);

/*
    Parameters  : date          - The SIBO format date to convert.
    Returns     : date_riscos   - The date in 5 byte RISC OS format.
    Description : Convert a date and time from SIBO to RISC OS format.
*/
const date_riscos *date_from_sibo(epoc16_file_time date);

/*
    Parameters  : date              - The date in 5 byte RISC OS format.
    Returns     : epoc16_file_time  - The SIBO format result.
    Description : Convert a date and time from RISC OS to SIBO format.
*/
epoc16_file_time date_to_sibo(const date_riscos *date);

/*
    Parameters  : date          - The UNIX format date to convert.
    Returns     : date_riscos   - The date in 5 byte RISC OS format.
    Description : Convert a date and time from UNIX to RISC OS format.
*/
const date_riscos *date_from_unix(date_unix date);

/*
    Parameters  : date          - The date in 5 byte RISC OS format.
    Returns     : date_unix     - The UNIX format result.
    Description : Convert a date and time from RISC OS to UNIX format.
*/
date_unix date_to_unix(const date_riscos *date);

#ifdef __cplusplus
    }
#endif

#endif
