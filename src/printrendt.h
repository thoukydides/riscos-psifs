/*
    File        : printrendt.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Textual print job rendering for the PsiFS filer.

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
#ifndef PRINTRENDT_H
#define PRINTRENDT_H

// Include cathlibcpp header files
#include "ostream.h"

// Include project header files
#include "printrend.h"

// Textual rendering engine class
class printrendt_text : public printrend_base
{
public:

    /*
        Parameters  : s                 - The stream to output to.
        Returns     : -
        Description : Constructor.
    */
    printrendt_text(ostream &s);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printrendt_text();

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
        Parameters  : void
        Returns     : void
        Description : Render a line feed primitive.
    */
    virtual void rend_line_feed();

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

private:

    ostream &s;                         // Stream to write output to
    bits pages;                         // Number of pages rendered
    bits any;                           // Anything in the curren section
};

#endif
