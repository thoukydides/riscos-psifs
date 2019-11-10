/*
    File        : swi.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : The SWI handler for the PsiFS module.

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
#include <stdio.h>
#include <string.h>
#include "kernel.h"

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "async.h"
#include "cache.h"
#include "clipboard.h"
#include "code.h"
#include "connect.h"
#include "ctrl.h"
#include "debug.h"
#include "err.h"
#include "idle.h"
#include "link.h"
#include "module.h"
#include "pollword.h"
#include "print.h"
#include "printing.h"
#include "psifs.h"
#include "stats.h"
#include "uid.h"
#include "user.h"
#include "wimpfilt.h"

// A padding word
#define SWI_PAD bits : 32;

// Parameters for PsiFS_Register
typedef union
{
    struct
    {
        const char *name;
        psifs_mask mask;
    } in;
    struct
    {
        int *pollword;
    } out;
} swi_register_params;

// Parameters for PsiFS_Unregister
typedef union
{
    struct
    {
        int *pollword;
    } in;
} swi_unregister_params;

// Parameters for PsiFS_Set
typedef union
{
    struct
    {
        psifs_selector reason;
    } in;
    struct
    {
        psifs_selector reason;
        bits value;
    } in_numeric;
    struct
    {
        psifs_selector reason;
        const char *value;
    } in_string;
} swi_set_params;

// Parameters for PsiFS_Get
typedef union
{
    struct
    {
        psifs_selector reason;
    } in;
    struct
    {
        psifs_selector reason;
        char *buffer;
        int size;
    } in_string;
    struct
    {
        SWI_PAD
        bits value;
    } out_numeric;
    struct
    {
        SWI_PAD
        SWI_PAD
        int spare;
    } out_string;
} swi_get_params;

// Parameters for PsiFS_Mode
typedef union
{
    struct
    {
        psifs_mode mode;
    } in;
    struct
    {
        psifs_mode mode;
        bool abort;
    } in_inactive;
    struct
    {
        psifs_mode mode;
        const char *device;
    } in_printer;
} swi_mode_params;

// Parameters for PsiFS_AsyncStart
typedef union
{
    struct
    {
        psifs_async_op reason;
    } in;
    struct
    {
        psifs_async_op reason;
        const char *pattern;
        const char *file;
        bool append;
    } in_shutdown;
    struct
    {
        psifs_async_op reason;
        const char *file;
        bool remove;
    } in_restart;
    struct
    {
        psifs_async_op reason;
        const char *src;
        const char *dest;
    } in_read;
    struct
    {
        psifs_async_op reason;
        const char *src;
        const char *dest;
        bool remove;
    } in_write;
    struct
    {
        psifs_async_op reason;
        const char *src;
        const char *dest;
        const char *prev;
        const char *scrap;
        const char *temp;
    } in_backup;
    struct
    {
        psifs_async_op reason;
        const char *src;
        const char *dest;
        const char *exe;
        bool remove;
    } in_write_start;
    struct
    {
        psifs_async_op reason;
        const char *inst_exe;
        const char *inst_src;
        const char *inst_dest;
        bool inst_remove;
        const char *pckg_src;
        const char *pckg_dest;
        bool pckg_remove;
    } in_install;
    struct
    {
        psifs_async_handle handle;
    } out;
} swi_async_start_params;

// Parameters for PsiFS_AsyncEnd
typedef union
{
    struct
    {
        psifs_async_handle handle;
    } in;
} swi_async_end_params;

// Parameters for PsiFS_AsyncPoll
typedef union
{
    struct
    {
        psifs_async_handle handle;
    } in;
    struct
    {
        psifs_async_status status;
        const char *desc;
        const char *detail;
        const char *error;
        bits taken;
        bits remain;
    } out;
} swi_async_poll_params;

// Parameters for PsiFS_AsyncControl
typedef union
{
    struct
    {
        psifs_async_control_op reason;
        psifs_async_handle handle;
    } in;
    struct
    {
        psifs_async_control_op reason;
        psifs_async_handle handle;
        psifs_async_response response;
    } in_simple;
} swi_async_control_params;

// Parameters for PsiFS_FileOp
typedef union
{
    struct
    {
        psifs_file_op_reason reason;
    } in;
    struct
    {
        psifs_file_op_reason reason;
        psifs_drive drive;
        const char *name;
    } in_name_disc;
} swi_file_op_params;

// Parameters for PsiFS_InterceptClaim
typedef union
{
    struct
    {
        int *pollword;
        bits type;
        psifs_intercept_type mask;
    } in;
} swi_intercept_claim_params;

// Parameters for PsiFS_InterceptRelease
typedef union
{
    struct
    {
        int *pollword;
    } in;
} swi_intercept_release_params;

// Parameters for PsiFS_InterceptPoll
typedef union
{
    struct
    {
        int *pollword;
    } in;
    struct
    {
        psifs_intercept_handle handle;
        bits type;
        psifs_intercept_type mask;
        const char *orig;
        const char *temp;
        wimp_t sender;
        wimp_t receiver;
    } out;
} swi_intercept_poll_params;

// Parameters for PsiFS_InterceptControl
typedef union
{
    struct
    {
        psifs_intercept_control_op reason;
        psifs_intercept_handle handle;
    } in;
} swi_intercept_control_params;

// Parameters for PsiFS_CheckUID
typedef union
{
    struct
    {
        bits uid1;
        bits uid2;
        bits uid3;
    } in;
    struct
    {
        bits uid4;
    } out;
} swi_check_uid_params;

// Parameters for PsiFS_ClipboardCopy
typedef union
{
    struct
    {
        const char *name;
    } in;
    struct
    {
        psifs_clipboard_flags flags;
        os_t timestamp;
    } out;
} swi_clipboard_copy_params;

// Parameters for PsiFS_ClipboardPaste
typedef union
{
    struct
    {
        const char *name;
    } in;
    struct
    {
        psifs_clipboard_flags flags;
        os_t timestamp;
    } out;
} swi_clipboard_paste_params;

// Parameters for PsiFS_GetTranslationTable
typedef union
{
    struct
    {
        psifs_character_set from;
        psifs_character_set to;
        psifs_translation_table *table;
    } in;
} swi_get_translation_table_params;

// Parameters for PsiFS_PrintJobPoll
typedef union
{
    struct
    {
        psifs_print_job_handle handle;
    } in;
    struct
    {
        psifs_print_job_handle handle;
        psifs_print_job_status status;
        bits received;
        bits read;
    } out;
} swi_print_job_poll_params;

// Parameters for PsiFS_PrintJobData
typedef union
{
    struct
    {
        psifs_print_job_handle handle;
        const char *name;
    } in;
} swi_print_job_data_params;

// Parameters for PsiFS_PrintJobCancel
typedef union
{
    struct
    {
        psifs_print_job_handle handle;
    } in;
} swi_print_job_cancel_params;

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_Register.
*/
static os_error *swi_register(swi_register_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params || !params->in.name) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_Register name='%s', mask=0x%08x", params->in.name, params->in.mask))

        // Register the client
        err = pollword_register(params->in.name, params->in.mask,
                                &params->out.pollword);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_Unregister.
