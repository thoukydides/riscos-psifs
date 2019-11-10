/*
    File        : baud.h
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

// Only include header file once
#ifndef BAUD_H
#define BAUD_H

// Include oslib header files
#include "oslib/types.h"

// Include project header files
#include "blockdrive.h"

// Recommended baud rates
extern blockdrive_speeds baud_recommended;

#ifdef __cplusplus
    extern "C" {
#endif

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
               blockdrive_speeds list);

#ifdef __cplusplus
    }
#endif

#endif
