/*
    File        : printjbwin.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print job window handling for the PsiFS filer.

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
#ifndef PRINTJBWIN_H
#define PRINTJBWIN_H

// Include project header files
#include "printjbobj.h"

// Include oslib header files
#include "oslib/toolbox.h"

// Print job status window class
class printjbwin_win
{
public:

    /*
        Parameters  : job       - The print job to print.
                      top_left  - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
        Returns     : -
        Description : Constructor.
    */
    printjbwin_win(printjbobj_obj &job, const os_coord *top_left = NULL);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~printjbwin_win();

    /*
        Parameters  : printer   - Has the selected printer changed.
        Returns     : void
        Description : Update the status of this print job status window.
    */
    void update(bool printer = FALSE);

    /*
        Parameters  : file      - The name of the output file.
        Returns     : void
        Description : Print as many pages as possible.
    */
    void print(const string &file);

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
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_PrintError messages.
    */
    static int print_error(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_PrintFile messages.
    */
    static int print_file(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_PrintTypeOdd messages.
    */
    static int print_type_odd(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataSaveAck messages.
    */
    static int data_save_ack(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataLoadAck messages.
    */
    static int data_load_ack(wimp_message *message, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      wimp_block    - The wimp poll block.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle unacknowledged messages.
    */
    static bool acknowledge(wimp_event_no event_code, wimp_block *block,
                            toolbox_block *id_block, void *handle);

    /*
        Parameters  : id_block          - A toolbox ID block.
        Returns     : printjbwin_win    - Pointer to the corresponding print job
                                          status window, or NULL if not found.
        Description : Convert a toolbox ID block into a print job status window
                      pointer.
    */
    static printjbwin_win *find(const toolbox_block *id_block);

    /*
        Parameters  : printer   - Has the selected printer changed.
        Returns     : void
        Description : Update any open print job status windows.
    */
    static void update_all(bool printer = FALSE);

    /*
        Parameters  : void
        Returns     : void
        Description : Cancel any open print job status windows.
    */
    static void cancel_all();

    /*
        Parameters  : void
        Returns     : bits      - The number of open print job status windows.
        Description : Check how many print job status windows are open.
    */
    static bits active_count();

private:

    printjbobj_obj job;                 // The print job being controlled
    toolbox_o obj;                      // The window object
    bool driver;                        // Is a printer driver loaded
    int pages;                          // The number of pages printed
    int my_ref;                         // Reference number of last message
};

#endif
