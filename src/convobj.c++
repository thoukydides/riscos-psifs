/*
    File        : convobj.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : File converter management for the PsiFS filer.

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
#include "convobj.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"
#include "oslib/proginfo.h"
#include "oslib/wimpspriteop.h"

// Include cathlibcpp header files
#include "fstream.h"
#include "sstream.h"

// Include alexlib header files
#include "displayfield_c.h"
#include "stringset_c.h"

// Include project header files
#include "config.h"
#include "filer.h"
#include "fs.h"
#include "options.h"
#include "psifs.h"
#include "uid.h"
#include "wildcard.h"

// Converter configuration file tags
const char convobj_info_name[] = "Name";
const char convobj_info_author[] = "Author";
const char convobj_info_version[] = "Version";
const char convobj_info_purpose[] = "Purpose";
const char convobj_info_web[] = "WebSite";
const char convobj_tag[] = "Tag";
const char convobj_menu[] = "Description";
const char convobj_type_in[] = "Input";
const char convobj_uid_in[] = "InputUID";
const char convobj_name_in[] = "InputName";
const char convobj_type_out[] = "Output";
const char convobj_intercept_run[] = "InterceptRun";
const char convobj_intercept_load[] = "InterceptLoad";
const char convobj_intercept_save[] = "InterceptSave";
const char convobj_quality[] = "Quality";
const char convobj_options[] = "Options";
const char convobj_command[] = "Convert";
const char convobj_command_silent[] = "ConvertSilent";
const char convobj_button_text[] = "ButtonText";
const char convobj_button_help[] = "ButtonHelp";

// Directory containing converters
#define CONVOBJ_DIR "<PsiFSConverters$Dir>"

// Files within the converters directory
#define CONVOBJ_FILE_BOOT "!Boot"
#define CONVOBJ_FILE_DETAILS "!PsiFS"
#define CONVOBJ_FILE_RES "!PsiFSRes"

// Own !Boot file
#define CONVOBJ_PSIFS_BOOT "<PsiFS$Dir>.!Boot"

// Default converter quality
#define CONVOBJ_QUALITY_DEFAULT (25)

// Text to be substituted to run converter
static string convobj_sub_src("<src>");
static string convobj_sub_dest("<dest>");
static string convobj_sub_orig("<orig>");
static string convobj_sub_opt("<opt>");

// Separator between textual file types
#define CONVOBJ_TYPE_SEPARATOR "/"

// Special numeric file types
#define CONVOBJ_TYPE_UNKNOWN (0x10000000)
#define CONVOBJ_TYPE_SPECIAL (0x20000000)

// Special textual file types
#define CONVOBJ_TYPE_DIR "Directory"
#define CONVOBJ_TYPE_APPLICATION "Application"
#define CONVOBJ_TYPE_UNTYPED "Untyped"
#define CONVOBJ_TYPE_NO_CONVERSION "NoConversion"

// Special sprite names
#define CONVOBJ_SPRITE_DIR "directory"
#define CONVOBJ_SPRITE_APPLICATION "application"
#define CONVOBJ_SPRITE_UNTYPED "file_xxx"
#define CONVOBJ_SPRITE_SPECIAL ""

// Wildcarded system variable
#define CONVOBJ_SYS_VAR "PsiFSConverter$*"

// List of all known converters
convobj_list convobj_all;

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
convobj_types::convobj_types()
{
    // Ensure that the set is empty
    clear();
}

/*
    Parameters  : types - A space separated list of file types.
    Returns     : -
    Description : Constructor.
*/
convobj_types::convobj_types(string types) : set<bits, less<bits> >()
{
    // Ensure that the set is empty
    clear();

    // Process all of the types specified
    istringstream str(types);
    string name;
    while ((str >> name) && !name.empty())
    {
        // Add this type to the set
        bits type = from_name(name);
        if (type != CONVOBJ_TYPE_UNKNOWN) insert(type);
    }
}

/*
    Parameters  : types - The file types to merge.
    Returns     : void
    Description : Merge the specified list of types.
*/
void convobj_types::merge(const convobj_types &types)
{
    // Include all of the file types
    for (set_iterator<bits, less<bits> > i = types.begin();
         i != types.end();
         i++)
    {
        // Add this file type
        insert(*i);
    }
}

