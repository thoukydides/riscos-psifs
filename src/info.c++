/*
    File        : info.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Information window handling for the PsiFS filer.

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
#include "info.h"

// Include clib header files
#include <ctype.h>
#include <stdio.h>

// Include cathlibcpp header files
#include "map.h"

// Include oslib header files
#include "oslib/menu.h"

// Include alexlib header files
#include "button_c.h"
#include "displayfield_c.h"
#include "slider_c.h"

// Include project header files
#include "config.h"
#include "filer.h"
#include "psifs.h"

// Menu gadgets
#define INFO_MENU_MACHINE ((toolbox_c) 0x10)
#define INFO_MENU_OWNER ((toolbox_c) 0x11)
#define INFO_MENU_POWER ((toolbox_c) 0x12)
#define INFO_MENU_LINK ((toolbox_c) 0x14)

// Machine details gadgets
#define INFO_MACHINE_TYPE ((toolbox_c) 0x10)
#define INFO_MACHINE_SOFTWARE ((toolbox_c) 0x11)
#define INFO_MACHINE_LANGUAGE ((toolbox_c) 0x12)
#define INFO_MACHINE_ID ((toolbox_c) 0x13)

// Owner information gadgets
#define INFO_OWNER_BEGIN ((toolbox_c) 0x10)
#define INFO_OWNER_END ((toolbox_c) 0x18)

// Power status window gadgets
#define INFO_POWER_MAIN_STATUS ((toolbox_c) 0x10)
#define INFO_POWER_MAIN_VOLT ((toolbox_c) 0x11)
#define INFO_POWER_MAIN_GRAPH ((toolbox_c) 0x12)
#define INFO_POWER_MAIN_MIN ((toolbox_c) 0x13)
#define INFO_POWER_MAIN_MAX ((toolbox_c) 0x14)
#define INFO_POWER_BACKUP_STATUS ((toolbox_c) 0x20)
#define INFO_POWER_BACKUP_VOLT ((toolbox_c) 0x21)
#define INFO_POWER_BACKUP_GRAPH ((toolbox_c) 0x22)
#define INFO_POWER_BACKUP_MIN ((toolbox_c) 0x23)
#define INFO_POWER_BACKUP_MAX ((toolbox_c) 0x24)
#define INFO_POWER_EXT ((toolbox_c) 0x30)

// Power warning window gadgets
#define INFO_POWER_WARN ((toolbox_c) 0x10)

// Link statistics window gadgets
#define INFO_LINK_RATE_TX_GRAPH ((toolbox_c) 0x10)
#define INFO_LINK_RATE_TX_VALUE ((toolbox_c) 0x11)
#define INFO_LINK_RATE_RX_GRAPH ((toolbox_c) 0x12)
#define INFO_LINK_RATE_RX_VALUE ((toolbox_c) 0x13)
#define INFO_LINK_FRAME_TX_VALID ((toolbox_c) 0x20)
#define INFO_LINK_FRAME_TX_INVALID ((toolbox_c) 0x21)
#define INFO_LINK_FRAME_TX_RETRY ((toolbox_c) 0x22)
#define INFO_LINK_FRAME_RX_VALID ((toolbox_c) 0x23)
#define INFO_LINK_FRAME_RX_INVALID ((toolbox_c) 0x24)
#define INFO_LINK_FRAME_RX_RETRY ((toolbox_c) 0x25)

// Maximum length of owner information line
#define INFO_OWNER_MAX (100)

// Minimum interval between updates to the transfer rate display
#define INFO_LINK_RATE_CENTISECONDS (25)

// Number of bits per byte for baud rate to cps conversion
#define INFO_LINK_BITS_PER_BYTE (10)

// Mappings from enumerated types to message tokens
static map<psifs_language, string, less<psifs_language> > info_map_language;
static map<psifs_battery_status, string, less<psifs_battery_status> > info_map_battery;

// Menu object
static toolbox_o info_menu_obj;

// Window objects
static toolbox_o info_machine_obj;
static toolbox_o info_owner_obj;
static toolbox_o info_power_obj;
static toolbox_o info_power_warn_obj;
static toolbox_o info_link_obj;

/*
    Parameters  : millivolts    - The voltage.
                  dp            - The number of decimal places.
    Returns     : char *        - A string representing the voltage.
    Description : Convert a voltage into an equivalent string.
*/
static string info_millivolts(bits millivolts, bits dp = 3)
{
    static char str[10];

    // Convert the voltage
    sprintf(str, "%.*f", dp, millivolts / 1000.0);

    // Return the result
    return filer_msgtrans("PowVolt", str);
}

