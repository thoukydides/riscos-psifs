/*
    File        : convwin.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : File converter window handling for the PsiFS filer.

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
#include "convwin.h"

// Include oslib header files
#include "oslib/osfscontrol.h"
#include "oslib/taskmanager.h"
#include "oslib/toolbox.h"
#include "event.h"

// Include cathlibcpp header files
#include "fstream.h"
#include "sstream.h"

// Include alexlib header files
#include "actionbutton_c.h"
#include "button_c.h"
#include "displayfield_c.h"
#include "draggable_c.h"
#include "numberrange_c.h"
#include "optionbutton_c.h"
#include "radiobutton_c.h"
#include "stringset_c.h"
#include "writablefield_c.h"

// Include project header files
#include "config.h"
#include "filer.h"
#include "fs.h"
#include "options.h"
#include "psifs.h"
#include "scrap.h"

// Converter window gadgets
#define CONVWIN_CONVERT ((toolbox_c) 0x00)
#define CONVWIN_CANCEL ((toolbox_c) 0x01)
#define CONVWIN_OPTIONS ((toolbox_c) 0x02)
#define CONVWIN_INFO ((toolbox_c) 0x03)
#define CONVWIN_WRITABLE ((toolbox_c) 0x10)
#define CONVWIN_DRAGGABLE ((toolbox_c) 0x11)
#define CONVWIN_CONVERTER ((toolbox_c) 0x12)
#define CONVWIN_BUTTON ((toolbox_c) 0x13)
#define CONVWIN_SENDER ((toolbox_c) 0x14)
#define CONVWIN_RECEIVER ((toolbox_c) 0x15)
#define CONVWIN_DISPLAY ((toolbox_c) 0x16)

// Options tags
static const char convwin_tag_convert[] = "Convert";
static const char convwin_tag_standard[] = "Standard";
static const char convwin_tag_load[] = "InterceptLoad";
static const char convwin_tag_run[] = "InterceptRun";
static const char convwin_tag_save[] = "InterceptSave";
static const char convwin_tag_options[] = "ConvertOptions";
static const char convwin_tag_format[] = "Version";

// Types of control
#define CONVWIN_NUMBERRANGE 'N'
#define CONVWIN_OPTIONBUTTON 'O'
#define CONVWIN_RADIOBUTTON 'R'
#define CONVWIN_STRINGSET 'S'
#define CONVWIN_WRITABLEFIELD 'W'

// The most recent Data_Save file name
static string convwin_last_save;

// Temporary file to specify in Data_Save message
#define CONVWIN_TEMP_FILE "<Wimp$Scrap>"

// Lists of conversion windows
static list<convwin_base *> convwin_base_list;
static list<convwin_win *> convwin_win_list;

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle action button selection events in the window.
*/
bool convwin_opt::action_this(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    convwin_opt *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Read the window state if Set clicked
        if (action->flags & actionbutton_SELECTED_DEFAULT) ptr->read();

        // Write the window state if Cancel clicked
        if (action->flags & actionbutton_SELECTED_CANCEL) ptr->write();

        // Close the window if adjust not used
        if (!(action->flags & actionbutton_SELECTED_ADJUST))
        {
            toolbox_hide_object(0, ptr->obj);
        }
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the window opening.
*/
bool convwin_opt::open_this(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    convwin_opt *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found or already open
    if (ptr && !ptr->open)
    {
        // Mark the window as opened
        ptr->open = TRUE;

        // Reset the window contents
        ptr->write();
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the window closing.
*/
bool convwin_opt::close_this(bits event_code, toolbox_action *action,
                             toolbox_block *id_block, void *handle)
{
    convwin_opt *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Just mark the window as closed
    if (ptr) ptr->open = FALSE;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : id_block      - A toolbox ID block.
    Returns     : convwin_opt   - Pointer to the corresponding options
                                  window, or NULL if not found.
    Description : Convert a toolbox ID block into an options window
                  pointer.
*/
convwin_opt *convwin_opt::find(const toolbox_block *id_block)
{
    // Attempt to read the client handle
    convwin_opt *ptr;
    if (xtoolbox_get_client_handle(0, id_block->this_obj, (void **) &ptr)) ptr = NULL;

    // Return the pointer
    return ptr;
}

/*
    Parameters  : void
    Returns     : void
    Description : Write the options to the window.
*/
void convwin_opt::write()
{
    // Convert both the format string and options to tagged format
    tag_store options_tag;
    options_tag = options;
    tag_store format_tag;
    format_tag = format.substr(format.find(' '));

    // Process all of the format specifications
    for (map_iterator<string, string, less<string> > i = format_tag.data.begin(); i != format_tag.data.end(); i++)
    {
        string keyword((*i).first);
        istringstream arg((*i).second);
        char type;
        toolbox_c cmp;
        if (arg >> type >> hex >> cmp)
        {
            // Action depends on the type of control
            switch (toupper(type))
            {
                case CONVWIN_NUMBERRANGE:
                    if (options_tag.exist(keyword))
                    {
                        numberrange_c(cmp, obj) = options_tag.get_num(keyword);
                    }
                    break;

                case CONVWIN_OPTIONBUTTON:
                    optionbutton_c(cmp, obj) = options_tag.exist(keyword);
                    break;

                case CONVWIN_RADIOBUTTON:
                    radiobutton_c(cmp, obj) = options_tag.exist(keyword);
                    break;

                case CONVWIN_STRINGSET:
                    if (options_tag.exist(keyword))
                    {
                        stringset_c(cmp, obj) = options_tag.get_str(keyword);
                    }
                    break;

                case CONVWIN_WRITABLEFIELD:
                    if (options_tag.exist(keyword))
                    {
                        writablefield_c(cmp, obj) = options_tag.get_str(keyword);
                    }
                    break;

                default:
                    // No other control types are supported
                    break;
            }
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Read the options from the window.
*/
void convwin_opt::read()
{
    // Convert the format string to tagged format
    tag_store format_tag;
    format_tag = format.substr(format.find(' '));

    // Process all of the format specifications
    tag_store options_tag;
    for (map_iterator<string, string, less<string> > i = format_tag.data.begin(); i != format_tag.data.end(); i++)
    {
        string keyword((*i).first);
        istringstream arg((*i).second);
        char type;
        toolbox_c cmp;
        if (arg >> type >> hex >> cmp)
        {
            // Action depends on the type of control
            switch (toupper(type))
            {
                case CONVWIN_NUMBERRANGE:
                    options_tag.set_num(keyword, numberrange_c(cmp, obj)());
                    break;

                case CONVWIN_OPTIONBUTTON:
                    if (optionbutton_c(cmp, obj)())
                    {
                        options_tag.set_str(keyword, "");
                    }
                    break;

                case CONVWIN_RADIOBUTTON:
                    if (radiobutton_c(cmp, obj)())
                    {
                        options_tag.set_str(keyword, "");
                    }
                    break;

                case CONVWIN_STRINGSET:
                    options_tag.set_str(keyword, stringset_c(cmp, obj).gadget_w_string::get_value());
                    break;

                case CONVWIN_WRITABLEFIELD:
                    options_tag.set_str(keyword, writablefield_c(cmp, obj)());
                    break;

                default:
                    // No other control types are supported
                    break;
            }
        }
    }

    // Set the options string
    options = options_tag();
}

/*
    Parameters  : conv  - The converter to create an options window for.
    Returns     : -
    Description : Constructor.
*/
convwin_opt::convwin_opt(convobj_obj *conv)
: obj(toolbox_NULL_OBJECT), open(FALSE), options("")
{
    // Get the file format converter's tag
    tag = conv->get_str(convobj_tag);

    // Get the options template and format
    format = conv->get_str(convobj_options);
    res = format.substr(0, format.find(' '));

    // Attempt to create the window
    if (!res.empty())
    {
        // Create the window
        obj = toolbox_create_object(0, (toolbox_id) res.c_str());

        // Set the client handle
        toolbox_set_client_handle(0, obj, this);

        // Register handlers for the window
        event_register_toolbox_handler(obj, action_ACTION_BUTTON_SELECTED,
                                       action_this, NULL);
        event_register_toolbox_handler(obj, action_WINDOW_ABOUT_TO_BE_SHOWN,
                                       open_this, NULL);
        event_register_toolbox_handler(obj, action_WINDOW_DIALOGUE_COMPLETED,
                                       close_this, NULL);

        // Retrieve any previous options if valid
        if (options_current.get_str(tag_store::tag(convwin_tag_options, tag, convwin_tag_format)) == format)
        {
            options = options_current.get_str(tag_store::tag(convwin_tag_options, tag));
        }

        // Clean up any existing options by writing and reading the window
        write();
        read();
    }
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
convwin_opt::~convwin_opt()
{
    // No action unless object created
    if (obj != toolbox_NULL_OBJECT)
    {
        // Deregister handlers for the window
        event_deregister_toolbox_handler(obj, action_ACTION_BUTTON_SELECTED,
                                         action_this, NULL);
        event_deregister_toolbox_handler(obj, action_WINDOW_ABOUT_TO_BE_SHOWN,
                                         open_this, NULL);
        event_deregister_toolbox_handler(obj, action_WINDOW_DIALOGUE_COMPLETED,
                                         close_this, NULL);

        // Destroy the window
        toolbox_delete_object(0, obj);
    }
}

/*
    Parameters  : void
    Returns     : string    - The options string.
    Description : Save the current options and return the options string.
*/
string convwin_opt::save()
{
    // Save the options
    options_current.set_str(tag_store::tag(convwin_tag_options, tag, convwin_tag_format), format);
    options_current.set_str(tag_store::tag(convwin_tag_options, tag), options);

    // Return the options string
    return options;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Update the window.
*/
bool convwin_base::update_this(bits event_code, toolbox_action *action,
                               toolbox_block *id_block, void *handle)
{
    convwin_base *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Update the window
        ptr->update();
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : name              - The name of the file to check.
    Returns     : epoc32_file_uid   - The UID of the file.
    Description : Read and check the UID of a file.
*/
epoc32_file_uid convwin_base::get_uid(string name)
{
    epoc32_file_uid uid;

    // Attempt to read the UID from the file
    ifstream file(name.c_str());
    bits uid4;
    if (!file.read((char *) &uid, sizeof(uid))
        || !file.read((char *) &uid4, sizeof(uid4))
        || (uid4 != psifs_check_uid(uid.uid1, uid.uid2, uid.uid3)))
    {
        // Use a default UID otherwise
        uid.uid1 = uid.uid2 = uid.uid3 = 0;
    }

    // Return the result
    return uid;
}

/*
    Parameters  : type      - The type of the file.
                  dir       - Should converters that produce directories be
                              included.
                  special   - Should special converters that do not produce
                              output files be included.
                  temp      - The file name of the temporary copy.
                  res       - The name of the resource object to use.
                  orig      - The suggested leaf name.
    Returns     : -
    Description : Constructor.
*/
convwin_base::convwin_base(bits type, bool dir, bool special, string temp,
                           string res, string orig)
: convobj_list(convobj_all, type, get_uid(temp), orig, dir, special),
  type(type), type_tag(convobj_types::to_tag(type)),
  temp(temp), orig(orig), opt(NULL)
{
    // Create the window
    obj = toolbox_create_object(0, (toolbox_id) res.c_str());

    // Set the client handle
    toolbox_set_client_handle(0, obj, this);

    // Register handlers to update the window
    event_register_toolbox_handler(obj, action_STRING_SET_VALUE_CHANGED,
                                   update_this, NULL);

    // Set the menu of converters
    update_menu(obj, CONVWIN_CONVERTER);

    // Read the default action button details
    actionbutton_c act(CONVWIN_CONVERT, obj);
    act_text = act.get_text();
    act_help = act.get_help_message();

    // Open the window at the current pointer position
    toolbox_show_object(0, obj, toolbox_POSITION_AT_POINTER, NULL,
                        toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

    // Add to the list of conversion windows
    convwin_base_list.push_back((convwin_base *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
convwin_base::~convwin_base()
{
    // Remove from the list of conversion windows
    convwin_base_list.remove((convwin_base *)(this));

    // Ensure that the options window is deleted
    if (opt) delete opt;

    // Deregister handlers to update the window
    event_deregister_toolbox_handler(obj, action_STRING_SET_VALUE_CHANGED,
                                     update_this, NULL);

    // Destroy the window
    toolbox_delete_object(0, obj);
}

/*
    Parameters  : conv  - Should the conversion type be set.
    Returns     : void
    Description : Reset the contents of the conversion window.
*/
void convwin_base::reset(bool conv)
{
    // Reset the conversion type if required
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag));
    if (conv) conv = !select(convwin_tag_convert);

    // Perform an immediate update of the window
    update();
}

/*
    Parameters  : tag   - The name of the options tag to use.
    Returns     : bool  - Was a converter selected.
    Description : Attempt to select the converter associated with the
                  specified options tag.
*/
bool convwin_base::select(string tag)
{
    bool selected = FALSE;

    // Check if the specified options tag exists
    if (options_current.exist(tag))
    {
        // Attempt to select the appropriate converter
        selected = set_menu(obj, CONVWIN_CONVERTER,
                            options_current.get_str(tag));
    }

    // Return whether a converter was selected
    return selected;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the status of the conversion window.
*/
void convwin_base::update(void)
{
    // Find the currently selected conversion
    convobj_obj *conv = get_menu(obj, CONVWIN_CONVERTER);

    // Set the file type sprite
    string output = convobj_types::to_sprite(get_type());
    set_sprite(output.empty() ? convobj_types::to_sprite(type) : output,
               output.empty());

    // Set the info window and action button
    if (conv) conv->update_info(actionbutton_c(CONVWIN_INFO, obj).get_show());
    actionbutton_c(CONVWIN_INFO, obj).set_faded(!conv);

    // Set the default action button
    actionbutton_c act(CONVWIN_CONVERT, obj);
    string conv_act_text;
    string conv_act_help;
    if (conv)
    {
        conv_act_text = conv->get_str(convobj_button_text);
        conv_act_help = conv->get_str(convobj_button_help);
    }
    act.set_text(conv_act_text.empty() ? act_text : conv_act_text);
    act.set_help_message(conv_act_help.empty() ? act_help : conv_act_help);

    // Set the options window
    if (opt)
    {
        delete opt;
        opt = NULL;
    }
    if (conv) opt = new convwin_opt(conv);
    actionbutton_c(CONVWIN_OPTIONS, obj).set_faded(!opt || (opt->obj == toolbox_NULL_OBJECT));
    if (opt) actionbutton_set_click_show(0, obj, CONVWIN_OPTIONS, opt->obj, 0x20>>3);
}

/*
    Parameters  : result    - The name of the converted file, or an empty
                              string if no conversion was performed.
                  open      - Should the window be left open if possible.
    Returns     : bool      - Was the conversion successful.
    Description : Perform a file conversion.
*/
bool convwin_base::convert(bool open)
{
    bool success = TRUE;
    string result;

    // Find the currently selected conversion
    convobj_obj *conv = get_menu(obj, CONVWIN_CONVERTER);

    // Warn that the conversion is going to be performed
    pre_convert(conv ? conv->get_str(convobj_tag) : string());

    // Perform the conversion
    if (conv)
    {
        // Generate a temporary name for the result of the conversion
        result = scrap_name();

        // Enable simple error handling
        filer_error_allowed++;

        // Perform the conversion
        success = conv->run(temp, result, orig, opt ? opt->save() : string());

        // Restore normal error handling
        filer_error_allowed--;
    }

    // Handle the result of the conversion
    if (success)
    {
        string output = convobj_types::to_sprite(get_type());
        if (output.empty() ? !open : !post_convert(result, open))
        {
            delete this;
        }
    }

    // Delete any result of the conversion
    if (conv) xosfscontrol_wipe(result.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);

    // Return the result
    return success;
}

/*
    Parameters  : conv  - The tag associated with this converter.
    Returns     : void
    Description : A conversion is about to be performed. This may be used
                  to store the current conversion options.
*/
void convwin_base::pre_convert(string conv)
{
    // Store the conversion type
    options_current.set_str(convwin_tag_convert, conv);
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag), conv);
}

/*
    Parameters  : result    - The name of the converted file, or an empty
                              string if no conversion was performed.
                  open      - Should the window be left open if possible.
    Returns     : bool      - Should the window be left open.
    Description : A conversion has been performed, and the result is ready
                  for use.
*/
bool convwin_base::post_convert(string result, bool open)
{
    NOT_USED(result);

    // Return whether the window should be left open
    return open;
}

/*
    Parameters  : open      - Should the window be left open if possible.
    Returns     : bool      - Should the window be left open.
    Description : Cancel the file conversion.
*/
bool convwin_base::pre_cancel(bool open)
{
    // Return whether the window should be left open
    return open;
}

/*
    Parameters  : void
    Returns     : bits  - The output file type.
    Description : Return the type of file produced as output by the
                  converter.
*/
bits convwin_base::get_type() const
{
    // Find the currently selected conversion
    convobj_obj *conv = get_menu(obj, CONVWIN_CONVERTER);

    // Return the file type
    return conv ? conv->get_type() : type;
}

/*
    Parameters  : id_block      - A toolbox ID block.
    Returns     : convwin_base  - Pointer to the corresponding conversion
                                  window, or NULL if not found.
    Description : Convert a toolbox ID block into a conversion window
                  pointer.
*/
convwin_base *convwin_base::find(const toolbox_block *id_block)
{
    // Choose the ancestor object ID
    toolbox_o id = id_block->ancestor_obj == toolbox_NULL_OBJECT
                   ? id_block->this_obj
                   : id_block->ancestor_obj;

    // Attempt to read the client handle
    convwin_base *ptr;
    if (xtoolbox_get_client_handle(0, id, (void **) &ptr)) ptr = NULL;

    // Return the pointer
    return ptr;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Perform the conversion.
*/
bool convwin_base::convert_this(bits event_code, toolbox_action *action,
                                toolbox_block *id_block, void *handle)
{
    convwin_base *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Perform the conversion
        ptr->convert(action->flags & actionbutton_SELECTED_ADJUST);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Cancel the conversion.
*/
bool convwin_base::cancel_this(bits event_code, toolbox_action *action,
                               toolbox_block *id_block, void *handle)
{
    convwin_base *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Either close the window or reset the contents
        if (ptr->pre_cancel(action->flags & actionbutton_SELECTED_ADJUST))
        {
            // Reset the contents of the window
            ptr->reset();
        }
        else
        {
            // Close the window
            delete ptr;
        }
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Cancel any file converters.
*/
void convwin_base::cancel_all()
{
    // Delete all windows
    while (!convwin_base_list.empty()) delete convwin_base_list.front();
}

/*
    Parameters  : void
    Returns     : bits      - The number of open file converter windows.
    Description : Check how many file converters are open.
*/
bits convwin_base::active_count()
{
    return convwin_base_list.size();
}

/*
    Parameters  : type      - The type of the file.
                  orig      - The suggested leaf name.
                  temp      - The file name of the temporary copy.
                  receiver  - The receiving task's handle.
    Returns     : -
    Description : Constructor.
*/
convwin_win::convwin_win(bits type, string orig, string temp, wimp_t sender)
: convwin_base(type, TRUE, TRUE, temp, string("WinConv"), orig), ref(0)
{
    char *name;

    // Register handlers to update the window
    event_register_toolbox_handler(obj, action_DRAGGABLE_DRAG_ENDED,
                                   drag_this, NULL);

    // Get the source task name
    tx = xtaskmanager_task_name_from_handle(sender, &name)
         ? filer_msgtrans("CnvUnTk")
         : string(name);

    // Reset the window contents
    reset();

    // Add to the list of conversion windows
    convwin_win_list.push_back((convwin_win *)(this));
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Perform a conversion when a drag ends.
*/
bool convwin_win::drag_this(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    convwin_win *ptr = (convwin_win *) convwin_base::find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        draggable_action_drag_ended *drag = (draggable_action_drag_ended *) &action->data;

        // Get the destination name
        string dest(writablefield_c(CONVWIN_WRITABLE, ptr->obj)());
        string::size_type pos = dest.find_last_of(FS_CHAR_SEPARATOR);
        if (pos != string::npos) dest.erase(0, pos + 1);

        // Start a save of the converted file
        wimp_message message;
        message.your_ref = 0;
        message.action = message_DATA_SAVE;
        message.data.data_xfer.w = drag->ids.wimp.w;
        message.data.data_xfer.i = drag->ids.wimp.i;
        message.data.data_xfer.pos = drag->pos;
        message.data.data_xfer.est_size = 0;
        message.data.data_xfer.file_type = ptr->get_type();
        strcpy(message.data.data_xfer.file_name, dest.c_str());
        message.size = ALIGN(strchr(message.data.data_xfer.file_name, '\0')
                             - (char *) &message + 1);
        wimp_send_message_to_window(wimp_USER_MESSAGE, &message,
                                    drag->ids.wimp.w, drag->ids.wimp.i);
        ptr->ref = message.my_ref;
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : ref           - The message reference.
    Returns     : convwin_win   - Pointer to the corresponding conversion
                                  window, or NULL if not found.
    Description : Convert a wimp message reference into a conversion window
                  pointer.
*/
convwin_win *convwin_win::find(int ref)
{
    list_iterator<convwin_win *> i = convwin_win_list.begin();

    // Attempt to find a match
    while ((i != convwin_win_list.end()) && ((*i)->ref != ref)) i++;

    // Return the result
    return i == convwin_win_list.end() ? NULL : *i;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
convwin_win::~convwin_win()
{
    // Remove from the list of conversion windows
    convwin_win_list.remove((convwin_win *)(this));

    // Deregister handlers to update the window
    event_deregister_toolbox_handler(obj, action_DRAGGABLE_DRAG_ENDED,
                                     drag_this, NULL);

    // Ensure that the temporary file is deleted
    xosfscontrol_wipe(temp.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
}

/*
    Parameters  : conv  - Should the conversion type be set.
    Returns     : void
    Description : Reset the contents of the conversion window.
*/
void convwin_win::reset(bool conv)
{
    // Restore the original filename
    writablefield_c(CONVWIN_WRITABLE, obj) = orig;

    // Reset the conversion type if required
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_standard));

    // Pass on to lower objects
    convwin_base::reset(conv);
}

/*
    Parameters  : conv  - The tag associated with this converter.
    Returns     : void
    Description : A conversion is about to be performed. This may be used
                  to store the current conversion options.
*/
void convwin_win::pre_convert(string conv)
{
    // Get the destination name
    string dest(writablefield_c(CONVWIN_WRITABLE, obj)());

    // Enable simple error handling
    filer_error_allowed++;

    // Generate an error if only a leaf name has been specified
    if (dest.find(FS_CHAR_SEPARATOR) == string::npos)
    {
        os_error err;

        // Generate the error
        err.errnum = 0;
        filer_msgtrans(err.errmess, sizeof(err.errmess), "ErrLeaf");
        os_generate_error(&err);
    }

    // Restore normal error handling
    filer_error_allowed--;

    // Store the conversion type
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_standard), conv);

    // Pass on to lower objects
    convwin_base::pre_convert(conv);
}

/*
    Parameters  : result    - The name of the converted file, or an empty
                              string if no conversion was performed.
                  open      - Should the window be left open if possible.
    Returns     : bool      - Should the window be left open.
    Description : A conversion has been performed, and the result is ready
                  for use.
*/
bool convwin_win::post_convert(string result, bool open)
{
    // Get the source name
    if (result.empty()) result = temp;

    // Get the destination name
    string dest(writablefield_c(CONVWIN_WRITABLE, obj)());

    // Enable simple error handling
    filer_error_allowed++;

    // Attempt to save the result of the conversion
    xosfscontrol_wipe(dest.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
    osfscontrol_copy(result.c_str(), dest.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);

    // Restore normal error handling
    filer_error_allowed--;

    // Return whether the window should be left open
    return open;
}

/*
    Parameters  : sprite    - The sprite name to set.
                  fade      - Should the sprite be faded.
    Returns     : void
    Description : Set the sprite to display.
*/
void convwin_win::set_sprite(string sprite, bool fade)
{
    // Set the sprite
    draggable_c gadget(CONVWIN_DRAGGABLE, obj);
    gadget.set_sprite(sprite);
    gadget.set_faded(fade);

    // Fade the writable file name field also
    writablefield_c(CONVWIN_WRITABLE, obj).set_faded(fade);
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSave wimp message events.
*/
int convwin_win::data_save(wimp_message *message, void *handle)
{
    bool claimed = FALSE;

    NOT_USED(handle);

    // Only process message if sent to iconbar icon
    if (message->data.data_xfer.w == wimp_ICON_BAR)
    {
        // Store the original file name
        convwin_last_save = message->data.data_xfer.file_name;

        // Respond with a temporary file name
        message->action = message_DATA_SAVE_ACK;
        message->your_ref = message->my_ref;
        message->data.data_xfer.est_size = -1;
        strcpy(message->data.data_xfer.file_name, CONVWIN_TEMP_FILE);
        message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                              - (char *) message + 1);
        wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSaveAck wimp message events.
*/
int convwin_win::data_save_ack(wimp_message *message, void *handle)
{
    bool claimed = FALSE;
    convwin_win *ptr = find(message->your_ref);

    NOT_USED(handle);

    // Only process message if suitable
    if (ptr)
    {
        // Set the path name field
        writablefield_c(CONVWIN_WRITABLE, ptr->obj) = message->data.data_xfer.file_name;

        // Perform the conversion
        if (ptr->convert(TRUE))
        {
            // Respond with a notification that the file has been saved
            message->action = message_DATA_LOAD;
            message->your_ref = message->my_ref;
            message->data.data_xfer.est_size = 0;
            wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
            ptr->ref = message->my_ref;
            claimed = TRUE;
        }
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoad wimp message events.
*/
int convwin_win::data_load(wimp_message *message, void *handle)
{
    bool claimed = FALSE;

    NOT_USED(handle);

    // Only process message if sent to iconbar icon
    if (message->data.data_xfer.w == wimp_ICON_BAR)
    {
        string temp(scrap_name());

        // Enable simple error handling
        filer_error_allowed++;

        // Handle the source file
        if (message->your_ref)
        {
            // Move to a new temporary file
            osfscontrol_copy(message->data.data_xfer.file_name, temp.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE | osfscontrol_COPY_DELETE, 0, 0, 0, 0, NULL);
        }
        else
        {
            // An original file is being loaded
            convwin_last_save = message->data.data_xfer.file_name;
            osfscontrol_copy(message->data.data_xfer.file_name, temp.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);
        }

        // Restore normal error handling
        filer_error_allowed--;

        // Create a new conversion window
        new convwin_win(message->data.data_xfer.file_type, convwin_last_save, temp, message->sender);

        // Acknowledge the load
        message->action = message_DATA_LOAD_ACK;
        message->your_ref = message->my_ref;
        wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoadAck wimp message events.
*/
int convwin_win::data_load_ack(wimp_message *message, void *handle)
{
    bool claimed = FALSE;
    convwin_win *ptr = find(message->your_ref);

    NOT_USED(handle);

    // Only process message if suitable
    if (ptr)
    {
        // Save successful so close the window
        delete ptr;
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : handle    - Handle of the file transfer.
                  type      - The type of the file.
                  dir       - Should converters that produce directories be
                              included.
                  special   - Should special converters that do not produce
                              output files be included.
                  orig      - The suggested leaf name.
                  temp      - The file name of the temporary copy.
                  sender    - The sending task's handle.
                  receiver  - The receiving task's handle.
                  res       - The name of the resource object to use.
    Returns     : -
    Description : Constructor.
*/
convwin_inter::convwin_inter(psifs_intercept_handle handle, bits type, bool dir,
                             bool special, string orig, string temp,
                             wimp_t sender, wimp_t receiver, string res)
: convwin_base(type, dir || config_current.get_bool(config_tag_intercept_directory), special, temp, res, orig), handle(handle), active(TRUE)
{
    char *name;

    // Get the task names
    tx = xtaskmanager_task_name_from_handle(sender, &name)
         ? filer_msgtrans("CnvUnTk")
         : string(name);
    rx = xtaskmanager_task_name_from_handle(receiver, &name)
         ? filer_msgtrans("CnvUnTk")
         : string(name);

    // Set the task name and original filename fields
    displayfield_c(CONVWIN_DISPLAY, obj) = orig;
    displayfield_c(CONVWIN_SENDER, obj) = tx;
    displayfield_c(CONVWIN_RECEIVER, obj) = rx;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
convwin_inter::~convwin_inter()
{
    // Cancel the intercept if still active
    if (active) xpsifsinterceptcontrol_cancel(handle);
}

/*
    Parameters  : result    - The name of the converted file, or an empty
                              string if no conversion was performed.
                  open      - Should the window be left open if possible.
    Returns     : bool      - Should the window be left open.
    Description : A conversion has been performed, and the result is ready
                  for use.
*/
bool convwin_inter::post_convert(string result, bool open)
{
    NOT_USED(open);

    // Enable simple error handling
    filer_error_allowed++;

    // Resume or replace the intercept
    if (result.empty()) psifsinterceptcontrol_restart(handle);
    else
    {
        // Overwrite the temporary file
        xosfscontrol_wipe(temp.c_str(), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
        osfscontrol_copy(result.c_str(), temp.c_str(), osfscontrol_COPY_RECURSE | osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);

        // Replace the intercept
        psifsinterceptcontrol_replace(handle);
    }

    // Restore normal error handling
    filer_error_allowed--;

    // Indicate that the intercept is no longer active
    active = FALSE;

    // Window must be closed
    return FALSE;
}

/*
    Parameters  : open      - Should the window be left open if possible.
    Returns     : bool      - Should the window be left open.
    Description : Cancel the file conversion.
*/
bool convwin_inter::pre_cancel(bool open)
{
    // No action unless the window is closing
    if (!open)
    {
        // Enable simple error handling
        filer_error_allowed++;

        // Cancel the intercept if the window is not being left open
        psifsinterceptcontrol_cancel(handle);

        // Restore normal error handling
        filer_error_allowed--;

        // Indicate that the intercept is no longer active
        active = FALSE;
    }

    // Return whether the window should be left open
    return open;
}

/*
    Parameters  : sprite    - The sprite name to set.
                  fade      - Should the sprite be faded.
    Returns     : void
    Description : Set the sprite to display.
*/
void convwin_inter::set_sprite(string sprite, bool fade)
{
    // Set the sprite
    button_c gadget(CONVWIN_BUTTON, obj);
    gadget = sprite;
    gadget.set_faded(fade);
}

/*
    Parameters  : conv  - Should the conversion type be set.
    Returns     : void
    Description : Reset the contents of the conversion window.
*/
void convwin_run::reset(bool conv)
{
    // Reset the conversion type if required
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_run));

    // Pass on to lower objects
    convwin_inter::reset(conv);
}

/*
    Parameters  : conv  - The tag associated with this converter.
    Returns     : void
    Description : A conversion is about to be performed. This may be used
                  to store the current conversion options.
*/
void convwin_run::pre_convert(string conv)
{
    // Store the conversion type
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_run), conv);

    // Pass on to lower objects
    convwin_inter::pre_convert(conv);
}

/*
    Parameters  : handle    - Handle of the file transfer.
                  type      - The type of the file.
                  orig      - The suggested leaf name.
                  temp      - The file name of the temporary copy.
                  sender    - The sending task's handle.
                  automatic - Is an automatic conversion allowed.
    Returns     : -
    Description : Constructor.
*/
convwin_run::convwin_run(psifs_intercept_handle handle, bits type, string orig,
                         string temp, wimp_t sender, bool automatic)
: convwin_inter(handle, type, FALSE, TRUE, orig, temp, sender, wimp_BROADCAST, "WinConvRun")
{
    // Reset the window contents
    reset();

    // Perform an automatic conversion if enabled
    if (automatic && config_current.get_bool(config_tag_intercept_run_auto))
    {
        convert();
    }
}

/*
    Parameters  : conv  - Should the conversion type be set.
    Returns     : void
    Description : Reset the contents of the conversion window.
*/
void convwin_load::reset(bool conv)
{
    // Reset the conversion type if required
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_load, rx));
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_load));

    // Pass on to lower objects
    convwin_inter::reset(conv);
}

/*
    Parameters  : conv  - The tag associated with this converter.
    Returns     : void
    Description : A conversion is about to be performed. This may be used
                  to store the current conversion options.
*/
void convwin_load::pre_convert(string conv)
{
    // Store the conversion type
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_load), conv);
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_load, rx), conv);

    // Pass on to lower objects
    convwin_inter::pre_convert(conv);
}

/*
    Parameters  : handle    - Handle of the file transfer.
                  type      - The type of the file.
                  orig      - The suggested leaf name.
                  temp      - The file name of the temporary copy.
                  sender    - The sending task's handle.
                  receiver  - The receiving task's handle.
                  automatic - Is an automatic conversion allowed.
    Returns     : -
    Description : Constructor.
*/
convwin_load::convwin_load(psifs_intercept_handle handle, bits type,
                           string orig, string temp, wimp_t sender,
                           wimp_t receiver, bool automatic)
: convwin_inter(handle, type, FALSE, FALSE, orig, temp, sender, receiver, "WinConvLoad")
{
    // Reset the window contents
    reset();

    // Perform an automatic conversion if enabled
    if (automatic && config_current.get_bool(config_tag_intercept_load_auto))
    {
        convert();
    }
}

/*
    Parameters  : conv  - Should the conversion type be set.
    Returns     : void
    Description : Reset the contents of the conversion window.
*/
void convwin_save::reset(bool conv)
{
    // Reset the conversion type if required
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save, tx));
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save));

    // Pass on to lower objects
    convwin_inter::reset(conv);
}

/*
    Parameters  : conv  - The tag associated with this converter.
    Returns     : void
    Description : A conversion is about to be performed. This may be used
                  to store the current conversion options.
*/
void convwin_save::pre_convert(string conv)
{
    // Store the conversion type
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save), conv);
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save, tx), conv);

    // Pass on to lower objects
    convwin_inter::pre_convert(conv);
}

/*
    Parameters  : handle    - Handle of the file transfer.
                  type      - The type of the file.
                  orig      - The suggested leaf name.
                  temp      - The file name of the temporary copy.
                  sender    - The sending task's handle.
                  receiver  - The receiving task's handle.
                  automatic - Is an automatic conversion allowed.
    Returns     : -
    Description : Constructor.
*/
convwin_save::convwin_save(psifs_intercept_handle handle, bits type,
                           string orig, string temp, wimp_t sender,
                           wimp_t receiver, bool automatic)
: convwin_inter(handle, type, TRUE, FALSE, orig, temp, sender, receiver, "WinConvSave")
{
    // Reset the window contents
    reset();

    // Perform an automatic conversion if enabled
    if (automatic && config_current.get_bool(config_tag_intercept_save_auto))
    {
        convert();
    }
}

/*
    Parameters  : conv  - Should the conversion type be set.
    Returns     : void
    Description : Reset the contents of the conversion window.
*/
void convwin_transfer::reset(bool conv)
{
    // Reset the conversion type if required
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save, tx, rx));
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save, tx));
    if (conv) conv = !select(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save));

    // Pass on to lower objects
    convwin_inter::reset(conv);
}

