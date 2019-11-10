/*
    File        : asyncwin.c++
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

// Include header file for this module
#include "asyncwin.h"

// Include oslib header files
#include "oslib/toolbox.h"
#include "oslib/window.h"
#include "oslib/wimp.h"
#include "event.h"

// Include cathlibcpp header files
#include "iomanip.h"
#include "list.h"
#include "sstream.h"

// Include alexlib header files
#include "actionbutton_c.h"
#include "button_c.h"

// Include project header files
#include "filer.h"
#include "fs.h"

// Asynchronous window gadgets
#define ASYNCWIN_LL ((toolbox_c) 0x00)
#define ASYNCWIN_L ((toolbox_c) 0x01)
#define ASYNCWIN_R ((toolbox_c) 0x02)
#define ASYNCWIN_RR ((toolbox_c) 0x03)
#define ASYNCWIN_C ((toolbox_c) 0x04)
#define ASYNCWIN_DESC_T ((toolbox_c) 0x10)
#define ASYNCWIN_DESC_B ((toolbox_c) 0x11)
#define ASYNCWIN_DESC_BL ((toolbox_c) 0x12)
#define ASYNCWIN_DESC_BR ((toolbox_c) 0x13)
#define ASYNCWIN_REM_T ((toolbox_c) 0x20)
#define ASYNCWIN_REM_TL ((toolbox_c) 0x21)
#define ASYNCWIN_REM_TR ((toolbox_c) 0x22)
#define ASYNCWIN_REM_B ((toolbox_c) 0x23)
#define ASYNCWIN_REM_BL ((toolbox_c) 0x24)
#define ASYNCWIN_REM_BR ((toolbox_c) 0x25)
#define ASYNCWIN_ERR ((toolbox_c) 0x30)

// Application specific response codes
#define ASYNCWIN_RESPONSE_NONE ((psifs_async_response) 0x10000)
#define ASYNCWIN_RESPONSE_ABORT ((psifs_async_response) 0x10001)
#define ASYNCWIN_RESPONSE_PAUSE ((psifs_async_response) 0x10002)
#define ASYNCWIN_RESPONSE_RESUME ((psifs_async_response) 0x10003)
#define ASYNCWIN_RESPONSE_CLOSE ((psifs_async_response) 0x10004)

// Control of position at which to automatically open
#define ASYNCWIN_OFFSET_X (64)
#define ASYNCWIN_OFFSET_Y (48)
#define ASYNCWIN_MIN_X (96)
#define ASYNCWIN_MAX_X (ASYNCWIN_MIN_X + ASYNCWIN_OFFSET_X * 8)
#define ASYNCWIN_MAX_X_OFFSET (716)
#define ASYNCWIN_MIN_Y (348)
#define ASYNCWIN_MAX_Y (ASYNCWIN_MIN_Y + ASYNCWIN_OFFSET_Y * 4)
#define ASYNCWIN_MAX_Y_OFFSET (96)

// A list of asynchronous windows
static list<asyncwin_win *> asyncwin_list;

/*
    Parameters  : void
    Returns     : int   - The screen width in OS units.
    Description : Returns the width of the current screen mode.
*/
static int asyncwin_scr_width()
{
    int xeig;
    int xwind;

    // Read the mode details
    os_read_mode_variable(os_CURRENT_MODE,
                          os_MODEVAR_XEIG_FACTOR, &xeig);
    os_read_mode_variable(os_CURRENT_MODE,
                          os_MODEVAR_XWIND_LIMIT, &xwind);

    // Return the result
    return (xwind + 1) << xeig;
}

/*
    Parameters  : void
    Returns     : int   - The screen height in OS units.
    Description : Returns the height of the current screen mode.
*/
static int asyncwin_scr_height()
{
    int yeig;
    int ywind;

    // Read the mode details
    os_read_mode_variable(os_CURRENT_MODE,
                          os_MODEVAR_YEIG_FACTOR, &yeig);
    os_read_mode_variable(os_CURRENT_MODE,
                          os_MODEVAR_YWIND_LIMIT, &ywind);

    // Return the result
    return (ywind + 1) << yeig;
}

/*
    Parameters  : value - The text to check.
    Returns     : bool  - Is it a filename.
    Description : Check whether the specified string is a filename.
*/
static bool asyncwin_filename(const string &value)
{
    bool valid = TRUE;

    // Check that it isn't an empty string
    if (value.empty()) valid = FALSE;

    // Check for any invalid characters
    if (value.find(char(' ')) != string::npos) valid = FALSE;

    // Return the result
    return valid;
}

