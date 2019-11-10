/*
    File        : transform.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Transformation handling for the PsiFS filer.

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
#include "transform.h"

// Include clib header files
#include <math.h>

// Conversion factors
#define TRANSFORM_CONVERT_TWIP (32)     // 1 twip = 32 internal units
#define TRANSFORM_CONVERT_MM (1843.2)   // 1 mm = 7.2 x 256 internal units
#define TRANSFORM_CONVERT_POINT (640)   // 1 point = 640 internal units
#define TRANSFORM_CONVERT_OS (256)      // 1 OS unit = 256 internal units

// Standard scaling transformations from internal units
const transform transform_to_twips(1.0 / TRANSFORM_CONVERT_TWIP);
const transform transform_to_mm(1.0 / TRANSFORM_CONVERT_MM);
const transform transform_to_millipoint(1000.0 / TRANSFORM_CONVERT_POINT);
const transform transform_to_point16(16.0 / TRANSFORM_CONVERT_POINT);
const transform transform_to_os(1.0 / TRANSFORM_CONVERT_OS);

/*
    Parameters  : scale         - The scale factor to apply.
                  translation_x - The horizontal translation to apply.
                  translation_y - The vertical translation to apply.
                  invert_y      - Should the y axis be inverted.
    Returns     : -
    Description : Constructor.
*/
transform_base::transform_base(double scale, int translation_x,
                               int translation_y, bool invert_y)
{
    // Store the transformation
    scale_x = scale;
    scale_y = invert_y ? -scale : scale;
    offset_x = translation_x;
    offset_y = translation_y;
}

/*
    Parameters  : value         - The value to scale.
                  mode          - The rounding mode to use.
    Returns     : int           - The scaled value.
    Description : Apply the transformation.
*/
int transform_base::scale(int value, rounding_mode mode) const
{
    // Return the scaled value
    return round(double(value) * scale_x, mode);
}

/*
    Parameters  : x             - The horizontal value to transform.
                  mode          - The rounding mode to use.
    Returns     : int           - The transformed x value.
    Description : Apply the transformation.
*/
inline int transform_base::x(int x, rounding_mode mode) const
{
    // Return the transformed x value
    return round(double(x) * scale_x + offset_x, mode);
}

/*
    Parameters  : y             - The vertical value to transform.
                  mode          - The rounding mode to use.
    Returns     : int           - The transformed y value.
    Description : Apply the transformation.
*/
inline int transform_base::y(int y, rounding_mode mode) const
{
    // Return the transformed y value
    return round(double(y) * scale_y + offset_y, mode);
}

/*
    Parameters  : void
    Returns     : os_trfm       - An operating system transformation
                                  matrix.
    Description : Return the matrix corresponding to this transformation.
*/
os_trfm transform_base::matrix() const
{
    int matrix[6];

    // Construct the matrix
    matrix[0] = round(scale_x * double(1 << 16));
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = round(scale_y * double(1 << 16));
    matrix[4] = round(offset_x);
    matrix[5] = round(offset_y);

    // Return the matrix
    return *(os_trfm *) matrix;
}

/*
    Parameters  : void
    Returns     : transform_base    - The inverse transformation.
    Description : Invert the transformation.
*/
transform_base transform_base::inverse() const
{
    transform_base inverted;

    // Calculate the inverse transformation
    inverted.scale_x = 1.0 / scale_x;
    inverted.scale_y = 1.0 / scale_y;
    inverted.offset_x = -offset_x / scale_x;
    inverted.offset_y = -offset_y / scale_y;

    // Return the inverse transformation
    return inverted;
}

/*
    Parameters  : first             - The first transformation.
                  second            - The second transformation.
    Returns     : transform_base    - The composite transformation.
    Description : Combine two transformations.
*/
transform_base transform_base::combine(const transform_base &first,
                                       const transform_base &second)
{
    transform_base combined;

    // Calculate the combined transformation
    combined.scale_x = first.scale_x * second.scale_x;
    combined.scale_y = first.scale_y * second.scale_y;
    combined.offset_x = first.offset_x * second.scale_x + second.offset_x;
    combined.offset_y = first.offset_y * second.scale_y + second.offset_y;

    // Return the combined transformation
    return combined;
}

/*
    Parameters  : value         - The value to round.
                  round         - The rounding mode to use.
    Returns     : int           - The rounded value.
    Description : Apply the specified rounding mode.
*/
int transform_base::round(double value, rounding_mode round)
{
    // Perform the rounding
    int result;
    switch (round)
    {
        case round_up:
            result = int(ceil(value));
            break;

        case round_down:
            result = int(floor(value));
            break;

        case round_zero:
            result = int(value);
            break;

        case round_nearest:
        default:
            result = int(floor(value + 0.5));
            break;
    }

    // Return the result
    return result;
}