/*
    Parameters  : conv  - The tag associated with this converter.
    Returns     : void
    Description : A conversion is about to be performed. This may be used
                  to store the current conversion options.
*/
void convwin_transfer::pre_convert(string conv)
{
    // Store the conversion type
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save), conv);
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save, tx), conv);
    options_current.set_str(tag_store::tag(convwin_tag_convert, type_tag, convwin_tag_save, tx, rx), conv);

    // Pass on to lower objects
    convwin_inter::pre_convert(conv);
}

/*
    Parameters  : handle    - Handle of the file transfer.
                  type      - The type of the file.
                  orig      - The suggested leaf name.
                  temp      - The file name of the temporary copy.
                  sender    - The sending task's handle.
                  receiver  - The receiving task's handle.
                  automatic - Is an automatic conversion allowed.
    Returns     : -
    Description : Constructor.
*/
convwin_transfer::convwin_transfer(psifs_intercept_handle handle, bits type,
                                   string orig, string temp, wimp_t sender,
                                   wimp_t receiver, bool automatic)
: convwin_inter(handle, type, FALSE, FALSE, orig, temp, sender, receiver, "WinConvTran")
{
    // Reset the window contents
    reset();

    // Perform an automatic conversion if enabled
    if (automatic && config_current.get_bool(config_tag_intercept_save_auto))
    {
        convert();
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Perform any file converter updates required.
*/
void convwin_update(void)
{
    bits type;
    psifs_intercept_type mask;
    char *orig;
    char *temp;
    wimp_t sender;
    wimp_t receiver;

    // Check for new file intercepts
    psifs_intercept_handle handle = psifs_intercept_poll(filer_poll_word, &type, &mask, &orig, &temp, &sender, &receiver);
    while (handle != psifs_INTERCEPT_INVALID)
    {
        bool automatic = !(mask & psifs_INTERCEPT_FORCED);

        // Take the appropriate action
        if (!(mask & psifs_INTERCEPT_FORCED)
            && (filer_eq_task(sender) || filer_eq_task(receiver)))
        {
            // Automatically restart the transfer if to or from PsiFS
            psifsinterceptcontrol_restart(handle);
        }
        else if (mask & psifs_INTERCEPT_LOAD)
        {
            // Intercepted load
            new convwin_load(handle, type, orig, temp,
                             sender, receiver, automatic);
        }
        else if (mask & psifs_INTERCEPT_SAVE)
        {
            // Intercepted save
            new convwin_save(handle, type, orig, temp,
                             sender, receiver, automatic);
        }
        else if (mask & psifs_INTERCEPT_TRANSFER)
        {
            // Intercepted transfer between applications
            new convwin_transfer(handle, type, orig, temp,
                                 sender, receiver, automatic);
        }
        else if (mask & (psifs_INTERCEPT_RUN_ALL
                         | psifs_INTERCEPT_RUN_UNCLAIMED))
        {
            // Intercepted run
            new convwin_run(handle, type, orig, temp, sender, automatic);
        }
        else
        {
            // There should be no other possibilities
            psifsinterceptcontrol_cancel(handle);
        }

        // Check for any more new intercepts
        handle = psifs_intercept_poll(filer_poll_word, &type, &mask, &orig, &temp, &sender, &receiver);
    }
}
