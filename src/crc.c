/*
    File        : crc.c
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

// Include header file for this module
#include "crc.h"

// Table of values to calculate CRC efficiently
static bits crc_table[256];
static bool crc_table_init = FALSE;

// Polynomial X^16 + X^12 + X^5 + 1
static bits crc_polynomial = 0x1021;

/*
    Parameters  : crc   - The CRC state to reset.
    Returns     : void
    Description : Reset the specified CRC state ready for a new calculation.
*/
void crc_reset(crc_state *crc)
{
    // Reset the value to zero
    *crc = 0;

    // Construct table on first use
    if (!crc_table_init)
    {
        bits i;

        // Initialise the table
        crc_table[0] = 0;
        for (i = 0; i < 128; i++)
        {
            bool carry = crc_table[i] & 0x8000;
            bits t = (crc_table[i] << 1) & 0xffff;
            crc_table[i * 2 + (carry ? 0 : 1)] = t ^ crc_polynomial;
            crc_table[i * 2 + (carry ? 1 : 0)] = t;
        }

        // Set the initialised flag
        crc_table_init = TRUE;
    }
}

/*
    Parameters  : crc   - The CRC state to update.
    Returns     : void
    Description : Update the specified CRC state with a single byte value.
*/
void crc_update(crc_state *crc, byte value)
{
    // Update the CRC with this value
    *crc = (*crc << 8) ^ crc_table[((*crc >> 8) ^ value) & 0xff];
}

/*
    Parameters  : crc   - The CRC state to process
    Returns     : byte  - The low byte of the CRC value.
    Description : Return the least significant byte of the result.
*/
byte crc_lsb(const crc_state *crc)
{
    // Return the low byte
    return *crc & 0xff;
}

/*
    Parameters  : crc   - The CRC state to process
    Returns     : byte  - The high byte of the CRC value.
    Description : Return the most significant byte of the result.
*/
byte crc_msb(const crc_state *crc)
{
    // Return the high byte
    return (*crc & 0xff00) >> 8;
}
