/*
    File        : printjbwin.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print job window handling for the PsiFS filer.

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
#include "printjbwin.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/hourglass.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/pdriver.h"
#include "oslib/window.h"
#include "event.h"

// Include cathlibcpp header files
#include "list.h"

// Include alexlib header files
#include "actionbutton_c.h"
#include "displayfield_c.h"
#include "numberrange_c.h"

// Include project header files
#include "config.h"
#include "filer.h"
#include "printrendg.h"
#include "scrap.h"

// Number of OS units to enlarge print rectangle by
#define PRINTJBWIN_BOX_MARGIN (3)

// Filename for printing direct to the printer
#define PRINTJBWIN_PRINTER_TEMP "<Printer$Temp>"
#define PRINTJBWIN_PRINTER_DIRECT "printer:"

// Print job control window gagdets
#define PRINTJBWIN_CANCEL ((toolbox_c) 0x00)
#define PRINTJBWIN_STATUS ((toolbox_c) 0x10)
#define PRINTJBWIN_PRINTER ((toolbox_c) 0x11)
#define PRINTJBWIN_PAGES ((toolbox_c) 0x12)
#define PRINTJBWIN_PRINTED ((toolbox_c) 0x13)

// List of print job status windows
static list<printjbwin_win *> printjbwin_list;

/*
    Parameters  : job       - The print job to print.
                  top_left  - The coordinates for the top-left corner of
                              the window, or NULL for the default.
    Returns     : -
    Description : Constructor.
*/
printjbwin_win::printjbwin_win(printjbobj_obj &job, const os_coord *top_left)
: job(job)
{
    // Initially no pages printed
    pages = 0;
    my_ref = 0;

    // Create the window
    obj = toolbox_create_object(0, (toolbox_id) "WinPrintJb");

    // Set the client handle
    toolbox_set_client_handle(0, obj, this);

    // Register toolbox handlers
    event_register_toolbox_handler(obj,
                                   action_ACTION_BUTTON_SELECTED,
                                   action, NULL);
    event_register_toolbox_handler(obj,
                                   action_WINDOW_DIALOGUE_COMPLETED,
                                   close, NULL);
    event_register_message_handler(message_PRINT_ERROR,
                                   print_error, this);
    event_register_message_handler(message_PRINT_FILE,
                                   print_file, this);
    event_register_message_handler(message_PRINT_TYPE_ODD,
                                   print_type_odd, this);
    event_register_message_handler(message_DATA_SAVE_ACK,
                                   data_save_ack, this);
    event_register_message_handler(message_DATA_LOAD_ACK,
                                   data_load_ack, this);
    event_register_wimp_handler(event_ANY, wimp_USER_MESSAGE_ACKNOWLEDGE,
                                acknowledge, this);

    // Update the window before showing for the first time
    update(TRUE);

    // Open at centre of the screen unless a position specified
    toolbox_show_object(0, obj,
                        top_left
                        ? toolbox_POSITION_TOP_LEFT
                        : toolbox_POSITION_CENTRED,
                        (toolbox_position *) top_left,
                        toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

    // Add to the list of print job status windows
    printjbwin_list.push_back((printjbwin_win *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printjbwin_win::~printjbwin_win()
{
    // Remove from the list of print job status windows
    printjbwin_list.remove((printjbwin_win *)(this));

    // Deregister handlers to update the window
    event_deregister_wimp_handler(event_ANY, wimp_USER_MESSAGE_ACKNOWLEDGE,
                                  acknowledge, this);
    event_deregister_message_handler(message_DATA_LOAD_ACK,
                                     data_load_ack, this);
    event_deregister_message_handler(message_DATA_SAVE_ACK,
                                     data_save_ack, this);
    event_deregister_message_handler(message_PRINT_TYPE_ODD,
                                     print_type_odd, this);
    event_deregister_message_handler(message_PRINT_FILE,
                                     print_file, this);
    event_deregister_message_handler(message_PRINT_ERROR,
                                     print_error, this);
    event_deregister_toolbox_handler(obj,
                                     action_WINDOW_DIALOGUE_COMPLETED,
                                     close, NULL);
    event_deregister_toolbox_handler(obj,
                                     action_ACTION_BUTTON_SELECTED,
                                     action, NULL);

    // Destroy the window
    toolbox_delete_object(0, obj);
}

/*
    Parameters  : printer   - Has the selected printer changed.
    Returns     : void
    Description : Update the status of this print job status window.
*/
void printjbwin_win::update(bool printer)
{
    NOT_USED(printer);

    // Check for the print job being cancelled or completed
    if (job.cancelled()
        || (!my_ref && job.complete() && (pages == job.pages())))
    {
        // Close the window and discard the print job
        delete this;
    }
    else
    {
        // Set the printer name if changed
        if (printer)
        {
            char *desc;
            driver = !xpdriver_info(NULL, NULL, NULL, NULL, &desc,
                                    NULL, NULL, NULL)
                     && desc && *desc;
            displayfield_c(PRINTJBWIN_PRINTER, obj) = driver ? string(desc) : filer_msgtrans("PrnPdNo");
        }

        // Update the number of pages received
        numberrange_c(PRINTJBWIN_PAGES, obj) = job.pages();

        // Check whether the print job is complete
        displayfield_c(PRINTJBWIN_STATUS, obj) = filer_msgtrans(job.complete() ? "PrnCtCp" : "PrnCtRx");

        // Check whether any more pages are ready to be printed
        if (driver && !my_ref && (pages < job.pages())
            && (!config_current.get_bool(config_tag_print_print_wait)
                || job.complete())
            && !printer)
        {
            // Send Message_PrintSave to start the print process
            wimp_message message;
            message.action = message_PRINT_SAVE;
            message.your_ref = 0;
            message.data.data_xfer.w = wimp_BACKGROUND;
            message.data.data_xfer.i = wimp_ICON_WINDOW;
            message.data.data_xfer.pos.x = message.data.data_xfer.pos.y = 0;
            message.data.data_xfer.est_size = 0;
            message.data.data_xfer.file_type = osfile_TYPE_PRINTOUT;
            strcpy(message.data.data_xfer.file_name,
                   filer_msgtrans("_TaskName").c_str());
            message.size = ALIGN(strchr(message.data.data_xfer.file_name, '\0')
                                 - (char *) &message + 1);
            wimp_send_message(wimp_USER_MESSAGE_RECORDED, &message,
                              wimp_BROADCAST);
            my_ref = message.my_ref;
        }
    }
}

/*
    Parameters  : file      - The name of the output file.
    Returns     : void
    Description : Print as many pages as possible.
*/
void printjbwin_win::print(const string &file)
{
    os_error *err = NULL;

    // Choose the print job title
    string title =  filer_msgtrans("PrnPrJb");

    // Choose the initial page range
    int first_page = pages + 1;
    int last_page = job.pages();

    // Read the printer characteristics
    pdriver_info_type type;
    int xres;
    int yres;
    pdriver_features features;
    pdriver_info(&type, &xres, &yres, &features, NULL, NULL, NULL, NULL);

    // Calculate the printer resolution in dots per OS unit
    int resolution = int(transform_to_millipoint(transform_to_os.inverse(1))
                         * double(xres < yres ? yres : xres) / pdriver_INCH);

    // Read the page size
    int xsize;
    int ysize;
    pdriver_page_size(&xsize, &ysize, NULL, NULL, NULL, NULL);

    // Open the printer device
    os_fw handle;
    os_fw old_job;
    err = xosfind_openoutw(osfind_NO_PATH | osfind_ERROR_IF_DIR,
                           file.c_str(), NULL, &handle);
    if (err) handle = 0;

    // Turn the hourglass on
    xhourglass_on();

    // Pre-declare fonts if required
    if (!err && (features & pdriver_FEATURE_DECLARE_FONT))
    {
        // Turn the hourglass LEDs on while processing fonts
        xhourglass_leds(3, 0, NULL);

        // Set the status text
        displayfield_c(PRINTJBWIN_STATUS, obj) = filer_msgtrans("PrnJbFn");

        // Determine the fonts used by all available pages
        set<string, less<string> > fonts;
        set_const_iterator<string, less<string> > i;
        for (int page = first_page; page <= last_page; page++)
        {
            // Set the hourglass percentage
            xhourglass_percentage(((page - first_page) * 100)
                                  / (last_page - first_page + 1));

            // Process this page
            printrendg_graph graph;
            job[page].render(graph);

            // Obtain the set of fonts
            drawobj_file drawfile = graph.get_draw_file();
            set<string, less<string> > page_fonts;
            page_fonts = drawfile.get_font_table().get_fonts();

            // Add to the composite set of fonts
            for (i = page_fonts.begin(); i != page_fonts.end(); i++)
            {
                fonts.insert(*i);
            }
        }

        // Start the print job
        err = xpdriver_select_jobw(handle, title.c_str(), &old_job);

        // Declare all of the fonts
        for (i = fonts.begin(); !err && (i != fonts.end()); i++)
        {
            err = xpdriver_declare_font(0, (*i).c_str(), pdriver_KERNED);
        }

        // Mark the end of the list of fonts
        if (!err) err = xpdriver_declare_font(0, NULL, 0);

        // Deselect the print job
        if (!err) err = xpdriver_select_job(old_job, NULL, NULL);
    }

    // Print all of the pages
    for (int page = first_page; !err && (page <= last_page); page++)
    {
        // Set the hourglass percentage and turn off the LEDs
        xhourglass_percentage(((page - first_page) * 100)
                              / (last_page - first_page + 1));
        xhourglass_leds(0, 0, NULL);

        // Set the status text
        char str[10];
        sprintf(str, "%i", page);
        displayfield_c(PRINTJBWIN_STATUS, obj) = filer_msgtrans("PrnJbPr", str);

        // Render this page
        printrendg_graph graph;
        job[page].render(graph);
        drawobj_file drawfile = graph.get_draw_file();

        // Turn one hourglass LED on
        xhourglass_leds(1, 0, NULL);

        // Select the print job
        err = xpdriver_select_jobw(handle, title.c_str(), &old_job);

        // Specify the rectangle to be printed
        os_box box = drawfile.get_box();
        box.x0 -= PRINTJBWIN_BOX_MARGIN;
        box.y0 -= PRINTJBWIN_BOX_MARGIN;
        box.x1 += PRINTJBWIN_BOX_MARGIN;
        box.y1 += PRINTJBWIN_BOX_MARGIN;
        int trfm[4];
        trfm[0] = trfm[3] = 1 << 16;
        trfm[1] = trfm[2] = 0;
        os_coord pos;
        pos.x = transform_to_millipoint(transform_to_os.inverse(box.x0));
        pos.y = ysize
                + transform_to_millipoint(transform_to_os.inverse(box.y0));
        err = xpdriver_give_rectangle(0, &box, (os_hom_trfm *) trfm, &pos,
                                      os_COLOUR_WHITE);

        // Print this page
        os_box rect;
        bool more;
        if (!err) err = xpdriver_draw_page(1, &rect, 0, NULL, &more, NULL);
        while (!err && more)
        {
            // Toggle the hourglass LEDs
            xhourglass_leds(3, 3, NULL);

            // Print this rectangle
            drawfile.paint(transform_internal(), rect, resolution);

            // Get the next rectangle to print
            err = xpdriver_get_rectangle(&rect, &more, NULL);
        }

        // Deselect the print job
        if (!err) err = xpdriver_select_job(old_job, NULL, NULL);

        // Check for more pages if fonts not pre-declared
        job.update();
        if (!(features & pdriver_FEATURE_DECLARE_FONT)) last_page = job.pages();
        if (!err && (page < last_page) && job.cancelled())
        {
            last_page = page;
            err = xpdriver_cancel_job(handle);
        }

        // Update the page counts
        numberrange_c(PRINTJBWIN_PAGES, obj) = job.pages();
        numberrange_c(PRINTJBWIN_PRINTED, obj) = page;
    }

    // Turn the hourglass off
    xhourglass_off();

    // End the print job
    if (!err) err = xpdriver_end_job(handle);
    if (err && handle) xpdriver_abort_job(handle);

    // Attempt to set the file type (may fail if not a normal file)
    if (!err)
    {
        xosfile_set_type(file.c_str(),
                         (type >> 16) == pdriver_TYPE_PS
                         ? osfile_TYPE_POSTSCRIPT
                         : osfile_TYPE_PRINTOUT);
    }

    // Close the output file
    if (handle) xosfind_closew(handle);

    // Handle any error that may have been produced
    if (err)
    {
        // Cancel the print job to prevent further errors
        delete this;

        // Enable simple error handling
        filer_error_allowed++;

        // Report the error
        os_generate_error(err);

        // Restore normal error handling
        filer_error_allowed--;
    }

    // Update the count of pages printed
    pages = last_page;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle an action button click.
*/
bool printjbwin_win::action(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    printjbwin_win *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Perform the appropriate action
        switch (id_block->this_cmp)
        {
            case PRINTJBWIN_CANCEL:
                // Discard the print job
                delete ptr;
                break;

            default:
                // No interested in any other buttons
                break;
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
    Description : Handle the window being closed.
*/
bool printjbwin_win::close(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    printjbwin_win *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Discard the print job
        if (ptr) delete ptr;
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_PrintError messages.
*/
int printjbwin_win::print_error(wimp_message *message, void *handle)
{
    printjbwin_win *ptr = (printjbwin_win *) handle;
    bool claimed = FALSE;

    // Only process the message if expected
    if (message->your_ref == ptr->my_ref)
    {
        // Discard the print job
        delete ptr;

        // Enable simple error handling
        filer_error_allowed++;

        // Report an error
        if (message->size <= 20)
        {
            os_error err;

            // Generate a default error
            err.errnum = 0;
            filer_msgtrans(err.errmess, sizeof(err.errmess), "ErrPrnt");
            os_generate_error(&err);
        }
        else
        {
            // Report the supplied error
            os_generate_error((os_error *) &message->data);
        }

        // Restore normal error handling
        filer_error_allowed--;

        // Claim the message
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_PrintFile messages.
*/
int printjbwin_win::print_file(wimp_message *message, void *handle)
{
    printjbwin_win *ptr = (printjbwin_win *) handle;
    bool claimed = FALSE;

    // Only process the message if expected
    if (message->your_ref == ptr->my_ref)
    {
        /*
            The Message_PrintFile message is ignored; the printer manager
            will send either Message_PrintTypeOdd or Message_DataSaveAck
            to handle the actual printing.
        */

        // Claim the message
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_PrintTypeOdd messages.
*/
int printjbwin_win::print_type_odd(wimp_message *message, void *handle)
{
    printjbwin_win *ptr = (printjbwin_win *) handle;
    bool claimed = FALSE;

    // Only process the message if expected
    if (message->your_ref == ptr->my_ref)
    {
        // Clear the saved message reference
        ptr->my_ref = 0;

        // No active print job, so print directly to the printer
        ptr->print(PRINTJBWIN_PRINTER_DIRECT);

        // Send a Message_PrintTypeKnown back to the printer manager
        message->size = ((char *) &message->data) - ((char *) message);
        message->action = message_PRINT_TYPE_KNOWN;
        message->your_ref = message->my_ref;
        wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

        // Claim the message
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSaveAck messages.
*/
int printjbwin_win::data_save_ack(wimp_message *message, void *handle)
{
    printjbwin_win *ptr = (printjbwin_win *) handle;
    bool claimed = FALSE;

    // Only process the message if expected
    if (message->your_ref == ptr->my_ref)
    {
        // Clear the saved message reference
        ptr->my_ref = 0;

        // Print to the specified file
        ptr->print(message->data.data_xfer.file_name);

        // Send a Message_DataLoad back to the printer manager
        message->action = message_DATA_LOAD;
        message->your_ref = message->my_ref;
        osfile_read_stamped_no_path(message->data.data_xfer.file_name,
                                    NULL, NULL,
                                    &message->data.data_xfer.est_size, NULL,
                                    &message->data.data_xfer.file_type);
        wimp_send_message(wimp_USER_MESSAGE_RECORDED, message, message->sender);
        ptr->my_ref = message->my_ref;

        // Claim the message
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoadAck messages.
*/
int printjbwin_win::data_load_ack(wimp_message *message, void *handle)
{
    printjbwin_win *ptr = (printjbwin_win *) handle;
    bool claimed = FALSE;

    // Only process the message if expected
    if (message->your_ref == ptr->my_ref)
    {
        // Clear the saved message reference
        ptr->my_ref = 0;

        // Claim the message
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle unacknowledged messages.
*/
bool printjbwin_win::acknowledge(wimp_event_no event_code, wimp_block *block,
                                 toolbox_block *id_block, void *handle)
{
    printjbwin_win *ptr = (printjbwin_win *) handle;
    wimp_message *message = &block->message;
    bool claimed = FALSE;

    NOT_USED(event_code);
    NOT_USED(id_block);

    // Only process the message if expected
    if (message->my_ref == ptr->my_ref)
    {
        // Clear the saved message reference
        ptr->my_ref = 0;

        // Action depends on the type of mesage
        switch (message->action)
        {
        case message_PRINT_SAVE:
            // No printer manager, so print directly to the printer
            ptr->print(PRINTJBWIN_PRINTER_DIRECT);
            break;

        case message_DATA_LOAD:
            // Printer manager failed to acknowledge transfer of data
            break;

        default:
            // Not interested in other messages
            break;
        }

        // Claim the message
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}

/*
    Parameters  : id_block          - A toolbox ID block.
    Returns     : printjbwin_win    - Pointer to the corresponding print job
                                      status window, or NULL if not found.
    Description : Convert a toolbox ID block into a print job status window
                  pointer.
*/
printjbwin_win *printjbwin_win::find(const toolbox_block *id_block)
{
    // Choose the ancestor object ID
    toolbox_o id = id_block->ancestor_obj == toolbox_NULL_OBJECT
                   ? id_block->this_obj
                   : id_block->ancestor_obj;

    // Attempt to read the client handle
    printjbwin_win *ptr;
    if (xtoolbox_get_client_handle(0, id, (void **) &ptr)) ptr = NULL;

    // Return the pointer
    return ptr;
}

/*
    Parameters  : printer   - Has the selected printer changed.
    Returns     : void
    Description : Update any open print job status windows.
*/
void printjbwin_win::update_all(bool printer)
{
    list_iterator<printjbwin_win *> i;

    // Update all of the print job status windows
    for (i = printjbwin_list.begin(); i != printjbwin_list.end(); i++)
    {
        (*i)->update(printer);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Cancel any open print job status windows.
*/
void printjbwin_win::cancel_all()
{
    // Delete all windows
    while (!printjbwin_list.empty()) delete printjbwin_list.front();
}

/*
    Parameters  : void
    Returns     : bits      - The number of open print job status windows.
    Description : Check how many print job status windows are open.
*/
bits printjbwin_win::active_count()
{
    return printjbwin_list.size();
}
