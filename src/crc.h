/*
    File        : crc.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Cyclic redundancy check (CRC) calculation for the PsiFS
                  module. This uses the standard X^16 + X^12 + X^5 + 1
                  polynomial to produce a two byte CRC.

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
#ifndef CRC_H
#define CRC_H

// Include oslib header files
#include "oslib/types.h"

// Type to store the state of a CRC calculation
typedef bits crc_state;

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : crc   - The CRC state to reset.
    Returns     : void
    Description : Reset the specified CRC state ready for a new calculation.
*/
void crc_reset(crc_state *crc);

/*
    Parameters  : crc   - The CRC state to update.
    Returns     : void
    Description : Update the specified CRC state with a single byte value.
*/
void crc_update(crc_state *crc, byte value);

/*
    Parameters  : crc   - The CRC state to process
    Returns     : byte  - The low byte of the CRC value.
    Description : Return the least significant byte of the result.
*/
byte crc_lsb(const crc_state *crc);

/*
    Parameters  : crc   - The CRC state to process
    Returns     : byte  - The high byte of the CRC value.
    Description : Return the most significant byte of the result.
*/
byte crc_msb(const crc_state *crc);

#ifdef __cplusplus
    }
#endif

#endif
