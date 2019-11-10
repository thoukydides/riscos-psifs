/*
    File        : backop.c++
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

// Include header file for this module
#include "backop.h"

// Include oslib header files
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"

// Include cathlibcpp header files
#include "list.h"

// Include project header files
#include "asyncwin.h"
#include "filer.h"
#include "psifs.h"
#include "scrap.h"

// Current asynchronous operation
static asyncwin_win *backop_win = NULL;

// Closed files
static string backop_closed;

// List of active or pending backup operations
static list<backop_op *> backop_list;

/*
    Parameters  : void
    Returns     : void
    Description : Start an operation to close all open files.
*/
static void backop_close()
{
    psifs_async_handle handle;

    // Generate a unique name for the output file
    backop_closed = scrap_name();

    // Start the shutdown operation
    handle = psifsasyncstart_shutdown(NULL, backop_closed.c_str(), FALSE);

    // Create an asynchronous window to control the operation
    backop_win = new asyncwin_win(handle, filer_msgtrans("BkOpTCl").c_str());
    backop_win->set_close(asyncwin_win::no_close);
    backop_win->set_delete(asyncwin_win::no_delete);
}

/*
    Parameters  : void
    Returns     : void
    Description : Start an operation to re-open all open files.
*/
static void backop_open()
{
    psifs_async_handle handle;

    // Start the restart operation
    handle = psifsasyncstart_restart(backop_closed.c_str(), TRUE);

    // Create an asynchronous window to control the operation
    backop_win = new asyncwin_win(handle, filer_msgtrans("BkOpTOp").c_str());
    backop_win->set_close(asyncwin_win::no_close);
    backop_win->set_delete(asyncwin_win::no_delete);

    // Clear the input file name
    backop_closed = "";
}

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
backop_op::backop_op(string src, string prev, bool changes,
                     const os_coord *top_left)
: src(src), prev(prev), status(waiting)
{
    // Generate unique file names
    dest = scrap_name();
    if (changes) scrap = scrap_name();
    temp = scrap_name();

    // Store the requested position
    pos.x = top_left ? top_left->x : 0;
    pos.y = top_left ? top_left->y : 0;

    // Add to list of pending backup operations
    backop_list.push_back((backop_op *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
backop_op::~backop_op()
{
    // Abort the operation
    abort();

    // Delete any output files
    xosfscontrol_wipe(dest.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
    xosfscontrol_wipe(scrap.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
    xosfscontrol_wipe(temp.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
}

/*
    Parameters  : void
    Returns     : status_type   - The current status.
    Description : Return the current status of this backup operation.
*/
backop_op::status_type backop_op::get_status()
{
    // Return the current status
    return status;
}

/*
    Parameters  : void
    Returns     : string    - The name of the output backup file.
    Description : Return the name of the output backup file.
*/
string backop_op::get_dest()
{
    os_fw file;

    // Check if the destination file exists and can be opened
    if (!xosfind_openinw(osfind_NO_PATH | osfind_ERROR_IF_ABSENT | osfind_ERROR_IF_DIR, dest.c_str(), NULL, &file) && file)
    {
        xosfind_closew(file);
    }
    else file = 0;

    // Return the name of the destination file
    return file ? dest : string();
}

/*
    Parameters  : void
    Returns     : string    - The name of the output scrap file.
    Description : Return the name of the output scrap file.
*/
string backop_op::get_scrap()
{
    os_fw file;

    // Check if the scrap file exists and can be opened
    if (!xosfind_openinw(osfind_NO_PATH | osfind_ERROR_IF_ABSENT | osfind_ERROR_IF_DIR, scrap.c_str(), NULL, &file) && file)
    {
        xosfind_closew(file);
    }
    else file = 0;

    // Return the name of the scrap file
    return file ? scrap : string();
}

/*
    Parameters  : void
    Returns     : void
    Description : Start the backup operation.
*/
void backop_op::start()
{
    psifs_async_handle handle;

    // Start the backup operation
    handle = psifsasyncstart_backup(src.c_str(), dest.c_str(),
                                    prev.empty() ? NULL : prev.c_str(),
                                    scrap.empty() ? NULL : scrap.c_str(),
                                    temp.c_str());

    // Create an asynchronous window to control the operation
    backop_win = new asyncwin_win(handle, filer_msgtrans("BkOpTBk", src.c_str()).c_str(), pos.x || pos.y ? &pos : NULL);
    backop_win->set_close(asyncwin_win::no_close);
    backop_win->set_delete(asyncwin_win::no_delete);

    // Mark the operation as active
    status = active;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the backup operation.
*/
void backop_op::update()
{
    // Action depends on the status
    switch (backop_win->get_status())
    {
        case asyncwin_win::success:
            // Backup completed successfully
            delete backop_win;
            backop_win = NULL;
            status = success;
            break;

        case asyncwin_win::error:
            // Backup failed with an error
            backop_win->set_close(asyncwin_win::allow_close);
            backop_win->set_delete(asyncwin_win::auto_delete);
            backop_win = NULL;
            status = error;
            break;

        case asyncwin_win::aborted:
            // Backup aborted
            delete backop_win;
            backop_win = NULL;
            status = aborted;
            break;

        default:
            // Otherwise assume backup active
            break;
    }

    // Special actions if finished
    if (status != active)
    {
        // Remove from list of pending backup operations
        backop_list.remove((backop_op *)(this));
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Abort the backup operation.
*/
void backop_op::abort()
{
    // No action unless active
    if ((status == waiting) || (status == active))
    {
        // Remove from list of pending backup operations
        backop_list.remove((backop_op *)(this));

        // Abort any active operation
        if (status == active)
        {
            delete backop_win;
            backop_win = NULL;
        }

        // Set the status
        status = aborted;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Abort any backup operations.
*/
void backop_op::abort_all()
{
    // Abort all backup operations
    while (!backop_list.empty()) backop_list.front()->abort();
}

/*
    Parameters  : void
    Returns     : void
    Description : Update any backup operations.
*/
void backop_op::update_all()
{
    // Process any active operation
    if (backop_win)
    {
        // Action depends on operation
        if (!backop_list.empty() && (backop_list.front()->status == active))
        {
            // Backup operation in progress
            backop_list.front()->update();
        }
        else
        {
            // Opening or closing files
            switch (backop_win->get_status())
            {
                case asyncwin_win::success:
                    // Operation completed successfully
                    delete backop_win;
                    backop_win = NULL;
                    break;

                case asyncwin_win::error:
                    // Operation failed with an error
                    backop_win->set_close(asyncwin_win::allow_close);
                    backop_win->set_delete(asyncwin_win::auto_delete);
                    backop_win = NULL;
                    backop_closed = "";
                    abort_all();
                    break;

                case asyncwin_win::aborted:
                    // Operation aborted
                    delete backop_win;
                    backop_win = NULL;
                    backop_closed = "";
                    abort_all();
                    break;

                default:
                    // Otherwise assume operation active
                    break;
            }
        }
    }

    // Start a new operation if appropriate
    if (!backop_win)
    {
        // Are any backup operations pending
        if (backop_list.empty())
        {
            // No backup operations pending
            if (!backop_closed.empty()) backop_open();
        }
        else
        {
            // Backup operations are pending
            if (backop_closed.empty()) backop_close();
            else backop_list.front()->start();
        }
    }
}
