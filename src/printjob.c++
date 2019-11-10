/*
    File        : printjob.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print job handling for the PsiFS filer.

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
#include "printjob.h"

// Include clib header files
#include <ctype.h>
#include <stdio.h>

// Include oslib header files
#include "oslib/pdriver.h"

// Include project header files
#include "config.h"
#include "psifs.h"
#include "printjbobj.h"
#include "printctwin.h"
#include "printprwin.h"
#include "printjbwin.h"

// Control of position at which to automatically open
#define PRINTJOB_OFFSET_X (64)
#define PRINTJOB_OFFSET_Y (48)
#define PRINTJOB_MIN_X (96)
#define PRINTJOB_MAX_X (PRINTJOB_MIN_X + PRINTJOB_OFFSET_X * 10)
#define PRINTJOB_MAX_X_OFFSET (656)
#define PRINTJOB_MIN_Y (476)
#define PRINTJOB_MAX_Y (PRINTJOB_MIN_Y + PRINTJOB_OFFSET_Y * 22)
#define PRINTJOB_MAX_Y_OFFSET (96)

/*
    Parameters  : void
    Returns     : int   - The screen width in OS units.
    Description : Returns the width of the current screen mode.
*/
static int printjob_scr_width()
{
    int xeig;
    int xwind;

    // Read the mode details
    os_read_mode_variable(os_CURRENT_MODE,
                          os_MODEVAR_XEIG_FACTOR, &xeig);
    os_read_mode_variable(os_CURRENT_MODE,
                          os_MODEVAR_XWIND_LIMIT, &xwind);

    // Return the result
    return (xwind + 1) << xeig;
}

/*
    Parameters  : void
    Returns     : int   - The screen height in OS units.
    Description : Returns the height of the current screen mode.
*/
static int printjob_scr_height()
{
    int yeig;
    int ywind;

    // Read the mode details
    os_read_mode_variable(os_CURRENT_MODE,
                          os_MODEVAR_YEIG_FACTOR, &yeig);
    os_read_mode_variable(os_CURRENT_MODE,
                          os_MODEVAR_YWIND_LIMIT, &ywind);

    // Return the result
    return (ywind + 1) << yeig;
}

/*
    Parameters  : void
    Returns     : os_coord  - Pointer to the coordinates for the top-left
                              corner of the next window.
    Description : Generate a position for the next window to be opened. The
                  result will be overwritten on the next call to this function,
                  so must be copied if required.
*/
static const os_coord *printjob_next_pos()
{
    static os_coord pos = {PRINTJOB_MAX_X, PRINTJOB_MIN_Y};
    int max_x;
    int max_y;

    // Update the position
    pos.x += PRINTJOB_OFFSET_X;
    pos.y -= PRINTJOB_OFFSET_Y;

    // Calculate the bounds
    max_x = printjob_scr_width() - PRINTJOB_MAX_X_OFFSET;
    if (PRINTJOB_MAX_X < max_x) max_x = PRINTJOB_MAX_X;
    max_y = printjob_scr_height() - PRINTJOB_MAX_Y_OFFSET;
    if (PRINTJOB_MAX_Y < max_y) max_y = PRINTJOB_MAX_Y;

    // Massage the position if unsuitable
    if (max_x < pos.x) pos.x = PRINTJOB_MIN_X;
    if (pos.y < PRINTJOB_MIN_Y) pos.y = max_y;

    // Return the result
    return &pos;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the print job status.
*/
void printjob_update(void)
{
    // Check for a new print job
    psifs_print_job_handle handle;
    if (psifs_print_job_poll(psifs_PRINT_JOB_INVALID, &handle, NULL, NULL)
        == psifs_PRINT_JOB_START)
    {
        // Create a new print job
        printjbobj_obj job(handle);

        // Choose the location for the new window
        const os_coord *top_left = printjob_next_pos();

        // Open the appropriate window type
        char *printer;
        if (config_current.get_bool(config_tag_print_auto_print)
            && !xpdriver_info(NULL, NULL, NULL, NULL, &printer,
                              NULL, NULL, NULL)
            && printer && *printer)
        {
            new printjbwin_win(job, top_left);
        }
        else if (config_current.get_bool(config_tag_print_auto_preview))
        {
            new printprwin_win(job, top_left);
        }
        else new printctwin_win(job, top_left);
    }

    // Update the print jobs
    printjbobj_obj::update_all();

    // Update the windows
    printctwin_win::update_all();
    printprwin_win::update_all();
    printjbwin_win::update_all();
}

/*
    Parameters  : void
    Returns     : void
    Description : Cancel all print jobs.
*/
void printjob_cancel(void)
{
    // Cancel all the print job windows
    printctwin_win::cancel_all();
    printprwin_win::cancel_all();
    printjbwin_win::cancel_all();
}

/*
    Parameters  : void
    Returns     : bits      - The number of open print job windows.
    Description : Check how many print jobs are active.
*/
bits printjob_active(void)
{
    // Return the total number of windows
    return printctwin_win::active_count()
           + printprwin_win::active_count()
           + printjbwin_win::active_count();
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_SetPrinter wimp message events.
*/
int printjob_set_printer(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Update the windows
    printctwin_win::update_all(TRUE);
    printprwin_win::update_all(TRUE);
    printjbwin_win::update_all(TRUE);

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_PsifsPrint wimp message events.
*/
int printjob_print(wimp_message *message, void *handle)
{
    psifs_message_print *data = (psifs_message_print *) &message->data;

    NOT_USED(handle);

    // Create a new print job
    printpgobj_obj page(data->file);
    printjbobj_obj job(page);

    // Choose the location for the new window
    const os_coord *top_left = (data->top_left.x != -1)
                               || (data->top_left.y != -1)
                               ? &data->top_left : printjob_next_pos();

    // Open the appropriate window type
    switch (data->op)
    {
        case psifs_PRINT_PRINT:
            new printjbwin_win(job, top_left);
            break;
        case psifs_PRINT_PREVIEW:
            new printprwin_win(job, top_left);
            break;
        default:
            new printctwin_win(job, top_left);
            break;
    }

    // Update all print jobs
    printjob_update();

    // Acknowledge the message
    message->your_ref = message->my_ref;
    wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, message, message->sender);

    // Always claim the event
    return TRUE;
}
