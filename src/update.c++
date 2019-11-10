/*
    File        : update.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Update the status for the PsiFS filer.

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
#include "update.h"

// Include project header files
#include "asyncwin.h"
#include "backobj.h"
#include "backop.h"
#include "clipwin.h"
#include "config.h"
#include "convwin.h"
#include "fs.h"
#include "icons.h"
#include "info.h"
#include "psifs.h"
#include "printjob.h"

// Current icons
static icons_idle *update_idle = NULL;
static icons_link *update_link = NULL;
static icons_drive *update_drive[psifs_DRIVE_LAST + 1];
static icons_drive *update_drive_all = NULL;
static icons_printer *update_printer = NULL;

/*
    Parameters  : void
    Returns     : bool  - Are there any drive icons.
    Description : Check whether the drive status has changed, and take
                  appropriate action.
*/
static bool update_drives()
{
    bool any = FALSE;
    psifs_drive drive;
    icons_drive *prev = NULL;

    // Loop through all of the possible drives
    for (drive = psifs_DRIVE_FIRST; drive <= psifs_DRIVE_LAST; drive++)
    {
        psifs_drive_status status = psifsget_drive_status(drive);

        // Handle this drive icon
        if ((status & psifs_DRIVE_STATUS_PRESENT)
            && (config_current.get_bool(config_tag_show_rom)
                || !(status & psifs_DRIVE_STATUS_ROM))
            && !config_current.get_bool(config_tag_show_all))
        {
            // Create a new icon if necessary
            if (!update_drive[drive])
            {
                // Create a new icon
                update_drive[drive] = new icons_drive(drive + psifs_DRIVE_FIRST_UPPER - psifs_DRIVE_FIRST, prev);

                // Open a filer window for this drive if required
                if (config_current.get_bool(config_tag_open_link))
                {
                    // Open a filer window for this drive
                    update_drive[drive]->open();
                }
            }
            else
            {
                // Update the text under the icon
                update_drive[drive]->update();
            }

            // Set the flag to indicate that there is a drive icon
            any = TRUE;

            // Set the previous icon pointer
            prev = update_drive[drive];
        }
        else if (update_drive[drive])
        {
            // Close any filer windows for this drive if required
            if (config_current.get_bool(config_tag_close_link))
            {
                // Close any filer windows for this drive
                update_drive[drive]->close();
            }

            // Delete the icon
            delete update_drive[drive];
            update_drive[drive] = NULL;
        }
    }

    // Update the virtual drive icon
    if ((psifsget_link_status()
         & (psifs_LINK_STATUS_EPOC16 | psifs_LINK_STATUS_EPOC32))
        && config_current.get_bool(config_tag_show_all))
    {
        // Create a new icon if necessary
        if (!update_drive_all)
        {
            // Create a new icon
            update_drive_all = new icons_drive(FS_CHAR_DRIVE_ALL);

            // Open a filer window for this drive if required
            if (config_current.get_bool(config_tag_open_link))
            {
                // Open a filer window for this drive
                update_drive_all->open();
            }
        }

        // Set the flag to indicate that there is a drive icon
        any = TRUE;
    }
    else if (update_drive_all)
    {
        // Close any filer windows for this drive if required
        if (config_current.get_bool(config_tag_close_link))
        {
            // Close any filer windows for this drive
            update_drive_all->close();
        }

        // Delete the icon
        delete update_drive_all;
        update_drive_all = NULL;
    }

    // Return whether there are any drive icons
    return any;
}

/*
    Parameters  : timed - Was the check triggered by a timer tick rather than
                          a poll word change.
    Returns     : void
    Description : Check whether the status has changed, and take appropriate
                  action.
*/
void update_check(bool timed)
{
    psifs_mode mode = psifsget_mode();

    // Handle the idle icon
    if (mode == psifs_MODE_INACTIVE)
    {
        // Create a new icon if necessary
        if (!update_idle) update_idle = new icons_idle;
    }
    else if (update_idle)
    {
        // Delete the icon
        delete update_idle;
        update_idle = NULL;
    }

    // Handle the remote link and drive icons
    if (!update_drives() && (mode == psifs_MODE_LINK))
    {
        // Create a new icon if necessary
        if (!update_link) update_link = new icons_link;
        else update_link->update();
    }
    else if (update_link)
    {
        // Delete the icon
        delete update_link;
        update_link = NULL;
    }

    // Handle the printer mirror icon
    if (mode == psifs_MODE_PRINTER)
    {
        // Create a new icon or update the existing one
        if (!update_printer) update_printer = new icons_printer;
        else update_printer->update();
    }
    else if (update_printer)
    {
        // Delete the icon
        delete update_printer;
        update_printer = NULL;
    }

    // Update the asynchronous operation windows if necessary
    asyncwin_win::update_all();

    // Update the backup objects if necessary
    backobj_obj::update_all();

    // Update the backup operations if necessary
    backop_op::update_all();

    // Update the information windows if necessary
    info_update(timed);

    // Update the file converters if necessary
    convwin_update();

    // Update the clipboard if necessary
    clipwin_update();

    // Update the print jobs if necessary
    printjob_update();
}
