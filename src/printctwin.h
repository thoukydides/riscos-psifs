/*
    File        : printctwin.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print job control window handling for the PsiFS filer.

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
#ifndef PRINTCTWIN_H
#define PRINTCTWIN_H

// Include project header files
#include "printjbobj.h"

// Include oslib header files
#include "oslib/toolbox.h"

// Print job control window class
class printctwin_win
{
public:

    /*
        Parameters  : job       - The print job to control.
                      top_left  - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
        Returns     : -
        Description : Constructor.
    */
    printctwin_win(printjbobj_obj &job, const os_coord *top_left = NULL);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printctwin_win();

    /*
        Parameters  : printer   - Has the selected printer changed.
        Returns     : void
        Description : Update the status of this print job control window.
    */
    void update(bool printer = FALSE);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle an action button click.
    */
    static bool action(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the window being closed.
    */
    static bool close(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle);

    /*
        Parameters  : id_block          - A toolbox ID block.
        Returns     : printctwin_win    - Pointer to the corresponding print job
                                          control window, or NULL if not found.
        Description : Convert a toolbox ID block into a print job control window
                      pointer.
    */
    static printctwin_win *find(const toolbox_block *id_block);

    /*
        Parameters  : printer   - Has the selected printer changed.
        Returns     : void
        Description : Update any open print job control windows.
    */
    static void update_all(bool printer = FALSE);

    /*
        Parameters  : void
        Returns     : void
        Description : Cancel any open print job control windows.
    */
    static void cancel_all();

    /*
        Parameters  : void
        Returns     : bits      - The number of open print job control windows.
        Description : Check how many print job control windows are open.
    */
    static bits active_count();

private:

    printjbobj_obj job;                 // The print job being controlled
    toolbox_o obj;                      // The window object
    bool driver;                        // Is a printer driver loaded
};

#endif
