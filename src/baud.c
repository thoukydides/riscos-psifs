/*
    File        : baud.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Baud rate list handling functions for the PsiFS module.

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
#include "baud.h"

// Recommended baud rates
blockdrive_speeds baud_recommended = {9600, 19200, 38400, 57600, 115200, 0};

/*
    Parameters  : list  - The list of baud rates to check.
                  value - The baud rate to search for.
    Returns     : bits  - The number of entries in the resulting list.
    Description : Check whether the specified baud rate is in the list.
*/
static bool baud_find(const blockdrive_speeds list, bits value)
{
    bits index = 0;

    // Search the list
    while (list[index] && (list[index] != value)) index++;

    // Return whether the entry was found
    return list[index];
}

/*
    Parameters  : driver        - The block driver to extract the list of baud
                                  rates from.
                  restrict      - A list of baud rates to filter, or NULL for
                                  no restriction.
                  target        - The required baud rate, or 0 to ignore.
                  list          - Variable to receive the resulting list.
    Returns     : bits          - The number of entries in the resulting list.
    Description : Construct a processed list of baud rates.
*/
bits baud_list(const blockdrive_driver driver,
               const blockdrive_speeds restrict, bits target,
               blockdrive_speeds list)
{
    bits from;
    bits count = 0;

    // Process each of the speeds supported by the block driver
    for (from = 0; driver->speeds[from]; from++)
    {
        bits baud = driver->speeds[from];

        // Check if this speed is allowed
        if ((!restrict || baud_find(restrict, baud))
            && (!target || (target == baud)))
        {
            // Add this to the output list
            list[count++] = baud;
        }
    }

    // Terminate the list
    list[count] = 0;

    // Return the number of entries
    return count;
}
