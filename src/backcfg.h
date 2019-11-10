/*
    File        : backcfg.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Backup configuration window handling for the PsiFS filer.

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

// Only include header file once
#ifndef BACKCFG_H
#define BACKCFG_H

// Include oslib header files
#include "oslib/toolbox.h"

// Include project header files
#include "tag.h"

// Backup configuration file tags
extern const char backcfg_always[];
extern const char backcfg_auto[];
extern const char backcfg_auto_interval[];
extern const char backcfg_versions[];
extern const char backcfg_changes[];
extern const char backcfg_last_time[];
extern const char backcfg_last_partial[];
extern const char backcfg_name[];
extern const char backcfg_backup_disc[];
extern const char backcfg_backup_drive[];
extern const char backcfg_backup_media[];
extern const char backcfg_backup_machine_low[];
extern const char backcfg_backup_machine_high[];
extern const char backcfg_ignore_disc[];
extern const char backcfg_ignore_drive[];
extern const char backcfg_ignore_machine[];

// A backup disc
class backcfg_disc
{
public:

    psifs_drive drive;
    string disc;
    psifs_drive_id media;
    bits machine_low;
    bits machine_high;

    /*
        Parameters  : -
        Returns     : -
        Description : Constructor.
    */
    backcfg_disc();

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~backcfg_disc() {}

    /*
        Parameters  : drive - The drive to read the details for.
        Returns     : bool  - Is there a valid disc.
        Description : Read the details for the specified drive.
    */
    bool read(psifs_drive drive);
};

/*
    Parameters  : lhs   - The first disc to compare.
                  rhs   - The second disc to compare.
    Returns     : bool  - Are the two discs the same.
    Description : Compare two discs.
*/
bool operator==(const backcfg_disc &lhs, const backcfg_disc &rhs);

// A backup configuration store
class backcfg_store : public tag_store
{
public:

    /*
        Parameters  : -
        Returns     : -
        Description : Constructor.
    */
    backcfg_store();

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~backcfg_store() {}

    /*
        Parameters  : dir   - The directory containing the backup.
        Returns     : bool  - Was the configuration successfully read.
        Description : Attempt to read the backup configuration file.
    */
    bool load(string dir);

    /*
        Parameters  : dir   - The directory containing the backup.
        Returns     : void
        Description : Attempt to write the backup configuration file.
    */
    void save(string dir);
};

// A backup configuration object
class backcfg_obj : public backcfg_store
{
    string dir;                         // Subdirectory containing this backup
    bool is_valid;                      // Is this configuration valid
    bool is_open;                       // Is the configuration window open
    toolbox_o obj;                      // Toolbox object ID for the window
    toolbox_o adv_obj;                  // Toolbox object ID for the advanced

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Update the configuration window.
    */
    static bool update(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle);

    /*
        Parameters  : name      - The suggested directory name.
        Returns     : string    - The unique subdirectory.
        Description : Generate a unique subdirectory name.
    */
    static string make_dir(string name);

    /*
        Parameters  : config    - The configuration to write.
        Returns     : void
        Description : Write the configuration to the window.
    */
    void write(const backcfg_store &config);

    /*
        Parameters  : obj       - The object ID of the window.
                      config    - The configuration to read.
        Returns     : void
        Description : Read the configuration from the window.
    */
    void read(backcfg_store &config);

    /*
        Parameters  : void
        Returns     : void
        Description : Reset the configuration displayed in the window.
    */
    void reset();

public:

    /*
        Parameters  : dir   - The subdirectory name.
        Returns     : -
        Description : Constructor.
    */
    backcfg_obj(string dir);

    /*
        Parameters  : disc  - Details of the disc.
        Returns     : -
        Description : Constructor.
    */
    backcfg_obj(const backcfg_disc &disc);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~backcfg_obj();

    /*
        Parameters  : disc  - The disc name.
        Returns     : bool  - Does the disc name match.
        Description : Check if this backup configuration object matches the
                      specified disc name.
    */
    bool eq_disc(const string disc) const;