/*
    Parameters  : obj   - The window's object ID.
    Returns     : void
    Description : Initialise the icons. This clears any current text.
*/
void asyncwin_icons::set_object(toolbox_o obj)
{
    // Store the details
    icon_c.set_object(obj);
    icon_l.set_object(obj);
    icon_r.set_object(obj);

    // Clear all of the icons
    set_value("");
}

/*
    Parameters  : value - The text string to set.
    Returns     : void
    Description : Set the value of this icon.
*/
void asyncwin_icons::set_value(const string &value)
{
    // Check for a space
    string::size_type space = value.find(char(' '));

    // Find the last directory separator
    string::size_type leaf = value.rfind(char(FS_CHAR_SEPARATOR));

    // Format as a filename if appropriate
    if (asyncwin_filename(value.substr(space == string::npos ? 0 : space + 1))
        && (leaf != string::npos))
    {
        // Format as a filename
        icon_c = "";
        icon_l = value.substr(0, leaf + 1);
        icon_r = value.substr(leaf + 1);
    }
    else
    {
        // Format as a simple message
        icon_c = value;
        icon_l = "";
        icon_r = "";
    }
}

/*
    Parameters  : value - The text string to set.
                  time  - The time in centi-seconds to display.
    Returns     : void
    Description : Set the value of this icon.
*/
void asyncwin_icons::set_value(const string &value, bits time)
{
    // Format the time
    ostringstream format;
    bits seconds = time / 100;
    bits minutes = seconds / 60;
    bits hours = minutes / 60;
    seconds %= 60;
    minutes %= 60;
    format << setw(2) << setfill('0') << hours << ":"
           << setw(2) << setfill('0') << minutes << ":"
           << setw(2) << setfill('0') << seconds;

    // Set the value
    icon_c = "";
    icon_l = value;
    icon_r = time ? format.str() : filer_msgtrans("OpTUnk");
}

/*
    Parameters  : value             - The text string to set.
    Returns     : asyncwin_icons    - This object.
    Description : Set the value of this icon.
*/
asyncwin_icons &asyncwin_icons::operator=(const char *value)
{
    // Set the value
    set_value(string(value));
    return *this;
}

/*
    Parameters  : value             - The text string to set.
    Returns     : asyncwin_icons    - This object.
    Description : Set the value of this icon.
*/
asyncwin_icons &asyncwin_icons::operator=(const string &value)
{
    // Set the value
    set_value(value);
    return *this;
}

/*
    Parameters  : response  - The response code.
                  help      - Should the help text be returned.
    Returns     : string    - The corresponding string.
    Description : Convert a response code into a suitable button label or help
                  text.
*/
static string asyncwin_label(psifs_async_response response, bool help = FALSE)
{
    string tag;

    // The text depends on the response code
    switch (response)
    {
        case psifs_ASYNC_RESPONSE_CONTINUE:     tag = "OpActCnt";   break;
        case psifs_ASYNC_RESPONSE_YES:          tag = "OpActYes";   break;
        case psifs_ASYNC_RESPONSE_NO:           tag = "OpActNo";    break;
        case psifs_ASYNC_RESPONSE_QUIET:        tag = "OpActQet";   break;
        case psifs_ASYNC_RESPONSE_SKIP:         tag = "OpActSkp";   break;
        case psifs_ASYNC_RESPONSE_RESTART:      tag = "OpActRst";   break;
        case psifs_ASYNC_RESPONSE_RETRY:        tag = "OpActRty";   break;
        case psifs_ASYNC_RESPONSE_COPY:         tag = "OpActCpy";   break;
        case ASYNCWIN_RESPONSE_ABORT:           tag = "OpActAbt";   break;
        case ASYNCWIN_RESPONSE_PAUSE:           tag = "OpActPse";   break;
        case ASYNCWIN_RESPONSE_RESUME:          tag = "OpActCnt";   break;
        case ASYNCWIN_RESPONSE_CLOSE:           tag = "OpActOK";    break;
        default:                                tag = "OpActUnk";   break;
    }

    // Modify the tag if the help text is required
    if (help) tag += 'H';

    // Lookup the tag and return the result
    return filer_msgtrans(tag.c_str());
}

