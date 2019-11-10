/*
    File        : wimpfilt.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : WIMP filter handlers for the PsiFS module.

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
#include "wimpfilt.h"

// Include clib header files
#include <stdio.h>
#include <stddef.h>
#include <string.h>

// Include oslib header files
#include "oslib/macros.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"
#include "oslib/taskmanager.h"

// Include project header files
#include "ctrl.h"
#include "debug.h"
#include "err.h"
#include "fs.h"
#include "mem.h"
#include "module.h"
#include "pollword.h"
#include "util.h"

/*
    Load from filer

        Normal:
            DataLoad with ref=0 specifying path
            -> load file
            DataLoadAck

        Intercept:
            DataLoad (hide)
            -> copy file, save details
            DataLoadAck (fake)

        Restart:
            Quit (trigger restart)
            DataLoad (fake, using original path)
            -> load file
            DataLoadAck (hide)

        Replace:
            Quit (trigger restart)
            DataLoad (fake, using new path)
            -> load file
            DataLoadAck (hide)

        Problems:
            Setting fields will not work, so need a way to suppress intercept

    Save to filer

        Normal:
            DataSave with ref=0 specifying leaf
            DataSaveAck specifying path and *safe*
            -> save file
            DataLoad
            DataLoadAck

        Intercept:
            DataSave (hide)
            DataSaveAck (fake)
            -> save file
            DataLoad (hide)
            DataLoadAck (fake)

        Restart:
            DataSave (fake)
            DataSaveAck (hide)
            Copy file
            DataLoad (fake)
            DataLoadAck (hide)
            DataSaved (fake, using stored ref)

        Replace:
            DataSave (fake)
            DataSaveAck (hide)
            Copy file
            DataLoad (fake)
            DataLoadAck (hide)
            DataSaved (fake, using stored ref)

    Data transfer between tasks

        Normal:
            DataSave with ref=0 specifying leaf
            [RAMFetch, not acknowledged]
            DataSaveAck with <Wimp$Scrap> and *unsafe*
            -> save file
            DataLoad
            -> load file, delete temporary
            DataLoadAck
        or
            DataSave with ref=0 specifying leaf
            RAMFetch*
            RAMTransmit*

        Intercept:
            DataSave (hide)
            DataSaveAck (fake)
            -> save file
            DataLoad (hide)
            DataLoadAck (fake)

        Restart:
            DataSave (fake)
            [RAMFetch (hide, not acknowledged)]
            DataSaveAck (hide)
            Copy file
            DataLoad (fake)
            DataLoadAck (hide)

        Replace:
            DataSave (fake)
            [RAMFetch (hide, not acknowledged)]
            DataSaveAck (hide)
            Copy file
            DataLoad (fake)
            DataLoadAck (hide)

    Run from filer

        Normal:
            DataOpen with ref=0 specifying path
            -> load file
            LoadDataAck

        Intercept (always):
            DataOpen (hide)
            -> copy file, save details
            LoadDataAck (fake)

        Intercept (unclaimed):
            DataOpen
            not acknowledged Data_Open (hide)
            -> copy file
            LoadDataAck (fake)

        Restart:
            Quit (trigger restart)
            DataOpen (fake)
            -> load file
            LoadDataAck (hide)

        Replace:
            Quit (trigger restart)
            DataOpen (fake)
            -> load file
            LoadDataAck (hide)

        Problems:
            Can't replace/restart if need to load application
*/

// A claimed intercept
typedef struct wimpfilt_claimed
{
    struct wimpfilt_claimed *next;
    struct wimpfilt_claimed *prev;
    int *pollword;
    bits type;
    psifs_intercept_type mask;
} wimpfilt_claimed;

// List of claimed intercepts
static wimpfilt_claimed *wimpfilt_claimed_head = NULL;

// Internal operation status
typedef bits wimpfilt_op;
#define WIMPFILT_CAPTURE ((wimpfilt_op) 0x00)
#define WIMPFILT_READY ((wimpfilt_op) 0x01)
#define WIMPFILT_IDLE ((wimpfilt_op) 0x02)
#define WIMPFILT_RESTART ((wimpfilt_op) 0x03)
#define WIMPFILT_REPLACE ((wimpfilt_op) 0x04)
#define WIMPFILT_DONE ((wimpfilt_op) 0x05)
#define WIMPFILT_OPS (WIMPFILT_DONE + 1)

// An active intercept
typedef struct wimpfilt_intercept
{
    struct wimpfilt_intercept *next;
    struct wimpfilt_intercept *prev;
    psifs_intercept_handle handle;
    int *pollword;
    wimpfilt_op op;
    wimp_t sender;
    wimp_t receiver;
    int ref;
    int saved_ref;
    wimp_message_data_xfer xfer;
    psifs_intercept_type mask;
    fs_pathname temp;
    os_t delay;
} wimpfilt_intercept;

// List of active intercepts
static wimpfilt_intercept *wimpfilt_intercept_head = NULL;

// The next handle to allocate
static psifs_intercept_handle wimpfilt_next_handle = 0;

// Directory to contain temporary files
#define WIMPFILT_TEMP_DIR "<PsiFSScrap$Dir>"
#define WIMPFILT_TEMP_SUBDIR "<PsiFSScrap$Dir>.Intercept"
#define WIMPFILT_TEMP_PATH "PsiFSScrap:Intercept."

// Delayed deletion of intercepts
#define WIMPFILT_DELAY_DIR (60 * 100)
#define WIMPFILT_DELAY_FILE (10 * 100)
static os_t wimpfilt_delay_wipe = 0;

// Fake message used to restart a transfer
#define WIMPFILT_MESSAGE_RESTART (message_MENU_WARNING)

// List of message types of interest
static const bits wimpfilt_actions[] =
{
    WIMPFILT_MESSAGE_RESTART,
    message_DATA_SAVE,
    message_DATA_SAVE_ACK,
    message_DATA_LOAD,
    message_DATA_LOAD_ACK,
    message_DATA_OPEN,
    message_RAM_FETCH
};
#define WIMPFILT_ACTIONS (sizeof(wimpfilt_actions) / sizeof(bits))

