/*
    File        : command.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : The command handler for the PsiFS module.

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

// Include clib header files
#include <ctype.h>
#include <stdio.h>

// Include oslib header files
#include "oslib/osfscontrol.h"

// Include project header files
#include "args.h"
#include "ctrl.h"
#include "debug.h"
#include "err.h"
#include "fs.h"
#include "idle.h"
#include "link.h"
#include "module.h"
#include "sis.h"
#include "test.h"
#include "uid.h"
#include "util.h"

// Name of a temporary file
#define COMMAND_SCRAP "<Wimp$Scrap>"

/*
    Parameters  : cmd_tail      - Pointer to the command tail.
                  cmd_args      - Number of parameters, as counted by OSCLI,
                                  i.e. space(s) separate parameters except
                                  within double quotation marks.
                  cmd_no        - The index for this command.
                  pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if command successful, or a
                                  pointer to an error block otherwise.
    Description : Module command handler.
*/
os_error *command_handler(const char *cmd_tail, int cmd_args,
                          int cmd_no, void *pw)
{
    os_error *err = NULL;
    bits cmd_len = ctrl_strlen(cmd_tail);
    NOT_USED(cmd_len)

    // Action depends on the command
    switch (cmd_no)
    {
#ifdef CMD_Fore
        case CMD_Fore:
            // Foreground operation
            DEBUG_PRINTF(("*Fore"))
            idle_start();
            while (!err) err = link_poll(TRUE);
            idle_end();
            break;
#endif

#ifdef CMD_Test
        case CMD_Test:
            // Testing
            DEBUG_PRINTF(("*Test '%.*s'", cmd_len, cmd_tail))
            err = test(cmd_tail);
            break;
#endif

        case CMD_Drive:
            // Set the default drive to use if the directory is unset
            DEBUG_PRINTF(("*Drive '%.*s'", cmd_len, cmd_tail))
            err = args_parse("", cmd_tail);
            if (!err)
            {
                const char *drive;

                args_read_string(0, &drive);
                err = util_set_drive(drive);
            }
            break;

        case CMD_Free:
            // Displays the total free space on a disc
            DEBUG_PRINTF(("*Free '%.*s'", cmd_len, cmd_tail))
            err = args_parse("", cmd_tail);
            if (!err)
            {
                const char *path;
                int free;
                int size;

                args_read_string(0, &path);
                err = util_disc_spec(path, &path);
                if (!err)
                {
                    err = xosfscontrol_free_space(path, &free, NULL, &size);
                }
                if (!err)
                {
                    int used = size - free;

                    printf("Bytes free &%08X = %13s\n",
                           free, util_int_comma(free));
                    printf("Bytes used &%08X = %13s\n",
                           used, util_int_comma(used));
                }
            }
            break;

        case CMD_NameDisc:
        case CMD_NameDisk:
            // Alter a disc's name
            DEBUG_PRINTF(("*NameDisc/*NameDisk '%.*s'", cmd_len, cmd_tail))
            err = args_parse("/A,/A", cmd_tail);
            if (!err)
            {
                const char *path;
                const char *name;

                idle_start();
                args_read_string(0, &path);
                args_read_string(1, &name);
                err = util_disc_spec(path, &path);
                if (!err) err = fs_internal_name_resolve(ctrl_strrchr(path, FS_CHAR_DISC), &path, NULL);
                if (!err) err = fs_drive_name(path[1], name);
                idle_end();
            }
            break;

        case CMD_PsiFS:
            // Select the PsiFS filing system
            DEBUG_PRINTF(("*PsiFS"))
            err = xosfscontrol_select_fs((osfscontrol_id) psifs_FS_NUMBER_PSIFS);
            break;

        case CMD_PsiFSDisable:
            // Disable the block driver
            DEBUG_PRINTF(("*PsiFSDisable '%.*s'", cmd_len, cmd_tail))
            err = args_parse("abort/S", cmd_tail);
            if (!err) err = link_disable(args_read_switch(0));
            break;

        case CMD_PsiFSDriver:
            // Configure the serial block driver
            DEBUG_PRINTF(("*PsiFSDriver '%.*s'", cmd_len, cmd_tail))
            if (!cmd_args) err = link_list_settings(TRUE);
            else if (link_active) err = &err_link_busy;
            else
            {
                // Reconfigure the link
                err = args_parse("driver,port/E,baud/E,options,autobaud/S,noautobaud/S", cmd_tail);
                if (!err && args_read_switch(4) && args_read_switch(5))
                {
                    err = &err_bad_parms;
                }
                if (!err)
                {
                    const char *driver;
                    int port;
                    int baud;
                    const char *options;
                    bool autobaud;

                    // Parse the result
                    if (!args_read_string(0, &driver)) driver = link_driver_name;
                    if (!args_read_evaluated(1, &port) || (port < 0)) port = link_driver_port;
                    if (!args_read_evaluated(2, &baud) || (baud < 0)) baud = link_driver_baud;
                    if (!args_read_string(3, &options)) options = link_driver_options;
                    if (args_read_switch(4)) autobaud = TRUE;
                    else if (args_read_switch(5)) autobaud = FALSE;
                    else autobaud = link_driver_autobaud;

                    // Use the new settings
                    if (!err) err = link_configure(driver, port, baud, options, autobaud);
                }
            }
            break;

        case CMD_PsiFSEnable:
            // Enable the block driver
            DEBUG_PRINTF(("*PsiFSEnable"))
            err = args_parse("link/S,print/S,device", cmd_tail);
            if (!err && args_read_switch(0)
                && (args_read_switch(1) || args_read_switch(2)))
            {
                err = &err_bad_parms;
            }
            if (!err)
            {
                if (args_read_switch(1) || args_read_switch(2))
                {
                    const char *device;

                    if (!args_read_string(2, &device)) device = NULL;
                    err = link_enable_print(device);
                }
                else err = link_enable_link();
            }
            break;

        case CMD_PsiFSListDrivers:
            // List the available serial block drivers
            DEBUG_PRINTF(("*PsiFSListDrivers '%.*s'", cmd_len, cmd_tail))
            err = args_parse("verbose/S", cmd_tail);
            if (!err) err = link_list_drivers(args_read_switch(0));
            break;

        case CMD_PsiFSMap:
            // Display or change the file type mapping
            DEBUG_PRINTF(("*PsiFSMap '%.*s'", cmd_len, cmd_tail))
            if (cmd_args)
            {
                const char *str;
                bits type = UID_CLEAR_TYPE;

                err = args_parse("ext/K,uid/K,other/S,type,mimemap/S,nomimemap/S", cmd_tail);
                if (!err && args_read_switch(4) && args_read_switch(5))
                {
                    err = &err_bad_parms;
                }
                if (!err && args_read_string(3, &str))
                {
                    err = uid_parse_type(str, &type);
                }
                if (!err && (1 < (args_read_switch(0) + args_read_switch(1)
                                  + args_read_switch(2))))
                {
                    err = &err_bad_parms;
                }
                if (!err)
                {
                    if (args_read_string(0, &str))
                    {
                        err = uid_ext_mapping(str, type);
                    }
                    else if (args_read_string(1, &str))
                    {
                        epoc32_file_uid uid;

                        err = uid_parse_uid(str, &uid);
                        if (!err) err = uid_uid_mapping(&uid, type);
                    }
                    else if (args_read_switch(2))
                    {
                        err = uid_other_mapping(type);
                    }
                    else if (args_read_switch(3))
                    {
                        err = &err_bad_parms;
                    }
                }
                if (!err && args_read_switch(4)) err = uid_mime_map(TRUE);
                if (!err && args_read_switch(5)) err = uid_mime_map(FALSE);
            }
            else err = uid_list_mapping();
            break;

        case CMD_PsiFSSIS:
            // Extract the contents from a SIS file
            DEBUG_PRINTF(("*PsiFSSIS '%.*s'", cmd_len, cmd_tail))
            err = args_parse("/A,dir,tar/K,scrap/K,language/K,recurse/S,drive/K,residual/K,verbose/S", cmd_tail);
            if (!err && (!args_read_switch(0) || (args_read_switch(1) && args_read_switch(2)))) err = &err_bad_parms;
            if (!err)
            {
                const char *str;
                const char *src;
                const char *dest;
                const char *scrap;
                const char *residual;
                bool tar = FALSE;
                psifs_language language = psifs_LANGUAGE_UNKNOWN;
                psifs_drive drive = SIS_SELECTABLE_DRIVE;

                args_read_string(0, &src);
                if (!args_read_string(1, &dest))
                {
                    if (args_read_string(2, &dest)) tar = TRUE;
                    else dest = NULL;
                }
                if (!args_read_string(3, &scrap)) scrap = COMMAND_SCRAP;
                if (args_read_string(4, &str))
                {
                    err = util_language_from_code(str, &language);
                }
                if (!err && args_read_string(6, &str))
                {
                    if (!isalpha(str[0]) || str[1]) err = &err_bad_drive;
                    else drive = toupper(str[0]);
                }
                if (!args_read_string(7, &residual)) residual = NULL;
                if (!err)
                {
                    err = util_read_sis(src, dest, tar, scrap,
                                        args_read_switch(5),
                                        residual, language, drive,
                                        args_read_switch(8));
                }
            }
            break;

        case CMD_PsiFSStatus:
            // Show the current block driver and connection status
            DEBUG_PRINTF(("*PsiFSStatus"))
            err = link_list_settings(FALSE);
            break;

        case CMD_PsiFSTar:
            // Extract the contents from a tar file
            DEBUG_PRINTF(("*PsiFSTar '%.*s'", cmd_len, cmd_tail))
            err = args_parse("/A,,,verbose/S", cmd_tail);
            if (!err && !args_read_switch(0)) err = &err_bad_parms;
            if (!err)
            {
                const char *src;
                const char *pattern;
                const char *dest;

                args_read_string(0, &src);
                if (!args_read_string(1, &pattern)) pattern = "*";
                if (!args_read_string(2, &dest))
                {
                    dest = args_read_switch(3) ? NULL : "@";
                }
                err = util_read_tar(src, pattern, dest, args_read_switch(3));
            }
            break;

        default:
            // Unrecognised command code
            DEBUG_PRINTF(("Unrecognised *command %u '%.*s'", cmd_no, cmd_len, cmd_tail))
            err = &err_bad_parms;
            break;
    }

    // Return any error produced
    return err;
}
