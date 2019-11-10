/*
    File        : clipwin.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Clipboard window handling for the PsiFS filer.

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
#include "clipwin.h"

// Include oslib header files
#include "oslib/menu.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"
#include "oslib/taskmanager.h"
#include "oslib/window.h"
#include "event.h"

// Include alexlib header files
#include "actionbutton_c.h"
#include "displayfield_c.h"

// Include cathlibcpp header files
#include "algorithm.h"
#include "fstream.h"
#include "list.h"

// Include project header files
#include "config.h"
#include "convwin.h"
#include "filer.h"
#include "options.h"
#include "psifs.h"
#include "scrap.h"

// Menu gadgets
#define CLIPWIN_MENU_CLIPBOARD ((toolbox_c) 0x13)

// Clipboard window gadgets
#define CLIPWIN_GADGET_EXPORT ((toolbox_c) 0x1)
#define CLIPWIN_GADGET_STATUS ((toolbox_c) 0x10)

// Fake icon handles to use for Message_DataRequest messages
#define CLIPWIN_ICON_REQUEST ((wimp_i) 0xdeafc0de)
#define CLIPWIN_ICON_REQUEST_FORCE ((wimp_i) 0xfeedc0de)

// Temporary file to specify in Message_DataSave message
#define CLIPWIN_TEMP_LEAF "Clipboard"
#define CLIPWIN_TEMP_FILE "<Wimp$Scrap>"

// Temporary file for converted clipboard data
static string clipwin_temp_converted = scrap_name("Converted");

// Clipboard file type
#define CLIPWIN_CLIPBOARD_TYPE (0x158)

// Reference numbers of the data transfer messages
static set<bits, less<bits> > clipwin_ref;

// Clipboard ownership
static wimp_t clipwin_clipboard_owner = wimp_BROADCAST;
static string clipwin_clipboard_task;

// Polling for changes to clipboard content
#define CLIPWIN_POLL_INTERVAL (500)
static os_t clipwin_poll_next = 0;
static bool clipwin_poll_force = FALSE;

// Menu object
static toolbox_o clipwin_menu_obj;

// Window object
static toolbox_o clipwin_win_obj;

// Temporary status messages
#define CLIPWIN_FLASH_TIME (250)
static bool clipwin_flash_active = FALSE;
static os_t clipwin_flash_end;

/*
    Parameters  : msg   - The status message.
    Returns     : void
    Description : Flash a temporary message in the status window.
*/
static void clipwin_flash_status(const string &msg)
{
    // Set the status message
    displayfield_c(CLIPWIN_GADGET_STATUS, clipwin_win_obj) = msg;

    // Reset the flash status and timer
    clipwin_flash_active = TRUE;
    clipwin_flash_end = os_read_monotonic_time() + CLIPWIN_FLASH_TIME;
}

/*
    Parameters  : a     - The first converter.
                  b     - The second converter.
    Returns     : bool  - Should a be ordered before b.
    Description : Compare the quality of two converters.
*/
static bool clipwin_compare_quality(const convobj_ptr &a, const convobj_ptr &b)
{
    return b->get_num(convobj_quality) < a->get_num(convobj_quality);
}