/*
    Parameters  : buttons   - List of the required action button response codes.
    Returns     : void
    Description : Set the action buttons to give the specified response codes.
*/
void asyncwin_win::set_action(const deque<psifs_async_response> &buttons)
{
    // The allocation of buttons depends on the total number
    switch (buttons.size())
    {
        case 0:
            // No buttons
            action_ll.hide();
            action_l.hide();
            action_r.hide();
            action_rr.hide();
            action_c.hide();
            break;

        case 1:
            // Single button
            action_ll.hide();
            action_l.hide();
            action_r.hide();
            action_rr.hide();
            action_c.show(buttons[0]);
            break;

        case 2:
            // Two buttons
            action_ll.show(buttons[0]);
            action_l.hide();
            action_r.hide();
            action_rr.show(buttons[1]);
            action_c.hide();
            break;

        case 3:
            // Three buttons
            action_ll.show(buttons[0]);
            action_l.hide();
            action_r.hide();
            action_rr.show(buttons[2]);
            action_c.show(buttons[1]);
            break;

        default:
            // Maximum of four buttons supported
            action_ll.show(buttons[0]);
            action_l.show(buttons[1]);
            action_r.show(buttons[2]);
            action_rr.show(buttons[3]);
            action_c.hide();
            break;
    }
}

/*
    Parameters  : obj   - The window's object ID.
                  cmp   - The component ID.
    Returns     : -
    Description : Initialise the action button. This deletes the
                  existing button and stores the details.
*/
void asyncwin_win::action::init(toolbox_o obj, toolbox_c cmp)
{
    // Store the details
    action::obj = obj;
    action::cmp = cmp;

    // Delete the existing gadget
    window_remove_gadget(0, obj, cmp);
}

/*
    Parameters  : response  - The response code to use when the button is
                              clicked.
    Returns     : void
    Description : Show this action button with a label corresponding to the
                  specified response code.
*/
void asyncwin_win::action::show(psifs_async_response response)
{
    string label = asyncwin_label(response);

    // Create the gadget if not already shown
    if (action::label.empty())
    {
        // Lookup the window details
        toolbox_resource_file_object *window;
        window = (toolbox_resource_file_object *)
                 toolbox_template_look_up(0, "WinAsync");

        // Extract the details of this gadget
        gadget_object *gadget = window_extract_gadget_info(0, window, cmp,
                                                           NULL);

        // Add the gadget to the window
        window_add_gadget(0, obj, gadget);
    }

    // Set and store the required text if changed
    if (action::label != label)
    {
        // Set the button label
        actionbutton_c(cmp, obj).set_text(label);
        action::label = label;

        // Set the button help text
        actionbutton_c(cmp, obj).set_help_message(asyncwin_label(response, TRUE));
    }

    // Store the response code
    action::response = response;
}