/*
    Parameters  : void
    Returns     : string    - The string corresponding to the file types.
    Description : Conversion to a string representation.
*/
string convobj_types::get_string() const
{
    string result;

    // Include all of the file types
    for (set_iterator<bits, less<bits> > i = begin(); i != end(); i++)
    {
        // Add this file type
        string name(to_name(*i));
        if (!name.empty())
        {
            if (!result.empty()) result += CONVOBJ_TYPE_SEPARATOR;
            result += name;
        }
    }

    // Return the result
    return result;
}

/*
    Parameters  : type  - The textual representation.
    Returns     : bits  - The numeric representation.
    Description : Convert a file type from its textual representation to
                  its numerical representation.
*/
bits convobj_types::from_name(string type)
{
    bits result;

    // Check for special cases first
    if (type == CONVOBJ_TYPE_DIR) result = osfile_TYPE_DIR;
    else if (type == CONVOBJ_TYPE_APPLICATION) result = osfile_TYPE_APPLICATION;
    else if (type == CONVOBJ_TYPE_UNTYPED) result = osfile_TYPE_UNTYPED;
    else if (type == CONVOBJ_TYPE_NO_CONVERSION) result = CONVOBJ_TYPE_SPECIAL;
    else
    {
        // Lookup the type string
        if (xosfscontrol_file_type_from_string(type.c_str(), &result))
        {
            result = CONVOBJ_TYPE_UNKNOWN;
        }
    }

    // Return the result
    return result;
}

/*
    Parameters  : type      - The numeric representation.
    Returns     : string    - The textual representation.
    Description : Convert a file type from its numeric representation to
                  its textual representation.
*/
string convobj_types::to_name(bits type)
{
    string result;

    // Check for special cases first
    if (type == osfile_TYPE_DIR) result = CONVOBJ_TYPE_DIR;
    else if (type == osfile_TYPE_APPLICATION) result = CONVOBJ_TYPE_APPLICATION;
    else if (type == osfile_TYPE_UNTYPED) result = CONVOBJ_TYPE_UNTYPED;
    else if (type == CONVOBJ_TYPE_SPECIAL) result = CONVOBJ_TYPE_NO_CONVERSION;
    else
    {
        // Lookup the type number
        char desc[9];
        if (!xosfscontrol_read_file_type(type, (bits *) desc,
                                         (bits *) (desc + 4)))
        {
            result.assign(desc, desc + 8);
        }
    }

    // Return the result
    return result;
}

/*
    Parameters  : type      - The numeric representation.
    Returns     : string    - The textual tag representation.
    Description : Convert a file type from its numeric representation to
                  its textual representation suitable for using as a tag.
*/
string convobj_types::to_tag(bits type)
{
    string result;

    // Check for special cases first
    if (type == osfile_TYPE_DIR) result = CONVOBJ_TYPE_DIR;
    else if (type == osfile_TYPE_APPLICATION) result = CONVOBJ_TYPE_APPLICATION;
    else if (type == osfile_TYPE_UNTYPED) result = CONVOBJ_TYPE_UNTYPED;
    else if (type == CONVOBJ_TYPE_SPECIAL) result = CONVOBJ_TYPE_NO_CONVERSION;
    else
    {
        // Convert to a three digit hexadecimal value
        char tag[10];
        sprintf(tag, "%03X", type);
        result = tag;
    }

    // Return the result
    return result;
}

/*
    Parameters  : type      - The numeric representation.
    Returns     : string    - The name of the sprite associated with this
                              file type.
    Description : Convert a file type from its numeric representation to
                  its sprite representation.
*/
string convobj_types::to_sprite(bits type)
{
    string result;

    // Handle special cases
    if (type == osfile_TYPE_DIR) result = CONVOBJ_SPRITE_DIR;
    else if (type == osfile_TYPE_APPLICATION) result = CONVOBJ_SPRITE_APPLICATION;
    else if (type == osfile_TYPE_UNTYPED) result = CONVOBJ_SPRITE_UNTYPED;
    else if (type == CONVOBJ_TYPE_SPECIAL) result = CONVOBJ_SPRITE_SPECIAL;
    else
    {
        // Construct the sprite name
        char name[9];
        sprintf(name, "file_%03x", type);
        result = name;

        // Check if the sprite exists
        if (xwimpspriteop_select_sprite(name, NULL))
        {
            // Use a default sprite if it does not
            result = CONVOBJ_SPRITE_UNTYPED;
        }
    }

    // Return the result
    return result;
}