    /*
        Parameters  : drive - The drive letter.
        Returns     : bool  - Does the drive letter match.
        Description : Check if this backup configuration object matches the
                      specified drive letter.
    */
    bool eq_drive(psifs_drive drive) const;

    /*
        Parameters  : media - The media unique identifier.
        Returns     : bool  - Does the media unique identifier match.
        Description : Check if this backup configuration object matches the
                      specified unique media identifier.
    */
    bool eq_media(psifs_drive_id media) const;

    /*
        Parameters  : low   - The low word of the machine unique identifier.
                      high  - The high word of the machine unique identifier.
        Returns     : bool  - Does the machine unique identifier match.
        Description : Check if this backup configuration object matches the
                      specified unique machine identifier.
    */
    bool eq_machine(bits low, bits high) const;

    /*
        Parameters  : top_left  - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
        Returns     : void
        Description : Open the window for this backup configuration.
    */
    void open(const os_coord *top_left = NULL);

    /*
        Parameters  : void
        Returns     : void
        Description : Close the window for this backup configuration.
    */
    void close();

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
    void store(string backup, string scrap, bool partial = FALSE);

    /*
        Parameters  : void
        Returns     : void
        Description : Attempt to read the backup configuration file.
    */
    void load();

    /*
        Parameters  : void
        Returns     : void
        Description : Attempt to write the backup configuration file.
    */
    void save();

    /*
        Parameters  : void
        Returns     : bool  - Is this backup configuration valid.
        Description : Check if this backup configuration is valid.
    */
    bool get_valid() const;

    /*
        Parameters  : void
        Returns     : bool  - Is an automatic backup due.
        Description : Check whether it is time to perform an automatic backup
                      of the disc associated with this configuration. This does
                      not take into account whether a backup has already been
                      started during the current session, or whether a
                      connection is active.
    */
    bool get_due() const;

    /*
        Parameters  : void
        Returns     : string    - The directory containing this backup.
        Description : Return the name of the directory containing this
                      backup.
    */
    string get_dir() const;

    /*
        Parameters  : void
        Returns     : string    - The name of the previous backup file.
        Description : Return the name of the previous backup file. An empty
                      string is returned if no previous backup exists.
    */
    string get_prev() const;

    /*
        Parameters  : id_block      - A toolbox ID block.
        Returns     : backcfg_obj   - Pointer to the corresponding backup
                                      configuration object, or NULL if not
                                      found.
        Description : Convert a toolbox ID block into a backup configuration
                      object pointer.
    */
    static backcfg_obj *find(const toolbox_block *id_block);

    /*
        Parameters  : disc          - Details of the disc.
        Returns     : backcfg_obj   - Pointer to the backup configuration
                                      object, or NULL if not found.
        Description : Find the closest matching backup configuration object
    */
    static backcfg_obj *find(const backcfg_disc &disc);

    /*
        Parameters  : disc  - Details of the disc.
        Returns     : bool  - Has the name been used for a different disc.
        Description : Check whether a backup configuration has been created for
                      a different disc with the same name.
    */
    static bool used(backcfg_disc &disc);

    /*
        Parameters  : name      - The backup configuration name to check.
                      ignore    - An optional backup configuration to ignore.
        Returns     : bool  - Is the name unique.
        Description : Check whether a backup configuration name is already used.
    */
    static bool unique(string name, backcfg_obj *ignore = NULL);

