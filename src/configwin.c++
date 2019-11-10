/*
    File        : configwin.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Configuration event handling for the PsiFS filer.

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
#include "configwin.h"

// Include cathlibcpp header files
#include "sstream.h"

// Include oslib header files
#include "oslib/draggable.h"
#include "oslib/hourglass.h"
#include "oslib/macros.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"
#include "oslib/wimp.h"
#include "event.h"

// Include alexlib header files
#include "actionbutton_c.h"
#include "label_c.h"
#include "numberrange_c.h"
#include "optionbutton_c.h"
#include "stringset_c.h"
#include "writablefield_c.h"

// Include project header files
#include "baud.h"
#include "blockdrive.h"
#include "config.h"
#include "filer.h"
#include "fs.h"
#include "psifs.h"
#include "update.h"

// Configuration window gadgets
#define CONFIGWIN_DRIVER ((toolbox_c) 0x10)
#define CONFIGWIN_PORT ((toolbox_c) 0x11)
#define CONFIGWIN_BAUD ((toolbox_c) 0x12)
#define CONFIGWIN_OPTIONS ((toolbox_c) 0x13)
#define CONFIGWIN_AUTO_BAUD ((toolbox_c) 0x14)
#define CONFIGWIN_AUTO_OPEN ((toolbox_c) 0x20)
#define CONFIGWIN_AUTO_CLOSE ((toolbox_c) 0x21)
#define CONFIGWIN_ICON ((toolbox_c) 0x22)
#define CONFIGWIN_PRINTER ((toolbox_c) 0x30)
#define CONFIGWIN_BACKUP_PATH ((toolbox_c) 0x40)
#define CONFIGWIN_BACKUP_DRAG ((toolbox_c) 0x41)
#define CONFIGWIN_BACKUP_DROP ((toolbox_c) 0x42)
#define CONFIGWIN_BACKUP_NEW ((toolbox_c) 0x43)
#define CONFIGWIN_SYNC_CLOCKS ((toolbox_c) 0x50)
#define CONFIGWIN_POWER_MONITOR ((toolbox_c) 0x51)
#define CONFIGWIN_POWER_CUSTOM ((toolbox_c) 0x52)
#define CONFIGWIN_POWER_MAIN_DEAD ((toolbox_c) 0x53)
#define CONFIGWIN_POWER_MAIN_VERY_LOW ((toolbox_c) 0x54)
#define CONFIGWIN_POWER_MAIN_LOW ((toolbox_c) 0x55)
#define CONFIGWIN_POWER_BACKUP_DEAD ((toolbox_c) 0x56)
#define CONFIGWIN_POWER_BACKUP_VERY_LOW ((toolbox_c) 0x57)
#define CONFIGWIN_POWER_BACKUP_LOW ((toolbox_c) 0x58)
#define CONFIGWIN_CLIPBOARD_INTEGRATE ((toolbox_c) 0x59)
#define CONFIGWIN_CLIPBOARD_POLL ((toolbox_c) 0x5a)
#define CONFIGWIN_INTERCEPT_RUN ((toolbox_c) 0x60)
#define CONFIGWIN_INTERCEPT_RUN_AUTO ((toolbox_c) 0x61)
#define CONFIGWIN_INTERCEPT_LOAD ((toolbox_c) 0x62)
#define CONFIGWIN_INTERCEPT_LOAD_AUTO ((toolbox_c) 0x63)
#define CONFIGWIN_INTERCEPT_SAVE ((toolbox_c) 0x64)
#define CONFIGWIN_INTERCEPT_SAVE_AUTO ((toolbox_c) 0x65)
#define CONFIGWIN_IDLE_LINK ((toolbox_c) 0x70)
#define CONFIGWIN_IDLE_LINK_MINUTES ((toolbox_c) 0x71)
#define CONFIGWIN_IDLE_PRINTER ((toolbox_c) 0x72)
#define CONFIGWIN_IDLE_PRINTER_MINUTES ((toolbox_c) 0x73)
#define CONFIGWIN_IDLE_BACKGROUND ((toolbox_c) 0x74)
#define CONFIGWIN_PRINT_AUTO ((toolbox_c) 0x80)
#define CONFIGWIN_PRINT_WAIT ((toolbox_c) 0x81)
#define CONFIGWIN_PRINT_PREVIEW ((toolbox_c) 0x82)
#define CONFIGWIN_PRINT_ANTIALIAS ((toolbox_c) 0x83)
#define CONFIGWIN_LABEL_POWER_DEAD ((toolbox_c) 0x151)
#define CONFIGWIN_LABEL_POWER_VERY_LOW ((toolbox_c) 0x152)
#define CONFIGWIN_LABEL_POWER_LOW ((toolbox_c) 0x153)
#define CONFIGWIN_LABEL_POWER_MAIN ((toolbox_c) 0x154)
#define CONFIGWIN_LABEL_POWER_BACKUP ((toolbox_c) 0x155)
#define CONFIGWIN_LABEL_CLIPBOARD_POLL ((toolbox_c) 0x156)
#define CONFIGWIN_LABEL_IDLE_LINK_MINUTES ((toolbox_c) 0x170)
#define CONFIGWIN_LABEL_IDLE_PRINTER_MINUTES ((toolbox_c) 0x171)
#define CONFIGWIN_LABEL_IDLE_LINK_AFTER ((toolbox_c) 0x172)
#define CONFIGWIN_LABEL_IDLE_PRINTER_AFTER ((toolbox_c) 0x173)
#define CONFIGWIN_LABEL_PRINT_PREVIEW ((toolbox_c) 0x182)
#define CONFIGWIN_LABEL_PRINT_PREVIEW_PERCENT ((toolbox_c) 0x183)

// Options for automatically opening and closing filer windows
#define CONFIGWIN_AUTO_OPEN_MANUAL (0)
#define CONFIGWIN_AUTO_OPEN_CONNECT (1)
#define CONFIGWIN_AUTO_CLOSE_MANUAL (0)
#define CONFIGWIN_AUTO_CLOSE_KILL (1)
#define CONFIGWIN_AUTO_CLOSE_DISCONNECT (2)

// Options for iconbar icons
#define CONFIGWIN_ICON_NO_ROM (0)
#define CONFIGWIN_ICON_ALL (1)
#define CONFIGWIN_ICON_SINGLE (2)

// Options for power monitoring
#define CONFIGWIN_POWER_NEVER (0)
#define CONFIGWIN_POWER_NO_EXTERNAL (1)
#define CONFIGWIN_POWER_ALWAYS (2)

// Options for file transfer intercepts
#define CONFIGWIN_RUN_NEVER (0)
#define CONFIGWIN_RUN_UNCLAIMED (1)
#define CONFIGWIN_RUN_ALWAYS (2)
#define CONFIGWIN_LOAD_NEVER (0)
#define CONFIGWIN_LOAD_ALWAYS (1)
#define CONFIGWIN_SAVE_NEVER (0)
#define CONFIGWIN_SAVE_FILER (1)
#define CONFIGWIN_SAVE_ALWAYS (2)

// Options for clipboard polling
#define CONFIGWIN_POLL_NEVER (0)
#define CONFIGWIN_POLL_SIZE (1)
#define CONFIGWIN_POLL_CONTENT (2)

// Options for print jobs
#define CONFIGWIN_PRINT_AUTO_CONTROL (0)
#define CONFIGWIN_PRINT_AUTO_PREVIEW (1)
#define CONFIGWIN_PRINT_AUTO_PRINT (2)
#define CONFIGWIN_PRINT_WAIT_PAGE (0)
#define CONFIGWIN_PRINT_WAIT_JOB (1)

// Default backup directory leafname
#define CONFIGWIN_BACKUP_LEAF "Backups"

// Scaling for custom battery voltages
#define CONFIGWIN_VOLTAGE_SCALE (10)

// Scaling for idle timeouts
#define CONFIGWIN_IDLE_SCALE (60)

// The object ID of the configuration window
static toolbox_o configwin_obj = toolbox_NULL_OBJECT;

// The reference of the last message sent
static int configwin_ref = 0;

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Update the configuration window.
*/
static bool configwin_update(bits event_code, toolbox_action *action,
                             toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    list<string> menu;
    bool found;

    // Build a list of block drivers
    fs_leafname file;
    int context = 0;
    while (!blockdrive_enumerate(&context, file, sizeof(file), &found) && found)
    {
        menu.push_back(file);
    }
    stringset_c name(CONFIGWIN_DRIVER, configwin_obj);
    name.set_available(menu);

    // Attempt to load the selected block driver
    blockdrive_driver driver;
    bits ports;
    found = !blockdrive_load(name.gadget_w_string::get_value().c_str(),
                             &driver, &ports);

    // Set the range of ports for the selected driver
    numberrange_c port(CONFIGWIN_PORT, configwin_obj);
    if (found)
    {
        port.set_upper_bound_scaled(ports - 1);
        if (ports <= port()) port = ports - 1;
    }
    else port = 0;
    port.set_faded(!found);

    // Update the baud rate gadget
    blockdrive_speeds speeds;
    if ((config_current.exist(config_tag_restrict_baud)
         && !config_current.get_bool(config_tag_restrict_baud))
        || !baud_list(driver, baud_recommended, 0, speeds))
    {
        baud_list(driver, NULL, 0, speeds);
    }
    menu.clear();
    for (bits i = 0; found && speeds[i]; i++)
    {
        ostringstream str;
        str << speeds[i];
        menu.push_back(str.str());
    }
    stringset_c baud(CONFIGWIN_BAUD, configwin_obj);
    baud.set_available(menu);
    if (!found || !driver->speeds[0]) baud = "";
    else if (baud.gadget_w_number::get_value() == -1)
    {
        baud = 0;
    }
    baud.set_faded(!found);

    // Update the options gadget
    writablefield_c options(CONFIGWIN_OPTIONS, configwin_obj);
    if (!found) options = "";
    options.set_faded(!found);

    // Update the automatic baud rate gadget
    optionbutton_c autobaud(CONFIGWIN_AUTO_BAUD, configwin_obj);
    autobaud.set_faded(!found);

    // Unload the driver
    if (found) blockdrive_unload(driver);

    // Update the battery checking options
    bool custom = optionbutton_c(CONFIGWIN_POWER_CUSTOM, configwin_obj)();
    numberrange_c main_dead(CONFIGWIN_POWER_MAIN_DEAD, configwin_obj);
    numberrange_c main_very_low(CONFIGWIN_POWER_MAIN_VERY_LOW, configwin_obj);
    numberrange_c main_low(CONFIGWIN_POWER_MAIN_LOW, configwin_obj);
    numberrange_c backup_dead(CONFIGWIN_POWER_BACKUP_DEAD, configwin_obj);
    numberrange_c backup_very_low(CONFIGWIN_POWER_BACKUP_VERY_LOW, configwin_obj);
    numberrange_c backup_low(CONFIGWIN_POWER_BACKUP_LOW, configwin_obj);
    label_c(CONFIGWIN_LABEL_POWER_DEAD, configwin_obj).set_faded(!custom);
    label_c(CONFIGWIN_LABEL_POWER_VERY_LOW, configwin_obj).set_faded(!custom);
    label_c(CONFIGWIN_LABEL_POWER_LOW, configwin_obj).set_faded(!custom);
    label_c(CONFIGWIN_LABEL_POWER_MAIN, configwin_obj).set_faded(!custom);
    label_c(CONFIGWIN_LABEL_POWER_BACKUP, configwin_obj).set_faded(!custom);
    main_dead.set_faded(!custom);
    main_very_low.set_faded(!custom);
    main_low.set_faded(!custom);
    backup_dead.set_faded(!custom);
    backup_very_low.set_faded(!custom);
    backup_low.set_faded(!custom);
    if (id_block && (id_block->this_cmp == CONFIGWIN_POWER_MAIN_DEAD) && (main_very_low() < main_dead())) main_very_low = main_dead();
    if (id_block && ((id_block->this_cmp == CONFIGWIN_POWER_MAIN_DEAD) || (id_block->this_cmp == CONFIGWIN_POWER_MAIN_VERY_LOW)) && (main_low() < main_very_low())) main_low = main_very_low();
    if (id_block && (id_block->this_cmp == CONFIGWIN_POWER_MAIN_LOW) && (main_low() < main_very_low())) main_very_low = main_low();
    if (id_block && ((id_block->this_cmp == CONFIGWIN_POWER_MAIN_VERY_LOW) || (id_block->this_cmp == CONFIGWIN_POWER_MAIN_LOW)) && (main_very_low() < main_dead())) main_dead = main_very_low();
    if (id_block && (id_block->this_cmp == CONFIGWIN_POWER_BACKUP_DEAD) && (backup_very_low() < backup_dead())) backup_very_low = backup_dead();
    if (id_block && ((id_block->this_cmp == CONFIGWIN_POWER_BACKUP_DEAD) || (id_block->this_cmp == CONFIGWIN_POWER_BACKUP_VERY_LOW)) && (backup_low() < backup_very_low())) backup_low = backup_very_low();
    if (id_block && (id_block->this_cmp == CONFIGWIN_POWER_BACKUP_LOW) && (backup_low() < backup_very_low())) backup_very_low = backup_low();
    if (id_block && ((id_block->this_cmp == CONFIGWIN_POWER_BACKUP_VERY_LOW) || (id_block->this_cmp == CONFIGWIN_POWER_BACKUP_LOW)) && (backup_very_low() < backup_dead())) backup_dead = backup_very_low();

    // Update the file intercept options
    bool never;
    never = stringset_c(CONFIGWIN_INTERCEPT_RUN, configwin_obj).gadget_w_number::get_value() == CONFIGWIN_RUN_NEVER;
    optionbutton_c run_auto(CONFIGWIN_INTERCEPT_RUN_AUTO, configwin_obj);
    run_auto.set_faded(never);
    if (never) run_auto = FALSE;
    never = stringset_c(CONFIGWIN_INTERCEPT_LOAD, configwin_obj).gadget_w_number::get_value() == CONFIGWIN_LOAD_NEVER;
    optionbutton_c load_auto(CONFIGWIN_INTERCEPT_LOAD_AUTO, configwin_obj);
    load_auto.set_faded(never);
    if (never) load_auto = FALSE;
    never = stringset_c(CONFIGWIN_INTERCEPT_SAVE, configwin_obj).gadget_w_number::get_value() == CONFIGWIN_SAVE_NEVER;
    optionbutton_c save_auto(CONFIGWIN_INTERCEPT_SAVE_AUTO, configwin_obj);
    save_auto.set_faded(never);
    if (never) save_auto = FALSE;

    // Update the clipboard options
    bool clipboard = optionbutton_c(CONFIGWIN_CLIPBOARD_INTEGRATE, configwin_obj)();
    stringset_c(CONFIGWIN_CLIPBOARD_POLL, configwin_obj).set_faded(!clipboard);
    label_c(CONFIGWIN_LABEL_CLIPBOARD_POLL, configwin_obj).set_faded(!clipboard);

    // Update the idle options
    bool idle_link = optionbutton_c(CONFIGWIN_IDLE_LINK, configwin_obj)();
    label_c(CONFIGWIN_LABEL_IDLE_LINK_AFTER, configwin_obj).set_faded(!idle_link);
    numberrange_c(CONFIGWIN_IDLE_LINK_MINUTES, configwin_obj).set_faded(!idle_link);
    label_c(CONFIGWIN_LABEL_IDLE_LINK_MINUTES, configwin_obj).set_faded(!idle_link);
    bool idle_printer = optionbutton_c(CONFIGWIN_IDLE_PRINTER, configwin_obj)();
    label_c(CONFIGWIN_LABEL_IDLE_PRINTER_AFTER, configwin_obj).set_faded(!idle_printer);
    numberrange_c(CONFIGWIN_IDLE_PRINTER_MINUTES, configwin_obj).set_faded(!idle_printer);
    label_c(CONFIGWIN_LABEL_IDLE_PRINTER_MINUTES, configwin_obj).set_faded(!idle_printer);

    // Update the print job options
    bool print;
    print = stringset_c(CONFIGWIN_PRINT_AUTO, configwin_obj).gadget_w_number::get_value() == CONFIGWIN_PRINT_AUTO_PRINT;
    label_c(CONFIGWIN_LABEL_PRINT_PREVIEW, configwin_obj).set_faded(print);
    numberrange_c(CONFIGWIN_PRINT_PREVIEW, configwin_obj).set_faded(print);
    label_c(CONFIGWIN_LABEL_PRINT_PREVIEW_PERCENT, configwin_obj).set_faded(print);
    optionbutton_c(CONFIGWIN_PRINT_ANTIALIAS, configwin_obj).set_faded(print);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle drags from the configuration window.
*/
static bool configwin_drag_end(bits event_code, toolbox_action *action,
                               toolbox_block *id_block, void *handle)
{
    draggable_action_drag_ended *drag = (draggable_action_drag_ended *) &action->data;
    wimp_message message;

    NOT_USED(event_code);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Choose the leaf name of the directory
    string leaf(writablefield_c(CONFIGWIN_BACKUP_PATH, configwin_obj)());
    string::size_type pos = leaf.find_last_of(FS_CHAR_SEPARATOR);
    if (pos != string::npos) leaf.erase(0, pos + 1);
    if (leaf.empty()) leaf = CONFIGWIN_BACKUP_LEAF;

    // Send a Message_DataSave to find the directory name
    message.your_ref = 0;
    message.action = message_DATA_SAVE;
    message.data.data_xfer.w = drag->ids.wimp.w;
    message.data.data_xfer.i = drag->ids.wimp.i;
    message.data.data_xfer.pos = drag->pos;
    message.data.data_xfer.est_size = 0;
    message.data.data_xfer.file_type = osfile_TYPE_DIR;
    strcpy(message.data.data_xfer.file_name, leaf.c_str());
    message.size = ALIGN(strchr(message.data.data_xfer.file_name, '\0')
                         - (char *) &message + 1);
    wimp_send_message_to_window(wimp_USER_MESSAGE, &message,
                                drag->ids.wimp.w, drag->ids.wimp.i);
    configwin_ref = message.my_ref;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSaveAck wimp message events.
*/
static int configwin_drag_ack(wimp_message *message, void *handle)
{
    bool claimed = FALSE;

    NOT_USED(handle);

    // Only process message if suitable
    if ((message->your_ref == configwin_ref)
        && (message->data.data_xfer.est_size == 0)
        && *message->data.data_xfer.file_name)
    {
        // Set the path name field
        claimed = TRUE;
        writablefield_c(CONFIGWIN_BACKUP_PATH, configwin_obj) = message->data.data_xfer.file_name;
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
static int configwin_drag_load(wimp_message *message, void *handle)
{
    bool claimed = FALSE;
    toolbox_o obj;
    toolbox_c cmp;

    NOT_USED(handle);

    // Convert the details to toolbox identifiers
    window_wimp_to_toolbox(0, message->data.data_xfer.w,
                           message->data.data_xfer.i, &obj, &cmp);

    // Only process message if suitable
    if ((message->your_ref == 0)
        && (obj == configwin_obj)
        && ((cmp == CONFIGWIN_BACKUP_PATH)
            || (cmp == CONFIGWIN_BACKUP_DROP))
        && (message->data.data_xfer.file_type == osfile_TYPE_DIR)
        && *message->data.data_xfer.file_name)
    {
        // Set the path name field
        claimed = TRUE;
        writablefield_c(CONFIGWIN_BACKUP_PATH, configwin_obj) = message->data.data_xfer.file_name;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : obj       - The object ID of the window.
                  config    - The configuration to write.
    Returns     : void
    Description : Write the configuration to the window.
*/
static void configwin_write(toolbox_o obj, const config_store &config)
{
    // Set the values in the window
    if (config.exist(config_tag_driver)) stringset_c(CONFIGWIN_DRIVER, obj) = config.get_str(config_tag_driver);
    if (config.exist(config_tag_port)) numberrange_c(CONFIGWIN_PORT, obj) = config.get_num(config_tag_port);
    if (config.exist(config_tag_baud)) stringset_c(CONFIGWIN_BAUD, obj) = config.get_str(config_tag_baud);
    if (config.exist(config_tag_options)) writablefield_c(CONFIGWIN_OPTIONS, obj) = config.get_str(config_tag_options);
    if (config.exist(config_tag_auto_baud)) optionbutton_c(CONFIGWIN_AUTO_BAUD, obj) = config.get_bool(config_tag_auto_baud);
    stringset_c(CONFIGWIN_AUTO_OPEN, obj) =
        config.get_bool(config_tag_open_link)
        ? CONFIGWIN_AUTO_OPEN_CONNECT
        : CONFIGWIN_AUTO_OPEN_MANUAL;
    stringset_c(CONFIGWIN_AUTO_CLOSE, obj) =
        config.get_bool(config_tag_close_link)
        ? CONFIGWIN_AUTO_CLOSE_DISCONNECT
        : (config.get_bool(config_tag_close_kill)
           ? CONFIGWIN_AUTO_CLOSE_KILL
           : CONFIGWIN_AUTO_CLOSE_MANUAL);
    stringset_c(CONFIGWIN_ICON, obj) =
        config.get_bool(config_tag_show_all)
        ? CONFIGWIN_ICON_SINGLE
        : (config.get_bool(config_tag_show_rom)
           ? CONFIGWIN_ICON_ALL
           : CONFIGWIN_ICON_NO_ROM);
    if (config.exist(config_tag_print)) stringset_c(CONFIGWIN_PRINTER, obj) = config.get_str(config_tag_print);
    if (config.exist(config_tag_backup_path)) writablefield_c(CONFIGWIN_BACKUP_PATH, obj) = config.get_str(config_tag_backup_path);
    if (config.exist(config_tag_backup_new)) optionbutton_c(CONFIGWIN_BACKUP_NEW, obj) = config.get_bool(config_tag_backup_new);
    if (config.exist(config_tag_sync_clocks)) optionbutton_c(CONFIGWIN_SYNC_CLOCKS, obj) = config.get_bool(config_tag_sync_clocks);
    stringset_c(CONFIGWIN_POWER_MONITOR, obj) =
        config.get_bool(config_tag_power_monitor)
        ? (config.get_bool(config_tag_power_external)
           ? CONFIGWIN_POWER_ALWAYS
           : CONFIGWIN_POWER_NO_EXTERNAL)
        : CONFIGWIN_POWER_NEVER;
    if (config.exist(config_tag_power_custom)) optionbutton_c(CONFIGWIN_POWER_CUSTOM, obj) = config.get_bool(config_tag_power_custom);
    if (config.exist(config_tag_power_main_dead)) numberrange_c(CONFIGWIN_POWER_MAIN_DEAD, obj) = config.get_num(config_tag_power_main_dead) / CONFIGWIN_VOLTAGE_SCALE;
    if (config.exist(config_tag_power_main_very_low)) numberrange_c(CONFIGWIN_POWER_MAIN_VERY_LOW, obj) = config.get_num(config_tag_power_main_very_low) / CONFIGWIN_VOLTAGE_SCALE;
    if (config.exist(config_tag_power_main_low)) numberrange_c(CONFIGWIN_POWER_MAIN_LOW, obj) = config.get_num(config_tag_power_main_low) / CONFIGWIN_VOLTAGE_SCALE;
    if (config.exist(config_tag_power_backup_dead)) numberrange_c(CONFIGWIN_POWER_BACKUP_DEAD, obj) = config.get_num(config_tag_power_backup_dead) / CONFIGWIN_VOLTAGE_SCALE;
    if (config.exist(config_tag_power_backup_very_low)) numberrange_c(CONFIGWIN_POWER_BACKUP_VERY_LOW, obj) = config.get_num(config_tag_power_backup_very_low) / CONFIGWIN_VOLTAGE_SCALE;
    if (config.exist(config_tag_power_backup_low)) numberrange_c(CONFIGWIN_POWER_BACKUP_LOW, obj) = config.get_num(config_tag_power_backup_low) / CONFIGWIN_VOLTAGE_SCALE;
    stringset_c(CONFIGWIN_INTERCEPT_RUN, obj) =
        config.get_bool(config_tag_intercept_run)
        ? (config.get_bool(config_tag_intercept_run_all)
           ? CONFIGWIN_RUN_ALWAYS
           : CONFIGWIN_RUN_UNCLAIMED)
        : CONFIGWIN_RUN_NEVER;
    if (config.exist(config_tag_intercept_run_auto)) optionbutton_c(CONFIGWIN_INTERCEPT_RUN_AUTO, obj) = config.get_bool(config_tag_intercept_run_auto);
    stringset_c(CONFIGWIN_INTERCEPT_LOAD, obj) =
        config.get_bool(config_tag_intercept_load)
        ? CONFIGWIN_LOAD_ALWAYS
        : CONFIGWIN_LOAD_NEVER;
    if (config.exist(config_tag_intercept_load_auto)) optionbutton_c(CONFIGWIN_INTERCEPT_LOAD_AUTO, obj) = config.get_bool(config_tag_intercept_load_auto);
    stringset_c(CONFIGWIN_INTERCEPT_SAVE, obj) =
        config.get_bool(config_tag_intercept_save)
        ? (config.get_bool(config_tag_intercept_transfer)
           ? CONFIGWIN_SAVE_ALWAYS
           : CONFIGWIN_SAVE_FILER)
        : CONFIGWIN_SAVE_NEVER;
    if (config.exist(config_tag_intercept_save_auto)) optionbutton_c(CONFIGWIN_INTERCEPT_SAVE_AUTO, obj) = config.get_bool(config_tag_intercept_save_auto);
    if (config.exist(config_tag_clipboard_integrate)) optionbutton_c(CONFIGWIN_CLIPBOARD_INTEGRATE, obj) = config.get_bool(config_tag_clipboard_integrate);
    stringset_c(CONFIGWIN_CLIPBOARD_POLL, obj) =
        config.get_bool(config_tag_clipboard_poll_disabled)
        ? CONFIGWIN_POLL_NEVER
        : (config.get_bool(config_tag_clipboard_poll_content)
           ? CONFIGWIN_POLL_CONTENT
           : CONFIGWIN_POLL_SIZE);
    if (config.exist(config_tag_idle_link))
    {
        bits idle_link = config.get_num(config_tag_idle_link);
        optionbutton_c(CONFIGWIN_IDLE_LINK, obj) = 0 < idle_link;
        if (0 < idle_link) numberrange_c(CONFIGWIN_IDLE_LINK_MINUTES, obj) = idle_link / CONFIGWIN_IDLE_SCALE;
    }
    if (config.exist(config_tag_idle_printer))
    {
        bits idle_printer = config.get_num(config_tag_idle_printer);
        optionbutton_c(CONFIGWIN_IDLE_PRINTER, obj) = 0 < idle_printer;
        if (0 < idle_printer) numberrange_c(CONFIGWIN_IDLE_PRINTER_MINUTES, obj) = idle_printer / CONFIGWIN_IDLE_SCALE;
    }
    if (config.exist(config_tag_idle_background)) optionbutton_c(CONFIGWIN_IDLE_BACKGROUND, obj) = config.get_bool(config_tag_idle_background);
    stringset_c(CONFIGWIN_PRINT_AUTO, obj) =
        config.get_bool(config_tag_print_auto_print)
        ? CONFIGWIN_PRINT_AUTO_PRINT
        : config.get_bool(config_tag_print_auto_preview)
          ? CONFIGWIN_PRINT_AUTO_PREVIEW
          : CONFIGWIN_PRINT_AUTO_CONTROL;
    stringset_c(CONFIGWIN_PRINT_WAIT, obj) =
        config.get_bool(config_tag_print_print_wait)
        ? CONFIGWIN_PRINT_WAIT_JOB
        : CONFIGWIN_PRINT_WAIT_PAGE;
    if (config.exist(config_tag_print_preview_scale)) numberrange_c(CONFIGWIN_PRINT_PREVIEW, obj) = config.get_num(config_tag_print_preview_scale);
    if (config.exist(config_tag_print_preview_antialias)) optionbutton_c(CONFIGWIN_PRINT_ANTIALIAS, obj) = config.get_bool(config_tag_print_preview_antialias);
}

/*
    Parameters  : obj       - The object ID of the window.
                  config    - The configuration to read.
    Returns     : void
    Description : Read the configuration from the window.
*/
static void configwin_read(toolbox_o obj, config_store &config)
{
    // Start with the active configuration
    config = config_current;

    // Overwrite with values from the window
    config.set_str(config_tag_driver, stringset_c(CONFIGWIN_DRIVER, obj).gadget_w_string::get_value());
    config.set_num(config_tag_port, numberrange_c(CONFIGWIN_PORT, obj)());
    config.set_str(config_tag_baud, stringset_c(CONFIGWIN_BAUD, obj).gadget_w_string::get_value());
    config.set_str(config_tag_options, writablefield_c(CONFIGWIN_OPTIONS, obj)());
    config.set_bool(config_tag_auto_baud, optionbutton_c(CONFIGWIN_AUTO_BAUD, obj)());
    switch (stringset_c(CONFIGWIN_AUTO_OPEN, obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_AUTO_OPEN_MANUAL:
            config.set_bool(config_tag_open_link, FALSE);
            break;

        case CONFIGWIN_AUTO_OPEN_CONNECT:
            config.set_bool(config_tag_open_link, TRUE);
            break;
    }
    switch (stringset_c(CONFIGWIN_AUTO_CLOSE, obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_AUTO_CLOSE_MANUAL:
            config.set_bool(config_tag_close_kill, FALSE);
            config.set_bool(config_tag_close_link, FALSE);
            break;

        case CONFIGWIN_AUTO_CLOSE_KILL:
            config.set_bool(config_tag_close_kill, TRUE);
            config.set_bool(config_tag_close_link, FALSE);
            break;

        case CONFIGWIN_AUTO_CLOSE_DISCONNECT:
            config.set_bool(config_tag_close_kill, TRUE);
            config.set_bool(config_tag_close_link, TRUE);
            break;
    }
    switch (stringset_c(CONFIGWIN_ICON, obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_ICON_NO_ROM:
            config.set_bool(config_tag_show_rom, FALSE);
            config.set_bool(config_tag_show_all, FALSE);
            break;

        case CONFIGWIN_ICON_ALL:
            config.set_bool(config_tag_show_rom, TRUE);
            config.set_bool(config_tag_show_all, FALSE);
            break;

        case CONFIGWIN_ICON_SINGLE:
            config.set_bool(config_tag_show_rom, TRUE);
            config.set_bool(config_tag_show_all, TRUE);
            break;
    }
    config.set_str(config_tag_print, stringset_c(CONFIGWIN_PRINTER, obj).gadget_w_string::get_value());
    config.set_str(config_tag_backup_path, writablefield_c(CONFIGWIN_BACKUP_PATH, obj)());
    config.set_bool(config_tag_backup_new, optionbutton_c(CONFIGWIN_BACKUP_NEW, obj)());
    config.set_bool(config_tag_sync_clocks, optionbutton_c(CONFIGWIN_SYNC_CLOCKS, obj)());
    switch (stringset_c(CONFIGWIN_POWER_MONITOR, obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_POWER_NEVER:
            config.set_bool(config_tag_power_monitor, FALSE);
            config.set_bool(config_tag_power_external, FALSE);
            break;

        case CONFIGWIN_POWER_NO_EXTERNAL:
            config.set_bool(config_tag_power_monitor, TRUE);
            config.set_bool(config_tag_power_external, FALSE);
            break;

        case CONFIGWIN_POWER_ALWAYS:
            config.set_bool(config_tag_power_monitor, TRUE);
            config.set_bool(config_tag_power_external, TRUE);
            break;
    }
    config.set_bool(config_tag_power_custom, optionbutton_c(CONFIGWIN_POWER_CUSTOM, obj)());
    config.set_num(config_tag_power_main_dead, numberrange_c(CONFIGWIN_POWER_MAIN_DEAD, obj)() * CONFIGWIN_VOLTAGE_SCALE);
    config.set_num(config_tag_power_main_very_low, numberrange_c(CONFIGWIN_POWER_MAIN_VERY_LOW, obj)() * CONFIGWIN_VOLTAGE_SCALE);
    config.set_num(config_tag_power_main_low, numberrange_c(CONFIGWIN_POWER_MAIN_LOW, obj)() * CONFIGWIN_VOLTAGE_SCALE);
    config.set_num(config_tag_power_backup_dead, numberrange_c(CONFIGWIN_POWER_BACKUP_DEAD, obj)() * CONFIGWIN_VOLTAGE_SCALE);
    config.set_num(config_tag_power_backup_very_low, numberrange_c(CONFIGWIN_POWER_BACKUP_VERY_LOW, obj)() * CONFIGWIN_VOLTAGE_SCALE);
    config.set_num(config_tag_power_backup_low, numberrange_c(CONFIGWIN_POWER_BACKUP_LOW, obj)() * CONFIGWIN_VOLTAGE_SCALE);

    switch (stringset_c(CONFIGWIN_INTERCEPT_RUN, configwin_obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_RUN_NEVER:
            config.set_bool(config_tag_intercept_run, FALSE);
            config.set_bool(config_tag_intercept_run_all, FALSE);
            break;

        case CONFIGWIN_RUN_UNCLAIMED:
            config.set_bool(config_tag_intercept_run, TRUE);
            config.set_bool(config_tag_intercept_run_all, FALSE);
            break;

        case CONFIGWIN_RUN_ALWAYS:
            config.set_bool(config_tag_intercept_run, TRUE);
            config.set_bool(config_tag_intercept_run_all, TRUE);
            break;
    }
    config.set_bool(config_tag_intercept_run_auto, optionbutton_c(CONFIGWIN_INTERCEPT_RUN_AUTO, configwin_obj)());
    switch (stringset_c(CONFIGWIN_INTERCEPT_LOAD, configwin_obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_LOAD_NEVER:
            config.set_bool(config_tag_intercept_load, FALSE);
            break;

        case CONFIGWIN_LOAD_ALWAYS:
            config.set_bool(config_tag_intercept_load, TRUE);
            break;
    }
    config.set_bool(config_tag_intercept_load_auto, optionbutton_c(CONFIGWIN_INTERCEPT_LOAD_AUTO, configwin_obj)());
    switch (stringset_c(CONFIGWIN_INTERCEPT_SAVE, configwin_obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_SAVE_NEVER:
            config.set_bool(config_tag_intercept_save, FALSE);
            config.set_bool(config_tag_intercept_transfer, FALSE);
            break;

        case CONFIGWIN_SAVE_FILER:
            config.set_bool(config_tag_intercept_save, TRUE);
            config.set_bool(config_tag_intercept_transfer, FALSE);
            break;

        case CONFIGWIN_SAVE_ALWAYS:
            config.set_bool(config_tag_intercept_save, TRUE);
            config.set_bool(config_tag_intercept_transfer, TRUE);
            break;
    }
    config.set_bool(config_tag_intercept_save_auto, optionbutton_c(CONFIGWIN_INTERCEPT_SAVE_AUTO, configwin_obj)());
    config.set_bool(config_tag_clipboard_integrate, optionbutton_c(CONFIGWIN_CLIPBOARD_INTEGRATE, configwin_obj)());
    switch (stringset_c(CONFIGWIN_CLIPBOARD_POLL, configwin_obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_POLL_NEVER:
            config.set_bool(config_tag_clipboard_poll_disabled, TRUE);
            config.set_bool(config_tag_clipboard_poll_content, FALSE);
            break;

        case CONFIGWIN_POLL_SIZE:
            config.set_bool(config_tag_clipboard_poll_disabled, FALSE);
            config.set_bool(config_tag_clipboard_poll_content, FALSE);
            break;

        case CONFIGWIN_POLL_CONTENT:
            config.set_bool(config_tag_clipboard_poll_disabled, FALSE);
            config.set_bool(config_tag_clipboard_poll_content, TRUE);
            break;
    }
    config.set_num(config_tag_idle_link, optionbutton_c(CONFIGWIN_IDLE_LINK, obj)() ? numberrange_c(CONFIGWIN_IDLE_LINK_MINUTES, obj)() * CONFIGWIN_IDLE_SCALE : 0);
    config.set_num(config_tag_idle_printer, optionbutton_c(CONFIGWIN_IDLE_PRINTER, obj)() ? numberrange_c(CONFIGWIN_IDLE_PRINTER_MINUTES, obj)() * CONFIGWIN_IDLE_SCALE : 0);
    config.set_bool(config_tag_idle_background, optionbutton_c(CONFIGWIN_IDLE_BACKGROUND, obj)());
    switch (stringset_c(CONFIGWIN_PRINT_AUTO, configwin_obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_PRINT_AUTO_CONTROL:
            config.set_bool(config_tag_print_auto_print, FALSE);
            config.set_bool(config_tag_print_auto_preview, FALSE);
            break;

        case CONFIGWIN_PRINT_AUTO_PREVIEW:
            config.set_bool(config_tag_print_auto_print, FALSE);
            config.set_bool(config_tag_print_auto_preview, TRUE);
            break;

        case CONFIGWIN_PRINT_AUTO_PRINT:
            config.set_bool(config_tag_print_auto_print, TRUE);
            config.set_bool(config_tag_print_auto_preview, FALSE);
            break;
    }
    switch (stringset_c(CONFIGWIN_PRINT_WAIT, configwin_obj).gadget_w_number::get_value())
    {
        case CONFIGWIN_PRINT_WAIT_PAGE:
            config.set_bool(config_tag_print_print_wait, FALSE);
            break;

        case CONFIGWIN_PRINT_WAIT_JOB:
            config.set_bool(config_tag_print_print_wait, TRUE);
            break;
    }
    config.set_num(config_tag_print_preview_scale, numberrange_c(CONFIGWIN_PRINT_PREVIEW, obj)());
    config.set_bool(config_tag_print_preview_antialias, optionbutton_c(CONFIGWIN_PRINT_ANTIALIAS, obj)());
}

/*
    Parameters  : obj       - The object ID of the window.
    Returns     : void
    Description : Reset the configuration displayed in the window.
*/
static void configwin_reset(toolbox_o obj)
{
    config_store config(config_current);

    // Merge the current configuration with the active configuration
    config.read();

    // Write the configuration to the window
    configwin_write(obj, config);

    // Fake an update
    configwin_update(0, NULL, NULL, NULL);
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Initialise when the configuration window is shown.
*/
bool configwin_show(bits event_code, toolbox_action *action,
                    toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if the window is already open
    if (configwin_obj == toolbox_NULL_OBJECT)
    {
        // Store the object ID
        configwin_obj = id_block->this_obj;

        // Register a handler to update the window
        event_register_toolbox_handler(configwin_obj,
                                       action_STRING_SET_VALUE_CHANGED,
                                       configwin_update, NULL);
        event_register_toolbox_handler(configwin_obj,
                                       action_NUMBER_RANGE_VALUE_CHANGED,
                                       configwin_update, NULL);
        event_register_toolbox_handler(configwin_obj,
                                       action_OPTION_BUTTON_STATE_CHANGED,
                                       configwin_update, NULL);

        // Register handlers to handle drag and drop path setting
        event_register_toolbox_handler(configwin_obj,
                                       action_DRAGGABLE_DRAG_ENDED,
                                       configwin_drag_end, NULL);
        event_register_message_handler(message_DATA_SAVE_ACK,
                                       configwin_drag_ack, NULL);
        event_register_message_handler(message_DATA_LOAD,
                                       configwin_drag_load, NULL);

        // Reset the displayed configuration
        configwin_reset(configwin_obj);
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
    Description : Tidy up when the configuration window is closed.
*/
bool configwin_hide(bits event_code, toolbox_action *action,
                    toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Unregister handler used to update the window
    event_deregister_toolbox_handler(configwin_obj,
                                     action_STRING_SET_VALUE_CHANGED,
                                     configwin_update, NULL);
    event_deregister_toolbox_handler(configwin_obj,
                                     action_NUMBER_RANGE_VALUE_CHANGED,
                                     configwin_update, NULL);
    event_deregister_toolbox_handler(configwin_obj,
                                     action_OPTION_BUTTON_STATE_CHANGED,
                                     configwin_update, NULL);

    // Unregister the handlers to handle drag and drop path setting
    event_deregister_toolbox_handler(configwin_obj,
                                     action_DRAGGABLE_DRAG_ENDED,
                                     configwin_drag_end, NULL);
    event_deregister_message_handler(message_DATA_SAVE_ACK,
                                     configwin_drag_ack, NULL);
    event_deregister_message_handler(message_DATA_LOAD,
                                     configwin_drag_load, NULL);

    // Clear the object ID
    configwin_obj = toolbox_NULL_OBJECT;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : src   - The source directory.
                  dest  - The destination directory.
    Returns     : void
    Description : Attempt to move the backup directory.
*/
static void configwin_move(const char *src, const char *dest)
{
    fileswitch_object_type src_type;
    fileswitch_object_type dest_type;

    // Only move if the source exists and the destination does not
    if (!xosfile_read_stamped_no_path(src, &src_type,
                                      NULL, NULL, NULL, NULL, NULL)
        && !xosfile_read_stamped_no_path(dest, &dest_type,
                                         NULL, NULL, NULL, NULL, NULL)
        && (src_type == fileswitch_IS_DIR)
        && (dest_type == fileswitch_NOT_FOUND))
    {
        // Attempt to rename the directory first
        if (xosfscontrol_rename(src, dest))
        {
            // Last resort is a copy with delete
            hourglass_on();
            if (!xosfscontrol_copy(src, dest, osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE | osfscontrol_COPY_DELETE | osfscontrol_COPY_LOOK, 0, 0, 0, 0, NULL))
            {
                // Delete any remaining source files if successful
                xosfscontrol_wipe(src, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
            }
            hourglass_off();
        }
    }
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the OK action button.
*/
bool configwin_action(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Copy the previous backup directory
    string prev_backup(config_current.get_str(config_tag_backup_path));

    // Read the configuration from the window
    configwin_read(configwin_obj, config_current);

    // Use the configuration
    config_current.write(TRUE);
    update_check(FALSE);

    // Close the window if appropriate
    if (!(action->flags & actionbutton_SELECTED_ADJUST))
    {
        toolbox_hide_object(0, configwin_obj);
    }

    // Move the backup directory if appropriate
    if (config_current.exist(config_tag_backup_path))
    {
        configwin_move(prev_backup.c_str(),
                       config_current.get_str(config_tag_backup_path).c_str());
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
    Description : Handle the cancel action button.
*/
bool configwin_cancel(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Either close the window or reset the gadgets
    if (action->flags & actionbutton_SELECTED_ADJUST)
    {
        // Reset the displayed configuration
        configwin_reset(configwin_obj);
    }
    else
    {
        // Close the window
        toolbox_hide_object(0, configwin_obj);
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
    Description : Handle the save action button.
*/
bool configwin_save(bits event_code, toolbox_action *action,
                    toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    config_store config;

    // Read the configuration from the window
    configwin_read(configwin_obj, config);

    // Enable simple error handling
    filer_error_allowed++;

    // Save the settings
    config.save();

    // Restore normal error handling
    filer_error_allowed--;

    // Claim the event
    return TRUE;
}
