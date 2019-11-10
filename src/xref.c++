/*
    File        : xref.c++
    Date:       : 17-Feb-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Cross reference the PsiFS documentation.

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

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/types.h"

// Include cathlibcpp header files
#include "string.h"
#include "fstream.h"
#include "functional.h"
#include "iostream.h"
#include "list.h"
#include "map.h"
#include "sstream.h"
#include "string.h"

// Include cathlibcpp container templates
#include "hlist.c++"
#include "hmap.c++"

// The path to the database file
static char database_path[] = "database";

// Prevent redefinition of less<string>
HoistBinaryPredicateProtocol &get_hoist_less_comparator(string *)
{
    static less<string> comparator;
    static HoistBinaryPredicate<less<string>, string> singleton(comparator);
    return singleton;
}

/*
    Parameters  : s         - The stream to read from.
                  v         - The string to read.
    Returns     : istream   - The stream.
    Description : Read a string from a file, including white space.
*/
istream &mygetline(istream& s, string &v)
{
    // Attempt to read the next line from the stream
    char c;
    v.erase();
    while (s.get(c) && (c != '\n')) v += c;

    // Return the stream
    return s;
}

// Command line parameters
static const char param_prefix = '-';
static const char param_url[] = "url";
static const char param_parent[] = "parent";
static const char param_title[] = "title";
static const char param_keywords[] = "keywords";
static const char param_index[] = "index";
static const char param_contents[] = "contents";
class params : public map<string, string, less<string> >
{
public:

    /*
        Parameters  : argc  - The number of command line arguments.
                      argv  - The value of the command line arguments.
        Returns     : -
        Description : Constructor.
    */
    params(int argc, char *argv[]);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
   ~params() {}
};

/*
    Parameters  : argc  - The number of command line arguments.
                  argv  - The value of the command line arguments.
    Returns     : -
    Description : Constructor.
*/
params::params(int argc, char *argv[])
{
    // Process all the command line parameters
    string c;
    for (int i = 0; i < argc; i++)
    {
        if (*argv[i] == param_prefix)
        {
            c = argv[i] + 1;
            (*this)[c] = "";
        }
        else
        {
            string &m = (*this)[c];
            if (!m.empty()) m += ' ';
            m += argv[i];
        }
    }
}

/*
    Parameters  : s         - The stream to write to.
                  p         - The parameters to write.
    Returns     : ostream   - The stream.
    Description : Write parameters to a stream.
*/
ostream &operator<<(ostream &s, params &p)
{
    // Write the parameters
    for (map_iterator<string, string, less<string> > i = p.begin();
         i != p.end();
         i++)
    {
        s << (*i).first << " = '" << (*i).second << "'" << endl;
    }

    // Return the stream
    return s;
}

// Details for a single page
static const char keyword_separator = ',';
class page
{
public:

    string url;                         // The URL of the page
    string parent;                      // The URL of the parent
    string title;                       // The title of the page
    list<string> keywords;              // List of keywords

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    page() {}

    /*
        Parameters  : url       - The URL of the page.
                      parent    - The URL of the parent.
                      title     - The title of the page.
                      keywords  - The keywords.
        Returns     : -
        Description : Constructor.
    */
    page(string url, string parent, string title, string keywords);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
   ~page() {}
};
inline void destroy(page *p) { p->~page(); }

/*
    Parameters  : lhs   - The first page to compare.
                  rhs   - The second page to compare.
    Returns     : bool  - Is lhs = rhs.
    Description : Compare the title of two pages.
*/
bool operator==(const page &lhs, const page &rhs)
{
    return lhs.title == rhs.title;
}

/*
    Parameters  : lhs   - The first page to compare.
                  rhs   - The second page to compare.
    Returns     : bool  - Is lhs < rhs.
    Description : Compare the title of two pages.
*/
bool operator<(const page &lhs, const page &rhs)
{
    return lhs.title < rhs.title;
}

