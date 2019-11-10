/*
    File        : tag.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Tagged string and numeric value handling for the PsiFS filer.

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

// Prevent OSLib/CathLibCPP name clashes
#include "noclash.h"

// Include header file for this module
#include "tag.h"

// Include oslib header files
#include "oslib/territory.h"

// Include cathlibcpp header files
#include "sstream.h"

// End of file marker
static const string tag_end("<EOF>");

// Tag/value separator
static const string tag_separator("=");

// True and false strings
static const string tag_true("True");
static const string tag_false("False");

// Composite tag components
static const string tag_tag_invalid("\n\r =");
static const string tag_tag_cat("_");
static const string tag_tag_null("(null)");

// Command line mapping components
static const string tag_cmd_keyword("-");
static const string tag_cmd_quote("\"");
static const string tag_cmd_separator(" ");

/*
    Parameters  : void
    Returns     : void
    Description : Clear all tags.
*/
void tag_store::clear(void)
{
    data.clear();
}

/*
    Parameters  : tag   - The tag to clear.
    Returns     : void
    Description : Clear the specified tags.
*/
void tag_store::clear(string tag)
{
    data.erase(tag);
}

/*
    Parameters  : tag   - The tag to check.
    Returns     : void
    Description : Check whether the specified tag exists.
*/
bool tag_store::exist(string tag) const
{
    return data.find(tag) != data.end();
}

/*
    Parameters  : tag       - The tag to get.
    Returns     : string    - The value of the tag.
    Description : Get the value of the specified tag.
*/
string tag_store::get_str(string tag) const
{
    return exist(tag) ? data[tag] : string("");
}

/*
    Parameters  : tag   - The tag to set.
                  value - The string value for this tag.
    Returns     : void
    Description : Set the value of the specified tag.
*/
void tag_store::set_str(string tag, string value)
{
    data[tag] = value;
}

/*
    Parameters  : tag   - The tag to get.
    Returns     : bits  - The value of the tag.
    Description : Get the value of the specified tag.
*/
bits tag_store::get_num(string tag) const
{
    bits value = 0;
    if (exist(tag))
    {
        istringstream str(data[tag]);
        str >> value;
    }
    return value;
}

/*
    Parameters  : tag   - The tag to set.
                  value - The numeric value for this tag.
    Returns     : void
    Description : Set the value of the specified tag.
*/
void tag_store::set_num(string tag, bits value)
{
    ostringstream str;
    str << value;
    data[tag] = str.str();
}

/*
    Parameters  : tag   - The tag to get.
    Returns     : bool  - Value of the tag.
    Description : Get the value of the specified tag.
*/
bool tag_store::get_bool(string tag) const
{
    return exist(tag) && (data[tag] == tag_true);
}

/*
    Parameters  : tag   - The tag to set.
                  value - The boolean value for this tag.
    Returns     : void
    Description : Set the value of the specified tag.
*/
void tag_store::set_bool(string tag, bool value)
{
    data[tag] = value ? tag_true : tag_false;
}

/*
    Parameters  : tag           - The tag to get.
    Returns     : date_riscos   - Value of the tag.
    Description : Get the value of the specified tag.
*/
date_riscos tag_store::get_date(string tag) const
{
    date_riscos value;
    memset(&value, sizeof(value), 0);
    if (exist(tag))
    {
        territory_ordinals ordinals;
        istringstream str(data[tag]);
        str >> ordinals.date
            >> ordinals.month
            >> ordinals.year
            >> ordinals.hour
            >> ordinals.minute
            >> ordinals.second
            >> ordinals.centisecond;
        if (str)
        {
            territory_convert_ordinals_to_time(territory_CURRENT,
                                               (os_date_and_time *) value.bytes,
                                               &ordinals);
        }
    }
    return value;
}

/*
    Parameters  : tag   - The tag to set.
                  value - The date value for this tag.
    Returns     : void
    Description : Set the value of the specified tag.
*/
void tag_store::set_date(string tag, date_riscos value)
{
    territory_ordinals ordinals;
    ostringstream str;
    territory_convert_time_to_ordinals(territory_CURRENT,
                                       (os_date_and_time *) value.bytes,
                                       &ordinals);
    str << ordinals.date << ' '
        << ordinals.month << ' '
        << ordinals.year << ' '
        << ordinals.hour << ' '
        << ordinals.minute << ' '
        << ordinals.second << ' '
        << ordinals.centisecond;
    data[tag] = str.str();
}

/*
    Parameters  : x - The tags to merge.
    Returns     : void
    Description : Merge the specified tags.
*/
void tag_store::merge(const tag_store &x)
{
    for (map_const_iterator<string, string, less<string> > i = x.data.begin();
         i != x.data.end();
         i++)
    {
        data[(*i).first] = (*i).second;
    }
}

