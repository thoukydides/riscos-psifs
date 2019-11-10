/*
    File        : printrendt.c++
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

// Include header file for this module
#include "printrendt.h"

// Separators
#define PRINTRENDT_WIDTH (80)
#define PRINTRENDT_PAGE ('=')
#define PRINTRENDT_SECTION ('-')

// Debug markers
#define PRINTRENDT_MESSAGE_START ("<")
#define PRINTRENDT_MESSAGE_END (">")
#define PRINTRENDT_IMPORTANT_START ("«")
#define PRINTRENDT_IMPORTANT_END ("»")

/*
    Parameters  : s                 - The stream to output to.
    Returns     : -
    Description : Constructor.
*/
printrendt_text::printrendt_text(ostream &s) : s(s)
{
    // Initially no pages rendered
    pages = 0;
    any = FALSE;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printrendt_text::~printrendt_text()
{
    // No action required
}

/*
    Parameters  : void
    Returns     : void
    Description : Begin rendering the page.
*/
void printrendt_text::rend_begin()
{
    // Pass on to the base class
    printrend_base::rend_begin();

    // Output a page separator if not the first page
    if (pages++) s << endl << string(PRINTRENDT_WIDTH, PRINTRENDT_PAGE) << endl;

    // Clear the anything indicator
    any = FALSE;
}

/*
    Parameters  : start             - The start parameters.
    Returns     : void
    Description : Render a start primitive.
*/
void printrendt_text::rend_start(const start_class &start)
{
    UNSET(start)

    // Pass on to the base class
    printrend_base::rend_start(start);

    // Output a section separator if anything in this section
    if (any) s << endl << string(PRINTRENDT_WIDTH, PRINTRENDT_SECTION) << endl;

    // Clear the anything indicator
    any = FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Render a line feed primitive.
*/
void printrendt_text::rend_line_feed()
{
    // Pass on to the base class
    printrend_base::rend_line_feed();

    // Output a newline
    s << endl;
}

/*
    Parameters  : text              - The draw text parameter.
                  pos               - The position parameter.
    Returns     : void
    Description : Render a draw text primitive.
*/
void printrendt_text::rend_draw_text(const string &text, const os_coord &pos)
{
    // Pass on to the base class
    printrend_base::rend_draw_text(text, pos);

    // Output the text with the character set converted
    s << convert_text(text);

    // Remember that something has been output
    any = TRUE;
}

/*
    Parameters  : text              - The draw text parameters.
    Returns     : void
    Description : Render a draw text justified primitive.
*/
void printrendt_text::rend_draw_text_justified(const text_class &text)
{
    // Pass on to the base class
    printrend_base::rend_draw_text_justified(text);

    // Output the text with the character set converted
    s << convert_text(text.text);

    // Remember that something has been output
    any = TRUE;
}

/*
    Parameters  : debug             - The debug message.
                  important         - Is the message important.
    Returns     : void
    Description : Log a debug message.
*/
void printrendt_text::rend_debug(const string &debug, bool important)
{
    // Pass on to the base class
    printrend_base::rend_debug(debug, important);

    // Output the debug text
    s << (important ? PRINTRENDT_IMPORTANT_START : PRINTRENDT_MESSAGE_START)
      << debug
      << (important ? PRINTRENDT_IMPORTANT_END : PRINTRENDT_MESSAGE_END);

    // Remember that something has been output
    any = TRUE;
}