    /*
        Parameters  : name      - The backup configuration name to check.
                      ignore    - An optional backup configuration to ignore.
        Returns     : string    - A unique backup configuration name.
        Description : Generate a unique backup configuration name based on the
                      name supplied.
    */
    static string make_unique(string name, backcfg_obj *ignore = NULL);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Initialise when the backup configuration window is shown.
    */
    static bool show(bits event_code, toolbox_action *action,
                     toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Tidy up when the backup configuration window is closed.
    */
    static bool hide(bits event_code, toolbox_action *action,
                     toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the set action button.
    */
    static bool set(bits event_code, toolbox_action *action,
                    toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the cancel action button.
    */
    static bool cancel(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle);

    /*
        Parameters  : void
        Returns     : void
        Description : Load any backup configurations.
    */
    static void initialise();

    /*
        Parameters  : void
        Returns     : void
        Description : Unload any backup configurations.
    */
    static void finalise();
};

// A backup configuration
class backcfg_cfg
{
    backcfg_disc disc;                  // Details of the disc
    backcfg_obj *config;                // The backup configuration object
    toolbox_o obj;                      // Toolbox object ID for the window
    bool is_open;                       // Is the warning window open

    /*
        Parameters  : void
        Returns     : os_coord *    - Pointer to the coordinates of the top-left
                                      corner of the window, or NULL if not open.
        Description : Return the position of the current window.
    */
    const os_coord *get_top_left() const;

    /*
        Parameters  : void
        Returns     : void
        Description : Create a new window object if required.
    */
    void create();

    /*
        Parameters  : void
        Returns     : void
        Description : Destroy any window object.
    */
    void destroy();

    /*
        Parameters  : void
        Returns     : void
        Description : Reset the configuration displayed in the window.
    */
    void reset();

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Update the window.
    */
    static bool update(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle);

public:

    /*
        Parameters  : disc  - Details of the disc.
        Returns     : -
        Description : Constructor.
    */
    backcfg_cfg(const backcfg_disc &disc);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~backcfg_cfg();

    /*
        Parameters  : void
        Returns     : backcfg_obj   - The backup configuration object.
        Description : Redirect access to the underlying backup configuration
                      object.
    */
    backcfg_obj *operator->();

    /*
        Parameters  : void
        Returns     : backcfg_obj   - The backup configuration object.
        Description : Redirect access to the underlying backup configuration
                      object.
    */
    backcfg_obj &operator*();

    /*
        Parameters  : top_left  - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
        Returns     : void
        Description : Open the window for this backup configuration.
    */
    void open(const os_coord *top_left = NULL);

    /*
        Parameters  : void
        Returns     : void
        Description : Close the window for this backup configuration.
    */
    void close();

    /*
        Parameters  : disc  - Details of the disc.
        Returns     : bool  - Do the details match.
        Description : Check if this backup configuration matches the specified
                      disc.
    */
    bool eq(const backcfg_disc &disc) const;

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
    void store(string backup, string scrap, bool partial = FALSE);

    /*
        Parameters  : void
        Returns     : bool  - Is this backup configuration valid.
        Description : Check if this backup configuration is valid.
    */
    bool get_valid() const;

    /*
        Parameters  : void
        Returns     : bool  - Is an automatic backup due.
        Description : Check whether it is time to perform an automatic backup
                      of the disc associated with this configuration. This does
                      not take into account whether a backup has already been
                      started during the current session, or whether a
                      connection is active.
    */
    bool get_due() const;

    /*
        Parameters  : void
        Returns     : string    - The name of the previous backup file.
        Description : Return the name of the previous backup file. An empty
                      string is returned if no previous backup exists.
    */
    string get_prev() const;

    /*
        Parameters  : void
        Returns     : psifs_drive   - The drive.
        Description : Get the drive for this backup configuration.
    */
    psifs_drive get_drive() const;

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Initialise when the backup configuration window is shown.
    */
    static bool show(bits event_code, toolbox_action *action,
                     toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Tidy up when the backup configuration window is closed.
    */
    static bool hide(bits event_code, toolbox_action *action,
                     toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the media unique identifier set action button.
    */
    static bool set_media(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the disc name set action button.
    */
    static bool set_disc(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the drive letter set action button.
    */
    static bool set_drive(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the machine unique identifier set action button.
    */
    static bool set_machine(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the cancel action button.
    */
    static bool cancel(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle);

    /*
        Parameters  : id_block      - A toolbox ID block.
        Returns     : backcfg_cfg   - Pointer to the corresponding backup
                                      configuration, or NULL if not found.
        Description : Convert a toolbox ID block into a backup configuration
                      pointer.
    */
    static backcfg_cfg *find(const toolbox_block *id_block);
};

#endif
