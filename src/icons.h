/*
    File        : icons.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Iconbar icon handling for the PsiFS filer.

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
#ifndef ICON_H
#define ICON_H

// Include oslib header files
#include "oslib/toolbox.h"

// Include cathlibcpp header files
#include "string.h"

// Include project header files
#include "psifs.h"

// A class to handle iconbar icons
class icons_icon
{
    // Reference counted details
    struct ref_class
    {
        toolbox_o id;                   // The toolbox object ID
        string text;                    // The currently displayed text
        int count;                      // Number of references

        /*
            Parameters  : void
            Returns     : -
            Description : Constructor.
        */
        ref_class() : count(1) {}

        /*
            Parameters  : void
            Returns     : -
            Description : Destructor.
        */
        ~ref_class();

        /*
            Parameters  : void
            Returns     : void
            Description : Increment the reference count.
        */
        void inc();

        /*
            Parameters  : void
            Returns     : void
            Description : Decrement the reference count.
        */
        void dec();
    };
    ref_class *ref;

public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    icons_icon() : ref(NULL) {}

    /*
        Parameters  : icon      - The template name of the iconbar object.
                      text      - The initial text to display under the icon,
                                  or NULL to use the text from the resource
                                  file.
                      handle    - Optional client handle for this icon.
                      neighbour - Optional icon to create this one next to.
                      right     - Should this icon be created to the left of
                                  the existing icon.
        Returns     : -
        Description : Constructor.
    */
    icons_icon(const char *icon, const char *text = NULL, void *handle = NULL,
               const icons_icon *neighbour = NULL, bool right = FALSE);

    /*
        Parameters  : icon  - The icon to copy.
        Returns     : -
        Description : Constructor.
    */
    icons_icon(const icons_icon &icon);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~icons_icon();

    /*
        Parameters  : -
        Returns     : toolbox_o - The menu object.
        Description : Return the menu assiciated with this icon.
    */
    toolbox_o menu() const;

    /*
        Parameters  : icon          - The icon to copy.
        Returns     : icons_icon    - A reference to this icon.
        Description : Icon assignment.
    */
    icons_icon &operator=(const icons_icon &icon);

    /*
        Parameters  : text          - The text to display under the icon.
        Returns     : icons_icon    - A reference to this icon.
        Description : Change the text under the icon.
    */
    icons_icon &operator=(const char *text);
};

// An idle icon
class icons_idle : private icons_icon
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    icons_idle() : icons_icon("IconInact") {}

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~icons_idle() {}
};

// A remote link icon
class icons_link : private icons_icon
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    icons_link() : icons_icon("IconAct") {}

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~icons_link() {}

    /*
        Parameters  : void
        Returns     : void
        Description : Update the text under the icon.
    */
    void update();
};

// A drive icon
class icons_drive : private icons_icon
{
public:

    psifs_drive drive;                  // The drive letter
    string name;                        // The drive name

    /*
        Parameters  : drive     - The drive letter for this icon.
                      prev      - The preceding drive icon if any.
        Returns     : -
        Description : Constructor.
    */
    icons_drive(psifs_drive drive, const icons_drive *prev = NULL);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~icons_drive() {}

    /*
        Parameters  : void
        Returns     : void
        Description : Update the text under the icon.
    */
    void update();

    /*
        Parameters  : void
        Returns     : void
        Description : Open a filer window for this drive.
    */
    void open() const;

    /*
        Parameters  : void
        Returns     : void
        Description : Close any filer windows for this drive.
    */
    void close() const;

    /*
        Parameters  : id_block      - A toolbox ID block.
        Returns     : icons_drive   - Pointer to the corresponding drive icon
                                      object.
        Description : Convert a toolbox ID block into a drive icon pointer.
    */
    static icons_drive *find(const toolbox_block *id_block);
};

// A printer mirror icon
class icons_printer : private icons_icon
{
public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor.
    */
    icons_printer() : icons_icon("IconPrint") {}

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~icons_printer() {}

    /*
        Parameters  : void
        Returns     : void
        Description : Update the text under the icon.
    */
    void update();
};

#endif
