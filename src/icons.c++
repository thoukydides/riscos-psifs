/*
    File        : icons.c++
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

// Include header file for this module
#include "icons.h"

// Include oslib header files
#include "oslib/iconbar.h"
#include "oslib/menu.h"

// Include project header files
#include "filer.h"
#include "fs.h"
#include "info.h"
#include "open.h"

// Menu component IDs
#define ICONS_DRIVE_NAME ((toolbox_c) 0x01)
#define ICONS_DRIVE_BACKUP ((toolbox_c) 0x02)

/*
    Parameters  : void
    Returns     : -
    Description : Destructor.
*/
icons_icon::ref_class::~ref_class()
{
    // Delete the icon if the reference count has reached zero
    if (!count) toolbox_delete_object(0, id);
}

/*
    Parameters  : void
    Returns     : void
    Description : Increment the reference count.
*/
void icons_icon::ref_class::inc()
{
    // Just increment the reference count
    count++;
}

/*
    Parameters  : void
    Returns     : void
    Description : Decrement the reference count.
*/
void icons_icon::ref_class::dec()
{
    // Decrease the reference count and delete if appropriate
    if (!--count) delete this;
}

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
icons_icon::icons_icon(const char *icon, const char *text, void *handle,
                       const icons_icon *neighbour, bool right)
{
    // Obtain a pointer to the icon template
    toolbox_resource_file_object *obj = (toolbox_resource_file_object *)
                                        toolbox_template_look_up(0, icon);
    iconbar_object *obj_icon = (iconbar_object *) &(obj->object);

    // Store the original position and priority
    int position = obj_icon->position;
    int priority = obj_icon->priority;

    // Change the icon position and priority if required
    if (neighbour && neighbour->ref)
    {
        obj_icon->position = (int) (right
                                    ? wimp_ICON_BAR_RIGHT_RELATIVE
                                    : wimp_ICON_BAR_LEFT_RELATIVE);
        obj_icon->priority = (int) iconbar_get_icon_handle(0, neighbour->ref->id);
    }

    // Change the default text if required
    if (text) strcpy(obj_icon->text, text);

    // Create the icon
    ref = new ref_class;
    ref->id = toolbox_create_object(toolbox_CREATE_GIVEN_OBJECT,
                                    (toolbox_id) obj);

    // Set the client handle
    toolbox_set_client_handle(0, ref->id, handle);

    // Copy the initial text
    ref->text = obj_icon->text;

    // Restore the original position and priority
    obj_icon->position = position;
    obj_icon->priority = priority;
}

