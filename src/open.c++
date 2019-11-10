/*
    File        : open.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Open and close filer windows for the PsiFS filer.

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
#include "open.h"

// Include cathlibcpp header files
#include "functional.h"
#include "set.h"

// Include project header files
#include "filer.h"
#include "fs.h"

// List of known disc names
static set<string, less<string> > open_discs;

/*
    Parameters  : disc  - The disc identifier.
    Returns     : void
    Description : Open a filer window for the root directory of the specified
                  disc.
*/
void open_open(string disc)
{
    // Construct the command
    string cmd = string("%Filer_OpenDir " FS_NAME) + FS_CHAR_DISC
                 + FS_CHAR_DISC + disc + FS_CHAR_SEPARATOR
                 + FS_CHAR_ROOT;

    // Enable simple error handling
    filer_error_allowed++;

    // Perform the command
    os_cli(cmd.c_str());

    // Restore normal error handling
    filer_error_allowed--;

    // Add to the list of known disc names
    open_discs.insert(disc);
}

/*
    Parameters  : void
    Returns     : void
    Description : Close any filer windows for PsiFS discs.
*/
void open_close()
{
    for (set_const_iterator<string, less<string> > i = open_discs.begin();
         i != open_discs.end();
         )
    {
        string disc = *(i++);
        open_close(disc);
    }
}

/*
    Parameters  : disc  - The disc identifier.
    Returns     : void
    Description : Close any filer window for the specified disc.
*/
void open_close(string disc)
{
    // Construct the command
    string cmd = string("%Filer_CloseDir " FS_NAME) + FS_CHAR_DISC
                 + FS_CHAR_DISC + disc + FS_CHAR_SEPARATOR
                 + FS_CHAR_ROOT;

    // Enable simple error handling
    filer_error_allowed++;

    // Perform the command
    os_cli(cmd.c_str());

    // Restore normal error handling
    filer_error_allowed--;

    // Remove from the list of known disc names
    open_discs.erase(disc);
}