/*
    Parameters  : file  - The name of the file to convert.
                  types - List of acceptable types.
    Returns     : void
    Description : Convert the clipboard file to one of the specified types
                  using the best available conversion.
*/
static void clipwin_convert_paste(const string &name, list<bits> &types)
{
    // Read the file type
    bits in_type;
    osfile_read_stamped_no_path(name.c_str(), 0, 0, 0, 0, &in_type);

    // Read the file UID
    epoc32_file_uid uid(convwin_base::get_uid(name));

    // Attempt to convert to each type in turn
    bool done = FALSE;
    list_iterator<bits> out_type = types.begin();
    while (!done && (out_type != types.end()))
    {
        // No further action if already the correct type
        if (in_type == *out_type) done = TRUE;
        else
        {
            string temp(scrap_name());

            // Construct a list of possible converters
            list<convobj_ptr> converters;
            for (const convobj_ptr *i = convobj_all.begin();
                 i != convobj_all.end();
                 i++)
            {
                // Add this converter to the list if suitable
                if ((*i)->supports_silent
                    && (*i)->type_in.count(in_type)
                    && ((*i)->uid_in.empty() || (*i)->uid_in.count(uid))
                    && ((*i)->get_type() == *out_type))
                {
                    converters.push_back(*i);
                }
            }

            // Sort the converters by quality
            converters.sort(clipwin_compare_quality);

            // Try each converter in turn until successful
            list_iterator<convobj_ptr> j = converters.begin();
            while (!done && (j != converters.end()))
            {
                // Update the status window
                clipwin_flash_status(filer_msgtrans("ClpSPCT", (*j)->get_str(convobj_menu).c_str()));

                // Perform the conversion
                done = (*j)->run(name, temp, CLIPWIN_TEMP_LEAF, "", TRUE);
                if (!done) j++;
            }

            // Replace the original file if successful
            if (done)
            {
                // Enable simple error handling
                filer_error_allowed++;

                // Attempt to save the result of the conversion
                xosfscontrol_wipe(name.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
                osfscontrol_copy(temp.c_str(), name.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE | osfscontrol_COPY_DELETE, 0, 0, 0, 0, NULL);

                // Restore normal error handling
                filer_error_allowed--;

                // Update the status window
                clipwin_flash_status(filer_msgtrans("ClpSPCS", (*j)->get_str(convobj_menu).c_str()));
            }
        }

        // Prepare for the next type
        out_type++;
    }

    // Update the status window
    if (!done) clipwin_flash_status(filer_msgtrans("ClpSPCF"));
}

/*
    Parameters  : type  - Variable to receive the list of types.
    Returns     : void
    Description : Produce a list of file types that can be converted silently
                  to clipboard files, ranked by the quality of conversion.
*/
static void clipwin_copy_types(list<bits> &types)
{
    // Construct a list of possible converters
    list<convobj_ptr> converters;
    for (const convobj_ptr *i = convobj_all.begin();
         i != convobj_all.end();
         i++)
    {
        // Add this converter to the list if suitable
        if ((*i)->supports_silent
            && ((*i)->get_type() == CLIPWIN_CLIPBOARD_TYPE))
        {
            converters.push_back(*i);
        }
    }

    // Sort the list of converters by quality
    converters.sort(clipwin_compare_quality);

    // Can always accept files already in clipboard format
    types.clear();
    set<bits, less<bits> > types_set;
    types.push_front(CLIPWIN_CLIPBOARD_TYPE);
    types_set.insert(CLIPWIN_CLIPBOARD_TYPE);

    // Construct a list of unique filetypes, preserving the quality order
    for (list_iterator<convobj_ptr> j = converters.begin();
         j != converters.end();
         j++)
    {
        for (set_iterator<bits, less<bits> > k = (*j)->type_in.begin();
             k != (*j)->type_in.end();
             k++)
        {
            if (types_set.find(*k) == types_set.end())
            {
                types.push_back(*k);
                types_set.insert(*k);
            }
        }
    }
}

/*
    Parameters  : name  - The name of the file to convert.
    Returns     : void
    Description : Convert the specified file to clipboard format using the
                  best available conversion.
*/
static void clipwin_copy_convert(const string &name)
{
    // Read the file type
    bits file_type;
    osfile_read_stamped_no_path(name.c_str(), 0, 0, 0, 0, &file_type);

    // Read the file UID
    epoc32_file_uid uid(convwin_base::get_uid(name));

    // No action required if already a clipboard file
    if (file_type == CLIPWIN_CLIPBOARD_TYPE)
    {
        // Update the status window
        clipwin_flash_status(filer_msgtrans("ClpSCCN"));
    }
    else
    {
        string temp(scrap_name());

        // Construct a list of possible converters
        list<convobj_ptr> converters;
        for (const convobj_ptr *i = convobj_all.begin();
             i != convobj_all.end();
             i++)
        {
            // Add this converter to the list if suitable
            if ((*i)->supports_silent
                && (*i)->type_in.count(file_type)
                && ((*i)->uid_in.empty() || (*i)->uid_in.count(uid))
                && ((*i)->get_type() == CLIPWIN_CLIPBOARD_TYPE))
            {
                converters.push_back(*i);
            }
        }

        // Sort the converters by quality
        converters.sort(clipwin_compare_quality);

        // Try each converter in turn until successful
        bool done = FALSE;
        list_iterator<convobj_ptr> j = converters.begin();
        while (!done && (j != converters.end()))
        {
            // Update the status window
            clipwin_flash_status(filer_msgtrans("ClpSCCT", (*j)->get_str(convobj_menu).c_str()));

            // Perform the conversion
            done = (*j)->run(name, temp, CLIPWIN_TEMP_LEAF, "", TRUE);
            if (!done) j++;
        }

        // Replace the original file if successful
        if (done)
        {
            // Enable simple error handling
            filer_error_allowed++;

            // Attempt to save the result of the conversion
            xosfscontrol_wipe(name.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
            osfscontrol_copy(temp.c_str(), name.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE | osfscontrol_COPY_DELETE, 0, 0, 0, 0, NULL);

            // Restore normal error handling
            filer_error_allowed--;

            // Update the status window
            clipwin_flash_status(filer_msgtrans("ClpSCCS", (*j)->get_str(convobj_menu).c_str()));
        }
        else
        {
            // Update the status window
            if (!done) clipwin_flash_status(filer_msgtrans("ClpSCCF"));
        }
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
static bool clipwin_export(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    NOT_USED(action);
    NOT_USED(event_code);
    NOT_USED(handle);

    // Only interested in the export button if the clipboard is available
    if ((id_block->this_cmp == CLIPWIN_GADGET_EXPORT)
        && (psifs_clipboard_paste(0, 0)
            & (psifs_CLIPBOARD_LOCAL_CHANGED | psifs_CLIPBOARD_REMOTE_SYNC)))
    {
        string temp(scrap_name());

        // Read the clipboard data
        psifs_clipboard_paste(temp.c_str(), 0);

        // Read the clipboard file type
        bits file_type;
        osfile_read_stamped_no_path(temp.c_str(), 0, 0, 0, 0, &file_type);

        // Create a new conversion window
        new convwin_win(file_type, CLIPWIN_TEMP_LEAF, temp, filer_task_handle);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the clipboard status window if necessary.
*/
static void clipwin_update_win()
{
    static bool active = FALSE;

    // Check the clipboard status
    psifs_clipboard_flags flags = psifs_clipboard_paste(0, 0);

    // Update the window
    if (flags & psifs_CLIPBOARD_SERVER_ACTIVE)
    {
        static psifs_clipboard_flags prev_flags;
        static wimp_t prev_owner;
        static bool prev_flash;

        // Check for the end of a flashed message
        if (clipwin_flash_active
            && ((clipwin_flash_end - os_read_monotonic_time()) < 0))
        {
            clipwin_flash_active = FALSE;
        }

        // Check for changes
        if (!active || (prev_flags != flags)
            || (prev_owner != clipwin_clipboard_owner)
            || (prev_flash != clipwin_flash_active))
        {
            // Store the new status
            prev_flags = flags;
            prev_owner = clipwin_clipboard_owner;
            prev_flash = clipwin_flash_active;

            // Set the status window contents
            displayfield_c status(CLIPWIN_GADGET_STATUS, clipwin_win_obj);
            if (clipwin_flash_active)
            {
                // Leave the temporary message
            }
            else if (flags & psifs_CLIPBOARD_LOCAL_CHANGED)
            {
                // Clipboard modified locally
                status = filer_msgtrans(flags & psifs_CLIPBOARD_LOCAL_SYNC
                                        ? (clipwin_clipboard_task.empty()
                                           ? "ClpSLSU" : "ClpSLSA")
                                        : (clipwin_clipboard_task.empty()
                                           ? "ClpSLMU" : "ClpSLMA"),
                                        clipwin_clipboard_task.c_str());
            }
            else if (flags & psifs_CLIPBOARD_REMOTE_CHANGED)
            {
                // Clipboard modified remotely
                status = filer_msgtrans(flags & psifs_CLIPBOARD_REMOTE_SYNC
                                        ? (clipwin_clipboard_owner
                                           == filer_task_handle
                                           ? "ClpSRSE" : "ClpSRIE")
                                        : "ClpSRME");
            }
            else
            {
                // Clipboard empty
                status = filer_msgtrans("ClpSEmt");
            }

            // Disable the export button if appropriate
            actionbutton_c(CLIPWIN_GADGET_EXPORT, clipwin_win_obj).set_faded(!(flags & (psifs_CLIPBOARD_LOCAL_CHANGED | psifs_CLIPBOARD_REMOTE_SYNC)));
        }

        // Enable the menu entry
        if (!active)
        {
            // Enable the menu entry
            menu_set_fade(0, clipwin_menu_obj, CLIPWIN_MENU_CLIPBOARD, FALSE);

            // Set the active flag
            active = TRUE;
        }
    }
    else
    {
        // Ensure that the window is closed
        if (active)
        {
            // Close the windows
            toolbox_hide_object(0, clipwin_win_obj);

            // Disable the menu entry
            menu_set_fade(0, clipwin_menu_obj, CLIPWIN_MENU_CLIPBOARD, TRUE);

            // Clear the active flag
            active = FALSE;
        }
    }
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_ClaimEntity wimp message events.
*/
int clipwin_claim_entity(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Only interested in changes to the clipboard
    if ((message->data.claim_entity.flags & wimp_CLAIM_CLIPBOARD)
        && (message->sender != filer_task_handle))
    {
        // Store the new clipboard owner
        clipwin_clipboard_owner = message->sender;

        // Update the clipboard status window
        clipwin_update_win();

        // Reset the timer for polling the clipboard contents
        clipwin_poll_next = os_read_monotonic_time();
        clipwin_poll_force = TRUE;
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataRequest wimp message events.
*/
int clipwin_data_request(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Only interested in requests for clipboard data if synchronized
    if ((clipwin_clipboard_owner == filer_task_handle)
        && (message->data.data_request.flags & wimp_CLAIM_CLIPBOARD)
        && (psifs_clipboard_paste(0, 0) & psifs_CLIPBOARD_REMOTE_SYNC))
    {
        // Construct a list of acceptable file types
        list<bits> file_types;
        bits *ptr = message->data.data_request.file_types;
        while (*ptr != -1) file_types.push_back(*ptr++);

        // Read the clipboard data
        psifs_clipboard_paste(clipwin_temp_converted.c_str(), 0);

        // Convert the clipboard to a more appropriate format
        clipwin_convert_paste(clipwin_temp_converted, file_types);

        // Read the converted clipboard file's details
        osfile_read_stamped_no_path(clipwin_temp_converted.c_str(), 0, 0,
                                    &message->data.data_xfer.est_size, 0,
                                    &message->data.data_xfer.file_type);

        // Start the file transfer
        message->action = message_DATA_SAVE;
        message->your_ref = message->my_ref;
        strcpy(message->data.data_xfer.file_name, CLIPWIN_TEMP_LEAF);
        message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                               - (char *) &message + 1);
        wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
        clipwin_ref.insert(message->my_ref);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSave wimp message events.
*/
int clipwin_data_save(wimp_message *message, void *handle)
{
    bool claimed = FALSE;

    NOT_USED(handle);

    // Only interested in the clipboard window if active
    if ((message->data.data_xfer.w
         == window_get_wimp_handle(0, clipwin_win_obj))
        && (psifs_clipboard_paste(0, 0) & psifs_CLIPBOARD_SERVER_ACTIVE))
    {
        bool ack = FALSE;

        // Check if a user drag or Message_DataRequest response
        if ((message->data.data_xfer.i == CLIPWIN_ICON_REQUEST)
            || (message->data.data_xfer.i == CLIPWIN_ICON_REQUEST_FORCE))
        {
            static int prev_est_size = 0;
            static bits prev_file_type = osfile_TYPE_UNTYPED;

            if ((message->data.data_xfer.i == CLIPWIN_ICON_REQUEST_FORCE)
                || config_current.get_bool(config_tag_clipboard_poll_content)
                || (message->data.data_xfer.est_size != prev_est_size)
                || (message->data.data_xfer.file_type != prev_file_type))
            {
                // Store the new values
                prev_est_size = message->data.data_xfer.est_size;
                prev_file_type = message->data.data_xfer.file_type;

                // Request the data
                ack = TRUE;
            }
        }
        else
        {
            // User drag
            ack = TRUE;
        }

        // Request the data if required
        if (ack)
        {
            message->action = message_DATA_SAVE_ACK;
            message->your_ref = message->my_ref;
            message->data.data_xfer.est_size = -1;
            strcpy(message->data.data_xfer.file_name, CLIPWIN_TEMP_FILE);
            message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                                  - (char *) message + 1);
            wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
        }

        // Claim the message
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSaveAck wimp message events.
*/
int clipwin_data_save_ack(wimp_message *message, void *handle)
{
    bool claimed = FALSE;

    NOT_USED(handle);

    // Only process the message if expected
    if (clipwin_ref.erase(message->your_ref))
    {
        // Enable simple error handling
        filer_error_allowed++;

        // Copy the converted clipboard file
        xosfscontrol_wipe(message->data.data_xfer.file_name, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
        osfscontrol_copy(clipwin_temp_converted.c_str(), message->data.data_xfer.file_name, osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE | osfscontrol_COPY_DELETE, 0, 0, 0, 0, NULL);

        // Restore normal error handling
        filer_error_allowed--;

        // Read the converted clipboard file's details
        osfile_read_stamped_no_path(message->data.data_xfer.file_name, 0, 0,
                                    &message->data.data_xfer.est_size, 0, 0);

        // Respond with notification that the file has been saved
        message->action = message_DATA_LOAD;
        message->your_ref = message->my_ref;
        wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
        clipwin_ref.insert(message->my_ref);
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoad wimp message events.
*/
int clipwin_data_load(wimp_message *message, void *handle)
{
    bool claimed = FALSE;

    NOT_USED(handle);

    // Only interested in the clipboard window
    if ((message->data.data_xfer.w
         == window_get_wimp_handle(0, clipwin_win_obj))
        && (psifs_clipboard_paste(0, 0) & psifs_CLIPBOARD_SERVER_ACTIVE))
    {
        bool copy = FALSE;

        // Check if a user drag or Message_DataRequest response
        if ((message->data.data_xfer.i == CLIPWIN_ICON_REQUEST)
            || (message->data.data_xfer.i == CLIPWIN_ICON_REQUEST_FORCE))
        {
            // Calculate the file's CRC
            ifstream file(message->data.data_xfer.file_name);
            vector<char> data;
            char c;
            while (file >> c) data.push_back(c);
            int crc = os_crc(0, (byte *) data.begin(), data.end(), 1);

            // Only interested in Message_DataRequest responses if data changed
            static int prev_crc = 0;
            if ((message->data.data_xfer.i == CLIPWIN_ICON_REQUEST_FORCE)
                || (crc != prev_crc))
            {
                // Store the new value
                prev_crc = crc;

                // Copy the clipboard
                copy = TRUE;
            }
        }
        else
        {
            // User drag
            copy = TRUE;
        }

        // Copy the file to the clipboard if required
        if (copy)
        {
            string temp(scrap_name());

            // Enable simple error handling
            filer_error_allowed++;

            // Take a copy of the file to convert
            osfscontrol_copy(message->data.data_xfer.file_name, temp.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);

            // Convert the file to clipboard format
            clipwin_copy_convert(temp);

            // Update the clipboard
            psifs_clipboard_copy(temp.c_str(), 0);

            // Delete the copied file
            xosfscontrol_wipe(temp.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);

            // Restore normal error handling
            filer_error_allowed--;

            // Get the name of the clipboard source
            char *name;
            clipwin_clipboard_task = xtaskmanager_task_name_from_handle(message->sender, &name) ? "" : name;

            // Reset the time of the next poll
            clipwin_poll_next = os_read_monotonic_time()
                                + CLIPWIN_POLL_INTERVAL;
        }

        // Acknowledge the load
        message->action = message_DATA_LOAD_ACK;
        message->your_ref = message->my_ref;
        wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoadAck wimp message events.
*/
int clipwin_data_load_ack(wimp_message *message, void *handle)
{
    bool claimed = FALSE;

    NOT_USED(handle);

    // Only process the message if expected
    if (clipwin_ref.erase(message->your_ref))
    {
        // The reference number was correct
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_TaskCloseDown wimp message events.
*/
int clipwin_task_close_down(wimp_message *message, void *handle)
{
    NOT_USED(handle);

    // Check if it is the current clipboard owner
    if (message->sender == clipwin_clipboard_owner)
    {
        // Discard the clipboard
        clipwin_clipboard_owner = wimp_BROADCAST;
        clipwin_poll_force = FALSE;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the global clipboard status if necessary
*/
static void clipwin_update_global()
{
    // No action unless the global clipboard support if enabled
    if (config_current.get_bool(config_tag_clipboard_integrate))
    {
        // Check for a change to the clipboard status
        os_t timestamp;
        psifs_clipboard_flags flags = psifs_clipboard_paste(0, &timestamp);

        // Check if the remote clipboard has changed
        static os_t prev_timestamp = 0;
        if ((timestamp != prev_timestamp)
            && (flags & psifs_CLIPBOARD_REMOTE_SYNC))
        {
            // Store the time of last change
            prev_timestamp = timestamp;

            // Beep to indicate that the clipboard has changed
            os_bell();

            // Claim the global clipboard if not already owned
            if (clipwin_clipboard_owner != filer_task_handle)
            {
                // Claim the global clipboard
                wimp_message message;
                message.your_ref = 0;
                message.action = message_CLAIM_ENTITY;
                message.data.claim_entity.flags = wimp_CLAIM_CLIPBOARD;
                message.size = sizeof(wimp_full_message_claim_entity);
                wimp_send_message(wimp_USER_MESSAGE, &message, wimp_BROADCAST);

                // Set the clipboard claimed flag
                clipwin_clipboard_owner = filer_task_handle;
                clipwin_poll_force = FALSE;
            }
        }

        // Only poll if active and not the current owner
        os_t now = os_read_monotonic_time();
        if ((clipwin_clipboard_owner != filer_task_handle)
            && (clipwin_clipboard_owner != wimp_BROADCAST)
            && (flags & psifs_CLIPBOARD_SERVER_ACTIVE)
            && ((flags & psifs_CLIPBOARD_LOCAL_SYNC)
                || !(flags & psifs_CLIPBOARD_LOCAL_CHANGED))
            && (!config_current.get_bool(config_tag_clipboard_poll_disabled)
                || clipwin_poll_force))
        {
            // Check if the poll timer has expired
            if ((clipwin_poll_next - now) < 0)
            {
                // Construct a list of acceptable types
                list<bits> convertable_types;
                clipwin_copy_types(convertable_types);

                // Request a copy of the clipboard
                wimp_message message;
                message.your_ref = 0;
                message.action = message_DATA_REQUEST;
                message.data.data_request.w = window_get_wimp_handle(0, clipwin_win_obj);
                message.data.data_request.i = clipwin_poll_force
                                              ? CLIPWIN_ICON_REQUEST_FORCE
                                              : CLIPWIN_ICON_REQUEST;
                message.data.data_request.pos.x = 0;
                message.data.data_request.pos.y = 0;
                message.data.data_request.flags = wimp_DATA_REQUEST_CLIPBOARD;
                bits types = 0;
                const bits max_types = sizeof(message.data.data_request.file_types) / sizeof(bits) - 1;
                while ((types < max_types) && !convertable_types.empty())
                {
                    message.data.data_request.file_types[types++] = *(convertable_types.begin());
                    convertable_types.pop_front();
                }
                message.data.data_request.file_types[types++] = (bits) -1;
                message.size = (char *) &message.data.data_request.file_types[types] - (char *) &message;
                wimp_send_message(wimp_USER_MESSAGE, &message,
                                  clipwin_clipboard_owner);

                 // Reset the timer for polling the clipboard contents
                clipwin_poll_next = now + CLIPWIN_POLL_INTERVAL;
                clipwin_poll_force = FALSE;
            }
        }
        else
        {
            // Reset the time of the next poll if currently unable to poll
            clipwin_poll_next = now + CLIPWIN_POLL_INTERVAL;
        }
    }
    else if (clipwin_clipboard_owner == filer_task_handle)
    {
        // Ensure that the clipboard is disowned otherwise
        clipwin_clipboard_owner = wimp_BROADCAST;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Perform any file converter updates required.
*/
void clipwin_update(void)
{
    static bool init = FALSE;

    // Initialise on first call
    if (!init)
    {
        // Create the menu object
        clipwin_menu_obj = toolbox_create_object(0, (toolbox_id) "MenuInfo");

        // Create the window objects
        clipwin_win_obj = toolbox_create_object(0, (toolbox_id) "WinClip");

        // Register toolbox handler
        event_register_toolbox_handler(clipwin_win_obj,
                                       action_ACTION_BUTTON_SELECTED,
                                       clipwin_export, 0);

        // Set the time for the next clipboard poll

        // Set the initialised flag
        init = TRUE;
    }

    // Update the global clipboard status if necessary
    clipwin_update_global();

    // Update the clipboard status window if necessary
    clipwin_update_win();
}
