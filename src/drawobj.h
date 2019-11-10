/*
    File        : drawobj.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Draw path and file handling for the PsiFS filer.

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
#ifndef DRAWOBJ_H
#define DRAWOBJ_H

// Include oslib header files
#include "oslib/drawfile.h"

// Include cathlibcpp header files
#include "functional.h"
#include "list.h"
#include "map.h"
#include "ostream.h"
#include "set.h"
#include "string.h"
#include "vector.h"

// Include project header files
#include "transform.h"

// A base class for draw file objects
class drawobj_base
{
public:

    // A class to control rendering
    struct render_control
    {
        transform_internal trfm;        // Transformation matrix
        int flatness;                   // Required flatness of paths
        bool gdraw;                     // Should anti-aliasing be performed
        os_box clip;                    // Clipping rectangle in internal units
        os_box window;                  // Graphics window in OS units
    };

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~drawobj_base();

    /*
        Parameters  : info          - Information required for rendering.
        Returns     : os_error *    - Pointer to a corresponding error
                                      block, or NULL if no error.
        Description : Render this object.
    */
    virtual os_error *paint(render_control &info) const;

    /*
        Parameters  : s             - The stream to write the object to.
                      trfm          - The transformation to apply during
                                      saving.
        Returns     : void
        Description : Write this object to the stream.
    */
    virtual void save(ostream &s, const transform &trfm) const;

    /*
        Parameters  : void
        Returns     : drawfile_type - Draw file object tag.
        Description : Obtain the type of this object.
    */
    drawfile_type get_type() const;

    /*
        Parameters  : void
        Returns     : int           - The length in bytes.
        Description : Obtain the size of this object when saved to a file.
    */
    int get_size() const;

    /*
        Parameters  : void
        Returns     : os_box        - The bounding box for this object.
        Description : Obtain the bounding box for this object.
    */
    os_box get_box() const;

    /*
        Parameters  : a             - The bounding box.
        Returns     : bool          - Is the bounding box valid.
        Description : Check for a null bounding box.
    */
    static bool valid(const os_box &a);

    /*
        Parameters  : a             - The first bounding box.
                      b             - The second bounding box.
        Returns     : bool          - Is there any overlap.
        Description : Check for overlap between the two bounding boxes.
    */
    static bool overlap(const os_box &a, const os_box &b);

    /*
        Parameters  : a             - The first bounding box.
                      b             - The second bounding box.
        Returns     : bool          - Does one box enclose the other.
        Description : Check whether the first bounding box completely encloses
                      the second bounding box.
    */
    static bool enclose(const os_box &a, const os_box &b);

    /*
        Parameters  : a             - The first bounding box.
                      b             - The second bounding box.
        Returns     : os_box        - The overlapping region.
        Description : Calculate the overlapping region between two bouding
                      boxes.
    */
    static os_box intersect(const os_box &a, const os_box &b);

    /*
        Parameters  : a             - The first bounding box.
                      b             - The second bounding box.
        Returns     : os_box        - The combined region.
        Description : Calculate the bounding box around the two bounding boxes.
    */
    static os_box combine(const os_box &a, const os_box &b);

protected:

    // Header types for setting object size
    enum header_type
    {
        header_none,
        header_short,
        header_full
    };

    /*
        Parameters  : type          - The type of draw file object.
        Returns     : -
        Description : Constructor.
    */
    drawobj_base(drawfile_type type);

    /*
        Parameters  : size          - The length in bytes of the body.
                      header        - The type of header.
        Returns     : void
        Description : Set the size of this object when saved to a file.
    */
    void set_size(int size, header_type header = header_full);

    /*
        Parameters  : box           - The bounding box for this object.
        Returns     : void
        Description : Set the bounding box for this object.
    */
    void set_box(const os_box &box);

    /*
        Parameters  : s             - The stream to write the object to.
                      trfm          - The transformation to apply during
                                      saving.
                      with_box      - Does the header include a bounding box.
        Returns     : void
        Description : Write the header of this object to the stream.
    */
    void save_header(ostream &s, const transform &trfm,
                     bool with_box = TRUE) const;

private:

    drawfile_type type;                 // The type of object
    int size;                           // Size of this object
    os_box box;                         // Bounding box for this object
};

