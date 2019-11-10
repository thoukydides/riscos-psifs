/*
    File        : convobj.h
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

// Only include header file once
#ifndef CONVOBJ_H
#define CONVOBJ_H

// Include cathlibcpp header files
#include "set.h"
#include "vector.h"

// Include oslib header files
#include "oslib/toolbox.h"

// Include project header files
#include "fs.h"
#include "tag.h"

// Converter configuration file tags
extern const char convobj_info_name[];
extern const char convobj_info_author[];
extern const char convobj_info_version[];
extern const char convobj_info_purpose[];
extern const char convobj_info_web[];
extern const char convobj_tag[];
extern const char convobj_menu[];
extern const char convobj_type_in[];
extern const char convobj_uid_in[];
extern const char convobj_name_in[];
extern const char convobj_type_out[];
extern const char convobj_intercept_run[];
extern const char convobj_intercept_load[];
extern const char convobj_intercept_save[];
extern const char convobj_quality[];
extern const char convobj_options[];
extern const char convobj_command[];
extern const char convobj_command_silent[];
extern const char convobj_button_text[];
extern const char convobj_button_help[];

// A set of types
class convobj_types : public set<bits, less<bits> >
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    convobj_types();

    /*
        Parameters  : types - A space separated list of file types.
        Returns     : -
        Description : Constructor.
    */
    convobj_types(string types);

    /*
        Parameters  : void
        Returns     : -
        Description : Destructor.
    */
    ~convobj_types() {}

    /*
        Parameters  : types - The file types to merge.
        Returns     : void
        Description : Merge the specified list of types.
    */
    void merge(const convobj_types &types);

    /*
        Parameters  : void
        Returns     : string    - The string corresponding to the file types.
        Description : Conversion to a string representation.
    */
    string get_string() const;

    /*
        Parameters  : type  - The textual representation.
        Returns     : bits  - The numeric representation.
        Description : Convert a file type from its textual representation to
                      its numerical representation.
    */
    static bits from_name(string type);

    /*
        Parameters  : type      - The numeric representation.
        Returns     : string    - The textual representation.
        Description : Convert a file type from its numeric representation to
                      its textual representation.
    */
    static string to_name(bits type);

    /*
        Parameters  : type      - The numeric representation.
        Returns     : string    - The textual tag representation.
        Description : Convert a file type from its numeric representation to
                      its textual representation suitable for using as a tag.
    */
    static string to_tag(bits type);

    /*
        Parameters  : type      - The numeric representation.
        Returns     : string    - The name of the sprite associated with this
                                  file type.
        Description : Convert a file type from its numeric representation to
                      its sprite representation.
    */
    static string to_sprite(bits type);
};

/*
    Parameters  : lhs   - The first UID to compare.
                  rhs   - The second UID to compare.
    Returns     : bool  - Is lhs < rhs.
    Description : Compare two EPOC UIDs.
*/
bool operator<(const epoc32_file_uid &lhs, const epoc32_file_uid &rhs);

// A set of EPOC UIDs
class convobj_uids : public set<epoc32_file_uid, less<epoc32_file_uid> >
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    convobj_uids();

    /*
        Parameters  : uids  - A space separated list of UIDs.
        Returns     : -
        Description : Constructor.
    */
    convobj_uids(string uids);

    /*
        Parameters  : void
        Returns     : -
        Description : Destructor.
    */
    ~convobj_uids() {}

    /*
        Parameters  : uid               - The textual representation.
        Returns     : epoc32_file_uid   - The numeric representation, or NULL
                                          if not valid.
        Description : Convert a UID from its textual representation to its
                      numerical representation.
    */
    static const epoc32_file_uid *from_text(string uid);
};

// A set of wildcarded leaf names
class convobj_names : public set<string, less<string> >
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    convobj_names();

    /*
        Parameters  : names - A space separated list of wildcarded names.
        Returns     : -
        Description : Constructor.
    */
    convobj_names(string names);

    /*
        Parameters  : void
        Returns     : -
        Description : Destructor.
    */
    ~convobj_names() {}

    /*
        Parameters  : name  - The name to check.
        Returns     : bool  - Does the name match any of the wildcarded names.
        Description : Check for a matching name.
    */
    bool match(string name);
};

// A single converter
class convobj_obj : public tag_store
{
public:

    convobj_types type_in;              // Supported input file types
    convobj_uids uid_in;                // Supported input UIDs
    convobj_names name_in;              // Supported input leaf names
    convobj_types type_out;             // File type of result
    convobj_types intercept_run;        // File types to intercept when run
    convobj_types intercept_load;       // File types to intercept when loaded
    convobj_types intercept_save;       // File types to intercept when saved
    bool supports_silent;               // Can silent conversions be performed

    /*
        Parameters  : details   - The details associated with this converter.
        Returns     : -
        Description : Constructor.
    */
    convobj_obj(tag_store &details);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~convobj_obj() {}

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
    bool run(string src, string dest, string orig, string opt,
             bool silent = FALSE) const;

    /*
        Parameters  : obj   - The program information object.
        Returns     : void
        Description : Set the fields of the specified program information
                      window with details for this converter.
    */
    void update_info(toolbox_o obj) const;

    /*
        Parameters  : void
        Returns     : bits  - The output file type.
        Description : Return the type of file produced as output by the
                      converter.
    */
    bits get_type() const;

    /*
        Parameters  : void
        Returns     : bool  - Is the converter valid.
        Description : Check if the converter is valid.
    */
    bool get_valid() const;
};

// Pointer to a converter
typedef convobj_obj *convobj_ptr;

// A list of converters
class convobj_list : public vector<convobj_obj *>
{
public:

    /*
        Parameters  : -
        Returns     : -
        Description : Constructor.
    */
    convobj_list();

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
    convobj_list(const convobj_list &list, bits type,
                 const epoc32_file_uid &uid, string orig,
                 bool dir, bool special);

    /*
        Parameters  :
        Returns     : -
        Description : Destructor.
    */
    ~convobj_list() {}

    /*
        Parameters  : obj   - The window object containing the stringset.
                      cmp   - The component ID of the stringset.
        Returns     : void
        Description : Set the menu options for the specified stringset to this
                      list of converters.
    */
    void update_menu(toolbox_o obj, toolbox_c cmp) const;

    /*
        Parameters  : obj   - The window object containing the stringset.
                      cmp   - The component ID of the stringset.
                      conv  - The tag associated with the converter, or an
                              empty string to select no conversion.
        Returns     : bool  - Was a converter selected.
        Description : Attempt to select the specified converter.
    */
    bool set_menu(toolbox_o obj, toolbox_c cmp, string conv) const;

    /*
        Parameters  : obj           - The window object containing the
                                      stringset.
                      cmp           - The component ID of the stringset.
        Returns     : convobj_obj   - The selected converter, or NULL if no
                                      conversion is selected.
        Description : Find the currently selected converter.
    */
    convobj_obj *get_menu(toolbox_o obj, toolbox_c cmp) const;

    /*
        Parameters  : report    - Should any errors be reported.
        Returns     : void
        Description : Update the file transfer intercepts based on this list of
                      converters and the current configuration.
    */
    void update_intercepts(bool report) const;
};

// List of all known converters
extern convobj_list convobj_all;

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the file converters. This must be called from a
                  live WIMP task to allow Wimp_StartTask to be used.
*/
void convobj_init();

#endif
