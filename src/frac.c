/*
    File        : frac.c
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

// Include header file for this module
#include "frac.h"

// Include oslib header files
#include "oslib/macros.h"

// Configuration of the fractional values
#define FRAC_BITS (32)
#define FRAC_SHIFT (31)

/*
    Parameters  : value - The number to check.
    Returns     : int   - The number of bits used.
    Description : Count the number of bits used by the specified value.
*/
static int frac_bits(bits value)
{
    bits count = 0;

    // Count the number of bits
    while (value)
    {
        count++;
        value >>= 1;
    }

    // Return the result
    return count;
}

/*
    Parameters  : value - The number to shift.
                  shift - The number of places to shift. This may be negative
                          to specify a right shift.
    Returns     : bits  - The shifted number.
    Description : Left shift the value by the specified number of places.
*/
static bits frac_left_shift(bits value, int shift)
{
    // Return the result
    return shift < 0
           ? (shift < -31 ? 0 : value >> -shift)
           : (0 < shift
              ? (31 < shift ? 0 : value << shift)
              : value);
}

/*
    Parameters  : value1        - The first fractional value.
                  value2        - The second fractional value.
                  shift         - The number of places to left shift the result.
    Returns     : frac_value    - The result of the multiplication.
    Description : Multiply two values.
*/
static bits frac_multiply_raw(bits value1, bits value2, int shift)
{
    int used = frac_bits(value1) + frac_bits(value2);

    // Pre-scale the values if necessary
    while (FRAC_BITS < used)
    {
        if (!(value2 & 1) || ((value1 & 1) && (value1 < value2))) value2 >>= 1;
        else value1 >>= 1;
        used--;
        shift++;
    }

    // Return the result
    return frac_left_shift(value1 * value2, shift);
}

/*
    Parameters  : numerator     - The numerator of the fraction.
                  denominator   - The denominator of the fraction.
    Returns     : frac_value    - The corresponding fractional value.
    Description : Convert the specified integer division into a fraction. The
                  result is clipped to the range 0.0 to 1.0.
*/
frac_value frac_create(bits numerator, bits denominator)
{
    int shift = FRAC_BITS - frac_bits(numerator);

    // Range check the parameters
    if (denominator < numerator) denominator = numerator;
    else if (!denominator) denominator = 1;

    // Return the result
    return frac_left_shift(frac_left_shift(numerator, shift) / denominator,
                           FRAC_SHIFT - shift);
}

/*
    Parameters  : value1        - The first fractional value.
                  value2        - The second fractional value.
    Returns     : frac_value    - The result of the multiplication.
    Description : Multiply two fractional values.
*/
frac_value frac_multiply(frac_value value1, frac_value value2)
{
    // Return the result
    return frac_multiply_raw(value1, value2, -FRAC_SHIFT);
}

/*
    Parameters  : value1        - The first fractional value.
                  value2        - The second fractional value.
    Returns     : frac_value    - The result of the addition.
    Description : Add two fractional values.
*/
frac_value frac_add(frac_value value1, frac_value value2)
{
    bits result = value1 + value2;

    // Return the result
    return result < FRAC_ONE ? result : FRAC_ONE;
}

/*
    Parameters  : value         - The fractional value.
    Returns     : frac_value    - The result of the calculation.
    Description : Subtract the fractional value from 1.0.
*/
frac_value frac_not(frac_value value)
{
    // Return the result
    return FRAC_ONE - value;
}

/*
    Parameters  : value - The integer.
                  frac  - The fractional value.
    Returns     : bits  - The result of the multiplication.
    Description : Multiply an integer by a fractional value.
*/
bits frac_scale(bits value, frac_value frac)
{
    // Return the result
    return frac_multiply_raw(value, frac, -FRAC_SHIFT);
}

/*
    Parameters  : value - The integer.
                  frac  - The fractional value.
    Returns     : bits  - The result of the division.
    Description : Divide an integer by a fractional value. If the result would
                  be likely to overflow then zero is returned.
*/
bits frac_inv_scale(bits value, frac_value frac)
{
    bits shift = FRAC_BITS - frac_bits(value);

    // Shift the values as appropriate
    value = frac_left_shift(value, shift);
    frac = frac_left_shift(frac, shift - FRAC_SHIFT);

    // Return the result
    return frac ? value / frac : 0;
}

/*
    Parameters  : value1    - The first fractional value.
                  value2    - The second fractional value.
    Returns     : int       - The result of the comparison:
                                < 0 if value1 < value2
                                 0  if value1 == value2
                                > 0 if value1 > value2
    Description : Compare two fractional values.
*/
int frac_cmp(frac_value value1, frac_value value2)
{
    // Return the result of the comparison
    return value1 < value2 ? -1 : (value2 < value1 ? 1 : 0);
}
