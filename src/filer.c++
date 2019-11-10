/*
    File        : filer.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : The main module for the desktop filer.

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
#include "filer.h"

// Include clib header files
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel.h"

// Include cathlibcpp header files
#include "iostream.h"
#include "sstream.h"

// Include oslib header files
#include "oslib/hourglass.h"
#include "oslib/os.h"
#include "oslib/osfind.h"
#include "oslib/osgbpb.h"
#include "oslib/osmodule.h"
#include "oslib/pdriver.h"
#include "oslib/proginfo.h"
#include "oslib/quit.h"
#include "oslib/saveas.h"
#include "oslib/toolbox.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"
#include "event.h"

// Include project header files
#include "action.h"
#include "asyncwin.h"
#include "backcfg.h"
#include "backobj.h"
#include "claim.h"
#include "clipwin.h"
#include "config.h"
#include "configwin.h"
#include "convobj.h"
#include "convwin.h"
#include "ctrl.h"
#include "fontobj.h"
#include "info.h"
#include "module.h"
#include "open.h"
#include "options.h"
#include "psifs.h"
#include "printjob.h"
#include "printprwin.h"
#include "update.h"
#include "util.h"

// User toolbox events
#define FILER_ACTION_QUIT_ALL (0x1000)
#define FILER_ACTION_QUIT_FILER (0x1001)
#define FILER_ACTION_HELP (0x1002)
#define FILER_ACTION_CONFIG_SHOW (0x1010)
#define FILER_ACTION_CONFIG_HIDE (0x1011)
#define FILER_ACTION_CONFIG (0x1012)
#define FILER_ACTION_CONFIG_CANCEL (0x1013)
#define FILER_ACTION_CONFIG_SAVE (0x1014)
#define FILER_ACTION_DISCONNECT (0x1100)
#define FILER_ACTION_LINK (0x1200)
#define FILER_ACTION_PRINTER_DEFAULT (0x1300)
#define FILER_ACTION_PRINTER_PARALLEL (0x1301)
#define FILER_ACTION_PRINTER_SERIAL (0x1302)
#define FILER_ACTION_FILER (0x1400)
#define FILER_ACTION_NAME_INIT (0x1401)
#define FILER_ACTION_NAME (0x1402)
#define FILER_ACTION_BACKUP (0x1403)
#define FILER_ACTION_FREE (0x1404)
#define FILER_ACTION_BACKUP_OPTIONS (0x1500)
#define FILER_ACTION_BACKUP_START (0x1501)
#define FILER_ACTION_BACKUP_ABORT (0x1502)
#define FILER_ACTION_BACKUP_CONFIG_SHOW (0x1510)
#define FILER_ACTION_BACKUP_CONFIG_HIDE (0x1511)
#define FILER_ACTION_BACKUP_CONFIG_SET (0x1512)
#define FILER_ACTION_BACKUP_CONFIG_CANCEL (0x1513)
#define FILER_ACTION_BACKUP_FILER (0x1514)
#define FILER_ACTION_BACKUP_WARN_SET_DISC (0x1520)
#define FILER_ACTION_BACKUP_WARN_SET_DRIVE (0x1521)
#define FILER_ACTION_BACKUP_WARN_SET_MACHINE (0x1522)
#define FILER_ACTION_BACKUP_WARN_CANCEL (0x1523)
#define FILER_ACTION_BACKUP_WARN_SHOW (0x1524)
#define FILER_ACTION_BACKUP_WARN_HIDE (0x1525)
#define FILER_ACTION_BACKUP_WARN_SET_MEDIA (0x1526)
#define FILER_ACTION_CONVERT_CONVERT (0x1600)
#define FILER_ACTION_CONVERT_CANCEL (0x1601)

// The directory which contains the resource files
#define FILER_DIRECTORY "<PsiFS$Dir>"

// The latest wimp version known about
#define FILER_WIMP wimp_VERSION_RO3

// Interval between null polls
#define FILER_POLL_INTERVAL (100)

// Wimp messages of interest, or 0 for all
static const wimp_message_list filer_wimp_messages[] = {0};

// Array of toolbox events of interest, or 0 for all events
static toolbox_action_list filer_toolbox_events[] = {0};

// Control closedown
static bool filer_quit = FALSE;         // Exit immediately
static bool filer_kill = FALSE;         // Kill module on exit
static wimp_t filer_quit_sender = wimp_BROADCAST;// Sender of PreQuit message
static toolbox_o filer_quit_obj;        // The quit object

// Toolbox and wimp variables
static char filer_directory[256];       // GSTrans'd application directory
static char filer_resources[256];       // GSTrans'd resource directory
static toolbox_block filer_id_block;    // The toolbox event ID block
static messagetrans_control_block filer_message_block;// Toolbox message block
static osspriteop_area *filer_sprite_area;// Sprite area pointer
static int filer_wimp_version;          // The current wimp version
static string filer_task_name;          // Name of this task

// Task handle for this task
wimp_t filer_task_handle;

// Poll word returned by PsiFS module
int *filer_poll_word = NULL;

// A variable to increment around error generating routines
bits filer_error_allowed = 0;

// Signal handling
static jmp_buf filer_env;               // Long jump environment
static os_error filer_err;              // The last OS error
static bool filer_nested = FALSE;       // Has the signal handler been nested
static const os_error filer_fatal = {0, "A serious error has occurred within the error handler. Application must quit immediately."};

// Function prototypes
static void filer_catch(int sig);

/*
    Parameters  : token - The token to lookup.
                  arg0  - The optional first parameter.
                  arg1  - The optional second parameter.
                  arg2  - The optional third parameter.
                  arg3  - The optional fourth parameter.
    Returns     : char  - Pointer to the result.
    Description : Lookup the specified token in the message file opened by
                  the toolbox, substituting up to four parameters.
*/
string filer_msgtrans(const char *token, const char *arg0, const char *arg1,
                      const char *arg2, const char *arg3)
{
    char *ptr;
    int used;
    string value;

    // First pass without substitution to find the required buffer size
    ptr = messagetrans_lookup(&filer_message_block, token, NULL, 0,
                              arg0, arg1, arg2, arg3, &used);

    // Special case if no substitution required
    if (!arg0 && !arg1 && !arg2 && !arg3) value.assign(ptr, used);
    else
    {
        // Allow extra space for parameter substitution
        if (arg0) used += strlen(arg0);
        if (arg1) used += strlen(arg1);
        if (arg2) used += strlen(arg2);
        if (arg3) used += strlen(arg3);

        // Allocate a buffer to hold the result
        ptr = new char[used + 1];

        // Perform the parameter substitution
        messagetrans_lookup(&filer_message_block, token, ptr, used + 1,
                            arg0, arg1, arg2, arg3, &used);
        value.assign(ptr, used);

        // Free the temporary buffer
        delete[] ptr;
    }

    // Return the result
    return value;
}