/*
    Parameters  : cps           - The number of characters per second.
                  dp            - The number of decimal places.
    Returns     : char *        - A string representing the rate.
    Description : Convert a serial transfer rate into an equivalent string.
*/
static string info_cps(bits cps, bits dp = 2)
{
    static char str[10];

    // Convert the rate
    sprintf(str, "%.*f", dp, cps / 1000.0);

    // Return the result
    return filer_msgtrans("LnkKBps", str);
}

/*
    Parameters  : frames        - The number of frames.
    Returns     : char *        - A string representing the frame count.
    Description : Convert a frame count into an equivalent string.
*/
static string info_frames(bits frames)
{
    static char str[10];

    // Convert the count
    if (frames < 1000) sprintf(str, "%u", frames);
    else sprintf(str, "%u,%03u", frames / 1000, frames % 1000);

    // Return the result
    return string(str);
}

/*
    Parameters  : language  - The language.
    Returns     : string    - A string representing the language.
    Description : Convert a language into a string.
*/
static string info_language(psifs_language language)
{
    string str;

    // Attempt to find a matching string
    map_iterator<psifs_language, string, less<psifs_language> > i(info_map_language.find(language));
    if (i != info_map_language.end()) str = filer_msgtrans((*i).second.c_str());
    else
    {
        // Generate a default string
        char code[10];
        sprintf(code, "%i", language);
        str = filer_msgtrans("MchLnUn", code);
    }

    // Return the result
    return str;
}

