/*
    File        : scrap.h
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

// Only include header file once
#ifndef SCRAP_H
#define SCRAP_H

// Include cathlibcpp header files
#include "string.h"

/*
    Parameters  : name      - Optional leaf name for the file.
    Returns     : string    - The file name.
    Description : Ensure that the scrap directory exists, and generate the
                  name of file within it. If a name is specified then that is
                  used as the leaf name, otherwise a unique name is generated.
*/
string scrap_name(const char *name = NULL);

#endif