/*
    Parameters  : icon  - The icon to copy.
    Returns     : -
    Description : Constructor.
*/
icons_icon::icons_icon(const icons_icon &icon)
{
    // Increase the reference count of the icon being copied
    if (icon.ref) icon.ref->inc();

    // Copy the pointer to the reference counted details
    ref = icon.ref;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
icons_icon::~icons_icon()
{
    // Decrement the reference count
    if (ref) ref->dec();
}

/*
    Parameters  : -
    Returns     : toolbox_o - The menu object.
    Description : Return the menu assiciated with this icon.
*/
toolbox_o icons_icon::menu() const
{
    // Return the menu object
    return ref ? iconbar_get_menu(0, ref->id) : toolbox_NULL_OBJECT;
}

/*
    Parameters  : icon          - The icon to copy.
    Returns     : icons_icon    - A reference to this icon.
    Description : Icon assignment.
*/
icons_icon &icons_icon::operator=(const icons_icon &icon)
{
    // Increase the reference count of the icon being copied
    if (icon.ref) icon.ref->inc();

    // Reduce the reference count of this icon
    if (ref) ref->dec();

    // Copy the pointer to the reference counted details
    ref = icon.ref;

    // Return a pointer to this icon
    return *this;
}

/*
    Parameters  : text          - The text to display under the icon.
    Returns     : icons_icon    - A reference to this icon.
    Description : Change the text under the icon.
*/
icons_icon &icons_icon::operator=(const char *text)
{
    // Change the text if appropriate
    if (ref && (ref->text != text))
    {
        iconbar_set_text(0, ref->id, text);
        ref->text = text;
    }

    // Return a pointer to this icon
    return *this;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the text under the icon.
*/
void icons_link::update()
{
    psifs_link_status status = psifsget_link_status();

    // Set the required text
    this->icons_icon::operator=(filer_msgtrans(status & psifs_LINK_STATUS_EPOC32
                                               ? "IcnE32"
                                               : status & psifs_LINK_STATUS_EPOC16
                                                 ? "IcnE16"
                                                 : "IcnLink").c_str());
}

/*
    Parameters  : drive     - The drive letter for this icon.
                  prev      - The preceding drive icon if any.
    Returns     : -
    Description : Constructor.
*/
icons_drive::icons_drive(psifs_drive drive, const icons_drive *prev)
    : drive(drive),
      icons_icon(psifsget_link_status() & psifs_LINK_STATUS_EPOC32
                 ? "IconE32"
                 : "IconE16",
                 NULL, this, prev, FALSE)
{
    // Update the drive name immediately
    update();

    // Disable unsuitable menu entries
    if (drive == FS_CHAR_DRIVE_ALL)
    {
        // The virtual drive is a special case
        menu_set_fade(0, menu(), ICONS_DRIVE_NAME, TRUE);
    }
    else if (psifsget_drive_status(drive) & psifs_DRIVE_STATUS_ROM)
    {
        // Disable the backup option if a ROM drive
        menu_set_fade(0, menu(), ICONS_DRIVE_BACKUP, TRUE);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the text under the icon.
*/
void icons_drive::update()
{
    fs_discname disc;

    // Attempt to read the drive name
    if (drive == FS_CHAR_DRIVE_ALL)
    {
        // The virtual drive is a special case
        filer_msgtrans(disc, sizeof(disc),
                       psifsget_link_status() & psifs_LINK_STATUS_EPOC32
                       ? "IcnE32"
                       : "IcnE16");
        name = FS_NAME_DRIVE_ALL;
    }
    else if ((psifsget_drive_name(drive, disc, sizeof(disc)) < 1)
             || (strlen(disc) < 2))
    {
        // Construct a default name if necessary
        disc[0] = FS_CHAR_DISC;
        disc[1] = drive;
        disc[2] = '\0';

        // Store the disc name
        name = drive;
    }
    else
    {
        // Store the disc name
        name = disc;
    }

    // Change the text under the icon
    this->icons_icon::operator=(disc);
}

/*
    Parameters  : void
    Returns     : void
    Description : Open a filer window for this drive.
*/
void icons_drive::open() const
{
    // Open a filer window for this drive
    open_open(name);
}

/*
    Parameters  : void
    Returns     : void
    Description : Close any filer windows for this drive.
*/
void icons_drive::close() const
{
    // Close any filer windows for this drive
    open_close(name);
}

/*
    Parameters  : id_block      - A toolbox ID block.
    Returns     : icons_drive   - Pointer to the corresponding drive icon
                                  object.
    Description : Convert a toolbox ID block into a drive icon pointer.
*/
icons_drive *icons_drive::find(const toolbox_block *id_block)
{
    toolbox_o id;

    // Choose the ancestor object ID
    id = id_block->ancestor_obj == toolbox_NULL_OBJECT
         ? id_block->this_obj
         : id_block->ancestor_obj;

    // Return a pointer to the drive icon object
    return (icons_drive *) toolbox_get_client_handle(0, id);
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the text under the icon.
*/
void icons_printer::update()
{
    bits bytes;

    // Attempt to read the number of bytes received
    if (!xpsifsget_statistics_received_bytes(&bytes) && bytes)
    {
        char buffer[20];

        // Convert the number of bytes to a string
        os_convert_file_size(bytes, buffer, sizeof(buffer));

        // Change the text under the icon
        this->icons_icon::operator=(buffer);
    }
}