/*
    Parameters  : status    - The battery status.
                  upper     - Should an initial capital letter be used.
    Returns     : string    - A string representing the battery status.
    Description : Convert a battery status into a string.
*/
static string info_battery_status(psifs_battery_status status,
                                  bool upper = TRUE)
{
    string str;

    // Attempt to find a matching string
    map_iterator<psifs_battery_status, string, less<psifs_battery_status> > i(info_map_battery.find(status));
    if (i != info_map_battery.end()) str = filer_msgtrans((*i).second.c_str());
    else
    {
        // Generate a default string
        char code[10];
        sprintf(code, "%i", status);
        str = filer_msgtrans("PowStUn", code);
    }

    // Convert the first character to upper case if required
    if (upper) str[0] = toupper(str[0]);

    // Return the result
    return str;
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the owner information window.
*/
static void info_update_owner(void)
{
    static bool valid = FALSE;
    string owner;

    // Read the owner information
    int size = 1 - psifsget_machine_owner(NULL, 0);
    char *str = new char[size];
    if (str)
    {
        if (0 < psifsget_machine_owner(str, size)) owner = str;
        delete[] str;
    }

    // Update the window appropriately
    if (!owner.empty())
    {
        static string prev_owner;

        // Check for changes
        if (!valid || (prev_owner != owner))
        {
            // Store the new value
            prev_owner = owner;

            // Replace any unsuitable control characters
            for (char *i = owner.begin(); i != owner.end(); i++)
            {
                if ((*i == '\6') || (*i == '\n') || (*i == '\r')) *i = '\n';
                else if (*i == '\t') *i = '';
                else if (iscntrl(*i)) *i = ' ';
            }

            // Set the owner information window contents
            for (toolbox_c cmp = INFO_OWNER_BEGIN; cmp != INFO_OWNER_END; cmp++)
            {
                string::size_type newline = owner.find(char('\n'));
                button_c(cmp, info_owner_obj) = owner.substr(0, newline).substr(0, INFO_OWNER_MAX);
                owner.erase(0, newline == string::npos ? newline : newline + 1);
            }
        }

        // Enable the menu entry
        if (!valid)
        {
            // Enable the menu entry
            menu_set_fade(0, info_menu_obj, INFO_MENU_OWNER, FALSE);

            // Set the valid flag
            valid = TRUE;
        }
    }
    else
    {
        // Ensure that the window is closed
        if (valid)
        {
            // Close the window
            toolbox_hide_object(0, info_owner_obj);

            // Disable the menu entry
            menu_set_fade(0, info_menu_obj, INFO_MENU_OWNER, TRUE);

            // Clear the valid flag
            valid = FALSE;
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the machine details window.
*/
static void info_update_machine(void)
{
    static bool valid = FALSE;
    string type;

    // Read the machine type
    int size = 1 - psifsget_machine_description(NULL, 0);
    char *str = new char[size];
    if (str)
    {
        if (0 < psifsget_machine_description(str, size)) type = str;
        delete[] str;
    }

    // Update the window appropriately
    if (!type.empty())
    {
        // Update the machine description
        static string prev_type;
        if (!valid || (prev_type != type))
        {
            prev_type = type;
            displayfield_c(INFO_MACHINE_TYPE, info_machine_obj) = type;
        }

        // Set the operating system version field
        string software = filer_msgtrans(psifsget_link_status() & psifs_LINK_STATUS_EPOC32 ? "MchOS32" : "MchOS16");
        bits major;
        bits minor;
        bits build;
        if ((psifsget_link_status() & psifs_LINK_STATUS_EPOC32)
            && !xpsifsget_machine_os_major(&major)
            && !xpsifsget_machine_os_minor(&minor)
            && !xpsifsget_machine_os_build(&build))
        {
            char str[40];
            sprintf(str, " %i.%02i (%i)", major, minor, build);
            software += str;
        }
        displayfield_c(INFO_MACHINE_SOFTWARE, info_machine_obj) = software;

        // Update the language field
        psifs_language language;
        if (!xpsifsget_machine_language(&language))
        {
            static psifs_language prev_language = psifs_LANGUAGE_TEST;
            if (prev_language != language)
            {
                prev_language = language;
                displayfield_c(INFO_MACHINE_LANGUAGE, info_machine_obj) = info_language(language);
            }
        }
        else displayfield_c(INFO_MACHINE_LANGUAGE, info_machine_obj) = filer_msgtrans("MchLnUv");

        // Update the unique identifier field
        bits low_id;
        bits high_id;
        if (!xpsifsget_machine_id_low(&low_id)
            && !xpsifsget_machine_id_high(&high_id)
            && (low_id || high_id))
        {
            char str[20];
            sprintf(str, "%04X-%04X-%04X-%04X",
                    high_id / 0x10000, high_id % 0x10000,
                    low_id / 0x10000, low_id % 0x10000);
            displayfield_c(INFO_MACHINE_ID, info_machine_obj) = str;
        }
        else displayfield_c(INFO_MACHINE_ID, info_machine_obj) = filer_msgtrans("MchIdUv");

        // Enable the menu entry
        if (!valid)
        {
            // Enable the menu entry
            menu_set_fade(0, info_menu_obj, INFO_MENU_MACHINE, FALSE);

            // Set the valid flag
            valid = TRUE;
        }
    }
    else
    {
        // Ensure that the window is closed
        if (valid)
        {
            // Close the window
            toolbox_hide_object(0, info_machine_obj);

            // Disable the menu entry
            menu_set_fade(0, info_menu_obj, INFO_MENU_MACHINE, TRUE);

            // Clear the valid flag
            valid = FALSE;
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the power status window.
*/
static void info_update_power(void)
{
    static bool valid = FALSE;
    static psifs_battery_status warned = psifs_BATTERY_STATUS_GOOD;
    psifs_battery_status main_status;
    bits main_millivolts;
    bits main_max_millivolts;
    psifs_battery_status backup_status;
    bits backup_millivolts;
    bits backup_max_millivolts;
    bool external;

    // Attempt to read the battery status
    if (!xpsifsget_power_main_status(&main_status)
        && !xpsifsget_power_main_voltage(&main_millivolts)
        && !xpsifsget_power_main_max_voltage(&main_max_millivolts)
        && !xpsifsget_power_backup_status(&backup_status)
        && !xpsifsget_power_backup_voltage(&backup_millivolts)
        && !xpsifsget_power_backup_max_voltage(&backup_max_millivolts)
        && !xpsifsget_power_external(&external))
    {
        static wimp_colour colour[] = {wimp_COLOUR_BLACK, wimp_COLOUR_RED, wimp_COLOUR_ORANGE, wimp_COLOUR_LIGHT_GREEN};
        static psifs_battery_status prev_main_status;
        static bits prev_main_millivolts;
        static bits prev_main_max_millivolts;
        static psifs_battery_status prev_backup_status;
        static bits prev_backup_millivolts;
        static bits prev_backup_max_millivolts;
        static bool prev_external;

        // Use custom thresholds if appropriate
        if (config_current.get_bool(config_tag_power_custom))
        {
            // Check the main battery status
            main_status = main_millivolts < config_current.get_num(config_tag_power_main_dead)
                          ? psifs_BATTERY_STATUS_DEAD
                          : main_millivolts < config_current.get_num(config_tag_power_main_very_low)
                            ? psifs_BATTERY_STATUS_VERY_LOW
                            : main_millivolts < config_current.get_num(config_tag_power_main_low)
                              ? psifs_BATTERY_STATUS_LOW
                              : psifs_BATTERY_STATUS_GOOD;

            // Check the backup battery status
            backup_status = backup_millivolts < config_current.get_num(config_tag_power_backup_dead)
                            ? psifs_BATTERY_STATUS_DEAD
                            : backup_millivolts < config_current.get_num(config_tag_power_backup_very_low)
                              ? psifs_BATTERY_STATUS_VERY_LOW
                              : backup_millivolts < config_current.get_num(config_tag_power_backup_low)
                                ? psifs_BATTERY_STATUS_LOW
                                : psifs_BATTERY_STATUS_GOOD;

        }

        // Check for changes
        if (!valid
            || (prev_main_status != main_status)
            || (prev_main_millivolts != main_millivolts)
            || (prev_main_max_millivolts != main_max_millivolts)
            || (prev_backup_status != backup_status)
            || (prev_backup_millivolts != backup_millivolts)
            || (prev_backup_max_millivolts != backup_max_millivolts)
            || (prev_external != external))
        {
            // Store the new values
            prev_main_status = main_status;
            prev_main_millivolts = main_millivolts;
            prev_main_max_millivolts = main_max_millivolts;
            prev_backup_status = backup_status;
            prev_backup_millivolts = backup_millivolts;
            prev_backup_max_millivolts = backup_max_millivolts;
            prev_external = external;

            // Open the warning window if appropriate
            if (((main_status < warned) || (backup_status < warned))
                && config_current.get_bool(config_tag_power_monitor)
                && (!external
                    || config_current.get_bool(config_tag_power_external)))
            {
                // Choose the battery to warn for
                if (main_status < backup_status)
                {
                    // Warn about the main batteries
                    button_c(INFO_POWER_WARN, info_power_warn_obj) = filer_msgtrans("PowWnMn", info_battery_status(main_status, FALSE).c_str());
                    warned = main_status;
                }
                else
                {
                    // Warn about the backup batteries
                    button_c(INFO_POWER_WARN, info_power_warn_obj) = filer_msgtrans("PowWnBk", info_battery_status(backup_status, FALSE).c_str());
                    warned = backup_status;
                }

                // Open the warning window
                toolbox_show_object(0, info_power_warn_obj,
                                    toolbox_POSITION_AT_POINTER, NULL,
                                    toolbox_NULL_OBJECT,
                                    toolbox_NULL_COMPONENT);
            }

            // Round the maximum voltages up to the nearest 0.1V
            main_max_millivolts = ((main_max_millivolts + 99) / 100) * 100;
            backup_max_millivolts = ((backup_max_millivolts + 99) / 100) * 100;

            // Set the status window contents
            displayfield_c(INFO_POWER_EXT, info_power_obj) = filer_msgtrans(external ? "PowExtP" : "PowExtN");
            displayfield_c(INFO_POWER_MAIN_STATUS, info_power_obj) = info_battery_status(main_status);
            displayfield_c(INFO_POWER_MAIN_VOLT, info_power_obj) = info_millivolts(main_millivolts);
            button_c(INFO_POWER_MAIN_MIN, info_power_obj) = info_millivolts(0, 1);
            button_c(INFO_POWER_MAIN_MAX, info_power_obj) = info_millivolts(main_max_millivolts, 1);
            slider_c(INFO_POWER_MAIN_GRAPH, info_power_obj).set_upper_bound(main_max_millivolts);
            slider_c(INFO_POWER_MAIN_GRAPH, info_power_obj).set_slider_colour(colour[main_status]);
            slider_c(INFO_POWER_MAIN_GRAPH, info_power_obj) = main_millivolts;
            displayfield_c(INFO_POWER_BACKUP_STATUS, info_power_obj) = info_battery_status(backup_status);
            displayfield_c(INFO_POWER_BACKUP_VOLT, info_power_obj) = info_millivolts(backup_millivolts);
            button_c(INFO_POWER_BACKUP_MIN, info_power_obj) = info_millivolts(0, 1);
            button_c(INFO_POWER_BACKUP_MAX, info_power_obj) = info_millivolts(backup_max_millivolts, 1);
            slider_c(INFO_POWER_BACKUP_GRAPH, info_power_obj).set_upper_bound(backup_max_millivolts);
            slider_c(INFO_POWER_BACKUP_GRAPH, info_power_obj).set_slider_colour(colour[backup_status]);
            slider_c(INFO_POWER_BACKUP_GRAPH, info_power_obj) = backup_millivolts;
        }

        // Enable the menu entry
        if (!valid)
        {
            // Enable the menu entry
            menu_set_fade(0, info_menu_obj, INFO_MENU_POWER, FALSE);

            // Set the valid flag
            valid = TRUE;
        }
    }
    else
    {
        // Ensure that the windows are closed
        if (valid)
        {
            // Close the windows
            toolbox_hide_object(0, info_power_obj);
            toolbox_hide_object(0, info_power_warn_obj);

            // Disable the menu entry
            menu_set_fade(0, info_menu_obj, INFO_MENU_POWER, TRUE);

            // Clear the valid flag and reset the warned level
            valid = FALSE;
            warned = psifs_BATTERY_STATUS_GOOD;
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the link statistics window.
*/
static void info_update_link(void)
{
    static bool valid = FALSE;
    bits tx_bytes;
    bits rx_bytes;
    bits tx_frames;
    bits tx_frames_retry;
    bits rx_frames;
    bits rx_frames_err;
    bits rx_frames_retry;
    bits baud_rate;

    // Attempt to read the link statistics
    if (!xpsifsget_statistics_transmitted_bytes(&tx_bytes)
        && !xpsifsget_statistics_received_bytes(&rx_bytes)
        && !xpsifsget_statistics_transmitted_valid_frames(&tx_frames)
        && !xpsifsget_statistics_transmitted_retried_frames(&tx_frames_retry)
        && !xpsifsget_statistics_received_valid_frames(&rx_frames)
        && !xpsifsget_statistics_received_invalid_frames(&rx_frames_err)
        && !xpsifsget_statistics_received_retried_frames(&rx_frames_retry)
        && !xpsifsget_driver_active_baud(&baud_rate)
        && (0 < baud_rate))
    {
        static os_t prev_now;
        static bits prev_tx_bytes;
        static bits prev_rx_bytes;
        static bits prev_tx_frames;
        static bits prev_tx_frames_retry;
        static bits prev_rx_frames;
        static bits prev_rx_frames_err;
        static bits prev_rx_frames_retry;
        static bits prev_baud_rate;

        // Update the transfer rates
        os_t now = os_read_monotonic_time();
        if (!valid
            || (INFO_LINK_RATE_CENTISECONDS <= (now - prev_now))
            || (prev_baud_rate != baud_rate))
        {
            // Calculate the transfer rates if previous values are available
            bits max_cps = baud_rate / INFO_LINK_BITS_PER_BYTE;
            bits tx_cps = 0;
            bits rx_cps = 0;
            if (valid
                && (prev_tx_bytes <= tx_bytes) && (prev_rx_bytes <= rx_bytes))
            {
                bits interval = now - prev_now;
                tx_cps = ((tx_bytes - prev_tx_bytes) * 100) / interval;
                rx_cps = ((rx_bytes - prev_rx_bytes) * 100) / interval;
            }

            // Store the new values
            prev_now = now;
            prev_tx_bytes = tx_bytes;
            prev_rx_bytes = rx_bytes;
            prev_baud_rate = baud_rate;

            // Set the statistics window contents
            slider_c(INFO_LINK_RATE_TX_GRAPH, info_link_obj).set_upper_bound(max_cps);
            slider_c(INFO_LINK_RATE_TX_GRAPH, info_link_obj) = tx_cps;
            displayfield_c(INFO_LINK_RATE_TX_VALUE, info_link_obj) = info_cps(tx_cps);
            slider_c(INFO_LINK_RATE_RX_GRAPH, info_link_obj).set_upper_bound(max_cps);
            slider_c(INFO_LINK_RATE_RX_GRAPH, info_link_obj) = rx_cps;
            displayfield_c(INFO_LINK_RATE_RX_VALUE, info_link_obj) = info_cps(rx_cps);
        }

        // Check for changes to the frame counts
        if (!valid
            || (prev_tx_frames != tx_frames)
            || (prev_tx_frames_retry != tx_frames_retry)
            || (prev_rx_frames != rx_frames)
            || (prev_rx_frames_err != rx_frames_err)
            || (prev_rx_frames_retry != rx_frames_retry))
        {
            // Store the new values
            prev_tx_frames = tx_frames;
            prev_tx_frames_retry = tx_frames_retry;
            prev_rx_frames = rx_frames;
            prev_rx_frames_err = rx_frames_err;
            prev_rx_frames_retry = rx_frames_retry;

            // Set the statistics window contents
            displayfield_c(INFO_LINK_FRAME_TX_VALID, info_link_obj) = info_frames(tx_frames);
            displayfield_c(INFO_LINK_FRAME_TX_RETRY, info_link_obj) = info_frames(tx_frames_retry);
            displayfield_c(INFO_LINK_FRAME_RX_VALID, info_link_obj) = info_frames(rx_frames);
            displayfield_c(INFO_LINK_FRAME_RX_INVALID, info_link_obj) = info_frames(rx_frames_err);
            displayfield_c(INFO_LINK_FRAME_RX_RETRY, info_link_obj) = info_frames(rx_frames_retry);
        }

        // Enable the menu entry
        if (!valid)
        {
            // Enable the menu entry
            menu_set_fade(0, info_menu_obj, INFO_MENU_LINK, FALSE);

            // Set the valid flag
            valid = TRUE;
        }
    }
    else
    {
        // Ensure that the window is closed
        if (valid)
        {
            // Close the window
            toolbox_hide_object(0, info_link_obj);

            // Disable the menu entry
            menu_set_fade(0, info_menu_obj, INFO_MENU_LINK, TRUE);

            // Clear the valid flag
            valid = FALSE;
        }
    }
}

/*
    Parameters  : timed - Was the check triggered by a timer tick rather than
                          a poll word change.
    Returns     : void
    Description : Update the status of the information windows.
*/
void info_update(bool timed)
{
    static bool init = FALSE;

    // Initialise on first call
    if (!init)
    {
        // Create the menu object
        info_menu_obj = toolbox_create_object(0, (toolbox_id) "MenuInfo");

        // Create the window objects
        info_machine_obj = toolbox_create_object(0, (toolbox_id) "WinMachine");
        info_owner_obj = toolbox_create_object(0, (toolbox_id) "WinOwner");
        info_power_obj = toolbox_create_object(0, (toolbox_id) "WinPower");
        info_power_warn_obj = toolbox_create_object(0, (toolbox_id) "WinBatWarn");
        info_link_obj = toolbox_create_object(0, (toolbox_id) "WinStats");

        // Initialise the language map
        info_map_language[psifs_LANGUAGE_UNKNOWN] = "MchLnUv";
        info_map_language[psifs_LANGUAGE_TEST] = "MchLnTs";
        info_map_language[psifs_LANGUAGE_UK_ENGLISH] = "MchLnEN";
        info_map_language[psifs_LANGUAGE_FRENCH] = "MchLnFR";
        info_map_language[psifs_LANGUAGE_GERMAN] = "MchLnGE";
        info_map_language[psifs_LANGUAGE_SPANISH] = "MchLnSP";
        info_map_language[psifs_LANGUAGE_ITALIAN] = "MchLnIT";
        info_map_language[psifs_LANGUAGE_SWEDISH] = "MchLnSW";
        info_map_language[psifs_LANGUAGE_DANISH] = "MchLnDA";
        info_map_language[psifs_LANGUAGE_NORWEGIAN] = "MchLnNO";
        info_map_language[psifs_LANGUAGE_FINNISH] = "MchLnFI";
        info_map_language[psifs_LANGUAGE_AMERICAN_ENGLISH] = "MchLnAM";
        info_map_language[psifs_LANGUAGE_SWISS_FRENCH] = "MchLnSF";
        info_map_language[psifs_LANGUAGE_SWISS_GERMAN] = "MchLnSG";
        info_map_language[psifs_LANGUAGE_PORTUGUESE] = "MchLnPO";
        info_map_language[psifs_LANGUAGE_TURKISH] = "MchLnTU";
        info_map_language[psifs_LANGUAGE_ICELANDIC] = "MchLnIC";
        info_map_language[psifs_LANGUAGE_RUSSIAN] = "MchLnRU";
        info_map_language[psifs_LANGUAGE_HUNGARIAN] = "MchLnHU";
        info_map_language[psifs_LANGUAGE_DUTCH] = "MchLnDU";
        info_map_language[psifs_LANGUAGE_BELGIAN_FLEMISH] = "MchLnBL";
        info_map_language[psifs_LANGUAGE_AUSTRALIAN_ENGLISH] = "MchLnAU";
        info_map_language[psifs_LANGUAGE_BELGIAN_FRENCH] = "MchLnBG";
        info_map_language[psifs_LANGUAGE_AUSTRIAN_GERMAN] = "MchLnAS";
        info_map_language[psifs_LANGUAGE_NEW_ZEALAND_ENGLISH] = "MchLnNZ";
        info_map_language[psifs_LANGUAGE_INTERNATIONAL_FRENCH] = "MchLnIF";
        info_map_language[psifs_LANGUAGE_CZECH] = "MchLnCS";
        info_map_language[psifs_LANGUAGE_SLOVAK] = "MchLnSK";
        info_map_language[psifs_LANGUAGE_POLISH] = "MchLnPL";
        info_map_language[psifs_LANGUAGE_SLOVENIAN] = "MchLnSL";
        info_map_language[psifs_LANGUAGE_TAIWAN_CHINESE] = "MchLnTC";
        info_map_language[psifs_LANGUAGE_HONG_KONG_CHINESE] = "MchLnHK";
        info_map_language[psifs_LANGUAGE_PRC_CHINESE] = "MchLnZH";
        info_map_language[psifs_LANGUAGE_JAPANESE] = "MchLnJA";
        info_map_language[psifs_LANGUAGE_THAI] = "MchLnTH";

        // Initialise the battery status maps
        info_map_battery[psifs_BATTERY_STATUS_DEAD] = "PowStDd";
        info_map_battery[psifs_BATTERY_STATUS_VERY_LOW] = "PowStVl";
        info_map_battery[psifs_BATTERY_STATUS_LOW] = "PowStLw";
        info_map_battery[psifs_BATTERY_STATUS_GOOD] = "PowStGd";

        // Set the initialised flag
        init = TRUE;
    }

    // Update the status of all the windows
    info_update_machine();
    info_update_owner();
    info_update_power();
    if (timed) info_update_link();
}