/*
    Parameters  : lhs   - The first UID to compare.
                  rhs   - The second UID to compare.
    Returns     : bool  - Is lhs < rhs.
    Description : Compare two EPOC UIDs.
*/
bool operator<(const epoc32_file_uid &lhs, const epoc32_file_uid &rhs)
{
    // Compare the two UIDs
    return (lhs.uid1 < rhs.uid1)
           || ((lhs.uid1 == rhs.uid1)
               && ((lhs.uid2 < rhs.uid2)
                   || ((lhs.uid2 == rhs.uid2)
                       && (lhs.uid3 < rhs.uid3))));
}

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
convobj_uids::convobj_uids()
{
    // Ensure that the set is empty
    clear();
}

/*
    Parameters  : uids  - A space separated list of UIDs.
    Returns     : -
    Description : Constructor.
*/
convobj_uids::convobj_uids(string uids)
{
    // Ensure that the set is empty
    clear();

    // Process all of the UIDs specified
    istringstream str(uids);
    string text;
    while ((str >> text) && !text.empty())
    {
        // Add this UID to the set if valid
        const epoc32_file_uid *uid = from_text(text);
        if (uid) insert(*uid);
    }
}

/*
    Parameters  : uid               - The textual representation.
    Returns     : epoc32_file_uid   - The numeric representation, or NULL
                                      if not valid.
    Description : Convert a UID from its textual representation to its
                  numerical representation.
*/
const epoc32_file_uid *convobj_uids::from_text(string uid)
{
    static epoc32_file_uid value;

    // Attempt to parse the UID string
    return uid_parse_uid(uid.c_str(), &value) ? NULL : &value;
}

/*
    Parameters  : void
    Returns     : -
    Description : Constructor.
*/
convobj_names::convobj_names()
{
    // Ensure that the set is empty
    clear();
}

/*
    Parameters  : names - A space separated list of wildcarded names.
    Returns     : -
    Description : Constructor.
*/
convobj_names::convobj_names(string names)
{
    // Ensure that the set is empty
    clear();

    // Process all of the names specified
    istringstream str(names);
    string name;
    while ((str >> name) && !name.empty())
    {
        // Add this name to the set
        insert(name);
    }
}

/*
    Parameters  : name  - The name to check.
    Returns     : bool  - Does the name match any of the wildcarded names.
    Description : Check for a matching name.
*/
bool convobj_names::match(string name)
{
    bool match = FALSE;

    // Loop though all of the patterns until matched
    for (set_iterator<string, less<string> > i = begin();
         !match && (i != end());
         i++)
    {
        // Check for a match
        match = !wildcard_cmp((*i).c_str(), name.c_str());
    }

    // Return the result
    return match;
}

/*
    Parameters  : details   - The details associated with this converter.
    Returns     : -
    Description : Constructor.
*/
convobj_obj::convobj_obj(tag_store &details)
: tag_store(details),
  type_in(get_str(convobj_type_in)),
  uid_in(get_str(convobj_uid_in)),
  name_in(get_str(convobj_name_in)),
  type_out(get_str(convobj_type_out)),
  intercept_run(get_str(convobj_intercept_run)),
  intercept_load(get_str(convobj_intercept_load)),
  intercept_save(get_str(convobj_intercept_save)),
  supports_silent(exist(convobj_command_silent))
{
    // Ensure that all intercept types are treated as inputs
    type_in.merge(intercept_run);
    type_in.merge(intercept_load);
    type_in.merge(intercept_save);

    // Add any essential missing fields
    if (!exist(convobj_info_name))
    {
        // Unknown name
        set_str(convobj_info_name, get_str(convobj_tag));
    }
    if (!exist(convobj_info_author))
    {
        // Unknown author
        set_str(convobj_info_author, filer_msgtrans("CnvUnAt"));
    }
    if (!exist(convobj_info_version))
    {
        // Unknown version
        set_str(convobj_info_version, filer_msgtrans("CnvUnVr"));
    }
    if (!exist(convobj_menu))
    {
        // Unknown menu description
        set_str(convobj_menu, filer_msgtrans("CnvMenu", type_in.get_string().c_str(), type_out.get_string().c_str(), get_str(convobj_info_name).c_str()));
    }
    if (!exist(convobj_info_purpose))
    {
        // Unknown purpose
        set_str(convobj_info_purpose, get_str(convobj_menu));
    }
    if (!exist(convobj_quality))
    {
        // Unknown quality
        set_num(convobj_quality, CONVOBJ_QUALITY_DEFAULT);
    }
}

