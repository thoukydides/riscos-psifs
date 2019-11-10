/*
    File        : printprwin.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Print preview window handling for the PsiFS filer.

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
#include "printprwin.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/colourtrans.h"
#include "oslib/menu.h"
#include "oslib/osfile.h"
#include "oslib/pdriver.h"
#include "oslib/saveas.h"
#include "oslib/scale.h"
#include "oslib/window.h"
#include "event.h"

// Include cathlibcpp header files
#include "fstream.h"
#include "list.h"

// Include alexlib header files
#include "button_c.h"
#include "numberrange_c.h"

// Include project header files
#include "config.h"
#include "filer.h"
#include "printjbwin.h"
#include "printrendg.h"
#include "printrendt.h"

// Print job preview tool window gadgets
#define PRINTPRWIN_PAGE ((toolbox_c) 0x10)
#define PRINTPRWIN_PAGES ((toolbox_c) 0x11)
#define PRINTPRWIN_COMMENT ((toolbox_c) 0x12)

// Print job preview menu components
#define PRINTPRWIN_MENU_ALL ((toolbox_c) 0x00)
#define PRINTPRWIN_MENU_PAGE ((toolbox_c) 0x01)
#define PRINTPRWIN_MENU_SCALE ((toolbox_c) 0x02)
#define PRINTPRWIN_MENU_ALL_PRINT ((toolbox_c) 0x10)
#define PRINTPRWIN_MENU_ALL_SAVE_TEXT ((toolbox_c) 0x11)
#define PRINTPRWIN_MENU_PAGE_PRINT ((toolbox_c) 0x20)
#define PRINTPRWIN_MENU_PAGE_SAVE ((toolbox_c) 0x21)
#define PRINTPRWIN_MENU_PAGE_SAVE_DRAW ((toolbox_c) 0x30)
#define PRINTPRWIN_MENU_PAGE_SAVE_TEXT ((toolbox_c) 0x31)
#define PRINTPRWIN_MENU_PAGE_SAVE_DATA ((toolbox_c) 0x32)

// Default paper size in millipoints (A4: 8.27" x 11.69")
static const int printprwin_width = 595350;
static const int printprwin_height = 841995;

// No page pattern repeat distance in OS units
#define PRINTPRWIN_PATTERN_SPACE (32)

// Default scale in percent
static const int printprwin_scale = 100;
#define PRINTPRWIN_SCALE_MILLIPOINT_PER_OS (400)

// List of print job preview windows
static list<printprwin_win *> printprwin_list;

/*
    Parameters  : job       - The print job to preview.
                  top_left  - The coordinates for the top-left corner of
                              the window, or NULL for the default.
    Returns     : -
    Description : Constructor.
*/
printprwin_win::printprwin_win(printjbobj_obj &job, const os_coord *top_left)
: job(job)
{
    // Initially no page being displayed
    page = 0;
    scale = config_current.exist(config_tag_print_preview_scale)
            ? config_current.get_num(config_tag_print_preview_scale)
            : printprwin_scale;
    drawfile_valid = FALSE;

    // Create the window
    obj = toolbox_create_object(0, (toolbox_id) "WinPrintPr");
    window_get_tool_bars(window_TOOL_BAR_ITL, obj, NULL, &tool, NULL, NULL);
    menu = window_get_menu(0, obj);
    menu_job = menu_get_sub_menu_show(0, menu, PRINTPRWIN_MENU_ALL);
    menu_page = menu_get_sub_menu_show(0, menu, PRINTPRWIN_MENU_PAGE);

    // Set the client handle
    toolbox_set_client_handle(0, obj, this);

    // Register handlers to update the window
    event_register_toolbox_handler(obj,
                                   action_WINDOW_DIALOGUE_COMPLETED,
                                   close, NULL);
    event_register_toolbox_handler(tool,
                                   action_NUMBER_RANGE_VALUE_CHANGED,
                                   set_page, NULL);
    event_register_toolbox_handler(event_ANY,
                                   action_SCALE_ABOUT_TO_BE_SHOWN,
                                   pre_scale, NULL);
    event_register_toolbox_handler(event_ANY,
                                   action_SCALE_APPLY_FACTOR,
                                   set_scale, NULL);
    event_register_toolbox_handler(event_ANY,
                                   action_SAVE_AS_ABOUT_TO_BE_SHOWN,
                                   pre_save, NULL);
    event_register_toolbox_handler(event_ANY,
                                   action_SAVE_AS_SAVE_TO_FILE,
                                   save_raw, NULL);
    event_register_toolbox_handler(event_ANY,
                                   action_MENU_SELECTION,
                                   menu_raw, NULL);
    event_register_wimp_handler(obj, wimp_REDRAW_WINDOW_REQUEST,
                                redraw_raw, NULL);

    // Update the window before showing for the first time
    update(TRUE);

    // Open at centre of the screen unless a position specified
    toolbox_show_object(0, obj,
                        top_left
                        ? toolbox_POSITION_TOP_LEFT
                        : toolbox_POSITION_CENTRED,
                        (toolbox_position *) top_left,
                        toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

    // Add to the list of print job preview windows
    printprwin_list.push_back((printprwin_win *)(this));
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor.
*/
printprwin_win::~printprwin_win()
{
    // Remove from the list of print job preview windows
    printprwin_list.remove((printprwin_win *)(this));

    // Deregister handlers to update the window
    event_deregister_wimp_handler(obj, wimp_REDRAW_WINDOW_REQUEST,
                                  redraw_raw, NULL);
    event_deregister_toolbox_handler(event_ANY,
                                     action_MENU_SELECTION,
                                     menu_raw, NULL);
    event_deregister_toolbox_handler(event_ANY,
                                     action_SAVE_AS_SAVE_TO_FILE,
                                     save_raw, NULL);
    event_deregister_toolbox_handler(event_ANY,
                                     action_SAVE_AS_ABOUT_TO_BE_SHOWN,
                                     pre_save, NULL);
    event_deregister_toolbox_handler(event_ANY,
                                     action_SCALE_APPLY_FACTOR,
                                     set_scale, NULL);
    event_deregister_toolbox_handler(event_ANY,
                                     action_SCALE_ABOUT_TO_BE_SHOWN,
                                     pre_scale, NULL);
    event_deregister_toolbox_handler(tool,
                                     action_NUMBER_RANGE_VALUE_CHANGED,
                                     set_page, NULL);
    event_deregister_toolbox_handler(obj,
                                     action_WINDOW_DIALOGUE_COMPLETED,
                                     close, NULL);

    // Destroy the window
    toolbox_delete_object(0, obj);
}

/*
    Parameters  : void
    Returns     : void
    Description : Redraw the print job preview window following a change
                  to the scale, page or printer.
*/
void printprwin_win::refresh()
{
    // Reset the draw file showing the paper margins
    printable = drawobj_file();

    // Read the page size and printable area
    os_coord mp_paper;
    os_box mp_printable;
    if (!xpdriver_page_size(&mp_paper.x, &mp_paper.y,
                            &mp_printable.x0, &mp_printable.y0,
                            &mp_printable.x1, &mp_printable.y1))
    {
        // Shift the origin to the top left corner of the page
        mp_printable.y0 -= mp_paper.y;
        mp_printable.y1 -= mp_paper.y;

        // Construct a draw file to show the paper margins
        drawobj_path *obj = new drawobj_path;
        obj->set_fill(os_COLOUR_VERY_LIGHT_GREY);
        obj->set_winding_rule(drawobj_path::winding_even_odd);
        os_box paper;
        paper.x0 = 0;
        paper.y0 = -transform_to_millipoint.inverse(mp_paper.y);
        paper.x1 = transform_to_millipoint.inverse(mp_paper.x);
        paper.y1 = 0;
        obj->add_rectangle(paper);
        obj->add_rectangle(transform_to_millipoint.inverse(mp_printable));
        obj->add_end();
        printable.add(obj);
    }
    else
    {
        // Use the default page size without margins
        mp_paper.x = printprwin_width;
        mp_paper.y = printprwin_height;
    }

    // Calculate the new window extent
    os_coord os_paper = transform_to_os(transform_to_millipoint.inverse(mp_paper));
    os_box extent;
    window_get_extent(0, obj, &extent);
    extent.x1 = (os_paper.x * scale) / 100;
    extent.y0 = (-os_paper.y * scale) / 100;

    // Ensure that the visible work area is within the new window extent
    wimp_window_state state;
    state.w = window_get_wimp_handle(0, obj);
    wimp_get_window_state(&state);
    if ((extent.x1 - extent.x0) < (state.visible.x1 - state.visible.x0))
    {
        state.visible.x1 = state.visible.x0 + (extent.x1 - extent.x0);
    }
    if ((extent.y1 - extent.y0) < (state.visible.y1 - state.visible.y0))
    {
        state.visible.y0 = state.visible.y1 - (extent.y1 - extent.y0);
    }
    if (extent.x1 < (state.visible.x1 - state.visible.x0 + state.xscroll))
    {
        state.xscroll = extent.x1 - (state.visible.x1 - state.visible.x0);
    }
    if ((state.visible.y0 - state.visible.y1 + state.yscroll) < extent.y0)
    {
        state.yscroll = extent.y0 - (state.visible.y0 - state.visible.y1);
    }
    wimp_open_window((wimp_open *) &state);

    // Resize the toolbar to fit
    wimp_window_state tool_state;
    tool_state.w = window_get_wimp_handle(0, tool);
    wimp_get_window_state(&tool_state);
    tool_state.visible.x1 = tool_state.visible.x0
                            + (state.visible.x1 - state.visible.x0);
    wimp_open_window((wimp_open *) &tool_state);

    // Set the new window extent
    window_set_extent(0, obj, &extent);

    // Force a redraw of the window
    window_force_redraw(0, obj, &extent);

    // Render the page
    if (page && !drawfile_valid)
    {
        // Render the current page
        printrendg_graph graph;
        job[page].render(graph);
        drawfile = graph.get_draw_file();
        drawfile_valid = TRUE;

        // Handle any errors
        const deque<string> &errors = graph.get_errors();
        if (!errors.empty())
        {
            button_c(PRINTPRWIN_COMMENT, tool) = filer_msgtrans(graph ? "PrnPdWn" : "PrnPdEr", errors.front().c_str());
        }
    }
}

/*
    Parameters  : draw          - Details of the rectangle to redraw.
    Returns     : void
    Description : Redraw the specified rectangle of the print job preview
                  window.
*/
void printprwin_win::redraw(wimp_draw &draw)
{
    // Show the print margin
    transform_internal trfm_scaled(draw, scale);
    printable.paint(trfm_scaled, draw.clip);

    // Page content depends on whether any pages have been received
    if (drawfile_valid)
    {
        // Render the current page
        drawfile.paint(trfm_scaled, draw.clip);
    }
    else
    {
        // Calculate a pattern aligned box covering the clipping rectangle
        transform trfm_redraw(draw);
        os_box pattern = trfm_redraw.inverse(draw.clip);
        pattern.x0 = (pattern.x0 / PRINTPRWIN_PATTERN_SPACE)
                     * PRINTPRWIN_PATTERN_SPACE;
        pattern.y0 = (pattern.y0 / PRINTPRWIN_PATTERN_SPACE)
                     * PRINTPRWIN_PATTERN_SPACE;
        pattern.x1 = (pattern.x1 / PRINTPRWIN_PATTERN_SPACE + 1)
                     * PRINTPRWIN_PATTERN_SPACE;
        pattern.y1 = (pattern.y1 / PRINTPRWIN_PATTERN_SPACE + 1)
                     * PRINTPRWIN_PATTERN_SPACE;
        os_box aligned = trfm_redraw(pattern);

        // Plot a pretty pattern
        wimp_set_colour(wimp_COLOUR_MID_LIGHT_GREY);
        for (int y = aligned.y0 - (aligned.x1 - aligned.x0);
             y < aligned.y1;
             y += PRINTPRWIN_PATTERN_SPACE)
        {
            os_plot(os_MOVE_TO, aligned.x0, y);
            os_plot(os_PLOT_TO, aligned.x1, y + aligned.x1 - aligned.x0);
            os_plot(os_MOVE_TO, aligned.x1, y);
            os_plot(os_PLOT_TO, aligned.x0, y + aligned.x1 - aligned.x0);
        }
    }
}

/*
    Parameters  : printer   - Has the selected printer changed.
    Returns     : void
    Description : Update the status of this print job preview window.
*/
void printprwin_win::update(bool printer)
{
    // Check for the print job being cancelled
    if (job.cancelled())
    {
        // Close the window and discard the print job
        delete this;
    }
    else
    {
        // Get the printer details if changed
        if (printer)
        {
            // Get the printer name
            char *desc;
            driver = !xpdriver_info(NULL, NULL, NULL, NULL, &desc,
                                    NULL, NULL, NULL)
                     && desc && *desc;
            button_c(PRINTPRWIN_COMMENT, tool) = filer_msgtrans("PrnPdDr", (driver ? string(desc) : filer_msgtrans("PrnPdNo")).c_str());

            // The margins may have changed, so force a redraw of the window
            refresh();
        }

        // Update the number of pages received
        bits pages = job.pages();
        numberrange_c total(PRINTPRWIN_PAGES, tool);
        numberrange_c current(PRINTPRWIN_PAGE, tool);
        if (total() != pages)
        {
            // Update the total number of pages
            total = pages;

            // Update the current page
            current.set_upper_bound(pages);
            if (pages && !page)
            {
                current.set_lower_bound(1);
                current.set_faded(FALSE);
                current = page = 1;
                refresh();
            }
        }

        // Enable the appropriate menu options
        menu_set_fade(0, menu, PRINTPRWIN_MENU_ALL,
                      !driver && (!pages || !job.complete()));
        menu_set_fade(0, menu, PRINTPRWIN_MENU_PAGE, !pages);
        menu_set_fade(0, menu, PRINTPRWIN_MENU_SCALE, !pages);
        menu_set_fade(0, menu_job, PRINTPRWIN_MENU_ALL_PRINT, !driver);
        menu_set_fade(0, menu_job, PRINTPRWIN_MENU_ALL_SAVE_TEXT,
                      !pages || !job.complete());
        menu_set_fade(0, menu_page, PRINTPRWIN_MENU_PAGE_PRINT, !driver);
    }
}

/*
    Parameters  : name          - The name of the file to save.
    Returns     : bool          - Was the file saved successfully.
    Description : Save the current page as a draw file.
*/
bool printprwin_win::save_draw(const string &name)
{
    // Open the output file
    ofstream s(name.c_str(),
               ios_base::out | ios_base::trunc | ios_base::binary);;
    if (!s) return FALSE;

    // Save the file
    drawfile.save(s);
    s.close();

    // Set the filetype
    osfile_set_type(name.c_str(), osfile_TYPE_DRAW);

    // The save was successful if this point reached
    return TRUE;
}

/*
    Parameters  : name          - The name of the file to save.
                  all           - Should the whole job be saved.
    Returns     : bool          - Was the file saved successfully.
    Description : Save either the current page of the whole print job
                  as a text file.
*/
bool printprwin_win::save_text(const string &name, bool all)
{
    // Open the output file
    ofstream s(name.c_str());
    if (!s) return FALSE;

    // Render the file and append any errors that were logged
    printrendt_text text(s);
    if (all) job.render(text);
    else { job[page].render(text); }

    // Append any errors to the output
    const deque<string> &errors = text.get_errors();
    if (!errors.empty())
    {
        const string *i;
        s << filer_msgtrans("PrnTxEr") << endl;
        for (i = errors.begin(); i != errors.end(); i++)
        {
            s << *i << endl;
        }
    }
    s.close();

    // Report any fatal error produced
    if (!text && !errors.empty())
    {
        // Enable simple error handling
        filer_error_allowed++;

        // Generate the error
        os_error err;
        err.errnum = 0;
        strcpy(err.errmess, errors.front().substr(0, sizeof(err.errmess) - 1).c_str());
        os_generate_error(&err);

        // Restore normal error handling
        filer_error_allowed--;
    }

    // The save was successful if this point reached
    return TRUE;
}

/*
    Parameters  : name          - The name of the file to save.
    Returns     : bool          - Was the file saved successfully.
    Description : Save the current page as a raw file.
*/
bool printprwin_win::save_raw(const string &name)
{
    // Save the raw print job data for the current page
    job[page].save(name);

    // File saved
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
bool printprwin_win::close(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    printprwin_win *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Discard the print job
        delete ptr;
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
    Description : Handle the page number changing.
*/
bool printprwin_win::set_page(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    printprwin_win *ptr = find(id_block);
    numberrange_action_value_changed *number = (numberrange_action_value_changed *) &action->data;

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Set the new page number if valid and changed
        if ((1 <= number->new_value) && (number->new_value <= ptr->job.pages())
            && (number->new_value != ptr->page))
        {
            ptr->page = number->new_value;
            ptr->drawfile_valid = FALSE;
            ptr->refresh();
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
    Description : Handle the scale window being opened.
*/
bool printprwin_win::pre_scale(bits event_code, toolbox_action *action,
                               toolbox_block *id_block, void *handle)
{
    printprwin_win *ptr = find(id_block);
    bool claimed = FALSE;

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Set the scale
        scale_set_value(0, id_block->this_obj, ptr->scale);

        // Claim the event
        claimed = TRUE;
    }

    // Claim the event if processed
    return claimed;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle the scale being changed.
*/
bool printprwin_win::set_scale(bits event_code, toolbox_action *action,
                               toolbox_block *id_block, void *handle)
{
    printprwin_win *ptr = find(id_block);
    scale_action_apply_factor *scale = (scale_action_apply_factor *) &action->data;
    bool claimed = FALSE;

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Check the allowed scale range
        int lower;
        int upper;
        scale_get_bounds(scale_SET_LOWER_BOUND | scale_SET_UPPER_BOUND,
                         id_block->this_obj, &lower, &upper, NULL);
        if (scale->percent < lower) ptr->scale = lower;
        else if (upper < scale->percent) ptr->scale = upper;
        else ptr->scale = scale->percent;
        if (ptr->scale != scale->percent)
        {
            scale_set_value(0, id_block->this_obj, ptr->scale);
        }

        // Redraw the window
        ptr->refresh();

        // Claim the event
        claimed = TRUE;
    }

    // Claim the event if processed
    return claimed;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle save window being opened.
*/
bool printprwin_win::pre_save(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    printprwin_win *ptr = find(id_block);
    bool claimed = FALSE;

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Set the file name
        char str[10];
        sprintf(str, "%i", ptr->page);

        // Set the file name and type
        switch (id_block->parent_cmp)
        {
            case PRINTPRWIN_MENU_ALL_SAVE_TEXT:
                saveas_set_file_name(0, id_block->this_obj,
                                     filer_msgtrans("PrnSaAT").c_str());
                saveas_set_file_type(0, id_block->this_obj, osfile_TYPE_TEXT);
                break;

            case PRINTPRWIN_MENU_PAGE_SAVE_DRAW:
                saveas_set_file_name(0, id_block->this_obj,
                                     filer_msgtrans("PrnSaPD", str).c_str());
                saveas_set_file_type(0, id_block->this_obj, osfile_TYPE_DRAW);
                break;

            case PRINTPRWIN_MENU_PAGE_SAVE_TEXT:
                saveas_set_file_name(0, id_block->this_obj,
                                     filer_msgtrans("PrnSaPT", str).c_str());
                saveas_set_file_type(0, id_block->this_obj, osfile_TYPE_TEXT);
                break;

            case PRINTPRWIN_MENU_PAGE_SAVE_DATA:
                saveas_set_file_name(0, id_block->this_obj,
                                     filer_msgtrans("PrnSaPR", str).c_str());
                saveas_set_file_type(0, id_block->this_obj, osfile_TYPE_DATA);
                break;

            default:
                // There shouldn't be any other options
                break;
        }

        // Pretend that the file will be small
        saveas_set_file_size(0, id_block->this_obj, 0);

        // Claim the event
        claimed = TRUE;
    }

    // Claim the event if processed
    return claimed;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle save window saves.
*/
bool printprwin_win::save_raw(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    printprwin_win *ptr = find(id_block);
    saveas_action_save_to_file *save = (saveas_action_save_to_file *) &action->data;
    bool claimed = FALSE;

    NOT_USED(event_code);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        bool success = FALSE;

        // Action depends on the requested filetype
        switch (id_block->parent_cmp)
        {
            case PRINTPRWIN_MENU_ALL_SAVE_TEXT:
                success = ptr->save_text(save->file_name, TRUE);
                break;

            case PRINTPRWIN_MENU_PAGE_SAVE_DRAW:
                success = ptr->save_draw(save->file_name);
                break;

            case PRINTPRWIN_MENU_PAGE_SAVE_TEXT:
                success = ptr->save_text(save->file_name);
                break;

            case PRINTPRWIN_MENU_PAGE_SAVE_DATA:
                success = ptr->save_raw(save->file_name);
                break;

            default:
                // There shouldn't be any other options
                break;
        }

        // Enable simple error handling
        filer_error_allowed++;

        // Indicate that the save has completed
        saveas_file_save_completed(success, id_block->this_obj,
                                   save->file_name);

        // Restore normal error handling
        filer_error_allowed--;

        // Claim the event
        claimed = TRUE;
    }

    // Claim the event if processed
    return claimed;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle menu selections.
*/
bool printprwin_win::menu_raw(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    printprwin_win *ptr = find(id_block);
    bool claimed = FALSE;

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // No action if object not found
    if (ptr)
    {
        // Read the current pointer position
        wimp_pointer pointer;
        wimp_get_pointer_info(&pointer);

        // Action depends on the option selected
        switch (id_block->this_cmp)
        {
            case PRINTPRWIN_MENU_PAGE_PRINT:
                // Print the current page
                {
                    printjbobj_obj page(ptr->job[ptr->page]);
                    new printjbwin_win(page, &pointer.pos);
                }
                break;

            case PRINTPRWIN_MENU_ALL_PRINT:
                // Print the whole job
                new printjbwin_win(ptr->job, &pointer.pos);
                break;

            default:
                // Not interested in any other menu options
                break;
        }

        // Claim the event
        claimed = TRUE;
    }

    // Claim the event if processed
    return claimed;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle window redraw requests.
*/
bool printprwin_win::redraw_raw(wimp_event_no event_code, wimp_block *block,
                                toolbox_block *id_block, void *handle)
{
    printprwin_win *ptr = find(id_block);

    NOT_USED(event_code);
    NOT_USED(handle);

    // Redraw the window
    bool more = wimp_redraw_window(&block->redraw);
    while (more)
    {
        if (ptr) ptr->redraw(block->redraw);
        more = wimp_get_rectangle(&block->redraw);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : id_block          - A toolbox ID block.
    Returns     : printprwin_win    - Pointer to the corresponding print
                                      job preview window, or NULL if not
                                      found.
    Description : Convert a toolbox ID block into a print job preview
                  window pointer.
*/
printprwin_win *printprwin_win::find(const toolbox_block *id_block)
{
    // Choose the ancestor object ID
    toolbox_o id = id_block->ancestor_obj == toolbox_NULL_OBJECT
                   ? id_block->this_obj
                   : id_block->ancestor_obj;

    // Attempt to read the client handle
    printprwin_win *ptr;
    if (xtoolbox_get_client_handle(0, id, (void **) &ptr)) ptr = NULL;

    // Return the pointer
    return ptr;
}

/*
    Parameters  : printer   - Has the selected printer changed.
    Returns     : void
    Description : Update any open print job preview windows.
*/
void printprwin_win::update_all(bool printer)
{
    list_iterator<printprwin_win *> i;

    // Update all of the print job preview windows
    for (i = printprwin_list.begin(); i != printprwin_list.end(); i++)
    {
        (*i)->update(printer);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Cancel any open print job preview windows.
*/
void printprwin_win::cancel_all()
{
    // Delete all windows
    while (!printprwin_list.empty()) delete printprwin_list.front();
}

/*
    Parameters  : void
    Returns     : bits      - The number of open print job preview windows.
    Description : Check how many print job preview windows are open.
*/
bits printprwin_win::active_count()
{
    return printprwin_list.size();
}
