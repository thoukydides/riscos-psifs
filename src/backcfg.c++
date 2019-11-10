/*
    File        : backcfg.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Backup configuration window handling for the PsiFS
                  filer.

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
#include "backcfg.h"

// Include clib header files
#include <ctype.h>
#include <stdio.h>

// Include oslib header files
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"
#include "oslib/osgbpb.h"
#include "oslib/osword.h"
#include "oslib/territory.h"
#include "oslib/window.h"
#include "event.h"

// Include cathlibcpp header files
#include "fstream.h"
#include "list.h"
#include "sstream.h"

// Include alexlib header files
#include "actionbutton_c.h"
#include "button_c.h"
#include "displayfield_c.h"
#include "numberrange_c.h"
#include "optionbutton_c.h"
#include "radiobutton_c.h"
#include "writablefield_c.h"

// Include project header files
#include "backobj.h"
#include "config.h"
#include "filer.h"
#include "fs.h"

// Backup configuration file tags
const char backcfg_always[] = "AlwaysUseSettings";
const char backcfg_auto[] = "AutomaticBackupEnabled";
const char backcfg_auto_interval[] = "AutomaticBackupInterval";
const char backcfg_versions[] = "ArchiveVersions";
const char backcfg_changes[] = "ArchiveChanges";
const char backcfg_last_time[] = "LastBackupTime";
const char backcfg_last_partial[] = "LastBackupPartial";
const char backcfg_name[] = "Backup";
const char backcfg_backup_disc[] = "BackupDiscName";
const char backcfg_backup_drive[] = "BackupDriveLetter";
const char backcfg_backup_media[] = "BackupMediaUID";
const char backcfg_backup_machine_low[] = "BackupMachineUIDLow";
const char backcfg_backup_machine_high[] = "BackupMachineUIDHigh";
const char backcfg_ignore_disc[] = "IgnoreDiscName";
const char backcfg_ignore_drive[] = "IgnoreDriveLetter";
const char backcfg_ignore_machine[] = "IgnoreMachineUID";

// The default backup directory
#define BACKCFG_DEFAULT_DIR "<PsiFS$Dir>.^.Backups"

// The backup configuration file name
#define BACKCFG_FILE_OPTIONS "Options"

// Backup file name bits
#define BACKCFG_FILE_CURRENT "Backup"
#define BACKCFG_FILE_FULL "Full"
#define BACKCFG_FILE_INCREMENTAL "Changes"
#define BACKCFG_FILE_SEPARATOR "-"
#define BACKCFG_MAX_VERSIONS (10)

// Prefix for unique configuration name and directory extensions
static const string backcfg_unique_ext(" ~ ");
#define BACKCFG_MAX_NAME (50)
#define BACKCFG_MAX_DIR (10)

// Backup configuration window gadgets
#define BACKCFG_SET ((toolbox_c) 0x00)
#define BACKCFG_CANCEL ((toolbox_c) 0x01)
#define BACKCFG_ADVANCED ((toolbox_c) 0x03)
#define BACKCFG_AUTO ((toolbox_c) 0x10)
#define BACKCFG_AUTO_EVERY ((toolbox_c) 0x11)
#define BACKCFG_AUTO_DAILY ((toolbox_c) 0x12)
#define BACKCFG_AUTO_WEEKLY ((toolbox_c) 0x13)
#define BACKCFG_VERSIONS ((toolbox_c) 0x20)
#define BACKCFG_CHANGES ((toolbox_c) 0x21)
#define BACKCFG_NAME ((toolbox_c) 0x30)
#define BACKCFG_ALWAYS ((toolbox_c) 0x31)
#define BACKCFG_DISC_IGNORE ((toolbox_c) 0x10)
#define BACKCFG_DISC ((toolbox_c) 0x11)
#define BACKCFG_DRIVE_IGNORE ((toolbox_c) 0x20)
#define BACKCFG_DRIVE ((toolbox_c) 0x21)
#define BACKCFG_MEDIA_IGNORE ((toolbox_c) 0x30)
#define BACKCFG_MEDIA ((toolbox_c) 0x31)
#define BACKCFG_MACHINE_IGNORE ((toolbox_c) 0x40)
#define BACKCFG_MACHINE ((toolbox_c) 0x41)

// Backup configuration warning window gadgets
#define BACKCFG_WARN_SET ((toolbox_c) 0x00)
#define BACKCFG_WARN_CANCEL ((toolbox_c) 0x01)
#define BACKCFG_WARN_TEXT ((toolbox_c) 0x10)
#define BACKCFG_WARN_IGNORE ((toolbox_c) 0x20)
#define BACKCFG_WARN_RENAME ((toolbox_c) 0x20)
#define BACKCFG_WARN_MOVE ((toolbox_c) 0x21)
#define BACKCFG_WARN_NEW ((toolbox_c) 0x22)
#define BACKCFG_WARN_NEW_NAME ((toolbox_c) 0x23)
#define BACKCFG_WARN_PREV ((toolbox_c) 0x24)
#define BACKCFG_WARN_RENAME_NAME ((toolbox_c) 0x25)
#define BACKCFG_WARN_NAME ((toolbox_c) 0x30)

// List of backup configuration objects
static list<backcfg_obj *> backcfg_list;

// Parent directory for all backup files
static string backcfg_dir;

/*
    Parameters  : date  - The date to convert.
    Returns     : bits  - The equivalent number of days.
    Description : Convert a date in UTC format to the number of days since
                  some fixed point.
*/
static bits backcfg_days(const date_riscos *date)
{
    territory_ordinals ordinals;

    // Start by converting to ordinals
    territory_convert_time_to_ordinals(territory_CURRENT, &date->bytes,
                                       &ordinals);

    // Return the number of days
    return (ordinals.year * 365) + ((ordinals.year - 1) / 4) + ordinals.yearday;
}

