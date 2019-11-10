/*
    File        : transform.h
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

// Only include header file once
#ifndef TRANSFORM_H
#define TRANSFORM_H

// Include oslib header files
#include "oslib/os.h"
#include "oslib/wimp.h"

// A class to represent a simple transformation
class transform_base
{
public:

    // Rounding modes
    enum rounding_mode
    {
        round_nearest,
        round_up,
        round_down,
        round_zero
    };

    /*
        Parameters  : scale         - The scale factor to apply.
                      translation_x - The horizontal translation to apply.
                      translation_y - The vertical translation to apply.
                      invert_y      - Should the y axis be inverted.
        Returns     : -
        Description : Constructor.
    */
    transform_base(double scale = 1.0, int translation_x = 0,
                   int translation_y = 0, bool invert_y = FALSE);

    /*
        Parameters  : value         - The value to scale.
                      mode          - The rounding mode to use.
        Returns     : int           - The scaled value.
        Description : Apply the transformation.
    */
    int scale(int value, rounding_mode mode = round_nearest) const;

    /*
        Parameters  : x             - The horizontal value to transform.
                      mode          - The rounding mode to use.
        Returns     : int           - The transformed x value.
        Description : Apply the transformation.
    */
    int x(int x, rounding_mode mode = round_nearest) const;

    /*
        Parameters  : y             - The vertical value to transform.
                      mode          - The rounding mode to use.
        Returns     : int           - The transformed y value.
        Description : Apply the transformation.
    */
    int y(int y, rounding_mode mode = round_nearest) const;

    /*
        Parameters  : void
        Returns     : os_trfm       - An operating system transformation
                                      matrix.
        Description : Return the matrix corresponding to this transformation.
    */
    os_trfm matrix() const;

    /*
        Parameters  : void
        Returns     : transform_base    - The inverse transformation.
        Description : Invert the transformation.
    */
    transform_base inverse() const;

    /*
        Parameters  : first             - The first transformation.
                      second            - The second transformation.
        Returns     : transform_base    - The composite transformation.
        Description : Combine two transformations.
    */
    static transform_base combine(const transform_base &first,
                                  const transform_base &second);

    /*
        Parameters  : value         - The value to round.
                      round         - The rounding mode to use.
        Returns     : int           - The rounded value.
        Description : Apply the specified rounding mode.
    */
    static int round(double value, rounding_mode round = round_nearest);

private:

    double scale_x;                     // The horizontal scale factor
    double scale_y;                     // The vertical scale factor
    double offset_x;                    // The horizontal offset
    double offset_y;                    // The vertical offset
};

// A class to represent a pre-computed transformation
class transform
{
public:

    // Rounding modes
    enum rounding_mode
    {
        round_nearest,
        round_up,
        round_down,
        round_zero,
        round_out,
        round_in
    };

    /*
        Parameters  : scale         - The scale factor to apply.
                      translation_x - The horizontal translation to apply.
                      translation_y - The vertical translation to apply.
                      invert_y      - Should the y axis be inverted.
        Returns     : -
        Description : Constructor.
    */
    transform(double scale = 1.0, int translation_x = 0, int translation_y = 0,
              bool invert_y = FALSE);

    /*
        Parameters  : redraw        - A window redraw rectangle.
                      percent       - The scale factor to apply.
        Returns     : -
        Description : Constructor.
    */
    transform(const wimp_draw &redraw, int percent = 100);

    /*
        Parameters  : size          - The size to transform.
                      round         - The rounding mode to use.
        Returns     : int           - The transformed size.
        Description : Apply the transformation.
    */
    int operator()(int size, rounding_mode round = round_nearest) const;

    /*
        Parameters  : coord         - The coordinates to transform.
                      round         - The rounding mode to use.
        Returns     : int           - The transformed coordinates.
        Description : Apply the transformation.
    */
    os_coord operator()(const os_coord &coord,
                        rounding_mode round = round_nearest) const;

    /*
        Parameters  : box           - The rectangle to transform.
                      round         - The rounding mode to use.
        Returns     : int           - The transformed rectangle.
        Description : Apply the transformation.
    */
    os_box operator()(const os_box &box,
                      rounding_mode round = round_nearest) const;

