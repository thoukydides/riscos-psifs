/*
    File        : drawobj.c++
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

// Include header file for this module
#include "drawobj.h"

// Include clib header files
#include <math.h>

// Include oslib header files
#include "oslib/colourtrans.h"
#include "oslib/osmodule.h"

// Include project header files
#include "config.h"
#include "fontobj.h"
#include "fs.h"
#include "gdraw.h"

// Module names to check before using the GDraw module
#define DRAWOBJ_MODULE_GDRAW "GDraw"
#define DRAWOBJ_MODULE_DITHEREXTEND "DitherExtend"

// Draw file header details
static const char drawobj_signature[4] = {'D', 'r', 'a', 'w'};
static const int drawobj_major_version = 201;
static const int drawobj_minor_version = 0;

// Sprite fields and constants
static const char drawobj_sprite_name[] = "sprite";
#define DRAWOBJ_SPRITE16BPP ((os_mode) ((5 << 27) | (90 << 14) | (90 << 1) | 1))
#define DRAWOBJ_SPRITE32BPP ((os_mode) ((6 << 27) | (90 << 14) | (90 << 1) | 1))
#define DRAWOBJ_SPRITE_AREA ((osspriteop_area *) 0x100)

// Control point offset for ellipse or circle as fraction of radius
static const double drawobj_ellipse_factor = 4.0 * (sqrt(2.0) - 1.0) / 3.0;

// Length of program and group name fields
#define DRAWOBJ_NAME_LENGTH (12)

// Corrected options object structure
struct drawobj_options_struct
{
    int paper_size;
    drawfile_paper_options paper_options;
    double grid_spacing;
    int grid_division;
    bool isometric;
    bool auto_adjust;
    bool show;
    bool lock;
    bool cm;
    int zoom_mul;
    int zoom_div;
    bool zoom_lock;
    bool toolbox;
    drawfile_entry_mode entry_mode;
    int undo_size;
};

/*
    Parameters  : type          - The type of draw file object.
    Returns     : -
    Description : Constructor.
*/
drawobj_base::drawobj_base(drawfile_type type)
{
    // Store the object type
    this->type = type;

    // Null object until initialised
    size = 0;

    // Dummy bounding box until initialised
    box.x0 = box.y0 = box.x1 = box.y1 = 0;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_base::~drawobj_base()
{
    // No action required
}

/*
    Parameters  : info          - Information required for rendering.
    Returns     : os_error *    - Pointer to a corresponding error
                                  block, or NULL if no error.
    Description : Render this object.
*/
os_error *drawobj_base::paint(render_control &info) const
{
    os_error *err = NULL;

    // No action required for null objects
    UNSET(info)

    // Return any error produced
    return err;
}

/*
    Parameters  : s             - The stream to write the object to.
                  trfm          - The transformation to apply during
                                  saving.
    Returns     : void
    Description : Save this object.
*/
void drawobj_base::save(ostream &s, const transform &trfm) const
{
    // No action require for null objects
    UNSET(s)
    UNSET(trfm)
}

/*
    Parameters  : void
    Returns     : drawfile_type - Draw file object tag.
    Description : Obtain the type of this object.
*/
drawfile_type drawobj_base::get_type() const
{
    return type;
}

/*
    Parameters  : void
    Returns     : int           - The length in bytes.
    Description : Obtain the size of this object when saved to a file.
*/
int drawobj_base::get_size() const
{
    return size;
}

/*
    Parameters  : void
    Returns     : os_box        - The bounding box for this object.
    Description : Obtain the bounding box for this object.
*/
os_box drawobj_base::get_box() const
{
    return box;
}

/*
    Parameters  : size          - The length in bytes of the body.
                  header        - The type of header.
    Returns     : void
    Description : Set the size of this object when saved to a file.
*/
void drawobj_base::set_size(int size, header_type header)
{
    // Add on the header size if not empty
    if (size)
    {
        switch (header)
        {
            case header_short:
                size += sizeof(drawfile_type) + sizeof(int);
                break;

            case header_full:
                size += sizeof(drawfile_type) + sizeof(int) + sizeof(os_box);
                break;

            case header_none:
            default:
                break;
        }
    }

    // Store the size
    this->size = size;
}

/*
    Parameters  : box           - The bounding box for this object.
    Returns     : void
    Description : Set the bounding box for this object.
*/
void drawobj_base::set_box(const os_box &box)
{
    this->box = box;
}

/*
    Parameters  : s             - The stream to write the object to.
                  trfm          - The transformation to apply during
                                  saving.
                  with_box      - Does the header include a bounding box.
    Returns     : void
    Description : Write the header of this object to the stream.
*/
void drawobj_base::save_header(ostream &s, const transform &trfm,
                               bool with_box) const
{
    // No action unless the size has been set
    if (size)
    {
        // Write the object type and size
        s.write((const char *) &type, sizeof(drawfile_type));
        s.write((const char *) &size, sizeof(int));

        // Add the transformed bounding box if required
        if (with_box)
        {
            os_box transformed = trfm(box);
            s.write((const char *) &transformed, sizeof(os_box));
        }
    }
}

/*
    Parameters  : a             - The bounding box.
    Returns     : bool          - Is the bounding box valid.
    Description : Check for a null bounding box.
*/
bool drawobj_base::valid(const os_box &a)
{
    // Check for a null bounding box
    return a.x0 || a.y0 || a.x1 || a.y1;
}

/*
    Parameters  : a             - The first bounding box.
                  b             - The second bounding box.
    Returns     : bool          - Is there any overlap.
    Description : Check for overlap between the two bounding boxes.
*/
bool drawobj_base::overlap(const os_box &a, const os_box &b)
{
    // Check for overlap
    return valid(a) && valid(b)
           && (a.x0 <= b.x1) && (b.x0 <= a.x1)
           && (a.y0 <= b.y1) && (b.y0 <= a.y1);
}

/*
    Parameters  : a             - The first bounding box.
                  b             - The second bounding box.
    Returns     : bool          - Does one box enclose the other.
    Description : Check whether the first bounding box completely encloses
                  the second bounding box.
*/
bool drawobj_base::enclose(const os_box &a, const os_box &b)
{
    // Compare the bounding boxes
    return (a.x0 <= b.x0) && (b.x1 <= a.x1)
           && (a.y0 <= b.y0) && (b.y0 <= a.y1);
}

/*
    Parameters  : a             - The first bounding box.
                  b             - The second bounding box.
    Returns     : os_box        - The overlapping region.
    Description : Calculate the overlapping region between two bouding
                  boxes.
*/
os_box drawobj_base::intersect(const os_box &a, const os_box &b)
{
    os_box intersection;

    // Check whether there is any overlap
    if (overlap(a, b))
    {
        // Calculate the intersection region
        intersection.x0 = a.x0 < b.x0 ? b.x0 : a.x0;
        intersection.y0 = a.y0 < b.y0 ? b.y0 : a.y0;
        intersection.x1 = a.x1 < b.x1 ? a.x1 : b.x1;
        intersection.y1 = a.y1 < b.y1 ? a.y1 : b.y1;
    }
    else
    {
        // No overlap, so construct a null region
        intersection.x0 = intersection.y0 = 0;
        intersection.x1 = intersection.y1 = 0;
    }

    // Return the result
    return intersection;
}

/*
    Parameters  : a             - The first bounding box.
                  b             - The second bounding box.
    Returns     : os_box        - The combined region.
    Description : Calculate the bounding box around the two bounding boxes.
*/
os_box drawobj_base::combine(const os_box &a, const os_box &b)
{
    // Check whether the first bounding box is valid
    if (!valid(a)) return b;

    // Check whether the second bounding box is valid
    if (!valid(b)) return a;

    // Combine the bounding boxes
    os_box combined;
    combined.x0 = a.x0 < b.x0 ? a.x0 : b.x0;
    combined.y0 = a.y0 < b.y0 ? a.y0 : b.y0;
    combined.x1 = a.x1 < b.x1 ? b.x1 : a.x1;
    combined.y1 = a.y1 < b.y1 ? b.y1 : a.y1;

    // Return the result
    return combined;
}

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
drawobj_options::drawobj_options() : drawobj_base(drawfile_TYPE_OPTIONS)
{
    // Default paper is A4 portrait
    paper_size = 4;
    paper_landscape = FALSE;

    // Set the object size
    set_size(sizeof(drawobj_options_struct));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_options::~drawobj_options()
{
    // No action required
}

/*
    Parameters  : s             - The stream to write the object to.
                  trfm          - The transformation to apply during
                                  saving.
    Returns     : void
    Description : Write this object to the stream.
*/
void drawobj_options::save(ostream &s, const transform &trfm) const
{
    // Write the options object header
    save_header(s, trfm);

    // Construct the default options
    drawobj_options_struct options;
    options.paper_size = (paper_size + 1) * 0x100;
    options.paper_options = drawfile_PAPER_SHOW | drawfile_PAPER_DEFAULT;
    if (paper_landscape) options.paper_options |= drawfile_PAPER_LANDSCAPE;
    options.grid_spacing = 1.0;
    options.grid_division = 2;
    options.isometric = FALSE;
    options.auto_adjust = TRUE;
    options.show = FALSE;
    options.lock = FALSE;
    options.cm = TRUE;
    options.zoom_mul = 1;
    options.zoom_div = 1;
    options.zoom_lock = FALSE;
    options.toolbox = TRUE;
    options.entry_mode = drawfile_ENTRY_MODE_SELECT;
    options.undo_size = 5000;

    // Save the options
    s.write((const char *) &options,
            sizeof(drawobj_options_struct));
}

/*
    Parameters  : box           - The bounding box for the whole file.
    Returns     : void
    Description : Set the most appropriate paper size for the draw file.
*/
void drawobj_options::set_paper_size(const os_box &box)
{
    // Extract the required size from the bounding box
    os_coord required;
    required.x = box.x1;
    required.y = -box.y0;

    // Start from the smallest paper size and work up
    for (int size = 5; 0 <= size; size--)
    {
        // Calculate the paper size
        os_coord paper = get_paper_size(size);

        // Try to fit the page in portrait orientation
        if ((required.x <= paper.x) && (required.y <= paper.y))
        {
            paper_size = size;
            paper_landscape = FALSE;
            return;
        }

        // Try to fit the page in landscape orientation
        if ((required.x <= paper.y) && (required.y <= paper.x))
        {
            paper_size = size;
            paper_landscape = TRUE;
            return;
        }
    }

    // No size fits, so use a default
    paper_size = 4;
    paper_landscape = FALSE;
}

/*
    Parameters  : void
    Returns     : os_coord      - The size of the paper in internal units.
    Description : Obtain the current paper size.
*/
os_coord drawobj_options::get_paper_size() const
{
    // Get the portrait paper size
    os_coord paper = get_paper_size(paper_size);

    // Swap the dimensions if landscape orientation required
    if (paper_landscape)
    {
        int t = paper.x;
        paper.x = paper.y;
        paper.y = t;
    }

    // Return the paper size
    return paper;
}

/*
    Parameters  : paper_size    - The A paper size.
    Returns     : os_coord      - The size of the paper.
    Description : Obtain a standard paper size.
*/
os_coord drawobj_options::get_paper_size(int paper_size) const
{
    // Calculate the page size in mm
    os_coord paper;
    paper.x = transform_base::round(1000.0 * pow(2.0, -0.25 - paper_size / 2.0));
    paper.y = transform_base::round(1000.0 * pow(2.0, 0.25 - paper_size / 2.0));

    // Convert the page size to internal units and return
    return transform_to_mm.inverse(paper);
}

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
drawobj_font_table::drawobj_font_table()
: drawobj_base(drawfile_TYPE_FONT_TABLE)
{
    // Body is empty initially
    body_size = 0;
    fonts.clear(); // This is a workaround for a CathLibCPP bug
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_font_table::~drawobj_font_table()
{
    // No action required
}

/*
    Parameters  : s             - The stream to write the object to.
                  trfm          - The transformation to apply during
                                  saving.
    Returns     : void
    Description : Write this object to the stream.
*/
void drawobj_font_table::save(ostream &s, const transform &trfm) const
{
    // No action unless fonts have been defined
    if (get_size())
    {
        // Write the font table object header
        save_header(s, trfm, FALSE);

        // Write the font table entries
        map_const_iterator<string, byte, less<string> > i;
        for (i = fonts.begin(); i != fonts.end(); i++)
        {
            // Write the font number and name
            s.write((const char *) &((*i).second), sizeof(byte));
            string name = (*i).first;
            s.write(name.c_str(), name.length() + 1);
        }

        // Pad to a word boundary
        int padding = 3 - ((body_size + 3) & 3);
        char zero = '\0';
        while (padding--) s.write(&zero, sizeof(zero));
    }
}

/*
    Parameters  : font                  - The required font.
    Returns     : drawfile_text_style   - The corresponding font number.
    Description : Obtain a font number from the font table.
*/
drawfile_text_style drawobj_font_table::get_font(const string &font)
{
    // Add a new mapping if necessary
    if (fonts.find(font) == fonts.end())
    {
        // Add the mapping
        fonts[font] = fonts.size() + 1;

        // Update the size of the font object
        body_size += font.length() + 2;
        set_size((body_size + 3) & ~3, header_short);
    }

    // Return the font number
    drawfile_text_style style;
    style.font_index = fonts[font];
    style.reserved[0] = style.reserved[1] = style.reserved[2] = 0;
    return style;
}

/*
    Parameters  : void
    Returns     : set<string>   - The fonts used by this file.
    Description : Obtain all the fonts from the font table.
*/
set<string, less<string> > drawobj_font_table::get_fonts() const
{
    // Copy the font names to a set
    set<string, less<string> > font_set;
    map_const_iterator<string, byte, less<string> > i;
    for (i = fonts.begin(); i != fonts.end(); i++)
    {
        font_set.insert((*i).first);
    }

    // Return the set of fonts
    return font_set;
}

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
drawobj_group::drawobj_group() : drawobj_base(drawfile_TYPE_GROUP)
{
    // Body is empty initially
    body_size = 0;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_group::~drawobj_group()
{
    // Delete all of the nested objects
    while (!objects.empty())
    {
        delete objects.front();
        objects.pop_front();
    }
}

/*
    Parameters  : obj           - The object to add.
    Returns     : void
    Description : Append a draw file object and transfer ownership.
*/
void drawobj_group::add(drawobj_base *obj)
{
    // No action unless an object was specified
    if (obj)
    {
        // Add to the end of the list
        objects.push_back(obj);

        // Update the size of the group object
        body_size += obj->get_size();
        if (body_size)
        {
            if (objects.size() == 1) set_size(body_size, header_none);
            else set_size(body_size + DRAWOBJ_NAME_LENGTH);
        }

        // Update the bounding box of the group object
        set_box(combine(get_box(), obj->get_box()));
    }
}

/*
    Parameters  : info          - Information required for rendering.
    Returns     : os_error *    - Pointer to a corresponding error
                                  block, or NULL if no error.
    Description : Render this object.
*/
os_error *drawobj_group::paint(render_control &info) const
{
    os_error *err = NULL;

    // No action unless the clip region overlaps the bounding box
    if (overlap(get_box(), info.clip))
    {
        // Paint all of the nested objects
        list_const_iterator<drawobj_base *> i;
        for (i = objects.begin(); !err && (i != objects.end()); i++)
        {
            err = (*i)->paint(info);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : s             - The stream to write the object to.
                  trfm          - The transformation to apply during
                                  saving.
    Returns     : void
    Description : Write this object to the stream.
*/
void drawobj_group::save(ostream &s, const transform &trfm) const
{
    // No action unless there are nested objects that need saving
    if (get_size())
    {
        // Special case for a single object
        if (objects.size() == 1)
        {
            // Flatten the group if only a single object contained
            objects.front()->save(s, trfm);
        }
        else
        {
            // Write the group object header
            save_header(s, trfm);
            string name(DRAWOBJ_NAME_LENGTH, ' ');
            s.write(name.c_str(), DRAWOBJ_NAME_LENGTH);

            // Write the nested objects
            list_const_iterator<drawobj_base *> i;
            for (i = objects.begin(); i != objects.end(); i++)
            {
                (*i)->save(s, trfm);
            }
        }
    }
}

/*
    Parameters  : clip          - The clipping rectangle in internal
                                  units.
    Returns     : -
    Description : Constructor.
*/
drawobj_clip::drawobj_clip(const os_box &clip) : clip(clip)
{
    // No action required
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_clip::~drawobj_clip()
{
    // No action required
}

/*
    Parameters  : info          - Information required for rendering.
    Returns     : os_error *    - Pointer to a corresponding error
                                  block, or NULL if no error.
    Description : Render this object.
*/
os_error *drawobj_clip::paint(render_control &info) const
{
    os_error *err = NULL;

    // No action unless the clip region overlaps the clip box
    if (overlap(this->clip, info.clip))
    {
        // Check whether any clipping is required
        bool clip_required = !enclose(this->clip, get_box());

        // Set a new graphics window if required
        os_box previous = info.window;
        if (clip_required)
        {
            os_box required = info.trfm.to_os()(this->clip,
                                                transform::round_out);
            info.window = valid(previous)
                          ? intersect(required, previous)
                          : required;
            if (valid(info.window)) set_graphics_window(info.window);
        }

        // Pass on to the base class to paint all of the nested objects
        err = drawobj_group::paint(info);

        // Restore the previous graphics window
        if (clip_required)
        {
            info.window = previous;
            set_graphics_window(previous);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : window        - The graphics window in OS units.
    Returns     : void
    Description : Set a graphics window.
*/
void drawobj_clip::set_graphics_window(const os_box &window) const
{
    // Special case for a null window
    if (!valid(window))
    {
        // Restore the default window (should only be used during printing)
        xos_reset_windows();
    }
    else
    {
        // Set the graphics window position
        xos_set_graphics_window();
        xos_writec(window.x0 & 0xff);
        xos_writec(window.x0 >> 8);
        xos_writec(window.y0 & 0xff);
        xos_writec(window.y0 >> 8);
        xos_writec(window.x1 & 0xff);
        xos_writec(window.x1 >> 8);
        xos_writec(window.y1 & 0xff);
        xos_writec(window.y1 >> 8);
    }
}

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
drawobj_path::drawobj_path() : drawobj_base(drawfile_TYPE_PATH)
{
    // Set the default path style
    fill = os_COLOUR_TRANSPARENT;
    outline = os_COLOUR_TRANSPARENT;
    width = 0;
    style.flags = (drawfile_PATH_ROUND << drawfile_PATH_JOIN_SHIFT)
                  | (drawfile_PATH_ROUND << drawfile_PATH_END_SHIFT)
                  | (drawfile_PATH_ROUND << drawfile_PATH_START_SHIFT);
    style.cap_width = 0;
    style.cap_length = 0;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_path::~drawobj_path()
{
    // No action required
}

/*
    Parameters  : colour        - The fill colour.
    Returns     : void
    Description : Set the fill colour.
*/
void drawobj_path::set_fill(os_colour colour)
{
    fill = colour;
}

/*
    Parameters  : colour        - The outline colour.
    Returns     : void
    Description : Set the outline colour.
*/
void drawobj_path::set_outline(os_colour colour)
{
    outline = colour;
}

/*
    Parameters  : width         - The outline width in internal units.
    Returns     : void
    Description : Set the outline width.
*/
void drawobj_path::set_width(int width)
{
    this->width = width;
}

/*
    Parameters  : joine         - The join style.
    Returns     : void
    Description : Set the join style.
*/
void drawobj_path::set_join_style(join_style join)
{
    style.flags = (style.flags & ~drawfile_PATH_JOIN)
                  | (join << drawfile_PATH_JOIN_SHIFT);
}

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
void drawobj_path::set_caps(cap_style start, cap_style end,
                            int width, int length)
{
    style.flags = (style.flags & ~(drawfile_PATH_START | drawfile_PATH_END))
                  | (start << drawfile_PATH_START_SHIFT)
                  | (end << drawfile_PATH_END_SHIFT);
    style.cap_width = width;
    style.cap_length = length;
}

/*
    Parameters  : rule          - The winding rule.
    Returns     : void
    Description : Set the winding rule for filling the path.
*/
void drawobj_path::set_winding_rule(winding_rule rule)
{
    if (rule == winding_even_odd) style.flags |= drawfile_PATH_WINDING_EVEN_ODD;
    else style.flags &= ~drawfile_PATH_WINDING_EVEN_ODD;
}

/*
    Parameters  : start         - Offset into the pattern.
                  pattern       - Lengths of dash pattern elements.
    Returns     : void
    Description : Set the dash pattern.
*/
void drawobj_path::set_dash_pattern(int start, const vector<int> &pattern)
{
    // Action depends on whether a pattern is being set
    if (pattern.empty())
    {
        // Clear any existing pattern
        style.flags &= ~drawfile_PATH_DASHED;
        this->pattern.clear();
    }
    else
    {
        // Set the new pattern
        style.flags |= drawfile_PATH_DASHED;
        this->pattern.clear();
        this->pattern.push_back(start);
        this->pattern.push_back(pattern.size());
        this->pattern.insert(this->pattern.end(),
                             pattern.begin(), pattern.end());
    }
}

/*
    Parameters  : to            - The position to move to.
    Returns     : void
    Description : Add a move path component.
*/
void drawobj_path::add_move(const os_coord &to)
{
    path.push_back(draw_MOVE_TO);
    path.push_back(to.x);
    path.push_back(to.y);
}

/*
    Parameters  : to            - The position to draw to.
    Returns     : void
    Description : Add a line path component.
*/
void drawobj_path::add_line(const os_coord &to)
{
    path.push_back(draw_LINE_TO);
    path.push_back(to.x);
    path.push_back(to.y);
}

/*
    Parameters  : first         - The first control point.
                  second        - The second control point.
                  to            - The position to draw to.
    Returns     : void
    Description : Add a beziere curve path component.
*/
void drawobj_path::add_bezier(const os_coord &first, const os_coord &second,
                              const os_coord &to)
{
    path.push_back(draw_BEZIER_TO);
    path.push_back(first.x);
    path.push_back(first.y);
    path.push_back(second.x);
    path.push_back(second.y);
    path.push_back(to.x);
    path.push_back(to.y);
}

/*
    Parameters  : void
    Returns     : void
    Description : Close the current sub-path.
*/
void drawobj_path::add_close()
{
    path.push_back(draw_CLOSE_LINE);
}

/*
    Parameters  : void
    Returns     : void
    Description : End the path.
*/
void drawobj_path::add_end()
{
    // End the path
    path.push_back(draw_END_PATH);

    // Set the object size
    set_size(sizeof(os_colour) * 2
             + sizeof(int)
             + sizeof(drawfile_path_style)
             + pattern.size() * sizeof(int)
             + path.size() * sizeof(int));

    // Set the bounding box
    draw_fill_style fill_style = get_fill_style(FALSE);
    fill_style |= draw_FILL_FLATTEN | draw_FILL_THICKEN | draw_FILL_REFLATTEN;
    draw_line_style line_style = get_line_style();
    os_box box;
    draw_process_path((draw_path *) &path[0], fill_style, NULL, 0, width, &line_style, (draw_dash_pattern *) (pattern.size() ? &pattern[0] : NULL), (draw_output_path) (int(&box) + int(draw_SPECIAL_BBOX)));
    set_box(box);
}

/*
    Parameters  : rectangle     - The bounding box of the rectangle.
    Returns     : void
    Description : Add a rectangle sub path.
*/
void drawobj_path::add_rectangle(const os_box &rectangle)
{
    // Add the sub path
    os_coord to;
    to.x = rectangle.x0;
    to.y = rectangle.y0;
    add_move(to);
    to.y = rectangle.y1;
    add_line(to);
    to.x = rectangle.x1;
    add_line(to);
    to.y = rectangle.y0;
    add_line(to);
    to.x = rectangle.x0;
    add_line(to);

    // Close the sub path
    add_close();
}

/*
    Parameters  : ellipse       - The bounding box of the ellipse.
    Returns     : void
    Description : Add a rectangle sub path.
*/
void drawobj_path::add_ellipse(const os_box &ellipse)
{
    // Calculate the centre of the ellipse
    os_coord centre;
    centre.x = (ellipse.x0 + ellipse.x1 + 1) / 2;
    centre.y = (ellipse.y0 + ellipse.y1 + 1) / 2;

    // Calculate the offsets for the control points
    os_coord control;
    control.x = transform_base::round((ellipse.x1 - ellipse.x0)
                                      * drawobj_ellipse_factor / 2.0);
    control.y = transform_base::round((ellipse.y1 - ellipse.y0)
                                      * drawobj_ellipse_factor / 2.0);

    // Add the sub path
    os_coord first;
    os_coord second;
    os_coord to;
    to.x = centre.x;
    to.y = ellipse.y0;
    add_move(to);
    first.x = centre.x + control.x;
    first.y = ellipse.y0;
    second.x = ellipse.x1;
    second.y = centre.y - control.y;
    to.x = ellipse.x1;
    to.y = centre.y;
    add_bezier(first, second, to);
    first.x = ellipse.x1;
    first.y = centre.y + control.y;
    second.x = centre.x + control.x;
    second.y = ellipse.y1;
    to.x = centre.x;
    to.y = ellipse.y1;
    add_bezier(first, second, to);
    first.x = centre.x - control.x;
    first.y = ellipse.y1;
    second.x = ellipse.x0;
    second.y = centre.y + control.y;
    to.x = ellipse.x0;
    to.y = centre.y;
    add_bezier(first, second, to);
    first.x = ellipse.x0;
    first.y = centre.y - control.y;
    second.x = centre.x - control.x;
    second.y = ellipse.y0;
    to.x = centre.x;
    to.y = ellipse.y0;
    add_bezier(first, second, to);

    // Close the sub path
    add_close();
}

/*
    Parameters  : start         - The start position of the line.
                  end           - The end position of the line.
    Returns     : void
    Description : Add a line sub path.
*/
void drawobj_path::add_line(const os_coord &start, const os_coord &end)
{
    // Add the sub path (does not need to be closed)
    add_move(start);
    add_line(end);
}

/*
    Parameters  : info          - Information required for rendering.
    Returns     : os_error *    - Pointer to a corresponding error
                                  block, or NULL if no error.
    Description : Render this object.
*/
os_error *drawobj_path::paint(render_control &info) const
{
    os_error *err = NULL;

    // No action unless the clip region overlaps the bounding box
    if (overlap(get_box(), info.clip))
    {
        // Fill the interior of the path if required
        if (fill != os_COLOUR_TRANSPARENT)
        {
            // Set the fill colour
            err = xcolourtrans_set_gcol(fill, colourtrans_USE_ECFS, os_ACTION_OVERWRITE | os_ACTION_USE_ECF4, NULL, NULL);

            // Fill the interior of the path
            if (!err)
            {
                draw_fill_style fill_style = get_fill_style();
                if (info.gdraw) err = xgdraw_fill((draw_path *) &path[0], fill_style | draw_FILL_ANTIALIAS, info.trfm.to_internal(), info.flatness);
                if (!info.gdraw || err) err = xdraw_fill((draw_path *) &path[0], fill_style, info.trfm.to_internal(), info.flatness);
            }
        }

        // Plot the outline of the path if required
        if (!err && (outline != os_COLOUR_TRANSPARENT))
        {
            // Set the line colour
            err = xcolourtrans_set_gcol(outline, colourtrans_USE_ECFS, os_ACTION_OVERWRITE | os_ACTION_USE_ECF4, NULL, NULL);

            // Stroke the outline of the path
            if (!err)
            {
                draw_fill_style fill_style = get_fill_style(FALSE);
                draw_line_style line_style = get_line_style();
                if (info.gdraw) err = xgdraw_stroke((draw_path *) &path[0], fill_style | draw_FILL_ANTIALIAS, info.trfm.to_internal(), info.flatness, width, &line_style, (draw_dash_pattern *) (pattern.size() ? &pattern[0] : NULL));
                if (!info.gdraw || err) err = xdraw_stroke((draw_path *) &path[0], fill_style, info.trfm.to_internal(), info.flatness, width, &line_style, (draw_dash_pattern *) (pattern.size() ? &pattern[0] : NULL));
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : s             - The stream to write the object to.
                  trfm          - The transformation to apply during
                                  saving.
    Returns     : void
    Description : Write this object to the stream.
*/
void drawobj_path::save(ostream &s, const transform &trfm) const
{
    // No action unless the path has been completed
    if (get_size())
    {
        // Write the path object header
        save_header(s, trfm);
        s.write((const char *) &fill, sizeof(os_colour));
        s.write((const char *) &outline, sizeof(os_colour));
        s.write((const char *) &width, sizeof(int));
        s.write((const char *) &style, sizeof(drawfile_path_style));

        // Write any pattern data
        s.write((const char *) &pattern[0], pattern.size() * sizeof(int));

        // Transform and write any path data
        vector<int> transformed = path;
        draw_transform_path((draw_path *) &transformed[0], NULL, trfm);
        s.write((const char *) &transformed[0],
                transformed.size() * sizeof(int));
    }
}

/*
    Parameters  : fill              - Is the style required for a fill
                                      operation.
    Returns     : draw_fill_style   - The style.
    Description : Construct a fill style for rendering this path.
*/
draw_fill_style drawobj_path::get_fill_style(bool fill) const
{
    draw_fill_style fill_style;

    // Set the basic fill style
    if (fill || width)
    {
        fill_style = draw_FILL_INTERIOR_BOUNDARY | draw_FILL_FULL_INTERIOR;
    }
    else
    {
        fill_style = draw_FILL_EXTERIOR_BOUNDARY | draw_FILL_INTERIOR_BOUNDARY;
    }

    // Change the winding rule if appropriate
    if (fill && (style.flags & drawfile_PATH_WINDING_EVEN_ODD))
    {
        fill_style |= draw_FILL_EVEN_ODD;
    }

    // Return the fill style
    return fill_style;
}

/*
    Parameters  : void
    Returns     : draw_line_style   - The style.
    Description : Construct a line style for rendering this path.
*/
draw_line_style drawobj_path::get_line_style() const
{
    draw_line_style line_style;

    // Join style and end caps
    line_style.join_style = (style.flags & drawfile_PATH_JOIN)
                            >> drawfile_PATH_JOIN_SHIFT;
    line_style.end_cap_style = (style.flags & drawfile_PATH_END)
                               >> drawfile_PATH_END_SHIFT;
    line_style.start_cap_style = (style.flags & drawfile_PATH_START)
                                 >> drawfile_PATH_START_SHIFT;
    line_style.reserved = 0;

    // The mitre limit is implicit for draw files
    line_style.mitre_limit = 10 << 16;

    // Triangle cap sizes
    line_style.start_cap_width = style.cap_width * (256 / 16);
    line_style.start_cap_length = style.cap_length * (256 / 16);
    line_style.end_cap_width = style.cap_width * (256 / 16);
    line_style.end_cap_length = style.cap_length * (256 / 16);

    // Return the line style
    return line_style;
}

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
drawobj_text::drawobj_text() : drawobj_base(drawfile_TYPE_TRFM_TEXT)
{
    // Set default options
    fill = os_COLOUR_BLACK;
    bg_hint = os_COLOUR_WHITE;
    style.font_index = 0;
    xsize = ysize = transform_to_point16.inverse(12 * 16);
    spacing = 0;
    base.x = base.y = 0;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_text::~drawobj_text()
{
    // No action required
}

/*
    Parameters  : base          - The text baseline position.
    Returns     : void
    Description : Set the baseline position.
*/
void drawobj_text::set_base(const os_coord &base)
{
    // Store the baseline position
    this->base = base;

    // Update the details
    update();
}

/*
    Parameters  : colour        - The fill colour.
    Returns     : void
    Description : Set the text colour.
*/
void drawobj_text::set_fill(os_colour colour)
{
    fill = colour;
}

/*
    Parameters  : colour        - The background hint colour.
    Returns     : void
    Description : Set the background colour for antialiasing.
*/
void drawobj_text::set_bg_hint(os_colour colour)
{
    bg_hint = colour;
}

/*
    Parameters  : font          - The name of the font.
                  font_table    - The font table object for this draw file.
    Returns     : void
    Description : Set the font face.
*/
void drawobj_text::set_font(const string &font, drawobj_font_table &font_table)
{
    // Store the font
    this->font = font;

    // Obtain the corresponding number from the font table
    style = font_table.get_font(font);

    // Update the details
    update();
}

/*
    Parameters  : size          - The size in internal units.
    Returns     : void
    Description : Set the font size.
*/
void drawobj_text::set_font_size(int size)
{
    // Store the size and discard any enhanced justification
    xsize = ysize = size;
    spacing = 0;

    // Update the details
    update();
}

/*
    Parameters  : spacing       - Extra spacing between characters in
                                  internal units.
                  width         - Font width if spacing cannot be used.
    Returns     : void
    Description : Set enhanced justification.
*/
void drawobj_text::set_justification(int spacing, int width)
{
    // Store the enhanced justification
    this->spacing = spacing;
    if (width) xsize = width;

    // Update the details
    update();
}

/*
    Parameters  : text          - The text.
    Returns     : void
    Description : Set the text.
*/
void drawobj_text::set_text(const string &text)
{
    // Store the text, treating a single space as being an empty string
    this->text = text != " " ? text : string();

    // Update the details
    update();
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the size and bounding box.
*/
void drawobj_text::update()
{
    // Trivial case if no text
    if (text.empty())
    {
        // Null object if no text
        os_box box;
        box.x0 = 0;
        box.y0 = 0;
        box.x1 = 0;
        box.y1 = 0;
        set_box(box);
        set_size(0);
    }
    else
    {
        // Set the object bounding box
        fontobj_handle handle(font, ysize);
        os_box box;
        box = handle.get_box(text, base, spacing);
        if (spacing || (xsize != ysize))
        {
            os_box box_scale;
            int trfm[6];
            trfm[0] = int(xsize * double(1 << 16) / ysize + 0.5);
            trfm[1] = trfm[2] = 0;
            trfm[3] = 1 << 16;
            trfm[4] = trfm[5] = 0;
            box_scale = handle.get_box(text, base, 0, (os_trfm *) trfm);
            box = combine(box, box_scale);
        }
        set_box(box);

        // Set the object size
        set_size(sizeof(os_trfm)
                 + sizeof(drawfile_text_flags)
                 + sizeof(os_colour) * 2
                 + sizeof(drawfile_text_style)
                 + sizeof(int) * 2
                 + sizeof(os_coord)
                 + ((text.length() + 4) & ~3));
    }
}

/*
    Parameters  : info          - Information required for rendering.
    Returns     : os_error *    - Pointer to a corresponding error
                                  block, or NULL if no error.
    Description : Render this object.
*/
os_error *drawobj_text::paint(render_control &info) const
{
    os_error *err = NULL;

    // No action unless the clip region overlaps the bounding box
    if (overlap(get_box(), info.clip))
    {
        // Paint the text
        fontobj_handle handle(font, info.trfm.to_internal()(ysize));
        err = handle.set_colour(bg_hint, fill);
        if (!err) err = handle.paint(info.trfm, text, base, spacing);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : s             - The stream to write the object to.
                  trfm          - The transformation to apply during
                                  saving.
    Returns     : void
    Description : Write this object to the stream.
*/
void drawobj_text::save(ostream &s, const transform &trfm) const
{
    // No action unless the text has been completed
    if (get_size())
    {
        // Write the text object header
        save_header(s, trfm);

        // Write the text object body
        int identity[6];
        identity[0] = identity[3] = 1 << 16;
        identity[1] = identity[2] = 0;
        identity[4] = identity[5] = 0;
        s.write((const char *) identity, sizeof(os_trfm));
        drawfile_text_flags flags = drawfile_TEXT_KERN;
        s.write((const char *) &flags, sizeof(drawfile_text_flags));
        s.write((const char *) &fill, sizeof(os_colour));
        s.write((const char *) &bg_hint, sizeof(os_colour));
        s.write((const char *) &style, sizeof(drawfile_text_style));
        s.write((const char *) &xsize, sizeof(int));
        s.write((const char *) &ysize, sizeof(int));
        os_coord pos = trfm(base);
        s.write((const char *) &pos, sizeof(os_coord));
        s.write(text.c_str(), text.length());

        // Pad to a word boundary
        int padding = 4 - (text.length() & 3);
        char zero = '\0';
        while (padding--) s.write(&zero, sizeof(zero));
    }
}

/*
    Parameters  : bpp           - Number of bits per pixel.
                  width         - Width in pixels.
                  height        - Height in pixels.
    Returns     : -
    Description : Constructor.
*/
drawobj_sprite::drawobj_sprite(int bpp, int width, int height)
: drawobj_base(drawfile_TYPE_SPRITE)
{
    // Choose the size of the palette and bitmap data
    int palette_entries = bpp <= 8 ? 1 << bpp : 0;
    int palette_size = palette_entries * sizeof(os_colour_pair);
    int bitmap_size = (((width * bpp + 7) / 8 + 3) & ~3) * height;

    // Construct the sprite header
    osspriteop_header header;
    header.size = sizeof(osspriteop_header) + palette_size + bitmap_size;
    memset(header.name, 0, sizeof(header.name));
    strcpy(header.name, drawobj_sprite_name);
    header.width = (width * bpp + 31) / 32 - 1;
    header.height = height - 1;
    header.left_bit = 0;
    header.right_bit = (width * bpp - 1) & 0x1F;
    header.image = sizeof(osspriteop_header) + palette_size;
    header.mask = header.image;
    switch (bpp)
    {
        case 1:     header.mode = os_MODE1BPP90X90;     break;
        case 2:     header.mode = os_MODE2BPP90X90;     break;
        case 4:     header.mode = os_MODE4BPP90X90;     break;
        case 8:     header.mode = os_MODE8BPP90X90;     break;
        case 16:    header.mode = DRAWOBJ_SPRITE16BPP;  break;
        case 32:    header.mode = DRAWOBJ_SPRITE32BPP;  break;
        default:    header.mode = os_CURRENT_MODE;      break;
    }

    // Prepare the sprite block
    sprite.resize(header.size, 0);
    *(osspriteop_header *) &sprite[0] = header;

    // Create a grey scale palette
    if (palette_entries)
    {
        vector<os_colour> palette;
        palette.reserve(palette_entries);
        for (int i = 0; i < palette_entries; i++)
        {
            int grey = (0xff * i) / (palette_entries - 1);
            palette.push_back((grey << 24) | (grey << 16) | (grey << 8));
        }
        set_palette(palette);
    }

    // Set the object size
    set_size(header.size);
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_sprite::~drawobj_sprite()
{
    // No action required
}

/*
    Parameters  : box           - The bounding box for this object.
    Returns     : void
    Description : Set the bounding box for this object.
*/
void drawobj_sprite::set_box(const os_box &box)
{
    drawobj_base::set_box(box);
}

/*
    Parameters  : palette       - The palette data.
    Returns     : void
    Description : Set the palette for this sprite.
*/
void drawobj_sprite::set_palette(const vector<os_colour> &palette)
{
    osspriteop_header &header = *(osspriteop_header *) &sprite[0];

    // Get the size of the palette
    int size = (header.image - sizeof(osspriteop_header))
               / sizeof(os_colour_pair);
    if (palette.size() < size) size = palette.size();

    // Store the palette entries
    os_colour_pair *entries = (os_colour_pair *)
                              &sprite[sizeof(osspriteop_header)];
    for (int i = 0; i < size; i++)
    {
        entries[i].on = entries[i].off = palette[i];
    }
}

/*
    Parameters  : bitmap        - The pixel data.
    Returns     : void
    Description : Set the pixel data for this sprite.
*/
void drawobj_sprite::set_bitmap(const vector<byte> &bitmap)
{
    osspriteop_header &header = *(osspriteop_header *) &sprite[0];

    // Choose the length of bitmap data to copy
    int size = sprite.size() - header.image;
    if (bitmap.size() < size) size = bitmap.size();

    // Copy the bitmap data
    memcpy(&sprite[header.image], &bitmap[0], size);
}

/*
    Parameters  : info          - Information required for rendering.
    Returns     : os_error *    - Pointer to a corresponding error
                                  block, or NULL if no error.
    Description : Render this object.
*/
os_error *drawobj_sprite::paint(render_control &info) const
{
    os_error *err = NULL;

    // No action unless the clip region overlaps the bounding box
    if (overlap(get_box(), info.clip))
    {
        osspriteop_header &header = *(osspriteop_header *) &sprite[0];

        // Transform the bounding box
        os_box box = info.trfm.to_internal()(get_box());
        os_coord rectangle[4];
        rectangle[0].x = rectangle[3].x = box.x0;
        rectangle[2].y = rectangle[3].y = box.y0;
        rectangle[1].x = rectangle[2].x = box.x1;
        rectangle[0].y = rectangle[1].y = box.y1;

        // Generate the colour translation table
        // (this may not handle 256 entry palettes correctly)
        int size;
        err = xcolourtrans_generate_table((os_mode) osspriteop_NAME, (os_palette *) &sprite[0], os_CURRENT_MODE, (os_palette *) -1, NULL, colourtrans_GIVEN_SPRITE | colourtrans_RETURN_WIDE_ENTRIES, NULL, NULL, &size);
        if (!err)
        {
            vector<byte> table(size);
            err = xcolourtrans_generate_table((os_mode) osspriteop_NAME, (os_palette *) &sprite[0], os_CURRENT_MODE, (os_palette *) -1, (osspriteop_trans_tab *) &table[0], colourtrans_GIVEN_SPRITE | colourtrans_RETURN_WIDE_ENTRIES, NULL, NULL, NULL);

            // Plot the sprite
            if (!err) err = xosspriteop_put_sprite_trfm(osspriteop_PTR, DRAWOBJ_SPRITE_AREA, (osspriteop_id) &sprite[0], osspriteop_DESTINATION_COORDS, NULL, os_ACTION_OVERWRITE | osspriteop_GIVEN_WIDE_ENTRIES | osspriteop_DITHERED, (os_trfm *) rectangle, (osspriteop_trans_tab *) &table[0]);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : s             - The stream to write the object to.
                  trfm          - The transformation to apply during
                                  saving.
    Returns     : void
    Description : Write this object to the stream.
*/
void drawobj_sprite::save(ostream &s, const transform &trfm) const
{
    // Write the sprite object header
    save_header(s, trfm);

    // Write the bitmap data
    s.write((const char *) &sprite[0], sprite.size());
}

/*
    Parameters  : void
    Returns     : -
    Description : Destructor.
*/
drawobj_file::ref_class::~ref_class()
{
    // No action required
}

/*
    Parameters  : void
    Returns     : void
    Description : Increment the reference count.
*/
void drawobj_file::ref_class::inc()
{
    // Just increment the reference count
    count++;
}

/*
    Parameters  : void
    Returns     : void
    Description : Decrement the reference count.
*/
void drawobj_file::ref_class::dec()
{
    // Decrease the reference count and delete if appropriate
    if (!--count) delete this;
}

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
drawobj_file::drawobj_file()
{
    // Create a new draw file
    ref = new ref_class;
}

/*
    Parameters  : file          - The draw file to copy.
    Returns     : -
    Description : Constructor.
*/
drawobj_file::drawobj_file(const drawobj_file &file)
{
    // Increase the reference count of the draw file being copied
    if (file.ref) file.ref->inc();

    // Copy the pointer to the reference counted details
    ref = file.ref;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
drawobj_file::~drawobj_file()
{
    // Decrement the reference count
    if (ref) ref->dec();
}

/*
    Parameters  : file          - The draw file to copy.
    Returns     : drawobj_file  - A reference to this draw file.
    Description : Draw file assignment.
*/
drawobj_file &drawobj_file::operator=(const drawobj_file &file)
{
    // Increase the reference count of the draw file being copied
    if (file.ref) file.ref->inc();

    // Reduce the reference count of this draw file
    if (ref) ref->dec();

    // Copy the pointer to the reference counted details
    ref = file.ref;

    // Return a pointer to this draw file
    return *this;
}

/*
    Parameters  : trfm          - The transformation to apply during
                                  rendering.
                  clip          - The clipping rectangle in OS units.
                  resolution    - Dots per OS unit, or 0 for the screen.
    Returns     : os_error *    - Pointer to a corresponding error
                                  block, or NULL if no error.
    Description : Render the file.
*/
os_error *drawobj_file::paint(const transform_internal &trfm,
                              const os_box &clip, int resolution) const
{
    os_error *err = NULL;

    // Transform the clipping rectangle to internal units
    drawobj_base::render_control info;
    info.window = clip;
    info.clip = trfm.to_os().inverse(clip, transform::round_out);

    // Store the transformation matrix
    info.trfm = trfm;

    // Calculate the required flatness
    if (resolution)
    {
        // Use the specified resolution to set the flatness
        info.flatness = trfm.to_os().inverse(1) / resolution;

        // No clipping window
        info.window.x0 = info.window.y0 = info.window.x1 = info.window.y1 = 0;
    }
    else
    {
        // Read the number of OS units per pixel
        int xeig;
        int yeig;
        os_read_mode_variable(os_CURRENT_MODE,
                              os_MODEVAR_XEIG_FACTOR, &xeig);
        os_read_mode_variable(os_CURRENT_MODE,
                              os_MODEVAR_YEIG_FACTOR, &yeig);

        // Use the highest resolution axis to set the flatness
        int pixel = 1 << (xeig < yeig ? xeig : yeig);
        info.flatness = trfm.to_os().inverse(pixel);

        // Read the current graphics window and origin
        static const vars[] =
        {
            os_VDUVAR_GWL_COL,
            os_VDUVAR_GWB_ROW,
            os_VDUVAR_GWR_COL,
            os_VDUVAR_GWT_ROW,
            os_VDUVAR_ORGX,
            os_VDUVAR_ORGY,
            -1
        };
        struct
        {
            os_box window;
            os_coord origin;
        } values;
        os_read_vdu_variables((const os_vdu_var_list *) vars, (int *) &values);

        // Convert the graphics window to OS units
        info.window.x0 = (values.window.x0 << xeig) - values.origin.x;
        info.window.y0 = (values.window.y0 << yeig) - values.origin.y;
        info.window.x1 = (values.window.x1 << xeig) - values.origin.x;
        info.window.y1 = (values.window.y1 << yeig) - values.origin.y;
    }

    // Check whether the GDraw module should be used
    info.gdraw = !resolution
                 && config_current.get_bool(config_tag_print_preview_antialias)
                 && !xosmodule_lookup(DRAWOBJ_MODULE_GDRAW,
                                      NULL, NULL, NULL, NULL, NULL)
                 && !xosmodule_lookup(DRAWOBJ_MODULE_DITHEREXTEND,
                                      NULL, NULL, NULL, NULL, NULL);

    // Paint all of the objects
    err = ref->group.paint(info);

    // Return any error produced
    return err;
}

/*
    Parameters  : s             - The stream to write the file to.
    Returns     : void
    Description : Write this file to the stream.
*/
void drawobj_file::save(ostream &s) const
{
    // Set the paper size and construct the transformation to apply
    drawobj_options options;
    options.set_paper_size(ref->group.get_box());
    os_coord page = options.get_paper_size();
    transform trfm(1.0, 0, page.y);

    // Write the file header
    s.write(drawobj_signature, sizeof(drawobj_signature));
    s.write((const char *) &drawobj_major_version,
            sizeof(drawobj_major_version));
    s.write((const char *) &drawobj_minor_version,
            sizeof(drawobj_minor_version));
    string name = FS_NAME + string(DRAWOBJ_NAME_LENGTH, ' ');
    s.write(name.c_str(), DRAWOBJ_NAME_LENGTH);
    os_box box = trfm(ref->group.get_box());
    s.write((const char *) &box, sizeof(os_box));

    // Write all the objects
    options.save(s, trfm);
    ref->fonts.save(s, trfm);
    ref->group.save(s, trfm);
}

/*
    Parameters  : obj           - The object to add.
    Returns     : void
    Description : Append a draw file object and transfer ownership.
*/
void drawobj_file::add(drawobj_base *obj)
{
    // Add the object to the group
    ref->group.add(obj);
}

/*
    Parameters  : void
    Returns     : os_box        - The bounding box for this file.
    Description : Obtain the bounding box for this file.
*/
os_box drawobj_file::get_box() const
{
    // Return the transformed bounding box
    return transform_to_os(ref->group.get_box());
}

/*
    Parameters  : void
    Returns     : drawobj_font_table    - The font table.
    Description : Obtain the font table for this file.
*/
drawobj_font_table &drawobj_file::get_font_table()
{
    return ref->fonts;
}
