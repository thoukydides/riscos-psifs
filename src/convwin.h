/*
    File        : convwin.h
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

// Only include header file once
#ifndef CONVWIN_H
#define CONVWIN_H

// Include oslib header files
#include "oslib/toolbox.h"

// Include project header files
#include "convobj.h"
#include "psifs.h"
#include "fs.h"

// Converter options window class
class convwin_opt
{
    bool open;                          // Is the window open
    string tag;                         // The file format converter tag
    string res;                         // The name of the window template
    string format;                      // The required options format
    string options;                     // The currently selected options

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle action button selection events in the window.
    */
    static bool action_this(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the window opening.
    */
    static bool open_this(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Handle the window closing.
    */
    static bool close_this(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle);

    /*
        Parameters  : id_block      - A toolbox ID block.
        Returns     : convwin_opt   - Pointer to the corresponding options
                                      window, or NULL if not found.
        Description : Convert a toolbox ID block into an options window
                      pointer.
    */
    static convwin_opt *find(const toolbox_block *id_block);

    /*
        Parameters  : void
        Returns     : void
        Description : Write the options to the window.
    */
    void write();

    /*
        Parameters  : void
        Returns     : void
        Description : Read the options from the window.
    */
    void read();

public:

    toolbox_o obj;                      // The window object

    /*
        Parameters  : conv  - The converter to create an options window for.
        Returns     : -
        Description : Constructor.
    */
    convwin_opt(convobj_obj *conv);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~convwin_opt();

    /*
        Parameters  : void
        Returns     : string    - The options string.
        Description : Save the current options and return the options string.
    */
    string save();
};

// Base converter window class
class convwin_base : public convobj_list
{
    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Update the window.
    */
    static bool update_this(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle);

protected:

    toolbox_o obj;                      // The window object
    bits type;                          // The original file type
    string type_tag;                    // Textual version of the type for tags
    string temp;                        // The name of the temporary file
    string orig;                        // The original filename
    convwin_opt *opt;                   // The options window
    string act_text;                    // The default action button text
    string act_help;                    // The default action button help

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
    convwin_base(bits type, bool dir, bool special, string temp, string res,
                 string orig);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    virtual ~convwin_base();

    /*
        Parameters  : conv  - Should the conversion type be set.
        Returns     : void
        Description : Reset the contents of the conversion window.
    */
    virtual void reset(bool conv = TRUE);

    /*
        Parameters  : tag   - The name of the options tag to use.
        Returns     : bool  - Was a converter selected.
        Description : Attempt to select the converter associated with the
                      specified options tag.
    */
    bool select(string tag);

    /*
        Parameters  : void
        Returns     : void
        Description : Update the status of the conversion window.
    */
    virtual void update();

    /*
        Parameters  : result    - The name of the converted file, or an empty
                                  string if no conversion was performed.
                      open      - Should the window be left open if possible.
        Returns     : bool      - Was the conversion successful.
        Description : Perform a file conversion.
    */
    bool convert(bool open = FALSE);

    /*
        Parameters  : conv  - The tag associated with this converter.
        Returns     : void
        Description : A conversion is about to be performed. This may be used
                      to store the current conversion options.
    */
    virtual void pre_convert(string conv);

    /*
        Parameters  : result    - The name of the converted file, or an empty
                                  string if no conversion was performed.
                      open      - Should the window be left open if possible.
        Returns     : bool      - Should the window be left open.
        Description : A conversion has been performed, and the result is ready
                      for use.
    */
    virtual bool post_convert(string result, bool open);

    /*
        Parameters  : open      - Should the window be left open if possible.
        Returns     : bool      - Should the window be left open.
        Description : Cancel the file conversion.
    */
    bool pre_cancel(bool open);

    /*
        Parameters  : sprite    - The sprite name to set.
                      fade      - Should the sprite be faded.
        Returns     : void
        Description : Set the sprite to display.
    */
    virtual void set_sprite(string sprite, bool fade) = 0;

    /*
        Parameters  : void
        Returns     : bits  - The output file type.
        Description : Return the type of file produced as output by the
                      converter.
    */
    bits get_type() const;

    /*
        Parameters  : id_block      - A toolbox ID block.
        Returns     : convwin_base  - Pointer to the corresponding conversion
                                      window, or NULL if not found.
        Description : Convert a toolbox ID block into a conversion window
                      pointer.
    */
    static convwin_base *find(const toolbox_block *id_block);

public:

    /*
        Parameters  : name              - The name of the file to check.
        Returns     : epoc32_file_uid   - The UID of the file.
        Description : Read and check the UID of a file.
    */
    static epoc32_file_uid get_uid(string name);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Perform the conversion.
    */
    static bool convert_this(bits event_code, toolbox_action *action,
                             toolbox_block *id_block, void *handle);

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Cancel the conversion.
    */
    static bool cancel_this(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle);

    /*
        Parameters  : void
        Returns     : void
        Description : Cancel any file converters.
    */
    static void cancel_all();

    /*
        Parameters  : void
        Returns     : bits      - The number of open file converter windows.
        Description : Check how many file converters are open.
    */
    static bits active_count();
};

// Simple conversion window class
class convwin_win : private convwin_base
{
    string tx;                          // Name of the sending task
    int ref;                            // Reference for last WIMP message

    /*
        Parameters  : event_code    - The event number.
                      action        - The toolbox event.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
        Returns     : int           - Was the event claimed.
        Description : Perform a conversion when a drag ends.
    */
    static bool drag_this(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle);

    /*
        Parameters  : ref           - The message reference.
        Returns     : convwin_win   - Pointer to the corresponding conversion
                                      window, or NULL if not found.
        Description : Convert a wimp message reference into a conversion window
                      pointer.
    */
    static convwin_win *find(int ref);

protected:

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~convwin_win();

    /*
        Parameters  : conv  - Should the conversion type be set.
        Returns     : void
        Description : Reset the contents of the conversion window.
    */
    virtual void reset(bool conv = TRUE);

    /*
        Parameters  : conv  - The tag associated with this converter.
        Returns     : void
        Description : A conversion is about to be performed. This may be used
                      to store the current conversion options.
    */
    virtual void pre_convert(string conv);

    /*
        Parameters  : result    - The name of the converted file, or an empty
                                  string if no conversion was performed.
                      open      - Should the window be left open if possible.
        Returns     : bool      - Should the window be left open.
        Description : A conversion has been performed, and the result is ready
                      for use.
    */
    virtual bool post_convert(string result, bool open);

    /*
        Parameters  : sprite    - The sprite name to set.
                      fade      - Should the sprite be faded.
        Returns     : void
        Description : Set the sprite to display.
    */
    virtual void set_sprite(string sprite, bool fade);

public:
    /*
        Parameters  : type      - The type of the file.
                      orig      - The suggested leaf name.
                      temp      - The file name of the temporary copy.
                      receiver  - The receiving task's handle.
        Returns     : -
        Description : Constructor.
    */
    convwin_win(bits type, string orig, string temp, wimp_t sender);

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataSave wimp message events.
    */
    static int data_save(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataSaveAck wimp message events.
    */
    static int data_save_ack(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataLoad wimp message events.
    */
    static int data_load(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - An unused handle.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataLoadAck wimp message events.
    */
    static int data_load_ack(wimp_message *message, void *handle);
};

// Intercepted file transfer window class
class convwin_inter : public convwin_base
{
protected:

    psifs_intercept_handle handle;      // Handle of the file intercept
    string tx;                          // Name of the sending task
    string rx;                          // Name of the receiving task
    bool active;                        // Is the intercept still active

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
    convwin_inter(psifs_intercept_handle handle, bits type, bool dir,
                  bool special, string orig, string temp, wimp_t sender,
                  wimp_t receiver, string res);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor.
    */
    ~convwin_inter();

    /*
        Parameters  : result    - The name of the converted file, or an empty
                                  string if no conversion was performed.
                      open      - Should the window be left open if possible.
        Returns     : bool      - Should the window be left open.
        Description : A conversion has been performed, and the result is ready
                      for use.
    */
    virtual bool post_convert(string result, bool open);

    /*
        Parameters  : open      - Should the window be left open if possible.
        Returns     : bool      - Should the window be left open.
        Description : Cancel the file conversion.
    */
    bool pre_cancel(bool open);

    /*
        Parameters  : sprite    - The sprite name to set.
                      fade      - Should the sprite be faded.
        Returns     : void
        Description : Set the sprite to display.
    */
    virtual void set_sprite(string sprite, bool fade);
};

// Intercepted file run window class
class convwin_run : private convwin_inter
{
protected:

    /*
        Parameters  : conv  - Should the conversion type be set.
        Returns     : void
        Description : Reset the contents of the conversion window.
    */
    virtual void reset(bool conv = TRUE);

    /*
        Parameters  : conv  - The tag associated with this converter.
        Returns     : void
        Description : A conversion is about to be performed. This may be used
                      to store the current conversion options.
    */
    virtual void pre_convert(string conv);

public:

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
    convwin_run(psifs_intercept_handle handle, bits type, string orig,
                string temp, wimp_t sender, bool automatic);
};

// Intercepted file load window class
class convwin_load : private convwin_inter
{
protected:

    /*
        Parameters  : conv  - Should the conversion type be set.
        Returns     : void
        Description : Reset the contents of the conversion window.
    */
    virtual void reset(bool conv = TRUE);

    /*
        Parameters  : conv  - The tag associated with this converter.
        Returns     : void
        Description : A conversion is about to be performed. This may be used
                      to store the current conversion options.
    */
    virtual void pre_convert(string conv);

public:

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
    convwin_load(psifs_intercept_handle handle, bits type, string orig,
                 string temp, wimp_t sender, wimp_t receiver, bool automatic);
};

// Intercepted file save window class
class convwin_save : private convwin_inter
{
protected:

    /*
        Parameters  : conv  - Should the conversion type be set.
        Returns     : void
        Description : Reset the contents of the conversion window.
    */
    virtual void reset(bool conv = TRUE);

    /*
        Parameters  : conv  - The tag associated with this converter.
        Returns     : void
        Description : A conversion is about to be performed. This may be used
                      to store the current conversion options.
    */
    virtual void pre_convert(string conv);

public:

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
    convwin_save(psifs_intercept_handle handle, bits type, string orig,
                 string temp, wimp_t sender, wimp_t receiver, bool automatic);
};

// Intercepted file transfer window class
class convwin_transfer : private convwin_inter
{
protected:

    /*
        Parameters  : conv  - Should the conversion type be set.
        Returns     : void
        Description : Reset the contents of the conversion window.
    */
    virtual void reset(bool conv = TRUE);

    /*
        Parameters  : conv  - The tag associated with this converter.
        Returns     : void
        Description : A conversion is about to be performed. This may be used
                      to store the current conversion options.
    */
    virtual void pre_convert(string conv);

public:

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
    convwin_transfer(psifs_intercept_handle, bits type, string orig,
                     string temp, wimp_t sender, wimp_t receiver,
                     bool automatic);
};

/*
    Parameters  : void
    Returns     : void
    Description : Perform any file converter updates required.
*/
void convwin_update();

#endif
