/*
    File        : options.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Option handling for the PsiFS filer.

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

// Prevent OSLib/CathLibCPP name clashes
#include "noclash.h"

// Include header file for this module
#include "options.h"

// Include system header files
#include <stdlib.h>

// Include cathlibcpp header files
#include "fstream.h"

// Include oslib header files
#define map _map
#include "oslib/osfile.h"

// Include project header files
#include "tag.h"

// Options filename
#define OPTIONS_DIR "<PsiFSConfig$Dir>"
#define OPTIONS_PATH "PsiFSConfig:"
#define OPTIONS_FILE "Options"
#define OPTIONS_FILE_READ OPTIONS_PATH OPTIONS_FILE
#define OPTIONS_FILE_WRITE OPTIONS_DIR "." OPTIONS_FILE

// The current options
options_store options_current;

/*
    Parameters  : void
    Returns     : bool      - Was the load successful.
    Description : Attempt to read the options file.
*/
bool options_store::load()
{
    // Attempt to open and read the configuration file
    ifstream file(OPTIONS_FILE_READ);
    return file >> *this ? TRUE : FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to write the options file.
*/
void options_store::save() const
{
    // Ensure that the directory exists
    osfile_create_dir(OPTIONS_DIR, 0);

    // Attempt to write the configuration file
    ofstream file(OPTIONS_FILE_WRITE);
    file << *this;
}
