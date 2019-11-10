/*
    File        : printrendg.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Graphical print job rendering for the PsiFS filer.

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
#include "printrendg.h"

// Include clib header files
#include <math.h>

// Include oslib header files
#include "oslib/colourtrans.h"
#include "oslib/draw.h"
#include "oslib/font.h"
#include "oslib/wimp.h"

// Include cathlibcpp header files
#include "vector.h"

// Base size for pen widths
static const int printrendg_thickness = transform_to_point16.inverse(8);

// Base size for dot and dash patterns
static const int printrendg_dot = 1;
static const int printrendg_dash = printrendg_thickness * 3;
static const int printrendg_gap = printrendg_thickness * 3;

// Settings for debug messages
static const char printrendg_debug_font[] = "Corpus.Medium";
static const int printrendg_debug_size = transform_to_point16.inverse(4 * 16);
static const int printrendg_debug_offset = transform_to_point16.inverse(8 * 16);
static const int printrendg_debug_margin = printrendg_debug_size / 4;
static const os_colour printrendg_debug_normal = os_COLOUR_BLUE;
static const os_colour printrendg_debug_important = os_COLOUR_LIGHT_RED;

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
printrendg_graph::printrendg_graph()
{
    // Construct the parsing transformation
    from_twips = transform::combine(transform_to_twips.inverse(),
                                    transform(1.0, 0, 0, TRUE));

    // No clipping object initially
    clip_obj = NULL;

    // No important debug messages initially
    debug_msgs_important = FALSE;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printrendg_graph::~printrendg_graph()
{
    // Delete any clipping object that may still exist
    delete(clip_obj);
}

/*
    Parameters  : void
    Returns     : void
    Description : Begin rendering the page.
*/
void printrendg_graph::rend_begin()
{
    // Pass on to the base class
    printrend_base::rend_begin();

    // Discard any previous draw file
    draw = drawobj_file();
}

/*
    Parameters  : start             - The draw line start parameter.
                  end               - The draw line end parameter.
    Returns     : void
    Description : Render a draw line primitive.
*/
void printrendg_graph::rend_draw_line(const os_coord &start,
                                      const os_coord &end)
{
    // Pass on to the base class
    printrend_base::rend_draw_line(start, end);

    // Construct the path object
    drawobj_path *obj = make_path(FALSE);
    obj->add_line(from_twips(start), from_twips(end));
    obj->add_end();

    // Add to the draw file
    add(obj);
}

/*
    Parameters  : ellipse           - The draw ellipse parameters.
    Returns     : void
    Description : Render a draw ellipse primitive.
*/
void printrendg_graph::rend_draw_ellipse(const os_box &ellipse)
{
    // Pass on to the base class
    printrend_base::rend_draw_ellipse(ellipse);

    // Construct the path object
    drawobj_path *obj = make_path();
    obj->add_ellipse(from_twips(ellipse));
    obj->add_end();

    // Add to the draw file
    add(obj);
}

/*
    Parameters  : rectangle         - The draw rectangle parameters.
    Returns     : void
    Description : Render a draw rectangle primitive.
*/
void printrendg_graph::rend_draw_rect(const os_box &rectangle)
{
    // Pass on to the base class
    printrend_base::rend_draw_rect(rectangle);

    // Construct the path object
    drawobj_path *obj = make_path();
    obj->add_rectangle(from_twips(rectangle));
    obj->add_end();

    // Add to the draw file
    add(obj);
}

/*
    Parameters  : vertices          - The draw polygon parameters.
                  fill              - The winding rule parameter.
    Returns     : void
    Description : Render a draw polygon primitive.
*/
void printrendg_graph::rend_draw_polygon(const vector<os_coord> &vertices,
                                         fill_rule fill)
{
    // Pass on to the base class
    printrend_base::rend_draw_polygon(vertices, fill);

    // Construct the path object
    drawobj_path *obj = make_path();
    obj->set_winding_rule(fill == fill_rule_winding
                          ? drawobj_path::winding_non_zero
                          : drawobj_path::winding_even_odd);
    for (const os_coord *i = vertices.begin(); i != vertices.end(); i++)
    {
        os_coord pos = from_twips(*i);
        if (i == vertices.begin()) obj->add_move(pos);
        else obj->add_line(pos);
    }
    obj->add_close();
    obj->add_end();

    // Add to the draw file
    add(obj);
}