// A draw file options object
class drawobj_options : public drawobj_base
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    drawobj_options();

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~drawobj_options();

    /*
        Parameters  : s             - The stream to write the object to.
                      trfm          - The transformation to apply during
                                      saving.
        Returns     : void
        Description : Write this object to the stream.
    */
    virtual void save(ostream &s, const transform &trfm) const;

    /*
        Parameters  : box           - The bounding box for the whole file.
        Returns     : void
        Description : Set the most appropriate paper size for the draw file.
    */
    void set_paper_size(const os_box &box);

    /*
        Parameters  : void
        Returns     : os_coord      - The size of the paper in internal units.
        Description : Obtain the current paper size.
    */
    os_coord get_paper_size() const;

private:

    int paper_size;                     // The current paper size
    bool paper_landscape;               // The current paper orientation

    /*
        Parameters  : paper_size    - The A paper size.
        Returns     : os_coord      - The size of the paper.
        Description : Obtain a standard paper size.
    */
    os_coord get_paper_size(int paper_size) const;
};

// A draw file font table object
class drawobj_font_table : public drawobj_base
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    drawobj_font_table();

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~drawobj_font_table();

    /*
        Parameters  : s             - The stream to write the object to.
                      trfm          - The transformation to apply during
                                      saving.
        Returns     : void
        Description : Write this object to the stream.
    */
    virtual void save(ostream &s, const transform &trfm) const;

    /*
        Parameters  : font                  - The required font.
        Returns     : drawfile_text_style   - The corresponding font number.
        Description : Obtain a font number from the font table.
    */
    drawfile_text_style get_font(const string &font);

    /*
        Parameters  : void
        Returns     : set<string>   - The fonts used by this file.
        Description : Obtain all the fonts from the font table.
    */
    set<string, less<string> > get_fonts() const;

public:

    int body_size;                      // Size of this object's body
    map<string, byte, less<string> > fonts; // The font table
};

// A draw file group object
class drawobj_group : public drawobj_base
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    drawobj_group();

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~drawobj_group();

    /*
        Parameters  : obj           - The object to add.
        Returns     : void
        Description : Append a draw file object and transfer ownership.
    */
    void add(drawobj_base *obj);

    /*
        Parameters  : info          - Information required for rendering.
        Returns     : os_error *    - Pointer to a corresponding error
                                      block, or NULL if no error.
        Description : Render this object.
    */
    virtual os_error *paint(render_control &info) const;

    /*
        Parameters  : s             - The stream to write the object to.
                      trfm          - The transformation to apply during
                                      saving.
        Returns     : void
        Description : Write this object to the stream.
    */
    virtual void save(ostream &s, const transform &trfm) const;

private:

    int body_size;                      // Size of this object's body
    list<drawobj_base *> objects;       // List of nested draw file objects

    // Copying is not supported
    drawobj_group(const drawobj_group &obj);
    operator=(const drawobj_group &obj);
};

// A draw file clipping object
class drawobj_clip : public drawobj_group
{
public:

    /*
        Parameters  : clip          - The clipping rectangle in internal
                                      units.
        Returns     : -
        Description : Constructor.
    */
    drawobj_clip(const os_box &clip);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~drawobj_clip();

    /*
        Parameters  : info          - Information required for rendering.
        Returns     : os_error *    - Pointer to a corresponding error
                                      block, or NULL if no error.
        Description : Render this object.
    */
    virtual os_error *paint(render_control &info) const;

private:

    os_box clip;                        // The clipping rectangle

    /*
        Parameters  : window        - The graphics window in OS units.
        Returns     : void
        Description : Set a graphics window.
    */
    void set_graphics_window(const os_box &window) const;
};

// A draw file path object
class drawobj_path : public drawobj_base
{
public:

    // Join styles
    enum join_style
    {
        join_mitred = 0,
        join_round = 1,
        join_bevelled = 2
    };

    // Cap styles
    enum cap_style
    {
        cap_butt = 0,
        cap_round = 1,
        cap_square = 2,
        cap_triangular = 3
    };

    // Winding rules
    enum winding_rule
    {
        winding_non_zero = 0,
        winding_even_odd = 1
    };

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    drawobj_path();

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~drawobj_path();

    /*
        Parameters  : colour        - The fill colour.
        Returns     : void
        Description : Set the fill colour.
    */
    void set_fill(os_colour colour);

    /*
        Parameters  : colour        - The outline colour.
        Returns     : void
        Description : Set the outline colour.
    */
    void set_outline(os_colour colour);

    /*
        Parameters  : width         - The outline width in internal units.
        Returns     : void
        Description : Set the outline width.
    */
    void set_width(int width);

    /*
        Parameters  : joine         - The join style.
        Returns     : void
        Description : Set the join style.
    */
    void set_join_style(join_style join);

