/*
    File        : backop.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Backup operation handling for the PsiFS filer.

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
#ifndef BACKOP_H
#define BACKOP_H

// Include oslib header files
#include "oslib/os.h"

// Include cathlibcpp header files
#include "string.h"

// A class to handle backup operations
class backop_op
{
public:

    // The current status
    enum status_type
    {
        waiting,
        active,
        success,
        error,
        aborted
    };

private:

    string src;                         // Name of directory to backup
    string prev;                        // Name of previous backup file
    string dest;                        // Name of output backup file
    string scrap;                       // Name of output changes backup file
    string temp;                        // Name of temporary file
    os_coord pos;                       // Optional window position
    status_type status;                 // The current status

    /*
        Parameters  : void
        Returns     : void
        Description : Start the backup operation.
    */
    void start();

    /*
        Parameters  : void
        Returns     : void
        Description : Update the backup operation.
    */
    void update();

public:

    /*
        Parameters  : src       - The directory to backup.
                      prev      - The file containing the previous backup of
                                  this disc.
                      changes   - Should the changes from the previous backup
                                  be stored.
                      top_left  - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
        Returns     : -
        Description : Constructor.
    */
    backop_op(string src, string prev, bool changes,
              const os_coord *top_left = NULL);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~backop_op();

    /*
        Parameters  : void
        Returns     : status_type   - The current status.
        Description : Return the current status of this backup operation.
    */
    status_type get_status();

    /*
        Parameters  : void
        Returns     : string    - The name of the output backup file.
        Description : Return the name of the output backup file.
    */
    string get_dest();

    /*
        Parameters  : void
        Returns     : string    - The name of the output scrap file.
        Description : Return the name of the output scrap file.
    */
    string get_scrap();

    /*
        Parameters  : void
        Returns     : void
        Description : Abort the backup operation.
    */
    void abort();

    /*
        Parameters  : void
        Returns     : void
        Description : Abort any backup operations.
    */
    static void abort_all();

    /*
        Parameters  : void
        Returns     : void
        Description : Update any backup operations.
    */
    static void update_all();
};

#endif
