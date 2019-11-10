/*
    File        : action.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Application level event handling for the PsiFS filer.

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
#include "action.h"

// Include oslib header files
#include "oslib/osfscontrol.h"
#include "oslib/saveas.h"

// Include cathlibcpp header files
#include "string.h"

// Include alexlib header files
#include "writablefield_c.h"

// Include project header files
#include "backobj.h"
#include "claim.h"
#include "config.h"
#include "filer.h"
#include "fs.h"
#include "icons.h"
#include "psifs.h"

// Name disk window gadgets
#define ACTION_NAME_NAME ((toolbox_c) 0)
#define ACTION_NAME_RENAME ((toolbox_c) 1)
#define ACTION_NAME_CANCEL ((toolbox_c) 2)

// Printer mirror destinations
#define ACTION_PRINTER_PARALLEL "printer#parallel:"
#define ACTION_PRINTER_SERIAL "printer#serial:"

// Path to the help text
#define ACTION_PATH_HELP "<PsiFS$Dir>.!Help"

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Perform a disconnect.
*/
bool action_disconnect(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Enable simple error handling
    filer_error_allowed++;

    // Perform a disconnect
    psifsmode_inactive(FALSE);

    // Restore normal error handling
    filer_error_allowed--;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the remote link.
*/
bool action_link(bits event_code, toolbox_action *action,
                 toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Enable the remote link
    claim_start_link();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the printer mirror with data sent to the default
                  destination.
*/
bool action_printer_default(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Enable the printer mirror
    claim_start_printer(config_current.get_str(config_tag_print));

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the printer mirror with data sent to the parallel port.
*/
bool action_printer_parallel(bits event_code, toolbox_action *action,
                             toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Enable the printer mirror
    claim_start_printer(ACTION_PRINTER_PARALLEL);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the printer mirror with data sent to the serial port.
*/
bool action_printer_serial(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Enable the printer mirror
    claim_start_printer(ACTION_PRINTER_SERIAL);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Enable the printer mirror with data sent to a specified file.
*/
bool action_printer_file(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    char *name = ((saveas_action_save_to_file *) &action->data)->file_name;

    // Enable the printer mirror
    claim_start_printer(name);

    // Inform the toolbox about the save
    saveas_file_save_completed(1, id_block->this_obj, name);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Open a filer window.
*/
bool action_filer(bits event_code, toolbox_action *action,
                  toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Open the filer window
    icons_drive::find(id_block)->open();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Prepare the name disk window.
*/
bool action_name_init(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Set the current name
    writablefield_c name(ACTION_NAME_NAME, id_block->this_obj);
    name = icons_drive::find(id_block)->name;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Rename a disk.
*/
bool action_name_action(bits event_code, toolbox_action *action,
                        toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Obtain a pointer to the drive object
    icons_drive *drive = icons_drive::find(id_block);

    // Get the required name
    writablefield_c field(ACTION_NAME_NAME, id_block->this_obj);
    string name(field());

    // Enable simple error handling
    filer_error_allowed++;

    // Generate an error if the name is too short
    if (name.length() < 2)
    {
        os_error err;

        // Generate the error
        err.errnum = 0;
        filer_msgtrans(err.errmess, sizeof(err.errmess), "ErrShrt");
        os_generate_error(&err);
    }

    // Close any old filer windows
    drive->close();

    // Rename the disk
    psifsfileop_name_disc(drive->drive, name.c_str());

    // Restore normal error handling
    filer_error_allowed--;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Open the backup window.
*/
bool action_backup(bits event_code, toolbox_action *action,
                   toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Open the backup control window
    backobj_obj::open(icons_drive::find(id_block)->drive);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Open the free space viewer.
*/
bool action_free(bits event_code, toolbox_action *action,
                 toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Obtain a pointer to the drive object
    icons_drive *drive = icons_drive::find(id_block);

    // Construct the command
    string cmd = string("%ShowFree -FS " FS_NAME " ") + drive->drive;

    // Enable simple error handling
    filer_error_allowed++;

    // Try to use the interactive free space viewer first
    if (xos_cli(cmd.c_str()))
    {
        // Perform a command line free space display
        cmd = string(FS_NAME ":Free ") + drive->drive;
        wimp_command_window(cmd.c_str());
        os_error *err = xos_cli(cmd.c_str());

        // Close the command window
        wimp_command_window((const char *) (err ? -1 : 0));
        if (err) os_generate_error(err);
    }

    // Restore normal error handling
    filer_error_allowed--;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Open the online help.
*/
bool action_help(bits event_code, toolbox_action *action,
                 toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Enable simple error handling
    filer_error_allowed++;

    // Open the help text
    os_cli(ACTION_PATH_HELP);

    // Restore normal error handling
    filer_error_allowed--;

    // Claim the event
    return TRUE;
}
