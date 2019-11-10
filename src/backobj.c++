/*
    File        : backobj.c++
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

// Include header file for this module
#include "backobj.h"

// Include oslib header files
#include "oslib/macros.h"
#include "oslib/territory.h"
#include "oslib/window.h"

// Include cathlibcpp header files
#include "list.h"
#include "sstream.h"

// Include alexlib header files
#include "actionbutton_c.h"
#include "displayfield_c.h"

// Include project header files
#include "config.h"
#include "filer.h"
#include "fs.h"

// Backup control window gadgets
#define BACKOBJ_CLOSE ((toolbox_c) 0x00)
#define BACKOBJ_OPTIONS ((toolbox_c) 0x01)
#define BACKOBJ_BACKUP ((toolbox_c) 0x02)
#define BACKOBJ_FILER ((toolbox_c) 0x03)
#define BACKOBJ_ABORT ((toolbox_c) 0x04)
#define BACKOBJ_DRIVE ((toolbox_c) 0x10)
#define BACKOBJ_DISC ((toolbox_c) 0x11)
#define BACKOBJ_LAST ((toolbox_c) 0x12)
#define BACKOBJ_STATUS ((toolbox_c) 0x13)

// Length of countdown
#define BACKOBJ_COUNTDOWN (500)

// Window offset
#define BACKOBJ_OFFSET_X (600)
#define BACKOBJ_OFFSET_Y (48)
#define BACKOBJ_MIN_X (48)
#define BACKOBJ_MAX_X (2048)
#define BACKOBJ_MAX_X_OFFSET (640)
#define BACKOBJ_MAX_Y (868)
#define BACKOBJ_MIN_Y (676)
#define BACKOBJ_MIN_Y_OFFSET (96)
#define BACKOBJ_OFFSET_CONFIG_X (0)
#define BACKOBJ_OFFSET_CONFIG_Y (798)
#define BACKOBJ_OFFSET_OP_X (-14)
#define BACKOBJ_OFFSET_OP_Y (-326)

// A list of backup objects
static list<backobj_obj *> backobj_list;

/*
    Parameters  : void
    Returns     : int   - The screen width in OS units.
    Description : Returns the width of the current screen mode.
*/
static int backobj_scr_width()
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
static int backobj_scr_height()
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
static const os_coord *backobj_next_pos()
{
    static os_coord pos;
    int max_x;
    int max_y;

    // Reset position if no other objects
    if (backobj_list.empty())
    {
        pos.x = BACKOBJ_MAX_X;
        pos.y = BACKOBJ_MIN_Y;
    }

    // Calculate the bounds
    max_x = backobj_scr_width() - BACKOBJ_MAX_X_OFFSET;
    if (BACKOBJ_MAX_X < max_x) max_x = BACKOBJ_MAX_X;
    max_y = backobj_scr_width() - BACKOBJ_MIN_Y_OFFSET;
    if (BACKOBJ_MAX_Y < max_y) max_y = BACKOBJ_MAX_Y;

    // Update the horizontal position
    pos.x += BACKOBJ_OFFSET_X;
    if (max_x < pos.x)
    {
        // Massage the horizontal position
        pos.x = BACKOBJ_MIN_X;

        // Update the vertical position
        pos.y -= BACKOBJ_OFFSET_Y;
        if (pos.y < BACKOBJ_MIN_Y) pos.y = max_y;
    }

    // Return the result
    return &pos;
}

