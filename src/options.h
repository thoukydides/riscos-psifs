/*
    File        : options.h
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

// Only include header file once
#ifndef OPTIONS_H
#define OPTIONS_H

// Include project header files
#include "tag.h"

// An options store
class options_store : public tag_store
{
public:

    /*
        Parameters  : void
        Returns     : bool      - Was the load successful.
        Description : Attempt to read the options file.
    */
    bool load();

    /*
        Parameters  : void
        Returns     : void
        Description : Attempt to write the options file.
    */
    void save() const;
};

// The current options
extern options_store options_current;

#endif