    /*
        Parameters  : start         - The start cap style.
                      end           - The end cap style.
                      width         - The width of triangular caps, in
                                      sixteenths of the line width.
                      length        - The length of triangular caps, in
                                      sixteenths of the line width.
        Returns     : void
        Description : Set the start and end cap styles.
    */
    void set_caps(cap_style start, cap_style end,
                  int width = 0, int length = 0);

    /*
        Parameters  : rule          - The winding rule.
        Returns     : void
        Description : Set the winding rule for filling the path.
    */
    void set_winding_rule(winding_rule rule);

    /*
        Parameters  : start         - Offset into the pattern.
                      pattern       - Lengths of dash pattern elements.
        Returns     : void
        Description : Set the dash pattern.
    */
    void set_dash_pattern(int start, const vector<int> &pattern);

    /*
        Parameters  : to            - The position to move to.
        Returns     : void
        Description : Add a move path component.
    */
    void add_move(const os_coord &to);

    /*
        Parameters  : to            - The position to draw to.
        Returns     : void
        Description : Add a line path component.
    */
    void add_line(const os_coord &to);

    /*
        Parameters  : first         - The first control point.
                      second        - The second control point.
                      to            - The position to draw to.
        Returns     : void
        Description : Add a beziere curve path component.
    */
    void add_bezier(const os_coord &first, const os_coord &second,
                    const os_coord &to);

    /*
        Parameters  : void
        Returns     : void
        Description : Close the current sub-path.
    */
    void add_close();

    /*
        Parameters  : void
        Returns     : void
        Description : End the path.
    */
    void add_end();

    /*
        Parameters  : rectangle     - The bounding box of the rectangle.
        Returns     : void
        Description : Add a rectangle sub path.
    */
    void add_rectangle(const os_box &rectangle);

    /*
        Parameters  : ellipse       - The bounding box of the ellipse.
        Returns     : void
        Description : Add a rectangle sub path.
    */
    void add_ellipse(const os_box &ellipse);

    /*
        Parameters  : start         - The start position of the line.
                      end           - The end position of the line.
        Returns     : void
        Description : Add a line sub path.
    */
    void add_line(const os_coord &start, const os_coord &end);

    /*
        Parameters  : info          - Information required for rendering.
        Returns     : os_error *    - Pointer to a corresponding error
                                      block, or NULL if no error.
        Description : Render this object.
    */
    virtual os_error *paint(render_control &info) const;

    /*
        Parameters  : s             - The stream to write the object to.
                      trfm          - The transformation to apply during
                                      saving.
        Returns     : void
        Description : Write this object to the stream.
    */
    virtual void save(ostream &s, const transform &trfm) const;

private:

    os_colour fill;                     // The fill colour
    os_colour outline;                  // The outline colour
    int width;                          // The outline width
    drawfile_path_style style;          // The path style
    vector<int> pattern;                // The dash pattern
    vector<int> path;                   // The path data

    /*
        Parameters  : fill              - Is the style required for a fill
                                          operation.
        Returns     : draw_fill_style   - The style.
        Description : Construct a fill style for rendering this path.
    */
    draw_fill_style get_fill_style(bool fill = TRUE) const;

    /*
        Parameters  : void
        Returns     : draw_line_style   - The style.
        Description : Construct a line style for rendering this path.
    */
    draw_line_style get_line_style() const;
};

// A draw file text object
class drawobj_text : public drawobj_base
{
public:

    // Justification styles
    enum justification_style
    {
        justify_none,
        justify_left,
        justify_centre,
        justify_right
    };

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    drawobj_text();

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~drawobj_text();

    /*
        Parameters  : colour        - The fill colour.
        Returns     : void
        Description : Set the text colour.
    */
    void set_fill(os_colour colour);

    /*
        Parameters  : colour        - The background hint colour.
        Returns     : void
        Description : Set the background colour for antialiasing.
    */
    void set_bg_hint(os_colour colour);

    /*
        Parameters  : font          - The name of the font.
                      font_table    - The font table object for this draw file.
        Returns     : void
        Description : Set the font face.
    */
    void set_font(const string &font, drawobj_font_table &font_table);

    /*
        Parameters  : size          - The size in internal units.
        Returns     : void
        Description : Set the font size.
    */
    void set_font_size(int size);

    /*
        Parameters  : spacing       - Extra spacing between characters in
                                      internal units.
                      width         - Font width if spacing cannot be used.
        Returns     : void
        Description : Set enhanced justification.
    */
    void set_justification(int spacing, int width);

    /*
        Parameters  : base          - The text baseline position.
        Returns     : void
        Description : Set the baseline positiont.
    */
    void set_base(const os_coord &base);