/*
    Parameters  : disc  - The details of the disc.
    Returns     : -
    Description : Constructor.
*/
backobj_obj::backobj_obj(const backcfg_disc &disc)
: config(disc), op(NULL), connected(FALSE), countdown(FALSE)
{
    // Create the control window
    obj = toolbox_create_object(0, (toolbox_id) "WinBackCtrl");

    // Set the client handle
    toolbox_set_client_handle(0, obj, this);

    // Choose the default control window position
    top_left = *backobj_next_pos();

    // Automatically open the configuration window if not recognised
    if (!config.get_valid() && config_current.get_bool(config_tag_backup_new))
    {
        os_coord top_left = get_top_left();
        top_left.x += BACKOBJ_OFFSET_CONFIG_X;
        top_left.y += BACKOBJ_OFFSET_CONFIG_Y;
        config.open();
    }

    // Mark the disc as connected
    connect();

    // Update the status immediately
    update();

    // Add to the list of backup objects
    backobj_list.push_back((backobj_obj *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
backobj_obj::~backobj_obj()
{
    // Remove from the list of backup objects
    backobj_list.remove((backobj_obj *)(this));

    // Delete any backup operation
    if (op) delete op;

    // Delete the control window
    toolbox_delete_object(0, obj);
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the backup object.
*/
void backobj_obj::update()
{
    // Check for a completed backup operation
    if (op)
    {
        bool finished = FALSE;
        bool partial = FALSE;

        // Action depends on the operation
        switch (op->get_status())
        {
            case backop_op::waiting:
                // Waiting to start backup
                status = filer_msgtrans("BkStWat");
                break;

            case backop_op::active:
                // Backup operation active
                status = filer_msgtrans("BkStPrg");
                break;

            case backop_op::success:
                // Operation completed successfully
                status = filer_msgtrans("BkStSuc");
                finished = TRUE;
                break;

            case backop_op::error:
                // Error occurred
                status = filer_msgtrans("BkStErr");
                finished = TRUE;
                partial = TRUE;
                break;

            case backop_op::aborted:
                // Backup operation aborted
                status = filer_msgtrans("BkStAbt");
                finished = TRUE;
                partial = TRUE;
                break;
        }

        // End the operation if appropriate
        if (finished)
        {
            // Process any output files
            string dest(op->get_dest());
            if (!dest.empty())
            {
                config.store(dest, op->get_scrap(), partial);
            }

            // Delete the operation
            delete op;
            op = NULL;
        }
    }

    // Update the disc and drive name gadgets
    displayfield_c(BACKOBJ_DRIVE, obj) = string("") + config.get_drive();
    displayfield_c(BACKOBJ_DISC, obj) = config->get_str(backcfg_name);

    // Update the time of last backup
    if (config.get_valid() && config->exist(backcfg_last_time))
    {
        char str[100];

        // Display time of previous backup
        date_riscos last = config->get_date(backcfg_last_time);
        territory_convert_standard_date_and_time(territory_CURRENT, (os_date_and_time *) last.bytes, str, sizeof(str));
        displayfield_c(BACKOBJ_LAST, obj) = filer_msgtrans(config->get_bool(backcfg_last_partial) ? "BkLsPrt" : "BkLsFul", str);
    }
    else
    {
        // No previous backup
        displayfield_c(BACKOBJ_LAST, obj) = filer_msgtrans("BkLsNvr");
    }

    // Update the status field
    if (countdown)
    {
        ostringstream time;
        time << (countdown_end - os_read_monotonic_time()) / 100;
        displayfield_c(BACKOBJ_STATUS, obj) = filer_msgtrans("BkStCnt", time.str().c_str());
    }
    else if (!config.get_valid())
    {
        displayfield_c(BACKOBJ_STATUS, obj) = filer_msgtrans("BkStOpt");
    }
    else if (!connected)
    {
        displayfield_c(BACKOBJ_STATUS, obj) = filer_msgtrans("BkStDsc");
    }
    else if (!status.empty())
    {
        displayfield_c(BACKOBJ_STATUS, obj) = status;
    }
    else
    {
        displayfield_c(BACKOBJ_STATUS, obj) = filer_msgtrans("BkStIdl");
    }

    // Disable the action buttons as appropriate
    actionbutton_c(BACKOBJ_BACKUP, obj).set_faded(!config.get_valid() || op);
    actionbutton_c(BACKOBJ_OPTIONS, obj).set_faded(op != NULL);
    actionbutton_c(BACKOBJ_ABORT, obj).set_faded(!countdown && !op);
    actionbutton_c(BACKOBJ_FILER, obj).set_faded(!config.get_valid());

    // Start a backup if countdown expired
    if (countdown && ((countdown_end - os_read_monotonic_time()) < 0)) start();
}

/*
    Parameters  : void
    Returns     : void
    Description : Start a backup operation for the disc associated with
                  this backup object.
*/
void backobj_obj::start()
{
    // No action if operation already started
    if (!op)
    {
        // Start a backup operation
        countdown = FALSE;
        os_coord top_left = get_top_left();
        top_left.x += BACKOBJ_OFFSET_OP_X;
        top_left.y += BACKOBJ_OFFSET_OP_Y;
        op = new backop_op(string() + FS_CHAR_DISC + config.get_drive() + FS_CHAR_SEPARATOR + FS_CHAR_ROOT, config.get_prev(), config->get_bool(backcfg_changes), &top_left);

        // Close any configuration windows
        config.close();

        // Update the status
        update();
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Start a backup operation for the disc associated with
                  this backup object.
*/
void backobj_obj::start_countdown()
{
    // No action if operation already started
    if (!op && !countdown)
    {
        // Start a countdown
        countdown = TRUE;
        countdown_end = os_read_monotonic_time() + BACKOBJ_COUNTDOWN;

        // Update the status
        update();
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : A connection has been established to the disc associated
                  with this backup object.
*/
void backobj_obj::connect()
{
    // Store the new status
    connected = TRUE;

    // If an automatic backup is due then open the control window
    if (config.get_due()) open();
}

/*
    Parameters  : void
    Returns     : void
    Description : The connection has been lost to the disc associated with
                  this backup object.
*/
void backobj_obj::disconnect()
{
    // Hide the control window
    toolbox_hide_object(0, obj);

    // Hide any configuration windows
    config.close();

    // Cancel any pending operations
    countdown = FALSE;
    if (op && (op->get_status() == backop_op::waiting)) op->abort();

    // Clear any status message
    status = "";

    // Store the new status
    connected = FALSE;
}

/*
    Parameters  : top_left  - The coordinates for the top-left corner of
                              the window, or NULL for the default.
    Returns     : void
    Description : Open the control window for this backup object.
*/
void backobj_obj::open(const os_coord *top_left)
{
    // Choose the window position
    os_coord pos = top_left ? *top_left : get_top_left();

    // Start the backup with a countdown if appropriate
    if (config.get_valid() && config->get_bool(backcfg_always)
        && !(toolbox_get_object_info(0, obj) & toolbox_INFO_SHOWING))
    {
        start_countdown();
    }

    // Show the backup window
    toolbox_show_object(0, obj, toolbox_POSITION_TOP_LEFT,
                        (const toolbox_position *) &pos,
                        toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
}

/*
    Parameters  : void
    Returns     : os_coord  - The coordinates for the top-left corner.
    Description : Return the position of this window if open, or the
                  suggested position otherwise.
*/
os_coord backobj_obj::get_top_left()
{
    // Read the current position if window is open
    if (toolbox_get_object_info(0, obj) & toolbox_INFO_SHOWING)
    {
        wimp_window_state state;

        // Control window is open
        state.w = window_get_wimp_handle(0, obj);
        wimp_get_window_state(&state);

        // Copy the window position
        top_left.x = state.visible.x0;
        top_left.y = state.visible.y1;
    }

    // Return the position
    return top_left;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the backup action button.
*/
bool backobj_obj::start(bits event_code, toolbox_action *action,
                        toolbox_block *id_block, void *handle)
{
    backobj_obj *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Start the backup immediately
        ptr->start();

        // Close the window if appropriate
        if (!(action->flags & actionbutton_SELECTED_ADJUST))
        {
            // Close the control window
            toolbox_hide_object(0, ptr->obj);
        }
        else
        {
            // Update the control window
            ptr->update();
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
    Description : Handle the options action button.
*/
bool backobj_obj::options(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle)
{
    backobj_obj *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Cancel any countdown
        ptr->countdown = FALSE;

        // Open the configuration window
        os_coord top_left = ptr->get_top_left();
        top_left.x += BACKOBJ_OFFSET_CONFIG_X;
        top_left.y += BACKOBJ_OFFSET_CONFIG_Y;
        ptr->config.open(&top_left);

        // Close the window if appropriate
        if (!(action->flags & actionbutton_SELECTED_ADJUST))
        {
            // Close the control window
            toolbox_hide_object(0, ptr->obj);
        }
        else
        {
            // Update the control window
            ptr->update();
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
    Description : Handle the abort action button.
*/
bool backobj_obj::abort(bits event_code, toolbox_action *action,
                        toolbox_block *id_block, void *handle)
{
    backobj_obj *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Abort any pending or active backup
        ptr->countdown = FALSE;
        if (ptr->op) ptr->op->abort();

        // Close the window if appropriate
        if (!(action->flags & actionbutton_SELECTED_ADJUST))
        {
            // Close the control window
            toolbox_hide_object(0, ptr->obj);
        }
        else
        {
            // Update the control window
            ptr->update();
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
    Description : Handle the open action button.
*/
bool backobj_obj::filer(bits event_code, toolbox_action *action,
                        toolbox_block *id_block, void *handle)
{
    backobj_obj *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found or not valid
    if (ptr && ptr->config->get_valid())
    {
        // Enable simple error handling
        filer_error_allowed++;

        // Open a filer window for the drive
        os_cli((string("Filer_OpenDir ") + ptr->config->get_dir()).c_str());

        // Restore normal error handling
        filer_error_allowed--;
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : id_block      - A toolbox ID block.
    Returns     : icons_drive   - Pointer to the corresponding backup
                                  object, or NULL if not found.
    Description : Convert a toolbox ID block into a backup object pointer.
*/
backobj_obj *backobj_obj::find(const toolbox_block *id_block)
{
    toolbox_o id;
    backobj_obj *ptr;

    // Choose the ancestor object ID
    id = id_block->ancestor_obj == toolbox_NULL_OBJECT
         ? id_block->this_obj
         : id_block->ancestor_obj;

    // Attempt to read the client handle
    if (xtoolbox_get_client_handle(0, id, (void **) &ptr)) ptr = NULL;

    // Return the pointer
    return ptr;
}

/*
    Parameters  : drive         - The drive letter, or FS_CHAR_DRIVE_ALL
                                  for all drives.
                  top_left      - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
    Returns     : void
    Description : Open the control window for the matching backup object.
*/
void backobj_obj::open(psifs_drive drive, const os_coord *top_left)
{
    list_iterator<backobj_obj *> i;

    // Open all of the macthing control windows
    for (i = backobj_list.begin(); i != backobj_list.end(); i++)
    {
        if (((drive == FS_CHAR_DRIVE_ALL)
             || ((*i)->config.get_drive() == drive))
            && (*i)->connected)
        {
            (*i)->open(top_left);
        }
    }
}

/*
    Parameters  : name          - The disc name.
    Returns     : void
    Description : Open the control window for all matching backup objects.
*/
void backobj_obj::open(string name)
{
    list_iterator<backobj_obj *> i;

    // Open all of the macthing control windows
    for (i = backobj_list.begin(); i != backobj_list.end(); i++)
    {
        if (((*i)->config->get_str(backcfg_name) == name) && (*i)->connected)
        {
            (*i)->open();
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the status of all backup objects.
*/
void backobj_obj::update_all()
{
    static bool active = FALSE;
    list_iterator<backobj_obj *> i;

    // Action depends on the current mode
    if (psifsget_mode() == psifs_MODE_LINK)
    {
        // Initialise the first time
        if (!active)
        {
            // Load all the backup configurations
            backcfg_obj::initialise();

            // Set the active flag
            active = TRUE;
        }

        // Check the status of each drive
        for (psifs_drive drive = psifs_DRIVE_FIRST_UPPER;
             drive <= psifs_DRIVE_LAST_UPPER;
             drive++)
        {
            backcfg_disc disc;
            bool valid = disc.read(drive);
            bool found = FALSE;

            // Compare all of the backup objects
            for (i = backobj_list.begin(); i != backobj_list.end(); i++)
            {
                if ((*i)->config.get_drive() == drive)
                {
                    if ((*i)->config.eq(disc))
                    {
                        // Drive is connected
                        if (!(*i)->connected) (*i)->connect();
                        found = TRUE;
                    }
                    else
                    {
                        // Drive is not connected
                        if ((*i)->connected) (*i)->disconnect();
                    }
                }
            }

            // Create a new backup object if necessary
            if (valid && !found) new backobj_obj(disc);
        }

        // Update all of the backup objects
        for (i = backobj_list.begin(); i != backobj_list.end(); i++)
        {
            (*i)->update();
        }
    }
    else
    {
        // No action if already inactive
        if (active)
        {
            // Delete all backup objects if remote link disabled
            while (!backobj_list.empty())
            {
                // Delete the backup object from the front of the list
                delete backobj_list.front();
            }

            // Unload any backup configuration objects
            backcfg_obj::finalise();

            // Clear the active flag
            active = FALSE;
        }
    }
}
