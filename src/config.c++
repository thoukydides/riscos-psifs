/*
    File        : config.c++
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

// Prevent OSLib/CathLibCPP name clashes
#include "noclash.h"

// Include header file for this module
#include "config.h"

// Include system header files
#include <stdlib.h>

// Include cathlibcpp header files
#include "fstream.h"

// Include oslib header files
#include "oslib/osfile.h"

// Include project header files
#include "convobj.h"
#include "filer.h"
#include "tag.h"
#include "psifs.h"

// Configuration file tags
const char config_tag_driver[] = "BlockdriverDriver";
const char config_tag_port[] = "BlockdriverPort";
const char config_tag_baud[] = "BlockdriverBaud";
const char config_tag_options[] = "BlockdriverOptions";
const char config_tag_auto_baud[] = "BlockdriverAutoBaud";
const char config_tag_restrict_baud[] = "BlockdriverRestrictBaud";
const char config_tag_open_link[] = "FilerOpenLink";
const char config_tag_close_kill[] = "FilerCloseKill";
const char config_tag_close_link[] = "FilerCloseLink";
const char config_tag_show_rom[] = "FilerShowROMDrives";
const char config_tag_show_all[] = "FilerShowVirtualDrive";
const char config_tag_print[] = "PrinterMirrorDefault";
const char config_tag_backup_path[] = "BackupDirectory";
const char config_tag_backup_new[] = "BackupOpenUnrecognised";
const char config_tag_sync_clocks[] = "LinkSynchronizeClocks";
const char config_tag_power_monitor[] = "LinkMonitorBatteries";
const char config_tag_power_external[] = "LinkMonitorBatteriesExternal";
const char config_tag_power_custom[] = "LinkMonitorBatteriesCustom";
const char config_tag_power_main_dead[] = "LinkMonitorBatteriesMainDead";
const char config_tag_power_main_very_low[] = "LinkMonitorBatteriesMainVeryLow";
const char config_tag_power_main_low[] = "LinkMonitorBatteriesMainLow";
const char config_tag_power_backup_dead[] = "LinkMonitorBatteriesBackupDead";
const char config_tag_power_backup_very_low[] = "LinkMonitorBatteriesBackupVeryLow";
const char config_tag_power_backup_low[] = "LinkMonitorBatteriesBackupLow";
const char config_tag_intercept_run[] = "InterceptFileRun";
const char config_tag_intercept_run_all[] = "InterceptFileRunAll";
const char config_tag_intercept_run_auto[] = "InterceptFileRunHidden";
const char config_tag_intercept_load[] = "InterceptFileLoad";
const char config_tag_intercept_load_auto[] = "InterceptFileLoadHidden";
const char config_tag_intercept_save[] = "InterceptFileSave";
const char config_tag_intercept_transfer[] = "InterceptFileSaveApplication";
const char config_tag_intercept_save_auto[] = "InterceptFileSaveHidden";
const char config_tag_intercept_directory[] = "InterceptFileDirectory";
const char config_tag_clipboard_integrate[] = "ClipboardIntegrate";
const char config_tag_clipboard_poll_disabled[] = "ClipboardPollDisabled";
const char config_tag_clipboard_poll_content[] = "ClipboardPollContent";
const char config_tag_idle_link[] = "IdleDisconnectRemoteLink";
const char config_tag_idle_printer[] = "IdleDisconnectPrinterMirror";
const char config_tag_idle_background[] = "IdleBackgroundThrottle";
const char config_tag_print_auto_print[] = "PrintJobAutoPrint";
const char config_tag_print_auto_preview[] = "PrintJobAutoPreview";
const char config_tag_print_preview_scale[] = "PrintJobPreviewScale";
const char config_tag_print_preview_antialias[] = "PrintJobPreviewAntialias";
const char config_tag_print_print_wait[] = "PrintJobPrintTogether";
const char config_tag_print_log_debug[] = "PrintJobLogDebug";

// Configuration filename
#define CONFIG_DIR "<PsiFSConfig$Dir>"
#define CONFIG_PATH "PsiFSConfig:"
#define CONFIG_FILE "Config"
#define CONFIG_FILE_READ CONFIG_PATH CONFIG_FILE
#define CONFIG_FILE_WRITE CONFIG_DIR "." CONFIG_FILE

// The current configuration
config_store config_current;

/*
    Parameters  : void
    Returns     : bool      - Was the load successful.
    Description : Attempt to read the configuration file.
*/
bool config_store::load()
{
    // Attempt to open and read the configuration file
    ifstream file(CONFIG_FILE_READ);
    return file >> *this ? TRUE : FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to write the configuration file.
*/
void config_store::save() const
{
    // Ensure that the directory exists
    osfile_create_dir(CONFIG_DIR, 0);

    // Attempt to write the configuration file
    ofstream file(CONFIG_FILE_WRITE);
    file << *this;

    // Generate an error if failed
    if (!file || file.bad())
    {
        os_error err;

        // Generate the error
        err.errnum = 0;
        filer_msgtrans(err.errmess, sizeof(err.errmess), "ErrSave");
        os_generate_error(&err);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to read the active configuration.
*/
void config_store::read()
{
    char *str;
    int size;

    // Read the block driver name
    size = 1 - psifsget_driver_name(NULL, 0);
    str = new char[size];
    if (str)
    {
        if (0 < psifsget_driver_name(str, size))
        {
            set_str(config_tag_driver, str);
        }
        delete[] str;
    }

    // Read the block driver port
    set_num(config_tag_port, psifsget_driver_port());

    // Read the block driver baud rate
    set_num(config_tag_baud, psifsget_driver_baud());

    // Read the block driver options
    size = 1 - psifsget_driver_options(NULL, 0);
    str = new char[size];
    if (str)
    {
        if (0 < psifsget_driver_options(str, size))
        {
            set_str(config_tag_options, str);
        }
        delete[] str;
    }

    // Read the block driver automatic baud rate identification
    set_bool(config_tag_auto_baud, psifsget_driver_auto_baud());

    // Read the clock synchronization mode
    set_bool(config_tag_sync_clocks, psifsget_sync_clocks());

    // Read the idle behaviour
    set_num(config_tag_idle_link, psifsget_idle_disconnect_link());
    set_num(config_tag_idle_printer, psifsget_idle_disconnect_printer());
    set_bool(config_tag_idle_background, psifsget_idle_background_throttle());
}

/*
    Parameters  : report    - Should any errors be reported.
    Returns     : void
    Description : Attempt to write the active configuration.
*/
void config_store::write(bool report) const
{
    os_error *err = NULL;

    // Update file intercepts
    convobj_all.update_intercepts(report);

    // Write the block driver configuration
    if (exist(config_tag_driver)) err = xpsifsset_driver_name(get_str(config_tag_driver).c_str());
    if (!err && exist(config_tag_port)) err = xpsifsset_driver_port(get_num(config_tag_port));
    if (!err && exist(config_tag_baud)) err = xpsifsset_driver_baud(get_num(config_tag_baud));
    if (!err && exist(config_tag_options)) err = xpsifsset_driver_options(get_str(config_tag_options).c_str());
    if (!err && exist(config_tag_auto_baud)) err = xpsifsset_driver_auto_baud(get_bool(config_tag_auto_baud));

    // Write the clock synchronization mode
    if (!err && exist(config_tag_sync_clocks)) err = xpsifsset_sync_clocks(get_bool(config_tag_sync_clocks));

    // Write the idle behaviour
    if (!err && exist(config_tag_idle_link)) err = xpsifsset_idle_disconnect_link(get_num(config_tag_idle_link));
    if (!err && exist(config_tag_idle_printer)) err = xpsifsset_idle_disconnect_printer(get_num(config_tag_idle_printer));
    if (!err && exist(config_tag_idle_background)) err = xpsifsset_idle_background_throttle(get_bool(config_tag_idle_background));

    // Throw an error if required
    if (report && err) os_generate_error(err);
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the configuration.
*/
void config_init()
{
    config_store config;

    // Read tha active configuration
    config_current.read();

    // Attempt to load the configuration file
    if (config.load());
    {
        // Merge the loaded configuration
        config_current.merge(config);

        // Set the configuration if changed
        config_current.write(FALSE);
    }
}