    /*
        Parameters  : text          - The text.
        Returns     : void
        Description : Set the text.
    */
    void set_text(const string &text);

    /*
        Parameters  : info          - Information required for rendering.
        Returns     : os_error *    - Pointer to a corresponding error
                                      block, or NULL if no error.
        Description : Render this object.
    */
    virtual os_error *paint(render_control &info) const;

    /*
        Parameters  : s             - The stream to write the object to.
                      trfm          - The transformation to apply during
                                      saving.
        Returns     : void
        Description : Write this object to the stream.
    */
    virtual void save(ostream &s, const transform &trfm) const;

private:

    os_colour fill;                     // Fill colour
    os_colour bg_hint;                  // Background hint colour
    string font;                        // Font name
    drawfile_text_style style;          // Font number
    int xsize;                          // Hozizontal font size
    int ysize;                          // Vertical font size
    int spacing;                        // Extra character spacing
    os_coord base;                      // Baseline start position
    string text;                        // The text

    /*
        Parameters  : void
        Returns     : void
        Description : Update the size and bounding box.
    */
    void update();
};

// A draw file bitmap object
class drawobj_sprite : public drawobj_base
{
public:

    /*
        Parameters  : bpp           - Number of bits per pixel.
                      width         - Width in pixels.
                      height        - Height in pixels.
        Returns     : -
        Description : Constructor.
    */
    drawobj_sprite(int bpp, int width, int height);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~drawobj_sprite();

    /*
        Parameters  : box           - The bounding box for this object.
        Returns     : void
        Description : Set the bounding box for this object.
    */
    void set_box(const os_box &box);

    /*
        Parameters  : palette       - The palette data.
        Returns     : void
        Description : Set the palette for this sprite.
    */
    void set_palette(const vector<os_colour> &palette);

    /*
        Parameters  : bitmap        - The pixel data.
        Returns     : void
        Description : Set the pixel data for this sprite.
    */
    void set_bitmap(const vector<byte> &bitmap);

    /*
        Parameters  : info          - Information required for rendering.
        Returns     : os_error *    - Pointer to a corresponding error
                                      block, or NULL if no error.
        Description : Render this object.
    */
    virtual os_error *paint(render_control &info) const;

    /*
        Parameters  : s             - The stream to write the object to.
                      trfm          - The transformation to apply during
                                      saving.
        Returns     : void
        Description : Write this object to the stream.
    */
    virtual void save(ostream &s, const transform &trfm) const;

private:

    vector<byte> sprite;                // The sprite data
};

// A complete draw file
class drawobj_file
{
    // Reference counted details
    struct ref_class
    {
        drawobj_font_table fonts;       // Font table for the whole draw file
        drawobj_group group;            // All other objects are in a group
        int count;                      // Number of references

        /*
            Parameters  : void
            Returns     : -
            Description : Constructor.
        */
        ref_class() : count(1) {}

        /*
            Parameters  : void
            Returns     : -
            Description : Destructor.
        */
        ~ref_class();

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
    };
    ref_class *ref;

public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    drawobj_file();

    /*
        Parameters  : file          - The draw file to copy.
        Returns     : -
        Description : Constructor.
    */
    drawobj_file(const drawobj_file &file);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~drawobj_file();

    /*
        Parameters  : file          - The draw file to copy.
        Returns     : drawobj_file  - A reference to this draw file.
        Description : Draw file assignment.
    */
    drawobj_file &operator=(const drawobj_file &file);

    /*
        Parameters  : trfm          - The transformation to apply during
                                      rendering.
                      clip          - The clipping rectangle in OS units.
                      resolution    - Dots per OS unit, or 0 for the screen.
        Returns     : os_error *    - Pointer to a corresponding error
                                      block, or NULL if no error.
        Description : Render this object.
    */
    os_error *paint(const transform_internal &trfm,
                    const os_box &clip, int resolution = 0) const;

    /*
        Parameters  : s             - The stream to write the file to.
        Returns     : void
        Description : Write this file to the stream.
    */
    void save(ostream &s) const;

    /*
        Parameters  : obj           - The object to add.
        Returns     : void
        Description : Append a draw file object and transfer ownership.
    */
    void add(drawobj_base *obj);

    /*
        Parameters  : void
        Returns     : os_box        - The bounding box for this file.
        Description : Obtain the bounding box for this file.
    */
    os_box get_box() const;

    /*
        Parameters  : void
        Returns     : drawobj_font_table    - The font table.
        Description : Obtain the font table for this file.
    */
    drawobj_font_table &get_font_table();
};

#endif
