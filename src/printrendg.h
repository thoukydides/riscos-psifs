/*
    File        : printrendg.h
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

// Only include header file once
#ifndef PRINTRENDG_H
#define PRINTRENDG_H

// Include project header files
#include "drawobj.h"
#include "fontobj.h"
#include "printrend.h"
#include "transform.h"

// Graphical rendering engine class
class printrendg_graph : public printrend_base
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    printrendg_graph();

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printrendg_graph();

    /*
        Parameters  : void
        Returns     : void
        Description : Begin rendering the page.
    */
    virtual void rend_begin();

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
        Returns     : drawobj_file      - The draw file.
        Description : Get the draw file for this page.
    */
    const drawobj_file &get_draw_file() const;

private:

    transform from_twips;               // Transformation for parsing
    drawobj_file draw;                  // The draw file being constructed
    os_box clip_box;                    // Current clipping rectangle
    drawobj_clip *clip_obj;             // Current clipping object
    deque<string> debug_msgs;           // Pending debug messages
    bool debug_msgs_important;          // Are any debug messages important

    /*
        Parameters  : void
        Returns     : string            - The current RISC OS font name.
        Description : Read the current mapped font name.
    */
    string get_font_name();

    /*
        Parameters  : void
        Returns     : int               - The current font size in internal
                                          units.
        Description : Read the current font size.
    */
    int get_font_size();

    /*
        Parameters  : filled            - Should the path be filled.
        Returns     : drawobj_path      - The path object.
        Description : Create a path object with the current style selected.
    */
    drawobj_path *make_path(bool filled = TRUE);

    /*
        Parameters  : justify           - The justification object.
        Returns     : drawobj_text      - The text object.
        Description : Create a text object with the current style selected.
    */
    drawobj_text *make_text(const fontobj_obj &justify);

    /*
        Parameters  : justify           - The justification object.
        Returns     : drawobj_path      - The path object.
        Description : Create a path object for the underline or strikethrough
                      required for a particular text object.
    */
    drawobj_path *make_text_lines(const fontobj_obj &justify);

    /*
        Parameters  : obj               - The draw object to add.
        Returns     : void
        Description : Add an object to the draw file.
    */
    void add(drawobj_base *obj);

    /*
        Parameters  : obj               - The draw object to be added.
        Returns     : void
        Description : Add any pending debug messages to an object before it is
                      added to the draw file.
    */
    drawobj_base *add_debug(drawobj_base *obj = NULL);
};

#endif