/*
    Parameters  : file  - The name of the file to delete.
    Returns     : void
    Description : Delete the specified file.
*/
static void backcfg_delete(string name)
{
    // Delete the file
    osfscontrol_wipe(name.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
}

/*
    Parameters  : src   - The name of the source file.
                  dest  - The name of the destination file.
    Returns     : void
    Description : Rename the specified file.
*/
static void backcfg_move(string src, string dest)
{
    // Start by attempting a rename
    if (xosfscontrol_rename(src.c_str(), dest.c_str()))
    {
        // Try a copy and delete if rename failed
        osfscontrol_copy(src.c_str(), dest.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE | osfscontrol_COPY_DELETE | osfscontrol_COPY_LOOK, 0, 0, 0, 0, NULL);

        // Delete any files left afterwards
        xosfscontrol_wipe(src.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
    }
}

/*
    Parameters  : name  - The name of the file or directory to check.
    Returns     : bool  - Does the file or directory exist.
    Description : Check for the existence of the specified file or directory.
*/
static bool backcfg_exist(string name)
{
    // Check if the file or directory exists
    os_fw handle;
    if (!xosfind_openinw(osfind_NO_PATH, name.c_str(), NULL, &handle))
    {
        if (handle) osfind_close(handle);
    }
    else handle = 0;

    // Return whether the file exists
    return handle != 0;
}

/*
    Parameters  : -
    Returns     : -
    Description : Constructor.
*/
backcfg_disc::backcfg_disc()
: disc(""), media(0), machine_low(0), machine_high(0)
{
    // No other initialisation required
}

/*
    Parameters  : drive - The drive to read the details for.
    Returns     : bool  - Is there a valid disc.
    Description : Read the details for the specified drive.
*/
bool backcfg_disc::read(psifs_drive drive)
{
    bool valid = FALSE;

    // Store the drive
    backcfg_disc::drive = drive;

    // Check if the drive is valid
    psifs_drive_status status = psifsget_drive_status(drive);
    if ((status & psifs_DRIVE_STATUS_PRESENT)
        && !(status & psifs_DRIVE_STATUS_ROM))
    {
        fs_discname name;

        // Mark the disc as valid
        valid = TRUE;

        // Attempt to read the drive name
        if ((psifsget_drive_name(drive, name, sizeof(name)) < 1)
            || (strlen(name) < 2))
        {
            // Construct a default name if necessary
            name[0] = drive;
            name[1] = '\0';
        }
        disc = name;

        // Attempt to read the disc unique identifier
        if (xpsifsget_drive_id(drive, &media) || (media == 0xffffffff))
        {
            media = 0;
        }

        // Attempt to read the machine unique identifier
        if (xpsifsget_machine_id_low(&machine_low)
            || xpsifsget_machine_id_high(&machine_high))
        {
            machine_low = machine_high = 0;
        }
    }
    else
    {
        // Set default values if not valid
        disc = "";
        media = machine_low = machine_high = 0;
    }

    // Return whether the disc is valid
    return valid;
}

/*
    Parameters  : lhs   - The first disc to compare.
                  rhs   - The second disc to compare.
    Returns     : bool  - Are the two discs the same.
    Description : Compare two discs.
*/
bool operator==(const backcfg_disc &lhs, const backcfg_disc &rhs)
{
    // Compare all of the fields
    return (lhs.drive == rhs.drive)
           && (lhs.disc == rhs.disc)
           && (lhs.media == rhs.media)
           && (lhs.machine_low == rhs.machine_low)
           && (lhs.machine_high == rhs.machine_high);
}

/*
    Parameters  : -
    Returns     : -
    Description : Constructor.
*/
backcfg_store::backcfg_store()
{
    // No other initialisation required
}

/*
    Parameters  : dir   - The directory containing the backup.
    Returns     : bool  - Was the configuration successfully read.
    Description : Attempt to read the backup configuration file.
*/
bool backcfg_store::load(string dir)
{
    // Attempt to open and read the configuration file
    ifstream file((dir + FS_CHAR_SEPARATOR + BACKCFG_FILE_OPTIONS).c_str());
    return file >> *this ? TRUE : FALSE;
}

/*
    Parameters  : dir   - The directory containing the backup.
    Returns     : void
    Description : Attempt to write the backup configuration file.
*/
void backcfg_store::save(string dir)
{
    // Attempt to write the configuration file
    ofstream file((dir + FS_CHAR_SEPARATOR + BACKCFG_FILE_OPTIONS).c_str());
    file << *this;

    // Generate an error if failed
    if (!file || file.bad())
    {
        os_error err;

        // Generate the error
        err.errnum = 0;
        filer_msgtrans(err.errmess, sizeof(err.errmess), "ErrSaveB");
        os_generate_error(&err);
    }
}


/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Update the configuration window.
*/
bool backcfg_obj::update(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle)
{
    backcfg_obj *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Disable the radio buttons if no automatic update
        bool automatic = optionbutton_c(BACKCFG_AUTO, ptr->obj)();
        radiobutton_c(BACKCFG_AUTO_EVERY, ptr->obj).set_faded(!automatic);
        radiobutton_c(BACKCFG_AUTO_DAILY, ptr->obj).set_faded(!automatic);
        radiobutton_c(BACKCFG_AUTO_WEEKLY, ptr->obj).set_faded(!automatic);

        // Disable the changes only option if no previous backups
        bool previous = 1 < numberrange_c(BACKCFG_VERSIONS, ptr->obj)();
        optionbutton_c(BACKCFG_CHANGES, ptr->obj).set_faded(!previous);
        if (!previous) optionbutton_c(BACKCFG_CHANGES, ptr->obj) = FALSE;

        // Disable ignoring drive name if no media unique identifier
        optionbutton_c(BACKCFG_DISC_IGNORE, ptr->adv_obj).set_faded(!ptr->get_num(backcfg_backup_media));
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : name      - The suggested directory name.
    Returns     : string    - The unique subdirectory.
    Description : Generate a unique subdirectory name.
*/
string backcfg_obj::make_dir(string name)
{
    // Loop until a unique directory name has been generated
    while (backcfg_exist(backcfg_dir + FS_CHAR_SEPARATOR + name))
    {
        bits count;

        // Check for an existing extension
        string::size_type pos = name.rfind(backcfg_unique_ext);
        string tail(name.substr(pos + backcfg_unique_ext.length()));
        int length;
        if ((sscanf(tail.c_str(), "%u%n", &count, &length) == 1)
            && (length = tail.length()))
        {
            // Remove the extension
            name.erase(pos);
            count++;
        }
        else count = 2;

        // Add a new extension
        char str[10];
        sprintf(str, "%u", count);
        name = name.substr(0, BACKCFG_MAX_DIR - backcfg_unique_ext.length() - strlen(str)) + backcfg_unique_ext + str;
    }

    // Return the result
    return name;
}

/*
    Parameters  : config    - The configuration to write.
    Returns     : void
    Description : Write the configuration to the window.
*/
void backcfg_obj::write(const backcfg_store &config)
{
    // Set the values in the window
    writablefield_c(BACKCFG_NAME, obj) = get_str(backcfg_name);
    if (config.exist(backcfg_auto)) optionbutton_c(BACKCFG_AUTO, obj) = config.get_bool(backcfg_auto);
    if (config.exist(backcfg_auto_interval))
    {
        bits every = config.get_num(backcfg_auto_interval);
        if (every == 0) radiobutton_c(BACKCFG_AUTO_EVERY, obj) = TRUE;
        else if (every <= 1) radiobutton_c(BACKCFG_AUTO_DAILY, obj) = TRUE;
        else if (every <= 7) radiobutton_c(BACKCFG_AUTO_WEEKLY, obj) = TRUE;
    }
    if (config.exist(backcfg_versions)) numberrange_c(BACKCFG_VERSIONS, obj) = config.get_num(backcfg_versions);
    if (config.exist(backcfg_changes)) optionbutton_c(BACKCFG_CHANGES, obj) = config.get_bool(backcfg_changes);
    if (config.exist(backcfg_always)) optionbutton_c(BACKCFG_ALWAYS, obj) = config.get_bool(backcfg_always);
    if (config.exist(backcfg_ignore_disc)) optionbutton_c(BACKCFG_DISC_IGNORE, adv_obj) = config.get_bool(backcfg_ignore_disc);
    if (config.exist(backcfg_ignore_drive)) optionbutton_c(BACKCFG_DRIVE_IGNORE, adv_obj) = config.get_bool(backcfg_ignore_drive);
    if (config.exist(backcfg_ignore_machine)) optionbutton_c(BACKCFG_MACHINE_IGNORE, adv_obj) = config.get_bool(backcfg_ignore_machine);
    displayfield_c(BACKCFG_DISC, adv_obj) = config.get_str(backcfg_backup_disc);
    displayfield_c(BACKCFG_DRIVE, adv_obj) = config.get_str(backcfg_backup_drive);
    if (config.exist(backcfg_backup_media))
    {
        char str[20];
        sprintf(str, "%04X-%04X",
                config.get_num(backcfg_backup_media) / 0x10000,
                config.get_num(backcfg_backup_media) % 0x10000);
        displayfield_c(BACKCFG_MEDIA, adv_obj) = str;
    }
    else displayfield_c(BACKCFG_MEDIA, adv_obj) = "";
    if (config.exist(backcfg_backup_machine_low) && config.exist(backcfg_backup_machine_high))
    {
        char str[20];
        sprintf(str, "%04X-%04X-%04X-%04X",
                config.get_num(backcfg_backup_machine_high) / 0x10000,
                config.get_num(backcfg_backup_machine_high) % 0x10000,
                config.get_num(backcfg_backup_machine_low) / 0x10000,
                config.get_num(backcfg_backup_machine_low) % 0x10000);
        displayfield_c(BACKCFG_MACHINE, adv_obj) = str;
    }
    else displayfield_c(BACKCFG_MACHINE, adv_obj) = "";
}

/*
    Parameters  : obj       - The object ID of the window.
                  config    - The configuration to read.
    Returns     : void
    Description : Read the configuration from the window.
*/
void backcfg_obj::read(backcfg_store &config)
{
    // Start with the active configuration
    config = *this;

    // Overwrite with values from the window
    set_str(backcfg_name, writablefield_c(BACKCFG_NAME, obj)());
    config.set_bool(backcfg_auto, optionbutton_c(BACKCFG_AUTO, obj)());
    switch (radiobutton_c(BACKCFG_AUTO_EVERY, obj).get_on())
    {
        case BACKCFG_AUTO_EVERY:
            config.set_num(backcfg_auto_interval, 0);
            break;

        case BACKCFG_AUTO_DAILY:
            config.set_num(backcfg_auto_interval, 1);
            break;

        case BACKCFG_AUTO_WEEKLY:
            config.set_num(backcfg_auto_interval, 7);
            break;
    }
    config.set_num(backcfg_versions, numberrange_c(BACKCFG_VERSIONS, obj)());
    config.set_bool(backcfg_changes, optionbutton_c(BACKCFG_CHANGES, obj)());
    config.set_bool(backcfg_always, optionbutton_c(BACKCFG_ALWAYS, obj)());
    config.set_bool(backcfg_ignore_disc, optionbutton_c(BACKCFG_DISC_IGNORE, adv_obj)());
    config.set_bool(backcfg_ignore_drive, optionbutton_c(BACKCFG_DRIVE_IGNORE, adv_obj)());
    config.set_bool(backcfg_ignore_machine, optionbutton_c(BACKCFG_MACHINE_IGNORE, adv_obj)());
}

/*
    Parameters  : void
    Returns     : void
    Description : Reset the configuration displayed in the window.
*/
void backcfg_obj::reset()
{
    // Write the configuration to the window
    write(*this);

    // Fake an update
    toolbox_block id_block;
    id_block.ancestor_obj = toolbox_NULL_OBJECT;
    id_block.this_obj = obj;
    update(0, NULL, &id_block, NULL);
}

/*
    Parameters  : dir   - The subdirectory name.
    Returns     : -
    Description : Constructor.
*/
backcfg_obj::backcfg_obj(string dir)
: dir(dir), is_valid(FALSE), is_open(FALSE)
{
    // Create the configuration window
    obj = toolbox_create_object(0, (toolbox_id) "WinBackCfg");

    // Obtain the handle of the advanced configuration window
    adv_obj = actionbutton_c(BACKCFG_ADVANCED, obj).get_show();

    // Set the client handle
    toolbox_set_client_handle(0, obj, this);

    // Register handlers to update the window
    event_register_toolbox_handler(obj,
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   update, NULL);
    event_register_toolbox_handler(adv_obj,
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   update, NULL);
    event_register_toolbox_handler(obj,
                                   action_NUMBER_RANGE_VALUE_CHANGED,
                                   update, NULL);

    // Attempt to load the configuration file
    load();

    // Add any missing essential fields
    if (!exist(backcfg_name)) set_str(backcfg_name, make_unique(dir));
    if (!exist(backcfg_backup_disc)) set_str(backcfg_backup_disc, dir);

    // Add to the list of backup configurations
    backcfg_list.push_back((backcfg_obj *)(this));
}

/*
    Parameters  : disc  - Details of the disc.
    Returns     : -
    Description : Constructor.
*/
backcfg_obj::backcfg_obj(const backcfg_disc &disc)
: dir(""), is_valid(FALSE), is_open(FALSE)
{
    // Create the configuration window
    obj = toolbox_create_object(0, (toolbox_id) "WinBackCfg");

    // Obtain the handle of the advanced configuration window
    adv_obj = actionbutton_c(BACKCFG_ADVANCED, obj).get_show();

    // Set the client handle
    toolbox_set_client_handle(0, obj, this);

    // Register handlers to update the window
    event_register_toolbox_handler(obj,
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   update, NULL);
    event_register_toolbox_handler(adv_obj,
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   update, NULL);
    event_register_toolbox_handler(obj,
                                   action_NUMBER_RANGE_VALUE_CHANGED,
                                   update, NULL);

    // Set the initial backup configuration details
    set_str(backcfg_name, make_unique(filer_msgtrans("BkDfNam", disc.disc.c_str(), string(string() + disc.drive).c_str())));
    set_str(backcfg_backup_disc, disc.disc);
    set_str(backcfg_backup_drive, string() + disc.drive);
    set_num(backcfg_backup_media, disc.media);
    set_num(backcfg_backup_machine_low, disc.machine_low);
    set_num(backcfg_backup_machine_high, disc.machine_high);
    set_bool(backcfg_ignore_disc, FALSE);
    set_bool(backcfg_ignore_drive, FALSE);
    set_bool(backcfg_ignore_machine, FALSE);

    // Add to the list of backup configurations
    backcfg_list.push_back((backcfg_obj *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
backcfg_obj::~backcfg_obj()
{
    // Remove from the list of backup configurations
    backcfg_list.remove((backcfg_obj *)(this));

    // Unregister handlers used to update the window
    event_deregister_toolbox_handler(obj,
                                     action_OPTION_BUTTON_STATE_CHANGED,
                                     update, NULL);
    event_deregister_toolbox_handler(adv_obj,
                                     action_OPTION_BUTTON_STATE_CHANGED,
                                     update, NULL);
    event_deregister_toolbox_handler(obj,
                                     action_NUMBER_RANGE_VALUE_CHANGED,
                                     update, NULL);

    // Delete the configuration window
    toolbox_delete_object(0, obj);
}

/*
    Parameters  : disc  - The disc name.
    Returns     : bool  - Does the disc name match.
    Description : Check if this backup configuration object matches the
                  specified disc name.
*/
bool backcfg_obj::eq_disc(const string disc) const
{
    // Return if the disc matches or should be ignored
    return (get_bool(backcfg_ignore_disc) && get_num(backcfg_backup_media))
           || (get_str(backcfg_backup_disc) == disc);
}

/*
    Parameters  : drive - The drive letter.
    Returns     : bool  - Does the drive letter match.
    Description : Check if this backup configuration object matches the
                  specified drive letter.
*/
bool backcfg_obj::eq_drive(psifs_drive drive) const
{
    // Return if the drive matches or should be ignored
    return get_bool(backcfg_ignore_drive)
           || !exist(backcfg_backup_drive)
           || (psifs_drive(get_str(backcfg_backup_drive)[0]) == drive);
}

/*
    Parameters  : media - The media unique identifier.
    Returns     : bool  - Does the media unique identifier match.
    Description : Check if this backup configuration object matches the
                  specified unique media identifier.
*/
bool backcfg_obj::eq_media(psifs_drive_id media) const
{
    // Return if the media unique identifier matches
    return !exist(backcfg_backup_media)
           || (get_num(backcfg_backup_media) == media);
}

/*
    Parameters  : low   - The low word of the machine unique identifier.
                  high  - The high word of the machine unique identifier.
    Returns     : bool  - Does the machine unique identifier match.
    Description : Check if this backup configuration object matches the
                  specified unique machine identifier.
*/
bool backcfg_obj::eq_machine(bits low, bits high) const
{
    // Return if the machine unique identifier matches or should be ignored
    return get_bool(backcfg_ignore_machine)
           || !exist(backcfg_backup_machine_low)
           || !exist(backcfg_backup_machine_high)
           || ((get_num(backcfg_backup_machine_low) == low)
               && (get_num(backcfg_backup_machine_high) == high));
}

/*
    Parameters  : top_left  - The coordinates for the top-left corner of
                              the window, or NULL for the default.
    Returns     : void
    Description : Open the window for this backup configuration.
*/
void backcfg_obj::open(const os_coord *top_left)
{
    // Show the configuration window
    toolbox_show_object(0, obj,
                        top_left
                        ? toolbox_POSITION_TOP_LEFT
                        : (toolbox_get_object_info(0, obj)
                           & toolbox_INFO_SHOWING
                           ? toolbox_POSITION_DEFAULT
                           : toolbox_POSITION_CENTRED),
                        (const toolbox_position *) top_left,
                        toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
}

/*
    Parameters  : void
    Returns     : void
    Description : Close the window for this backup configuration.
*/
void backcfg_obj::close()
{
    // Close the configuration window
    toolbox_hide_object(0, obj);
}

/*
    Parameters  : backup    - The new backup file.
                  scrap     - An optional backup file containing the
                              changes from the previous backup.
                  partial   - Was a partial backup performed.
    Returns     : void
    Description : Shuffle the files that comprise this backup to include
                  the specified new backup files. This also sets the date
                  of the last backup.
*/
void backcfg_obj::store(string backup, string scrap, bool partial)
{
    // Ensure that the backup directory exists
    string path(get_dir());

    // Shuffle or delete any previous files
    bits index;
    bits versions = get_num(backcfg_versions);
    string suffix;
    for (index = BACKCFG_MAX_VERSIONS - 1;
         index && !get_bool(backcfg_last_partial);
         index--)
    {
        // Generate the next suffix
        ostringstream next;
        next << BACKCFG_FILE_SEPARATOR << index;

        // Loop through all matching files
        int context = 0;
        int read;
        fs_pathname leaf;
        while (!xosgbpb_dir_entries(path.c_str(), (osgbpb_string_list *) leaf, 1, context, sizeof(leaf), (FS_CHAR_WILD_ANY + next.str()).c_str(), &read, &context) && (context != -1))
        {
            if (read)
            {
                string name = path + FS_CHAR_SEPARATOR + leaf;

                // Action depends on number of versions to store
                if (suffix.empty())
                {
                    // Delete this file
                    backcfg_delete(name);
                }
                else
                {
                    // Rename this file
                    backcfg_move(name, name.substr(0, name.length() - next.str().length()) + suffix);
                }
            }
        }

        // Store the suffix if appropriate
        if (index < versions) suffix = next.str();
    }

    // Deal with any previous backup
    string prev(get_prev());
    if (!prev.empty())
    {
        // Action depends on whether an incremental backup was performed
        if (suffix.empty())
        {
            // Do not keep previous versions
            backcfg_delete(prev);
            if (!scrap.empty()) backcfg_delete(scrap);
        }
        else if (scrap.empty())
        {
            // Full backup performed
            backcfg_move(prev, path + FS_CHAR_SEPARATOR + BACKCFG_FILE_FULL + suffix);
        }
        else
        {
            // Incremental backup performed
            backcfg_move(scrap, path + FS_CHAR_SEPARATOR + BACKCFG_FILE_INCREMENTAL + suffix);
            backcfg_delete(prev);
        }
    }

    // Move the new backup file
    backcfg_move(backup, path + FS_CHAR_SEPARATOR + BACKCFG_FILE_CURRENT);

    // Set the type of backup
    set_bool(backcfg_last_partial, partial);

    // Set the time of the last backup
    oswordreadclock_utc_block utc;
    utc.op = oswordreadclock_OP_UTC;
    oswordreadclock_utc(&utc);
    set_date(backcfg_last_time, *(date_riscos *) &utc);

    // Save the settings
    save();
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to read the backup configuration file.
*/
void backcfg_obj::load()
{
    // Attempt to load the configuration
    if (backcfg_store::load(get_dir())) is_valid = TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to write the backup configuration file.
*/
void backcfg_obj::save()
{
    // Ensure that the configuration name is unique
    set_str(backcfg_name, make_unique(get_str(backcfg_name), this));

    // Generate a directory name if necessary
    if (dir.empty())
    {
        // Ensure that the parent directory exists
        osfile_create_dir(backcfg_dir.c_str(), 0);

        // Generate the directory name
        dir = make_dir(get_str(backcfg_name));

        // Ensure that the backup directory exists
        osfile_create_dir(get_dir().c_str(), 0);
    }

    // Enable simple error handling
    filer_error_allowed++;

    // Save the configuration
    backcfg_store::save(get_dir());

    // Restore normal error handling
    filer_error_allowed--;

    // Set the valid flag
    is_valid = TRUE;
}

/*
    Parameters  : void
    Returns     : bool  - Is this backup configuration valid.
    Description : Check if this backup configuration is valid.
*/
bool backcfg_obj::get_valid() const
{
    // Return the value of the flag
    return is_valid;
}

/*
    Parameters  : void
    Returns     : bool  - Is an automatic backup due.
    Description : Check whether it is time to perform an automatic backup
                  of the disc associated with this configuration. This does
                  not take into account whether a backup has already been
                  started during the current session, or whether a
                  connection is active.
*/
bool backcfg_obj::get_due() const
{
    bool due;

    // Handle special cases
    if (!is_valid || !get_bool(backcfg_auto))
    {
        // Automatic backup disabled or no valid configuration
        due = FALSE;
    }
    else if (!exist(backcfg_last_time) || get_bool(backcfg_last_partial))
    {
        // No or incomplete previous backup
        due = TRUE;
    }
    else
    {
        // Read the current time
        oswordreadclock_utc_block now;
        now.op = oswordreadclock_OP_UTC;
        oswordreadclock_utc(&now);

        // Read the time of the last backup
        date_riscos last = get_date(backcfg_last_time);

        // Check the number of days since last backup
        due = backcfg_days(&last) + get_num(backcfg_auto_interval)
              <= backcfg_days((date_riscos *) now.utc);
    }


    // Backup interval
    get_num(backcfg_auto_interval);

    // Return the result
    return due;
}

/*
    Parameters  : void
    Returns     : string    - The directory containing this backup.
    Description : Return the name of the directory containing this
                  backup.
*/
string backcfg_obj::get_dir() const
{
    // Return the full directory name
    return backcfg_dir + FS_CHAR_SEPARATOR + dir;
}

/*
    Parameters  : void
    Returns     : string    - The name of the previous backup file.
    Description : Return the name of the previous backup file. An empty
                  string is returned if no previous backup exists.
*/
string backcfg_obj::get_prev() const
{
    // Generate the name of the previous backup file
    string name = get_dir() + FS_CHAR_SEPARATOR + BACKCFG_FILE_CURRENT;

    // Clear the name if file does not exist
    fileswitch_object_type type;
    if (xosfile_read_stamped_no_path(name.c_str(), &type,
                                     NULL, NULL, NULL, NULL, NULL)
        || (type == fileswitch_NOT_FOUND))
    {
        name = "";
    }

    // Return the name
    return name;
}

/*
    Parameters  : id_block      - A toolbox ID block.
    Returns     : backcfg_obj   - Pointer to the corresponding backup
                                  configuration object, or NULL if not
                                  found.
    Description : Convert a toolbox ID block into a backup configuration
                  object pointer.
*/
backcfg_obj *backcfg_obj::find(const toolbox_block *id_block)
{
    // Choose the ancestor object ID
    toolbox_o id = id_block->ancestor_obj == toolbox_NULL_OBJECT
                   ? id_block->this_obj
                   : id_block->ancestor_obj;

    // Attempt to read the client handle
    backcfg_obj *ptr;
    if (xtoolbox_get_client_handle(0, id, (void **) &ptr)) ptr = NULL;

    // Return the pointer
    return ptr;
}

/*
    Parameters  : disc          - Details of the disc.
    Returns     : backcfg_obj   - Pointer to the backup configuration
                                  object, or NULL if not found.
    Description : Find the closest matching backup configuration object.
*/
backcfg_obj *backcfg_obj::find(const backcfg_disc &disc)
{
    backcfg_obj *ptr = NULL;

    // Loop through all of the backup configuration objects
    for (list_iterator<backcfg_obj *> i = backcfg_list.begin();
         i != backcfg_list.end();
         i++)
    {
        bool better;

        // Check if it is a possible match
        better = ((*i)->get_num(backcfg_backup_media)
                  && (*i)->eq_media(disc.media))
                 || (!(*i)->get_bool(backcfg_ignore_disc)
                     && (*i)->eq_disc(disc.disc));

        // Attempt to compare to the previous best match
        if (better && ptr)
        {
            bool b_drive = (*i)->eq_drive(disc.drive)
                           && (!ptr->eq_drive(disc.drive)
                               || (!(*i)->get_bool(backcfg_ignore_drive)
                                   && ptr->get_bool(backcfg_ignore_drive)));
            bool b_machine = ((*i)->eq_machine(disc.machine_low,
                                               disc.machine_high)
                              && (!ptr->eq_machine(disc.machine_low,
                                                   disc.machine_high)
                                  || (!(*i)->get_bool(backcfg_ignore_machine)
                                      && ptr->get_bool(backcfg_ignore_machine))
                                  || (((*i)->get_bool(backcfg_ignore_machine)
                                       == ptr->get_bool(backcfg_ignore_machine))
                                      && b_drive)))
                             || (!ptr->eq_machine(disc.machine_low,
                                                  disc.machine_high)
                                 && b_drive);
            bool b_disc = ((*i)->eq_disc(disc.disc)
                           && (!ptr->eq_disc(disc.disc)
                               || (!(*i)->get_bool(backcfg_ignore_disc)
                                   && ptr->get_bool(backcfg_ignore_disc))
                               || (((*i)->get_bool(backcfg_ignore_disc)
                                    == ptr->get_bool(backcfg_ignore_disc))
                                   && b_machine)))
                          || (!ptr->eq_disc(disc.disc)
                              && b_machine);
            bool b_media = ((*i)->eq_media(disc.media)
                            && (!ptr->eq_media(disc.media)
                                || b_disc))
                           || (!ptr->eq_media(disc.media)
                               && b_disc);
            better = b_media;
        }

        // Store the new pointer if it is a better match
        if (better) ptr = *i;
    }

    // Return the result
    return ptr;
}

/*
    Parameters  : disc  - Details of the disc.
    Returns     : bool  - Has the name been used for a different disc.
    Description : Check whether a backup configuration has been created for
                  a different disc with the same name.
*/
bool backcfg_obj::used(backcfg_disc &disc)
{
    bool used = FALSE;
    bool match = FALSE;

    // Loop through all of the backup configuration objects
    for (list_iterator<backcfg_obj *> i = backcfg_list.begin();
         !match && (i != backcfg_list.end());
         i++)
    {
        // Only interested in matching disc name
        if ((*i)->eq_disc(disc.disc) && !(*i)->get_bool(backcfg_ignore_disc))
        {
            // Action depends on whether the media unique identifier matches
            if ((*i)->eq_media(disc.media)) match = TRUE;
            else used = TRUE;
        }
    }

    // Return the result
    return used && !match;
}

/*
    Parameters  : name      - The backup configuration name to check.
                  ignore    - An optional backup configuration to ignore.
    Returns     : bool  - Is the name unique.
    Description : Check whether a backup configuration name is already used.
*/
bool backcfg_obj::unique(string name, backcfg_obj *ignore)
{
    bool unique = !name.empty();

    // Loop through all of the backup configuration objects
    for (list_iterator<backcfg_obj *> i = backcfg_list.begin();
         unique && (i != backcfg_list.end());
         i++)
    {
        if ((*i != ignore) && ((*i)->get_str(backcfg_name) == name))
        {
            unique = FALSE;
        }
    }

    // Return the result
    return unique;
}

/*
    Parameters  : name      - The backup configuration name to check.
                  ignore    - An optional backup configuration to ignore.
    Returns     : string    - A unique backup configuration name.
    Description : Generate a unique backup configuration name based on the
                  name supplied.
*/
string backcfg_obj::make_unique(string name, backcfg_obj *ignore)
{
    // Loop until a unique name has been generated
    while (!unique(name, ignore))
    {
        bits count;

        // Check for an existing extension
        string::size_type pos = name.rfind(backcfg_unique_ext);
        string tail(name.substr(pos + backcfg_unique_ext.length()));
        int length;
        if ((sscanf(tail.c_str(), "%u%n", &count, &length) == 1)
            && (length = tail.length()))
        {
            // Remove the extension
            name.erase(pos);
            count++;
        }
        else count = 2;

        // Add a new extension
        char str[10];
        sprintf(str, "%u", count);
        name = name.substr(0, BACKCFG_MAX_NAME - backcfg_unique_ext.length() - strlen(str)) + backcfg_unique_ext + str;
    }

    // Return the result
    return name;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Initialise when the backup configuration window is shown.
*/
bool backcfg_obj::show(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle)
{
    backcfg_obj *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found or the window is already open
    if (ptr && !ptr->is_open)
    {
        // Set the open flag
        ptr->is_open = TRUE;

        // Reset the displayed configuration
        ptr->reset();
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
    Description : Tidy up when the backup configuration window is closed.
*/
bool backcfg_obj::hide(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle)
{
    backcfg_obj *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Clear the open flag
        ptr->is_open = FALSE;
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
    Description : Handle the set action button.
*/
bool backcfg_obj::set(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle)
{
    backcfg_obj *ptr = (backcfg_obj *) toolbox_get_client_handle(0, id_block->this_obj);

    NOT_USED(event_code);
    NOT_USED(handle);

    // Read the configuration from the window
    ptr->read(*ptr);

    // Save the settings
    ptr->save();

    // Close the window if appropriate
    if (action->flags & actionbutton_SELECTED_ADJUST)
    {
        // Fake an update
        update(0, NULL, id_block, NULL);
    }
    else
    {
        // Close the configuration window
        toolbox_hide_object(0, ptr->obj);

        // Update all backup objects
        backobj_obj::update_all();

        // Open the backup window
        backobj_obj::open(ptr->get_str(backcfg_name));
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
bool backcfg_obj::cancel(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle)
{
    backcfg_obj *ptr = (backcfg_obj *) toolbox_get_client_handle(0, id_block->this_obj);

    NOT_USED(event_code);
    NOT_USED(handle);

    // Either close the window or reset the gadgets
    if (action->flags & actionbutton_SELECTED_ADJUST)
    {
        // Reset the displayed configuration
        ptr->reset();
    }
    else
    {
        // Close the window
        toolbox_hide_object(0, ptr->obj);

        // Open the backup window
        backobj_obj::open(ptr->get_str(backcfg_name));
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Load any backup configurations.
*/
void backcfg_obj::initialise()
{
    // No action if already initialised
    if (backcfg_dir.empty())
    {
        // Find the parent directory
        backcfg_dir = config_current.get_str(config_tag_backup_path);
        if (backcfg_dir.empty()) backcfg_dir = string(BACKCFG_DEFAULT_DIR);

        // Loop through all matching files
        int context = 0;
        int read;
        fs_info info;
        while (!xosgbpb_dir_entries_info(backcfg_dir.c_str(),
                                         (osgbpb_info_list *) &info, 1, context,
                                         sizeof(info), NULL, &read, &context)
               && (context != -1))
        {
            // Read the backup configuration from the directory if found
            if (read && (info.obj_type == fileswitch_IS_DIR))
            {
                new backcfg_obj(string(info.name));
            }
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Unload any backup configurations.
*/
void backcfg_obj::finalise()
{
    // Delete all backup configuration objects
    while (!backcfg_list.empty())
    {
        // Delete the backup configuration object from the front of the list
        delete backcfg_list.front();
    }

    // Clear the parent directory variable
    backcfg_dir = "";
}

/*
    Parameters  : void
    Returns     : os_coord *    - Pointer to the coordinates of the top-left
                                  corner of the window, or NULL if not open.
    Description : Return the position of the current window.
*/
const os_coord *backcfg_cfg::get_top_left() const
{
    static os_coord top_left;

    // Check if the window is open
    bool open = (obj != toolbox_NULL_OBJECT)
                && (toolbox_get_object_info(0, obj) & toolbox_INFO_SHOWING);

    // Read the current position if window is open
    if (open)
    {
        wimp_window_state state;

        // Read the window state
        state.w = window_get_wimp_handle(0, obj);
        wimp_get_window_state(&state);

        // Copy the window position
        top_left.x = state.visible.x0;
        top_left.y = state.visible.y1;
    }

    // Return the position
    return open ? &top_left : NULL;
}

/*
    Parameters  : void
    Returns     : void
    Description : Create a new window object if required.
*/
void backcfg_cfg::create()
{
    // No action if already created
    if (obj == toolbox_NULL_OBJECT)
    {
        // Create the appropriate object
        if (config->used(disc))
        {
            // Create the media unique identifier warning window
            obj = toolbox_create_object(0, (toolbox_id) "WinBackMedi");
            button_c(BACKCFG_WARN_TEXT, obj) = filer_msgtrans("BkWnMed", disc.disc.c_str(), string(string() + disc.drive).c_str());
            writablefield_c(BACKCFG_WARN_RENAME_NAME, obj) = disc.disc;
        }
        else if (!config->eq_disc(disc.disc))
        {
            // Create the disc name warning window
            obj = toolbox_create_object(0, (toolbox_id) "WinBackDisc");
            button_c(BACKCFG_WARN_TEXT, obj) = filer_msgtrans("BkWnDsc", disc.disc.c_str(), string(string() + disc.drive).c_str(), config->get_str(backcfg_backup_disc).c_str());
            radiobutton_c(BACKCFG_WARN_IGNORE, obj).set_faded(!config->get_num(backcfg_backup_media));
            radiobutton_c(BACKCFG_WARN_PREV, obj).set_faded(!config->get_valid());
        }
        else if (!config->eq_machine(disc.machine_low, disc.machine_high))
        {
            // Create the machine unique identifier warning window
            obj = toolbox_create_object(0, (toolbox_id) "WinBackMach");
            button_c(BACKCFG_WARN_TEXT, obj) = filer_msgtrans("BkWnMch", disc.disc.c_str(), string(string() + disc.drive).c_str());
        }
        else if (!config->eq_drive(disc.drive))
        {
            // Create the drive letter warning window
            obj = toolbox_create_object(0, (toolbox_id) "WinBackDriv");
            button_c(BACKCFG_WARN_TEXT, obj) = filer_msgtrans("BkWnDrv", disc.disc.c_str(), string(string() + disc.drive).c_str(), config->get_str(backcfg_backup_drive).c_str());
        }

        // Perform common initialisation if created
        if (obj != toolbox_NULL_OBJECT)
        {
            // Set the client handle
            toolbox_set_client_handle(0, obj, this);

            // Register toolbox handlers to update the window
            event_register_toolbox_handler(obj,
                                           action_RADIO_BUTTON_STATE_CHANGED,
                                           update, NULL);
            event_register_toolbox_handler(obj,
                                           action_WRITABLE_FIELD_VALUE_CHANGED,
                                           update, NULL);

            // Unable to move backup if not valid
            radiobutton_c(BACKCFG_WARN_MOVE, obj).set_faded(!config->get_valid());

            // Perform an initial reset
            reset();
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Destroy any window object.
*/
void backcfg_cfg::destroy()
{
    // No action if window does not exist
    if (obj != toolbox_NULL_OBJECT)
    {
        // Unregister toolbox handlers used to update the window
        event_deregister_toolbox_handler(obj,
                                         action_RADIO_BUTTON_STATE_CHANGED,
                                         update, NULL);
        event_deregister_toolbox_handler(obj,
                                         action_WRITABLE_FIELD_VALUE_CHANGED,
                                         update, NULL);

        // Delete the object
        toolbox_delete_object(0, obj);
        obj = toolbox_NULL_OBJECT;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Reset the configuration displayed in the window.
*/
void backcfg_cfg::reset()
{
    // Set the default options
    radiobutton_c(radiobutton_c(BACKCFG_WARN_IGNORE, obj).get_faded() ? BACKCFG_WARN_NEW : BACKCFG_WARN_IGNORE, obj) = TRUE;
    writablefield_c(BACKCFG_WARN_NEW_NAME, obj) = config->make_unique(config->get_str(backcfg_name));

    // Fake an update
    toolbox_block id_block;
    id_block.ancestor_obj = toolbox_NULL_OBJECT;
    id_block.this_obj = obj;
    update(0, NULL, &id_block, NULL);
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Update the window.
*/
bool backcfg_cfg::update(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle)
{
    backcfg_cfg *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        bool clone = radiobutton_c(BACKCFG_WARN_NEW, ptr->obj)();

        // Set the configuration name
        displayfield_c(BACKCFG_WARN_NAME, ptr->obj) = ptr->config->get_str(backcfg_name);

        // Disable the writable field if appropriate
        writablefield_c(BACKCFG_WARN_NEW_NAME, ptr->obj).set_faded(!clone);

        // Disable the set button if appropriate
        actionbutton_c(BACKCFG_WARN_SET, ptr->obj).set_faded(clone ? !ptr->config->unique(writablefield_c(BACKCFG_WARN_NEW_NAME, ptr->obj)()) : FALSE);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : disc  - Details of the disc.
    Returns     : -
    Description : Constructor.
*/
backcfg_cfg::backcfg_cfg(const backcfg_disc &disc)
: disc(disc), obj(toolbox_NULL_OBJECT), is_open(FALSE)
{
    // Attempt to find a matching configuration
    config = backcfg_obj::find(disc);

    // Create a new configuration if no match
    if (!config) config = new backcfg_obj(disc);
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
backcfg_cfg::~backcfg_cfg()
{
    // Delete any window
    destroy();
}

/*
    Parameters  : void
    Returns     : backcfg_obj   - The backup configuration object.
    Description : Redirect access to the underlying backup configuration
                  object.
*/
backcfg_obj *backcfg_cfg::operator->()
{
    // Pass on to the configuration object
    return config;
}

/*
    Parameters  : void
    Returns     : backcfg_obj   - The backup configuration object.
    Description : Redirect access to the underlying backup configuration
                  object.
*/
backcfg_obj &backcfg_cfg::operator*()
{
    // Pass on to the configuration object
    return *config;
}

/*
    Parameters  : top_left  - The coordinates for the top-left corner of
                              the window, or NULL for the default.
    Returns     : void
    Description : Open the window for this backup configuration.
*/
void backcfg_cfg::open(const os_coord *top_left)
{
    // Create a window object if required
    create();

    // Open the appropriate window
    if (obj)
    {
        // Open the warning window
        if (!top_left) top_left = get_top_left();
        toolbox_show_object(0, obj,
                            top_left
                            ? toolbox_POSITION_TOP_LEFT
                            : toolbox_POSITION_CENTRED,
                            (const toolbox_position *) top_left,
                            toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
    }
    else
    {
        // Otherwise pass onto the configuration object
        config->open(top_left);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Close the window for this backup configuration.
*/
void backcfg_cfg::close()
{
    // Delete any window object
    destroy();

    // Pass on to the configuration object
    config->close();
}

/*
    Parameters  : disc  - Details of the disc.
    Returns     : bool  - Do the details match.
    Description : Check if this backup configuration matches the specified
                  disc.
*/
bool backcfg_cfg::eq(const backcfg_disc &disc) const
{
    // Compare the disc details only
    return this->disc == disc;
}

/*
    Parameters  : backup    - The new backup file.
                  scrap     - An optional backup file containing the
                              changes from the previous backup.
                  partial   - Was a partial backup performed.
    Returns     : void
    Description : Shuffle the files that comprise this backup to include
                  the specified new backup files. This also sets the date
                  of the last backup.
*/
void backcfg_cfg::store(string backup, string scrap, bool partial)
{
    // Pass on to the configuration object if valid
    if (get_valid()) config->store(backup, scrap, partial);
}

/*
    Parameters  : void
    Returns     : bool  - Is this backup configuration valid.
    Description : Check if this backup configuration is valid.
*/
bool backcfg_cfg::get_valid() const
{
    // Return whether the backup configuration is valid
    return config->eq_media(disc.media)
           && config->eq_disc(disc.disc)
           && config->eq_drive(disc.drive)
           && config->eq_machine(disc.machine_low, disc.machine_high)
           && config->get_valid();
}

/*
    Parameters  : void
    Returns     : bool  - Is an automatic backup due.
    Description : Check whether it is time to perform an automatic backup
                  of the disc associated with this configuration. This does
                  not take into account whether a backup has already been
                  started during the current session, or whether a
                  connection is active.
*/
bool backcfg_cfg::get_due() const
{
    // Pass on to the configuration object if valid
    return get_valid() && config->get_due();
}

/*
    Parameters  : void
    Returns     : string    - The name of the previous backup file.
    Description : Return the name of the previous backup file. An empty
                  string is returned if no previous backup exists.
*/
string backcfg_cfg::get_prev() const
{
    // Pass on to the configuration object if valid
    return get_valid() ? config->get_prev() : string();
}

/*
    Parameters  : void
    Returns     : psifs_drive   - The drive.
    Description : Get the drive for this backup configuration.
*/
psifs_drive backcfg_cfg::get_drive() const
{
    // Return the drive letter
    return disc.drive;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Initialise when the backup configuration window is shown.
*/
bool backcfg_cfg::show(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle)
{
    backcfg_cfg *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found or the window is already open
    if (ptr && !ptr->is_open)
    {
        // Set the open flag
        ptr->is_open = TRUE;

        // Reset the displayed configuration
        ptr->reset();

        // Ensure that any associated configuration windows are closed
        ptr->config->close();
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
    Description : Tidy up when the backup configuration window is closed.
*/
bool backcfg_cfg::hide(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle)
{
    backcfg_cfg *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Clear the open flag
        ptr->is_open = FALSE;
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
    Description : Handle the media unique identifier set action button.
*/
bool backcfg_cfg::set_media(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    backcfg_cfg *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Perform the appropriate action
        switch (radiobutton_c(BACKCFG_WARN_RENAME, ptr->obj).get_on())
        {
            case BACKCFG_WARN_RENAME:
                // Rename the disc
                filer_error_allowed++;
                psifsfileop_name_disc(ptr->disc.drive, writablefield_c(BACKCFG_WARN_RENAME_NAME, ptr->obj)().c_str());
                filer_error_allowed--;
                break;

            case BACKCFG_WARN_MOVE:
                // Update the backup configuration with this media identifier
                ptr->config->set_num(backcfg_backup_media, ptr->disc.media);
                break;

            case BACKCFG_WARN_NEW:
                // Create a new backup configuration for this media identifier
                ptr->config = new backcfg_obj(ptr->disc);
                ptr->config->set_str(backcfg_name, writablefield_c(BACKCFG_WARN_NEW_NAME, ptr->obj)());
                break;
        }

        // Save the changes if the configuration if valid
        if (ptr->config->get_valid()) ptr->config->save();

        // Read the current window position
        const os_coord *top_left = ptr->get_top_left();

        // Destroy the window
        ptr->destroy();

        // Open another window as appropriate
        if (!ptr->get_valid() || (action->flags & actionbutton_SELECTED_ADJUST))
        {
            // Open a new window at the original position
            ptr->open(top_left);
        }
        else
        {
            // Open the backup window
            backobj_obj::open(ptr->config->get_str(backcfg_name));
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
    Description : Handle the disc name set action button.
*/
bool backcfg_cfg::set_disc(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    backcfg_cfg *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Perform the appropriate action
        switch (radiobutton_c(BACKCFG_WARN_IGNORE, ptr->obj).get_on())
        {
            case BACKCFG_WARN_IGNORE:
                // Ignore the disc name
                ptr->config->set_bool(backcfg_ignore_disc, TRUE);
                break;

            case BACKCFG_WARN_MOVE:
                // Update the backup configuration with this disc name
                ptr->config->set_str(backcfg_backup_disc, ptr->disc.disc);
                break;

            case BACKCFG_WARN_NEW:
                // Create a new backup configuration for this disc name
                ptr->config = new backcfg_obj(ptr->disc);
                ptr->config->set_str(backcfg_name, writablefield_c(BACKCFG_WARN_NEW_NAME, ptr->obj)());
                break;

            case BACKCFG_WARN_PREV:
                // Rename the disc back to its orginal name
                filer_error_allowed++;
                psifsfileop_name_disc(ptr->disc.drive, ptr->config->get_str(backcfg_backup_disc).c_str());
                filer_error_allowed--;
                break;
        }

        // Save the changes if the configuration if valid
        if (ptr->config->get_valid()) ptr->config->save();

        // Read the current window position
        const os_coord *top_left = ptr->get_top_left();

        // Destroy the window
        ptr->destroy();

        // Open another window as appropriate
        if (!ptr->get_valid() || (action->flags & actionbutton_SELECTED_ADJUST))
        {
            // Open a new window at the original position
            ptr->open(top_left);
        }
        else
        {
            // Open the backup window
            backobj_obj::open(ptr->config->get_str(backcfg_name));
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
    Description : Handle the drive letter set action button.
*/
bool backcfg_cfg::set_drive(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    backcfg_cfg *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Perform the appropriate action
        switch (radiobutton_c(BACKCFG_WARN_IGNORE, ptr->obj).get_on())
        {
            case BACKCFG_WARN_IGNORE:
                // Ignore the drive letter
                ptr->config->set_bool(backcfg_ignore_drive, TRUE);
                break;

            case BACKCFG_WARN_MOVE:
                // Update the backup configuration with this drive letter
                ptr->config->set_str(backcfg_backup_drive, string() + ptr->disc.drive);
                break;

            case BACKCFG_WARN_NEW:
                // Create a new backup configuration for this drive letter
                ptr->config = new backcfg_obj(ptr->disc);
                ptr->config->set_str(backcfg_name, writablefield_c(BACKCFG_WARN_NEW_NAME, ptr->obj)());
                break;
        }

        // Save the changes if the configuration if valid
        if (ptr->config->get_valid()) ptr->config->save();

        // Read the current window position
        const os_coord *top_left = ptr->get_top_left();

        // Destroy the window
        ptr->destroy();

        // Open another window as appropriate
        if (!ptr->get_valid() || (action->flags & actionbutton_SELECTED_ADJUST))
        {
            // Open a new window at the original position
            ptr->open(top_left);
        }
        else
        {
            // Open the backup window
            backobj_obj::open(ptr->config->get_str(backcfg_name));
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
    Description : Handle the machine unique identifier set action button.
*/
bool backcfg_cfg::set_machine(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    backcfg_cfg *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Perform the appropriate action
        switch (radiobutton_c(BACKCFG_WARN_IGNORE, ptr->obj).get_on())
        {
            case BACKCFG_WARN_IGNORE:
                // Ignore the machine unique identifier and drive letter
                ptr->config->set_bool(backcfg_ignore_drive, TRUE);
                ptr->config->set_bool(backcfg_ignore_machine, TRUE);
                break;

            case BACKCFG_WARN_MOVE:
                // Update the backup configuration with this machine identifier
                ptr->config->set_str(backcfg_backup_drive, string() + ptr->disc.drive);
                ptr->config->set_num(backcfg_backup_machine_low, ptr->disc.machine_low);
                ptr->config->set_num(backcfg_backup_machine_high, ptr->disc.machine_high);
                break;

            case BACKCFG_WARN_NEW:
                // Create a new backup configuration for this machine identifier
                ptr->config = new backcfg_obj(ptr->disc);
                ptr->config->set_str(backcfg_name, writablefield_c(BACKCFG_WARN_NEW_NAME, ptr->obj)());
                break;
        }

        // Save the changes if the configuration if valid
        if (ptr->config->get_valid()) ptr->config->save();

        // Read the current window position
        const os_coord *top_left = ptr->get_top_left();

        // Destroy the window
        ptr->destroy();

        // Open another window as appropriate
        if (!ptr->get_valid() || (action->flags & actionbutton_SELECTED_ADJUST))
        {
            // Open a new window at the original position
            ptr->open(top_left);
        }
        else
        {
            // Open the backup window
            backobj_obj::open(ptr->config->get_str(backcfg_name));
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
    Description : Handle the cancel action button.
*/
bool backcfg_cfg::cancel(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle)
{
    backcfg_cfg *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Reopen either the warning or backup window
        if (action->flags & actionbutton_SELECTED_ADJUST)
        {
            // Reset the window contents
            ptr->reset();
        }
        else
        {
            // Destroy the warning window
            ptr->destroy();

            // Open the backup window
            backobj_obj::open(ptr->config->get_str(backcfg_name));
        }
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : id_block      - A toolbox ID block.
    Returns     : backcfg_cfg   - Pointer to the corresponding backup
                                  configuration, or NULL if not found.
    Description : Convert a toolbox ID block into a backup configuration
                  pointer.
*/
backcfg_cfg *backcfg_cfg::find(const toolbox_block *id_block)
{
    // Choose the ancestor object ID
    toolbox_o id = id_block->ancestor_obj == toolbox_NULL_OBJECT
                   ? id_block->this_obj
                   : id_block->ancestor_obj;

    // Attempt to read the client handle
    backcfg_cfg *ptr;
    if (xtoolbox_get_client_handle(0, id, (void **) &ptr)) ptr = NULL;

    // Return the pointer
    return ptr;
}
