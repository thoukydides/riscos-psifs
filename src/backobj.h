/*
    File        : backobj.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Backup root object handling for the PsiFS filer.

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
#ifndef BACKOBJ_H
#define BACKOBJ_H

// Include oslib header files
#include "oslib/toolbox.h"

// Include cathlibcpp header files
#include "string.h"

// Include project header files
#include "backcfg.h"
#include "backop.h"
#include "fs.h"
#include "psifs.h"

// A backup object
class backobj_obj
{
    backcfg_cfg config;                 // Backup configuration
    string status;                      // Most recent status message
    backop_op *op;                      // The active backup operation
    toolbox_o obj;                      // Toolbox object ID for control window
    bool connected;                     // The connection status of this disc
    bool countdown;                     // Start a countdown to start backup
    os_t countdown_end;                 // End of countdown to start backup
    os_coord top_left;                  // Default position of control window

    /*
        Parameters  : void
        Returns     : void
        Description : Update the backup object.
    */
    void update();

    /*
        Parameters  : void
        Returns     : void
        Description : Start a backup operation for the disc associated with
                      this backup object.
    */
    void start();

    /*
        Parameters  : void
        Returns     : void
        Description : Start a backup operation for the disc associated with
                      this backup object.
    */
    void start_countdown();

    /*
        Parameters  : void
        Returns     : void
        Description : A connection has been established to the disc associated
                      with this backup object.
    */
    void connect();

    /*
        Parameters  : void
        Returns     : void
        Description : The connection has been lost to the disc associated with
                      this backup object.
    */
    void disconnect();

    /*
        Parameters  : top_left  - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
        Returns     : void
        Description : Open the control window for this backup object.
    */
    void open(const os_coord *top_left = NULL);

    /*
        Parameters  : void
        Returns     : os_coord  - The coordinates for the top-left corner.
        Description : Return the position of this window if open, or the
                      suggested position otherwise.
    */
    os_coord get_top_left();

public:

    /*
        Parameters  : disc  - The details of the disc.
        Returns     : -
        Description : Constructor.
    */
    backobj_obj(const backcfg_disc &disc);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~backobj_obj();

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the backup action button.
    */
    static bool start(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the options action button.
    */
    static bool options(bits event_code, toolbox_action *action,
                        toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the abort action button.
    */
    static bool abort(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the open action button.
    */
    static bool filer(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle);

    /*
        Parameters  : id_block      - A toolbox ID block.
        Returns     : icons_drive   - Pointer to the corresponding backup
                                      object, or NULL if not found.
        Description : Convert a toolbox ID block into a backup object pointer.
    */
    static backobj_obj *find(const toolbox_block *id_block);

    /*
        Parameters  : drive         - The drive letter, or FS_CHAR_DRIVE_ALL
                                      for all drives.
                      top_left      - The coordinates for the top-left corner of
                                      the window, or NULL for the default.
        Returns     : void
        Description : Open the control window for the matching backup object.
    */
    static void open(psifs_drive drive, const os_coord *top_left = NULL);

    /*
        Parameters  : name          - The disc name.
        Returns     : void
        Description : Open the control window for all matching backup objects.
    */
    static void open(string name);

    /*
        Parameters  : void
        Returns     : void
        Description : Update the status of all backup objects.
    */
    static void update_all();
};

#endif