    /*
        Parameters  : void
        Returns     : os_trfm       - An operating system transformation
                                      matrix, or NULL if it would be the
                                      identity.
        Description : Return the matrix corresponding to this transformation.
    */
    operator const os_trfm *() const;

    /*
        Parameters  : void
        Returns     : transform     - The inverse transformation.
        Description : Construct the inverse transformation.
    */
    transform inverse() const;

    /*
        Parameters  : size          - The size to transform.
                      round         - The rounding mode to use.
        Returns     : int           - The transformed size.
        Description : Apply the inverse transformation.
    */
    int inverse(int size, rounding_mode round = round_nearest) const;

    /*
        Parameters  : coord         - The coordinates to transform.
                      round         - The rounding mode to use.
        Returns     : int           - The transformed coordinates.
        Description : Apply the inverse transformation.
    */
    os_coord inverse(const os_coord &coord,
                     rounding_mode round = round_nearest) const;

    /*
        Parameters  : box           - The rectangle to transform.
                      round         - The rounding mode to use.
        Returns     : int           - The transformed rectangle.
        Description : Apply the inverse transformation.
    */
    os_box inverse(const os_box &box,
                   rounding_mode round = round_nearest) const;

    /*
        Parameters  : first         - The first transformation.
                      second        - The second transformation.
        Returns     : transform     - The composite transformation.
        Description : Combine two transformations.
    */
    static transform combine(const transform &first, const transform &second);

private:

    // Types of transformation
    enum transformation_type
    {
        type_single,
        type_lower,
        type_upper
    };

    bool identity;                      // Is this an identity transform
    transform_base trfm;                // The transformation
    os_trfm matrix;                     // The corresponding matrix
    transform_base trfm_inverse;        // The inverse transformation
    os_trfm matrix_inverse;             // The corresponding matrix

    /*
        Parameters  : trfm          - The required transformation.
        Returns     : -
        Description : Constructor.
    */
    transform(const transform_base &trfm);

    /*
        Parameters  : identity          - Is this an identity transform
                      trfm              - The transformation
                      matrix            - The corresponding matrix
                      trfm_inverse      - The inverse transformation
                      matrix_inverse    - The corresponding matrix
        Returns     : -
        Description : Constructor.
    */
    transform(bool identity, const transform_base &trfm, const os_trfm &matrix,
              const transform_base &trfm_inverse,
              const os_trfm &matrix_inverse);

    /*
        Parameters  : trfm          - The required transformation.
        Returns     : void
        Description : Initialise the transformation.
    */
    void init(const transform_base &trfm);

    /*
        Parameters  : round         - The requested rounding mode to use.
                      type          - The type of transformation.
        Returns     : rounding_mode - The actual rounding mode to use.
        Description : Choose a rounding mode.
    */
    static transform_base::rounding_mode map_round(rounding_mode mode, transformation_type type = type_single);
};

// A class to represent transformations from internal to other units
class transform_internal
{
public:

    /*
        Parameters  : -
        Returns     : -
        Description : Constructor.
    */
    transform_internal();

    /*
        Parameters  : redraw        - A window redraw rectangle.
                      percent       - The scale factor to apply.
        Returns     : -
        Description : Constructor.
    */
    transform_internal(const wimp_draw &redraw, int percent = 100);

    /*
        Parameters  : void
        Returns     : transform     - The transformation.
        Description : Transformation from internal units to transformed
                      internal units.
    */
    const transform &to_internal() const;

    /*
        Parameters  : void
        Returns     : transform     - The transformation.
        Description : Transformation from internal units to millipoint.
    */
    const transform &to_millipoint() const;

    /*
        Parameters  : void
        Returns     : transform     - The transformation.
        Description : Transformation from internal units to 1/16 point.
    */
    const transform &to_point16() const;

    /*
        Parameters  : void
        Returns     : transform     - The transformation.
        Description : Transformation from internal units to operating system
                      units.
    */
    const transform &to_os() const;

private:

    transform trfm_internal;            // Transformation to internal units
    transform trfm_millipoint;          // Transformation to millipoint
    transform trfm_os;                  // Transformation to OS units
};

// Standard scaling transformations from internal units
extern const transform transform_to_twips;
extern const transform transform_to_mm;
extern const transform transform_to_millipoint;
extern const transform transform_to_point16;
extern const transform transform_to_os;

#endif