/*
    Parameters  : s         - The stream to read from.
                  p         - The page to read.
    Returns     : istream   - The stream.
    Description : Read a page from a stream.
*/
istream &operator>>(istream &s, page &p)
{
    // Read the page header
    mygetline(s, p.url);
    mygetline(s, p.parent);
    mygetline(s, p.title);

    // Read the keywords
    string k;
    p.keywords.clear();
    while (mygetline(s, k) && !k.empty()) p.keywords.push_back(k);

    // Return the stream
    return s;
}

/*
    Parameters  : s         - The stream to write to.
                  p         - The page to write.
    Returns     : osstream  - The stream.
    Description : Write a page to a stream.
*/
ostream &operator<<(ostream &s, page &p)
{
    // Write the page header
    s << p.url << endl
      << p.parent << endl
      << p.title << endl;

    // Write the keywords
    list_iterator<string> e = p.keywords.end();
    for (list_iterator<string> i = p.keywords.begin();
         i != e;
         i++)
    {
        s << *i << endl;
    }
    s << endl;

    // Return the stream
    return s;
}

/*
    Parameters  : url       - The URL of the page.
                  parent    - The URL of the parent.
                  title     - The title of the page.
                  keywords  - The keywords.
    Returns     : -
    Description : Constructor.
*/
page::page(string url, string parent, string title, string keywords)
: url(url), parent(parent), title(title)
{
    // Remove any leading colons from the URLs
    if (url[0] == ':') page::url.erase(0, 1);
    if (parent[0] == ':') page::parent.erase(0, 1);

    // Process the keywords
    while (!keywords.empty())
    {
        int pos = keywords.find(keyword_separator);
        if (pos) page::keywords.push_back(keywords.substr(0, pos));
        keywords.erase(0, pos == string::npos ? pos : pos + 1);
    }
    page::keywords.unique();
}

// A database indexed by URL
class database : public map<string, page, less<string> >
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    database() {}

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
   ~database() {}
};

/*
    Parameters  : s         - The stream to read from.
                  d         - The database to read.
    Returns     : istream   - The stream.
    Description : Read a database from a stream.
*/
istream &operator>>(istream &s, database &d)
{
    // Read the database
    page p;
    d.clear();
    while (s >> p) d[p.url] = p;

    // Return the stream
    return s;
}

/*
    Parameters  : s         - The stream to write to.
                  d         - The database to write.
    Returns     : ostream   - The stream.
    Description : Write a database to a stream.
*/
ostream &operator<<(ostream &s, database &d)
{
    // Write the database
    for (map_iterator<string, page, less<string> > i = d.begin();
         i != d.end();
         i++)
    {
        s << (*i).second;
    }

    // Return the stream
    return s;
}

// Contents
class contents : public list<page>
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    contents() {}

    /*
        Parameters  : d - Page database.
        Returns     : -
        Description : Constructor.
    */
    contents(const database &d);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~contents() {}
};
inline void destroy(contents *p) { p->~contents(); }

// Dummy remove implementation to prevent compilation errors
void list<page>::remove(const page &value)
{
    (void) value;
}

/*
    Parameters  : d - Page database.
    Returns     : -
    Description : Constructor.
*/
contents::contents(const database &d)
{
    // Copy all of the pages
    for (map_const_iterator<string, page, less<string> > i = d.begin();
         i != d.end();
         i++)
    {
        (*this).push_back((*i).second);
    }

    // Sort by page title
    sort();
}

/*
    Parameters  : s         - The stream to write to.
                  d         - The contents to write.
    Returns     : ostream   - The stream.
    Description : Write a contents to a stream.
*/
ostream &operator<<(ostream &s, const contents &d)
{
    // Write the pages
    for (list_const_iterator<page> i = d.begin(); i != d.end(); i++)
    {
        s << "    <XREFI HREF=\":" << (*i).url << "\" TITLE=\"" << (*i).title << "\">" << endl;
    }

    // Return the stream
    return s;
}