/*
    Parameters  : scale         - The scale factor to apply.
                  translation_x - The horizontal translation to apply.
                  translation_y - The vertical translation to apply.
                  invert_y      - Should the y axis be inverted.
    Returns     : -
    Description : Constructor.
*/
transform::transform(double scale, int translation_x, int translation_y,
                     bool invert_y)
{
    // Initialise the transformation
    init(transform_base(scale, translation_x, translation_y, invert_y));
}

/*
    Parameters  : redraw        - A window redraw rectangle.
                  percent       - The scale factor to apply.
    Returns     : -
    Description : Constructor.
*/
transform::transform(const wimp_draw &redraw, int percent)
{
    // Initialise the transformation
    init(transform_base(percent / 100.0, redraw.box.x0 - redraw.xscroll,
                        redraw.box.y1 - redraw.yscroll));
}

/*
    Parameters  : trfm          - The required transformation.
    Returns     : -
    Description : Constructor.
*/
transform::transform(const transform_base &trfm)
{
    // Initialise the transformation
    init(trfm);
}

/*
    Parameters  : identity          - Is this an identity transform
                  trfm              - The transformation
                  matrix            - The corresponding matrix
                  trfm_inverse      - The inverse transformation
                  matrix_inverse    - The corresponding matrix
    Returns     : -
    Description : Constructor.
*/
transform::transform(bool identity, const transform_base &trfm,
                     const os_trfm &matrix, const transform_base &trfm_inverse,
                     const os_trfm &matrix_inverse)
: identity(identity), trfm(trfm), matrix(matrix),
  trfm_inverse(trfm_inverse), matrix_inverse(matrix_inverse)
{
    // No action required
}

/*
    Parameters  : trfm          - The required transformation.
    Returns     : void
    Description : Initialise the transformation.
*/
void transform::init(const transform_base &trfm)
{
    // Store the transformation
    this->trfm = trfm;

    // Calculate the inverse transformation
    trfm_inverse = trfm.inverse();

    // Cache the transformation matrices
    matrix = trfm.matrix();
    matrix_inverse = trfm_inverse.matrix();

    // Check if this is an identity transform
    const int *m = (const int *) &matrix;
    identity = (m[0] == (1 << 16)) && (m[3] == (1 << 16))
               && (m[4] == 0) && (m[5] == 0);
}

/*
    Parameters  : size          - The size to transform.
                  round         - The rounding mode to use.
    Returns     : int           - The transformed size.
    Description : Apply the transformation.
*/
int transform::operator()(int size, rounding_mode round) const
{
    // Apply the transformation
    if (identity) return size;
    return trfm.scale(size, map_round(round));
}

/*
    Parameters  : coord         - The coordinates to transform.
                  round         - The rounding mode to use.
    Returns     : int           - The transformed coordinates.
    Description : Apply the transformation.
*/
os_coord transform::operator()(const os_coord &coord, rounding_mode round) const
{
    // Handle the trivial case first
    if (identity) return coord;

    // Choose the rounding
    transform_base::rounding_mode round_mapped = map_round(round);

    // Apply the transformation
    os_coord transformed;
    transformed.x = trfm.x(coord.x, round_mapped);
    transformed.y = trfm.y(coord.y, round_mapped);

    // Return the result
    return transformed;
}

/*
    Parameters  : box           - The rectangle to transform.
                  round         - The rounding mode to use.
    Returns     : int           - The transformed rectangle.
    Description : Apply the transformation.
*/
os_box transform::operator()(const os_box &box, rounding_mode round) const
{
    // Handle the trivial case first
    if (identity) return box;

    // Choose the rounding
    transform_base::rounding_mode round_lower = map_round(round, type_lower);
    transform_base::rounding_mode round_upper = map_round(round, type_upper);

    // Apply the transformation
    os_box transformed;
    transformed.x0 = trfm.x(box.x0, round_lower);
    transformed.y0 = trfm.y(box.y0, round_lower);
    transformed.x1 = trfm.x(box.x1, round_upper);
    transformed.y1 = trfm.y(box.y1, round_upper);

    // Return the result
    return transformed;
}

/*
    Parameters  : void
    Returns     : os_trfm       - An operating system transformation
                                  matrix, or NULL if it would be the
                                  identity.
    Description : Return the matrix corresponding to this transformation.
*/
transform::operator const os_trfm *() const
{
    // Return the matrix
    return identity ? NULL : &matrix;
}

/*
    Parameters  : void
    Returns     : transform     - The inverse transformation.
    Description : Construct the inverse transformation.
*/
transform transform::inverse() const
{
    // Return the inverse transformation
    return transform(identity, trfm_inverse, matrix_inverse, trfm, matrix);
}

