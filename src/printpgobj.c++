/*
    File        : printpgobj.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print job page processing for the PsiFS filer.

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
#include "printpgobj.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/osfscontrol.h"

// Include cathlibcpp header files
#include "fstream.h"
#include "vector.h"

// Inlcude project header files
#include "config.h"
#include "filer.h"
#include "scrap.h"

// Macros for debugging primitives
#define PRINTPGOBJ_DEBUG_PRIMITIVE(x) debug(x);
#define PRINTPGOBJ_DEBUG_PRIMITIVE_UNKNOWN(x) debug("UNKNOWN_" x, TRUE);

// Known primitives
#define PRINTPGOBJ_START (0x00)
#define PRINTPGOBJ_END (0x01)
//#define PRINTPGOBJ_UNKNOWN_02 (0x02)
#define PRINTPGOBJ_SET_DRAW_MODE (0x03)
#define PRINTPGOBJ_SET_CLIPPING_RECT (0x04)
#define PRINTPGOBJ_CANCEL_CLIPPING_RECT (0x05)
#define PRINTPGOBJ_UNKNOWN_06 (0x06)
#define PRINTPGOBJ_USE_FONT (0x07)
#define PRINTPGOBJ_DISCARD_FONT (0x08)
#define PRINTPGOBJ_SET_UNDERLINE_STYLE (0x09)
#define PRINTPGOBJ_SET_STRIKETHROUGH_STYLE (0x0A)
#define PRINTPGOBJ_LINE_FEED (0x0B)
#define PRINTPGOBJ_CARRIAGE_RETURN (0x0C)
#define PRINTPGOBJ_SET_PEN_COLOUR (0x0D)
#define PRINTPGOBJ_SET_PEN_STYLE (0x0E)
#define PRINTPGOBJ_SET_PEN_SIZE (0x0F)
#define PRINTPGOBJ_SET_BRUSH_COLOUR (0x10)
#define PRINTPGOBJ_SET_BRUSH_STYLE (0x11)
//#define PRINTPGOBJ_SET_BRUSH_ORIGIN (0x12)
//#define PRINTPGOBJ_SET_BRUSH_PATTERN (0x13)
//#define PRINTPGOBJ_DISCARD_BRUSH_PATTERN (0x14)
//#define PRINTPGOBJ_UNKNOWN_15 (0x15)
//#define PRINTPGOBJ_UNKNOWN_16 (0x16)
#define PRINTPGOBJ_UNKNOWN_17 (0x17)
//#define PRINTPGOBJ_UNKNOWN_18 (0x18)
#define PRINTPGOBJ_DRAW_LINE (0x19)
//#define PRINTPGOBJ_UNKNOWN_1A (0x1A)
#define PRINTPGOBJ_UNKNOWN_1B (0x1B)
//#define PRINTPGOBJ_UNKNOWN_1C (0x1C)
//#define PRINTPGOBJ_UNKNOWN_1D (0x1D)
//#define PRINTPGOBJ_UNKNOWN_1E (0x1E)
#define PRINTPGOBJ_DRAW_ELLIPSE (0x1F)
#define PRINTPGOBJ_DRAW_RECT (0x20)
//#define PRINTPGOBJ_UNKNOWN_21 (0x21)
//#define PRINTPGOBJ_UNKNOWN_22 (0x22)
#define PRINTPGOBJ_DRAW_POLYGON (0x23)
//#define PRINTPGOBJ_UNKNOWN_1E (0x24)
#define PRINTPGOBJ_DRAW_BITMAP_RECT (0x25)
#define PRINTPGOBJ_DRAW_BITMAP_SRC (0x26)
#define PRINTPGOBJ_DRAW_TEXT (0x27)
#define PRINTPGOBJ_DRAW_TEXT_JUSTIFIED (0x28)

// Bitmap encodings
#define PRINTPGOBJ_BITMAP_UNCOMPRESSED (0x00)
#define PRINTPGOBJ_BITMAP_RLE (0x01)

// Magic values
#define PRINTPGOBJ_HEADER (0x000003e8)

// Start sections
#define PRINTPGOBJ_START_HEADER (0x00)
#define PRINTPGOBJ_START_BODY (0x01)
#define PRINTPGOBJ_START_FOOTER1 (0x02)
#define PRINTPGOBJ_START_FOOTER2 (0x03)

// Font style bits
#define PRINTPGOBJ_FONT_POSTURE (1 << 0)
#define PRINTPGOBJ_FONT_POSTURE_SHIFT (0)
#define PRINTPGOBJ_FONT_WEIGHT (1 << 1)
#define PRINTPGOBJ_FONT_WEIGHT_SHIFT (1)
#define PRINTPGOBJ_FONT_POSITION (3 << 2)
#define PRINTPGOBJ_FONT_POSITION_SHIFT (2)
#define PRINTPGOBJ_FONT_RESERVED (0xfffffff0)

/*
    Parameters  : s                 - The stream containing the page data.
                  rend              - The object to perform the rendering.
    Returns     : -
    Description : Constructor.
*/
printpgobj_rend::printpgobj_rend(istream &s, printrend_base &rend)
: s(s), rend(rend)
{
    // Check whether debug messages should be logged
    log_debug = config_current.get_bool(config_tag_print_log_debug);
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printpgobj_rend::~printpgobj_rend()
{
    // No action required
}

/*
    Parameters  : void
    Returns     : void
    Description : Perform the rendering.
*/
void printpgobj_rend::render()
{
    // Check the file header
    if (get4() != PRINTPGOBJ_HEADER) error("PrnEHW1");
    if (get4() != PRINTPGOBJ_HEADER) error("PrnEHW2");

    // Process the whole page
    bool done = FALSE;
    while (!done && s && rend)
    {
        // Obtain the tag for the next primitive
        bits primitive = get1();

        // Decode this primitive
        switch (primitive)
        {
            case PRINTPGOBJ_START:
                render_start();
                break;
            case PRINTPGOBJ_END:
                done = TRUE;
                break;
            case PRINTPGOBJ_SET_DRAW_MODE:
                render_set_draw_mode();
                break;
            case PRINTPGOBJ_SET_CLIPPING_RECT:
                render_set_clipping_rect();
                break;
            case PRINTPGOBJ_CANCEL_CLIPPING_RECT:
                render_cancel_clipping_rect();
                break;
            case PRINTPGOBJ_UNKNOWN_06:
                render_unknown_06();
                break;
            case PRINTPGOBJ_USE_FONT:
                render_use_font();
                break;
            case PRINTPGOBJ_DISCARD_FONT:
                render_discard_font();
                break;
            case PRINTPGOBJ_SET_UNDERLINE_STYLE:
                render_set_underline_style();
                break;
            case PRINTPGOBJ_SET_STRIKETHROUGH_STYLE:
                render_set_strikethrough_style();
                break;
            case PRINTPGOBJ_LINE_FEED:
                render_line_feed();
                break;
            case PRINTPGOBJ_CARRIAGE_RETURN:
                render_carriage_return();
                break;
            case PRINTPGOBJ_SET_PEN_COLOUR:
                render_set_pen_colour();
                break;
            case PRINTPGOBJ_SET_PEN_STYLE:
                render_set_pen_style();
                break;
            case PRINTPGOBJ_SET_PEN_SIZE:
                render_set_pen_size();
                break;
            case PRINTPGOBJ_SET_BRUSH_COLOUR:
                render_set_brush_colour();
                break;
            case PRINTPGOBJ_SET_BRUSH_STYLE:
                render_set_brush_style();
                break;
            case PRINTPGOBJ_UNKNOWN_17:
                render_unknown_17();
                break;
            case PRINTPGOBJ_DRAW_LINE:
                render_draw_line();
                break;
            case PRINTPGOBJ_UNKNOWN_1B:
                render_unknown_1b();
                break;
            case PRINTPGOBJ_DRAW_ELLIPSE:
                render_draw_ellipse();
                break;
            case PRINTPGOBJ_DRAW_RECT:
                render_draw_rect();
                break;
            case PRINTPGOBJ_DRAW_POLYGON:
                render_draw_polygon();
                break;
            case PRINTPGOBJ_DRAW_BITMAP_RECT:
                render_draw_bitmap_rect();
                break;
            case PRINTPGOBJ_DRAW_BITMAP_SRC:
                render_draw_bitmap_src();
                break;
            case PRINTPGOBJ_DRAW_TEXT:
                render_draw_text();
                break;
            case PRINTPGOBJ_DRAW_TEXT_JUSTIFIED:
                render_draw_text_justified();
                break;
            default:
                error("PrnECmU", TRUE);
                break;
        }
    }

    // Check that the whole page was processed
    if (s.eof()) error("PrnEFEf", TRUE);
    else if (s.peek() != EOF) error("PrnEFIn", TRUE);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a start primitive.
*/
void printpgobj_rend::render_start()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("START")

    // Decode this primitive
    bits value = get4();
    printrend_base::start_class start;
    start.page = value >> 2;
    switch (value & 0x03)
    {
        case PRINTPGOBJ_START_HEADER:
            start.section = printrend_base::section_header;
            break;

        case PRINTPGOBJ_START_BODY:
            start.section = printrend_base::section_body;
            break;

        case PRINTPGOBJ_START_FOOTER1:
        case PRINTPGOBJ_START_FOOTER2:
            start.section = printrend_base::section_footer;
            break;
    }

    // Render this primitive
    rend.rend_start(start);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set draw mode primitive.
*/
void printpgobj_rend::render_set_draw_mode()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_DRAW_MODE")

    // Decode this primitive
    printrend_base::draw_mode mode = (printrend_base::draw_mode) get1();
    if (mode != printrend_base::draw_mode_pen) error("PrnEDMU");

    // Render this primitive
    rend.rend_set_draw_mode(mode);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set clipping rectangle primitive.
*/
void printpgobj_rend::render_set_clipping_rect()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_CLIPPING_RECT")

    // Decode this primitive
    os_box clip = get_box();

    // Render this primitive
    rend.rend_set_clipping_rect(clip);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a cancel clipping rectangle primitive.
*/
void printpgobj_rend::render_cancel_clipping_rect()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("CANCEL_CLIPPING_RECT")

    // Render this primitive
    rend.rend_cancel_clipping_rect();
}

/*
    Parameters  : void
    Returns     : void
    Description : Render an unknown 06 primitive.
*/
void printpgobj_rend::render_unknown_06()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE_UNKNOWN("06")

    // No parameters
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a use font primitive.
*/
void printpgobj_rend::render_use_font()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("USE_FONT")

    // Decode this primitive
    printrend_base::font_class font;
    font.face = get_string();
    font.screen = get1();
    font.size_base = get2();
    bits value = get4();
    font.posture = (printrend_base::font_posture)
                   ((value & PRINTPGOBJ_FONT_POSTURE)
                    >> PRINTPGOBJ_FONT_POSTURE_SHIFT);
    font.stroke_weight = (printrend_base::font_stroke_weight)
                         ((value & PRINTPGOBJ_FONT_WEIGHT)
                          >> PRINTPGOBJ_FONT_WEIGHT_SHIFT);
    font.print_position = (printrend_base::font_print_position)
                          ((value & PRINTPGOBJ_FONT_POSITION)
                           >> PRINTPGOBJ_FONT_POSITION_SHIFT);
    if (value & PRINTPGOBJ_FONT_RESERVED) error("PrnEFnS");
    font.size = get2();
    if (get2() != 0) error("PrnEFnP");
    font.baseline = int(get4());

    // Render this primitive
    rend.rend_use_font(font);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a discard font primitive.
*/
void printpgobj_rend::render_discard_font()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DISCARD_FONT")

    // Render this primitive
    rend.rend_discard_font();
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set underline style primitive.
*/
void printpgobj_rend::render_set_underline_style()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_UNDERLINE_STYLE")

    // Decode this primitive
    printrend_base::font_underline style = (printrend_base::font_underline) get1();
    if ((style != printrend_base::font_underline_off)
        && (style != printrend_base::font_underline_on))
    {
        error("PrnEUlP");
    }

    // Render this primitive
    rend.rend_set_underline_style(style);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set strikethrough style primitive.
*/
void printpgobj_rend::render_set_strikethrough_style()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_STRIKETHROUGH_STYLE")

    // Decode this primitive
    printrend_base::font_strikethrough style = (printrend_base::font_strikethrough) get1();
    if ((style != printrend_base::font_strikethrough_off)
        && (style != printrend_base::font_strikethrough_on))
    {
        error("PrnESkP");
    }

    // Render this primitive
    rend.rend_set_strikethrough_style(style);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a line feed primitive.
*/
void printpgobj_rend::render_line_feed()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("LINE_FEED")

    // Decode this primitive
    bits x = get4();
    bits y = get4();

    // Render this primitive
    rend.rend_line_feed();
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a carriage return primitive.
*/
void printpgobj_rend::render_carriage_return()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("CARRIAGE_RETURN")

    // Check that the parameter is as expected
    if (get4() != 0) error("PrnECrP");
    if (get4() != 0) error("PrnECrP");

    // Render this primitive
    rend.rend_carriage_return();
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set pen colour primitive.
*/
void printpgobj_rend::render_set_pen_colour()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_PEN_COLOUR")

    // Decode this primitive
    os_colour colour = get_colour();

    // Render this primitive
    rend.rend_set_pen_colour(colour);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set pen style primitive.
*/
void printpgobj_rend::render_set_pen_style()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_PEN_STYLE")

    // Decode this primitive
    printrend_base::pen_style style = (printrend_base::pen_style) get1();

    // Render this primitive
    rend.rend_set_pen_style(style);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set pen size primitive.
*/
void printpgobj_rend::render_set_pen_size()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_PEN_SIZE")

    // Decode this primitive
    os_coord size;
    size.x = get4();
    size.y = get4();
    if (size.x != size.y) error("PrnEPZP");

    // Render this primitive
    rend.rend_set_pen_size(size);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set brush colour primitive.
*/
void printpgobj_rend::render_set_brush_colour()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_BRUSH_COLOUR")

    // Decode this primitive
    os_colour colour = get_colour();

    // Render this primitive
    rend.rend_set_brush_colour(colour);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a set brush style primitive.
*/
void printpgobj_rend::render_set_brush_style()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("SET_BRUSH_STYLE")

    // Decode this primitive
    printrend_base::brush_style style = (printrend_base::brush_style) get1();
    if ((style != printrend_base::brush_style_null)
        && (style != printrend_base::brush_style_solid))
    {
        error("PrnEBSP");
    }

    // Render this primitive
    rend.rend_set_brush_style(style);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render an unknown 17 primitive.
*/
void printpgobj_rend::render_unknown_17()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE_UNKNOWN("17")

    // Decode this primitive
    bits x = get4();
    bits y = get4();
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a draw line primitive.
*/
void printpgobj_rend::render_draw_line()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DRAW_LINE")

    // Decode this primitive
    os_coord start;
    start.x = get4();
    start.y = get4();
    os_coord end;
    end.x = get4();
    end.y = get4();

    // Render this primitive
    rend.rend_draw_line(start, end);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render an unknown 1B primitive.
*/
void printpgobj_rend::render_unknown_1b()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE_UNKNOWN("1B")

    // Decode this primitive
    if (get4() != 0) error("PrnE1BP");
    get4();
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a draw ellipse primitive.
*/
void printpgobj_rend::render_draw_ellipse()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DRAW_ELLIPSE")

    // Decode this primitive
    os_box ellipse = get_box();

    // Render this primitive
    rend.rend_draw_ellipse(ellipse);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a draw rectangle primitive.
*/
void printpgobj_rend::render_draw_rect()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DRAW_RECT")

    // Decode this primitive
    os_box rectangle = get_box();

    // Render this primitive
    rend.rend_draw_rect(rectangle);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a draw polygon primitive.
*/
void printpgobj_rend::render_draw_polygon()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DRAW_POLYGON")

    // Decode this primitive
    bits number = get4();
    vector<os_coord> vertices;
    vertices.reserve(number);
    for (int i = 0; i < number; i++)
    {
        os_coord pos;
        pos.x = get4();
        pos.y = get4();
        vertices.push_back(pos);
    }
    printrend_base::fill_rule fill = (printrend_base::fill_rule) get1();
    if ((fill != printrend_base::fill_rule_alternate)
        && (fill != printrend_base::fill_rule_winding))
    {
        error("PrnEDPP");
    }

    // Render this primitive
    rend.rend_draw_polygon(vertices, fill);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a draw bitmap rectangle primitive.
*/
void printpgobj_rend::render_draw_bitmap_rect()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DRAW_BITMAP_RECT")

    // Decode this primitive
    os_box dest = get_box();
    printrend_base::bitmap_class bitmap;
    get_bitmap(bitmap);

    // Render this primitive
    rend.rend_draw_bitmap_rect(dest, bitmap);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a draw bitmap source primitive.
*/
void printpgobj_rend::render_draw_bitmap_src()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DRAW_BITMAP_SRC")

    // Decode this primitive
    os_box dest = get_box();
    printrend_base::bitmap_class bitmap;
    get_bitmap(bitmap);
    os_box src = get_box();

    // Render this primitive
    rend.rend_draw_bitmap_src(src, dest, bitmap);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a draw text primitive.
*/
void printpgobj_rend::render_draw_text()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DRAW_TEXT")

    // Decode this primitive
    string text = get_string();
    os_coord pos;
    pos.x = get4();
    pos.y = get4();

    // Render this primitive
    rend.rend_draw_text(text, pos);
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a draw text justified primitive.
*/
void printpgobj_rend::render_draw_text_justified()
{
    PRINTPGOBJ_DEBUG_PRIMITIVE("DRAW_TEXT_JUSTIFIED")

    // Decode this primitive
    printrend_base::text_class text;
    text.text = get_string();
    text.bound = get_box();
    text.baseline = get4();
    text.align = (printrend_base::text_align) get1();
    if ((text.align != printrend_base::text_align_left)
        && (text.align != printrend_base::text_align_centre)
        && (text.align != printrend_base::text_align_right))
    {
        error("PrnEDTA");
    }
    text.margin = get4();

    // Render this primitive
    rend.rend_draw_text_justified(text);
}

/*
    Parameters  : token             - The message token describing the
                                      problem that was encountered.
                  fatal             - Is the problem fatal.
    Returns     : void
    Description : Log an error.
*/
void printpgobj_rend::error(const char *token, bool fatal)
{
    // Construct the error message
    char str[10];
    sprintf(str, "%x", s.tellg());
    string error = filer_msgtrans("PrnTxAt",
                                  filer_msgtrans(token).c_str(), str);

    // Log the error
    rend.rend_error(error, fatal);
    if (log_debug)
    {
        rend.rend_debug((fatal ? "Error: " : "Warning: ") + error, TRUE);
    }
}

/*
    Parameters  : debug             - The debug message.
                  important         - Is the message important.
    Returns     : void
    Description : Log a debug message.
*/
void printpgobj_rend::debug(const string &debug, bool important)
{
    // No action unless logging is enabled
    if (log_debug)
    {
        // Log the message with the current location prefixed
        char str[13];
        sprintf(str, "@%x: ", s.tellg());
        rend.rend_debug(str + debug, important);
    }
}

/*
    Parameters  : void
    Returns     : bits              - The value read from the page data.
    Description : Read the next single byte value from the page data.
*/
bits printpgobj_rend::get1()
{
    char c;
    s.read(&c, 1);
    return byte(c);
}

/*
    Parameters  : void
    Returns     : bits              - The value read from the page data.
    Description : Read the next double byte value from the page data.
*/
bits printpgobj_rend::get2()
{
    bits value;
    value = get1();
    value += get1() << 8;
    return int(value);
}

/*
    Parameters  : void
    Returns     : bits              - The value read from the page data.
    Description : Read the next quad byte value from the page data.
*/
bits printpgobj_rend::get4()
{
    bits value;
    value = get1();
    value += get1() << 8;
    value += get1() << 16;
    value += get1() << 24;
    return int(value);
}

/*
    Parameters  : void
    Returns     : os_colour         - The value read from the page data.
    Description : Read the next colour value from the page data.
*/
os_colour printpgobj_rend::get_colour()
{
    bits red = get1();
    bits green = get1();
    bits blue = get1();
    return (red << 8) | (green << 16) | (blue << 24);
}

/*
    Parameters  : void
    Returns     : os_box            - The value read from the page data.
    Description : Read the next bounding box value from the page data.
*/
os_box printpgobj_rend::get_box()
{
    os_box box;
    box.x0 = get4();
    box.y1 = get4();
    box.x1 = get4();
    box.y0 = get4();
    return box;
}

/*
    Parameters  : void
    Returns     : string            - The value read from the page data.
    Description : Read the next string value from the page data.
*/
string printpgobj_rend::get_string()
{
    // Decode the length field
    bits value = get1();
    size_t length = value & 0x01 ? (value >> 3) | (get1() << 5) : value >> 2;
    if (((value >> (value & 0x01 ? 1 : 0)) & 0x03) != 0x02) error("PrnELnB");

    // Get the string data
    vector<char> buf(length);
    s.read(&buf[0], length);

    // Return the resulting string
    return string(&buf[0], length);
}

/*
    Parameters  : bitmap            - Variable to receive the bitmap data.
    Returns     : void
    Description : Read the next bitmap value from the page data.
*/
void printpgobj_rend::get_bitmap(printrend_base::bitmap_class &bitmap)
{
    // Remember the start position
    int start = s.tellg();

    // Decode the bitmap header
    bits length = get4();
    bits offset = get4();
    bitmap.columns = get4();
    bitmap.rows = get4();
    bitmap.width = get4();
    bitmap.height = get4();
    bitmap.mode = (printrend_base::display_mode) get4();
    if (bitmap.mode != printrend_base::display_mode_grey2) error("PrnEBiD");
    if (get4() != 0) error("PrnEBiP");
    if (get4() != 0) error("PrnEBiP");
    bits encoding = get4();

    // Advance to the start of the pixel data if required
    if (offset != 0x28) s.seekg(start + offset);

    // Decode the pixel data
    bitmap.pixels.resize(0, 0);
    switch (encoding)
    {
        case PRINTPGOBJ_BITMAP_UNCOMPRESSED:
            // Raw pixel data
            bitmap.pixels.resize(length - offset, 0);
            s.read((char *) &bitmap.pixels[0], length - offset);
            break;

        case PRINTPGOBJ_BITMAP_RLE:
            // Run length encoded pixel data (assume 1bbp for reserved size)
            bitmap.pixels.reserve((((bitmap.columns + 7) / 8 + 3) & ~3) * bitmap.rows);
            while (s.tellg() < (start + length))
            {
                bits marker = get1();
                if (marker < 0x80)
                {
                    bitmap.pixels.insert(bitmap.pixels.end(), marker + 1,
                                         get1());
                }
                else
                {
                    bits raw = 0x100 - marker;
                    bits at = bitmap.pixels.size();
                    bitmap.pixels.resize(at + raw, 0);
                    s.read((char *) &bitmap.pixels[at], raw);
                }
            }
            break;

        default:
            // Unsupported encoding
            error("PrnEBiE");
            s.seekg(start + length);
            break;
    }
}

/*
    Parameters  : void
    Returns     : -
    Description : Destructor.
*/
printpgobj_obj::ref_class::~ref_class()
{
    // Delete the page data if the reference count has reached zero
    if (!count) xosfscontrol_wipe(name.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
}

/*
    Parameters  : void
    Returns     : void
    Description : Increment the reference count.
*/
void printpgobj_obj::ref_class::inc()
{
    // Just increment the reference count
    count++;
}

/*
    Parameters  : void
    Returns     : void
    Description : Decrement the reference count.
*/
void printpgobj_obj::ref_class::dec()
{
    // Decrease the reference count and delete if appropriate
    if (!--count) delete this;
}

/*
    Parameters  : name              - Name of the file containing the page
                                      data.
    Returns     : -
    Description : Constructor.
*/
printpgobj_obj::printpgobj_obj(string name)
{
    // Create the page
    ref = new ref_class;

    // Enable simple error handling
    filer_error_allowed++;

    // Copy the file data
    ref->name = scrap_name();
    osfscontrol_copy(name.c_str(), ref->name.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);

    // Restore normal error handling
    filer_error_allowed--;
}

/*
    Parameters  : page              - The page to copy.
    Returns     : -
    Description : Constructor.
*/
printpgobj_obj::printpgobj_obj(const printpgobj_obj &page)
{
    // Increase the reference count of the page being copied
    if (page.ref) page.ref->inc();

    // Copy the pointer to the reference counted details
    ref = page.ref;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printpgobj_obj::~printpgobj_obj()
{
    // Decrement the reference count
    if (ref) ref->dec();
}

/*
    Parameters  : page              - The page to copy.
    Returns     : printpgobj_obj    - A reference to this page.
    Description : Page assignment.
*/
printpgobj_obj &printpgobj_obj::operator=(const printpgobj_obj &page)
{
    // Increase the reference count of the page being copied
    if (page.ref) page.ref->inc();

    // Reduce the reference count of this page
    if (ref) ref->dec();

    // Copy the pointer to the reference counted details
    ref = page.ref;

    // Return a pointer to this page
    return *this;
}

/*
    Parameters  : rend              - The object to perform the rendering.
    Returns     : void
    Description : Render the page using the specified object.
*/
void printpgobj_obj::render(printrend_base &rend)
{
    // No action unless active
    if (ref)
    {
        // Prepare to render this page
        rend.rend_begin();

        // Open a stream on the page data
        ifstream page(ref->name.c_str(), ios_base::in | ios_base::binary);

        // Render the page
        if (page) printpgobj_rend(page, rend).render();
        else rend.rend_error(filer_msgtrans("PrnEFOp", ref->name.c_str()), TRUE);

        // End rendering this page
        rend.rend_end();
    }
}

/*
    Parameters  : name              - Name of the file to write the page
                                      data to.
    Returns     : -
    Description : Save the page data.
*/
void printpgobj_obj::save(string name)
{
    // No action unless active
    if (ref)
    {
        // Enable simple error handling
        filer_error_allowed++;

        // Copy the page data
        osfscontrol_copy(ref->name.c_str(), name.c_str(), osfscontrol_COPY_RECURSE, 0, 0, 0, 0, NULL);

        // Restore normal error handling
        filer_error_allowed--;
    }
}

/*
    Parameters  : lhs   - The first page to compare.
                  rhs   - The second page to compare.
    Returns     : bool  - Are the two pages the same.
    Description : Compare two print job pages.
*/
bool operator==(const printpgobj_obj &lhs, const printpgobj_obj &rhs)
{
    // Compare the reference data pointers
    return lhs.ref == rhs.ref;
}

/*
    Parameters  : lhs   - The first page to compare.
                  rhs   - The second page to compare.
    Returns     : bool  - Is lhs < rhs.
    Description : Compare two print job pages.
*/
bool operator<(const printpgobj_obj &lhs, const printpgobj_obj &rhs)
{
    // Compare the reference data pointers
    return lhs.ref < rhs.ref;
}
