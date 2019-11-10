/*
    File        : printrend.h
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

// Only include header file once
#ifndef PRINTREND_H
#define PRINTREND_H

// Include oslib header files
#include "oslib/os.h"

// Include cathlibcpp header files
#include "deque.h"
#include "string.h"
#include "vector.h"

// Include project header files
#include "psifs.h"

// Base rendering engine class
class printrend_base
{
public:

    // Display modes
    enum display_mode
    {
        // display_mode_none,
        display_mode_grey2 = 1,
        // display_mode_grey4,
        // display_mode_grey16,
        // display_mode_grey256,
        // display_mode_colour16,
        // display_mode_colour256,
        // display_mode_colour64k,
        // display_mode_colour16m,
        // display_mode_rgb,
        // display_mode_color4k
    };

    // Font posture
    enum font_posture
    {
        posture_upright,
        posture_italic
    };

    // Font stroke weight
    enum font_stroke_weight
    {
        stroke_weight_normal,
        stroke_weight_bold
    };

    // Font print position
    enum font_print_position
    {
        print_pos_normal,
        print_pos_superscript,
        print_pos_subscript
    };

    // Font underline style
    enum font_underline
    {
        font_underline_off,
        font_underline_on
    };

    // Font strikethrough style
    enum font_strikethrough
    {
        font_strikethrough_off,
        font_strikethrough_on
    };

    // Text alignment
    enum text_align
    {
        text_align_left,
        text_align_centre,
        text_align_right
    };

    // Drawing mode
    enum draw_mode_components
    {
        draw_mode_component_invert_screen = 1,
        draw_mode_component_xor = 2,
        draw_mode_component_or = 4,
        draw_mode_component_and = 8,
        draw_mode_component_logical_op = 14,
        draw_mode_component_invert_pen = 16,
        draw_mode_component_penmode = 32
    };
    enum draw_mode
    {
        // draw_mode_and = draw_mode_component_and,
        // draw_mode_not_and = draw_mode_component_invert_screen
        //                     | draw_mode_component_and,
        draw_mode_pen = draw_mode_component_penmode,
        // draw_mode_and_not = draw_mode_component_and
        //                     | draw_mode_component_invert_pen,
        // draw_mode_xor = draw_mode_component_xor,
        // draw_mode_or = draw_mode_component_or,
        // draw_mode_not_and_not = draw_mode_component_invert_screen
        //                         | draw_mode_component_and
        //                         | draw_mode_component_invert_pen,
        // draw_mode_not_xor = draw_mode_component_invert_screen
        //                     | draw_mode_component_xor,
        // draw_mode_not_screen = draw_mode_component_invert_screen,
        // draw_mode_not_or = draw_mode_component_invert_screen
        //                    | draw_mode_component_or,
        // draw_mode_not_pen = draw_mode_component_invert_pen
        //                     | draw_mode_component_penmode,
        // draw_mode_or_not = draw_mode_component_or
        //                    | draw_mode_component_invert_pen,
        // draw_mode_not_or_not = draw_mode_component_invert_screen
        //                        | draw_mode_component_or
        //                        | draw_mode_component_invert_pen
    };

    // Pen styles
    enum pen_style
    {
        pen_style_null,
        pen_style_solid,
        pen_style_dotted,
        pen_style_dashed,
        pen_style_dot_dash,
        pen_style_dot_dot_dash
    };

    // Brush styles
    enum brush_style
    {
        brush_style_null,
        brush_style_solid,
        // brush_style_patterned,
        // brush_style_vertical_hatch,
        // brush_style_forward_diagonal_hatch,
        // brush_style_horizontal_hatch,
        // brush_style_rearward_diagonal_hatch,
        // brush_style_square_cross_hatch,
        // brush_style_diamond_cross_hatch
    };

    // Fill rules
    enum fill_rule
    {
        fill_rule_alternate,
        fill_rule_winding
    };

    // Region types
    enum section_type
    {
        section_header,
        section_body,
        section_footer
    };
    struct start_class
    {
        bits page;
        section_type section;
    };

    // Font details
    struct font_class
    {
        string face;
        bits screen;
        font_posture posture;
        font_stroke_weight stroke_weight;
        font_print_position print_position;
        bits size_base;
        bits size;
        int baseline;
    };

    // Bitmap details
    struct bitmap_class
    {
        bits columns;
        bits rows;
        bits width;
        bits height;
        display_mode mode;
        vector<byte> pixels;
    };

    // Text details
    struct text_class
    {
        string text;
        os_box bound;
        text_align align;
        int baseline;
        int margin;
    };

    // Pen details
    struct pen_class
    {
        os_colour colour;
        pen_style style;
        os_coord size;
    };

    // Brush details
    struct brush_class
    {
        os_colour colour;
        brush_style style;
    };

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printrend_base();

    /*
        Parameters  : void
        Returns     : void
        Description : Begin rendering the page.
    */
    virtual void rend_begin();

    /*
        Parameters  : start             - The start parameters.
        Returns     : void
        Description : Render a start primitive.
    */
    virtual void rend_start(const start_class &start);

    /*
        Parameters  : mode              - The draw mode parameters.
        Returns     : void
        Description : Render a set draw mode primitive.
    */
    virtual void rend_set_draw_mode(draw_mode mode);

    /*
        Parameters  : clip              - The set clipping rectangle parameters.
        Returns     : void
        Description : Render a set clipping rectangle primitive.
    */
    virtual void rend_set_clipping_rect(const os_box &clip);

    /*
        Parameters  : void
        Returns     : void
        Description : Render a cancel clipping rectangle primitive.
    */
    virtual void rend_cancel_clipping_rect();

    /*
        Parameters  : font              - The use font parameters.
        Returns     : void
        Description : Render a use font primitive.
    */
    virtual void rend_use_font(const font_class &font);

    /*
        Parameters  : void
        Returns     : void
        Description : Render a discard font primitive.
    */
    virtual void rend_discard_font();

    /*
        Parameters  : style             - The set underline style parameters.
        Returns     : void
        Description : Render a set underline style primitive.
    */
    virtual void rend_set_underline_style(font_underline style);

    /*
        Parameters  : style             - The set strikethrough parameters.
        Returns     : void
        Description : Render a set strikethrough style primitive.
    */
    virtual void rend_set_strikethrough_style(font_strikethrough style);

    /*
        Parameters  : void
        Returns     : void
        Description : Render a line feed primitive.
    */
    virtual void rend_line_feed();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a carriage return primitive.
    */
    virtual void rend_carriage_return();

    /*
        Parameters  : colour            - The set pen colour parameters.
        Returns     : void
        Description : Render a set pen colour primitive.
    */
    virtual void rend_set_pen_colour(os_colour colour);

    /*
        Parameters  : style             - The set pen style parameters.
        Returns     : void
        Description : Render a set pen style primitive.
    */
    virtual void rend_set_pen_style(pen_style style);

    /*
        Parameters  : size              - The set pen size parameters.
        Returns     : void
        Description : Render a set pen size primitive.
    */
    virtual void rend_set_pen_size(const os_coord &size);

    /*
        Parameters  : colour            - The set brush colour parameters.
        Returns     : void
        Description : Render a set brush colour primitive.
    */
    virtual void rend_set_brush_colour(os_colour colour);

    /*
        Parameters  : style             - The set brush style parameters.
        Returns     : void
        Description : Render a set brush style primitive.
    */
    virtual void rend_set_brush_style(brush_style style);

    /*
        Parameters  : start             - The draw line start parameter.
                      end               - The draw line end parameter.
        Returns     : void
        Description : Render a draw line primitive.
    */
    virtual void rend_draw_line(const os_coord &start, const os_coord &end);

    /*
        Parameters  : ellipse           - The draw ellipse parameters.
        Returns     : void
        Description : Render a draw ellipse primitive.
    */
    virtual void rend_draw_ellipse(const os_box &ellipse);

    /*
        Parameters  : rectangle         - The draw rectangle parameters.
        Returns     : void
        Description : Render a draw rectangle primitive.
    */
    virtual void rend_draw_rect(const os_box &rectangle);

    /*
        Parameters  : vertices          - The draw polygon parameters.
                      fill              - The winding rule parameter.
        Returns     : void
        Description : Render a draw polygon primitive.
    */
    virtual void rend_draw_polygon(const vector<os_coord> &vertices,
                                   fill_rule fill);

    /*
        Parameters  : dest              - The destination rectangle parameter.
                      bitmap            - The draw bitmap parameter.
        Returns     : void
        Description : Render a draw bitmap rectangle primitive.
    */
    virtual void rend_draw_bitmap_rect(const os_box &dest,
                                       const bitmap_class &bitmap);

    /*
        Parameters  : src               - The source rectangle parameter.
                      dest              - The destination rectangle parameter.
                      bitmap            - The draw bitmap parameter.
        Returns     : void
        Description : Render a draw bitmap source primitive.
    */
    virtual void rend_draw_bitmap_src(const os_box &src, const os_box &dest,
                                      const bitmap_class &bitmap);

    /*
        Parameters  : text              - The draw text parameter.
                      pos               - The position parameter.
        Returns     : void
        Description : Render a draw text primitive.
    */
    virtual void rend_draw_text(const string &text, const os_coord &pos);

    /*
        Parameters  : text              - The draw text parameters.
        Returns     : void
        Description : Render a draw text justified primitive.
    */
    virtual void rend_draw_text_justified(const text_class &text);

    /*
        Parameters  : error             - The problem that was encountered.
                      fatal             - Is the problem fatal.
        Returns     : void
        Description : Log an error.
    */
    virtual void rend_error(const string &error, bool fatal = FALSE);

    /*
        Parameters  : debug             - The debug message.
                      important         - Is the message important.
        Returns     : void
        Description : Log a debug message.
    */
    virtual void rend_debug(const string &debug, bool important = FALSE);

    /*
        Parameters  : void
        Returns     : void
        Description : End rendering the page.
    */
    virtual void rend_end();

    /*
        Parameters  : void
        Returns     : bool              - FALSE if there has been a fatal
                                          error, otherwise TRUE.
        Description : Check the status.
    */
    virtual operator bool() const;

    /*
        Parameters  : void
        Returns     : deque<string>     - List of error messages.
        Description : Return the errors that have been logged.
    */
    virtual const deque<string> &get_errors() const;