/*
    Parameters  : src       - The source file name.
                  dest      - The destination file name.
                  orig      - The suggested leaf name.
                  opt       - The options for the conversion.
                  silent    - Should a silent conversion be performed if
                              possible,
    Returns     : bool      - Was the conversion successful.
    Description : Attempt to perform the conversion.
*/
bool convobj_obj::run(string src, string dest, string orig, string opt, bool silent) const
{
    // Ensure that the output file does not already exist
    xosfscontrol_wipe(dest.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);

    // Remove any path from the original leaf name
    string::size_type pos = orig.rfind(FS_CHAR_SEPARATOR);
    if (pos != string::npos) orig.erase(0, pos + 1);

    // Choose a silent or non-silent conversion
    string cmd = get_str(silent && exist(convobj_command_silent)
                         ? convobj_command_silent : convobj_command);

    // Construct the command to execute
    pos = cmd.find(convobj_sub_src);
    if (pos != string::npos) cmd.replace(pos, convobj_sub_src.length(), src);
    else cmd += " " + src;
    pos = cmd.find(convobj_sub_dest);
    if (pos != string::npos) cmd.replace(pos, convobj_sub_dest.length(), dest);
    else cmd += " " + dest;
    pos = cmd.find(convobj_sub_orig);
    if (pos != string::npos) cmd.replace(pos, convobj_sub_orig.length(), orig);
    pos = cmd.find(convobj_sub_opt);
    if (pos != string::npos) cmd.replace(pos, convobj_sub_opt.length(), opt);

    // Run the converter
    wimp_start_task(cmd.c_str());

    // Check if the output file was produced
    bool success = FALSE;
    fileswitch_object_type type;
    if (!xosfile_read_stamped_no_path(dest.c_str(), &type,
                                      NULL, NULL, NULL, NULL, NULL)
        && (type != fileswitch_NOT_FOUND))
    {
        success = TRUE;
    }

    // Return the result
    return success;
}

/*
    Parameters  : obj   - The program information object.
    Returns     : void
    Description : Set the fields of the specified program information
                  window with details for this converter.
*/
void convobj_obj::update_info(toolbox_o obj) const
{
    // Get the underlying window object
    toolbox_o win = proginfo_get_window_id(0, obj);

    // Set all of the standard fields
    displayfield_c(proginfo_NAME, win) = get_str(convobj_info_name);
    displayfield_c(proginfo_PURPOSE, win) = get_str(convobj_info_purpose);
    displayfield_c(proginfo_AUTHOR, win) = get_str(convobj_info_author);
    displayfield_c(proginfo_VERSION, win) = get_str(convobj_info_version);

    // Set the web site button
    proginfo_set_uri(0, obj, get_str(convobj_info_web).c_str());
}

/*
    Parameters  : void
    Returns     : bits  - The output file type.
    Description : Return the type of file produced as output by the converter.
*/
bits convobj_obj::get_type() const
{
    // Return the output type
    return *type_out.begin();
}

/*
    Parameters  : void
    Returns     : bool  - Is the converter valid.
    Description : Check if the converter is valid.
*/
bool convobj_obj::get_valid() const
{
    // Check that suitable details have been specified
    return !get_str(convobj_tag).empty()
           && !type_in.empty() && (type_out.size() == 1)
           && !get_str(convobj_command).empty();
}

/*
    Parameters  : -
    Returns     : -
    Description : Constructor.
*/
convobj_list::convobj_list()
{
    // No initialisation required
}