*/
static os_error *swi_unregister(swi_unregister_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params || !params->in.pollword) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_Unregister pollword=%p", params->in.pollword))

        // Unregister the client
        err = pollword_unregister(params->in.pollword);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_Set.
*/
static os_error *swi_set(swi_set_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        // Action depends on the reason code
        switch (params->in.reason)
        {
            case psifs_SELECT_DRIVER_NAME:
                // Set the serial block driver name
                DEBUG_PRINTF(("SWI PsiFS_Set block driver name = '%s'", params->in_string.value))
                err = link_configure(params->in_string.value,
                                     link_driver_port,
                                     link_driver_baud,
                                     link_driver_options,
                                     link_driver_autobaud);
                break;

            case psifs_SELECT_DRIVER_PORT:
                // Set the serial block driver port number
                DEBUG_PRINTF(("SWI PsiFS_Set block driver port = %u", params->in_numeric.value))
                err = link_configure(link_driver_name,
                                     params->in_numeric.value,
                                     link_driver_baud,
                                     link_driver_options,
                                     link_driver_autobaud);
                break;

            case psifs_SELECT_DRIVER_BAUD:
                // Set the serial block driver baud rate
                DEBUG_PRINTF(("SWI PsiFS_Set block driver baud rate = %u", params->in_numeric.value))
                err = link_configure(link_driver_name,
                                     link_driver_port,
                                     params->in_numeric.value,
                                     link_driver_options,
                                     link_driver_autobaud);
                break;

            case psifs_SELECT_DRIVER_OPTIONS:
                // Set the serial block driver options
                DEBUG_PRINTF(("SWI PsiFS_Set block driver options = '%s'", params->in_string.value))
                err = link_configure(link_driver_name,
                                     link_driver_port,
                                     link_driver_baud,
                                     params->in_string.value,
                                     link_driver_autobaud);
                break;

            case psifs_SELECT_DRIVER_AUTO_BAUD:
                // Set the serial block driver automatic baud identification
                DEBUG_PRINTF(("SWI PsiFS_Set block driver automatic baud rate identification = %u", params->in_numeric.value))
                err = link_configure(link_driver_name,
                                     link_driver_port,
                                     link_driver_baud,
                                     link_driver_options,
                                     params->in_numeric.value);
                break;

            case psifs_SELECT_SYNC_CLOCKS:
                // Set the clock synchronization mode
                DEBUG_PRINTF(("SWI PsiFS_Set clock synchronization = %u", params->in_numeric.value))
                cache_sync = params->in_numeric.value;
                break;

            case psifs_SELECT_IDLE_DISCONNECT_LINK:
                // Set the remote link idle disconnect time
                DEBUG_PRINTF(("SWI PsiFS_Set remote link idle disconnect = %u", params->in_numeric.value))
                idle_disconnect_link = params->in_numeric.value;
                break;

            case psifs_SELECT_IDLE_DISCONNECT_PRINTER:
                // Set the printer mirror idle disconnect time
                DEBUG_PRINTF(("SWI PsiFS_Set printer mirror idle disconnect = %u", params->in_numeric.value))
                idle_disconnect_printer = params->in_numeric.value;
                break;

            case psifs_SELECT_IDLE_BACKGROUND_THROTTLE:
                // Set the idle background operations mode
                DEBUG_PRINTF(("SWI PsiFS_Set idle background = %u", params->in_numeric.value))
                idle_background_throttle = params->in_numeric.value;
                break;

            default:
                // Unrecognised reason code
                err = &err_bad_parms;
                break;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - The control-character terminated string value
                                  to return.
                  params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Handle returning a string result from PsiFS_Get.
*/
static os_error *swi_get_string(const char *value, swi_get_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        // Treat a NULL pointer as an empty string
        if (!value) value = "";

        // Calculate the number of spare bytes in the buffer
        params->out_string.spare = params->in_string.size - ctrl_strlen(value);

        // Copy the string if buffer large enough
        if (params->in_string.buffer && (0 < params->out_string.spare))
        {
            ctrl_strcpy(params->in_string.buffer, value);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value         - The null terminated string value to return.
                  params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Handle returning a string result from PsiFS_Get.
*/
static os_error *swi_get_string_null(const char *value, swi_get_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        // Treat a NULL pointer as an empty string
        if (!value) value = "";

        // Calculate the number of spare bytes in the buffer
        params->out_string.spare = params->in_string.size - strlen(value);

        // Copy the string if buffer large enough
        if (params->in_string.buffer && (0 < params->out_string.spare))
        {
            strcpy(params->in_string.buffer, value);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : reason        - The get or set option reason code.
                  drive         - Variable to receive the drive letter.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a reason code into a drive letter.
*/
static os_error *swi_drive(bits reason, char *drive)
{
    os_error *err = NULL;

    // Check parameters
    if (!drive) err = &err_bad_parms;
    else
    {
        // Mask out all except the drive specification
        reason &= psifs_DRIVE_MASK;

        // Convert the drive specification to standard form
        if (/*(psifs_DRIVE_FIRST <= reason) &&*/ (reason <= psifs_DRIVE_LAST))
        {
            *drive = 'a' + reason - psifs_DRIVE_FIRST;
        }
        else if ((psifs_DRIVE_FIRST_UPPER <= reason)
                 && (reason <= psifs_DRIVE_LAST_UPPER))
        {
            *drive = 'a' + reason - psifs_DRIVE_FIRST_UPPER;
        }
        else if ((psifs_DRIVE_FIRST_LOWER <= reason)
                 && (reason <= psifs_DRIVE_LAST_LOWER))
        {
            *drive = 'a' + reason - psifs_DRIVE_FIRST_LOWER;
        }
        else err = &err_bad_parms;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_Get.
*/
static os_error *swi_get(swi_get_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        // Action depends on the reason code
        switch (params->in.reason)
        {
            case psifs_SELECT_MODE:
                // Get the mode of operation
                DEBUG_PRINTF(("SWI PsiFS_Get mode"))
                params->out_numeric.value = user_mode;
                break;

            case psifs_SELECT_DRIVER_NAME:
                // Get the serial block driver name
                DEBUG_PRINTF(("SWI PsiFS_Get block driver name buffer=%p, size=%i", params->in_string.buffer, params->in_string.size))
                err = swi_get_string(link_driver_name, params);
                break;

            case psifs_SELECT_DRIVER_PORT:
                // Get the serial block driver port number
                DEBUG_PRINTF(("SWI PsiFS_Get block driver port"))
                params->out_numeric.value = link_driver_port;
                break;

            case psifs_SELECT_DRIVER_BAUD:
                // Get the serial block driver baud rate
                DEBUG_PRINTF(("SWI PsiFS_Get block driver baud rate"))
                params->out_numeric.value = link_driver_baud;
                break;

            case psifs_SELECT_DRIVER_OPTIONS:
                // Get the serial block driver options
                DEBUG_PRINTF(("SWI PsiFS_Get block driver options buffer=%p, size=%i", params->in_string.buffer, params->in_string.size))
                err = swi_get_string(link_driver_options, params);
                break;

            case psifs_SELECT_DRIVER_AUTO_BAUD:
                // Get the serial block driver automatic baud identification
                DEBUG_PRINTF(("SWI PsiFS_Get block driver automatic baud rate identification mode"))
                params->out_numeric.value = link_driver_autobaud;
                break;

            case psifs_SELECT_DRIVER_ACTIVE_BAUD:
                // Get the active serial block driver baud rate
                DEBUG_PRINTF(("SWI PsiFS_Get active block driver baud rate"))
                params->out_numeric.value = link_driver_active_baud;
                break;

            case psifs_SELECT_SYNC_CLOCKS:
                // Get the clock synchronization mode
                DEBUG_PRINTF(("SWI PsiFS_Get clock synchronization mode"))
                params->out_numeric.value = cache_sync;
                break;

            case psifs_SELECT_IDLE_DISCONNECT_LINK:
                // Get the remote link idle disconnect time
                DEBUG_PRINTF(("SWI PsiFS_Get remote link idle disconnect time"))
                params->out_numeric.value = idle_disconnect_link;
                break;

            case psifs_SELECT_IDLE_DISCONNECT_PRINTER:
                // Get the printer mirror idle disconnect time
                DEBUG_PRINTF(("SWI PsiFS_Get printer mirror idle disconnect time"))
                params->out_numeric.value = idle_disconnect_printer;
                break;

            case psifs_SELECT_IDLE_BACKGROUND_THROTTLE:
                // Get the idle background operations mode
                DEBUG_PRINTF(("SWI PsiFS_Get idle background operations mode"))
                params->out_numeric.value = idle_background_throttle;
                break;

            case psifs_SELECT_STATISTICS_RECEIVED_BYTES:
                // Get the number of bytes of serial data received
                DEBUG_PRINTF(("SWI PsiFS_Get number of bytes of serial data received"))
                params->out_numeric.value = stats_rx_bytes;
                break;

            case psifs_SELECT_STATISTICS_TRANSMITTED_BYTES:
                // Get the number of bytes of serial data transmitted
                DEBUG_PRINTF(("SWI PsiFS_Get number of bytes of serial data transmitted"))
                params->out_numeric.value = stats_tx_bytes;
                break;

            case psifs_SELECT_STATISTICS_RECEIVED_VALID_FRAMES:
                // Get the number of valid protocol frames received
                DEBUG_PRINTF(("SWI PsiFS_Get number of valid protocol frames received"))
                params->out_numeric.value = stats_rx_frame;
                break;

            case psifs_SELECT_STATISTICS_RECEIVED_INVALID_FRAMES:
                // Get the number of invalid protocol frames received
                DEBUG_PRINTF(("SWI PsiFS_Get number of invalid protocol frames received"))
                params->out_numeric.value = stats_rx_err_frame;
                break;

            case psifs_SELECT_STATISTICS_RECEIVED_RETRIED_FRAMES:
                // Get the number of retries for received protocol frames
                DEBUG_PRINTF(("SWI PsiFS_Get number of retries for received protocol frames"))
                params->out_numeric.value = stats_rx_retry_frame;
                break;

            case psifs_SELECT_STATISTICS_TRANSMITTED_FRAMES:
                // Get the number of protocol frames transmitted
                DEBUG_PRINTF(("SWI PsiFS_Get number of protocol frames transmitted"))
                params->out_numeric.value = stats_tx_frame;
                break;

            case psifs_SELECT_STATISTICS_TRANSMITTED_RETRIED_FRAMES:
                // Get the number of retries for transmitted protocol frames
                DEBUG_PRINTF(("SWI PsiFS_Get number of retries for transmitted protocol frames"))
                params->out_numeric.value = stats_tx_retry_frame;
                break;

            case psifs_SELECT_LINK_STATUS:
                // Get the remote link status
                DEBUG_PRINTF(("SWI PsiFS_Get remote link status"))
                params->out_numeric.value = 0;
                if (connect_active) params->out_numeric.value |= psifs_LINK_STATUS_ACTIVE;
                if (connect_connected)
                {
                    params->out_numeric.value |= connect_era
                                                 ? psifs_LINK_STATUS_EPOC32
                                                 : psifs_LINK_STATUS_EPOC16;
                }
                break;

            case psifs_SELECT_MACHINE_TYPE:
                // Get the type of the remote machine
                DEBUG_PRINTF(("SWI PsiFS_Get type of the remote machine"))
                err = cache_machine_status(&params->out_numeric.value, NULL, NULL, NULL, NULL);
                break;

            case psifs_SELECT_MACHINE_DESCRIPTION:
                // Get the description of the remote machine type
                DEBUG_PRINTF(("SWI PsiFS_Get description of the remote machine type"))
                {
                    const char *desc;
                    err = cache_machine_status(NULL, &desc, NULL, NULL, NULL);
                    if (!err) err = swi_get_string(desc, params);
                }
                break;

            case psifs_SELECT_MACHINE_LANGUAGE:
                // Get the language of the remote machine
                DEBUG_PRINTF(("SWI PsiFS_Get language of the remote machine"))
                err = cache_machine_status(NULL, NULL, NULL, &params->out_numeric.value, NULL);
                break;

            case psifs_SELECT_MACHINE_OS_MAJOR:
                // Get the major version of the remote machine operating system
                DEBUG_PRINTF(("SWI PsiFS_Get major version of the remote machine operating system"))
                {
                    unified_version version;
                    err = cache_machine_status(NULL, NULL, NULL, NULL, &version);
                    if (!err) params->out_numeric.value = version.major;
                }
                break;

            case psifs_SELECT_MACHINE_OS_MINOR:
                // Get the minor version of the remote machine operating system
                DEBUG_PRINTF(("SWI PsiFS_Get minor version of the remote machine operating system"))
                {
                    unified_version version;
                    err = cache_machine_status(NULL, NULL, NULL, NULL, &version);
                    if (!err) params->out_numeric.value = version.minor;
                }
                break;

            case psifs_SELECT_MACHINE_OS_BUILD:
                // Get the build of the remote machine operating system
                DEBUG_PRINTF(("SWI PsiFS_Get build of the remote machine operating system"))
                {
                    unified_version version;
                    err = cache_machine_status(NULL, NULL, NULL, NULL, &version);
                    if (!err) params->out_numeric.value = version.build;
                }
                break;

            case psifs_SELECT_MACHINE_ID_LOW:
                // Get the low word of the unique identifier of the remote machine
                DEBUG_PRINTF(("SWI PsiFS_Get low word of the unique identifier of the remote machine"))
                {
                    unified_machine_id id;
                    err = cache_machine_status(NULL, NULL, &id, NULL, NULL);
                    if (!err) params->out_numeric.value = id.low;
                }
                break;

            case psifs_SELECT_MACHINE_ID_HIGH:
                // Get the high word of the unique identifier of the remote machine
                DEBUG_PRINTF(("SWI PsiFS_Get high word of the unique identifier of the remote machine"))
                {
                    unified_machine_id id;
                    err = cache_machine_status(NULL, NULL, &id, NULL, NULL);
                    if (!err) params->out_numeric.value = id.high;
                }
                break;

            case psifs_SELECT_MACHINE_OWNER:
                // Get the owner information of the remote machine
                DEBUG_PRINTF(("SWI PsiFS_Get owner information of the remote machine"))
                {
                    const char *owner;
                    err = cache_owner_status(&owner);
                    if (!err) err = swi_get_string_null(owner, params);
                }
                break;

            case psifs_SELECT_POWER_MAIN_STATUS:
                // Get the status of the main battery
                DEBUG_PRINTF(("SWI PsiFS_Get status of the main battery"))
                {
                    unified_battery status;
                    err = cache_power_status(&status, NULL, NULL);
                    if (!err) params->out_numeric.value = status.status;
                }
                break;

            case psifs_SELECT_POWER_MAIN_VOLTAGE:
                // Get the voltage of the main battery
                DEBUG_PRINTF(("SWI PsiFS_Get voltage of the main battery"))
                {
                    unified_battery status;
                    err = cache_power_status(&status, NULL, NULL);
                    if (!err) params->out_numeric.value = status.mv;
                }
                break;

            case psifs_SELECT_POWER_MAIN_MAX_VOLTAGE:
                // Get the maximum voltage of the main battery
                DEBUG_PRINTF(("SWI PsiFS_Get maximum voltage of the main battery"))
                {
                    unified_battery status;
                    err = cache_power_status(&status, NULL, NULL);
                    if (!err) params->out_numeric.value = status.mv_max;
                }
                break;

            case psifs_SELECT_POWER_BACKUP_STATUS:
                // Get the status of the backup battery
                DEBUG_PRINTF(("SWI PsiFS_Get status of the backup battery"))
                {
                    unified_battery status;
                    err = cache_power_status(NULL, &status, NULL);
                    if (!err) params->out_numeric.value = status.status;
                }
                break;

            case psifs_SELECT_POWER_BACKUP_VOLTAGE:
                // Get the voltage of the backup battery
                DEBUG_PRINTF(("SWI PsiFS_Get voltage of the backup battery"))
                {
                    unified_battery status;
                    err = cache_power_status(NULL, &status, NULL);
                    if (!err) params->out_numeric.value = status.mv;
                }
                break;

            case psifs_SELECT_POWER_BACKUP_MAX_VOLTAGE:
                // Get the maximum voltage of the backup battery
                DEBUG_PRINTF(("SWI PsiFS_Get maximum voltage of the backup battery"))
                {
                    unified_battery status;
                    err = cache_power_status(NULL, &status, NULL);
                    if (!err) params->out_numeric.value = status.mv_max;
                }
                break;

            case psifs_SELECT_POWER_EXTERNAL:
                // Get the status of external power
                DEBUG_PRINTF(("SWI PsiFS_Get status of external power"))
                err = cache_power_status(NULL, NULL, (bool *) &params->out_numeric.value);
                break;

            case psifs_SELECT_PRINTER_STATUS:
                // Get the status of the printer mirror
                DEBUG_PRINTF(("SWI PsiFS_Get printer mirror status"))
                params->out_numeric.value = 0;
                if (print_active) params->out_numeric.value |= psifs_PRINTER_STATUS_ACTIVE;
                break;

            case psifs_SELECT_PRINTER_DEVICE:
                // Get the name of the printer mirror destination device
                DEBUG_PRINTF(("SWI PsiFS_Get printer mirror device buffer=%p, size=%i", params->in_string.buffer, params->in_string.size))
                err = swi_get_string(print_device, params);
                break;

            default:
                // Not a simple reason code
                if ((params->in.reason & ~psifs_DRIVE_MASK)
                    == psifs_SELECT_DRIVE_STATUS)
                {
                    char drive;

                    // Get the status of the specified remote drive
                    DEBUG_PRINTF(("SWI PsiFS_Get drive %u status", params->in.reason & psifs_DRIVE_MASK))

                    err = swi_drive(params->in.reason, &drive);
                    if (!err) err = cache_drive_status(drive, &params->out_numeric.value, NULL, NULL);
                }
                else if ((params->in.reason & ~psifs_DRIVE_MASK)
                         == psifs_SELECT_DRIVE_NAME)
                {
                    char drive;
                    const char *name;

                    // Get the name of the specified remote drive
                    DEBUG_PRINTF(("SWI PsiFS_Get drive %u name buffer=%p, size=%i", params->in.reason & psifs_DRIVE_MASK, params->in_string.buffer, params->in_string.size))

                    err = swi_drive(params->in.reason, &drive);
                    if (!err) err = cache_drive_status(drive, NULL, &name, NULL);
                    if (!err) err = swi_get_string(name, params);
                }
                else if ((params->in.reason & ~psifs_DRIVE_MASK)
                         == psifs_SELECT_DRIVE_ID)
                {
                    char drive;

                    // Get the unique ID of the specified remote drive
                    DEBUG_PRINTF(("SWI PsiFS_Get drive %u unique ID", params->in.reason & psifs_DRIVE_MASK))

                    err = swi_drive(params->in.reason, &drive);
                    if (!err) err = cache_drive_status(drive, NULL, NULL, &params->out_numeric.value);
                }
                else
                {
                    // Unrecognised reason code
                    err = &err_bad_parms;
                }
                break;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_Mode.
*/
static os_error *swi_mode(swi_mode_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        // Action depends on the specified mode
        switch (params->in.mode)
        {
            case psifs_MODE_INACTIVE:
                // Set the inactive mode
                DEBUG_PRINTF(("SWI PsiFS_Mode inactive abort=%u", params->in_inactive.abort))
                err = link_disable(params->in_inactive.abort);
                break;

            case psifs_MODE_LINK:
                // Set remote link mode
                DEBUG_PRINTF(("SWI PsiFS_Mode remote link"))
                err = link_enable_link();
                break;

            case psifs_MODE_PRINTER:
                // Set printer mirror mode
                DEBUG_PRINTF(("SWI PsiFS_Mode printer mirror device='%s'", params->in_printer.device))
                err = link_enable_print(params->in_printer.device);
                break;

            default:
                // Unrecognised reason code
                DEBUG_PRINTF(("Unrecognised SWI PsiFS_Mode mode %u", params->in.mode))
                err = &err_bad_parms;
                break;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_AsyncStart.
*/
static os_error *swi_async_start(swi_async_start_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        async_op op;

        // Action depends on the specified reason code
        switch (params->in.reason)
        {
            case psifs_ASYNC_SHUTDOWN:
                // Close all open files on a drive
                DEBUG_PRINTF(("SWI PsiFS_AsyncStart shutdown pattern='%s', file='%s', append=%u", params->in_shutdown.pattern, params->in_shutdown.file, params->in_shutdown.append))
                if ((op.data.shutdown.pattern
                     && (sizeof(op.data.shutdown.pattern)
                         <= ctrl_strlen(params->in_shutdown.pattern)))
                    || (sizeof(op.data.shutdown.path)
                        <= ctrl_strlen(params->in_shutdown.file)))
                {
                    err = &err_bad_name;
                }
                if (!err)
                {
                    op.op = psifs_ASYNC_SHUTDOWN;
                    ctrl_strcpy(op.data.shutdown.pattern,
                                params->in_shutdown.pattern
                                ? params->in_shutdown.pattern
                                : "");
                    ctrl_strcpy(op.data.shutdown.path,
                                params->in_shutdown.file);
                    op.data.shutdown.append = params->in_shutdown.append;
                    err = async_start(&op, &params->out.handle);
                }
                break;

            case psifs_ASYNC_RESTART:
                // Open the specified files
                DEBUG_PRINTF(("SWI PsiFS_AsyncStart restart file='%s', remove=%u", params->in_restart.file, params->in_restart.remove))
                if (sizeof(op.data.restart.path)
                    <= ctrl_strlen(params->in_restart.file))
                {
                    err = &err_bad_name;
                }
                if (!err)
                {
                    op.op = psifs_ASYNC_RESTART;
                    ctrl_strcpy(op.data.restart.path, params->in_restart.file);
                    op.data.restart.remove = params->in_restart.remove;
                    err = async_start(&op, &params->out.handle);
                }
                break;

            case psifs_ASYNC_READ:
                // Read the specified file
                DEBUG_PRINTF(("SWI PsiFS_AsyncStart read src='%s', dest='%s'", params->in_read.src, params->in_read.dest))
                if ((sizeof(op.data.read.src)
                     <= ctrl_strlen(params->in_read.src))
                    || (sizeof(op.data.read.dest)
                        <= ctrl_strlen(params->in_read.dest)))
                {
                    err = &err_bad_name;
                }
                if (!err)
                {
                    op.op = psifs_ASYNC_READ;
                    ctrl_strcpy(op.data.read.src, params->in_read.src);
                    ctrl_strcpy(op.data.read.dest, params->in_read.dest);
                    err = async_start(&op, &params->out.handle);
                }
                break;

            case psifs_ASYNC_WRITE:
                // Write the specified file
                DEBUG_PRINTF(("SWI PsiFS_AsyncStart write src='%s', dest='%s', remove=%u", params->in_write.src, params->in_write.dest, params->in_write.remove))
                if ((sizeof(op.data.write.src)
                     <= ctrl_strlen(params->in_write.src))
                    || (sizeof(op.data.write.dest)
                        <= ctrl_strlen(params->in_write.dest)))
                {
                    err = &err_bad_name;
                }
                if (!err)
                {
                    op.op = psifs_ASYNC_WRITE;
                    ctrl_strcpy(op.data.write.src, params->in_write.src);
                    ctrl_strcpy(op.data.write.dest, params->in_write.dest);
                    op.data.write.remove = params->in_write.remove;
                    err = async_start(&op, &params->out.handle);
                }
                break;

            case psifs_ASYNC_BACKUP:
                // Backup a single directory tree
                DEBUG_PRINTF(("SWI PsiFS_AsyncStart write src='%s', dest='%s', prev='%s', scrap='%s', temp='%s'", params->in_backup.src, params->in_backup.dest, params->in_backup.prev ? params->in_backup.prev : "", params->in_backup.scrap ? params->in_backup.scrap : "", params->in_backup.temp))
                if ((sizeof(op.data.backup.src)
                     <= ctrl_strlen(params->in_backup.src))
                    || (sizeof(op.data.backup.dest)
                        <= ctrl_strlen(params->in_backup.dest))
                    || (params->in_backup.prev
                        && (sizeof(op.data.backup.prev)
                            <= ctrl_strlen(params->in_backup.prev)))
                    || (params->in_backup.scrap
                        && (sizeof(op.data.backup.scrap)
                            <= ctrl_strlen(params->in_backup.scrap)))
                    || (sizeof(op.data.backup.temp)
                        <= ctrl_strlen(params->in_backup.temp)))
                {
                    err = &err_bad_name;
                }
                if (!err)
                {
                    op.op = psifs_ASYNC_BACKUP;
                    ctrl_strcpy(op.data.backup.src, params->in_backup.src);
                    ctrl_strcpy(op.data.backup.dest, params->in_backup.dest);
                    ctrl_strcpy(op.data.backup.prev, params->in_backup.prev
                                                     ? params->in_backup.prev
                                                     : "");
                    ctrl_strcpy(op.data.backup.scrap, params->in_backup.scrap
                                                      ? params->in_backup.scrap
                                                      : "");
                    ctrl_strcpy(op.data.backup.temp, params->in_backup.temp);
                    err = async_start(&op, &params->out.handle);
                }
                break;

            case psifs_ASYNC_WRITE_START:
                // Write and start the specified file
                DEBUG_PRINTF(("SWI PsiFS_AsyncStart write and start src='%s', dest='%s', exe='%s', remove=%u", params->in_write_start.src, params->in_write_start.dest, params->in_write_start.exe, params->in_write_start.remove))
                if ((sizeof(op.data.write_start.src)
                     <= ctrl_strlen(params->in_write_start.src))
                    || (sizeof(op.data.write_start.dest)
                        <= ctrl_strlen(params->in_write_start.dest))
                    || (op.data.write_start.exe
                        && (sizeof(op.data.write_start.exe)
                            <= ctrl_strlen(params->in_write_start.exe))))
                {
                    err = &err_bad_name;
                }
                if (!err)
                {
                    op.op = psifs_ASYNC_WRITE_START;
                    ctrl_strcpy(op.data.write_start.src,
                                params->in_write_start.src);
                    ctrl_strcpy(op.data.write_start.dest,
                                params->in_write_start.dest);
                    ctrl_strcpy(op.data.write_start.exe,
                                params->in_write_start.exe
                                ? params->in_write_start.exe
                                : "");
                    op.data.write_start.remove = params->in_write_start.remove;
                    err = async_start(&op, &params->out.handle);
                }
                break;

            case psifs_ASYNC_INSTALL:
                // Write and install the specified file
                DEBUG_PRINTF(("SWI PsiFS_AsyncStart install inst_exe='%s', inst_src='%s', inst_dest='%s', inst_remove=%u, pckg_src='%s', pckg_dest='%s', pckg_remove=%u", params->in_install.inst_exe, params->in_install.inst_src, params->in_install.inst_dest, params->in_install.inst_remove, params->in_install.pckg_src, params->in_install.pckg_dest, params->in_install.pckg_remove))
                if ((sizeof(op.data.write_start.exe)
                     <= ctrl_strlen(params->in_write_start.exe))
                    || (sizeof(op.data.write_start.src)
                        <= ctrl_strlen(params->in_write_start.src))
                    || (sizeof(op.data.write_start.dest)
                        <= ctrl_strlen(params->in_write_start.dest)))
                {
                    err = &err_bad_name;
                }
                if (!err)
                {
                    op.op = psifs_ASYNC_INSTALL;
                    ctrl_strcpy(op.data.install.inst_exe,
                                params->in_install.inst_exe);
                    ctrl_strcpy(op.data.install.inst_src,
                                params->in_install.inst_src);
                    ctrl_strcpy(op.data.install.inst_dest,
                                params->in_install.inst_dest);
                    op.data.install.inst_remove = params->in_install.inst_remove;
                    ctrl_strcpy(op.data.install.pckg_src,
                                params->in_install.pckg_src);
                    ctrl_strcpy(op.data.install.pckg_dest,
                                params->in_install.pckg_dest);
                    op.data.install.pckg_remove = params->in_install.pckg_remove;
                    err = async_start(&op, &params->out.handle);
                }
                break;

            default:
                // Unrecognised reason code
                DEBUG_PRINTF(("Unrecognised SWI PsiFS_AsyncStart reason %u", params->in.reason))
                err = &err_bad_parms;
                break;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_AsyncEnd.
*/
static os_error *swi_async_end(swi_async_end_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_AsyncEnd handle=%u", params->in.handle))

        // End the operation
        err = async_end(params->in.handle);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_AsyncPoll.
*/
static os_error *swi_async_poll(swi_async_poll_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        // DEBUG_PRINTF(("SWI PsiFS_AsyncPoll handle=%u", params->in.handle))

        // Poll the status of the operation
        err = async_poll(params->in.handle, &params->out.status,
                         &params->out.desc, &params->out.detail,
                         &params->out.error, &params->out.taken,
                         &params->out.remain);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_AsyncControl.
*/
static os_error *swi_async_control(swi_async_control_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_AsyncControl"))

        // Action depends on the specified reason code
        switch (params->in.reason)
        {
            case psifs_ASYNC_SIMPLE_RESPONSE:
                // Simple response to query
                DEBUG_PRINTF(("SWI PsiFS_AsyncControl simple response handle=%u response=%u", params->in.handle, params->in_simple.response))
                err = async_response(params->in.handle,
                                     params->in_simple.response);
                break;

            case psifs_ASYNC_PAUSE:
                // Pause operation
                DEBUG_PRINTF(("SWI PsiFS_AsyncControl pause operation handle=%u", params->in.handle))
                err = async_pause(params->in.handle);
                break;

            case psifs_ASYNC_RESUME:
                // Resume operation
                DEBUG_PRINTF(("SWI PsiFS_AsyncControl resume operation handle=%u", params->in.handle))
                err = async_resume(params->in.handle);
                break;

            default:
                // Unrecognised reason code
                DEBUG_PRINTF(("Unrecognised SWI PsiFS_AsyncControl reason %u", params->in.reason))
                err = &err_bad_parms;
                break;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_FileOp.
*/
static os_error *swi_file_op(swi_file_op_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_FileOp"))

        // Action depends on the specified reason code
        switch (params->in.reason)
        {
            case psifs_FILE_OP_NAME_DISC:
                // Change the name of the specified remote drive
                DEBUG_PRINTF(("SWI PsiFS_FileOp name disc drive=%u name=%s", params->in_name_disc.drive, params->in_name_disc.name))
                {

                    char drive;
                    idle_start();
                    err = swi_drive(params->in_name_disc.drive, &drive);
                    if (!err) err = fs_drive_name(drive, params->in_name_disc.name);
                    idle_end();
                }
                break;

            default:
                // Unrecognised reason code
                DEBUG_PRINTF(("Unrecognised SWI PsiFS_FileOp reason %u", params->in.reason))
                err = &err_bad_parms;
                break;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_InterceptClaim.
*/
static os_error *swi_intercept_claim(swi_intercept_claim_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_InterceptClaim pollword=%p type=0x%03x, mask=%08x", params->in.pollword, params->in.type, params->in.mask))

        // Claim the file type intercept
        err = wimpfilt_claim(params->in.pollword, params->in.type,
                             params->in.mask);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_InterceptRelease.
*/
static os_error *swi_intercept_release(swi_intercept_release_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_InterceptRelease pollword=%p", params->in.pollword))

        // Release all file type intercepts
        err = wimpfilt_release(params->in.pollword);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_InterceptPoll.
*/
static os_error *swi_intercept_poll(swi_intercept_poll_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_InterceptPoll pollword=%p", params->in.pollword))

        // Poll for new intercepted file transfer intercepts
        err = wimpfilt_poll(params->in.pollword, &params->out.handle,
                            &params->out.type, &params->out.mask,
                            &params->out.orig, &params->out.temp,
                            &params->out.sender, &params->out.receiver);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_InterceptControl.
*/
static os_error *swi_intercept_control(swi_intercept_control_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_InterceptControl"))

        // Action depends on the specified reason code
        switch (params->in.reason)
        {
            case psifs_INTERCEPT_RESTART:
                // Restart the intercepted file transfer
                DEBUG_PRINTF(("SWI PsiFS_InterceptControl restart handle=%u", params->in.handle))
                err = wimpfilt_restart(params->in.handle);
                break;

            case psifs_INTERCEPT_REPLACE:
                // Replace the intercepted file transfer
                DEBUG_PRINTF(("SWI PsiFS_InterceptControl replace handle=%u", params->in.handle))
                err = wimpfilt_replace(params->in.handle);
                break;

            case psifs_INTERCEPT_CANCEL:
                // Cancel the intercepted file transfer
                DEBUG_PRINTF(("SWI PsiFS_InterceptControl cancel handle=%u", params->in.handle))
                err = wimpfilt_cancel(params->in.handle);
                break;

            default:
                // Unrecognised reason code
                DEBUG_PRINTF(("Unrecognised SWI PsiFS_InterceptControl operation %u", params->in.reason))
                err = &err_bad_parms;
                break;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_CheckUID.
*/
static os_error *swi_check_uid(swi_check_uid_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        epoc32_file_uid uid;

        DEBUG_PRINTF(("SWI PsiFS_CheckUID uid1=%08x uid2=%08x uid3=%08x", params->in.uid1, params->in.uid2, params->in.uid3))

        // Calculate the UID checksum
        uid.uid1 = params->in.uid1;
        uid.uid2 = params->in.uid2;
        uid.uid3 = params->in.uid3;
        err = uid_checksum_uid(&uid, &params->out.uid4);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_ClipboardCopy.
*/
static os_error *swi_clipboard_copy(swi_clipboard_copy_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_ClipboardCopy %s", params->in.name))

        // Perform the copy
        err = clipboard_copy(params->in.name);

        // Read the clipboard status
        if (!err) err = clipboard_poll(&params->out.flags, &params->out.timestamp);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_ClipboardPaste.
*/
static os_error *swi_clipboard_paste(swi_clipboard_paste_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_ClipboardPaste %s", params->in.name))

        // Perform the paste if appropriate
        if (params->in.name) err = clipboard_paste(params->in.name);

        // Read the clipboard status
        if (!err) err = clipboard_poll(&params->out.flags, &params->out.timestamp);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_GetTranslationTable.
*/
static os_error *swi_get_translation_table(swi_get_translation_table_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_GetTranslationTable from=%u to=%u table=%p", params->in.from, params->in.to, params->in.table))

        // Construct the required table
        err = code_get_table(params->in.from, params->in.to, params->in.table);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_PrintJobPoll.
*/
static os_error *swi_print_job_poll(swi_print_job_poll_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_PrintJobPoll handle=%u", params->in.handle))

        // Find the handle of the next print job if appropriate
        if (params->in.handle == psifs_PRINT_JOB_INVALID)
        {
            err = printing_next(&params->out.handle);
        }
        else params->out.handle = params->in.handle;

        // Poll the status of a print job
        if (!err)
        {
            err = printing_poll(params->out.handle, &params->out.status,
                                &params->out.received, &params->out.read);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_PrintJobData.
*/
static os_error *swi_print_job_data(swi_print_job_data_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_PrintJobData handle=%u, name=%s", params->in.handle, params->in.name))

        // Read a page from a print job
        err = printing_read(params->in.handle, params->in.name);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : params        - The input and output registers for this SWI.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : SWI handler for PsiFS_PrintJob.
*/
static os_error *swi_print_job_cancel(swi_print_job_cancel_params *params)
{
    os_error *err = NULL;

    // Check parameters
    if (!params) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("SWI PsiFS_PrintJobCancel handle=%u", params->in.handle))

        // Cancel a print job
        err = printing_cancel(params->in.handle);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : swi_offset    - The offset within the SWI chunk.
                  r             - The input and output register values.
                  pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if command successful, or a
                                  pointer to an error block otherwise.
    Description : Module SWI handler.
*/
os_error *swi_handler(int swi_offset, os_register_block *r, void *pw)
{
    os_error *err = NULL;

    // The action depends on the specified SWI
    switch (PsiFS_00 + swi_offset)
    {
        case PsiFS_Register:
            // Register a client to be informed of changes
            err = swi_register((swi_register_params *) r);
            break;

        case PsiFS_Unregister:
            // Unregister a previously registered client
            err = swi_unregister((swi_unregister_params *) r);
            break;

        case PsiFS_Set:
            // Set the value of an option
            err = swi_set((swi_set_params *) r);
            break;

        case PsiFS_Get:
            // Get an option or status value
            err = swi_get((swi_get_params *) r);
            break;

        case PsiFS_Mode:
            // Set the mode of operation
            err = swi_mode((swi_mode_params *) r);
            break;

        case PsiFS_AsyncStart:
            // Start an asynchronous remote operation
            err = swi_async_start((swi_async_start_params *) r);
            break;

        case PsiFS_AsyncEnd:
            // End an asynchronous remote operation
            err = swi_async_end((swi_async_end_params *) r);
            break;

        case PsiFS_AsyncPoll:
            // Poll the status of an asynchronous remote operation
            err = swi_async_poll((swi_async_poll_params *) r);
            break;

        case PsiFS_AsyncControl:
            // Control an asynchronous remote operation
            err = swi_async_control((swi_async_control_params *) r);
            break;

        case PsiFS_FileOp:
            // Perform a miscellaneous file system operation
            err = swi_file_op((swi_file_op_params *) r);
            break;

        case PsiFS_InterceptClaim:
            // Claim an intercepted file type
            err = swi_intercept_claim((swi_intercept_claim_params *) r);
            break;

        case PsiFS_InterceptRelease:
            // Release all intercepted file types
            err = swi_intercept_release((swi_intercept_release_params *) r);
            break;

        case PsiFS_InterceptPoll:
            // Poll for any new intercepted file transfers
            err = swi_intercept_poll((swi_intercept_poll_params *) r);
            break;

        case PsiFS_InterceptControl:
            // Control an intercepted file transfer
            err = swi_intercept_control((swi_intercept_control_params *) r);
            break;

        case PsiFS_CheckUID:
            // Calculate the checksum for a UID
            err = swi_check_uid((swi_check_uid_params *) r);
            break;

        case PsiFS_ClipboardCopy:
            // Write to the remote clipboard
            err = swi_clipboard_copy((swi_clipboard_copy_params *) r);
            break;

        case PsiFS_ClipboardPaste:
            // Read from the remote clipboard
            err = swi_clipboard_paste((swi_clipboard_paste_params *) r);
            break;

        case PsiFS_GetTranslationTable:
            // Obtain a character translation table
            err = swi_get_translation_table((swi_get_translation_table_params *) r);
            break;

        case PsiFS_PrintJobPoll:
            // Poll the status of a print job
            err = swi_print_job_poll((swi_print_job_poll_params *) r);
            break;

        case PsiFS_PrintJobData:
            // Read a page from a print job
            err = swi_print_job_data((swi_print_job_data_params *) r);
            break;

        case PsiFS_PrintJobCancel:
            // Cancel a print job
            err = swi_print_job_cancel((swi_print_job_cancel_params *) r);
            break;

        default:
            // Unrecognised SWI offset
            DEBUG_PRINTF(("Unrecognised SWI offset %u", swi_offset))
            err = (os_error *) error_BAD_SWI;
            break;
    }

    // Return any error produced
    return err;
}
