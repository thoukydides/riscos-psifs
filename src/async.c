/*
    File        : async.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Asynchronous remote operations for the PsiFS module.

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
#include "async.h"

// Include clib header files
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "cache.h"
#include "debug.h"
#include "err.h"
#include "frac.h"
#include "idle.h"
#include "mem.h"
#include "pollword.h"
#include "sysvar.h"
#include "timer.h"
#include "unified.h"
#include "util.h"
#include "wildcard.h"

// Internal operations
#define ASYNC_BACKUP_LIST ((psifs_async_op) 0x100)
#define ASYNC_BACKUP_PREV ((psifs_async_op) 0x101)
#define ASYNC_BACKUP_COPY ((psifs_async_op) 0x102)
#define ASYNC_TAR_COMPLETE ((psifs_async_op) 0x103)
#define ASYNC_FIND ((psifs_async_op) 0x104)

// Stages of processing an operation
typedef bits async_stage;
#define ASYNC_INITIALISE ((async_stage) 0x00)
#define ASYNC_PROCESS ((async_stage) 0x01)
#define ASYNC_ABORT ((async_stage) 0x02)
#define ASYNC_PRE_FINALISE ((async_stage) 0x03)
#define ASYNC_POST_FINALISE ((async_stage) 0x04)

// Data types for operations
typedef char async_detail[1024];
typedef struct async_data
{
    struct async_data *prev;
    struct async_data *next;
    struct async_data *child;
    struct async_data *parent;
    psifs_async_handle handle;
    psifs_async_status status;
    bool initialised;
    bool finalised;
    bool abort;
    bool pause;
    bool threaded;
    bool recurse;
    bool suspend;
    bool suspended;
    psifs_async_response response;
    bool quiet;
    async_detail detail;
    bool has_err;
    bool has_stored_err;
    os_error err;
    os_error stored_err;
    async_op op;
    bits time_acc;
    bits time_done;
    os_t time_start;
    bool time_running;
    frac_value frac_done;
    frac_value frac_step;
    bits pretty_total;
    bits pretty_base;
    int pretty_error;
    bits pretty_last;
    bits index;
    bits num;
    fs_info info;
    os_fw osfile;
    FILE *file;
    backtree_handle tree;
    tar_handle dest_tar;
    tar_handle prev_tar;
    tar_handle scrap_tar;
    fs_handle remote;
    union
    {
        struct
        {
            unified_cmd cmd;
            unified_reply reply;
        } unified;
        struct
        {
            cache_cmd cmd;
            cache_reply reply;
        } cache;
    } data;
    union
    {
        void *ptr;
        ncp_app *tasks;
        fs_info *info;
    } buffer;
    size_t buffer_size;
} async_data;

// List of asynchronous remote operations
static async_data *async_head = NULL;

// System variable to hold the last handle allocated
#define ASYNC_VAR_NEXT_HANDLE FS_NAME "$AsyncLastHandle"

// Application suffix
#define ASYNC_APP_EXT "/app"

// Termination of task list components
#define ASYNC_TERM_NAME ' '
#define ASYNC_TERM_ARGS '\n'

// Buffer sizes
#define ASYNC_MIN_TASKS (4)
#define ASYNC_MIN_FILES (4)
#define ASYNC_COPY_SIZE (4096)

// Delay between successive operations (centi-seconds)
#define ASYNC_TAR_DELAY (1)
#define ASYNC_CLOSE_DELAY (100)

// Weighting for different components of timing
#define ASYNC_FILE_WEIGHT (2 * 1024 * 10)
#define ASYNC_FILE_SIZE_WEIGHT (1 * 10)
#define ASYNC_TAR_WEIGHT (1024)
#define ASYNC_TAR_SIZE_WEIGHT (1)

// Settings for combining and prettifying timings
#define ASYNC_CONFIDENT_MIN (200)
#define ASYNC_CONFIDENT_MAX (500)
#define ASYNC_PRETTY_INIT (300)
#define ASYNC_PRETTY_HIDE (1000)
#define ASYNC_PRETTY_STEP (50)
#define ASYNC_PRETTY_DOWN_PERC (25)
#define ASYNC_PRETTY_DOWN_MIN_TIME (300)
#define ASYNC_PRETTY_DOWN_MAX_TIME (1000)
#define ASYNC_PRETTY_UP_PERC (50)
#define ASYNC_PRETTY_UP_MIN_TIME (200)
#define ASYNC_PRETTY_UP_MAX_TIME (1000)
#define ASYNC_PRETTY_UP_ERROR (500)
#define ASYNC_PRETTY_PERC_ERROR (25)

// Function prototypes
static os_error *async_process(async_data *data);
static os_error *async_new(async_data *parent, const async_op *op,
                           async_data **data);
static os_error *async_abort(async_data *data);

/*
    Parameters  : data          - Pointer to an operation data structure.
                  size          - The required buffer size.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Ensure that the buffer is at least the requested size.
*/
static os_error *async_buffer(async_data *data, size_t size)
{
    os_error *err = NULL;

    // Check parameters
    if (!data || !size) err = &err_bad_parms;
    else
    {
        // No action if buffer already large enough
        if (data->buffer_size < size)
        {
            void *ptr = MEM_REALLOC(data->buffer.ptr, size);
            if (ptr)
            {
                data->buffer.ptr = ptr;
                data->buffer_size = size;
            }
            else err = &err_buffer;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The name of the application.
                  dest          - Variable to receive a pointer to the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Generate a tidied version of the specified application name.
*/
static os_error *async_tidy_app(const char *src, const char **dest)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        static fs_pathname path;

        // Copy the string if possible
        if (strlen(src) < sizeof(path))
        {
            char *ptr;

            // Copy the application name
            strcpy(path, src);

            // Start by removing any process number
            ptr = strrchr(path, '$');
            if (ptr && isdigit(ptr[1])) *ptr = '\0';

            // Remove any trailing period
            ptr = strrchr(path, '.');
            if (ptr && !ptr[1]) *ptr = '\0';

            // Remove any application extension
            ptr = strchr(path, '\0');
            if (path + strlen(ASYNC_APP_EXT) <= ptr)
            {
                ptr -= strlen(ASYNC_APP_EXT);
                if (!wildcard_cmp(ASYNC_APP_EXT, ptr)) *ptr = '\0';
            }

            // Skip any initial path
            ptr = strrchr(path, FS_CHAR_SEPARATOR);
            *dest = ptr && ptr[1] ? ptr + 1 : path;
        }
        else
        {
            // Use the name without removing any suffix
            *dest = src;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The arguments string.
                  dest          - Variable to receive a pointer to the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Generate a tidied version of the specified arguments string.
*/
static os_error *async_tidy_args(const char *src, const char **dest)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest) err = &err_bad_parms;
    else
    {
        static fs_pathname path;

        // Prefix with the filing system name if appropriate
        if ((src[0] == FS_CHAR_DISC)
            && (strlen(FS_NAME) + 1 + strlen(src) < sizeof(path)))
        {
            // Prefix the filing system name
            sprintf(path, "%s%c%s", FS_NAME, FS_CHAR_DISC, src);
            *dest = path;
        }
        else
        {
            // Use the name unchanged
            *dest = src;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  app           - The name of the application.
                  args          - The command line arguments.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Set the detail string for an operation involving an
                  application.
*/
static os_error *async_detail_app(async_data *data, const char *app,
                                  const char *args)
{
    os_error *err = NULL;

    // Check parameters
    if (!data || !app || !args) err = &err_bad_parms;
    else
    {
        // Tidy up the application name and arguments
        err = async_tidy_app(app, &app);
        if (!err) err = async_tidy_args(args, &args);

        // Generate the detailed description
        if (!err)
        {
            if (strcmp(app, args)) sprintf(data->detail, "%s %s", app, args);
            else strcpy(data->detail, app);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : file  - The file to read from.
                  term  - The termination character.
                  str   - Variable to receive the result.
                  size  - The size of the destination buffer.
    Returns     : bool  - Was a line read successfully.
    Description : Attempt to read a line from the specified file. This can fail
                  if either the end of file is reached or the line is too long.
*/
static bool async_getline(FILE *file, char term, char *str, size_t size)
{
    int ch;

    // Keep reading characters until finished
    ch = fgetc(file);
    while ((ch != EOF) && (ch != term) && (1 < size))
    {
        // Add the character to the string
        *str++ = ch;
        size--;

        // Read the next character from the file
        ch = fgetc(file);
    }

    // Terminate the resulting string
    *str = '\0';

    // Return whether a string was read
    return ch == term;
}

/*
    Parameters  : data  - Pointer to an operation data structure.
    Returns     : bool  - Has the operation finished.
    Description : Check whether the specified operation has finished.
*/
static bool async_done(const async_data *data)
{
    // Return whether the operation has finished
    return (data->status == psifs_ASYNC_SUCCESS)
           || (data->status == psifs_ASYNC_ERROR)
           || (data->status == psifs_ASYNC_ABORTED);
}

/*
    Parameters  : data  - Pointer to an operation data structure.
    Returns     : bool  - Is the operation idle.
    Description : Check whether the specified operation is idle.
*/
static bool async_idle(const async_data *data)
{
    // Return whether the operation is idle
    return data->status == psifs_ASYNC_BUSY;
}

/*
    Parameters  : data  - Pointer to an operation data structure.
    Returns     : bool  - Is the operation idle.
    Description : Check whether the specified operation is paused.
*/
static bool async_paused(const async_data *data)
{
    // Return whether the operation is paused
    return data->status == psifs_ASYNC_PAUSED;
}

/*
    Parameters  : data  - Pointer to an operation data structure.
    Returns     : bool  - Is the operation waiting for a response.
    Description : Check whether the specified operation is waiting for a
                  response.
*/
static bool async_waiting(const async_data *data)
{
    // Return whether the operation is waiting for a response
    return (data->status == psifs_ASYNC_WAIT_COPY)
           || (data->status == psifs_ASYNC_WAIT_RESTART)
           || (data->status == psifs_ASYNC_WAIT_NEWER)
           || (data->status == psifs_ASYNC_WAIT_READ);
}

/*
    Parameters  : handle        - The previously allocated handle.
                  data          - Variable to receive a pointer to the data
                                  for this operation.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Attempt to locate the data associated with the specified
                  handle.
*/
static os_error *async_find(psifs_async_handle handle, async_data **data)
{
    os_error *err = NULL;
    async_data *ptr = async_head;

    // Attempt to find the specified handle
    while (ptr && (ptr->handle != handle)) ptr = ptr->next;

    // Set the return values
    if (data) *data = ptr;
    if (!ptr) err = &err_bad_async_handle;

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  status        - Variable to receive the status.
                  desc          - Variable to receive a pointer to the
                                  description of the status.
                  detail        - Variable to receive a pointer to the
                                  details of the status.
                  error         - Variable to receive a pointer to any
                                  error text.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Generate a description for the specified operation.
*/
static os_error *async_desc(async_data *data, psifs_async_status *status,
                            const char **desc, const char **detail,
                            const char **error)
{
    os_error *err = NULL;

    // Check parameters
    if (!data || !status || !desc || !detail) err = &err_bad_parms;
    else
    {
        static char str[32];
        static async_detail copy;

        // Initially no descriptions
        *desc = NULL;
        *detail = NULL;
        *error = NULL;

        // Copy the status
        *status = data->status;

        // Attempt to map the status
        switch (*status)
        {
            case psifs_ASYNC_SUCCESS:
                // Completed successfully
                *desc = "Completed successfully";
                break;

            case psifs_ASYNC_ERROR:
                // Completed with an error
                *desc = "Aborted due to error";
                *error = data->has_err ? data->err.errmess : "Unknown error";
                break;

            case psifs_ASYNC_ABORTED:
                // Operation being processed
                *desc = "Aborted";
                break;

            case psifs_ASYNC_BUSY:
                // Operation being processed
                *desc = "Processing";
                break;

            case psifs_ASYNC_DELEGATE:
                // Operation being delegated
                *desc = "Delegating";
                break;

            case psifs_ASYNC_WAIT_COPY:
                // Waiting for a yes or no response
                *desc = "Copy file";
                *detail = data->detail;
                break;

            case psifs_ASYNC_WAIT_RESTART:
                // Waiting for a skip or retry response
                *desc = "Error when starting";
                *detail = data->detail;
                *error = data->err.errmess;
                break;

            case psifs_ASYNC_WAIT_NEWER:
                // Waiting for a skip or copy response
                *desc = "Previous backup is newer";
                *detail = data->detail;
                break;

            case psifs_ASYNC_WAIT_READ:
                // Waiting for a skip or retry response
                *desc = "Error when reading";
                *detail = data->detail;
                *error = data->err.errmess;
                break;

            case psifs_ASYNC_PAUSED:
                // Operation paused
                *desc = "Paused";
                break;

            case psifs_ASYNC_PROG_LIST:
                // Reading list of tasks
                *desc = "Enumerating open files";
                break;

            case psifs_ASYNC_PROG_DETAIL:
                // Reading command line for a task
                *desc = "Reading command line";
                *detail = data->detail;
                break;

            case psifs_ASYNC_PROG_CLOSE:
                // Closing task
                *desc = "Terminating";
                *detail = data->detail;
                break;

            case psifs_ASYNC_PROG_OPEN:
                // Opening task
                *desc = "Starting";
                *detail = data->detail;
                break;

            case psifs_ASYNC_FILE_OPEN:
                // Opening file
                *desc = "Opening";
                *detail = data->detail;
                break;

            case psifs_ASYNC_FILE_CLOSE:
                // Closing file
                *desc = "Closing";
                *detail = data->detail;
                break;

            case psifs_ASYNC_FILE_READ:
                // Reading file
                *desc = "Reading";
                *detail = data->detail;
                break;

            case psifs_ASYNC_FILE_MKDIR:
                // Creating directory
                *desc = "Creating directory";
                *detail = data->detail;
                break;

            case psifs_ASYNC_FILE_DELETE:
                // Deleting file
                *desc = "Deleting";
                *detail = data->detail;
                break;

            case psifs_ASYNC_FILE_WRITE:
                // Writing file
                *desc = "Writing";
                *detail = data->detail;
                break;

            case psifs_ASYNC_CAT_READ:
                // Reading catalogue entry
                *desc = "Reading details";
                *detail = data->detail;
                break;

            case psifs_ASYNC_CAT_WRITE:
                // Writing catalogue entry
                *desc = "Writing details";
                *detail = data->detail;
                break;

            case psifs_ASYNC_TAR_KEEP:
                // Copying between tar files
                *desc = "Keeping";
                *detail = data->detail;
                break;

            case psifs_ASYNC_TAR_SCRAP:
                // Copying file to backup scrap file
                *desc = "Scrapping";
                *detail = data->detail;
                break;

            case psifs_ASYNC_TAR_SKIP:
                // Skipping unused files from backup
                *desc = "Skipping";
                *detail = data->detail;
                break;

            case psifs_ASYNC_TAR_ADD:
                // Adding to tar files
                *desc = "Adding";
                *detail = data->detail;
                break;

            case psifs_ASYNC_TAR_EXTRACT:
                // Extracting from tar files
                *desc = "Extracting";
                *detail = data->detail;
                break;

            default:
                // Unknown status
                sprintf(str, "Unknown status %u", data->status);
                *desc = str;
                break;
        }

        // Copy the detail string if appropriate
        if (*detail && (strlen(*detail) < sizeof(copy)))
        {
            strcpy(copy, *detail);
            *detail = copy;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : a         - The first value.
                  conf_a    - The confidence in the first value.
                  b         - The second value.
                  conf_b    - The confidence in the second value.
    Returns     : bits      - The combined result.
    Description : Combine the two values, taking their relative confidence
                  into account. Zero values are assumed to be invalid.
*/
bits async_combine(bits a, frac_value conf_a, bits b, frac_value conf_b)
{
    // Treat zero values as invalid
    if (!a) conf_a = 0;
    if (!b) conf_b = 0;

    // Return the result
    return frac_inv_scale(frac_scale(a, conf_a) + frac_scale(b, conf_b),
                          frac_add(conf_a, conf_b));
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  taken         - Variable to receive the number of
                                  centi-seconds taken so far.
                  remain        - Variable to receive an estimate of the number
                                  of centi-seconds remaining.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Recursively calculate the timing information for the
                  operation.
*/
static os_error *async_time(async_data *data, bits *taken, bits *remain)
{
    os_error *err = NULL;

    // Check parameters
    if (!data || !taken || !remain) err = &err_bad_parms;
    else
    {
        bits child_taken = 0;
        bits child_remain = 0;
        bits total;
        bits step;
        frac_value frac_child;

        // Recurse if appropriate
        if (!data->time_running && data->child)
        {
            err = async_time(data->child, &child_taken, &child_remain);
        }

        // Calculate the time taken
        *taken = data->time_acc + child_taken;
        if (data->time_running) *taken += util_time() - data->time_start;

        // Estimate total time to complete based on previous steps only
        total = frac_inv_scale(data->time_done, data->frac_done);

        // Estimate time required to complete this step
        frac_child = frac_create(child_taken, data->time_done);
        if (child_taken < ASYNC_CONFIDENT_MIN) frac_child = FRAC_ZERO;
        else if (child_taken < ASYNC_CONFIDENT_MAX)
        {
            frac_value frac_temp = frac_create(child_taken - ASYNC_CONFIDENT_MIN, ASYNC_CONFIDENT_MAX - ASYNC_CONFIDENT_MIN);
            if (frac_cmp(frac_temp, frac_child) < 0) frac_child = frac_temp;
        }
        step = async_combine(frac_scale(total, data->frac_step),
                             frac_not(frac_child),
                             child_taken + child_remain,
                             frac_child);

        // Apply limits to the estimated time for this step
        step = MAX(step, child_taken);
        step = MAX(step, *taken - data->time_done);

        // Estimate the total time required for the operation
        total = async_combine(total,
                              data->frac_done,
                              frac_inv_scale(step, data->frac_step),
                              data->frac_step);

        // Improve the estimate of the total time using known details
        total = data->time_done + step
                + frac_scale(total, frac_not(frac_add(data->frac_done, data->frac_step)));

        // Estimate the remaining time to complete the operation
        *remain = total ? total - *taken : 0;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  taken         - Variable to receive the number of
                                  centi-seconds taken so far.
                  remain        - Variable to receive an estimate of the number
                                  of centi-seconds remaining.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Calculate and prettify the timing information for the
                  operation.
*/
static os_error *async_time_pretty(async_data *data, bits *taken, bits *remain)
{
    os_error *err = NULL;

    // Check parameters
    if (!data || !taken || !remain) err = &err_bad_parms;
    else
    {
        // Calculate the raw timings
        err = async_time(data, taken, remain);

        // Ignore the initial estimates
        if (ASYNC_PRETTY_INIT < *taken)
        {
            bits interval = *taken - data->pretty_last;

            // Preserve the last estimate unless time changed significantly
            if (ASYNC_PRETTY_STEP < interval)
            {
                bits total = *taken + *remain;
                bits offset = *taken - data->pretty_base;
                int error = total - data->pretty_total;
                bool reset = FALSE;

                // Action depends on the sign of the error
                if (error < 0)
                {
                    // New estimate is less than previous
                    if (data->pretty_error < 0)
                    {
                        // Same direction as previous error
                        if (data->pretty_error < error)
                        {
                            data->pretty_error = error;
                        }
                        else
                        {
                            data->pretty_error -= frac_scale(data->pretty_error - error, frac_create(interval * ASYNC_PRETTY_PERC_ERROR, 10000));
                        }
                        if ((ASYNC_PRETTY_DOWN_MIN_TIME < offset)
                            && ((offset < ASYNC_PRETTY_DOWN_MAX_TIME)
                                || (((*remain * ASYNC_PRETTY_DOWN_PERC) / 100)
                                    < -data->pretty_error)))
                        {
                            // Include the error
                            data->pretty_total += data->pretty_error;
                            data->pretty_error = -1;
                        }
                    }
                    else reset = TRUE;
                }
                else
                {
                    // New estimate is more than previous
                    if (data->pretty_total < *taken)
                    {
                        // End of prettified estimate reached
                        data->pretty_total = total;
                        error = 1;
                        reset = TRUE;
                    }
                    else if (0 < data->pretty_error)
                    {
                        // Same direction as previous error
                        if (error < data->pretty_error)
                        {
                            data->pretty_error = error;
                        }
                        else
                        {
                            data->pretty_error += frac_scale(error - data->pretty_error, frac_create(interval * ASYNC_PRETTY_PERC_ERROR, 10000));
                        }
                        if ((ASYNC_PRETTY_UP_MIN_TIME < offset)
                            && ((offset < ASYNC_PRETTY_UP_MAX_TIME)
                                || (((*remain * ASYNC_PRETTY_UP_PERC) / 100)
                                    < data->pretty_error)))
                        {
                            // Include some or all of the error
                            if ((data->pretty_error < ASYNC_PRETTY_UP_ERROR)
                                && ((ASYNC_PRETTY_UP_MAX_TIME
                                     + ASYNC_PRETTY_UP_ERROR)
                                    < (*taken - data->pretty_base)))
                            {
                                data->pretty_total += interval;
                            }
                            else
                            {
                                // need another trigger for this
                                data->pretty_total += data->pretty_error;
                                data->pretty_error = 1;
                            }
                        }
                    }
                    else reset = TRUE;
                }

                // Reset the base status if appropriate
                if (reset)
                {
                    data->pretty_error = error;
                    data->pretty_base = *taken;
                }
                data->pretty_last = *taken;
            }

            // Calculate the massaged time remaining
            *remain = *taken < data->pretty_total
                      ? data->pretty_total - *taken : 0;
        }

        // Hide the time remaining if just started
        if (*taken < ASYNC_PRETTY_HIDE) *remain = 0;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the timer for the specified operation.
*/
static os_error *async_start_time(async_data *data)
{
    os_error *err = NULL;

    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        // No action if already running
        if (!data->time_running)
        {
            // Record the start time
            data->time_start = util_time();

            // Mark the timer as running
            data->time_running = TRUE;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Stop the timer for the specified operation.
*/
static os_error *async_stop_time(async_data *data)
{
    os_error *err = NULL;

    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        // No action if not running
        if (data->time_running)
        {
            // Mark the timer as stopped
            data->time_running = FALSE;

            // Add the current time to the accumulator
            data->time_acc += util_time() - data->time_start;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  done          - Units of operation completed.
                  remain        - Units of operation still to be completed.
                  step          - Units to be completed during current step.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Mark the current time for tracking progress of the specified
                  operation.
*/
static os_error *async_mark_time(async_data *data, bits done, bits remain,
                                 bits step)
{
    os_error *err = NULL;

    // Check parameters
    if (!data || (remain < step)) err = &err_bad_parms;
    else
    {
        // Calculate the percentages
        data->frac_done = frac_create(done, done + remain);
        data->frac_step = frac_create(step, done + remain);

        // Store the time taken so far
        data->time_done = data->time_acc;
        if (data->time_running)
        {
            data->time_done += util_time() - data->time_start;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - The error to store, or NULL to store an
                                  existing error.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Store a specified error and clear the error status.
*/
static os_error *async_store_error(async_data *data, os_error *err)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        // Preserve any existing error
        if (data->has_stored_err)
        {
            // An error has already been stored
            data->has_err = FALSE;
        }
        else if (data->has_err)
        {
            // Store the previous error
            data->stored_err = data->err;
            data->has_stored_err = TRUE;
            data->has_err = FALSE;
        }
        else if (err)
        {
            // Store this error
            data->stored_err = *err;
            data->has_stored_err = TRUE;
        }

        // Clear the active error
        err = NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any active error.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Restore a previously stored error.
*/
static os_error *async_restore_error(async_data *data, os_error *err)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else if (data->has_stored_err)
    {
        // Restore the error
        err = &data->stored_err;
        data->has_stored_err = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : user          - User specified handle for this operation.
                  err           - Any error produced by the operation.
                  reply         - The reply data block passed when the
                                  operation was queued, filled with any
                                  response data.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Callback function for a channel operation.
*/
static os_error *async_callback(void *user, os_error *err, const void *reply)
{
    // Check parameters
    if (!user || (!err && !reply)) err = &err_bad_parms;
    else
    {
        async_data *data = (async_data *) user;

        // Ignore the result if not expected
        if (!async_done(data) && !async_idle(data))
        {
            // Special action if an error was produced
            if (err && !data->has_err)
            {
                // Copy the error
                data->has_err = TRUE;
                data->err = *err;
            }

            // Start processing the result
            data->status = psifs_ASYNC_BUSY;
            err = async_process(data);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a shutdown operation by a single stage.
*/
static os_error *async_process_shutdown(async_data *data, os_error *err,
                                        async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->suspend = TRUE;
                data->file = NULL;
                if (!err)
                {
                    data->file = fopen(data->op.data.shutdown.path,
                                       data->op.data.shutdown.append
                                       ? "a" : "w");
                    if (!data->file) err = &err_not_found;
                }
                if (!err)
                {
                    data->index = 0;
                    data->status = psifs_ASYNC_PROG_LIST;
                    data->data.unified.cmd.op = UNIFIED_TASKS;
                    data->data.unified.cmd.data.tasks.size = ASYNC_MIN_TASKS;
                    err = async_buffer(data, data->data.unified.cmd.data.tasks.size * sizeof(ncp_app));
                }
                if (!err)
                {
                    data->data.unified.cmd.data.tasks.buffer = data->buffer.tasks;
                    err = unified_back(&data->data.unified.cmd,
                                       &data->data.unified.reply,
                                       data, async_callback);
                }
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (err)
                {
                    if ((data->data.unified.cmd.op == UNIFIED_TASKS)
                        && ERR_EQ(*err, err_ncp_too_many))
                    {
                        // Try to increase the size of the buffer
                        data->status = psifs_ASYNC_PROG_LIST;
                        data->data.unified.cmd.data.tasks.size *= 2;
                        err = async_buffer(data, data->data.unified.cmd.data.tasks.size * sizeof(ncp_app));
                        if (!err)
                        {
                            data->data.unified.cmd.data.tasks.buffer = data->buffer.tasks;
                            err = unified_back(&data->data.unified.cmd,
                                               &data->data.unified.reply,
                                               data, async_callback);
                        }
                    }
                }
                else
                {
                    if (data->data.unified.cmd.op == UNIFIED_TASKS)
                    {
                        data->index = 0;
                        data->num = data->data.unified.reply.tasks.used;
                    }
                    if ((data->data.unified.cmd.op == UNIFIED_DETAIL)
                        && (!*data->op.data.shutdown.pattern
                            || !wildcard_cmp(data->op.data.shutdown.pattern, data->data.unified.reply.detail.args)))
                    {
                        // Store the result
                        fprintf(data->file, "%s%c%s%c",
                                data->data.unified.reply.detail.name,
                                ASYNC_TERM_NAME,
                                data->data.unified.reply.detail.args,
                                ASYNC_TERM_ARGS);

                        // Attempt to shutdown the task
                        data->status = psifs_ASYNC_PROG_CLOSE;
                        data->data.unified.cmd.op = UNIFIED_STOP;
                        if (sizeof(data->data.unified.cmd.data.stop.name) <= strlen(data->buffer.tasks[data->index].name))
                        {
                            err = &err_bad_name;
                        }
                        else
                        {
                            strcpy(data->data.unified.cmd.data.stop.name, data->buffer.tasks[data->index].name);
                            err = unified_back(&data->data.unified.cmd,
                                               &data->data.unified.reply,
                                               data, async_callback);
                        }
                        if (!err)
                        {
                            err = async_mark_time(data, data->index * 5 + 1, (data->num - data->index) * 5 - 1, 4);
                        }

                        // Increment the task index
                        if (!err) data->index++;
                    }
                    else
                    {
                        if (data->data.unified.cmd.op == UNIFIED_DETAIL)
                        {
                            data->index++;
                        }
                        if (data->index < data->num)
                        {
                            // Read the details of the next task
                            data->status = psifs_ASYNC_PROG_DETAIL;
                            data->data.unified.cmd.op = UNIFIED_DETAIL;
                            err = async_detail_app(data, data->buffer.tasks[data->index].name, data->buffer.tasks[data->index].args);
                            if (!err && (sizeof(data->data.unified.cmd.data.detail.name) <= strlen(data->buffer.tasks[data->index].name)))
                            {
                                err = &err_bad_name;
                            }
                            if (!err)
                            {
                                strcpy(data->data.unified.cmd.data.detail.name, data->buffer.tasks[data->index].name);
                                err = unified_back(&data->data.unified.cmd,
                                                   &data->data.unified.reply,
                                                   data, async_callback);
                            }
                            if (!err)
                            {
                                err = async_mark_time(data, data->index * 5, (data->num - data->index) * 5, 1);
                            }
                        }
                        else data->status = psifs_ASYNC_SUCCESS;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->file)
                {
                    // Close the output file
                    fclose(data->file);
                    data->file = NULL;
                }
                if (err || data->abort)
                {
                    // Delete the object if error or operation aborted
                    xosfscontrol_wipe(data->op.data.shutdown.path,
                                      osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Result is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a restart operation by a single stage.
*/
static os_error *async_process_restart(async_data *data, os_error *err,
                                       async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->suspend = TRUE;
                data->file = NULL;
                if (!err)
                {
                    data->file = fopen(data->op.data.restart.path, "r");
                    if (!data->file) err = &err_not_found;
                }
                if (!err)
                {
                    data->index = 0;
                    data->num = 0;
                    while (async_getline(data->file, ASYNC_TERM_NAME, data->data.unified.cmd.data.start.name, sizeof(data->data.unified.cmd.data.start.name)) && async_getline(data->file, ASYNC_TERM_ARGS, data->data.unified.cmd.data.start.args, sizeof(data->data.unified.cmd.data.start.args)))
                    {
                        data->num++;
                    }
                    rewind(data->file);
                    data->status = psifs_ASYNC_BUSY;
                    data->response = psifs_ASYNC_RESPONSE_SKIP;
                }
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (err)
                {
                    // Query whether the restart from be retried
                    data->status = psifs_ASYNC_WAIT_RESTART;
                    data->err = *err;
                    err = NULL;
                }
                else
                {
                    if (data->response == psifs_ASYNC_RESPONSE_SKIP)
                    {
                        // Read the details of the next task
                        if (!async_getline(data->file, ASYNC_TERM_NAME, data->data.unified.cmd.data.start.name, sizeof(data->data.unified.cmd.data.start.name)) || !async_getline(data->file, ASYNC_TERM_ARGS, data->data.unified.cmd.data.start.args, sizeof(data->data.unified.cmd.data.start.args)))
                        {
                            data->status = psifs_ASYNC_SUCCESS;
                        }
                    }
                    else
                    {
                        // Try starting the same task again
                        data->num++;
                        data->response = psifs_ASYNC_RESPONSE_SKIP;
                    }
                    if (data->status != psifs_ASYNC_SUCCESS)
                    {
                        // Start this task
                        data->status = psifs_ASYNC_PROG_OPEN;
                        data->data.unified.cmd.op = UNIFIED_START;
                        err = async_detail_app(data, data->data.unified.cmd.data.start.name, data->data.unified.cmd.data.start.args);
                        if (!err)
                        {
                            data->data.unified.cmd.data.start.action = strlen(data->data.unified.cmd.data.start.args) ? UNIFIED_START_OPEN : UNIFIED_START_DEFAULT;
                            err = unified_back(&data->data.unified.cmd,
                                               &data->data.unified.reply,
                                               data, async_callback);
                        }
                        if (!err)
                        {
                            err = async_mark_time(data, data->index, data->num - data->index, 1);
                        }
                        if (!err) data->index++;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->file)
                {
                    // Close the output file
                    fclose(data->file);
                    data->file = NULL;

                    // Remove the file if required
                    if (data->op.data.restart.remove)
                    {
                        xosfscontrol_wipe(data->op.data.restart.path,
                                          osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
                    }
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a read operation by a single stage.
*/
static os_error *async_process_read(async_data *data, os_error *err,
                                    async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        const char *str;

        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->suspend = TRUE;
                data->osfile = 0;
                data->remote = FS_NONE;
                xosfscontrol_wipe(data->op.data.read.dest,
                                  osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
                if (!err)
                {
                    data->status = psifs_ASYNC_CAT_READ;
                    data->data.cache.cmd.op = CACHE_INFO;
                    err = async_tidy_args(data->op.data.read.src, &str);
                }
                if (!err)
                {
                    strcpy(data->detail, str);
                    strcpy(data->data.cache.cmd.data.info.path,
                           data->op.data.read.src);
                    err = cache_back(&data->data.cache.cmd,
                                     &data->data.cache.reply,
                                     data, async_callback);
                }
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                err = async_restore_error(data, err);
                if (!err)
                {
                    if (data->data.cache.cmd.op == CACHE_INFO)
                    {
                        data->info = data->data.cache.reply.info.info;
                        if (data->info.obj_type == fileswitch_NOT_FOUND)
                        {
                            err = &err_not_found;
                        }
                        else if (data->info.obj_type == fileswitch_IS_DIR)
                        {
                            // Create directory
                            err = xosfile_create_dir(data->op.data.read.dest, 0);
                            data->status = psifs_ASYNC_SUCCESS;
                        }
                        else
                        {
                            // Create file
                            err = xosfind_openoutw(osfind_NO_PATH
                                                   | osfind_ERROR_IF_DIR,
                                                   data->op.data.read.dest,
                                                   NULL, &data->osfile);
                            if (!err && !data->osfile) err = &err_not_found;
                            if (err) data->osfile = 0;
                            if (!err)
                            {
                                data->status = psifs_ASYNC_FILE_OPEN;
                                data->data.cache.cmd.op = CACHE_OPEN;
                                strcpy(data->data.cache.cmd.data.open.path,
                                       data->op.data.read.src);
                                data->data.cache.cmd.data.open.mode = FS_MODE_IN;
                                data->data.cache.cmd.data.open.handle = 0;
                                err = cache_back(&data->data.cache.cmd,
                                                 &data->data.cache.reply,
                                                 data, async_callback);
                            }
                        }
                    }
                    else if (data->data.cache.cmd.op == CACHE_CLOSE)
                    {
                        // Source file closed
                        data->remote = FS_NONE;
                        data->status = psifs_ASYNC_SUCCESS;
                    }
                    else
                    {
                        if (data->data.cache.cmd.op == CACHE_OPEN)
                        {
                            // Just opened
                            data->remote = data->data.cache.reply.open.handle;
                            data->data.cache.cmd.data.read.offset = 0;
                            data->data.cache.cmd.data.read.length = ASYNC_COPY_SIZE;
                            err = async_buffer(data, data->data.cache.cmd.data.read.length);
                            if (!err) data->data.cache.cmd.data.read.buffer = data->buffer.ptr;
                        }
                        else
                        {
                            int unwritten;

                            // Block read
                            err = xosgbpb_writew(data->osfile,(byte *) data->data.cache.cmd.data.read.buffer, data->data.cache.cmd.data.read.length, &unwritten);
                            if (!err && unwritten) err = &err_eof;
                            if (!err) data->data.cache.cmd.data.read.offset += data->data.cache.cmd.data.read.length;
                        }
                        if (!err)
                        {
                            if (data->data.cache.cmd.data.read.offset
                                < data->info.size)
                            {
                                // Read the next block of data
                                data->status = psifs_ASYNC_FILE_READ;
                                data->data.cache.cmd.op = CACHE_READ;
                                data->data.cache.cmd.data.read.handle = data->remote;
                                if (data->info.size
                                    - data->data.cache.cmd.data.read.offset
                                    < data->data.cache.cmd.data.read.length)
                                {
                                    data->data.cache.cmd.data.read.length = data->info.size - data->data.cache.cmd.data.read.offset;
                                }
                                err = cache_back(&data->data.cache.cmd,
                                                 &data->data.cache.reply,
                                                 data, async_callback);
                                if (!err)
                                {
                                    err = async_mark_time(data, data->data.cache.cmd.data.read.offset, data->info.size - data->data.cache.cmd.data.read.offset, data->data.cache.cmd.data.read.length);
                                }
                            }
                            else
                            {
                                // Close the input file when finished
                                data->status = psifs_ASYNC_FILE_CLOSE;
                                data->data.cache.cmd.op = CACHE_CLOSE;
                                data->data.cache.cmd.data.close.handle = data->remote;
                                err = cache_back(&data->data.cache.cmd,
                                                 &data->data.cache.reply,
                                                 data, async_callback);
                            }
                        }
                    }
                }
                if (err && (data->remote != FS_NONE))
                {
                    // Close the remote file
                    err = async_store_error(data, err);
                    data->status = psifs_ASYNC_FILE_CLOSE;
                    data->data.cache.cmd.op = CACHE_CLOSE;
                    data->data.cache.cmd.data.close.handle = data->remote;
                    data->remote = FS_NONE;
                    err = cache_back(&data->data.cache.cmd,
                                     &data->data.cache.reply,
                                     data, async_callback);
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                err = async_restore_error(data, err);
                if (data->remote != FS_NONE)
                {
                    // Close the remote file
                    err = async_store_error(data, err);
                    data->status = psifs_ASYNC_FILE_CLOSE;
                    data->data.cache.cmd.op = CACHE_CLOSE;
                    data->data.cache.cmd.data.close.handle = data->remote;
                    data->remote = FS_NONE;
                    err = cache_back(&data->data.cache.cmd,
                                     &data->data.cache.reply,
                                     data, async_callback);
                }
                else if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->osfile)
                {
                    // Close the output file
                    xosfind_closew(data->osfile);
                    data->osfile = 0;
                }
                if (!err && !data->abort)
                {
                    // Attempt to write the catalogue information
                    err = xosfile_write(data->op.data.read.dest,
                                        data->info.load_addr,
                                        data->info.exec_addr,
                                        data->info.attr);
                }
                if (err || data->abort)
                {
                    // Delete the file if error or operation aborted
                    xosfscontrol_wipe(data->op.data.read.dest,
                                      osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a write operation by a single stage.
*/
static os_error *async_process_write(async_data *data, os_error *err,
                                     async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        const char *str;

        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->suspend = TRUE;
                data->osfile = 0;
                data->remote = FS_NONE;
                err = xosfile_read_stamped(data->op.data.write.src,
                                           &data->info.obj_type,
                                           &data->info.load_addr,
                                           &data->info.exec_addr,
                                           &data->info.size,
                                           &data->info.attr,
                                           NULL);
                if (!err && (data->info.obj_type == fileswitch_NOT_FOUND))
                {
                    err = &err_not_found;
                }
                if (!err)
                {
                    data->status = psifs_ASYNC_CAT_READ;
                    data->data.cache.cmd.op = CACHE_INFO;
                    err = async_tidy_args(data->op.data.write.dest, &str);
                }
                if (!err)
                {
                    strcpy(data->detail, str);
                    strcpy(data->data.cache.cmd.data.info.path,
                           data->op.data.write.dest);
                    err = cache_back(&data->data.cache.cmd,
                                     &data->data.cache.reply,
                                     data, async_callback);
                }
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                err = async_restore_error(data, err);
                if (!err)
                {
                    if (data->data.cache.cmd.op == CACHE_INFO)
                    {
                        // Read details of existing object
                        data->num = data->data.cache.reply.info.info.obj_type;
                        if (data->num == fileswitch_NOT_FOUND)
                        {
                            // Skip to creating the new object
                            data->data.cache.cmd.op = CACHE_REMOVE;
                            data->status = psifs_ASYNC_BUSY;
                        }
                        else
                        {
                            // Unlock the existing object
                            data->status = psifs_ASYNC_CAT_WRITE;
                            data->data.cache.cmd.op = CACHE_ACCESS;
                            strcpy(data->data.cache.cmd.data.access.path,
                                   data->op.data.write.dest);
                            data->data.cache.cmd.data.access.attr = fileswitch_ATTR_OWNER_READ | fileswitch_ATTR_OWNER_WRITE;
                            err = cache_back(&data->data.cache.cmd,
                                             &data->data.cache.reply,
                                             data, async_callback);
                        }
                    }
                    else if (data->data.cache.cmd.op == CACHE_ACCESS)
                    {
                        if (data->num == fileswitch_NOT_FOUND)
                        {
                            // The final step was setting the attributes
                            data->status = psifs_ASYNC_SUCCESS;
                        }
                        else if ((data->num == fileswitch_IS_DIR)
                                 && (data->info.obj_type == fileswitch_IS_DIR))
                        {
                            // Just need to change the date stamp and attributes
                            data->data.cache.cmd.op = CACHE_MKDIR;
                            data->status = psifs_ASYNC_BUSY;
                        }
                        else if ((data->num != fileswitch_IS_DIR)
                                 && (data->info.obj_type != fileswitch_IS_DIR))
                        {
                            // Overwrite existing file
                            data->data.cache.cmd.op = CACHE_REMOVE;
                            data->status = psifs_ASYNC_BUSY;
                        }
                        else
                        {
                            // Object unlocked
                            data->status = psifs_ASYNC_FILE_DELETE;
                            data->data.cache.cmd.op = CACHE_REMOVE;
                            strcpy(data->data.cache.cmd.data.remove.path,
                                   data->op.data.write.dest);
                            err = cache_back(&data->data.cache.cmd,
                                             &data->data.cache.reply,
                                             data, async_callback);
                        }
                        data->num = fileswitch_NOT_FOUND;
                    }
                    else if (data->data.cache.cmd.op == CACHE_REMOVE)
                    {
                        // Source prepared
                        if (data->info.obj_type == fileswitch_IS_DIR)
                        {
                            // Create directory
                            data->status = psifs_ASYNC_FILE_MKDIR;
                            data->data.cache.cmd.op = CACHE_MKDIR;
                            strcpy(data->data.cache.cmd.data.mkdir.path,
                                   data->op.data.write.dest);
                            err = cache_back(&data->data.cache.cmd,
                                             &data->data.cache.reply,
                                             data, async_callback);
                        }
                        else
                        {
                            // Create file
                            err = xosfind_openinw(osfind_NO_PATH
                                                  | osfind_ERROR_IF_ABSENT
                                                  | osfind_ERROR_IF_DIR,
                                                  data->op.data.write.src,
                                                  NULL, &data->osfile);
                            if (!err && !data->osfile) err = &err_not_found;
                            if (err) data->osfile = 0;
                            if (!err)
                            {
                                data->status = psifs_ASYNC_FILE_OPEN;
                                data->data.cache.cmd.op = CACHE_OPEN;
                                strcpy(data->data.cache.cmd.data.open.path,
                                       data->op.data.write.dest);
                                data->data.cache.cmd.data.open.mode = FS_MODE_OUT;
                                data->data.cache.cmd.data.open.handle = 0;
                                err = cache_back(&data->data.cache.cmd,
                                                 &data->data.cache.reply,
                                                 data, async_callback);
                            }
                        }
                    }
                    else if (data->data.cache.cmd.op == CACHE_OPEN)
                    {
                        // Just opened
                        data->remote = data->data.cache.reply.open.handle;
                        data->status = psifs_ASYNC_FILE_WRITE;
                        data->data.cache.cmd.op = CACHE_EXTENT;
                        data->data.cache.cmd.data.extent.handle = data->remote;
                        data->data.cache.cmd.data.extent.size = data->info.size;
                        err = cache_back(&data->data.cache.cmd,
                                         &data->data.cache.reply,
                                         data, async_callback);
                    }
                    else if ((data->data.cache.cmd.op == CACHE_EXTENT)
                             || (data->data.cache.cmd.op == CACHE_WRITE))
                    {
                        if (data->data.cache.cmd.op == CACHE_EXTENT)
                        {
                            // File extent set
                            data->data.cache.cmd.data.write.offset = 0;
                            data->data.cache.cmd.data.write.length = ASYNC_COPY_SIZE;
                            err = async_buffer(data, data->data.cache.cmd.data.write.length);
                            if (!err) data->data.cache.cmd.data.write.buffer = data->buffer.ptr;
                        }
                        else
                        {
                            // Written another block
                            data->data.cache.cmd.data.write.offset += data->data.cache.cmd.data.write.length;
                        }
                        if (data->data.cache.cmd.data.write.offset
                            < data->info.size)
                        {
                            int unread;

                            // Write the next block of data
                            if (data->info.size
                                - data->data.cache.cmd.data.write.offset
                                < data->data.cache.cmd.data.write.length)
                            {
                                data->data.cache.cmd.data.write.length = data->info.size - data->data.cache.cmd.data.write.offset;
                            }
                            err = xosgbpb_readw(data->osfile, (byte *) data->data.cache.cmd.data.write.buffer, data->data.cache.cmd.data.write.length, &unread);
                            if (!err && unread) err = &err_eof;
                            if (!err)
                            {
                                data->status = psifs_ASYNC_FILE_WRITE;
                                data->data.cache.cmd.op = CACHE_WRITE;
                                data->data.cache.cmd.data.write.handle = data->remote;
                                err = cache_back(&data->data.cache.cmd,
                                                 &data->data.cache.reply,
                                                 data, async_callback);
                            }
                            if (!err)
                            {
                                err = async_mark_time(data, data->data.cache.cmd.data.write.offset, data->info.size - data->data.cache.cmd.data.write.offset, data->data.cache.cmd.data.write.length);
                            }
                        }
                        else
                        {
                            // Close the input file when finished
                            data->status = psifs_ASYNC_FILE_CLOSE;
                            data->data.cache.cmd.op = CACHE_CLOSE;
                            data->data.cache.cmd.data.close.handle = data->remote;
                            err = cache_back(&data->data.cache.cmd,
                                             &data->data.cache.reply,
                                             data, async_callback);
                        }
                    }
                    else if ((data->data.cache.cmd.op == CACHE_MKDIR)
                             || (data->data.cache.cmd.op == CACHE_CLOSE))
                    {
                        // Object created so set the date stamp
                        data->remote = FS_NONE;
                        data->status = psifs_ASYNC_CAT_WRITE;
                        data->data.cache.cmd.op = CACHE_STAMP;
                        strcpy(data->data.cache.cmd.data.stamp.path,
                               data->op.data.write.dest);
                        data->data.cache.cmd.data.stamp.load = data->info.load_addr;
                        data->data.cache.cmd.data.stamp.exec = data->info.exec_addr;
                        err = cache_back(&data->data.cache.cmd,
                                         &data->data.cache.reply,
                                         data, async_callback);
                    }
                    else if (data->data.cache.cmd.op == CACHE_STAMP)
                    {
                        // Set the attributes
                        data->status = psifs_ASYNC_CAT_WRITE;
                        data->data.cache.cmd.op = CACHE_ACCESS;
                        strcpy(data->data.cache.cmd.data.access.path,
                               data->op.data.write.dest);
                        data->data.cache.cmd.data.access.attr = data->info.attr;
                        err = cache_back(&data->data.cache.cmd,
                                         &data->data.cache.reply,
                                         data, async_callback);
                    }
                }
                if (err && (data->remote != FS_NONE))
                {
                    // Close the remote file
                    err = async_store_error(data, err);
                    data->status = psifs_ASYNC_FILE_CLOSE;
                    data->data.cache.cmd.op = CACHE_CLOSE;
                    data->data.cache.cmd.data.close.handle = data->remote;
                    data->remote = FS_NONE;
                    err = cache_back(&data->data.cache.cmd,
                                     &data->data.cache.reply,
                                     data, async_callback);
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                err = async_restore_error(data, err);
                if (data->remote != FS_NONE)
                {
                    // Close the remote file
                    err = async_store_error(data, err);
                    data->status = psifs_ASYNC_FILE_CLOSE;
                    data->data.cache.cmd.op = CACHE_CLOSE;
                    data->data.cache.cmd.data.close.handle = data->remote;
                    data->remote = FS_NONE;
                    err = cache_back(&data->data.cache.cmd,
                                     &data->data.cache.reply,
                                     data, async_callback);
                }
                else if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->osfile)
                {
                    // Close the input file
                    xosfind_closew(data->osfile);
                    data->osfile = 0;
                }
                if (data->op.data.write.remove)
                {
                    // Delete the object if required
                    xosfscontrol_wipe(data->op.data.write.src,
                                      osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a backup operation by a single stage.
*/
static os_error *async_process_backup(async_data *data, os_error *err,
                                      async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        static async_op op;

        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->tree = BACKTREE_NONE;
                err = backtree_create(&data->tree);
                if (!err)
                {
                    xosfscontrol_wipe(data->op.data.backup.dest,
                                      osfscontrol_WIPE_FORCE,
                                      0, 0, 0, 0);
                    if (*data->op.data.backup.scrap)
                    {
                        xosfscontrol_wipe(data->op.data.backup.scrap,
                                          osfscontrol_WIPE_FORCE,
                                          0, 0, 0, 0);
                    }
                    xosfscontrol_wipe(data->op.data.backup.temp,
                                      osfscontrol_WIPE_FORCE,
                                      0, 0, 0, 0);
                    data->index = 0;
                    data->recurse = TRUE;
                    data->status = psifs_ASYNC_DELEGATE;
                    op.op = ASYNC_BACKUP_LIST;
                    op.data.backup_list.tree = data->tree;
                    strcpy(op.data.backup_list.src, data->op.data.backup.src);
                    *op.data.backup_list.sub = '\0';
                    err = async_new(data, &op, NULL);
                }
                if (!err) err = async_mark_time(data, 0, 40, 2);
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (!err)
                {
                    bits files;
                    bits size;

                    // Select the next operation
                    data->index++;
                    if (data->index == 1)
                    {
                        // Backup tree created
                        data->status = psifs_ASYNC_DELEGATE;
                        op.op = ASYNC_BACKUP_PREV;
                        op.data.backup_list.tree = data->tree;
                        strcpy(op.data.backup_prev.src,
                               data->op.data.backup.src);
                        strcpy(op.data.backup_prev.dest,
                               data->op.data.backup.dest);
                        strcpy(op.data.backup_prev.prev,
                               data->op.data.backup.prev);
                        strcpy(op.data.backup_prev.scrap,
                               data->op.data.backup.scrap);
                        err = async_new(data, &op, NULL);
                        if (!err)
                        {
                            err = backtree_count(data->tree, &files, &size);
                        }
                        if (!err)
                        {
                            data->num = files * ASYNC_TAR_WEIGHT
                                        + size * ASYNC_TAR_SIZE_WEIGHT;
                            err = async_mark_time(data, 2, 38, 3);
                        }
                    }
                    else if (data->index == 2)
                    {
                        // Previous backup processed
                        data->status = psifs_ASYNC_DELEGATE;
                        op.op = ASYNC_BACKUP_COPY;
                        op.data.backup_list.tree = data->tree;
                        strcpy(op.data.backup_copy.src,
                               data->op.data.backup.src);
                        strcpy(op.data.backup_copy.dest,
                               data->op.data.backup.dest);
                        strcpy(op.data.backup_copy.temp,
                               data->op.data.backup.temp);
                        err = async_new(data, &op, NULL);
                        if (!err)
                        {
                            err = backtree_count(data->tree, &files, &size);
                        }
                        if (!err)
                        {
                            bits size = files * ASYNC_FILE_WEIGHT
                                        + size * ASYNC_FILE_SIZE_WEIGHT
                                        + files * ASYNC_TAR_WEIGHT
                                        + size * ASYNC_TAR_SIZE_WEIGHT;
                            size = frac_inv_scale(3, frac_create(data->num, size));
                            err = async_mark_time(data, 5, size, size);
                        }
                    }
                    else
                    {
                        // Operation complete after remote files copied
                        data->status = psifs_ASYNC_SUCCESS;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->child)
                {
                    // End any children
                    async_abort(data->child);
                }
                if (data->tree != BACKTREE_NONE)
                {
                    // Destroy the backup tree
                    backtree_destroy(&data->tree);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a write and start operation by a single stage.
*/
static os_error *async_process_write_start(async_data *data, os_error *err,
                                           async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        const char *str;
        static async_op op;

        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->suspend = TRUE;
                data->recurse = TRUE;
                data->status = psifs_ASYNC_DELEGATE;
                data->index = 0;
                op.op = psifs_ASYNC_WRITE;
                strcpy(op.data.write.src, data->op.data.write_start.src);
                strcpy(op.data.write.dest, data->op.data.write_start.dest);
                op.data.write.remove = data->op.data.write_start.remove;
                err = async_new(data, &op, NULL);
                if (!err) err = async_mark_time(data, 0, 100, 85);
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (!err)
                {
                    // Select the next operation
                    data->index++;
                    if (data->index == 1)
                    {
                        // Wait to allow the file to be closed
                        data->status = psifs_ASYNC_FILE_CLOSE;
                        err = async_tidy_args(data->op.data.write_start.dest, &str);
                        if (!err)
                        {
                            strcpy(data->detail, str);
                            err = timer_back(util_time() + ASYNC_CLOSE_DELAY,
                                             data, async_callback);
                        }
                        if (!err) err = async_mark_time(data, 85, 15, 5);
                    }
                    else if (data->index == 2)
                    {
                        // Start the file
                        data->status = psifs_ASYNC_PROG_OPEN;
                        data->data.unified.cmd.op = UNIFIED_START;
                        if (*data->op.data.write_start.exe)
                        {
                            data->data.unified.cmd.data.start.action = UNIFIED_START_OPEN;
                            if ((sizeof(data->data.unified.cmd.data.start.name) <= strlen(data->op.data.write_start.exe)) || (sizeof(data->data.unified.cmd.data.start.args) <= strlen(data->op.data.write_start.dest))) err = &err_bad_name;
                            else
                            {
                                strcpy(data->data.unified.cmd.data.start.name, data->op.data.write_start.exe);
                                strcpy(data->data.unified.cmd.data.start.args, data->op.data.write_start.dest);
                            }
                        }
                        else
                        {
                            data->data.unified.cmd.data.start.action = UNIFIED_START_DEFAULT;
                            if (sizeof(data->data.unified.cmd.data.start.name) <= strlen(data->op.data.write_start.dest)) err = &err_bad_name;
                            else
                            {
                                strcpy(data->data.unified.cmd.data.start.name, data->op.data.write_start.dest);
                                *data->data.unified.cmd.data.start.args = '\0';
                            }
                        }
                        if (!err)
                        {
                            err = async_detail_app(data, data->data.unified.cmd.data.start.name, data->data.unified.cmd.data.start.args);
                        }
                        if (!err)
                        {
                            err = unified_back(&data->data.unified.cmd,
                                               &data->data.unified.reply,
                                               data, async_callback);
                        }
                        if (!err) err = async_mark_time(data, 90, 10, 10);
                    }
                    else
                    {
                        // Operation complete after file started
                        data->status = psifs_ASYNC_SUCCESS;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->child)
                {
                    // End any children
                    async_abort(data->child);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress an install operation by a single stage.
*/
static os_error *async_process_install(async_data *data, os_error *err,
                                       async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        static async_op op;

        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->index = 0;
                data->recurse = TRUE;
                data->status = psifs_ASYNC_DELEGATE;
                op.op = ASYNC_FIND;
                strcpy(op.data.find.path, data->op.data.install.inst_exe);
                op.data.find.path[1] = FS_CHAR_DRIVE_ALL;
                err = async_new(data, &op, NULL);
                if (!err) err = async_mark_time(data, 0, 100, 5);
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (!err)
                {
                    data->index++;
                    if (data->index == 1)
                    {
                        // Install the installer if necessary
                        if (data->child->op.data.find.drive == FS_CHAR_DRIVE_ALL)
                        {
                            data->status = psifs_ASYNC_DELEGATE;
                            op.op = psifs_ASYNC_WRITE_START;
                            strcpy(op.data.write_start.src, data->op.data.install.inst_src);
                            strcpy(op.data.write_start.dest, data->op.data.install.inst_dest);
                            *op.data.write_start.exe = '\0';
                            op.data.write_start.remove = data->op.data.install.inst_remove;
                            err = async_new(data, &op, NULL);
                            if (!err) err = async_mark_time(data, 5, 95, 25);
                        }
                        else
                        {
                            // Store the actual drive letter
                            data->op.data.install.inst_exe[1] = data->child->op.data.find.drive;
                        }
                    }
                    else if (data->index == 2)
                    {
                        // Install the package
                        data->status = psifs_ASYNC_DELEGATE;
                        op.op = psifs_ASYNC_WRITE_START;
                        strcpy(op.data.write_start.src, data->op.data.install.pckg_src);
                        strcpy(op.data.write_start.dest, data->op.data.install.pckg_dest);
                        strcpy(op.data.write_start.exe, data->op.data.install.inst_exe);
                        op.data.write_start.remove = data->op.data.install.pckg_remove;
                        err = async_new(data, &op, NULL);
                        if (!err) err = async_mark_time(data, 30, 70, 70);
                    }
                    else
                    {
                        // Operation complete after file started
                        data->status = psifs_ASYNC_SUCCESS;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->child)
                {
                    // End any children
                    async_abort(data->child);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a list backup files operation by a single stage.
*/
static os_error *async_process_backup_list(async_data *data, os_error *err,
                                           async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        static async_op op;
        const char *str;

        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->index = 0;
                data->num = 0;
                data->tree = BACKTREE_NONE;
                err = backtree_clone(data->op.data.backup_list.tree,
                                     &data->tree);
                if (!err && (sizeof(data->data.cache.cmd.data.enumerate.path) <= strlen(data->op.data.backup_list.src) + strlen(data->op.data.backup_list.sub) + 1)) err = &err_bad_name;
                if (!err)
                {
                    data->status = psifs_ASYNC_CAT_READ;
                    data->data.cache.cmd.op = CACHE_ENUMERATE;
                    strcpy(data->data.cache.cmd.data.enumerate.path,
                           data->op.data.backup_list.src);
                    if (*data->op.data.backup_list.sub)
                    {
                        sprintf(strchr(data->data.cache.cmd.data.enumerate.path, '\0'), "%c%s",
                                FS_CHAR_SEPARATOR,
                                data->op.data.backup_list.sub);
                    }
                    err = async_tidy_args(data->data.cache.cmd.data.enumerate.path, &str);
                }
                if (!err)
                {
                    strcpy(data->detail, str);
                    strcpy(data->data.cache.cmd.data.enumerate.match, "*");
                    data->data.cache.cmd.data.enumerate.offset = 0;
                    data->data.cache.cmd.data.enumerate.size = ASYNC_MIN_FILES;
                    err = async_buffer(data, data->data.cache.cmd.data.enumerate.size * sizeof(fs_info));
                }
                if (!err)
                {
                    data->data.cache.cmd.data.enumerate.buffer = data->buffer.info;
                    err = cache_back(&data->data.cache.cmd,
                                     &data->data.cache.reply,
                                     data, async_callback);
                }
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (!err)
                {
                    if (!data->num
                        && (0 <= data->data.cache.reply.enumerate.offset))
                    {
                        // Buffer too small to read all entries
                        data->status = psifs_ASYNC_CAT_READ;
                        data->data.cache.cmd.data.enumerate.size *= 2;
                        err = async_buffer(data, data->data.cache.cmd.data.enumerate.size * sizeof(fs_info));
                        if (!err)
                        {
                        data->data.cache.cmd.data.enumerate.buffer = data->buffer.info;
                        err = cache_back(&data->data.cache.cmd,
                                         &data->data.cache.reply,
                                         data, async_callback);
                        }
                    }
                    else if (!data->num)
                    {
                        // Directory contents read
                        while (!err && (data->index < data->data.cache.reply.enumerate.read))
                        {
                            // Add the sub-directory to the path
                            if (sizeof(data->buffer.info[data->index].name) <= strlen(data->op.data.backup_list.sub) + strlen(data->data.cache.cmd.data.enumerate.path) + 1) err = &err_bad_name;
                            if (!err && *data->op.data.backup_list.sub)
                            {
                                strcpy(data->data.cache.cmd.data.enumerate.path, data->buffer.info[data->index].name);
                                sprintf(data->buffer.info[data->index].name, "%s%c%s", data->op.data.backup_list.sub, FS_CHAR_SEPARATOR, data->data.cache.cmd.data.enumerate.path);
                            }
                            if (!err) err = backtree_add(data->tree, &data->buffer.info[data->index]);
                            if (!err)
                            {
                                if (data->buffer.info[data->index].obj_type == fileswitch_IS_DIR) data->num++;
                                data->index++;
                            }
                        }
                        if (!err)
                        {
                            data->index = 0;
                            if (!data->num) data->status = psifs_ASYNC_SUCCESS;
                        }
                    }
                    if (data->num)
                    {
                        if (data->index < data->num)
                        {
                            // Recurse through the next sub-directory
                            while (data->buffer.info[data->data.cache.cmd.data.enumerate.offset].obj_type != fileswitch_IS_DIR)
                            {
                                data->data.cache.cmd.data.enumerate.offset++;
                            }
                            data->recurse = TRUE;
                            data->status = psifs_ASYNC_DELEGATE;
                            op.op = ASYNC_BACKUP_LIST;
                            op.data.backup_list.tree = data->tree;
                            strcpy(op.data.backup_list.src, data->op.data.backup_list.src);
                            strcpy(op.data.backup_list.sub, data->buffer.info[data->data.cache.cmd.data.enumerate.offset].name);
                            err = async_new(data, &op, NULL);
                            if (!err)
                            {
                                data->index++;
                                data->data.cache.cmd.data.enumerate.offset++;
                                err = async_mark_time(data, data->index, data->num + 1 - data->index, 1);
                            }
                        }
                        else data->status = psifs_ASYNC_SUCCESS;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->child)
                {
                    // End any children
                    async_abort(data->child);
                }
                if (data->tree != BACKTREE_NONE)
                {
                    // Destroy the backup tree
                    backtree_destroy(&data->tree);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a previous backup processing operation by a single
                  stage.
*/
static os_error *async_process_backup_prev(async_data *data, os_error *err,
                                           async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        // Action depends on the current stage
        switch (stage)
        {
            static async_op op;

            case ASYNC_INITIALISE:
                // Operation initialising
                data->tree = BACKTREE_NONE;
                data->prev_tar = NULL;
                data->scrap_tar = NULL;
                data->dest_tar = NULL;
                if (*data->op.data.backup_prev.prev)
                {
                    err = backtree_clone(data->op.data.backup_prev.tree,
                                         &data->tree);
                    if (!err)
                    {
                        err = tar_open_in(data->op.data.backup_prev.prev,
                                          &data->prev_tar);
                    }
                    if (!err && *data->op.data.backup_prev.scrap)
                    {
                        err = tar_open_out(data->op.data.backup_prev.scrap,
                                           &data->scrap_tar, FALSE);
                    }
                    if (!err)
                    {
                        err = tar_open_out(data->op.data.backup_prev.dest,
                                           &data->dest_tar, FALSE);
                    }
                    if (!err) data->response = psifs_ASYNC_RESPONSE_CONTINUE;
                }
                else data->status = psifs_ASYNC_SUCCESS;
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (!err)
                {
                    // Action depends on the response
                    if (data->response == psifs_ASYNC_RESPONSE_QUIET)
                    {
                        // Default is to copy both older and newer files
                        data->quiet = TRUE;
                        data->response = psifs_ASYNC_RESPONSE_COPY;
                    }
                    if (data->response == psifs_ASYNC_RESPONSE_COPY)
                    {
                        // Copy to scrap file
                        data->response = psifs_ASYNC_RESPONSE_CONTINUE;
                        if (data->scrap_tar)
                        {
                            data->status = psifs_ASYNC_DELEGATE;
                            op.data.tar_complete.status = psifs_ASYNC_TAR_SCRAP;
                            err = tar_copy(data->prev_tar, data->scrap_tar);
                        }
                        else
                        {
                            /*
                            data->status = psifs_ASYNC_DELEGATE;
                            op.data.tar_complete.status = psifs_ASYNC_TAR_SKIP;
                            */
                            err = tar_skip(data->prev_tar);
                        }
                    }
                    else if (data->response == psifs_ASYNC_RESPONSE_SKIP)
                    {
                        // Copy to destination file
                        data->response = psifs_ASYNC_RESPONSE_CONTINUE;
                        data->status = psifs_ASYNC_DELEGATE;
                        op.data.tar_complete.status = psifs_ASYNC_TAR_KEEP;
                        err = tar_copy(data->prev_tar, data->dest_tar);
                        if (!err)
                        {
                            err = backtree_ignore(data->tree, &data->info);
                        }
                    }
                    else if (data->response == psifs_ASYNC_RESPONSE_CONTINUE)
                    {
                        const fs_info *info;
                        const char *str;
                        backtree_result result;
                        bits done;
                        bits remain;
                        bits step;

                        err = tar_info(data->prev_tar, &info);
                        if (!err && info)
                        {
                            if (sizeof(data->info.name) <= strlen(data->op.data.backup_prev.src) + strlen(info->name) + 1) err = &err_bad_name;
                            else
                            {
                                sprintf(data->info.name, "%s%c%s", data->op.data.backup_prev.src, FS_CHAR_SEPARATOR, info->name);
                                err = async_tidy_args(data->info.name, &str);
                            }
                            if (!err)
                            {
                                strcpy(data->detail, str);
                                data->info = *info;
                                err = tar_position(data->prev_tar, &done, &remain, &step);
                            }
                            if (!err) err = async_mark_time(data, done, remain, step);
                            if (!err) err = backtree_check(data->tree, info, &result);
                            if (!err)
                            {
                                if (result == BACKTREE_SAME)
                                {
                                    data->response = psifs_ASYNC_RESPONSE_SKIP;
                                }
                                else if ((result == BACKTREE_NEWER)
                                         && !data->quiet)
                                {
                                    data->status = psifs_ASYNC_WAIT_NEWER;
                                }
                                else
                                {
                                    data->response = psifs_ASYNC_RESPONSE_COPY;
                                }
                            }
                        }
                        else data->status = psifs_ASYNC_SUCCESS;
                    }
                    if (!err && (data->status == psifs_ASYNC_DELEGATE))
                    {
                        data->recurse = TRUE;
                        op.op = ASYNC_TAR_COMPLETE;
                        op.data.tar_complete.handle = data->prev_tar;
                        if (sizeof(op.data.tar_complete.detail) <= strlen(data->detail)) err = &err_bad_name;
                        else
                        {
                            strcpy(op.data.tar_complete.detail, data->detail);
                            err = async_new(data, &op, NULL);
                        }
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->dest_tar)
                {
                    // Close and if appropriate delete the output file
                    tar_close(&data->dest_tar);
                    if (data->status != psifs_ASYNC_SUCCESS)
                    {
                        xosfscontrol_wipe(data->op.data.backup_prev.dest,
                                          osfscontrol_WIPE_FORCE,
                                          0, 0, 0, 0);
                    }
                }
                if (data->scrap_tar)
                {
                    // Close and if appropriate delete the scrap backup file
                    tar_close(&data->scrap_tar);
                    if (data->status != psifs_ASYNC_SUCCESS)
                    {
                        xosfscontrol_wipe(data->op.data.backup_prev.scrap,
                                          osfscontrol_WIPE_FORCE,
                                          0, 0, 0, 0);
                    }
                }
                if (data->prev_tar)
                {
                    // Close the previous backup file
                    tar_close(&data->prev_tar);
                }
                if (data->tree != BACKTREE_NONE)
                {
                    // Destroy the backup tree
                    backtree_destroy(&data->tree);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a copying backup files operation by a single stage.
*/
static os_error *async_process_backup_copy(async_data *data, os_error *err,
                                           async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        static async_op op;

        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->tree = BACKTREE_NONE;
                data->dest_tar = NULL;
                err = backtree_clone(data->op.data.backup_copy.tree,
                                     &data->tree);
                if (!err)
                {
                    err = tar_open_out(data->op.data.backup_copy.dest,
                                       &data->dest_tar, TRUE);
                }
                if (!err)
                {
                    bits files;
                    bits size;

                    err = backtree_count(data->tree, &files, &size);
                    if (!err)
                    {
                        data->index = 0;
                        data->num = files * ASYNC_FILE_WEIGHT
                                    + size * ASYNC_FILE_SIZE_WEIGHT
                                    + files * ASYNC_TAR_WEIGHT
                                    + size * ASYNC_TAR_SIZE_WEIGHT;
                    }
                }
                if (!err) data->response = psifs_ASYNC_RESPONSE_CONTINUE;
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (err)
                {
                    if (data->child && (data->child->op.op == psifs_ASYNC_READ))
                    {
                        // Failed to read file
                        data->status = psifs_ASYNC_WAIT_READ;
                        data->err = *err;
                        err = NULL;
                    }
                }
                else
                {
                    if (data->response == psifs_ASYNC_RESPONSE_SKIP)
                    {
                        // Skip over the file that failed
                        data->response = psifs_ASYNC_RESPONSE_CONTINUE;
                        err = backtree_ignore(data->tree, &data->info);
                    }
                    else if (data->response == psifs_ASYNC_RESPONSE_RETRY)
                    {
                        // Retry reading the file that failed
                        op = data->child->op;
                        data->response = psifs_ASYNC_RESPONSE_COPY;
                        data->status = psifs_ASYNC_DELEGATE;
                        err = async_new(data, &op, NULL);
                    }
                    else if (data->response == psifs_ASYNC_RESPONSE_COPY)
                    {
                        // File has been read
                        data->response = psifs_ASYNC_RESPONSE_CONTINUE;
                        err = async_abort(data->child);
                        if (!err)
                        {
                            err = backtree_ignore(data->tree, &data->info);
                        }
                        if (!err)
                        {
                            err = tar_add(data->op.data.backup_copy.temp,
                                          data->info.name, data->dest_tar);
                        }
                        if (!err)
                        {
                            data->recurse = TRUE;
                            data->status = psifs_ASYNC_DELEGATE;
                            op.op = ASYNC_TAR_COMPLETE;
                            op.data.tar_complete.handle = data->dest_tar;
                            op.data.tar_complete.status = psifs_ASYNC_TAR_ADD;
                            if (sizeof(op.data.tar_complete.detail) <= strlen(data->detail)) err = &err_bad_name;
                            else
                            {
                                strcpy(op.data.tar_complete.detail,
                                       data->detail);
                                err = async_new(data, &op, NULL);
                            }
                            if (!err)
                            {
                                bits size = ASYNC_TAR_WEIGHT + (data->info.obj_type != fileswitch_IS_DIR ? data->info.size * ASYNC_TAR_SIZE_WEIGHT : 0);

                                err = async_mark_time(data, data->index, data->num - data->index, size);
                                if (!err) data->index += size;
                            }
                        }
                    }
                    else if (data->response == psifs_ASYNC_RESPONSE_CONTINUE)
                    {
                        const char *str;
                        const fs_info *info;

                        err = backtree_enumerate(data->tree, &info);
                        if (!err && info)
                        {
                            data->response = psifs_ASYNC_RESPONSE_COPY;
                            data->info = *info;
                            data->recurse = TRUE;
                            data->status = psifs_ASYNC_DELEGATE;
                            op.op = psifs_ASYNC_READ;
                            if (sizeof(op.data.read.src) <= strlen(data->op.data.backup_copy.src) + strlen(info->name) + 1) err = &err_bad_name;
                            else
                            {
                                sprintf(op.data.read.src, "%s%c%s", data->op.data.backup_copy.src, FS_CHAR_SEPARATOR, info->name);
                                err = async_tidy_args(op.data.read.src, &str);
                            }
                            if (!err)
                            {
                                strcpy(data->detail, str);
                                strcpy(op.data.read.dest,
                                       data->op.data.backup_copy.temp);
                                err = async_new(data, &op, NULL);
                            }
                            if (!err)
                            {
                                bits size = ASYNC_FILE_WEIGHT + (info->obj_type != fileswitch_IS_DIR ? info->size * ASYNC_FILE_SIZE_WEIGHT : 0);

                                err = async_mark_time(data, data->index, data->num - data->index, size);
                                if (!err) data->index += size;
                            }
                        }
                        else data->status = psifs_ASYNC_SUCCESS;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->child)
                {
                    // End any children
                    async_abort(data->child);
                }
                if (data->dest_tar)
                {
                    // Close the output file
                    tar_close(&data->dest_tar);
                }
                if (data->tree != BACKTREE_NONE)
                {
                    // Destroy the backup tree
                    backtree_destroy(&data->tree);
                }
                xosfscontrol_wipe(data->op.data.backup_copy.temp,
                                  osfscontrol_WIPE_FORCE,
                                  0, 0, 0, 0);
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a complete tar file operation by a single stage.
*/
static os_error *async_process_tar_complete(async_data *data, os_error *err,
                                            async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                data->dest_tar = NULL;
                err = tar_clone(data->op.data.tar_complete.handle,
                                &data->dest_tar);
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (!err)
                {
                    bits done;
                    bits remain;
                    bits step;

                    // Progress the tar file operation
                    err = tar_continue(data->dest_tar, &done, &remain, &step);
                    if (!err)
                    {
                        if (remain)
                        {
                            // Delay before continuing operation
                            data->status = data->op.data.tar_complete.status;
                            strcpy(data->detail,
                                   data->op.data.tar_complete.detail);
                            err = timer_back(util_time() + ASYNC_TAR_DELAY,
                                             data, async_callback);
                            if (!err)
                            {
                                err = async_mark_time(data, done, remain, step);
                            }
                        }
                        else data->status = psifs_ASYNC_SUCCESS;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                if (data->dest_tar)
                {
                    // Close the tar file
                    tar_close(&data->dest_tar);
                }
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  err           - Any error produced by an operation.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress a find file operation by a single stage.
*/
static os_error *async_process_find(async_data *data, os_error *err,
                                    async_stage stage)
{
    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        const char *str;

        // Action depends on the current stage
        switch (stage)
        {
            case ASYNC_INITIALISE:
                // Operation initialising
                if ((data->op.data.find.path[0] != FS_CHAR_DISC)
                    || (((data->op.data.find.path[1] < psifs_DRIVE_FIRST_UPPER)
                         || (psifs_DRIVE_LAST_UPPER < data->op.data.find.path[1]))
                        && (data->op.data.find.path[1] != FS_CHAR_DRIVE_ALL))
                    || (data->op.data.find.path[2] != FS_CHAR_SEPARATOR)
                    || (data->op.data.find.path[3] != FS_CHAR_ROOT))
                {
                    err = &err_bad_name;
                }
                else if (data->op.data.find.path[1] == FS_CHAR_DRIVE_ALL)
                {
                    // Loop through all possible drives
                    data->op.data.find.drive = psifs_DRIVE_FIRST_UPPER;
                    data->num = psifs_DRIVE_LAST_UPPER
                                - psifs_DRIVE_FIRST_UPPER + 1;
                }
                else
                {
                    // Just check a single specified drive
                    data->op.data.find.drive = data->op.data.find.path[1];
                    data->num = 1;
                }
                if (!err)
                {
                    data->index = 0;
                    data->status = psifs_ASYNC_BUSY;
                }
                break;

            case ASYNC_PROCESS:
                // Operation is progressing
                if (!err && data->index
                    && (data->data.cache.reply.info.info.obj_type
                        != fileswitch_NOT_FOUND))
                {
                    // Object found on the current drive
                    data->status = psifs_ASYNC_SUCCESS;
                }
                else if (!err
                         || ERR_EQ(*err, err_drive_empty)
                         || ERR_EQ(*err, err_not_found))
                {
                    err = NULL;
                    if (data->index < data->num)
                    {
                        // Try the next drive letter
                        data->data.cache.cmd.op = CACHE_INFO;
                        data->status = psifs_ASYNC_CAT_READ;
                        if (data->index) data->op.data.find.drive++;
                        strcpy(data->data.cache.cmd.data.info.path,
                               data->op.data.find.path);
                        data->data.cache.cmd.data.info.path[1] = data->op.data.find.drive;
                        err = async_tidy_args(data->data.cache.cmd.data.info.path, &str);
                        if (!err)
                        {
                            strcpy(data->detail, str);
                            err = cache_back(&data->data.cache.cmd,
                                             &data->data.cache.reply,
                                             data, async_callback);
                        }
                        if (!err) err = async_mark_time(data, data->index, data->num - data->index, 1);
                        if (!err) data->index++;
                    }
                    else
                    {
                        // Object not found on any drive
                        data->op.data.find.drive = FS_CHAR_DRIVE_ALL;
                        data->status = psifs_ASYNC_SUCCESS;
                    }
                }
                break;

            case ASYNC_ABORT:
                // Operation should be aborted
                if (!err) data->status = psifs_ASYNC_ABORTED;
                break;

            case ASYNC_PRE_FINALISE:
                // Result is about to be used
                break;

            case ASYNC_POST_FINALISE:
                // Operation is about to be deleted
                break;

            default:
                // Not an expected stage
                err = &err_bad_async_state;
        }
    }

    DEBUG_ERR(err);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Suspend or resume background cache updates.
*/
static os_error *async_suspend(async_data *data)
{
    os_error *err = NULL;

    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        bool suspend;

        // Determine whether updates should be suspended
        suspend = data->suspend && !async_done(data) && !async_paused(data)
                  && !async_waiting(data) && !data->child;

        // No action unless state changed
        if (data->suspended != suspend)
        {
            // Update the suspension state
            if (suspend) cache_disable++;
            else cache_disable--;

            // Store the new setting
            data->suspended = suspend;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  stage         - The stage of the operation to process.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress the specified operation by a single stage.
*/
static os_error *async_process_op(async_data *data, async_stage stage)
{
    os_error *err = NULL;

    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Asynchronous process op data=%p, stage=%u", data, stage))

        // Update any relevant pollwords
        if (!err) err = pollword_update(psifs_MASK_LINK_ASYNC_STATE);
        if (!err && async_done(data))
        {
            err = pollword_update(psifs_MASK_LINK_ASYNC_END);
        }

        // Pause the operation if required
        if (!err && data->pause)
        {
            data->status = psifs_ASYNC_PAUSED;
        }
        else
        {
            // Ensure that the timer if started
            if (!err) err = async_start_time(data);

            // Activate and clear any stored error
            if (!err && data->has_err)
            {
                err = &data->err;
                data->has_err = FALSE;
            }

            // Action depends on the operation
            switch (data->op.op)
            {
                case psifs_ASYNC_SHUTDOWN:
                    // Close all open files on a drive
                    err = async_process_shutdown(data, err, stage);
                    break;

                case psifs_ASYNC_RESTART:
                    // Open the specified files
                    err = async_process_restart(data, err, stage);
                    break;

                case psifs_ASYNC_READ:
                    // Read the specified file
                    err = async_process_read(data, err, stage);
                    break;

                case psifs_ASYNC_WRITE:
                    // Write the specified file
                    err = async_process_write(data, err, stage);
                    break;

                case psifs_ASYNC_BACKUP:
                    // Backup a single directory tree
                    err = async_process_backup(data, err, stage);
                    break;

                case psifs_ASYNC_WRITE_START:
                    // Write and start a single file
                    err = async_process_write_start(data, err, stage);
                    break;

                case psifs_ASYNC_INSTALL:
                    // Write and install a file
                    err = async_process_install(data, err, stage);
                    break;

                case ASYNC_BACKUP_LIST:
                    // Build the tree of files to backup
                    err = async_process_backup_list(data, err, stage);
                    break;

                case ASYNC_BACKUP_PREV:
                    // Process the previous backup file
                    err = async_process_backup_prev(data, err, stage);
                    break;

                case ASYNC_BACKUP_COPY:
                    // Copy files to backup
                    err = async_process_backup_copy(data, err, stage);
                    break;

                case ASYNC_TAR_COMPLETE:
                    // Completing a tar file operation
                    err = async_process_tar_complete(data, err, stage);
                    break;

                case ASYNC_FIND:
                    // Find a file
                    err = async_process_find(data, err, stage);
                    break;

                default:
                    // Not a supported asynchronous operation
                    err = &err_bad_async_op;
                    break;
            }

            // Record any error produced
            if (err)
            {
                // Store the error within the operation
                data->status = psifs_ASYNC_ERROR;
                data->has_err = TRUE;
                data->err = *err;

                // Do not return the error
                err = NULL;
            }
        }

        // Update the suspension state
        if (!err) err = async_suspend(data);

        // Stop the timer if appropriate
        if (!err && (async_done(data) || async_paused(data)
                     || async_waiting(data) || data->child))
        {
            err = async_stop_time(data);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Destroy an operation.
*/
static os_error *async_free(async_data *data)
{
    os_error *err = NULL;

    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Asynchronous free data=%p", data))

        // Unlink the structure
        if (data->prev) data->prev->next = data->next;
        else async_head = data->next;
        if (data->next) data->next->prev = data->prev;

        // Free the memory
        if (data->buffer.ptr) MEM_FREE(data->buffer.ptr);
        MEM_FREE(data);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Progress the specified operation as far as possible.
*/
static os_error *async_process(async_data *data)
{
    os_error *err = NULL;

    // Check parameters
    if (!data) err = &err_bad_parms;
    else if (data->threaded)
    {
        DEBUG_PRINTF(("Asynchronous process data=%p (threaded)", data))

        // Processing is delayed until unthreaded
    }
    else
    {
        DEBUG_PRINTF(("Asynchronous process data=%p", data))

        // Set the threaded flag
        data->threaded = TRUE;

        // Keep processing until no more actions possible
        while (!err && async_idle(data))
        {
            async_stage stage;

            // Choose the stage of operation
            if (!data->initialised)
            {
                idle_start();
                stage = ASYNC_INITIALISE;
                data->initialised = TRUE;
            }
            else if (data->abort) stage = ASYNC_ABORT;
            else stage = ASYNC_PROCESS;

            // Process the operation
            err = async_process_op(data, stage);
        }

        // Special case if just finished
        if (!err && !data->finalised && async_done(data))
        {
            // Tidy up if required
            idle_end();
            data->finalised = TRUE;
            err = async_process_op(data, ASYNC_PRE_FINALISE);

            // Notify any parent
            if (!err && data->parent)
            {
                data->parent->time_acc += data->time_acc;
                err = async_callback(data->parent,
                                     data->has_err ? &data->err : NULL, data);
            }
        }

        // Delete the operation if aborted
        if (!err && data->abort && async_done(data))
        {
            // Perform any clean-up required
            err = async_process_op(data, ASYNC_POST_FINALISE);

            // Free the memory
            if (!err)
            {
                err = async_free(data);
                data = NULL;
            }
        }

        // Clear the threaded flag if operation still exists
        if (data) data->threaded = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : parent        - An optional pointer to a parent operation
                                  data structure, or NULL if none.
                  op            - Details for the operation to perform.
                                  A temporary structure may be used since the
                                  contents will be copied.
                  data          - Variable to receive a pointer to the
                                  operation data structure.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a new operation.
*/
static os_error *async_new(async_data *parent, const async_op *op,
                           async_data **data)
{
    os_error *err = NULL;
    async_data *ptr;

    DEBUG_PRINTF(("Asynchronous new parent=%p, op=%p", parent, op))

    // Ensure that any other children of the same parent are aborted
    if (parent && parent->child) err = async_abort(parent->child);

    // Attempt to allocate memory for the operation data structure
    if (!err)
    {
        ptr = (async_data *) MEM_MALLOC(sizeof(async_data));
        if (!ptr) err = &err_buffer;
    }

    // Allocate a unique handle
    if (!err)
    {
        int handle;

        // Attempt to read the last handle system variable
        if (sysvar_read_int(ASYNC_VAR_NEXT_HANDLE, &handle)) handle = 0;

        // Choose the next hadle
        handle++;
        if (handle < 0) handle = 0;
        if (handle == psifs_ASYNC_INVALID) handle++;
        ptr->handle = handle;

        // Update the last handle system variable
        err = sysvar_write_int(ASYNC_VAR_NEXT_HANDLE, handle);
    }

    // Fill in the details for this operation
    if (!err)
    {
        // Fill in the details of the structure
        ptr->status = psifs_ASYNC_BUSY;
        ptr->initialised = FALSE;
        ptr->finalised = FALSE;
        ptr->abort = FALSE;
        ptr->pause = FALSE;
        ptr->threaded = FALSE;
        ptr->recurse = FALSE;
        ptr->suspend = FALSE;
        ptr->suspended = FALSE;
        ptr->response = psifs_ASYNC_RESPONSE_CONTINUE;
        ptr->quiet = parent ? parent->quiet : FALSE;
        *ptr->detail = '\0';
        ptr->has_err = FALSE;
        ptr->has_stored_err = FALSE;
        ptr->op = *op;
        ptr->time_acc = 0;
        ptr->time_done = 0;
        ptr->time_running = FALSE;
        ptr->pretty_total = 0;
        ptr->pretty_base = 0;
        ptr->pretty_error = 0;
        ptr->pretty_last = 0;
        ptr->frac_done = FRAC_ZERO;
        ptr->frac_step = FRAC_ZERO;
        ptr->buffer.ptr = NULL;
        ptr->buffer_size = 0;

        // Link in the new structure
        ptr->parent = parent;
        if (parent) parent->child = ptr;
        ptr->child = NULL;
        ptr->next = async_head;
        if (async_head) async_head->prev = ptr;
        ptr->prev = NULL;
        async_head = ptr;
    }

    // Start processing the operation
    if (!err) err = async_process(ptr);

    // Set the return value
    if (data) *data = err ? NULL : ptr;

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Abort an operation.
*/
static os_error *async_abort(async_data *data)
{
    os_error *err = NULL;

    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        DEBUG_PRINTF(("Asynchronous abort data=%p", data))

        // Set the abort flag
        data->abort = TRUE;

        // Unpause the operation
        data->pause = FALSE;
        if (async_paused(data)) data->status = psifs_ASYNC_BUSY;

        // Cancel any pending query
        if (async_waiting(data))
        {
            data->status = psifs_ASYNC_BUSY;
            data->response = psifs_ASYNC_RESPONSE_CONTINUE;
        }

        // Unlink from any parent
        if (data->parent)
        {
            data->parent->child = NULL;
            data->parent = NULL;
        }

        // Abort any children
        if (data->child)
        {
            if (!async_done(data)) data->status = psifs_ASYNC_BUSY;
            err = async_abort(data->child);
        }

        // Attempt to process the operation
        if (!err) err = async_process(data);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : op            - Details of the operation to perform.
                                  A temporary structure may be used since the
                                  contents will be copied.
                  handle        - Variable to receive a handle for this
                                  operation.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start an asynchronous operation.
*/
os_error *async_start(const async_op *op, psifs_async_handle *handle)
{
    os_error *err = NULL;

    // Check parameters
    if (!op) err = &err_bad_parms;
    else
    {
        async_data *data;

        DEBUG_PRINTF(("Asynchronous start op=%u", op->op))

        // Attempt to create a new asynchronous operation
        err = async_new(NULL, op, &data);

        // Return the handle
        if (handle) *handle = err ? psifs_ASYNC_INVALID : data->handle;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The previously allocated handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End an asynchronous operation.
*/
os_error *async_end(psifs_async_handle handle)
{
    os_error *err = NULL;
    async_data *data;

    DEBUG_PRINTF(("Asynchronous end handle=%u", handle))

    // Attempt to find the data associated with the specified handle
    err = async_find(handle, &data);

    // Abort the operation
    if (!err) err = async_abort(data);

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The previously allocated handle.
                  status        - Variable to receive the status.
                  desc          - Variable to receive a pointer to the
                                  description of the status.
                  detail        - Variable to receive a pointer to the
                                  details of the status.
                  error         - Variable to receive a pointer to any
                                  error text.
                  taken         - Variable to receive the number of
                                  centi-seconds taken so far.
                  remain        - Variable to receive an estimate of the number
                                  of centi-seconds remaining.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Poll the status of an asynchronous operation.
*/
os_error *async_poll(psifs_async_handle handle, psifs_async_status *status,
                     const char **desc, const char **detail,
                     const char **error, bits *taken, bits *remain)
{
    os_error *err = NULL;

    // Check parameters
    if (!status || !desc || !detail || !error || !taken || !remain)
    {
        err = &err_bad_parms;
    }
    else
    {
        async_data *data;

        // DEBUG_PRINTF(("Asynchronous poll handle=%u", handle))

        // Attempt to find the data associated with the specified handle
        err = async_find(handle, &data);

        // Calculate timings
        if (!err) err = async_time_pretty(data, taken, remain);

        // Recurse if appropriate
        if (!err && !async_done(data))
        {
            while (!err && data->recurse && data->child
                   && !async_done(data->child))
            {
                // Process the child instead
                data = data->child;
            }
        }

        // Construct the descriptions
        if (!err) err = async_desc(data, status, desc, detail, error);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The previously allocated handle.
                  response      - The reponse.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Process a response to a query.
*/
os_error *async_response(psifs_async_handle handle,
                         psifs_async_response response)
{
    os_error *err = NULL;
    async_data *data;

    DEBUG_PRINTF(("Asynchronous response handle=%u, response=%u", handle, response))

    // Attempt to find the data associated with the specified handle
    err = async_find(handle, &data);

    // Recurse if appropriate
    while (!err && !async_waiting(data) && data->child)
    {
        // Process the child instead
        data = data->child;
    }

    // No action if not waiting for response
    if (!err && async_waiting(data))
    {
        // Store the response
        data->response = response;

        // Start processing the response
        data->status = psifs_ASYNC_BUSY;
        err = async_process(data);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The previously allocated handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Pause an operation.
*/
os_error *async_pause(psifs_async_handle handle)
{
    os_error *err = NULL;
    async_data *data;

    DEBUG_PRINTF(("Asynchronous pause handle=%u", handle))

    // Attempt to find the data associated with the specified handle
    err = async_find(handle, &data);

    // Mark all children as paused
    while (!err && data)
    {
        // Mark the operation as paused
        data->pause = TRUE;

        // Process the child
        data = data->child;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - The previously allocated handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Resume a previously paused operation.
*/
os_error *async_resume(psifs_async_handle handle)
{
    os_error *err = NULL;
    async_data *data;

    DEBUG_PRINTF(("Asynchronous resume handle=%u", handle))

    // Attempt to find the data associated with the specified handle
    err = async_find(handle, &data);

    // Mark all children as not paused
    while (!err && data && !async_paused(data))
    {
        // Mark the operation as not paused
        data->pause = FALSE;

        // Process the child
        data = data->child;
    }

    // Resume the paused operation
    if (!err && data)
    {
        data->pause = FALSE;
        data->status = psifs_ASYNC_BUSY;
        err = async_process(data);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to an operation data structure.
                  level         - Level of recursion.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the specified asynchronous
                  operation and any children.
*/
static os_error *async_status_recurse(async_data *data, bits level)
{
    os_error *err = NULL;

    // Check parameters
    if (!data) err = &err_bad_parms;
    else
    {
        psifs_async_status status;
        const char *desc;
        const char *detail;
        const char *error;

        // Display the line start
        if (!level) printf("Asynchronous operation");
        else printf("... Using suboperation");
        printf(" %010u ", data->handle);

        // Display a description
        switch (data->op.op)
        {
            case psifs_ASYNC_SHUTDOWN:
                // Close all open files on a drive
                printf("shutting files '%s',",
                       data->op.data.shutdown.pattern);
                if (data->op.data.shutdown.append) printf(" appending");
                printf(" to '%s'", data->op.data.shutdown.path);
                break;

            case psifs_ASYNC_RESTART:
                // Open the specified files
                printf("restarting files from '%s'",
                       data->op.data.restart.path);
                if (data->op.data.restart.remove) printf(", to be deleted");
                break;

            case psifs_ASYNC_READ:
                // Read the specified file
                printf("reading '%s' as '%s'",
                       data->op.data.read.src, data->op.data.read.dest);
                break;

            case psifs_ASYNC_WRITE:
                // Write the specified file
                printf("writing '%s' as '%s'",
                       data->op.data.write.src, data->op.data.write.dest);
                if (data->op.data.write.remove) printf(", deleting source");
                break;

            case psifs_ASYNC_BACKUP:
                // Backup a single directory tree
                printf("backup '%s'", data->op.data.backup.src);
                if (*data->op.data.backup.prev)
                {
                    printf(" from '%s'", data->op.data.backup.prev);
                }
                printf(" to '%s' using '%s'",
                       data->op.data.backup.dest, data->op.data.backup.temp);
                break;

            case psifs_ASYNC_WRITE_START:
                // Write and start the specified file
                printf("writing and starting '%s' as '%s'",
                       data->op.data.write_start.src,
                       data->op.data.write_start.dest);
                if (*data->op.data.write_start.exe)
                {
                    printf(" executing '%s'", data->op.data.write_start.exe);
                }
                if (data->op.data.write_start.remove) printf(", deleting source");
                break;

            case psifs_ASYNC_INSTALL:
                // Write and install the specified file
                printf("installing '%s' as '%s' using '%s' from '%s' as '%s'",
                       data->op.data.install.pckg_src,
                       data->op.data.install.pckg_dest,
                       data->op.data.install.inst_exe,
                       data->op.data.install.inst_src,
                       data->op.data.install.inst_dest);
                if (data->op.data.install.inst_remove) printf(", deleting installer source");
                if (data->op.data.install.pckg_remove) printf(", deleting package source");
                break;

            case ASYNC_BACKUP_LIST:
                // Build the tree of files to backup
                printf("listing backup files from '%s' in '%s'",
                       data->op.data.backup_list.src,
                       data->op.data.backup_list.sub);
                break;

            case ASYNC_BACKUP_PREV:
                // Process the previous backup file
                printf("processing previous backup '%s' to '%s'",
                       data->op.data.backup_prev.prev,
                       data->op.data.backup_prev.dest);
                if (*data->op.data.backup_prev.scrap)
                {
                    printf(", scrap to '%s'", data->op.data.backup_prev.scrap);
                }
                break;

            case ASYNC_BACKUP_COPY:
                // Copy files to backup
                printf("copying backup files from '%s' to '%s' using '%s'",
                       data->op.data.backup_copy.src,
                       data->op.data.backup_copy.dest,
                       data->op.data.backup_copy.temp);
                break;

            case ASYNC_TAR_COMPLETE:
                // Completing a tar file operation
                printf("tar file operation");
                break;

            case ASYNC_FIND:
                // Finding a file
                printf("finding file '%s'", data->op.data.find.path);
                break;

            default:
                // Not a supported asynchronous operation
                printf("is an unrecognised operation %u", data->op.op);
                err = &err_bad_async_op;
                break;
        }

        // Display the status
        err = async_desc(data, &status, &desc, &detail, &error);
        if (!err && desc)
        {
            printf(" (%s", desc);
            if (detail) printf(" - %s", detail);
            if (error) printf(" / %s", error);
            if (!data->initialised) printf(", not initialised");
            if (data->finalised) printf(", finalised");
            printf(")");
        }

        // End the details
        printf(".\n");

#ifdef DEBUG

        // Show extra details if debugging enabled
        printf("    [parent=%010u, child=%010u, op=0x%02x, status=0x%04x]\n",
               data->parent ? data->parent->handle : psifs_ASYNC_INVALID,
               data->child ? data->child->handle : psifs_ASYNC_INVALID,
               data->op.op,
               data->status);
        printf("    [acc=%u, done=%u, done=0x%08x, step=0x%08x]\n",
               data->time_acc, data->time_done,
               data->frac_done, data->frac_step);

#endif

        // Recurse through any children
        if (!err && data->child && data->child->parent)
        {
            err = async_status_recurse(data->child, level + 1);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the asynchronous remote
                  operations layer.
*/
os_error *async_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying asynchronous remote operations status"))

    // No action unless asynchronous operations pending
    if (async_head)
    {
        async_data *data = async_head;

        // Display the current asynchronous operations status
        while (!err && data)
        {
            // Only show if a root operation
            if (!data->parent) err = async_status_recurse(data, 0);

            // Advance to the next entry
            data = data->next;
        }
    }

    // Return any error produced
    return err;
}
