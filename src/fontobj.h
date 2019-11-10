/*
    File        : fontobj.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Font handling for the PsiFS filer.

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
#ifndef FONTOBJ_H
#define FONTOBJ_H

// Include oslib header files
#include "oslib/toolbox.h"

// Include cathlibcpp header files
#include "string.h"

// Include project header files
#include "transform.h"

// A raw font handle
class fontobj_base
{
public:

    /*
        Parameters  : void
        Returns     : string            - The name of the font.
        Description : Return the name of the font.
    */
    string get_font() const;

    /*
        Parameters  : void
        Returns     : int               - The size of the font.
        Description : Return the size of the font.
    */
    int get_size() const;

    /*
        Parameters  : void
        Returns     : font_f            - The font handle.
        Description : Return the raw font handle.
    */
    operator font_f() const;

    /*
        Parameters  : position          - The underline position in 1/256 em.
                      thickness         - The underline thickness in 1/256 em.
        Returns     : bool              - Is underline information available.
        Description : Read the recommended underline position and thickness
                      from the font metrics.
    */
    bool get_underline(int &position, int &thickness) const;

    /*
        Parameters  : void
        Returns     : void
        Description : Increment the reference count.
    */
    void inc();

    /*
        Parameters  : void
        Returns     : void
        Description : Decrement the reference count.
    */
    void dec();

    /*
        Parameters  : font              - The name of the font.
                      size              - The size of the font in sixteenths
                                          of a point.
        Returns     : -
        Description : Obtain a font handle.
    */
    static fontobj_base *find(const string &font, int size);

    /*
        Parameters  : void
        Returns     : void
        Description : Lose all fonts.
    */
    static void lose_all();

private:

    string font;                        // Font identifier
    int size;                           // Font size in sixteenths of a point
    font_f handle;                      // Font handle
    bool underline;                     // Is underline information specified
    int underline_position;             // Underline position in 1/256 em
    int underline_thickness;            // Underline thickness in 1/256 em
    int count;                          // Reference count

    /*
        Parameters  : font              - The name of the font.
                      size              - The size of the font in sixteenths
                                          of a point.
        Returns     : -
        Description : Constructor.
    */
    fontobj_base(const string &font, int size);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~fontobj_base();
};

// A font handle handler
class fontobj_handle
{
public:

    /*
        Parameters  : font              - The name of the font.
                      size              - The size of the font in internal
                                          units.
        Returns     : -
        Description : Constructor.
    */
    fontobj_handle(const string &font, int size);

    /*
        Parameters  : handle            - The font handle.
        Returns     : -
        Description : Constructor.
    */
    fontobj_handle(const fontobj_handle &handle);

    /*
        Parameters  : handle            - The font handle to copy.
        Returns     : fontobj_handle    - A reference to this handle.
        Description : Handle assignment.
    */
    fontobj_handle &operator=(const fontobj_handle &handle);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~fontobj_handle();

    /*
        Parameters  : void
        Returns     : font_f            - The font handle.
        Description : Return the raw font handle.
    */
    operator font_f() const;

    /*
        Parameters  : bottom            - Offset from baseline to bottom of
                                          underline in internal units.
                      top               - Offset from baseline to top of
                                          underline in internal units.
        Returns     : void
        Description : Obtain the recommended underline position.
    */
    void get_underline(int &bottom, int &top) const;

    /*
        Parameters  : bottom            - Offset from baseline to bottom of
                                          strikethrough in internal units.
                      top               - Offset from baseline to top of
                                          strikethrough in internal units.
        Returns     : void
        Description : Obtain the recommended strikethrough position.
    */
    void get_strikethrough(int &bottom, int &top) const;

    /*
        Parameters  : text              - The text to process.
                      left              - The start position in internal units.
                      spacing           - Additional spacing between characters.
                      matrix            - Font transformation matrix.
        Returns     : os_box            - The bounding box in internal units.
        Description : Calculate the bouning box for the specified text.
    */
    os_box get_box(const string &text, const os_coord &left,
                   int spacing = 0, const os_trfm *matrix = NULL) const;

