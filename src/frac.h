/*
    File        : frac.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Manipulation of fractional values for the PsiFS module.

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
#ifndef FRAC_H
#define FRAC_H

// Include oslib header files
#include "oslib/os.h"

// Type to hold a fractional value
typedef bits frac_value;

#ifdef __cplusplus
    extern "C" {
#endif

// Special values
#define FRAC_ZERO (0)
#define FRAC_ONE (1u << 31)

/*
    Parameters  : numerator     - The numerator of the fraction.
                  denominator   - The denominator of the fraction.
    Returns     : frac_value    - The corresponding fractional value.
    Description : Convert the specified integer division into a fraction. The
                  result is clipped to the range 0.0 to 1.0.
*/
frac_value frac_create(bits numerator, bits denominator);

/*
    Parameters  : value1        - The first fractional value.
                  value2        - The second fractional value.
    Returns     : frac_value    - The result of the multiplication.
    Description : Multiply two fractional values.
*/
frac_value frac_multiply(frac_value value1, frac_value value2);

/*
    Parameters  : value1        - The first fractional value.
                  value2        - The second fractional value.
    Returns     : frac_value    - The result of the addition.
    Description : Add two fractional values. The result is clipped if it
                  exceeds 1.0.
*/
frac_value frac_add(frac_value value1, frac_value value2);

/*
    Parameters  : value         - The fractional value.
    Returns     : frac_value    - The result of the calculation.
    Description : Subtract the fractional value from 1.0.
*/
frac_value frac_not(frac_value value);

/*
    Parameters  : value - The integer.
                  frac  - The fractional value.
    Returns     : bits  - The result of the multiplication.
    Description : Multiply an integer by a fractional value.
*/
bits frac_scale(bits value, frac_value frac);

/*
    Parameters  : value - The integer.
                  frac  - The fractional value.
    Returns     : bits  - The result of the division.
    Description : Divide an integer by a fractional value. If the result would
                  be likely to overflow then zero is returned.
*/
bits frac_inv_scale(bits value, frac_value frac);

/*
    Parameters  : value1    - The first fractional value.
                  value2    - The second fractional value.
    Returns     : int       - The result of the comparison:
                                < 0 if value1 < value2
                                 0  if value1 == value2
                                > 0 if value1 > value2
    Description : Compare two fractional values.
*/
int frac_cmp(frac_value value1, frac_value value2);

#ifdef __cplusplus
    }
#endif

#endif