// Keys to forcibly enable (left Alt) or disable (right Alt, Shift) intercept
#define WIMPFILT_KEY_ENABLE (0xfa)
#define WIMPFILT_KEY_DISABLE (0xf7)
#define WIMPFILT_KEY_DISABLE2 (0xff)

// Mask of main intercept types
#define WIMPFILT_MASK (psifs_INTERCEPT_LOAD | psifs_INTERCEPT_SAVE | psifs_INTERCEPT_TRANSFER | psifs_INTERCEPT_RUN_ALL | psifs_INTERCEPT_RUN_UNCLAIMED)

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle user messages or acknowledgements.
*/
typedef os_error *wimpfilt_act_func(wimpfilt_intercept *intercept,
                                    wimp_event_no *event, wimp_message *message,
                                    wimp_t task, bool *done);

// Prototype functions to be used for state machine actions
static wimpfilt_act_func wimpfilt_start_load;
static wimpfilt_act_func wimpfilt_restart_load;
static wimpfilt_act_func wimpfilt_replace_load;
static wimpfilt_act_func wimpfilt_start_save;
static wimpfilt_act_func wimpfilt_copy_save;
static wimpfilt_act_func wimpfilt_ready_save;
static wimpfilt_act_func wimpfilt_done_save;
static wimpfilt_act_func wimpfilt_restart_save;
static wimpfilt_act_func wimpfilt_start_run_all;
static wimpfilt_act_func wimpfilt_start_run_unclaimed;
static wimpfilt_act_func wimpfilt_restart_run;
static wimpfilt_act_func wimpfilt_replace_run;
static wimpfilt_act_func wimpfilt_fail_run;
static wimpfilt_act_func wimpfilt_msg_ready;
static wimpfilt_act_func wimpfilt_msg_bad;
static wimpfilt_act_func wimpfilt_msg_end;
static wimpfilt_act_func wimpfilt_msg_suppress;

// Structures for state machine actions
typedef struct
{
    wimpfilt_act_func *msg;
    wimpfilt_act_func *ack;
} wimpfilt_states_record;
typedef wimpfilt_states_record wimpfilt_states_ops[WIMPFILT_OPS];
typedef wimpfilt_states_ops wimpfilt_states_actions[WIMPFILT_ACTIONS];

// Actions if not intercepted (only Capture operation defined)
static const wimpfilt_states_actions wimpfilt_states_none =
{
    // Restart transfer
    {
        {NULL,                          NULL}                       // Start
    },
    // Message_DataSave
    {
        {wimpfilt_start_save,           NULL}                       // Start
    },
    // Message_DataSaveAck
    {
        {NULL,                          NULL}                       // Start
    },
    // Message_DataLoad
    {
        {wimpfilt_start_load,           NULL}                       // Start
    },
    // Message_DataLoadAck
    {
        {NULL,                          NULL}                       // Start
    },
    // Message_DataOpen
    {
        {wimpfilt_start_run_all,        wimpfilt_start_run_unclaimed}// Start
    },
    // Message_RAMFetch
    {
        {NULL,                          NULL}                       // Start
    }
};

// Actions if intercepted load
static const wimpfilt_states_actions wimpfilt_states_load =
{
    // Restart transfer
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_restart_load,         wimpfilt_msg_end},          // Restart
        {wimpfilt_replace_load,         wimpfilt_msg_end},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataSave
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataSaveAck
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataLoad
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {NULL,                          wimpfilt_msg_end},          // Restart
        {NULL,                          wimpfilt_msg_end},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataLoadAck
    {
        {wimpfilt_msg_ready,            wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_end,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_end,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataOpen
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_RAMFetch
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    }
};