/*
    Parameters  : token - The token to lookup.
                  arg0  - The optional first parameter.
                  arg1  - The optional second parameter.
                  arg2  - The optional third parameter.
                  arg3  - The optional fourth parameter.
    Returns     : char  - Pointer to the supplied buffer.
    Description : Lookup the specified token in the message file opened by
                  the toolbox, substituting up to four parameters, and placing
                  the result in the supplied buffer.
*/
char *filer_msgtrans(char *buffer, size_t size, const char *token,
                     const char *arg0, const char *arg1,
                     const char *arg2, const char *arg3)
{
    int used;

    // Perform the translation as requested
    messagetrans_lookup(&filer_message_block, token, buffer, size,
                        arg0, arg1, arg2, arg3, &used);

    // Ensure that a null terminator is used
    buffer[used < size ? used : size - 1] = '\0';

    // Return a pointer to the result
    return buffer;
}

/*
    Parameters  : task  - The task handle.
    Returns     : bool  - Are the tasks the same.
    Description : Check if the specified task handle refers to this task.
*/
bool filer_eq_task(wimp_t task)
{
    // Return the result of the comparison
    return (((bits) task) & UTIL_TASK_MASK)
           == (((bits) filer_task_handle) & UTIL_TASK_MASK);
}

/*
    Parameters  : void
    Returns     : bool  - Can the quit proceed.
    Description : Check if there is any reason for a quit to be postponed.
*/
static bool filer_quit_query()
{
    // The default action is not to restart a shutdown
    filer_quit_sender = wimp_BROADCAST;

    // Check for active operations
    vector<string> active;
    if (convwin_base::active_count())
    {
        active.push_back(filer_msgtrans("QuitCon"));
    }
    if (printjob_active())
    {
        active.push_back(filer_msgtrans("QuitPrj"));
    }
    if (asyncwin_win::active_count())
    {
        active.push_back(filer_msgtrans("QuitAsn"));
    }

    // Query the quit if any active operations
    if (!active.empty())
    {
        // Combine the descriptions of the active operations
        string desc;
        if (active.size() == 1)
        {
            desc = filer_msgtrans("QuitAc1", active[0].c_str());
        }
        else if (active.size() == 2)
        {
            desc = filer_msgtrans("QuitAc2", active[0].c_str(),
                                  active[1].c_str());
        }
        else
        {
            desc = filer_msgtrans("QuitAc3", active[0].c_str(),
                                  active[1].c_str(), active[2].c_str());
        }

        // Display the quit query
        quit_set_message(0, filer_quit_obj,
                         filer_msgtrans("QuitQst", desc.c_str()).c_str());
        toolbox_show_object(toolbox_SHOW_AS_MENU, filer_quit_obj,
                            toolbox_POSITION_CENTRED, NULL, toolbox_NULL_OBJECT,
                            toolbox_NULL_COMPONENT);
    }

    // Allow the quit if no active operations
    return active.empty();
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Quit both the filer and module.
*/
static bool filer_quit_all(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code)
    NOT_USED(action)
    NOT_USED(id_block)
    NOT_USED(handle)

    // Set the kill module flag
    filer_kill = TRUE;

    // Set the quit flag
    filer_quit = filer_quit_query();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Quit just the filer.
*/
static bool filer_quit_filer(bits event_code, toolbox_action *action,
                             toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code)
    NOT_USED(action)
    NOT_USED(id_block)
    NOT_USED(handle)

    // Clear the kill module flag
    filer_kill = FALSE;

    // Set the quit flag
    filer_quit = filer_quit_query();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Proceed with the quit.
*/
static bool filer_quit_quit(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code)
    NOT_USED(action)
    NOT_USED(id_block)
    NOT_USED(handle)

    // Set the quit flag (filer_kill should still be set correctly)
    filer_quit = TRUE;

    // Restart the desktop closedown sequence if required
    if (filer_quit_sender != wimp_BROADCAST)
    {
        // Send Ctrl-Shift-F12 to the task
        wimp_block block;
        wimp_get_caret_position((wimp_caret *) &block.key);
        block.key.c = wimp_KEY_CONTROL | wimp_KEY_SHIFT
                      | wimp_KEY_F12;
        wimp_send_message(wimp_KEY_PRESSED,
                          (wimp_message *) &block,
                          filer_quit_sender);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_PreQuit wimp message events.
*/
static int filer_pre_quit_message(wimp_message *message, void *handle)
{
    NOT_USED(handle);

    // Set the kill module flag (used if query displayed and confirmed)
    filer_kill = TRUE;

    // Check whether happy to quit
    if (!filer_quit_query())
    {
        // Store the sender's task handle
        filer_quit_sender = message->sender;

        // Object to the closedown
        message->your_ref = message->my_ref;
        wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, message,
                          filer_quit_sender);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_Quit wimp message events.
*/
static int filer_quit_message(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Set the kill module flag
    filer_kill = TRUE;

    // Set the quit flag (too late to object now)
    filer_quit = TRUE;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_TaskInitialise wimp message events.
*/
static int filer_init_message(wimp_message *message, void *handle)
{
    wimp_message_task_initialise *ptr = (wimp_message_task_initialise *) &message->data;

    NOT_USED(handle);

    // Check whether the task name matches
    if ((message->sender != filer_task_handle)
        && !ctrl_strcmp(ptr->task_name, filer_task_name.c_str()))
    {
        toolbox_action action;

        // Raise a toolbox event
        memset(&action, sizeof(action), 0);
        action.size = sizeof(action);
        action.action_no = FILER_ACTION_QUIT_FILER;
        action.flags = 0;
        toolbox_raise_toolbox_event(0, toolbox_NULL_OBJECT,
                                    toolbox_NULL_COMPONENT, &action);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_SaveDesktop wimp message events.
*/
static int filer_save_message(wimp_message *message, void *handle)
{
    NOT_USED(handle);

    // Construct the line to add to the desktop boot file
    ostringstream str;
    str << "Run " << filer_directory << endl;

    // Add the line to the desktop boot file
    osgbpb_write(message->data.save_desktop.file,
                 (byte *) str.str().c_str(), str.str().length());

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle key presses.
*/
static bool filer_key(wimp_event_no event_code, wimp_block *block,
                      toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Pass all unrecognised keys to the next task
    wimp_process_key(block->key.c);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle background updates.
*/
static bool filer_update(wimp_event_no event_code, wimp_block *block,
                         toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(block);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Enusre that the pollword is ready for next time
    *filer_poll_word = FALSE;

    // Check whether the status has changed
    update_check(event_code == wimp_NULL_REASON_CODE);

    // Lose fonts when idle
    if (event_code == wimp_NULL_REASON_CODE) fontobj_base::lose_all();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the toolbox system.
*/
static void filer_init_toolbox()
{
    int used;
    toolbox_o id;

    // GSTrans the resource directory name
    os_gs_trans(FILER_DIRECTORY, filer_directory, sizeof(filer_directory),
                &used);
    filer_directory[used] = 0;
    sprintf(filer_resources, "%s.Resources", filer_directory);

    // Initialise event library
    event_initialise(&filer_id_block);

    // Ensure that required reason codes are delivered
    wimp_poll_flags mask;
    event_get_mask(&mask);
    mask = (mask & ~(wimp_MASK_NULL | wimp_QUEUE_KEY | wimp_MASK_POLLWORD))
           | wimp_GIVEN_POLLWORD;
    event_set_mask(mask);

    // Initialise as a toolbox task
    filer_task_handle = toolbox_initialise(0, FILER_WIMP, filer_wimp_messages,
                                           filer_toolbox_events,
                                           filer_resources,
                                           &filer_message_block,
                                           &filer_id_block,
                                           &filer_wimp_version,
                                           &filer_sprite_area);

    // Read the name of this task
    filer_task_name = filer_msgtrans("_TaskName");

    // Create the quit object
    filer_quit_obj = toolbox_create_object(0, (toolbox_id) "Quit");

    // Register wimp event handlers to pass on key presses
    event_register_wimp_handler(event_ANY, wimp_KEY_PRESSED,
                                filer_key, NULL);

    // Register wimp event handlers to handle background updates
    event_register_wimp_handler(event_ANY, wimp_NULL_REASON_CODE,
                                filer_update, NULL);
    event_register_wimp_handler(event_ANY, wimp_POLLWORD_NON_ZERO,
                                filer_update, NULL);

    // Register toolbox handlers to deal with quitting
    event_register_toolbox_handler(event_ANY, FILER_ACTION_QUIT_ALL,
                                   filer_quit_all, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_QUIT_FILER,
                                   filer_quit_filer, NULL);
    event_register_toolbox_handler(filer_quit_obj, action_QUIT_QUIT,
                                   filer_quit_quit, NULL);
    event_register_message_handler(message_PREQUIT,
                                   filer_pre_quit_message, NULL);
    event_register_message_handler(message_QUIT,
                                   filer_quit_message, NULL);
    event_register_message_handler(message_TASK_INITIALISE,
                                   filer_init_message, NULL);

    // Register a wimp message handler to implement the desktop save protocol
    event_register_message_handler(message_SAVE_DESKTOP,
                                   filer_save_message, NULL);

    // Register wimp message handlers to deal with files being dragged
    event_register_message_handler(message_DATA_SAVE,
                                   convwin_win::data_save, NULL);
    event_register_message_handler(message_DATA_SAVE_ACK,
                                   convwin_win::data_save_ack, NULL);
    event_register_message_handler(message_DATA_LOAD,
                                   convwin_win::data_load, NULL);
    event_register_message_handler(message_DATA_LOAD_ACK,
                                   convwin_win::data_load_ack, NULL);

    // Register wimp message handlers to deal with clipboard control
    event_register_message_handler(message_CLAIM_ENTITY,
                                   clipwin_claim_entity, NULL);
    event_register_message_handler(message_DATA_REQUEST,
                                   clipwin_data_request, NULL);
    event_register_message_handler(message_DATA_SAVE,
                                   clipwin_data_save, NULL);
    event_register_message_handler(message_DATA_SAVE_ACK,
                                   clipwin_data_save_ack, NULL);
    event_register_message_handler(message_DATA_LOAD,
                                   clipwin_data_load, NULL);
    event_register_message_handler(message_DATA_LOAD_ACK,
                                   clipwin_data_load_ack, NULL);
    event_register_message_handler(message_TASK_CLOSE_DOWN,
                                   clipwin_task_close_down, NULL);

    // Register wimp message handlers to deal with printer selection
    event_register_message_handler(message_SET_PRINTER,
                                   printjob_set_printer, NULL);

    // Register wimp message handlers to deal with the device claim protocol
    event_register_message_handler(message_DEVICE_CLAIM,
                                   claim_device_claim, NULL);
    event_register_message_handler(message_DEVICE_IN_USE,
                                   claim_device_in_use, NULL);
    event_register_wimp_handler(event_ANY, wimp_USER_MESSAGE_ACKNOWLEDGE,
                                claim_acknowledge, NULL);

    // Register wimp message handlers to deal with application messages
    event_register_message_handler(message_PSIFS_ASYNC_START,
                                   asyncwin_win::start, NULL);
    event_register_message_handler(message_PSIFS_PRINT,
                                   printjob_print, NULL);

    // Register toolbox handlers for other application events
    event_register_toolbox_handler(event_ANY, FILER_ACTION_CONFIG_SHOW,
                                   configwin_show, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_CONFIG_HIDE,
                                   configwin_hide, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_CONFIG,
                                   configwin_action, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_CONFIG_CANCEL,
                                   configwin_cancel, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_CONFIG_SAVE,
                                   configwin_save, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_DISCONNECT,
                                   action_disconnect, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_LINK,
                                   action_link, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_PRINTER_DEFAULT,
                                   action_printer_default, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_PRINTER_PARALLEL,
                                   action_printer_parallel, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_PRINTER_SERIAL,
                                   action_printer_serial, NULL);
    event_register_toolbox_handler(event_ANY, action_SAVE_AS_SAVE_TO_FILE,
                                   action_printer_file, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_FILER,
                                   action_filer, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_NAME_INIT,
                                   action_name_init, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_NAME,
                                   action_name_action, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP,
                                   action_backup, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_FREE,
                                   action_free, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_HELP,
                                   action_help, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_OPTIONS,
                                   backobj_obj::options, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_START,
                                   backobj_obj::start, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_ABORT,
                                   backobj_obj::abort, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_CONFIG_SHOW,
                                   backcfg_obj::show, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_CONFIG_HIDE,
                                   backcfg_obj::hide, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_CONFIG_SET,
                                   backcfg_obj::set, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_CONFIG_CANCEL,
                                   backcfg_obj::cancel, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_FILER,
                                   backobj_obj::filer, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_WARN_SET_DISC,
                                   backcfg_cfg::set_disc, NULL);
    event_register_toolbox_handler(event_ANY,
                                   FILER_ACTION_BACKUP_WARN_SET_DRIVE,
                                   backcfg_cfg::set_drive, NULL);
    event_register_toolbox_handler(event_ANY,
                                   FILER_ACTION_BACKUP_WARN_SET_MACHINE,
                                   backcfg_cfg::set_machine, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_WARN_CANCEL,
                                   backcfg_cfg::cancel, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_WARN_SHOW,
                                   backcfg_cfg::show, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_BACKUP_WARN_HIDE,
                                   backcfg_cfg::hide, NULL);
    event_register_toolbox_handler(event_ANY,
                                   FILER_ACTION_BACKUP_WARN_SET_MEDIA,
                                   backcfg_cfg::set_media, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_CONVERT_CONVERT,
                                   convwin_base::convert_this, NULL);
    event_register_toolbox_handler(event_ANY, FILER_ACTION_CONVERT_CANCEL,
                                   convwin_base::cancel_this, NULL);

    // Set the version field in the program information window
    id = toolbox_create_object(0, (toolbox_id) "ProgInfo");
    proginfo_set_version(0, id, filer_msgtrans("Version").c_str());
}

/*
    Parameters  : void
    Returns     : void
    Description : Perform any tidying up required before the application quits.
*/
static void filer_atexit()
{
    // Save the options file
    options_current.save();

    // Cancel any active operations
    asyncwin_win::cancel_all();
    convwin_base::cancel_all();
    printjob_cancel();

    // Ensure that any fonts have been lost
    fontobj_base::lose_all();

    // Unregister from the PsiFS module
    if (filer_poll_word)
    {
        xpsifs_unregister(filer_poll_word);
        filer_poll_word = NULL;
    }

    // Kill the PsiFS module if required
    if (filer_kill)
    {
        // Close any open filer windows if required
        if (config_current.get_bool(config_tag_close_kill)) open_close();

        // Kill the module
        xosmodule_kill(Module_Title);
    }
}

/*
    Parameters  : sig   - The signal that was raised.
    Returns     : void
    Description : Signal handler after a stack has been acquired.
*/
static void filer_signal(int sig)
{
    os_error err;
    bits *dump;
    bits *showregs;
    char pc[16];
    bool quit = TRUE;
    string msg;

    // Read where the C library put the registers
    xos_change_environment(os_HANDLER_CALL_BACK, NULL, NULL, NULL,
                           NULL, NULL, (byte **) &dump);

    // Read where *ShowRegs gets them from
    xos_change_environment(os_HANDLER_EXCEPTION_REGISTERS, NULL, NULL, NULL,
                           (void **) &showregs, NULL, NULL);

    // Copy the C register dump
    if (dump && showregs) memcpy(showregs, dump, 16 * sizeof(bits));

    // Attempt to extract the PC value
    if (dump) sprintf(pc, "&%08X", dump[15] & ~0xfc000003);
    else strcpy(pc, "&deaddead");

    // Replace some error messages
    switch (filer_err.errnum & 0x00ffffff)
    {
        case 1:
            // Prefetch abort
            msg = filer_msgtrans("SigErr1:%0 at %1", filer_err.errmess, pc);
            break;

        case 2:
            // Abort on data transfer
            msg = filer_msgtrans("SigErr2:%0 at %1", filer_err.errmess, pc);
            break;

        case 3:
            // Address exception
            msg = filer_msgtrans("SigErr3:%0 at %1", filer_err.errmess, pc);
            break;

        case 5:
            // Branch through zero
            msg = filer_msgtrans("SigErr5:%0 at %1", filer_err.errmess, pc);
            break;

        default:
            // Not a recognised error
            break;
    }

    // Clear the signal if a message has already been generated
    if (!msg.empty()) sig = 0;

    // Choose an appropriate message
    switch (sig)
    {
        case 0:
            // Already substituted a message
            break;

        case SIGFPE:
            // Floating point exception
            msg = filer_msgtrans("SigFPE:%0", filer_err.errmess);
            break;

        case SIGSTAK:
            // Stack overflow
            msg = filer_msgtrans("SigStak:%0", filer_err.errmess);
            break;

        case SIGILL:
            // Illegal instruction
            msg = filer_msgtrans("SigIll:%0 at %1", filer_err.errmess, pc);
            break;

        case SIGSEGV:
            // An unexpected illegal memory access
            msg = filer_msgtrans("SigSegV:%0 at %1", filer_err.errmess, pc);
            break;

        case SIGOSERROR:
            // An operating system error
            if (filer_error_allowed) quit = FALSE;
            else msg = filer_msgtrans("SigErr:%0", filer_err.errmess);
            break;

        default:
            // Unrecognised signal
            sprintf(pc, "%i", sig);
            msg = filer_msgtrans("SigUnk:%0", pc);
            break;
    }

    // Destroy any hourglass that was being used
    xhourglass_smash();

    // Cancel any print job that was in progress
    os_fw handle;
    if (!xpdriver_current_jobw(&handle) && handle)
    {
        xpdriver_abort_jobw(handle);
        osfind_closew(handle);
    }

    // Special case if an expected error
    if (quit)
    {
        // Use a suitable form of error window
        if (wimp_VERSION_RO35 <= filer_wimp_version)
        {
            // Construct the error block
            err.errnum = 0;
            filer_msgtrans(err.errmess, sizeof(err.errmess), "SigTpl:%0",
                           msg.c_str());

            // Display the error message
            quit = wimp_report_error_by_category(&err, 0,
                        filer_task_name.c_str(),
                        NULL, wimpspriteop_AREA,
                        filer_msgtrans("SigBut:Continue,Quit").c_str())
                   != 3;
        }
        else
        {
            // Construct the error block
            err.errnum = 0;
            filer_msgtrans(err.errmess, sizeof(err.errmess), "SigTplO:%0",
                           msg.c_str());

            // Display the error message
            quit = wimp_report_error(&err, wimp_ERROR_BOX_OK_ICON
                                     | wimp_ERROR_BOX_CANCEL_ICON,
                                     filer_task_name.c_str())
                   != wimp_ERROR_BOX_SELECTED_OK;
        }
    }
    else
    {
        // Display the error
        wimp_report_error(&filer_err, wimp_ERROR_BOX_CANCEL_ICON,
                          filer_task_name.c_str());
    }

    // Exit if the Quit option was selected
    if (quit) exit(EXIT_FAILURE);
}

/*
    Parameters  : argc  - The number of command line arguments.
                  argv  - The command line arguments.
    Returns     : int   - The return code.
    Description : The main program!
*/
int main(int argc, char *argv[])
{
    wimp_block poll_block;
    wimp_event_no event_code;
    os_t next_time;
    int sig;

    NOT_USED(argc)
    NOT_USED(argv)

    // Register a function to tidy up
    atexit(filer_atexit);

    // Initialise the configuration
    config_init();

    // Load the options file
    options_current.load();

    // Initialise as a toolbox task
    filer_init_toolbox();

    // Initialise the font handling
    fontobj_init();

    // Register with the PsiFS module
    filer_poll_word = psifs_register(filer_task_name.c_str(),
                                     psifs_MASK_MODE
                                     | psifs_MASK_LINK_STATUS
                                     | psifs_MASK_LINK_DRIVES
                                     | psifs_MASK_LINK_ASYNC_END
                                     | psifs_MASK_LINK_ASYNC_STATE
                                     | psifs_MASK_INTERCEPT_STATUS
                                     | psifs_MASK_CLIPBOARD_STATUS
                                     | psifs_MASK_PRINT_JOB_STATUS);
    *filer_poll_word = TRUE;

    // Initialise the converters
    convobj_init();

    // Set the jump point for signal handling
    sig = setjmp(filer_env);
    if (0 < sig) filer_signal(sig);

    // Clear the nested handler flag
    filer_nested = FALSE;

    // Install signal handlers
    signal(SIGFPE, filer_catch);
    signal(SIGILL, filer_catch);
    signal(SIGSEGV, filer_catch);
    signal(SIGSTAK, filer_catch);
    signal(SIGOSERROR, filer_catch);

    // Keep polling until quit flag is set
    next_time = os_read_monotonic_time();
    while (!filer_quit)
    {
        // Ensure that the acceptable error variable is cleared
        filer_error_allowed = 0;

        // Poll for anything interesting
        event_poll_idle(&event_code, &poll_block, next_time, filer_poll_word);
        if (event_code == wimp_NULL_REASON_CODE)
        {
            os_t now = os_read_monotonic_time();
            while (0 <= (now - next_time)) next_time += FILER_POLL_INTERVAL;
        }
    }

    // If the program gets this far then everything went alright
    return EXIT_SUCCESS;
}

// Disable stack checking for the signal handler (could be out of stack signal)
#pragma no_check_stack

/*
    Parameters  : sig   - The signal that was raised.
    Returns     : void
    Description : Signal handler.
*/
static void filer_catch(int sig)
{
    // Check if it is a nested error
    if (filer_nested)
    {
        // Take panic action
        wimp_report_error(&filer_fatal, wimp_ERROR_BOX_OK_ICON, "application");
        os_exit(&filer_fatal, EXIT_FAILURE);
    }

    // Set the nested error flag
    filer_nested = TRUE;

    // Store the error message
    filer_err = * (os_error *) _kernel_last_oserror();

    // Return to main to report the error
    longjmp(filer_env, sig);

    // Just in case
    exit(EXIT_FAILURE);
}