/*
    Parameters  : void
    Returns     : void
    Description : Hide this action button.
*/
void asyncwin_win::action::hide()
{
    // Delete the gadget if not already hidden
    if (!label.empty())
    {
        // Delete the gadget
        window_remove_gadget(0, obj, cmp);

        // Clear the stored label
        label = "";
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the status of this window.
*/
void asyncwin_win::update()
{
    deque<psifs_async_response> buttons;

    // Only update the status if still active
    if (get_status() == active)
    {
        psifs_async_status op;
        char *desc;
        char *detail;
        char *msg;
        bits taken;
        bits remain;

        // Poll the status
        op = psifs_async_poll(handle, &desc, &detail, &msg, &taken, &remain);

        // Update the status and add any extra buttons
        switch (op)
        {
            case psifs_ASYNC_SUCCESS:
                // Completed successfully
                status = success;
                break;

            case psifs_ASYNC_ERROR:
                // Completed with error
                status = error;
                break;

            case psifs_ASYNC_ABORTED:
                // Operation aborted (not expected)
                status = aborted;
                break;

            case psifs_ASYNC_WAIT_COPY:
                // Add yes and no button
                buttons.push_back(psifs_ASYNC_RESPONSE_YES);
                buttons.push_back(psifs_ASYNC_RESPONSE_NO);
                break;

            case psifs_ASYNC_WAIT_RESTART:
                // Add skip and retry buttons
                buttons.push_back(psifs_ASYNC_RESPONSE_SKIP);
                buttons.push_back(psifs_ASYNC_RESPONSE_RETRY);
                break;

            case psifs_ASYNC_WAIT_NEWER:
                // Add skip, copy and quiet buttons
                buttons.push_back(psifs_ASYNC_RESPONSE_SKIP);
                buttons.push_back(psifs_ASYNC_RESPONSE_COPY);
                buttons.push_back(psifs_ASYNC_RESPONSE_QUIET);
                break;

            case psifs_ASYNC_WAIT_READ:
                // Add skip and retry buttons
                buttons.push_back(psifs_ASYNC_RESPONSE_SKIP);
                buttons.push_back(psifs_ASYNC_RESPONSE_RETRY);
                break;

            case psifs_ASYNC_PAUSED:
                // Operation paused
                if (pause_mode == allow_pause)
                {
                    buttons.push_back(ASYNCWIN_RESPONSE_RESUME);
                }
                break;

            default:
                // Other codes are just general activity
                if (pause_mode == allow_pause)
                {
                    buttons.push_back(ASYNCWIN_RESPONSE_PAUSE);
                }
                break;
        }

        // Update the status fields
        desc_t = desc ? desc : "";
        desc_b = detail ? detail : "";
        if (msg) err = msg;
        expand(msg ? TRUE : FALSE);
        if (get_status() == active)
        {
            rem_t.set_value(filer_msgtrans("OpTTkn"), taken);
            rem_b.set_value(filer_msgtrans("OpTRmn"), remain);
        }
        else
        {
            rem_t.set_value(filer_msgtrans("OpTTok"), taken);
            rem_b.set_value("");
        }
    }
    if (get_status() == aborted)
    {
        // Ensure a consistent appearance for an aborted operation
        desc_t = filer_msgtrans("OpAbt");
        desc_b = "";
        rem_t = filer_msgtrans("OpInact");
        rem_b = "";
        expand(FALSE);
    }

    // Update the buttons
    if (get_status() == active)
    {
        // Start with an abort button
        if (abort_mode == allow_abort)
        {
            buttons.push_front(ASYNCWIN_RESPONSE_ABORT);
        }
    }
    else if (close_mode != no_close)
    {
        // Add a close button
        buttons.push_back(ASYNCWIN_RESPONSE_CLOSE);
    }
    set_action(buttons);

    // Close the window if appropriate
    if (((get_status() == success) || (get_status() == aborted))
        && get_open() && (close_mode == auto_close))
    {
        toolbox_hide_object(0, obj);
        open = FALSE;
    }

    // Delete this window if appropriate
    if (!get_open() && (delete_mode == auto_delete)) delete this;
}

/*
    Parameters  : expanded  - Should the window be expanded to maximum size.
    Returns     : void
    Description : Expand or contract the window.
*/
void asyncwin_win::expand(bool expanded)
{
    // No action if already the right size
    if (asyncwin_win::expanded != expanded)
    {
        wimp_window_info info;
        toolbox_position position;

        // Read the current window details
        info.w = window_get_wimp_handle(0, obj);
        wimp_get_window_info_header_only(&info);

        // Required settings depend on the required state
        if (expanded)
        {
            // Set the basic details
            position.full.visible = info.extent;
            position.full.visible.x1 = info.extent.x1 - info.extent.x0;
            position.full.visible.y1 = info.extent.y1 - info.extent.y0;
            position.full.visible.x0 = (asyncwin_scr_width()
                                        - position.full.visible.x1) / 2;
            position.full.visible.y0 = (asyncwin_scr_height()
                                        - position.full.visible.y1) / 2;
            position.full.visible.x1 += position.full.visible.x0;
            position.full.visible.y1 += position.full.visible.y0;
            position.full.xscroll = info.extent.x0;
            position.full.yscroll = info.extent.y1;
            position.full.next = wimp_TOP;
        }
        else
        {
            // Resize the window keeping its current position and stacking
            position.full.xscroll = -info.extent.x0;
            position.full.yscroll = -info.extent.y1;
            position.full.xscroll = 0;
            position.full.yscroll = 0;
            position.full.visible.x0 = info.visible.x0
                                       + position.full.xscroll - info.xscroll;
            position.full.visible.y1 = info.visible.y1
                                       + position.full.yscroll - info.yscroll;
            position.full.visible.x1 = position.full.visible.x0 + info.xmin;
            position.full.visible.y0 = position.full.visible.y1 - info.ymin;
            position.full.next = info.next;
        }

        // Show the window at the new position
        toolbox_show_object(0, obj, toolbox_POSITION_FULL, &position,
                            toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

        // Store the new status
        asyncwin_win::expanded = expanded;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Handle an action button click in this window.
*/
void asyncwin_win::action(toolbox_block *id_block, bits flags)
{
    NOT_USED(flags)

    psifs_async_response response;

    // Find the associated response code
    switch (id_block->this_cmp)
    {
        case ASYNCWIN_LL:   response = action_ll.response;      break;
        case ASYNCWIN_L:    response = action_l.response;       break;
        case ASYNCWIN_R:    response = action_r.response;       break;
        case ASYNCWIN_RR:   response = action_rr.response;      break;
        case ASYNCWIN_C:    response = action_c.response;       break;
        default:            response = ASYNCWIN_RESPONSE_NONE;  break;
    }

    // Take the appropriate action
    switch (response)
    {
        case ASYNCWIN_RESPONSE_NONE:
            // No action required
            break;

        case ASYNCWIN_RESPONSE_ABORT:
            // Abort the current operation
            if ((get_status() != aborted) && (abort_mode == allow_abort))
            {
                psifs_async_end(handle);
                status = aborted;
            }
            break;

        case ASYNCWIN_RESPONSE_PAUSE:
            // Pause the operation
            if (pause_mode == allow_pause) psifsasynccontrol_pause(handle);
            break;

        case ASYNCWIN_RESPONSE_RESUME:
            // Resume the operation
            if (pause_mode == allow_pause) psifsasynccontrol_resume(handle);
            break;

        case ASYNCWIN_RESPONSE_CLOSE:
            // Close the window
            if (get_open() && (close_mode != no_close))
            {
                toolbox_hide_object(0, obj);
                open = FALSE;
            }
            break;

        default:
            // All other codes are passed to the asynchronous operation
            if (get_status() == active)
            {
                psifsasynccontrol_simple_response(handle, response);
            }
            break;
    }

    // Finally update the window
    update();
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle an action button click.
*/
bool asyncwin_win::action_raw(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    asyncwin_win *ptr = (asyncwin_win *) handle;

    NOT_USED(event_code);

    // Forward to the class specific handler
    ptr->action(id_block, action->flags);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : void
    Returns     : os_coord  - Pointer to the coordinates for the top-left
                              corner of the next window.
    Description : Generate a position for the next window to be opened. The
                  result will be overwritten on the next call to this function,
                  so must be copied if required.
*/
static const os_coord *asyncwin_next_pos()
{
    static os_coord pos = {ASYNCWIN_MAX_X, ASYNCWIN_MIN_Y};
    int max_x;
    int max_y;

    // Update the position
    pos.x += ASYNCWIN_OFFSET_X;
    pos.y -= ASYNCWIN_OFFSET_Y;

    // Calculate the bounds
    max_x = asyncwin_scr_width() - ASYNCWIN_MAX_X_OFFSET;
    if (ASYNCWIN_MAX_X < max_x) max_x = ASYNCWIN_MAX_X;
    max_y = asyncwin_scr_height() - ASYNCWIN_MAX_Y_OFFSET;
    if (ASYNCWIN_MAX_Y < max_y) max_y = ASYNCWIN_MAX_Y;

    // Massage the position if unsuitable
    if (max_x < pos.x) pos.x = ASYNCWIN_MIN_X;
    if (pos.y < ASYNCWIN_MIN_Y) pos.y = max_y;

    // Return the result
    return &pos;
}

/*
    Parameters  : handle    - Handle for the asynchronous operation.
                  title     - The required window title, or NULL for the
                              default.
                  top_left  - The coordinates for the top-left corner of
                              the window, or NULL for the default.
    Returns     : -
    Description : Constructor.
*/
asyncwin_win::asyncwin_win(psifs_async_handle handle, const char *title,
                           const os_coord *top_left)
: handle(handle), status(active), open(TRUE), expanded(FALSE),
  desc_t(ASYNCWIN_DESC_T),
  desc_b(ASYNCWIN_DESC_B, ASYNCWIN_DESC_BL, ASYNCWIN_DESC_BR),
  rem_t(ASYNCWIN_REM_T, ASYNCWIN_REM_TL, ASYNCWIN_REM_TR),
  rem_b(ASYNCWIN_REM_B, ASYNCWIN_REM_BL, ASYNCWIN_REM_BR),
  err(ASYNCWIN_ERR),
  close_mode(allow_close), abort_mode(allow_abort),
  pause_mode(allow_pause), delete_mode(auto_delete)
{
    // Create the window
    obj = toolbox_create_object(0, (toolbox_id) "WinAsync");

    // Register toolbox handler
    event_register_toolbox_handler(obj,
                                   action_ACTION_BUTTON_SELECTED,
                                   action_raw, this);

    // Set the window title if specified
    if (title) window_set_title(0, obj, title);

    // Initialise all of the action buttons
    action_ll.init(obj, ASYNCWIN_LL);
    action_l.init(obj, ASYNCWIN_L);
    action_r.init(obj, ASYNCWIN_R);
    action_rr.init(obj, ASYNCWIN_RR);
    action_c.init(obj, ASYNCWIN_C);

    // Initialise the other icons
    desc_t.set_object(obj);
    desc_b.set_object(obj);
    rem_t.set_object(obj);
    rem_b.set_object(obj);
    err.set_object(obj);

    // Show the window
    if (!top_left) top_left = asyncwin_next_pos();
    toolbox_show_object(0, obj,
                        toolbox_POSITION_TOP_LEFT,
                        (toolbox_position *) top_left,
                        toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

    // Update this window
    update();

    // Add to the list of windows
    asyncwin_list.push_back((asyncwin_win *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
asyncwin_win::~asyncwin_win()
{
    // Remove from the list of windows
    asyncwin_list.remove((asyncwin_win *)(this));

    // Deregister toolbox handler
    event_deregister_toolbox_handler(obj,
                                     action_ACTION_BUTTON_SELECTED,
                                     action_raw, this);

    // Delete the window
    toolbox_delete_object(0, obj);

    // End the operation if still active
    if (get_status() != aborted)
    {
        psifs_async_end(handle);
        status = aborted;
    }
}

/*
    Parameters  : void
    Returns     : bool  - Is the window open.
    Description : Return whether the window is still open.
*/
bool asyncwin_win::get_open() const
{
    // Return the status
    return open;
}

/*
    Parameters  : void
    Returns     : bool  - The current status.
    Description : Return the status of the operation.
*/
asyncwin_win::status_type asyncwin_win::get_status() const
{
    // Return the status
    return status;
}

/*
    Parameters  : mode          - The new close window mode.
    Returns     : asyncwin_win  - This object.
    Description : Set the mode of window closing.
*/
asyncwin_win &asyncwin_win::set_close(close_mode_type mode)
{
    // Store the new setting
    close_mode = mode;

    // Return the object
    return *this;
}

/*
    Parameters  : mode          - The new abort operation mode.
    Returns     : asyncwin_win  - This object.
    Description : Set the mode of operation aborting.
*/
asyncwin_win &asyncwin_win::set_abort(abort_mode_type mode)
{
    // Store the new setting
    abort_mode = mode;

    // Return the object
    return *this;
}

/*
    Parameters  : mode          - The new pause operation mode.
    Returns     : asyncwin_win  - This object.
    Description : Set the mode of operation pausing.
*/
asyncwin_win &asyncwin_win::set_pause(pause_mode_type mode)
{
    // Store the new setting
    pause_mode = mode;

    // Return the object
    return *this;
}

/*
    Parameters  : mode          - The new object deletion mode.
    Returns     : asyncwin_win  - This object.
    Description : Set the mode of deleting this object.
*/
asyncwin_win &asyncwin_win::set_delete(delete_mode_type mode)
{
    // Store the new setting
    delete_mode = mode;

    // Return the object
    return *this;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update any open asynchronous operation windows.
*/
void asyncwin_win::update_all()
{
    list_iterator<asyncwin_win *> i;

    // Update all of the windows
    for (i = asyncwin_list.begin(); i != asyncwin_list.end(); i++)
    {
        (*i)->update();
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Cancel any open asynchronous operation windows.
*/
void asyncwin_win::cancel_all()
{
    // Delete all windows
    while (!asyncwin_list.empty()) delete asyncwin_list.front();
}

/*
    Parameters  : void
    Returns     : bits      - The number of open asynchronous operation
                              windows.
    Description : Check how many asynchronous operation windows are open.
*/
bits asyncwin_win::active_count()
{
    return asyncwin_list.size();
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_PsifsAsyncStart wimp message events.
*/
int asyncwin_win::start(wimp_message *message, void *handle)
{
    psifs_message_async_start *data = (psifs_message_async_start *) &message->data;

    NOT_USED(handle);

    // Create a new window to handle the asynchronous operation
    asyncwin_win *ptr = new asyncwin_win(data->handle, data->title,
                                         (data->top_left.x != -1)
                                         || (data->top_left.y != -1)
                                         ? &data->top_left : NULL);

    // Set the options for the window
    ptr->set_close(data->close ? allow_close : auto_close);
    ptr->set_abort(data->abort ? allow_abort : no_abort);
    ptr->set_pause(data->pause ? allow_pause : no_pause);

    // Acknowledge the message
    message->your_ref = message->my_ref;
    wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, message, message->sender);

    // Always claim the event
    return TRUE;
}