/*
    Parameters  : void
    Returns     : string    - A single string containing all of the data.
    Description : Convert the tag store tag-value pairs into a suitable
                  format for a command line.
*/
string tag_store::operator()() const
{
    string result;
    for (map_const_iterator<string, string, less<string> > i = data.begin();
         i != data.end();
         i++)
    {
        // Add a separator between values
        if (!result.empty()) result += tag_cmd_separator;

        // Append the next keyword
        result += tag_cmd_keyword + (*i).first;

        // Add any value
        string value((*i).second);
        if (!value.empty())
        {
            // Add a separator between the tag and value
            result += tag_cmd_separator;

            // Decide whether the value needs to be quoted
            bool quote = FALSE;
            if (value.find(tag_cmd_separator) != string::npos) quote = TRUE;
            if (value.find(tag_cmd_keyword) == 0) quote = TRUE;
            string::size_type pos = value.find(tag_cmd_quote);
            while (pos != string::npos)
            {
                quote = TRUE;
                value.insert(pos, tag_cmd_quote);
                pos = value.find(tag_cmd_quote, pos + 2);
            }

            // Append the value
            if (quote) result += tag_cmd_quote + value + tag_cmd_quote;
            else result += value;
        }
    }

    // Return the result
    return result;
}

/*
    Parameters  : value     - The value to set.
    Returns     : tag_store - This object.
    Description : Set the value of this tag store. The string should be in
                  the same format as returned by operator().
*/
tag_store &tag_store::operator=(string value)
{
    // Start by deleting any existing tags
    clear();

    // Loop until the whole string has been processed
    while (!value.empty())
    {
        string keyword;
        string arg;

        // Skip any initial white space
        while (value.find(tag_cmd_separator) == 0)
        {
            value.erase(0, tag_cmd_separator.length());
        }

        // Check for a keyword marker
        if (value.find(tag_cmd_keyword) == 0)
        {
            // Delete the keyword marker
            value.erase(0, tag_cmd_keyword.length());

            // Separate the keyword
            string::size_type pos = value.find(tag_cmd_separator);
            keyword = value.substr(0, pos);
            value.erase(0, pos);
        }

        // Skip any more white space
        while (value.find(tag_cmd_separator) == 0)
        {
            value.erase(0, tag_cmd_separator.length());
        }

        // Check for a quoted string
        if (value.find(tag_cmd_quote) == 0)
        {
            // Process a quoted value
            value.erase(0, tag_cmd_quote.length());
            while (!value.empty()
                   && ((value.find(tag_cmd_quote) != 0)
                       || (value.find(tag_cmd_quote + tag_cmd_quote) == 0)))
            {
                // Special case of a quoted quote
                if (value.find(tag_cmd_quote) == 0)
                {
                    arg += tag_cmd_quote;
                    value.erase(0, tag_cmd_quote.length() * 2);
                }
                else
                {
                    arg += value.substr(0, 1);
                    value.erase(0, 1);
                }
            }
            value.erase(0, tag_cmd_quote.length());
        }
        else if (value.find(tag_cmd_keyword) != 0)
        {
            // Process any non-quoted value
            string::size_type pos = value.find(tag_cmd_separator);
            arg = value.substr(0, pos);
            value.erase(0, pos);
        }

        // Store this keyword if valid
        if (!keyword.empty()) data[keyword] = arg;
        else value = "";
    }

    // Return this tag store
    return *this;
}

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
string tag_store::tag(string tag1)
{
    // Remove any unsuitable characters
    string::size_type pos = tag1.find_first_of(tag_tag_invalid);
    while (pos != string::npos)
    {
        // Delete invalid characters
        tag1.erase(pos, 1);

        // Check for any more unsuitable characters
        pos = tag1.find_first_of(tag_tag_invalid);
    }

    // Check for an empty tag
    if (tag1.empty()) tag1 = tag_tag_null;

    // Just check for a blank tag
    return tag1;
}
string tag_store::tag(string tag1, string tag2)
{
    // Combine the tags
    return tag(tag1) + tag_tag_cat + tag(tag2);
}
string tag_store::tag(string tag1, string tag2, string tag3)
{
    // Combine the tags
    return tag(tag1, tag2) + tag_tag_cat + tag(tag3);
}
string tag_store::tag(string tag1, string tag2, string tag3, string tag4)
{
    // Combine the tags
    return tag(tag1, tag2, tag3) + tag_tag_cat + tag(tag4);
}
string tag_store::tag(string tag1, string tag2, string tag3, string tag4,
                      string tag5)
{
    // Combine the tags
    return tag(tag1, tag2, tag3, tag4) + tag_tag_cat + tag(tag5);
}


/*
    Parameters  : s - The stream to input from.
    Returns     : c - The tag class to update.
    Description : Attempt to read the tags from a stream.
*/
istream &operator>>(istream &s, tag_store &c)
{
    // Start by deleting any existing tags
    c.clear();

    // Read each line of the stream
    string line;
    while (getline(s, line) && (line != tag_end))
    {
        string::size_type pos = line.find(tag_separator);
        if (pos != string::npos) c.data[line.substr(0, pos)] = line.substr(pos + tag_separator.length());
    }

    // Return the stream
    return s;
}

/*
    Parameters  : s - The stream to output to.
    Returns     : c - The tag class to output.
    Description : Attempt to write the tags to a stream.
*/
ostream &operator<<(ostream &s, const tag_store &c)
{
    // Output all the tags
    for (map_const_iterator<string, string, less<string> > i = c.data.begin();
         i != c.data.end();
         i++)
    {
        s << (*i).first << tag_separator << (*i).second << endl;
    }

    // Output the termination sequence
    s << tag_end << endl;

    // Return the stream
    return s;
}
