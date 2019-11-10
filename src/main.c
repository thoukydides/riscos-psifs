/*
    File        : main.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : The main module interface for the PsiFS module.

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
#include "kernel.h"

// Include oslib header files
#include "oslib/fileswitch.h"
#include "oslib/filter.h"
#include "oslib/free.h"
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "ctrl.h"
#include "debug.h"
#include "fs.h"
#include "link.h"
#include "mem.h"
#include "module.h"
#include "pollword.h"
#include "veneer.h"
#include "wimpfilt.h"

// Interval in centiseconds between ticks
#define MAIN_TICK_INTERVAL (1)

// Tick division rate for memory tidy
#define MAIN_TICK_MEM_TIDY (100)

// Mask of WIMP messages to filter
#define MAIN_FILTER_MASK (~(wimp_MASK_NULL | wimp_MASK_MESSAGE | wimp_MASK_RECORDED | wimp_MASK_ACKNOWLEDGE))

// Filing system and module names
static const char main_name[] = FS_NAME;

// Are callbacks enabled
static bool main_tick = FALSE;

// Are pre- and post-filters active
static bool main_filter = FALSE;

// A union to allow casting of function pointers
typedef union
{
    const void *ptr;
    void (*func)(void);
} main_cast;

/*
    Parameters  : r             - The input and output register values.
                  pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handler for OS_CallAfter calls.
*/
os_error *main_call_after_handler(os_register_block *r, void *pw)
{
    os_error *err = NULL;

    // Request a callback if enabled
    if (main_tick) err = xos_add_call_back((void *) (int) main_callback, pw);

    // No error can be returned
    return NULL;
}

/*
    Parameters  : r             - The input and output register values.
                  pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handler for OS_AddCallBack calls.
*/
os_error *main_callback_handler(os_register_block *r, void *pw)
{
    static bits mem_count = 0;
    os_error *err = NULL;

    // Poll the link, ignoring any error returned
    link_poll(FALSE);

    // Tidy the memory manager, ignoring any error returned
    if (MAIN_TICK_MEM_TIDY <= ++mem_count)
    {
        mem_tidy();
        mem_count = 0;
    }

    // Trigger the next callback if enabled
    if (main_tick)
    {
        err = xos_call_after(MAIN_TICK_INTERVAL,
                             (void *) (int) main_call_after, pw);
    }

    // No error can be returned
    return NULL;
}

/*
    Parameters  : pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Start callbacks.
*/
static os_error *main_start_tick(void *pw)
{
    os_error *err = NULL;

    // Enable callbacks
    main_tick = TRUE;

    // Trigger the first callback
    err = xos_call_after(MAIN_TICK_INTERVAL,
                         (void *) (int) main_call_after, pw);

    // Return any error produced
    return err;
}

/*
    Parameters  : pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : End callbacks.
*/
static os_error *main_end_tick(void *pw)
{
    os_error *err = NULL;

    // Disable callbacks
    main_tick = FALSE;

    // Remove any pending callbacks
    err = xos_remove_ticker_event((void *) (int) main_call_after, pw);
    if (!err) err = xos_remove_call_back((void *) (int) main_callback, pw);

    // Return any error produced
    return err;
}

/*
    Parameters  : r             - The input and output register values.
                  pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handler for WIMP pre-filter calls.
*/
os_error *main_pre_filter_handler(os_register_block *r, void *pw)
{
    // Peform any processing of the event required
    wimpfilt_pre((wimp_poll_flags *) &r->registers[0],
                 (wimp_block *) r->registers[1],
                 (wimp_t) r->registers[2]);

    // No error can be returned
    return NULL;
}

/*
    Parameters  : r             - The input and output register values.
                  pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handler for WIMP post-filter calls.
*/
os_error *main_post_filter_handler(os_register_block *r, void *pw)
{
    // Peform any processing of the event required
    wimpfilt_post((wimp_event_no *) &r->registers[0],
                  (wimp_block *) r->registers[1],
                  (wimp_t) r->registers[2]);

    // No error can be returned
    return NULL;
}

/*
    Parameters  : pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Add WIMP pre- and post-filter on all tasks.
*/
static os_error *main_add_filter(void *pw)
{
    os_error *err = NULL;
    main_cast cast;

    DEBUG_PRINTF(("main_add_filter"))

    // Register a pre-filter
    cast.func = main_pre_filter;
    err = xfilter_register_pre_filter(main_name, cast.ptr, pw, wimp_BROADCAST);

    // Register a post-filter
    if (!err)
    {
        cast.func = main_post_filter;
        err = xfilter_register_post_filter(main_name, cast.ptr, pw,
                                           wimp_BROADCAST, MAIN_FILTER_MASK);
    }

    // Set the status and clear any error produced
    if (!err) main_filter = TRUE;
    else err = NULL;

    // Return any error produced
    return err;
}

/*
    Parameters  : pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Remove any WIMP pre- and post-filters. Any error produced is
                  suppressed to ensure that the filing system can be
                  reinitialised.
*/
static os_error *main_remove_filter(void *pw)
{
    DEBUG_PRINTF(("main_remove_filter"))

    // No action unless filters registered
    if (main_filter)
    {
        main_cast cast;

        // Clear the flag
        main_filter = FALSE;

        // Deregister the pre-filter
        cast.func = main_pre_filter;
        xfilter_de_register_pre_filter(main_name, cast.ptr, pw,
                                       wimp_BROADCAST);

        // Deregister the post-filter
        cast.func = main_post_filter;
        xfilter_de_register_post_filter(main_name, cast.ptr, pw,
                                        wimp_BROADCAST, MAIN_FILTER_MASK);
    }

    // Suppress any error to ensure that the filing system can be reinitialised
    return NULL;
}

