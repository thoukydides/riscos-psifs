/*
    File        : printpgobj.h
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

// Only include header file once
#ifndef PRINTPGOBJ_H
#define PRINTPGOBJ_H

// Include cathlibcpp header files
#include "string.h"
#include "istream.h"

// Include project header files
#include "printrend.h"

// Print job page rendering class
class printpgobj_rend
{
public:

    /*
        Parameters  : s                 - The stream containing the page data.
                      rend              - The object to perform the rendering.
        Returns     : -
        Description : Constructor.
    */
    printpgobj_rend(istream &s, printrend_base &rend);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printpgobj_rend();

    /*
        Parameters  : void
        Returns     : void
        Description : Perform the rendering.
    */
    void render();

private:

    istream &s;                         // Stream used to read the page data
    printrend_base &rend;               // Object to perform the rendering
    bool log_debug;                     // Should debug messages be logged

    /*
        Parameters  : void
        Returns     : void
        Description : Render a start primitive.
    */
    void render_start();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set draw mode primitive.
    */
    void render_set_draw_mode();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set clipping rectangle primitive.
    */
    void render_set_clipping_rect();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a cancel clipping rectangle primitive.
    */
    void render_cancel_clipping_rect();

    /*
        Parameters  : void
        Returns     : void
        Description : Render an unknown 06 primitive.
    */
    void render_unknown_06();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a use font primitive.
    */
    void render_use_font();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a discard font primitive.
    */
    void render_discard_font();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set underline style primitive.
    */
    void render_set_underline_style();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set strikethrough style primitive.
    */
    void render_set_strikethrough_style();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a line feed primitive.
    */
    void render_line_feed();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a carriage return primitive.
    */
    void render_carriage_return();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set pen colour primitive.
    */
    void render_set_pen_colour();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set pen style primitive.
    */
    void render_set_pen_style();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set pen size primitive.
    */
    void render_set_pen_size();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set brush colour primitive.
    */
    void render_set_brush_colour();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a set brush style primitive.
    */
    void render_set_brush_style();

    /*
        Parameters  : void
        Returns     : void
        Description : Render an unknown 17 primitive.
    */
    void render_unknown_17();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a draw line primitive.
    */
    void render_draw_line();

    /*
        Parameters  : void
        Returns     : void
        Description : Render an unknown 1B primitive.
    */
    void render_unknown_1b();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a draw ellipse primitive.
    */
    void render_draw_ellipse();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a draw rectangle primitive.
    */
    void render_draw_rect();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a draw polygon primitive.
    */
    void render_draw_polygon();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a draw bitmap rectangle primitive.
    */
    void render_draw_bitmap_rect();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a draw bitmap source primitive.
    */
    void render_draw_bitmap_src();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a draw text primitive.
    */
    void render_draw_text();

    /*
        Parameters  : void
        Returns     : void
        Description : Render a draw text justified primitive.
    */
    void render_draw_text_justified();

    /*
        Parameters  : token             - The message token describing the
                                          problem that was encountered.
                      fatal             - Is the problem fatal.
        Returns     : void
        Description : Log an error.
    */
    void error(const char *token, bool fatal = FALSE);

    /*
        Parameters  : debug             - The debug message.
                      important         - Is the message important.
        Returns     : void
        Description : Log a debug message.
    */
    void debug(const string &debug, bool important = FALSE);

    /*
        Parameters  : void
        Returns     : bits              - The value read from the page data.
        Description : Read the next single byte value from the page data.
    */
    bits get1();

    /*
        Parameters  : void
        Returns     : bits              - The value read from the page data.
        Description : Read the next double byte value from the page data.
    */
    bits get2();

    /*
        Parameters  : void
        Returns     : bits              - The value read from the page data.
        Description : Read the next quad byte value from the page data.
    */
    bits get4();

    /*
        Parameters  : void
        Returns     : os_colour         - The value read from the page data.
        Description : Read the next colour value from the page data.
    */
    os_colour get_colour();

    /*
        Parameters  : void
        Returns     : os_box            - The value read from the page data.
        Description : Read the next bounding box value from the page data.
    */
    os_box get_box();

    /*
        Parameters  : void
        Returns     : string            - The value read from the page data.
        Description : Read the next string value from the page data.
    */
    string get_string();

    /*
        Parameters  : bitmap            - Variable to receive the bitmap data.
        Returns     : void
        Description : Read the next bitmap value from the page data.
    */
    void get_bitmap(printrend_base::bitmap_class &bitmap);
};

// Print job page object class
class printpgobj_obj
{
    // Reference counted details
    struct ref_class
    {
        string name;                    // Name of file containing page data
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
        Parameters  : -
        Returns     : -
        Description : Constructor.
    */
    printpgobj_obj() : ref(NULL) {}

    /*
        Parameters  : name              - Name of the file containing the page
                                          data.
        Returns     : -
        Description : Constructor.
    */
    printpgobj_obj(string name);

    /*
        Parameters  : page              - The page to copy.
        Returns     : -
        Description : Constructor.
    */
    printpgobj_obj(const printpgobj_obj &page);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printpgobj_obj();

    /*
        Parameters  : page              - The page to copy.
        Returns     : printpgobj_obj    - A reference to this page.
        Description : Page assignment.
    */
    printpgobj_obj &operator=(const printpgobj_obj &page);

    /*
        Parameters  : rend              - The object to perform the rendering.
        Returns     : void
        Description : Render the page using the specified object.
    */
    void render(printrend_base &rend);

    /*
        Parameters  : name              - Name of the file to write the page
                                          data to.
        Returns     : -
        Description : Save the page data.
    */
    void save(string name);

    /*
        Parameters  : lhs   - The first page to compare.
                      rhs   - The second page to compare.
        Returns     : bool  - Are the two pages the same.
        Description : Compare two print job pages.
    */
    friend bool operator==(const printpgobj_obj &lhs, const printpgobj_obj &rhs);

    /*
        Parameters  : lhs   - The first page to compare.
                      rhs   - The second page to compare.
        Returns     : bool  - Is lhs < rhs.
        Description : Compare two print job pages.
    */
    friend bool operator<(const printpgobj_obj &lhs, const printpgobj_obj &rhs);
};

#endif