    /*
        Parameters  : text              - The text to process.
        Returns     : int               - The baseline width in internal units.
        Description : Calculate the baseline width for the specified text
                      without any additional spacing or transformations.
    */
    int get_width(const string &text) const;

    /*
        Parameters  : background        - The background colour.
                      foreground        - The foreground colour.
        Returns     : os_error *        - Pointer to a corresponding error
                                          block, or NULL if no error.
        Description : Set the colours associated with this font handle.
    */
    os_error *set_colour(os_colour background, os_colour foreground) const;

    /*
        Parameters  : trfm              - The transformation to apply.
                      text              - The text to process.
                      left              - The start position in internal units.
                      spacing           - Additional spacing between characters.
                      trfm              - Font transformation matrix.
        Returns     : os_error *        - Pointer to a corresponding error
                                          block, or NULL if no error.
        Description : Paint the specified text.
    */
    os_error *paint(const transform_internal &trfm, const string &text,
                    const os_coord &left, int spacing = 0,
                    const os_trfm *matrix = NULL) const;

private:

    fontobj_base *ref;                  // The font handle
    int size;                           // Font size in internal units
};

// A class to handle fonts
class fontobj_obj
{
public:

    // Possible justifications
    enum alignment_style
    {
        align_left,
        align_centre,
        align_right
    };

    /*
        Parameters  : font      - The name of the font.
                      size      - The size of the font in internal units.
        Returns     : -
        Description : Constructor.
    */
    fontobj_obj(const string &font, int size);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~fontobj_obj();

    /*
        Parameters  : text              - The text to justify.
        Returns     : void
        Description : Set the text.
    */
    void set_text(const string &text);

    /*
        Parameters  : pos               - The horizontal position to use for
                                          justification.
                      base              - The vertical position of the baseline.
        Returns     : void
        Description : Set the justification point.
    */
    void set_position(int pos, int base);

    /*
        Parameters  : left              - The left-hand edge of the baseline
                                          to use for justification.
                      right             - The right-hand edge of the baseline
                                          to use for justification.
                      base              - The vertical position of the baseline.
        Returns     : void
        Description : Set the justification line.
    */
    void set_position(int left, int right, int base);

    /*
        Parameters  : align             - The required alignment.
        Returns     : void
        Description : Set the text alignment.
    */
    void set_alignment(alignment_style align = align_left);

    /*
        Paramters   : void
        Returns     : os_coord          - The text start position.
        Description : Obtain the baseline start position after justification.
    */
    os_coord get_left() const;

    /*
        Parameters  : void
        Returns     : int               - Additional spacing per character.
        Description : Obtain the additional inter-character spacing required
                      to justify the text.
    */
    int get_spacing() const;

    /*
        Parameters  : void
        Returns     : int               - Font width.
        Description : Obtain the modified font width required to jutify the
                      text if additional inter-character spacing is not used.
    */
    int get_width() const;

    /*
        Parameters  : void
        Returns     : os_box            - The underline rectangle.
        Description : Obtain the rectangle to underline the text.
    */
    os_box get_underline() const;

    /*
        Parameters  : void
        Returns     : os_box            - The strikethrough rectangle.
        Description : Obtain the rectangle to strikethrough the text.
    */
    os_box get_strikethrough() const;

    /*
        Parameters  : epoc              - The EPOC font name.
                      screen            - The screen font number.
                      bold              - Is a bold style required.
                      italic            - Is an italic style required.
        Returns     : string            - The name of the corresponding RISC OS
                                          font.
        Description : Map an EPOC font to the closest RISC OS equivalent.
    */
    static string map(const string &epoc, bits screen, bool bold, bool italic);

private:

    int size;                           // Font size in internal units
    fontobj_handle handle;              // Font handle
    string text;                        // Text to justify
    alignment_style align;              // Text alignment
    os_box base;                        // Requested text baseline
    bool valid;                         // Has justification been calculated
    os_box justified;                   // Baseline after justification
    int spacing;                        // Additional spacing
    int width;                          // Adjusted font width

    /*
        Parameters  : void
        Returns     : void
        Description : Calculate the required justification.
    */
    void justify() const;
};

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the font support.
*/
void fontobj_init();

#endif