/*
    Parameters  : dest              - The destination rectangle parameter.
                  bitmap            - The draw bitmap parameter.
    Returns     : void
    Description : Render a draw bitmap rectangle primitive.
*/
void printrendg_graph::rend_draw_bitmap_rect(const os_box &dest,
                                             const bitmap_class &bitmap)
{
    // ... Should decode bpp properly

    // Pass on to the base class
    printrend_base::rend_draw_bitmap_rect(dest, bitmap);

    // Convert the position to internal units
    os_box pos = from_twips(dest);

    // Construct the sprite object
    drawobj_sprite *obj = new drawobj_sprite(1/*bitmap.bpp*/, bitmap.columns,
                                             bitmap.rows);
    obj->set_box(pos);
    obj->set_bitmap(bitmap.pixels);

    // Add to the draw file
    add(obj);
}

/*
    Parameters  : src               - The source rectangle parameter.
                  dest              - The destination rectangle parameter.
                  bitmap            - The draw bitmap parameter.
    Returns     : void
    Description : Render a draw bitmap source primitive.
*/
void printrendg_graph::rend_draw_bitmap_src(const os_box &src,
                                            const os_box &dest,
                                            const bitmap_class &bitmap)
{
    // ... Should decode bpp properly
    // ... Should use the source rectangle

    // Pass on to the base class
    printrend_base::rend_draw_bitmap_src(src, dest, bitmap);

    // Convert the position to internal units
    os_box pos = from_twips(dest);

    // Construct the sprite object
    drawobj_sprite *obj = new drawobj_sprite(1/*bitmap.bpp*/, bitmap.columns,
                                             bitmap.rows);
    obj->set_box(pos);
    obj->set_bitmap(bitmap.pixels);

    // Add to the draw file
    add(obj);
}

/*
    Parameters  : text              - The draw text parameter.
                  pos               - The position parameter.
    Returns     : void
    Description : Render a draw text primitive.
*/
void printrendg_graph::rend_draw_text(const string &text, const os_coord &pos)
{
    // Pass on to the base class
    printrend_base::rend_draw_text(text, pos);

    // Convert the position to internal units
    os_coord start = from_twips(pos);

    // Perform character translations on the text
    string str = convert_text(text);

    // Construct the justification object
    fontobj_obj justify(get_font_name(), get_font_size());
    justify.set_text(str);
    justify.set_position(start.x, start.y);

    // Construct the text object
    drawobj_text *obj = make_text(justify);
    obj->set_text(str);

    // Add objects for underline or strikethrough if required
    if ((get_underline() != font_underline_off)
        || (get_strikethrough() != font_strikethrough_off))
    {
        // Create a group object to contain the text and any lines
        drawobj_group *group = new drawobj_group;
        group->add(obj);
        group->add(make_text_lines(justify));

        // Add the group containing the text and line objects to the draw file
        add(group);
    }
    else
    {
        // Add the raw text object to the draw file
        add(obj);
    }
}

/*
    Parameters  : text              - The draw text parameters.
    Returns     : void
    Description : Render a draw text justified primitive.
*/
void printrendg_graph::rend_draw_text_justified(const text_class &text)
{
    // ... Use the baseline
    // ... Apply the clipping rectangle

    // Pass on to the base class
    printrend_base::rend_draw_text_justified(text);

    // Convert the bounding box to internal units
    os_box pos = from_twips(text.bound);

    // Perform character translations on the text
    string str = convert_text(text.text);

    // Apply the margin
    int left = pos.x0;
    int right = pos.x1;
    if (text.align == text_align_right) right -= from_twips(text.margin);
    else left += from_twips(text.margin);

    // Construct the justification object
    fontobj_obj justify(get_font_name(), get_font_size());
    justify.set_text(str);
    justify.set_position(left, right,
                         (pos.y0 * 3 + pos.y1) / 4
                         - from_twips(get_font().baseline));
    /*
    justify.set_position(left, right,
                         pos.y1 - from_twips(text.baseline));
    */
    justify.set_alignment((fontobj_obj::alignment_style) text.align);

    // Construct a group object
    drawobj_group *group = new drawobj_group;

    // Fill the background if required
    const brush_class &brush = get_brush();
    if (brush.style != brush_style_null)
    {
        drawobj_path *obj = new drawobj_path;
        obj->set_fill(brush.colour);
        obj->add_rectangle(pos);
        obj->add_end();
        group->add(obj);
    }

    // Construct a clipping object
    drawobj_clip *clip = new drawobj_clip(pos);

    // Construct the text object
    drawobj_text *obj = make_text(justify);
    obj->set_text(str);
    clip->add(obj);

    // Add underline or strikethrough if required
    clip->add(make_text_lines(justify));

    // Combine it all together
    group->add(clip);
    add(group);
}

