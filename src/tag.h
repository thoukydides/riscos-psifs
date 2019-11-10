/*
    File        : tag.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Tagged string and numeric value handling for the PsiFS
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

// Only include header file once
#ifndef TAG_H
#define TAG_H

// Include cathlibcpp header files
#include "iostream.h"
#include "functional.h"
#include "map.h"
#include "string.h"

// Include oslib header files
#include "oslib/types.h"

// Include project header files
#include "date.h"

// A tag store
class tag_store
{
public:

    // The tagged data
    map<string, string, less<string> > data;

    /*
        Parameters  : void
        Returns     : void
        Description : Clear all tags.
    */
    void clear(void);

    /*
        Parameters  : tag   - The tag to clear.
        Returns     : void
        Description : Clear the specified tags.
    */
    void clear(string tag);

    /*
        Parameters  : tag   - The tag to check.
        Returns     : void
        Description : Check whether the specified tag exists.
    */
    bool exist(string tag) const;

    /*
        Parameters  : tag       - The tag to get.
        Returns     : string    - The value of the tag.
        Description : Get the value of the specified tag.
    */
    string get_str(string tag) const;

    /*
        Parameters  : tag   - The tag to set.
                      value - The string value for this tag.
        Returns     : void
        Description : Set the value of the specified tag.
    */
    void set_str(string tag, string value);

    /*
        Parameters  : tag   - The tag to get.
        Returns     : bits  - The value of the tag.
        Description : Get the value of the specified tag.
    */
    bits get_num(string tag) const;

    /*
        Parameters  : tag   - The tag to set.
                      value - The numeric value for this tag.
        Returns     : void
        Description : Set the value of the specified tag.
    */
    void set_num(string tag, bits value);

    /*
        Parameters  : tag   - The tag to get.
        Returns     : bool  - Value of the tag.
        Description : Get the value of the specified tag.
    */
    bool get_bool(string tag) const;

    /*
        Parameters  : tag   - The tag to set.
                      value - The boolean value for this tag.
        Returns     : void
        Description : Set the value of the specified tag.
    */
    void set_bool(string tag, bool value);

    /*
        Parameters  : tag           - The tag to get.
        Returns     : date_riscos   - Value of the tag.
        Description : Get the value of the specified tag.
    */
    date_riscos get_date(string tag) const;

    /*
        Parameters  : tag   - The tag to set.
                      value - The date value for this tag.
        Returns     : void
        Description : Set the value of the specified tag.
    */
    void set_date(string tag, date_riscos value);

    /*
        Parameters  : x - The tags to merge.
        Returns     : void
        Description : Merge the specified tags.
    */
    void merge(const tag_store &x);

    /*
        Parameters  : void
        Returns     : string    - A single string containing all of the data.
        Description : Convert the tag store tag-value pairs into a suitable
                      format for a command line.
    */
    string operator()() const;

    /*
        Parameters  : value     - The value to set.
        Returns     : tag_store - This object.
        Description : Set the value of this tag store. The string should be in
                      the same format as returned by operator().
    */
    tag_store &operator=(string value);

    /*
        Parameters  : tag1      - The first tag component.
                      tag2      - The second tag component.
                      tag3      - The third tag component.
                      tag4      - The fourth tag component.
                      tag5      - The fifth tag component.
        Returns     : string    - The combined tag.
        Description : Construct a composite tag from up to five components.
                      Note that not specifying a tag component is not the same
                      as specifying a blank tag component.
    */
    static string tag(string tag1);
    static string tag(string tag1, string tag2);
    static string tag(string tag1, string tag2, string tag3);
    static string tag(string tag1, string tag2, string tag3, string tag4);
    static string tag(string tag1, string tag2, string tag3, string tag4,
                      string tag5);

    /*
        Parameters  : s - The stream to input from.
        Returns     : c - The tag class to update.
        Description : Attempt to read the tags from a stream.
    */
    friend istream &operator>>(istream &s, tag_store &c);

    /*
        Parameters  : s - The stream to output to.
        Returns     : c - The tag class to output.
        Description : Attempt to write the tags to a stream.
    */
    friend ostream &operator<<(ostream &s, const tag_store &c);
};

#endif
