/*
    File        : printrend.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print job rendering for the PsiFS filer.

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
#include "printrend.h"

/*
    Parameters  : -
    Returns     : -
    Description : Constructor.
*/
printrend_base::printrend_base()
{
    // No fatal errors initially
    fatal = FALSE;

    // Character translation table not initialised initially
    table_init = FALSE;

    // Set the initial state
    reset();
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printrend_base::~printrend_base()
{
    // No action required
}

/*
    Parameters  : void
    Returns     : void
    Description : Begin rendering the page.
*/
void printrend_base::rend_begin()
{
    // Restore the initial state
    reset();
}

/*
    Parameters  : start             - The start parameters.
    Returns     : void
    Description : Render a start primitive.
*/
void printrend_base::rend_start(const start_class &start)
{
    // Restore the initial state and store the section details
    reset();
    this->start = start;
}

/*
    Parameters  : mode              - The draw mode parameters.
    Returns     : void
    Description : Render a set draw mode primitive.
*/
void printrend_base::rend_set_draw_mode(draw_mode mode)
{
    this->mode = mode;
}

/*
    Parameters  : clip              - The set clipping rectangle parameters.
    Returns     : void
    Description : Render a set clipping rectangle primitive.
*/
void printrend_base::rend_set_clipping_rect(const os_box &clip)
{
    this->clip = clip;
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a cancel clipping rectangle primitive.
*/
void printrend_base::rend_cancel_clipping_rect()
{
    clip.x0 = clip.y0 = clip.x1 = clip.y1 = 0;
}

/*
    Parameters  : font              - The use font parameters.
    Returns     : void
    Description : Render a use font primitive.
*/
void printrend_base::rend_use_font(const font_class &font)
{
    this->font = font;
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a discard font primitive.
*/
void printrend_base::rend_discard_font()
{
    font.face = "";
    font.screen = 0;
    font.posture = posture_upright;
    font.stroke_weight = stroke_weight_normal;
    font.print_position = print_pos_normal;
    font.size_base = 0;
    font.size = 0;
}

/*
    Parameters  : style             - The set underline style parameters.
    Returns     : void
    Description : Render a set underline style primitive.
*/
void printrend_base::rend_set_underline_style(font_underline style)
{
    underline = style;
}

/*
    Parameters  : style             - The set strikethrough parameters.
    Returns     : void
    Description : Render a set strikethrough style primitive.
*/
void printrend_base::rend_set_strikethrough_style(font_strikethrough style)
{
    strikethrough = style;
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a line feed primitive.
*/
void printrend_base::rend_line_feed()
{
    // No action required
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a carriage return primitive.
*/
void printrend_base::rend_carriage_return()
{
    // No action required
}

/*
    Parameters  : colour            - The set pen colour parameters.
    Returns     : void
    Description : Render a set pen colour primitive.
*/
void printrend_base::rend_set_pen_colour(os_colour colour)
{
    pen.colour = colour;
}

/*
    Parameters  : style             - The set pen style parameters.
    Returns     : void
    Description : Render a set pen style primitive.
*/
void printrend_base::rend_set_pen_style(pen_style style)
{
    pen.style = style;
}

/*
    Parameters  : size              - The set pen size parameters.
    Returns     : void
    Description : Render a set pen size primitive.
*/
void printrend_base::rend_set_pen_size(const os_coord &size)
{
    pen.size = size;
}

/*
    Parameters  : colour            - The set brush colour parameters.
    Returns     : void
    Description : Render a set brush colour primitive.
*/
void printrend_base::rend_set_brush_colour(os_colour colour)
{
    brush.colour = colour;
}

/*
    Parameters  : style             - The set brush style parameters.
    Returns     : void
    Description : Render a set brush style primitive.
*/
void printrend_base::rend_set_brush_style(brush_style style)
{
    brush.style = style;
}

/*
    Parameters  : start             - The draw line start parameter.
                  end               - The draw line end parameter.
    Returns     : void
    Description : Render a draw line primitive.
*/
void printrend_base::rend_draw_line(const os_coord &start, const os_coord &end)
{
    // No action required
    UNSET(start)
    UNSET(end)
}

/*
    Parameters  : ellipse           - The draw ellipse parameters.
    Returns     : void
    Description : Render a draw ellipse primitive.
*/
void printrend_base::rend_draw_ellipse(const os_box &ellipse)
{
    // No action required
    UNSET(ellipse)
}

/*
    Parameters  : rectangle         - The draw rectangle parameters.
    Returns     : void
    Description : Render a draw rectangle primitive.
*/
void printrend_base::rend_draw_rect(const os_box &rectangle)
{
    // No action required
    UNSET(rectangle)
}

/*
    Parameters  : vertices          - The draw polygon parameters.
                  fill              - The winding rule parameter.
    Returns     : void
    Description : Render a draw polygon primitive.
*/
void printrend_base::rend_draw_polygon(const vector<os_coord> &vertices,
                                       fill_rule fill)
{
    // No action required
    UNSET(vertices)
    UNSET(fill)
}

/*
    Parameters  : dest              - The destination rectangle parameter.
                  bitmap            - The draw bitmap parameter.
    Returns     : void
    Description : Render a draw bitmap rectangle primitive.
*/
void printrend_base::rend_draw_bitmap_rect(const os_box &dest,
                                           const bitmap_class &bitmap)
{
    // No action required
    UNSET(dest)
    UNSET(bitmap)
}

/*
    Parameters  : src               - The source rectangle parameter.
                  dest              - The destination rectangle parameter.
                  bitmap            - The draw bitmap parameter.
    Returns     : void
    Description : Render a draw bitmap source primitive.
*/
void printrend_base::rend_draw_bitmap_src(const os_box &src, const os_box &dest,
                                          const bitmap_class &bitmap)
{
    // No action required
    UNSET(src)
    UNSET(dest)
    UNSET(bitmap)
}

/*
    Parameters  : text              - The draw text parameter.
                  pos               - The position parameter.
    Returns     : void
    Description : Render a draw text primitive.
*/
void printrend_base::rend_draw_text(const string &text, const os_coord &pos)
{
    // No action required
    UNSET(text)
    UNSET(pos)
}

/*
    Parameters  : text              - The draw text parameters.
    Returns     : void
    Description : Render a draw text justified primitive.
*/
void printrend_base::rend_draw_text_justified(const text_class &text)
{
    // No action required
    UNSET(text)
}

/*
    Parameters  : error             - The problem that was encountered.
                  fatal             - Is the problem fatal.
    Returns     : void
    Description : Log an error.
*/
void printrend_base::rend_error(const string &error, bool fatal)
{
    // The first fatal error is special
    if (fatal && !this->fatal)
    {
        // Add to the start of the list of errors and set the fatal flag
        errors.push_front(error);
        this->fatal = TRUE;
    }
    else
    {
        // Add to the end of the list of errors
        errors.push_back(error);
    }
}

/*
    Parameters  : debug             - The debug message.
                  important         - Is the message important.
    Returns     : void
    Description : Log a debug message.
*/
void printrend_base::rend_debug(const string &debug, bool important)
{
    // No action required
    UNSET(debug)
    UNSET(important)
}

/*
    Parameters  : void
    Returns     : void
    Description : End rendering the page.
*/
void printrend_base::rend_end()
{
    // No action required
}

/*
    Parameters  : void
    Returns     : bool              - FALSE if there has been a fatal
                                      error, otherwise TRUE.
    Description : Check the status.
*/
printrend_base::operator bool() const
{
    return !fatal;
}

/*
    Parameters  : void
    Returns     : deque<string>     - List of error messages.
    Description : Return the errors that have been logged.
*/
const deque<string> &printrend_base::get_errors() const
{
    return errors;
}

/*
    Parameters  : void
    Returns     : start_enum        - The current start block.
    Description : Read the current state.
*/
const printrend_base::start_class &printrend_base::get_start() const
{
    return start;
}

/*
    Parameters  : void
    Returns     : os_box            - The current clip region.
    Description : Read the current state.
*/
const os_box &printrend_base::get_clip() const
{
    return clip;
}

/*
    Parameters  : void
    Returns     : draw_mode         - The current draw mode.
    Description : Read the current state.
*/
printrend_base::draw_mode printrend_base::get_draw_mode() const
{
    return mode;
}

/*
    Parameters  : void
    Returns     : font_class        - The current font.
    Description : Read the current state.
*/
const printrend_base::font_class &printrend_base::get_font() const
{
    return font;
}

/*
    Parameters  : void
    Returns     : font_underline    - The current underline style.
    Description : Read the current state.
*/
printrend_base::font_underline printrend_base::get_underline() const
{
    return underline;
}

/*
    Parameters  : void
    Returns     : font_strikethrough - The current strikethrough style.
    Description : Read the current state.
*/
printrend_base::font_strikethrough printrend_base::get_strikethrough() const
{
    return strikethrough;
}

/*
    Parameters  : void
    Returns     : pen_class         - The current pen settings.
    Description : Read the current state.
*/
const printrend_base::pen_class &printrend_base::get_pen() const
{
    return pen;
}

/*
    Parameters  : void
    Returns     : brush_class       - The current brush settings.
    Description : Read the current state.
*/
const printrend_base::brush_class &printrend_base::get_brush() const
{
    return brush;
}

/*
    Parameters  : text              - The text to convert.
    Returns     : string            - The converted text.
    Description : Translate a text string to the local character set.
*/
string printrend_base::convert_text(const string &text) const
{
    // Prepare the character translation table on first use
    if (!table_init)
    {
        printrend_base *this_mutable = (printrend_base *) this;
        psifs_get_translation_table(psifs_WINDOWS_ANSI, psifs_LATIN1,
                                    &this_mutable->table);
        this_mutable->table_init = TRUE;
    }

    // Translate the text
    string out;
    out.reserve(text.length());
    for (const char *i = text.begin(); i != text.end(); i++)
    {
        char mapped = table.mapping[(byte) *i];
        if (mapped) out += mapped;
    }

    // Return the result
    return out;
}

/*
    Parameters  : void
    Returns     : void
    Description : Reset the state.
*/
void printrend_base::reset()
{
    // Restore the initial state
    start.page = 0;
    start.section = section_header;
    mode = draw_mode_pen;
    clip.x0 = clip.y0 = clip.x1 = clip.y1 = 0;
    font.face = "";
    font.screen = 0;
    font.posture = posture_upright;
    font.stroke_weight = stroke_weight_normal;
    font.print_position = print_pos_normal;
    font.size_base = 0;
    font.size = 0;
    font.baseline = 0;
    underline = font_underline_off;
    strikethrough = font_strikethrough_off;
    pen.colour = os_COLOUR_BLACK;
    pen.style = pen_style_solid;
    pen.size.x = pen.size.y = 1;
    brush.colour = os_COLOUR_WHITE;
    brush.style = brush_style_null;
}
