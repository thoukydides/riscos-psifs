/*
    File        : config.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Configuration event handling for the PsiFS filer.

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
#ifndef CONFIG_H
#define CONFIG_H

// Include project header files
#include "tag.h"

// Configuration file tags
extern const char config_tag_driver[];
extern const char config_tag_port[];
extern const char config_tag_baud[];
extern const char config_tag_options[];
extern const char config_tag_auto_baud[];
extern const char config_tag_restrict_baud[];
extern const char config_tag_open_link[];
extern const char config_tag_close_kill[];
extern const char config_tag_close_link[];
extern const char config_tag_show_rom[];
extern const char config_tag_show_all[];
extern const char config_tag_print[];
extern const char config_tag_backup_path[];
extern const char config_tag_backup_new[];
extern const char config_tag_sync_clocks[];
extern const char config_tag_power_monitor[];
extern const char config_tag_power_external[];
extern const char config_tag_power_custom[];
extern const char config_tag_power_main_dead[];
extern const char config_tag_power_main_very_low[];
extern const char config_tag_power_main_low[];
extern const char config_tag_power_backup_dead[];
extern const char config_tag_power_backup_very_low[];
extern const char config_tag_power_backup_low[];
extern const char config_tag_intercept_run[];
extern const char config_tag_intercept_run_all[];
extern const char config_tag_intercept_run_auto[];
extern const char config_tag_intercept_load[];
extern const char config_tag_intercept_load_auto[];
extern const char config_tag_intercept_save[];
extern const char config_tag_intercept_transfer[];
extern const char config_tag_intercept_save_auto[];
extern const char config_tag_intercept_directory[];
extern const char config_tag_clipboard_integrate[];
extern const char config_tag_clipboard_poll_disabled[];
extern const char config_tag_clipboard_poll_content[];
extern const char config_tag_idle_link[];
extern const char config_tag_idle_printer[];
extern const char config_tag_idle_background[];
extern const char config_tag_print_auto_print[];
extern const char config_tag_print_auto_preview[];
extern const char config_tag_print_preview_scale[];
extern const char config_tag_print_preview_antialias[];
extern const char config_tag_print_print_wait[];
extern const char config_tag_print_log_debug[];

// A configuration
class config_store : public tag_store
{
public:

    /*
        Parameters  : void
        Returns     : bool      - Was the load successful.
        Description : Attempt to read the configuration file.
    */
    bool load();

    /*
        Parameters  : void
        Returns     : void
        Description : Attempt to write the configuration file.
    */
    void save() const;

    /*
        Parameters  : void
        Returns     : void
        Description : Attempt to read the active configuration.
    */
    void read();

    /*
        Parameters  : report    - Should any errors be reported.
        Returns     : void
        Description : Attempt to write the active configuration.
    */
    void write(bool report) const;
};

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the configuration.
*/
void config_init();

// The current configuration
extern config_store config_current;

#endif