/*
    Parameters  : pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Add PsiFS as a filing system.
*/
static os_error *main_add_fs(void *pw)
{
    os_error *err = NULL;
    osfscontrol_fs_info_block info;

    DEBUG_PRINTF(("main_add_fs"))

    // Fill in the filing system information block
    info.name_offset = (int) main_name - Image__RO_Base;
    info.banner_offset = (int) main_name - Image__RO_Base;
    info.open_offset = (int) fsentry_open - Image__RO_Base;
    info.get_bytes = (int) fsentry_getbytes - Image__RO_Base;
    info.put_bytes = (int) fsentry_putbytes - Image__RO_Base;
    info.args_offset = (int) fsentry_args - Image__RO_Base;
    info.close_offset = (int) fsentry_close - Image__RO_Base;
    info.file_offset = (int) fsentry_file - Image__RO_Base;
    info.info = FS_INFO;
    info.func_offset = (int) fsentry_func - Image__RO_Base;
    info.gbpb_offset = (int) fsentry_gbpb - Image__RO_Base;
    info.extra_info = FS_EXTRA_INFO;

    // Add the filing system
    err = xosfscontrol_add_fs((byte const *) Image__RO_Base,
                              (int) &info - Image__RO_Base,
                              pw);

    // Register with the Free module, ignoring any error
    if (!err)
    {
        main_cast cast;
        cast.func = fsentry_free;
        xfree_register(psifs_FS_NUMBER_PSIFS, cast.ptr, pw);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Remove PsiFS as a filing system. Any error produced is
                  suppressed to ensure that the filing system can be
                  reinitialised.
*/
static os_error *main_remove_fs(void *pw)
{
    main_cast cast;

    DEBUG_PRINTF(("main_remove_fs"))

    // Deregister with the Free module
    cast.func = fsentry_free;
    xfree_de_register(psifs_FS_NUMBER_PSIFS, cast.ptr, pw);

    // Remove the filing system
    xosfscontrol_remove_fs(main_name);

    // Suppress any error to ensure that the filing system can be reinitialised
    return NULL;
}

/*
    Parameters  : cmd_tail      - The string of arguments with which the module
                                  is invoked, may be "".
                  podule_base   - 0 unless the code has been invoked from a
                                  podule.
                  pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if initialisation successful, or a
                                  pointer to an error block otherwise.
    Description : Module initialisation entry point.
*/
os_error *main_initialisation(const char *cmd_tail, int podule_base,
                              void *pw)
{
    os_error *err = NULL;
    bits cmd_len = ctrl_strlen(cmd_tail);
    NOT_USED(cmd_len);

    DEBUG_PRINTF(("Module initialisation '%.*s'", cmd_len, cmd_tail))

    // Initialise the memory manager
    err = mem_initialise();

    // Prepare the serial link
    if (!err) err = link_initialise();

    // Start callbacks
    if (!err) err = main_start_tick(pw);

    // Add the filing system
    if (!err) err = main_add_fs(pw);

    // Add WIMP filters
    if (!err) err = main_add_filter(pw);

    // Return any error produced
    return err;
}

/*
    Parameters  : fatal         - Fatality indication: 0 for non-fatal, and
                                  1 for fatal finalisation.
                  instatiation  - Dynamic instantiation number.
                  pw            - The r12 value established by module
                                  initialisation.
    Returns     : os_error *    - NULL if finalisation successful, or a
                                  pointer to an error block otherwise.
    Description : Module finalisation entry point.
*/
os_error *main_finalisation(int fatal, int instantiation, void *pw)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Module finalisation fatality=%i", fatal))

    // Check if able to die
    err = pollword_pre_finalise();

    // Remove WIMP filters
    if (!err) err = main_remove_filter(pw);

    // Remove the filing system
    if (!err) err = main_remove_fs(pw);

    // End callbacks
    if (!err) err = main_end_tick(pw);

    // Kill the serial link
    if (!err) err = link_finalise();

    // Terminate the memory manager
    if (!err) err = mem_finalise();

    // Return any error produced
    return err;
}

/*
    Parameters  : service_number    - The number of the service call to handle.
                  r                 - The input and output register values.
                  pw                - The r12 value established by module
                                      initialisation.
    Returns     : void
    Description : Module service call handler.
*/
void main_service_call(int service_number, os_register_block *r, void *pw)
{
    // Action depends on the service call
    switch (service_number)
    {
        case Service_FSRedeclare:
            // Filing system reinitialise
            DEBUG_PRINTF(("Service_FSRedeclare"))
            main_add_fs(pw);
            break;

        case Service_FilterManagerInstalled:
            // Register WIMP filters
            DEBUG_PRINTF(("Service_FilterManagerInstalled"))
            main_add_filter(pw);
            break;

        case Service_FilterManagerDying:
            // WIMP filters have been deregistered
            DEBUG_PRINTF(("Service_FilterManagerDying"))
            main_filter = FALSE;
            break;

        default:
            // Unrecognised service call
            DEBUG_PRINTF(("Unrecognised service call %i", service_number))
            break;
    }
}