/*
    Parameters  : size          - The size to transform.
                  round         - The rounding mode to use.
    Returns     : int           - The transformed size.
    Description : Apply the inverse transformation.
*/
int transform::inverse(int size, rounding_mode round) const
{
    // Apply the inverse transformation
    if (identity) return size;
    return trfm_inverse.scale(size, map_round(round));
}

/*
    Parameters  : coord         - The coordinates to transform.
                  round         - The rounding mode to use.
    Returns     : int           - The transformed coordinates.
    Description : Apply the inverse transformation.
*/
os_coord transform::inverse(const os_coord &coord, rounding_mode round) const
{
    // Handle the trivial case first
    if (identity) return coord;

    // Choose the rounding
    transform_base::rounding_mode round_mapped = map_round(round);

    // Apply the inverse transformation
    os_coord transformed;
    transformed.x = trfm_inverse.x(coord.x, round_mapped);
    transformed.y = trfm_inverse.y(coord.y, round_mapped);

    // Return the result
    return transformed;
}

/*
    Parameters  : box           - The rectangle to transform.
                  round         - The rounding mode to use.
    Returns     : int           - The transformed rectangle.
    Description : Apply the inverse transformation.
*/
os_box transform::inverse(const os_box &box, rounding_mode round) const
{
    // Handle the trivial case first
    if (identity) return box;

    // Choose the rounding
    transform_base::rounding_mode round_lower = map_round(round, type_lower);
    transform_base::rounding_mode round_upper = map_round(round, type_upper);

    // Apply the inverse transformation
    os_box transformed;
    transformed.x0 = trfm_inverse.x(box.x0, round_lower);
    transformed.y0 = trfm_inverse.y(box.y0, round_lower);
    transformed.x1 = trfm_inverse.x(box.x1, round_upper);
    transformed.y1 = trfm_inverse.y(box.y1, round_upper);

    // Return the result
    return transformed;
}

/*
    Parameters  : first         - The first transformation.
                  second        - The second transformation.
    Returns     : transform     - The composite transformation.
    Description : Combine two transformations.
*/
transform transform::combine(const transform &first, const transform &second)
{
    // Combine the transformations
    return transform(transform_base::combine(first.trfm, second.trfm));
}

/*
    Parameters  : round         - The requested rounding mode to use.
                  type          - The type of transformation.
    Returns     : rounding_mode - The actual rounding mode to use.
    Description : Choose a rounding mode.
*/
transform_base::rounding_mode transform::map_round(rounding_mode mode,
                                                   transformation_type type)
{
    // Map the rounding mode
    transform_base::rounding_mode result;
    switch (mode)
    {
        case round_up:
            result = transform_base::round_up;
            break;

        case round_down:
            result = transform_base::round_down;
            break;

        case round_zero:
            result = transform_base::round_zero;
            break;

        case round_out:
            if (type == type_lower) result = transform_base::round_down;
            else if (type == type_upper) result = transform_base::round_up;
            else result = transform_base::round_nearest;
            break;

        case round_in:
            if (type == type_lower) result = transform_base::round_up;
            else if (type == type_upper) result = transform_base::round_down;
            else result = transform_base::round_nearest;
            break;

        case round_nearest:
        default:
            result = transform_base::round_nearest;
            break;
    }

    // Return the result
    return result;
}

/*
    Parameters  : -
    Returns     : -
    Description : Constructor.
*/
transform_internal::transform_internal()
: trfm_millipoint(transform_to_millipoint),
  trfm_os(transform_to_os)
{
    // No action required
}

/*
    Parameters  : redraw        - A window redraw rectangle.
                  percent       - The scale factor to apply.
    Returns     : -
    Description : Constructor.
*/
transform_internal::transform_internal(const wimp_draw &redraw, int percent)
{
    // Start with the basic transformations
    transform_internal basic;

    // Construct the transformation from work area to screen coordinates
    transform trfm_redraw(redraw, percent);
    trfm_os = transform::combine(basic.to_os(), trfm_redraw);
    trfm_internal = transform::combine(trfm_os, basic.to_os().inverse());

    // Construct the composite transformations
    trfm_millipoint = transform::combine(trfm_internal, basic.to_millipoint());
}

/*
    Parameters  : void
    Returns     : transform     - The transformation.
    Description : Transformation from internal units to transformed
                  internal units.
*/
const transform &transform_internal::to_internal() const
{
    return trfm_internal;
}

/*
    Parameters  : void
    Returns     : transform     - The transformation.
    Description : Transformation from internal units to millipoint.
*/
const transform &transform_internal::to_millipoint() const
{
    return trfm_millipoint;
}

/*
    Parameters  : void
    Returns     : transform     - The transformation.
    Description : Transformation from internal units to operating system
                  units.
*/
const transform &transform_internal::to_os() const
{
    return trfm_os;
}
