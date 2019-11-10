/*
    File        : printprwin.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print preview window handling for the PsiFS filer.

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
#ifndef PRINTPRWIN_H
#define PRINTPRWIN_H

// Include oslib header files
#include "oslib/toolbox.h"

// Include project header files
#include "drawobj.h"
#include "printjbobj.h"

// Print preview window class
class printprwin_win
{
public:

    /*
        Parameters  : job       - The print job to preview.
                      top_left  - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
        Returns     : -
        Description : Constructor.
    */
    printprwin_win(printjbobj_obj &job, const os_coord *top_left = NULL);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printprwin_win();

    /*
        Parameters  : void
        Returns     : void
        Description : Redraw the print job preview window following a change
                      to the scale, page or printer.
    */
    void refresh();

    /*
        Parameters  : draw          - Details of the rectangle to redraw.
        Returns     : void
        Description : Redraw the specified rectangle of the print job preview
                      window.
    */
    void redraw(wimp_draw &draw);

    /*
        Parameters  : printer       - Has the selected printer changed.
        Returns     : void
        Description : Update the status of this print job preview window.
    */
    void update(bool printer = FALSE);

    /*
        Parameters  : name          - The name of the file to save.
        Returns     : bool          - Was the file saved successfully.
        Description : Save the current page as a draw file.
    */
    bool save_draw(const string &name);

    /*
        Parameters  : name          - The name of the file to save.
                      all           - Should the whole job be saved.
        Returns     : bool          - Was the file saved successfully.
        Description : Save either the current page of the whole print job
                      as a text file.
    */
    bool save_text(const string &name, bool all = FALSE);

    /*
        Parameters  : name          - The name of the file to save.
        Returns     : bool          - Was the file saved successfully.
        Description : Save the current page as a raw file.
    */
    bool save_raw(const string &name);

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
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the page number changing.
    */
    static bool set_page(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the scale window being opened.
    */
    static bool pre_scale(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the scale being changed.
    */
    static bool set_scale(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle save window being opened.
    */
    static bool pre_save(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle save window saves.
    */
    static bool save_raw(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle menu selections.
    */
    static bool menu_raw(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      wimp_block    - The wimp poll block.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle window redraw requests.
    */
    static bool redraw_raw(wimp_event_no event_code, wimp_block *block,
                           toolbox_block *id_block, void *handle);

    /*
        Parameters  : id_block          - A toolbox ID block.
        Returns     : printprwin_win    - Pointer to the corresponding print
                                          job preview window, or NULL if not
                                          found.
        Description : Convert a toolbox ID block into a print job preview
                      window pointer.
    */
    static printprwin_win *find(const toolbox_block *id_block);

    /*
        Parameters  : printer   - Has the selected printer changed.
        Returns     : void
        Description : Update any open print job preview windows.
    */
    static void update_all(bool printer = FALSE);

    /*
        Parameters  : void
        Returns     : void
        Description : Cancel any open print job preview windows.
    */
    static void cancel_all();

    /*
        Parameters  : void
        Returns     : bits      - The number of open print job preview windows.
        Description : Check how many print job preview windows are open.
    */
    static bits active_count();

private:

    printjbobj_obj job;                 // The print job being controlled
    toolbox_o obj;                      // The window object
    toolbox_o tool;                     // The tool window object
    toolbox_o menu;                     // The main menu
    toolbox_o menu_job;                 // The print job menu
    toolbox_o menu_page;                // The page menu
    bool driver;                        // Is a printer driver loaded
    bits page;                          // Current page being displayed
    int scale;                          // Current display scale
    drawobj_file printable;             // Printable area as a draw file
    drawobj_file drawfile;              // Current page as a draw file
    bool drawfile_valid;                // Has this page been rendered
};

#endif
