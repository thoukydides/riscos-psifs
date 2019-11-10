/*
    File        : scrap.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Scrap directory manipulation for the PsiFS filer.

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
#include "scrap.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "fs.h"

// Directory to contain scrap files
#define SCRAP_DIR "<PsiFSScrap$Dir>"
#define SCRAP_SUBDIR "<PsiFSScrap$Dir>.Filer"
#define SCRAP_PATH "PsiFSScrap:Filer."

/*
    Parameters  : void
    Returns     : const char *  - Pointer to a unique leaf name.
    Description : Generate a unique leaf name.
*/
const char *scrap_unique()
{
    static bits count = ((bits) os_read_monotonic_time()) << 16;
    static fs_leafname leaf;

    // Generate a new leaf name
    sprintf(leaf, "FS%08X", count++);

    // Return a pointer to the name
    return leaf;
}

/*
    Parameters  : name      - Optional leaf name for the file.
    Returns     : string    - The file name.
    Description : Ensure that the scrap directory exists, and generate the
                  name of file within it. If a name is specified then that is
                  used as the leaf name, otherwise a unique name is generated.
*/
string scrap_name(const char *name)
{
    // Ensure that the directories exists
    osfile_create_dir(SCRAP_DIR, 0);
    osfile_create_dir(SCRAP_SUBDIR, 0);

    // Generate a unique leaf name if necessary
    if (!name) name = scrap_unique();

    // Return the result
    return string(SCRAP_PATH) + name;
}
