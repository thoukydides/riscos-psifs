/*
    File        : open.h
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

// Only include header file once
#ifndef OPEN_H
#define OPEN_H

// Include cathlibcpp header files
#include "string.h"

/*
    Parameters  : disc  - The disc identifier.
    Returns     : void
    Description : Open a filer window for the root directory of the specified
                  disc.
*/
void open_open(string disc);

/*
    Parameters  : void
    Returns     : void
    Description : Close any filer windows for PsiFS discs.
*/
void open_close();

/*
    Parameters  : disc  - The disc identifier.
    Returns     : void
    Description : Close any filer window for the specified disc.
*/
void open_close(string disc);

#endif