/*
    Parameters  : lhs   - The first index entry to compare.
                  rhs   - The second index entry to compare.
    Returns     : bool  - Is lhs = rhs.
    Description : Compare the lists of pages.
*/
bool operator==(contents &lhs, contents &rhs)
{
    return list<page>(lhs) == list<page>(rhs);
}

/*
    Parameters  : lhs   - The first page to compare.
                  rhs   - The second page to compare.
    Returns     : bool  - Is lhs < rhs.
    Description : Compare the lists of pages.
*/
bool operator<(contents &lhs, contents &rhs)
{
    return list<page>(lhs) < list<page>(rhs);
}

// Index
class index : public map<string, contents, less<string> >
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    index() {}

    /*
        Parameters  : d - Page database.
        Returns     : -
        Description : Constructor.
    */
    index(const database &d);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~index() {}
};

/*
    Parameters  : d - Page database.
    Returns     : -
    Description : Constructor.
*/
index::index(const database &d)
{
    // Process all of the pages
    for (map_const_iterator<string, page, less<string> > i = d.begin();
         i != d.end();
         i++)
    {
        // Process all of the keywords
        list_const_iterator<string> e = (*i).second.keywords.end();
        for (list_const_iterator<string> j = (*i).second.keywords.begin();
             j != e;
             j++)
        {
            (*this)[*j].push_back((*i).second);
        }
    }

    // Sort the pages for each keyword
    for (map_iterator<string, contents, less<string> > k = begin();
         k != end();
         k++)
    {
        (*k).second.sort();
    }
}

/*
    Parameters  : s         - The stream to write to.
                  d         - The index to write.
    Returns     : ostream   - The stream.
    Description : Write an index to a stream.
*/
ostream &operator<<(ostream &s, const index &d)
{
    // Write the keywords
    for (map_const_iterator<string, contents, less<string> > i = d.begin();
         i != d.end();
         i++)
    {
        s << "    <XREFK KEYWORD=\"" << (*i).first << "\">" << endl
          << (*i).second
          << "    </XREFK>" << endl;
    }

    // Return the stream
    return s;
}

/*
    Parameters  : argc  - The number of command line arguments.
                  argv  - The value of the command line arguments.
    Returns     : int   - Return code.
    Description : The main function.
*/
int main(int argc, char *argv[])
{
    // The first parameter is always the program name
    string n(argv[0]);

    // Process the command line parameters
    params p(argc - 1, argv + 1);
    cout << "<*" << endl << n << " parameters:" << endl << p << "*>" << endl;
    if ((p.find("") == p.end())
        && !p[param_url].empty() && !p[param_title].empty())
    {
        database d;

        // Read the database
        {
            ifstream f(database_path);
            f >> d;
        }

        // Add this page to the database
        d[p[param_url]] = page(p[param_url], p[param_parent], p[param_title], p[param_keywords]);

        // Write the database
        {
            ofstream f(database_path);
            f << d;
        }

        // Build an index if required
        if (p.find(param_index) != p.end())
        {
            cout << "<$macro XREF_INDEX>" << endl
                 << "    <XREF>" << endl
                 << index(d)
                 << "    </XREF>" << endl
                 << "</$macro>" << endl;
        }

        // Build a contents list if required
        if (p.find(param_contents) != p.end())
        {
            cout << "<$macro XREF_CONTENTS>" << endl
                 << "    <XREF>" << endl
                 << contents(d)
                 << "    </XREF>" << endl
                 << "</$macro>" << endl;
        }
    }
    else
    {
        // Unsuitable command line parameters
        cout << "<$message TEXT=\"Syntax: " << n
             << " -url <url>"
             << " [-parent <parent>]"
             << " -title <title>"
             << " [-keywords <keywords>]"
             << " [-contents]"
             << " [-index]\""
             << " CLASS=\"fatal\">" << endl;

        // Return an error code
        return EXIT_FAILURE;
    }

    // Successful if this point reached
    return EXIT_SUCCESS;
}
