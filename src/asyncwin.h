/*
    File        : asyncwin.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Asynchronous operation window handling for the PsiFS
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
#ifndef ASYNCWIN_H
#define ASYNCWIN_H

// Include cathlibcpp header files
#include "deque.h"
#include "string.h"

// Include oslib header files
#include "oslib/toolbox.h"

// Include alexlib header files
#include "button_c.h"

// Include project header files
#include "psifs.h"

// A combination of three icons to provide alternative message displays
class asyncwin_icons
{
    button_c icon_c;                    // The central icon
    button_c icon_l;                    // The left icon
    button_c icon_r;                    // The right icon

    public:

    /*
        Parameters  : centre    - The component ID of the central icon.
                      left      - The component ID of the left icon.
                      right     - The component ID of the right icon.
        Returns     : -
        Description : Constructor.
    */
    asyncwin_icons(toolbox_c centre, toolbox_c left, toolbox_c right)
    : icon_c(centre), icon_l(left), icon_r(right) {}

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~asyncwin_icons() {}

    /*
        Parameters  : obj   - The window's object ID.
        Returns     : void
        Description : Initialise the icons. This clears any current text.
    */
    void set_object(toolbox_o object);

    /*
        Parameters  : value - The text string to set.
        Returns     : void
        Description : Set the value of this icon.
    */
    void set_value(const string &value);

    /*
        Parameters  : value - The text string to set.
                      time  - The time in centi-seconds to display.
        Returns     : void
        Description : Set the value of this icon.
    */
    void set_value(const string &value, bits time);

    /*
        Parameters  : value             - The text string to set.
        Returns     : asyncwin_icons    - This object.
        Description : Set the value of this icon.
    */
    asyncwin_icons &operator=(const char *value);

    /*
        Parameters  : value             - The text string to set.
        Returns     : asyncwin_icons    - This object.
        Description : Set the value of this icon.
    */
    asyncwin_icons &operator=(const string &value);
};

// A class to handle asynchronous operation windows
class asyncwin_win
{
    // A class to handle action buttons
    class action
    {
        toolbox_o obj;                  // The object ID
        toolbox_c cmp;                  // The component ID
        string label;                   // The current label

    public:

        psifs_async_response response;  // The response code

        /*
            Parameters  : -
            Returns     : -
            Description : Constructor.
        */
        action() : obj(toolbox_NULL_OBJECT), cmp(toolbox_NULL_COMPONENT) {}

        /*
            Parameters  : -
            Returns     : -
            Description : Destructor.
        */
        ~action() {}

        /*
            Parameters  : obj   - The window's object ID.
                          cmp   - The component ID.
            Returns     : void
            Description : Initialise the action button. This deletes the
                          existing button and stores the details.
        */
        void init(toolbox_o obj, toolbox_c cmp);

        /*
            Parameters  : response  - The response code to use when the button
                                      is clicked.
            Returns     : void
            Description : Show this action button with a label corresponding to
                          the specified response code.
        */
        void show(psifs_async_response response);

        /*
            Parameters  : void
            Returns     : void
            Description : Hide this action button.
        */
        void hide();
    };

    psifs_async_handle handle;          // The asynchronous operation handle
    bool open;                          // Is the window open
    bool expanded;                      // Is the window expanded
    toolbox_o obj;                      // The window object ID
    action action_ll;                   // Left most action button
    action action_l;                    // Left of centre action button
    action action_r;                    // Right of centre action button
    action action_rr;                   // Right most action button
    action action_c;                    // Central action button
    button_c desc_t;                    // Top description icon
    asyncwin_icons desc_b;              // Bottom description icons
    asyncwin_icons rem_t;               // Top remark icons
    asyncwin_icons rem_b;               // Bottom remark icons
    button_c err;                       // Error description icon

    /*
        Parameters  : void
        Returns     : void
        Description : Update the status of this window.
    */
    void update();

    /*
        Parameters  : expanded  - Should the window be expanded to maximum size.
        Returns     : void
        Description : Expand or contract the window.
    */
    void expand(bool expanded);

    /*
        Parameters  : void
        Returns     : void
        Description : Handle an action button click in this window.
    */
    void action(toolbox_block *id_block, bits flags);

    /*
        Parameters  : buttons   - List of the required action button response
                                  codes.
        Returns     : void
        Description : Set the action buttons to give the specified response
                      codes.
    */
    void set_action(const deque<psifs_async_response> &buttons);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle an action button click.
    */
    static bool action_raw(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle);

public:

    // Close modes
    enum close_mode_type
    {
        no_close,
        allow_close,
        auto_close
    };

    // Abort modes
    enum abort_mode_type
    {
        no_abort,
        allow_abort
    };

    // Pause modes
    enum pause_mode_type
    {
        no_pause,
        allow_pause
    };

    // Delete modes
    enum delete_mode_type
    {
        no_delete,
        auto_delete
    };

    // The current status
    enum status_type
    {
        active,
        success,
        error,
        aborted
    };

    /*
        Parameters  : handle    - Handle for the asynchronous operation.
                      title     - The required window title, or NULL for the
                                  default.
                      top_left  - The coordinates for the top-left corner of
                                  the window, or NULL for the default.
        Returns     : -
        Description : Constructor.
    */
    asyncwin_win(psifs_async_handle handle, const char *title = NULL,
                 const os_coord *top_left = NULL);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~asyncwin_win();

    /*
        Parameters  : void
        Returns     : bool  - Is the window open.
        Description : Return whether the window is still open.
    */
    bool get_open() const;

    /*
        Parameters  : void
        Returns     : bool  - The current status.
        Description : Return the status of the operation.
    */
    status_type get_status() const;

    /*
        Parameters  : mode          - The new close window mode.
        Returns     : asyncwin_win  - This object.
        Description : Set the mode of window closing.
    */
    asyncwin_win &set_close(close_mode_type mode = allow_close);

    /*
        Parameters  : mode          - The new abort operation mode.
        Returns     : asyncwin_win  - This object.
        Description : Set the mode of operation aborting.
    */
    asyncwin_win &set_abort(abort_mode_type mode = allow_abort);

    /*
        Parameters  : mode          - The new pause operation mode.
        Returns     : asyncwin_win  - This object.
        Description : Set the mode of operation pausing.
    */
    asyncwin_win &set_pause(pause_mode_type mode = allow_pause);

    /*
        Parameters  : mode          - The new object deletion mode.
        Returns     : asyncwin_win  - This object.
        Description : Set the mode of deleting this object.
    */
    asyncwin_win &set_delete(delete_mode_type mode = auto_delete);

    /*
        Parameters  : void
        Returns     : void
        Description : Update any open asynchronous operation windows.
    */
    static void update_all();

    /*
        Parameters  : void
        Returns     : void
        Description : Cancel any open asynchronous operation windows.
    */
    static void cancel_all();

    /*
        Parameters  : void
        Returns     : bits      - The number of open asynchronous operation
                                  windows.
        Description : Check how many asynchronous operation windows are open.
    */
    static bits active_count();

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_PsifsAsyncStart wimp message events.
    */
    static int start(wimp_message *message, void *handle);

private:

    close_mode_type close_mode;         // The close window mode
    abort_mode_type abort_mode;         // The abort operation mode
    pause_mode_type pause_mode;         // The pause operation mode
    delete_mode_type delete_mode;       // The object deletion mode
    status_type status;                 // The current status
};

#endif