/*
    Parameters  : list      - The list of converters to base this list on.
                  type      - The type of file to be converted.
                  uid       - The UID of the file to be converted.
                  leaf      - The original leaf name of the file to be
                              converted.
                  dir       - Should converters that produce directories be
                              included.
                  special   - Should special converters that do not produce
                              output files be included.
    Returns     : -
    Description : Constructor.
*/
convobj_list::convobj_list(const convobj_list &list, bits type,
                           const epoc32_file_uid &uid, string orig,
                           bool dir, bool special)
{
    // Remove any path from the original leaf name
    string::size_type pos = orig.rfind(FS_CHAR_SEPARATOR);
    if (pos != string::npos) orig.erase(0, pos + 1);

    // Check all of the list entries
    for (const convobj_ptr *i = list.begin(); i != list.end(); i++)
    {
        // Add this converter to the list if the filetype matches
        if ((*i)->type_in.count(type)
            && ((*i)->uid_in.empty() || (*i)->uid_in.count(uid))
            && ((*i)->name_in.empty() || (*i)->name_in.match(orig))
            && (dir || ((*i)->get_type() != osfile_TYPE_DIR))
            && (special || ((*i)->get_type() != CONVOBJ_TYPE_SPECIAL)))
        {
            push_back(*i);
        }
    }
}

/*
    Parameters  : obj   - The window object containing the stringset.
                  cmp   - The component ID of the stringset.
    Returns     : void
    Description : Set the menu options for the specified stringset to this
                  list of converters.
*/
void convobj_list::update_menu(toolbox_o obj, toolbox_c cmp) const
{
    // Build a list of converter names
    list<string> menu;
    for (const convobj_ptr *i = begin(); i != end(); i++)
    {
        menu.push_back((*i)->get_str(convobj_menu));
    }

    // Add a no conversion option
    menu.push_front(filer_msgtrans("CnvNone"));

    // Set the stringset control
    stringset_c(cmp, obj).set_available(menu);

    // Default to no conversion
    stringset_c(cmp, obj) = 0;
}

/*
    Parameters  : obj   - The window object containing the stringset.
                  cmp   - The component ID of the stringset.
                  conv  - The tag associated with the converter, or an
                          empty string to select no conversion.
    Returns     : bool  - Was a converter selected.
    Description : Attempt to select the specified converter.
*/
bool convobj_list::set_menu(toolbox_o obj, toolbox_c cmp, string conv) const
{
    bool selected = FALSE;

    // Special case if no conversion required
    if (conv.empty())
    {
        // Select no conversion
        stringset_c(cmp, obj) = 0;
        selected = TRUE;
    }
    else
    {
        // Attempt to find a matching conversion
        for (const convobj_ptr *i = begin(); !selected && (i != end()); i++)
        {
            if ((*i)->get_str(convobj_tag) == conv)
            {
                stringset_c(cmp, obj) = (i - begin()) + 1;
                selected = TRUE;
            }
        }
    }

    // Return whether the converter was selected
    return selected;
}

/*
    Parameters  : obj           - The window object containing the
                                  stringset.
                  cmp           - The component ID of the stringset.
    Returns     : convobj_obj   - The selected converter, or NULL if no
                                  conversion is selected.
    Description : Find the currently selected converter.
*/
convobj_obj *convobj_list::get_menu(toolbox_o obj, toolbox_c cmp) const
{
    // Find the index of the selected menu item
    bits index = stringset_c(cmp, obj).gadget_w_number::get_value();

    // Return the corresponding converter
    return index ? at(index - 1) : NULL;
}

/*
    Parameters  : report    - Should any errors be reported.
    Returns     : void
    Description : Update the file transfer intercepts based on this list of
                  converters and the current configuration.
*/
void convobj_list::update_intercepts(bool report) const
{
    os_error *err = NULL;

    // Construct the list of all interesting types
    map<bits, psifs_intercept_type, less<bits> > masks;
    for (const convobj_ptr *i = begin(); i != end(); i++)
    {
        set_iterator<bits, less<bits> > j;

        // Include file types supported when forced
        for (j = (*i)->type_in.begin(); j != (*i)->type_in.end(); j++)
        {
            masks[*j] = masks[*j] | psifs_INTERCEPT_FORCED;
        }

        // Include file types to intercept when run
        for (j = (*i)->intercept_run.begin();
             j != (*i)->intercept_run.end();
             j++)
        {
            if (config_current.get_bool(config_tag_intercept_run))
            {
                masks[*j] = masks[*j] | psifs_INTERCEPT_RUN_UNCLAIMED;
            }
            if (config_current.get_bool(config_tag_intercept_run_all))
            {
                masks[*j] = masks[*j] | psifs_INTERCEPT_RUN_ALL;
            }
        }

        // Include file types to intercept when loaded
        for (j = (*i)->intercept_load.begin();
             j != (*i)->intercept_load.end();
             j++)
        {
            if (config_current.get_bool(config_tag_intercept_load))
            {
                    masks[*j] = masks[*j] | psifs_INTERCEPT_LOAD;
            }
        }

        // Include file types to intercept when saved or transferred
        for (j = (*i)->intercept_save.begin();
             j != (*i)->intercept_save.end();
             j++)
        {
            if (config_current.get_bool(config_tag_intercept_save))
            {
                masks[*j] = masks[*j] | psifs_INTERCEPT_SAVE;
            }
            if (config_current.get_bool(config_tag_intercept_transfer))
            {
                masks[*j] = masks[*j] | psifs_INTERCEPT_TRANSFER;
            }
        }
    }

    // Release all previous intercept claims
    err = xpsifs_intercept_release(filer_poll_word);

    // Claim all of the new intercepts
    map_iterator<bits, psifs_intercept_type, less<bits> > k;
    for (k = masks.begin(); !err && (k != masks.end()); k++)
    {
        err = xpsifs_intercept_claim(filer_poll_word, (*k).first, (*k).second);
    }

    // Throw an error if required
    if (report && err) os_generate_error(err);
}