// Actions if intercepted save or transfer
static const wimpfilt_states_actions wimpfilt_states_save =
{
    // Restart transfer
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_restart_save,         wimpfilt_msg_end},          // Restart
        {wimpfilt_restart_save,         wimpfilt_msg_end},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataSave
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {NULL,                          wimpfilt_msg_end},          // Restart
        {NULL,                          wimpfilt_msg_end},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataSaveAck
    {
        {NULL,                          wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_copy_save,            wimpfilt_msg_bad},          // Restart
        {wimpfilt_copy_save,            wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataLoad
    {
        {wimpfilt_ready_save,           wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {NULL,                          wimpfilt_msg_end},          // Restart
        {NULL,                          wimpfilt_msg_end},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataLoadAck
    {
        {wimpfilt_msg_ready,            wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_done_save,            wimpfilt_msg_bad},          // Restart
        {wimpfilt_done_save,            wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataOpen
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_RAMFetch
    {
        {wimpfilt_msg_suppress,         wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_suppress,         NULL},                      // Restart
        {wimpfilt_msg_suppress,         NULL},                      // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    }
};

// Actions if intercepted run all
static const wimpfilt_states_actions wimpfilt_states_run_all =
{
    // Restart transfer
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_restart_run,          wimpfilt_msg_end},          // Restart
        {wimpfilt_replace_run,          wimpfilt_msg_end},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataSave
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataSaveAck
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataLoad
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataLoadAck
    {
        {wimpfilt_msg_ready,            wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_end,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_end,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataOpen
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {NULL,                          wimpfilt_fail_run},         // Restart
        {NULL,                          wimpfilt_fail_run},         // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_RAMFetch
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    }
};

// Actions if intercepted run unclaimed
static const wimpfilt_states_actions wimpfilt_states_run_unclaimed =
{
    // Restart transfer
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_restart_run,          wimpfilt_msg_end},          // Restart
        {wimpfilt_replace_run,          wimpfilt_msg_end},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataSave
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataSaveAck
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataLoad
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataLoadAck
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_end,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_end,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_DataOpen
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {NULL,                          wimpfilt_fail_run},         // Restart
        {NULL,                          wimpfilt_fail_run},         // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    },
    // Message_RAMFetch
    {
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Capture
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Ready
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Idle
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Restart
        {wimpfilt_msg_bad,              wimpfilt_msg_bad},          // Replace
        {wimpfilt_msg_bad,              wimpfilt_msg_bad}           // Done
    }
};

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Copy the file associated with the specified intercept and set
                  the file type appropriately.
*/
static os_error *wimpfilt_copy(wimpfilt_intercept *intercept)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept) err = &err_bad_parms;
    else
    {
        // Copy the file
        err = xosfscontrol_copy(intercept->xfer.file_name, intercept->temp,
                                osfscontrol_COPY_RECURSE
                                | osfscontrol_COPY_FORCE
                                | osfscontrol_COPY_LOOK,
                                0, 0, 0, 0, NULL);

        // Set the file type
        if (!err) err = xosfile_set_type(intercept->temp,
                                         intercept->xfer.file_type);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : xfer          - The data transfer details.
                  mask          - The type of intercept.
                  intercept     - Variable to receive pointer to intercept
                                  data structure, or NULL if not intercepted.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Start an intercept for the specified message exchange if
                  appropriate.
*/
static os_error *wimpfilt_start(wimp_message_data_xfer *xfer,
                                psifs_intercept_type mask,
                                wimpfilt_intercept **intercept)
{
    os_error *err = NULL;

    // Check parameters
    if (!mask || !intercept) err = &err_bad_parms;
    else
    {
        wimpfilt_claimed *claimed = wimpfilt_claimed_head;
        bool enable;
        bool disable;

        // Initially no intercept
        *intercept = NULL;

        // Check for special keys
        err = xosbyte1(osbyte_IN_KEY, WIMPFILT_KEY_ENABLE, 0xff, &enable);
        if (!err) err = xosbyte1(osbyte_IN_KEY, WIMPFILT_KEY_DISABLE, 0xff, &disable);
        if (!err && !disable) err = xosbyte1(osbyte_IN_KEY, WIMPFILT_KEY_DISABLE2, 0xff, &disable);

        // Update the intercept mask as appropriate
        if (!err && enable) mask |= psifs_INTERCEPT_FORCED;
        if (!err && disable) mask = 0;

        // Attempt to find a matching claim
        while (!err && claimed
               && ((claimed->type != xfer->file_type)
                   || !(claimed->mask & mask)))
        {
            claimed = claimed->next;
        }

        // Force an intercept if necessary
        if (!err && !claimed && enable) claimed = wimpfilt_claimed_head;

        // Decide whether the file should be intercepted
        if (!err && claimed)
        {
            // Allocate a new structure
            *intercept = (wimpfilt_intercept *) MEM_MALLOC(sizeof(wimpfilt_intercept));
            if (!*intercept) err = &err_buffer;

            // Ensure that the temporary directory exists
            if (!err) err = xosfile_create_dir(WIMPFILT_TEMP_DIR, 0);
            if (!err) err = xosfile_create_dir(WIMPFILT_TEMP_SUBDIR, 0);

            // Generate a unique filename
            if (!err) sprintf((*intercept)->temp, WIMPFILT_TEMP_PATH "DT%08X", (bits) *intercept);

            // Set the initial fields
            if (!err)
            {
                // Allocate a unique handle
                wimpfilt_next_handle++;
                if (wimpfilt_next_handle == psifs_INTERCEPT_INVALID)
                {
                    wimpfilt_next_handle++;
                }

                // Fill in the details of the structure
                (*intercept)->handle = wimpfilt_next_handle;
                (*intercept)->pollword = claimed->pollword;
                (*intercept)->sender = wimp_BROADCAST;
                (*intercept)->receiver = wimp_BROADCAST;
                (*intercept)->ref = 0;
                (*intercept)->saved_ref = 0;
                (*intercept)->xfer = *xfer;
                (*intercept)->mask = mask;
                (*intercept)->op = WIMPFILT_CAPTURE;
                (*intercept)->delay = 0;
            }

            // Link the structure in to the list
            if (!err)
            {
                (*intercept)->next = wimpfilt_intercept_head;
                (*intercept)->prev = NULL;
                if (wimpfilt_intercept_head)
                {
                    wimpfilt_intercept_head->prev = *intercept;
                }
                wimpfilt_intercept_head = *intercept;
            }

            // Release the memory if an error produced
            if (err && *intercept)
            {
                MEM_FREE(*intercept);
                *intercept = NULL;
            }
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : End a previous intercept. This releases any memory claimed
                  and deletes any temporary files.
*/
static os_error *wimpfilt_end(wimpfilt_intercept *intercept)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept) err = &err_bad_parms;
    else
    {
        // Delete any temporary file
        xosfscontrol_wipe(intercept->temp, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);

        // Unlink the structure
        if (intercept->next) intercept->next->prev = intercept->prev;
        if (intercept->prev) intercept->prev->next = intercept->next;
        else wimpfilt_intercept_head = intercept->next;

        // Free the memory
        MEM_FREE(intercept);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : End a previous intercept. This delays to allow the file to
                  be loaded before releasinm any memory claimed and deleting
                  any temporary files.
*/
static os_error *wimpfilt_end_delayed(wimpfilt_intercept *intercept)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept) err = &err_bad_parms;
    else
    {
        // Set the intercept status
        intercept->op = WIMPFILT_DONE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Mark the specified intercept as ready to control.
*/
static os_error *wimpfilt_ready(wimpfilt_intercept *intercept)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || (intercept->op != WIMPFILT_CAPTURE)) err = &err_bad_parms;
    else
    {
        // Set the intercept status
        intercept->op = WIMPFILT_READY;

        // Update any relevant pollwords
        if (!err) err = pollword_update(psifs_MASK_INTERCEPT_STATUS);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle of the intercept to find.
                  intercept     - Variable to receive a pointer to the
                                  intercept.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the specified intercept by handle.
*/
static os_error *wimpfilt_find_handle(psifs_intercept_handle handle,
                                      wimpfilt_intercept **intercept)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept) err = &err_bad_parms;
    else
    {
        // Attempt to find the handle
        *intercept = wimpfilt_intercept_head;
        while (*intercept && ((*intercept)->handle != handle))
        {
            *intercept = (*intercept)->next;
        }

        // Set an error if not found
        if (!*intercept) err = &err_bad_intercept_handle;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : ref           - The message reference number.
                  intercept     - Variable to receive a pointer to the
                                  intercept.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Find the specified intercept by reference.
*/
static os_error *wimpfilt_find_ref(int ref, wimpfilt_intercept **intercept)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept) err = &err_bad_parms;
    else
    {
        // Attempt to find the reference number
        *intercept = wimpfilt_intercept_head;
        while (*intercept && (!(*intercept)->ref || ((*intercept)->ref != ref)))
        {
            *intercept = (*intercept)->next;
        }

        // Set an error if not found
        if (!*intercept) err = &err_bad_intercept_handle;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a Message_DataLoad user message to start an intercept
                  for file loads.
*/
static os_error *wimpfilt_start_load(wimpfilt_intercept *intercept,
                                     wimp_event_no *event,
                                     wimp_message *message, wimp_t task,
                                     bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!message->your_ref)
    {
        // Attempt to create a new intercept
        err = wimpfilt_start(&message->data.data_xfer,
                             psifs_INTERCEPT_LOAD, &intercept);
        if (!err && intercept)
        {
            // Store the sender and receiver handles
            intercept->sender = message->sender;
            intercept->receiver = task;

            // Acknowledge the message to claim the load
            message->your_ref = message->my_ref;
            message->action = message_DATA_LOAD_ACK;
            err = xwimp_send_message(wimp_USER_MESSAGE, message,
                                     message->sender);
            if (!err) intercept->ref = message->my_ref;

            // Attempt to copy the file and set the filetype
            if (!err) err = wimpfilt_copy(intercept);

            // Suppress the event
            if (!err) *event = -1;

            // End the intercept if an error was produced
            if (err) wimpfilt_end(intercept);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a restart user message for an intercepted load.
*/
static os_error *wimpfilt_restart_load(wimpfilt_intercept *intercept,
                                       wimp_event_no *event,
                                       wimp_message *message, wimp_t task,
                                       bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->sender, task)) err = &err_bad_intercept_msg;
    else
    {
        // Send a Message_DataLoad to load the original file
        message->your_ref = 0;
        message->action = message_DATA_LOAD;
        message->data.data_xfer = intercept->xfer;
        message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                              - (char *) message + 1);
        err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, message,
                                 intercept->receiver);
        if (!err)
        {
            // Store the new reference and claim the event
            intercept->ref = message->my_ref;
            *event = -1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a replace user message for an intercepted load.
*/
static os_error *wimpfilt_replace_load(wimpfilt_intercept *intercept,
                                       wimp_event_no *event,
                                       wimp_message *message, wimp_t task,
                                       bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->sender, task)) err = &err_bad_intercept_msg;
    else
    {
        // Send a Message_DataLoad to load the replacement file
        message->your_ref = 0;
        message->action = message_DATA_LOAD;
        message->data.data_xfer = intercept->xfer;
        strcpy(message->data.data_xfer.file_name, intercept->temp);
        message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                              - (char *) message + 1);
        err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, message,
                                 intercept->receiver);
        if (!err)
        {
            // Store the new reference and claim the event
            intercept->ref = message->my_ref;
            *event = -1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a Message_DataSave user message to start an
                  intercept for file saves or transfers between applications.
*/
static os_error *wimpfilt_start_save(wimpfilt_intercept *intercept,
                                     wimp_event_no *event,
                                     wimp_message *message, wimp_t task,
                                     bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!message->your_ref)
    {
        char *name;

        // Check if the target is likely to be the filer
        err = xtaskmanager_task_name_from_handle(task, &name);

        // Attempt to create a new intercept
        if (!err)
        {
            err = wimpfilt_start(&message->data.data_xfer,
                                 ctrl_strcmp(name, "Filer")
                                 ? psifs_INTERCEPT_TRANSFER
                                 : psifs_INTERCEPT_SAVE,
                                 &intercept);
        }
        if (!err && intercept)
        {
            // Store the sender and receiver handles
            intercept->sender = message->sender;
            intercept->receiver = task;

            // Store the reference for a possible future Message_DataSaved
            intercept->saved_ref = message->my_ref;

            // Acknowledge the message to specify the filename for the save
            if (!err)
            {
                message->your_ref = message->my_ref;
                message->action = message_DATA_SAVE_ACK;
                message->data.data_xfer.est_size = -1;
                strcpy(message->data.data_xfer.file_name, intercept->temp);
                message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                                      - (char *) message + 1);
                err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, message,
                                         message->sender);
            }
            if (!err) intercept->ref = message->my_ref;

            // Suppress the event
            if (!err) *event = -1;

            // End the intercept if an error was produced
            if (err) wimpfilt_end(intercept);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a Message_DataSaveAck user message to save the
                  original or replacement file.
*/
static os_error *wimpfilt_copy_save(wimpfilt_intercept *intercept,
                                    wimp_event_no *event,
                                    wimp_message *message, wimp_t task,
                                    bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->sender, task)) err = &err_bad_intercept_msg;
    else
    {
        // Clear the saved reference if not safe
        if (message->data.data_xfer.est_size == -1) intercept->saved_ref = 0;

        // Copy the file
        err = xosfscontrol_copy(intercept->temp,
                                message->data.data_xfer.file_name,
                                osfscontrol_COPY_RECURSE
                                | osfscontrol_COPY_FORCE
                                | osfscontrol_COPY_LOOK,
                                0, 0, 0, 0, NULL);

        // Send a Message_DataLoad to load the copied file
        if (!err)
        {
            message->your_ref = message->my_ref;
            message->action = message_DATA_LOAD;
            err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, message,
                                     intercept->receiver);
        }
        if (!err)
        {
            // Store the new reference and claim the event
            intercept->ref = message->my_ref;
            *event = -1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a Message_DataLoad user message to complete the
                  intercept for a file save.
*/
static os_error *wimpfilt_ready_save(wimpfilt_intercept *intercept,
                                    wimp_event_no *event,
                                    wimp_message *message, wimp_t task,
                                    bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->receiver, task)) err = &err_bad_intercept_msg;
    else
    {
        // Acknowledge the message to claim the load
        message->your_ref = message->my_ref;
        message->action = message_DATA_LOAD_ACK;
        err = xwimp_send_message(wimp_USER_MESSAGE, message,
                                 intercept->sender);
        if (!err)
        {
            // Store the new reference and claim the event
            intercept->ref = message->my_ref;
            *event = -1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a Message_DataLoadAck user message to send a data
                  safe message to the original sender if appropriate.
*/
static os_error *wimpfilt_done_save(wimpfilt_intercept *intercept,
                                    wimp_event_no *event,
                                    wimp_message *message, wimp_t task,
                                    bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->sender, task)) err = &err_bad_intercept_msg;
    else
    {
        // Send a Message_DataSaved message if appropriate
        if (intercept->saved_ref)
        {
            message->size = ALIGN(offsetof(wimp_message, data));
            message->your_ref = intercept->saved_ref;
            message->action = message_DATA_SAVED;
            err = xwimp_send_message(wimp_USER_MESSAGE, message,
                                     intercept->receiver);
            if (!err) intercept->ref = message->my_ref;
        }

        // The operation is complete
        if (!err)
        {
            *event = -1;
            *done = TRUE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a restart or replace user message for an intercepted
                  save.
*/
static os_error *wimpfilt_restart_save(wimpfilt_intercept *intercept,
                                       wimp_event_no *event,
                                       wimp_message *message, wimp_t task,
                                       bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->sender, task)) err = &err_bad_intercept_msg;
    else
    {
        // Send a Message_DataSave to save the temporary file
        message->your_ref = 0;
        message->action = message_DATA_SAVE;
        message->data.data_xfer = intercept->xfer;
        message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                              - (char *) message + 1);
        err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, message,
                                 intercept->receiver);
        if (!err)
        {
            // Store the new reference and claim the event
            intercept->ref = message->my_ref;
            *event = -1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a Message_DataOpen user message to start an intercept
                  for all runs.
*/
static os_error *wimpfilt_start_run_all(wimpfilt_intercept *intercept,
                                        wimp_event_no *event,
                                        wimp_message *message, wimp_t task,
                                        bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!message->your_ref)
    {
        // Attempt to create a new intercept
        err = wimpfilt_start(&message->data.data_xfer,
                             psifs_INTERCEPT_RUN_ALL, &intercept);
        if (!err && intercept)
        {
            // Store the sender handle
            intercept->sender = message->sender;

            // Acknowledge the message to claim the load
            message->your_ref = message->my_ref;
            message->action = message_DATA_LOAD_ACK;
            err = xwimp_send_message(wimp_USER_MESSAGE, message,
                                     message->sender);
            if (!err) intercept->ref = message->my_ref;

            // Attempt to copy the file and set the filetype
            if (!err) err = wimpfilt_copy(intercept);

            // Suppress the event
            if (!err) *event = -1;

            // End the intercept if an error was produced
            if (err) wimpfilt_end(intercept);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a Message_DataOpen user messages acknowledgement to
                  start an intercept for unclaimed runs.
*/
static os_error *wimpfilt_start_run_unclaimed(wimpfilt_intercept *intercept,
                                              wimp_event_no *event,
                                              wimp_message *message,
                                              wimp_t task, bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!message->your_ref)
    {
        // Attempt to create a new intercept
        err = wimpfilt_start(&message->data.data_xfer,
                             psifs_INTERCEPT_RUN_UNCLAIMED, &intercept);
        if (!err && intercept)
        {
            // Store the sender handle
            intercept->sender = message->sender;

            // Modify message to claim the load
            *event = wimp_USER_MESSAGE;
            message->your_ref = message->my_ref;
            message->action = message_DATA_LOAD_ACK;

            // Attempt to copy the file and set the filetype
            if (!err) err = wimpfilt_copy(intercept);

            // Update the status
            if (!err) err = wimpfilt_ready(intercept);

            // End the intercept if an error was produced
            if (err) wimpfilt_end(intercept);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a restart user message for an intercepted run.
*/
static os_error *wimpfilt_restart_run(wimpfilt_intercept *intercept,
                                      wimp_event_no *event,
                                      wimp_message *message, wimp_t task,
                                      bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->sender, task)) err = &err_bad_intercept_msg;
    else
    {
        // Broadcast a Message_DataOpen to run the original file
        message->your_ref = 0;
        message->action = message_DATA_OPEN;
        message->data.data_xfer = intercept->xfer;
        message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                              - (char *) message + 1);
        err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, message,
                                 wimp_BROADCAST);
        if (!err)
        {
            // Store the new reference and claim the event
            intercept->ref = message->my_ref;
            *event = -1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle a replace user message for an intercepted run.
*/
static os_error *wimpfilt_replace_run(wimpfilt_intercept *intercept,
                                      wimp_event_no *event,
                                      wimp_message *message, wimp_t task,
                                      bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->sender, task)) err = &err_bad_intercept_msg;
    else
    {
        // Broadcast a Message_DataOpen to run the replacement file
        message->your_ref = 0;
        message->action = message_DATA_OPEN;
        message->data.data_xfer = intercept->xfer;
        strcpy(message->data.data_xfer.file_name, intercept->temp);
        message->size = ALIGN(strchr(message->data.data_xfer.file_name, '\0')
                              - (char *) message + 1);
        err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED, message,
                                 wimp_BROADCAST);
        if (!err)
        {
            // Store the new reference and claim the event
            intercept->ref = message->my_ref;
            *event = -1;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle failure to run file by generating an appropriate error.
*/
static os_error *wimpfilt_fail_run(wimpfilt_intercept *intercept,
                                   wimp_event_no *event,
                                   wimp_message *message, wimp_t task,
                                   bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else if (!util_eq_task(intercept->sender, task)) err = &err_bad_intercept_msg;
    else err = &err_intercept_run;

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle user messages or acknowledgements that complete an
                  intercept ready for external processing.
*/
static os_error *wimpfilt_msg_ready(wimpfilt_intercept *intercept,
                                  wimp_event_no *event,
                                  wimp_message *message, wimp_t task,
                                  bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else err = wimpfilt_ready(intercept);

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle unexpected user messages or acknowledgements by
                  generating an appropriate error.
*/
static os_error *wimpfilt_msg_bad(wimpfilt_intercept *intercept,
                                  wimp_event_no *event,
                                  wimp_message *message, wimp_t task,
                                  bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else err = &err_bad_intercept_msg;

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle user messages or acknowledgements that end the current
                  intercept without error.
*/
static os_error *wimpfilt_msg_end(wimpfilt_intercept *intercept,
                                  wimp_event_no *event,
                                  wimp_message *message, wimp_t task,
                                  bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else
    {
        // Suppress the event and end the intercept
        *event = -1;
        *done = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : intercept     - The previously allocated intercept data
                                  structure.
                  event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
                  done          - Variable to receive whether the operation has
                                  completed.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle user messages or acknowledgements that need to be
                  suppressed.
*/
static os_error *wimpfilt_msg_suppress(wimpfilt_intercept *intercept,
                                       wimp_event_no *event,
                                       wimp_message *message, wimp_t task,
                                       bool *done)
{
    os_error *err = NULL;

    // Check parameters
    if (!intercept || !event || !message || !done) err = &err_bad_parms;
    else *event = -1;

    // Return any error produced
    return err;
}

/*
    Parameters  : message       - Pointer to the message as returned by
                                  Wimp_Poll.
                  intercept     - Variable to receive a pointer to the matching
                                  intercept data structure.
                  record        - Variable to receive a pointer to the
                                  appropriate state machine record.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Choose the action corresponding to the specified message.
*/
static os_error *wimpfilt_choose_action(wimp_message *message,
                                        wimpfilt_intercept **intercept,
                                        const wimpfilt_states_record **record)
{
    os_error *err = NULL;

    // Check parameters
    if (!message || !intercept || !record) err = &err_bad_parms;
    else
    {
        const wimpfilt_states_ops *actions = wimpfilt_states_none;
        bits action = 0;
        bits op = 0;

        // Attempt to find a matching intercept
        if (wimpfilt_find_ref(message->my_ref, intercept))
        {
            if (wimpfilt_find_ref(message->your_ref, intercept))
            {
                *intercept = NULL;
            }
        }

        // Select the actions structure
        if (*intercept)
        {
            // Structure depends on the intercept type
            switch ((*intercept)->mask & WIMPFILT_MASK)
            {
                case psifs_INTERCEPT_LOAD:
                    // Load intercepted
                    actions = wimpfilt_states_load;
                    break;

                case psifs_INTERCEPT_SAVE:
                case psifs_INTERCEPT_TRANSFER:
                    // Save or transfer between applications intercepted
                    actions = wimpfilt_states_save;
                    break;

                case psifs_INTERCEPT_RUN_ALL:
                    // All runs intercepted
                    actions = wimpfilt_states_run_all;
                    break;

                case psifs_INTERCEPT_RUN_UNCLAIMED:
                    // Unclaimed run intercepted
                    actions = wimpfilt_states_run_unclaimed;
                    break;

                default:
                    // There should be no other intercept types
                    err = &err_bad_intercept_type;
                    break;
            }
        }

        // Select the operations data structure
        while (!err && (action < WIMPFILT_ACTIONS)
               && (wimpfilt_actions[action] != message->action))
        {
            action++;
        }

        // Select the operation
        if (!err && *intercept) op = (*intercept)->op;
        if (WIMPFILT_OPS <= op) err = &err_bad_intercept_state;

        // Set the return value
        *record = !err && (action < WIMPFILT_ACTIONS)
                  ? &actions[action][op] : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : void
    Description : Perform any necessary idle processing.
*/
static os_error *wimpfilt_idle(void)
{
    os_error *err = NULL;
    wimpfilt_intercept *intercept = wimpfilt_intercept_head;
    os_t now = util_time();

    // Loop through all intercepts
    while (!err && intercept)
    {
        // Check if any action is required for this intercept
        wimpfilt_intercept *next = intercept->next;
        if (intercept->op == WIMPFILT_DONE)
        {
            // Check if it is time to delete the intercept
            if (!intercept->delay) intercept->delay = now + WIMPFILT_DELAY_FILE;
            else if (0 < (now - intercept->delay))
            {
                err = wimpfilt_end(intercept);
            }
        }
        intercept = next;
    }

    // Attempt to delete the temporary directory if no active intercepts
    if (!err && !wimpfilt_intercept_head
        && (!wimpfilt_delay_wipe || (0 < (now - wimpfilt_delay_wipe))))
    {
        // Attempt to delete the temporary directory
        xosfscontrol_wipe(WIMPFILT_TEMP_SUBDIR, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);

        // Schedule the next temporary directory deletion
        wimpfilt_delay_wipe = now + WIMPFILT_DELAY_DIR;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle received user messages.
*/
static os_error *wimpfilt_msg(wimp_event_no *event, wimp_message *message,
                              wimp_t task)
{
    os_error *err = NULL;

    // Check parameters
    if (!event || !message) err = &err_bad_parms;
    else
    {
        wimpfilt_intercept *intercept;
        const wimpfilt_states_record *record;
        bool done = FALSE;

        // Choose the action
        err = wimpfilt_choose_action(message, &intercept, &record);

        // Call the appropriate function
        if (!err && record && record->msg)
        {
            err = (*record->msg)(intercept, event, message, task, &done);
        }

        // Cancel the intercept if an error was produced or finished
        if (err && intercept) wimpfilt_end(intercept);
        else if (done && intercept) wimpfilt_end_delayed(intercept);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  message       - Pointer to the message as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new message.
                  task          - The task handle of the task that is being
                                  returned to.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handle received user message acknowledges.
*/
static os_error *wimpfilt_ack(wimp_event_no *event, wimp_message *message,
                              wimp_t task)
{
    os_error *err = NULL;

    // Check parameters
    if (!event || !message) err = &err_bad_parms;
    else
    {
        wimpfilt_intercept *intercept;
        const wimpfilt_states_record *record;
        bool done = FALSE;

        // Choose the action
        err = wimpfilt_choose_action(message, &intercept, &record);

        // Call the appropriate function
        if (!err && record && record->ack)
        {
            err = (*record->ack)(intercept, event, message, task, &done);
        }

        // Cancel the intercept if an error was produced or finished
        if (err && intercept) wimpfilt_end(intercept);
        else if (done && intercept) wimpfilt_end_delayed(intercept);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : mask          - Pointer to the event mask as passed to
                                  Wimp_Poll. This may be modified to provide
                                  a new event mask.
                  block         - Pointer to the event block as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new event.
                  task          - The task handle of the task that is being
                                  returned to.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handler for WIMP post-filter calls.
*/
os_error *wimpfilt_pre(wimp_poll_flags *mask, wimp_block *block, wimp_t task)
{
    os_error *err = NULL;

    // Check parameters
    if (!mask || !block) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Pre filter mask=%u, task=%u", *mask, (bits) task))

        // Ensure that user messages are returned
        *mask &= ~(wimp_MASK_MESSAGE | wimp_MASK_RECORDED
                   | wimp_MASK_ACKNOWLEDGE);
    }

    // Report any error to the user
    if (err) err = xwimp_report_error(err, wimp_ERROR_BOX_CANCEL_ICON, Module_Title, NULL);

    // Return any error produced
    return err;
}

/*
    Parameters  : event         - Pointer to the event reason code as returned
                                  by Wimp_Poll. This may be modified to provide
                                  a new event, or set to -1 to claim the event
                                  and prevent it being passed to the task.
                  block         - Pointer to the event block as returned by
                                  Wimp_Poll. This may be modified to provide a
                                  new event.
                  task          - The task handle of the task that is being
                                  returned to.
    Returns     : os_error *    - NULL if successful, or a pointer to an error
                                  block otherwise.
    Description : Handler for WIMP post-filter calls.
*/
os_error *wimpfilt_post(wimp_event_no *event, wimp_block *block, wimp_t task)
{
    os_error *err = NULL;

    // Check parameters
    if (!event || ((*event != wimp_NULL_REASON_CODE) && !block)) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Post filter event=%u, task=%u", *event, (bits) task))

        // Action depends on the event
        switch (*event)
        {
            case wimp_NULL_REASON_CODE:
                // The desktop is idle
                err = wimpfilt_idle();
                break;

            case wimp_USER_MESSAGE:
            case wimp_USER_MESSAGE_RECORDED:
                // A new message received
                err = wimpfilt_msg(event, &block->message, task);
                break;

            case wimp_USER_MESSAGE_ACKNOWLEDGE:
                // A message was not acknowledged or replied to
                err = wimpfilt_ack(event, &block->message, task);
                break;

            default:
                // Not interested in any other events
                break;
        }

        // Claim the event if an error was produced
        if (err) *event = -1;
    }

    // Report any error to the user
    if (err) err = xwimp_report_error(err, wimp_ERROR_BOX_CANCEL_ICON, Module_Title, NULL);

    // Return any error produced
    return err;
}

/*
    Parameters  : pollword      - The previously allocated pollword pointer.
                  type          - The file type.
                  mask          - The masks of intercept types to handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Claim an intercepted file type for the specified client.
                  If 0 is passed for the mask then the intercept is released.
*/
os_error *wimpfilt_claim(int *pollword, bits type, psifs_intercept_type mask)
{
    os_error *err = NULL;

    // Check parameters
    if (!pollword) err = &err_bad_parms;
    else
    {
        wimpfilt_claimed *claimed = wimpfilt_claimed_head;

        DEBUG_PRINTF(("Filter claim %p type=0x%03x mask=0x%08x", pollword, type, mask))

        // Attempt to find an existing match
        while (!err && claimed
               && ((claimed->pollword != pollword) || (claimed->type != type)))
        {
            claimed = claimed->next;
        }

        // Unlink any existing claim
        if (!err && claimed)
        {
            if (claimed->next) claimed->next->prev = claimed->prev;
            if (claimed->prev) claimed->prev->next = claimed->next;
            else wimpfilt_claimed_head = claimed->next;
        }

        // Action depends on whether the intercept should be released
        if (mask)
        {
            // Create a new entry if necessary
            if (!err && !claimed)
            {
                claimed = (wimpfilt_claimed *) MEM_MALLOC(sizeof(wimpfilt_claimed));
                if (!claimed) err = &err_buffer;
            }

            // Set the fields
            if (!err)
            {
                claimed->pollword = pollword;
                claimed->type = type;
                claimed->mask = mask;
            }

            // Link the entry in to the list
            if (!err)
            {
                claimed->next = wimpfilt_claimed_head;
                claimed->prev = NULL;
                if (wimpfilt_claimed_head) wimpfilt_claimed_head->prev = claimed;
               wimpfilt_claimed_head = claimed;
            }

            // Free the memory if problems
            if (err && claimed) MEM_FREE(claimed);
        }
        else
        {
            // Free any memory claimed
            if (claimed) MEM_FREE(claimed);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : pollword      - The previously allocated pollword pointer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Release any intercepts associated with the specified client.
*/
os_error *wimpfilt_release(int *pollword)
{
    os_error *err = NULL;

    // Check parameters
    if (!pollword) err = &err_bad_parms;
    else
    {
        wimpfilt_claimed *claimed = wimpfilt_claimed_head;
        wimpfilt_intercept *intercept = wimpfilt_intercept_head;

        DEBUG_PRINTF(("Filter release %p", pollword))

        // Release any claimed intercepts
        while (!err && claimed)
        {
            wimpfilt_claimed *next = claimed->next;
            if (claimed->pollword == pollword)
            {
                err = wimpfilt_claim(pollword, claimed->type, 0);
            }
            claimed = next;
        }

        // Also cancel any active intercepts for this client
        while (!err && intercept)
        {
            wimpfilt_intercept *next = intercept->next;
            if (intercept->pollword == pollword) err = wimpfilt_end(intercept);
            intercept = next;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : pollword      - The previously allocated pollword pointer.
                  handle        - Variable to receive the handle for the
                                  intercept, or 0 for none.
                  type          - Variable to receive a the file type.
                  mask          - Variable to receive the intercept type.
                  orig          - Variable to receive a pointer to the original
                                  file name.
                  temp          - Variable to receive a pointer to the name of
                                  the temporary copy of the file.
                  sender        - Variable to receive the task handle of the
                                  sending task.
                  receiver      - Variable to receive the task handle of the
                                  receiving task.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check for any new intercepted operations for the specified
                  client. This should be called until no more operations are
                  returned.
*/
os_error *wimpfilt_poll(int *pollword, psifs_intercept_handle *handle,
                        bits *type, psifs_intercept_type *mask,
                        const char **orig, const char **temp,
                        wimp_t *sender, wimp_t *receiver)
{
    os_error *err = NULL;

    // Check parameters
    if (!pollword || !handle || !type || !mask || !orig || !temp)
    {
        err = &err_bad_parms;
    }
    else
    {
        wimpfilt_intercept *intercept = wimpfilt_intercept_head;

        DEBUG_PRINTF(("Filter poll %p", pollword))

        // Attempt to find an active intercept for this client
        while (intercept
               && ((intercept->op != WIMPFILT_READY)
                   || (intercept->pollword != pollword)))
        {
            // Try the next intercept
            intercept = intercept->next;
        }

        // Change the status of the intercept if found
        if (intercept) intercept->op = WIMPFILT_IDLE;

        // Set the return values
        *handle = intercept ? intercept->handle : psifs_INTERCEPT_INVALID;
        *type = intercept ? intercept->xfer.file_type : osfile_TYPE_UNTYPED;
        *mask = intercept ? intercept->mask : 0;
        *orig = intercept ? intercept->xfer.file_name : NULL;
        *temp = intercept ? intercept->temp : NULL;
        *sender = intercept ? intercept->sender : wimp_BROADCAST;
        *receiver = intercept ? intercept->receiver : wimp_BROADCAST;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle of the intercept to restart.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Restart an intercepted operation. This must be called from
                  a WIMP task.
*/
os_error *wimpfilt_restart(psifs_intercept_handle handle)
{
    os_error *err = NULL;
    wimpfilt_intercept *intercept;

    // Attempt to find the intercept associated with the specified handle
    err = wimpfilt_find_handle(handle, &intercept);
    if (!err && (intercept->op != WIMPFILT_IDLE)) err = &err_bad_parms;
    if (!err)
    {
        static wimp_message message;

        DEBUG_PRINTF(("Filter restart %u", handle))

        // Store the operation
        intercept->op = WIMPFILT_RESTART;

        // Send a fake message to restart the transfer
        message.size = ALIGN(offsetof(wimp_message, data));
        message.your_ref = intercept->ref;
        message.action = WIMPFILT_MESSAGE_RESTART;
        err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED,
                                 &message, intercept->sender);
        if (!err) intercept->ref = message.my_ref;

        // End the operation if an error was produced
        if (err) wimpfilt_end(intercept);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle of the intercept to replace.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Replace an intercepted operation. This must be called from
                  a WIMP task.
*/
os_error *wimpfilt_replace(psifs_intercept_handle handle)
{
    os_error *err = NULL;
    wimpfilt_intercept *intercept;

    // Attempt to find the intercept associated with the specified handle
    err = wimpfilt_find_handle(handle, &intercept);
    if (!err && (intercept->op != WIMPFILT_IDLE)) err = &err_bad_parms;
    if (!err)
    {
        static wimp_message message;

        DEBUG_PRINTF(("Filter replace %u", handle))

        // Store the operation
        intercept->op = WIMPFILT_REPLACE;

        // Read the details of the replacement file
        if (!err)
        {
            err = xosfile_read_stamped_no_path(intercept->temp, NULL, NULL,
                                               NULL, &intercept->xfer.est_size,
                                               NULL,
                                               &intercept->xfer.file_type);
        }

        // Send a fake message to restart the transfer
        if (!err)
        {
            message.size = ALIGN(offsetof(wimp_message, data));
            message.your_ref = intercept->ref;
            message.action = WIMPFILT_MESSAGE_RESTART;
            err = xwimp_send_message(wimp_USER_MESSAGE_RECORDED,
                                     &message, intercept->sender);
        }
        if (!err) intercept->ref = message.my_ref;

        // End the operation if an error was produced
        if (err) wimpfilt_end(intercept);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The handle of the intercept to cancel.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Cancel an intercepted operation. This must be called from
                  a WIMP task.
*/
os_error *wimpfilt_cancel(psifs_intercept_handle handle)
{
    os_error *err = NULL;
    wimpfilt_intercept *intercept;

    // Attempt to find the intercept associated with the specified handle
    err = wimpfilt_find_handle(handle, &intercept);
    if (!err && (intercept->op != WIMPFILT_IDLE)) err = &err_bad_parms;
    if (!err)
    {
        DEBUG_PRINTF(("Filter cancel %u", handle))

        // Cancel the operation
        err = wimpfilt_end(intercept);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the WIMP post-filters.
*/
os_error *wimpfilt_status(void)
{
    os_error *err = NULL;
    wimpfilt_intercept *intercept = wimpfilt_intercept_head;

    DEBUG_PRINTF(("Displaying post filter status"))

    // Display the status of all active intercepts
    while (!err && intercept)
    {
        // Display the status of this intercept
        printf("Intercepted file '%s'", intercept->xfer.file_name);
        if (intercept->mask & psifs_INTERCEPT_FORCED)
        {
            printf(" forced");
        }
        if (intercept->mask & psifs_INTERCEPT_LOAD)
        {
            printf(" load");
        }
        if (intercept->mask & psifs_INTERCEPT_SAVE)
        {
            printf(" save");
        }
        if (intercept->mask & psifs_INTERCEPT_TRANSFER)
        {
            printf(" transfer");
        }
        if (intercept->mask & psifs_INTERCEPT_RUN_ALL)
        {
            printf(" run");
        }
        if (intercept->mask & psifs_INTERCEPT_RUN_UNCLAIMED)
        {
            printf(" unclaimed");
        }
        printf(" to '%s'", intercept->temp);
        if (intercept->op == WIMPFILT_CAPTURE) printf(" capturing");
        else if (intercept->op == WIMPFILT_IDLE) printf(" waiting");
        else if (intercept->op == WIMPFILT_RESTART) printf(" restarting");
        else if (intercept->op == WIMPFILT_REPLACE) printf(" replacing");
        else if (intercept->op == WIMPFILT_DONE) printf(" completing");
        printf("\n");

        // Advance to the next intercept
        intercept = intercept->next;
    }

    // Return any error produced
    return err;
}