protected:

    /*
        Parameters  : -
        Returns     : -
        Description : Constructor.
    */
    printrend_base();

    /*
        Parameters  : void
        Returns     : start_enum        - The current start block.
        Description : Read the current state.
    */
    const start_class &get_start() const;

    /*
        Parameters  : void
        Returns     : os_box            - The current clip region.
        Description : Read the current state.
    */
    const os_box &get_clip() const;

    /*
        Parameters  : void
        Returns     : draw_mode         - The current draw mode.
        Description : Read the current state.
    */
    draw_mode get_draw_mode() const;

    /*
        Parameters  : void
        Returns     : font_class        - The current font.
        Description : Read the current state.
    */
    const font_class &get_font() const;

    /*
        Parameters  : void
        Returns     : font_underline    - The current underline style.
        Description : Read the current state.
    */
    font_underline get_underline() const;

    /*
        Parameters  : void
        Returns     : font_strikethrough - The current strikethrough style.
        Description : Read the current state.
    */
    font_strikethrough get_strikethrough() const;

    /*
        Parameters  : void
        Returns     : pen_class         - The current pen settings.
        Description : Read the current state.
    */
    const pen_class &get_pen() const;

    /*
        Parameters  : void
        Returns     : brush_class       - The current brush settings.
        Description : Read the current state.
    */
    const brush_class &get_brush() const;

    /*
        Parameters  : text              - The text to convert.
        Returns     : string            - The converted text.
        Description : Translate a text string to the local character set.
    */
    string convert_text(const string &text) const;

private:

    bool fatal;                         // Has there been a fatal error
    deque<string> errors;               // List of errors
    start_class start;                  // Current start
    draw_mode mode;                     // Current draw mode
    os_box clip;                        // Current clip region
    font_class font;                    // Current font
    font_underline underline;           // Current underline style
    font_strikethrough strikethrough;   // Current strikethrough style
    pen_class pen;                      // Current pen settings
    brush_class brush;                  // Current brush settings
    psifs_translation_table table;      // Character translation table
    bool table_init;                    // Translation table initialisation

    /*
        Parameters  : void
        Returns     : void
        Description : Reset the state.
    */
    void reset();
};

#endif