/*
    Parameters  : dir   - The directory to initialise.
    Returns     : void
    Description : Initialise the file converters contained within the specified
                  directory.
*/
static void convobj_init_dir(string dir)
{
    // Attempt to read the converter details file
    tag_store options;
    tag_store common;
    ifstream file((dir + FS_CHAR_SEPARATOR + CONVOBJ_FILE_DETAILS).c_str());
    if (file >> options)
    {
        // Merge any new tags into the current options
        options.merge(options_current);
        options_current.merge(options);
    }

    // Attempt to read the common converter tags
    if (file >> common)
    {
        // Attempt to load any resource file
        xtoolbox_load_resources(0, (dir + FS_CHAR_SEPARATOR + CONVOBJ_FILE_RES).c_str());
    }

    // Loop through all of the converters
    tag_store body;
    while (file >> body)
    {
        // Combine the header and individual details
        tag_store details(common);
        details.merge(body);

        // Create a new converter
        convobj_obj *converter = new convobj_obj(details);

        // Add it to the list if valid
        if (converter->get_valid()) convobj_all.push_back(converter);
        else
        {
            // Output details of bad converters
            cerr << "Bad converter configuration in '" << dir << "':" << endl
                 << *converter << endl;
            delete converter;
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Boot all embedded file converters.
*/
static void convobj_init_boot()
{
    // Loop through all directories
    int context = 0;
    int read;
    fs_info info;
    while (!xosgbpb_dir_entries_info(CONVOBJ_DIR,
                                     (osgbpb_info_list *) &info, 1, context,
                                     sizeof(info), NULL, &read, &context)
           && (context != -1))
    {
        // Check if a directory was found
        if (read && (info.obj_type == fileswitch_IS_DIR))
        {
            // Run any !Boot file inside the directory
            string boot(string(CONVOBJ_DIR) + FS_CHAR_SEPARATOR + info.name + FS_CHAR_SEPARATOR + CONVOBJ_FILE_BOOT);
            fileswitch_object_type type;
            if (!xosfile_read_stamped_no_path(boot.c_str(), &type,
                                              NULL, NULL, NULL, NULL, NULL)
                && (type != fileswitch_NOT_FOUND))
            {
                xwimp_start_task(boot.c_str(), NULL);
            }
        }
    }

    // Run own !Boot file to restore file sprites
    xwimp_start_task(CONVOBJ_PSIFS_BOOT, NULL);
}

/*
    Parameters  : void
    Returns     : void
    Description : Enumerate all converter variables.
*/
static void convobj_init_enum()
{
    // Enumerate through all matching variables
    fs_pathname path;
    int used;
    int context = 0;
    while (!xos_read_var_val(CONVOBJ_SYS_VAR, path, sizeof(path) - 1, context,
                             os_VARTYPE_EXPANDED, &used, &context, NULL))
    {
        // Initialise the converter
        path[used] = '\0';
        convobj_init_dir(path);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the file converters. This must be called from a
                  live WIMP task to allow Wimp_StartTask to be used.
*/
void convobj_init()
{
    // Boot all embedded converters
    convobj_init_boot();

    // Enumerate file converters
    convobj_init_enum();

    // Set initial file intercepts
    convobj_all.update_intercepts(TRUE);
}