/*
    Parameters  : debug             - The debug message.
                  important         - Is the message important.
    Returns     : void
    Description : Log a debug message.
*/
void printrendg_graph::rend_debug(const string &debug, bool important)
{
    // Pass on to the base class
    printrend_base::rend_debug(debug, important);

    // Add to the list of pending debug messages
    debug_msgs.push_back(debug);
    if (important) debug_msgs_important = TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : End rendering the page.
*/
void printrendg_graph::rend_end()
{
    // Add any clipping object to the draw file
    if (clip_obj)
    {
        draw.add(clip_obj);
        clip_obj = NULL;
    }

    // Add any debug messages that are still outstanding
    draw.add(add_debug());

    // Pass on to the base class
    printrend_base::rend_end();
}

/*
    Parameters  : void
    Returns     : drawobj_file      - The draw file.
    Description : Get the draw file for this page.
*/
const drawobj_file &printrendg_graph::get_draw_file() const
{
    // Return the draw file
    return draw;
}

/*
    Parameters  : void
    Returns     : string            - The current RISC OS font name.
    Description : Read the current mapped font name.
*/
string printrendg_graph::get_font_name()
{
    // Read the current font details
    const font_class &face = get_font();

    // Map and return the font name
    return fontobj_obj::map(face.face, face.screen,
                            face.stroke_weight != stroke_weight_normal,
                            face.posture != posture_upright);
}

/*
    Parameters  : void
    Returns     : int               - The current font size in internal
                                      units.
    Description : Read the current font size.
*/
int printrendg_graph::get_font_size()
{
    // Scale and return the font size
    return from_twips(get_font().size);
}

/*
    Parameters  : filled            - Should the path be filled.
    Returns     : drawobj_path      - The path object.
    Description : Create a path object with the current style selected.
*/
drawobj_path *printrendg_graph::make_path(bool filled)
{
    // Create a path object
    drawobj_path *obj = new drawobj_path;

    // Apply the brush style
    if (filled)
    {
        const brush_class &brush = get_brush();
        if (brush.style != brush_style_null) obj->set_fill(brush.colour);
    }

    // Apply the pen style
    const pen_class &pen = get_pen();
    if (pen.style != pen_style_null)
    {
        // Set the pen colour and size
        obj->set_outline(pen.colour);
        obj->set_width(printrendg_thickness * (pen.size.x + pen.size.y) / 2);

        // Set any dot or dash pattern
        vector<int> pattern;
        switch (pen.style)
        {
            case pen_style_dotted:
                // Dotted (. . . . . . )
                pattern.push_back(printrendg_dot);
                pattern.push_back(printrendg_gap);
                break;
            case pen_style_dashed:
                // Dashed (- - - - - - )
                pattern.push_back(printrendg_dash);
                pattern.push_back(printrendg_gap);
                break;
            case pen_style_dot_dash:
                // Dot dash (- . - . - . )
                pattern.push_back(printrendg_dash);
                pattern.push_back(printrendg_gap);
                pattern.push_back(printrendg_dot);
                pattern.push_back(printrendg_gap);
                break;
            case pen_style_dot_dot_dash:
                // Dot dot dash (- . . - . . )
                pattern.push_back(printrendg_dash);
                pattern.push_back(printrendg_gap);
                pattern.push_back(printrendg_dot);
                pattern.push_back(printrendg_gap);
                pattern.push_back(printrendg_dot);
                pattern.push_back(printrendg_gap);
                break;
            default:
                // Solid
                break;
        }
        obj->set_dash_pattern(0, pattern);
    }

    // Return a pointer to the path object
    return obj;
}

/*
    Parameters  : justify           - The justification object.
    Returns     : drawobj_text      - The text object.
    Description : Create a text object with the current style selected.
*/
drawobj_text *printrendg_graph::make_text(const fontobj_obj &justify)
{
    // Create a text object
    drawobj_text *obj = new drawobj_text;

    // Set the text colours
    obj->set_fill(get_pen().colour);
    const brush_class &brush = get_brush();
    obj->set_bg_hint(brush.style == brush_style_null
                     ? os_COLOUR_WHITE : brush.colour);

    // Set the font name and size
    obj->set_font(get_font_name(), draw.get_font_table());
    obj->set_font_size(get_font_size());

    // Set the text position
    obj->set_base(justify.get_left());

    // Set the enhanced justification
    obj->set_justification(justify.get_spacing(), justify.get_width());

    // Return a pointer to the text object
    return obj;
}

/*
    Parameters  : justify           - The justification object.
    Returns     : drawobj_path      - The path object.
    Description : Create a path object for the underline or strikethrough
                  required for a particular text object.
*/
drawobj_path *printrendg_graph::make_text_lines(const fontobj_obj &justify)
{
    // No action unless either underline or strikethrough enabled
    if (!get_underline() && !get_strikethrough()) return NULL;

    // Create a path object
    drawobj_path *obj = new drawobj_path;

    // Set the path style
    obj->set_fill(get_pen().colour);

    // Add sub paths for underline and strikethrough as appropriate
    if (get_underline() != font_underline_off)
    {
        obj->add_rectangle(justify.get_underline());
    }
    if (get_strikethrough() != font_strikethrough_off)
    {
        obj->add_rectangle(justify.get_strikethrough());
    }

    // End the path
    obj->add_end();

    // Return a pointer to the path object
    return obj;
}

/*
    Parameters  : obj               - The draw object to add.
    Returns     : void
    Description : Add an object to the draw file.
*/
void printrendg_graph::add(drawobj_base *obj)
{
    // Add any pending debug messages
    obj = add_debug(obj);

    // Close any existing clipping object if required
    os_box clip = from_twips(get_clip());
    if (clip_obj
        && ((clip_box.x0 != clip.x0) || (clip_box.y0 != clip.y0)
            || (clip_box.x1 != clip.x1) || (clip_box.y1 != clip.y1)))
    {
        draw.add(clip_obj);
        clip_obj = NULL;
    }

    // Create a new clipping object if required
    if (!clip_obj && (clip.x0 || clip.y0 || clip.x1 || clip.y1))
    {
        // There is an active clipping rectangle
        clip_box = clip;
        clip_obj = new drawobj_clip(clip);
    }

    // Add this object to either the clipping object or the draw file
    if (clip_obj) clip_obj->add(obj);
    else draw.add(obj);
}

/*
    Parameters  : obj               - The draw object to be added.
    Returns     : void
    Description : Add any pending debug messages to an object before it is
                  added to the draw file.
*/
drawobj_base *printrendg_graph::add_debug(drawobj_base *obj)
{
    // No action unless messages are pending
    if (!debug_msgs.empty())
    {
        // Create a group object to contain the debug messages
        drawobj_group *group = new drawobj_group;

        // Choose the colour for these messages
        os_colour colour = debug_msgs_important
                           ? printrendg_debug_important
                           : printrendg_debug_normal;

        // Choose the initial text baseline
        os_box box = obj ? obj->get_box() : draw.get_box();
        os_coord pos;
        pos.x = box.x1 + printrendg_debug_offset;
        pos.y = box.y1;

        // Add text objects for all of the pending debug messages
        while (!debug_msgs.empty())
        {
            // Create a text object
            drawobj_text *text = new drawobj_text;

            // Set the text style for debug messages
            text->set_font(printrendg_debug_font, draw.get_font_table());
            text->set_font_size(printrendg_debug_size);
            text->set_fill(colour);

            // Set the text baseline
            text->set_base(pos);
            pos.y -= printrendg_debug_size;

            // Set the text
            text->set_text(debug_msgs.front());
            debug_msgs.pop_front();

            // Add this debug message to the group
            group->add(text);
        }
        os_box text_box = group->get_box();

        // Add the object to the group
        group->add(obj);

        // Create a path object
        drawobj_path *path = new drawobj_path;

        // Set the path style
        path->set_outline(colour);

        // Add some lines to join the debug messages to their object
        text_box.x0 -= printrendg_debug_margin;
        text_box.y0 -= printrendg_debug_margin;
        text_box.x1 += printrendg_debug_margin;
        text_box.y1 += printrendg_debug_margin;
        path->add_rectangle(text_box);
        os_coord start;
        start.x = box.x1;
        start.y = box.y1;
        pos.x = text_box.x0;
        pos.y = text_box.y1;
        path->add_line(start, pos);
        pos.x = box.x0;
        pos.y = box.y1;
        path->add_line(start, pos);
        pos.x = box.x1;
        pos.y = box.y0;
        path->add_line(start, pos);
        path->add_end();

        // Add the path to the group
        group->add(path);

        // No important debug messages pending
        debug_msgs_important = FALSE;

        // The group replaces the original object
        obj = group;
    }

    // Return the object
    return obj;
}
