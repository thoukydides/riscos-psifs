/*
    File        : printctwin.c++
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

// Include header file for this module
#include "printctwin.h"

// Include oslib header files
#include "oslib/pdriver.h"
#include "oslib/window.h"
#include "event.h"

// Include cathlibcpp header files
#include "list.h"

// Include alexlib header files
#include "actionbutton_c.h"
#include "displayfield_c.h"
#include "numberrange_c.h"

// Include project header files
#include "filer.h"
#include "printjbwin.h"
#include "printprwin.h"

// Print job control window gagdets
#define PRINTCTWIN_CANCEL ((toolbox_c) 0x00)
#define PRINTCTWIN_PRINT ((toolbox_c) 0x01)
#define PRINTCTWIN_PREVIEW ((toolbox_c) 0x02)
#define PRINTCTWIN_STATUS ((toolbox_c) 0x10)
#define PRINTCTWIN_PRINTER ((toolbox_c) 0x11)
#define PRINTCTWIN_PAGES ((toolbox_c) 0x12)

// List of print job control windows
static list<printctwin_win *> printctwin_list;

/*
    Parameters  : job       - The print job to control.
                  top_left  - The coordinates for the top-left corner of
                              the window, or NULL for the default.
    Returns     : -
    Description : Constructor.
*/
printctwin_win::printctwin_win(printjbobj_obj &job, const os_coord *top_left)
: job(job)
{
    // Create the window
    obj = toolbox_create_object(0, (toolbox_id) "WinPrintCt");

    // Set the client handle
    toolbox_set_client_handle(0, obj, this);

    // Register toolbox handlers
    event_register_toolbox_handler(obj,
                                   action_ACTION_BUTTON_SELECTED,
                                   action, NULL);
    event_register_toolbox_handler(obj,
                                   action_WINDOW_DIALOGUE_COMPLETED,
                                   close, NULL);

    // Update the window before showing for the first time
    update(TRUE);

    // Open at centre of the screen unless a position specified
    toolbox_show_object(0, obj,
                        top_left
                        ? toolbox_POSITION_TOP_LEFT
                        : toolbox_POSITION_CENTRED,
                        (toolbox_position *) top_left,
                        toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

    // Add to the list of print job control windows
    printctwin_list.push_back((printctwin_win *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printctwin_win::~printctwin_win()
{
    // Remove from the list of print job control windows
    printctwin_list.remove((printctwin_win *)(this));

    // Deregister toolbox handlers
    event_deregister_toolbox_handler(obj,
                                     action_WINDOW_DIALOGUE_COMPLETED,
                                     close, this);
    event_deregister_toolbox_handler(obj,
                                     action_ACTION_BUTTON_SELECTED,
                                     action, this);

    // Destroy the window
    toolbox_delete_object(0, obj);
}

/*
    Parameters  : printer   - Has the selected printer changed.
    Returns     : void
    Description : Update the status of this print job control window.
*/
void printctwin_win::update(bool printer)
{
    // Check for the print job being cancelled
    if (job.cancelled())
    {
        // Close the window and discard the print job
        delete this;
    }
    else
    {
        // Set the printer name if changed
        if (printer)
        {
            char *desc;
            driver = !xpdriver_info(NULL, NULL, NULL, NULL, &desc,
                                    NULL, NULL, NULL)
                     && desc && *desc;
            displayfield_c(PRINTCTWIN_PRINTER, obj) = driver ? string(desc) : filer_msgtrans("PrnPdNo");
        }

        // Update the number of pages received
        numberrange_c(PRINTCTWIN_PAGES, obj) = job.pages();

        // Enable the print buttons if a printer driver is available
        actionbutton_c(PRINTCTWIN_PRINT, obj).set_faded(!driver);

        // Check if the print job is complete
        displayfield_c(PRINTCTWIN_STATUS, obj) = filer_msgtrans(job.complete() ? "PrnCtCp" : "PrnCtRx");
    }
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle an action button click.
*/
bool printctwin_win::action(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    printctwin_win *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Read the current pointer position
        wimp_pointer pointer;
        wimp_get_pointer_info(&pointer);

        // Perform the appropriate action
        switch (id_block->this_cmp)
        {
            case PRINTCTWIN_CANCEL:
                // Discard the print job
                delete ptr;
                break;

            case PRINTCTWIN_PRINT:
                // Open a print status window
                new printjbwin_win(ptr->job, &pointer.pos);
                break;

            case PRINTCTWIN_PREVIEW:
                // Open a print preview window
                new printprwin_win(ptr->job, &pointer.pos);
                break;

            default:
                // No interested in any other buttons
                break;
        }
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the window being closed.
*/
bool printctwin_win::close(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    printctwin_win *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Discard the print job
        if (ptr) delete ptr;
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : id_block          - A toolbox ID block.
    Returns     : printctwin_win    - Pointer to the corresponding print job
                                      control window, or NULL if not found.
    Description : Convert a toolbox ID block into a print job control window
                  pointer.
*/
printctwin_win *printctwin_win::find(const toolbox_block *id_block)
{
    // Choose the ancestor object ID
    toolbox_o id = id_block->ancestor_obj == toolbox_NULL_OBJECT
                   ? id_block->this_obj
                   : id_block->ancestor_obj;

    // Attempt to read the client handle
    printctwin_win *ptr;
    if (xtoolbox_get_client_handle(0, id, (void **) &ptr)) ptr = NULL;

    // Return the pointer
    return ptr;
}

/*
    Parameters  : printer   - Has the selected printer changed.
    Returns     : void
    Description : Update any open print job control windows.
*/
void printctwin_win::update_all(bool printer)
{
    list_iterator<printctwin_win *> i;

    // Update all of the print job control windows
    for (i = printctwin_list.begin(); i != printctwin_list.end(); i++)
    {
        (*i)->update(printer);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Cancel any open print job control windows.
*/
void printctwin_win::cancel_all()
{
    // Delete all windows
    while (!printctwin_list.empty()) delete printctwin_list.front();
}

/*
    Parameters  : void
    Returns     : bits      - The number of open print job control windows.
    Description : Check how many print job control windows are open.
*/
bits printctwin_win::active_count()
{
    return printctwin_list.size();
}
