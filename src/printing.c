/*
    File        : printing.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
    Description : Remote print job handling for the PsiFS module.

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
#include "printing.h"

// Include clib header files
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// Inlcude oslib header files
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "wprt.h"
#include "debug.h"
#include "err.h"
#include "idle.h"
#include "mem.h"
#include "pollword.h"
#include "util.h"

// Is the print job handling active
static bool printing_active = FALSE;

// Print jobs
typedef struct printing_job
{
    struct printing_job *next;
    struct printing_job *prev;
    psifs_print_job_handle handle;
    bits base;
    psifs_print_job_status status;
    os_fw file;
    bits received;
    bits read;
} printing_job;
static printing_job *printing_job_head = NULL;

// The next page file number to allocate
static bits printing_next_page = 0;

// The next handle to allocate
static psifs_print_job_handle printing_next_handle = 0;

// Directory to contain temporary files
#define PRINTING_TEMP_DIR "<PsiFSScrap$Dir>"
#define PRINTING_TEMP_SUBDIR "<PsiFSScrap$Dir>.PrintJobs"
#define PRINTING_TEMP_PATH "PsiFSScrap:PrintJobs."

/*
    Parameters  : handle            - Handle of the print job.
    Returns     : printing_job *    - Pointer to the print job, or NULL if not
                                      found.
    Description : Find the specified print job.
*/
static printing_job *printing_find(psifs_print_job_handle handle)
{
    printing_job *job = printing_job_head;

    // Try all of the jobs
    while (job && (job->handle != handle)) job = job->next;

    // Return the result
    return job;
}

/*
    Parameters  : page          - Page number.
    Returns     : name          - The file name.
    Description : Construct the file name for the specified page data.
*/
static const char *printing_name(bits page)
{
    static fs_pathname name;

    // Construct the name
    sprintf(name, PRINTING_TEMP_PATH "PJ%08X", page);

    // Return the name
    return name;
}

/*
    Parameters  : job           - Variable to receive a pointer to the job.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Create a new print job.
*/
os_error *printing_new(printing_job **job)
{
    os_error *err = NULL;

    // Check parameters
    if (!job) err = &err_bad_parms;
    else
    {
        // Allocate a new structure
        *job = (printing_job *) MEM_MALLOC(sizeof(printing_job));
        if (!*job) err = &err_buffer;

        // Ensure that the temporary directory exists
        if (!err) err = xosfile_create_dir(PRINTING_TEMP_DIR, 0);
        if (!err) err = xosfile_create_dir(PRINTING_TEMP_SUBDIR, 0);

        // Set the initial fields
        if (!err)
        {
            // Allocate a unique handle
            printing_next_handle++;
            if (printing_next_handle == psifs_PRINT_JOB_INVALID)
            {
                printing_next_handle++;
            }

            // Fill in the details of the structure
            (*job)->handle = printing_next_handle;
            (*job)->base = printing_next_page;
            (*job)->status = psifs_PRINT_JOB_IDLE;
            (*job)->file = 0;
            (*job)->received = 0;
            (*job)->read = 0;
        }

        // Link the structure to the list
        if (!err)
        {
            (*job)->next = printing_job_head;
            (*job)->prev = NULL;
            if (printing_job_head)
            {
                printing_job_head->prev = *job;
            }
            printing_job_head = *job;
        }

        // Release the memory if an error produced
        if (err && *job)
        {
            MEM_FREE(*job);
            *job = NULL;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : job           - Pointer to the print job.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Delete a print job.
*/
os_error *printing_delete(printing_job *job)
{
    os_error *err = NULL;

    // Check parameters
    if (!job) err = &err_bad_parms;
    else
    {
        // Close any open file
        if (job->file)
        {
            xosfind_closew(job->file);
            job->file = 0;
            job->received++;
        }

        // Delete any temporary files
        while (job->read != job->received)
        {
            xosfscontrol_wipe(printing_name(job->base + job->read), osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
            job->read++;
        }

        // Unlink the structure
        if (job->next) job->next->prev = job->prev;
        if (job->prev) job->prev->next = job->next;
        else printing_job_head = job->next;

        // Free the memory
        MEM_FREE(job);

        // Delete the temporary directory if no more print jobs
        if (!printing_job_head)
        {
            xosfscontrol_wipe(PRINTING_TEMP_SUBDIR, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Variable to receive the handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Obtain the handle of the next unclaimed print job.
*/
os_error *printing_next(psifs_print_job_handle *handle)
{
    os_error *err = NULL;

    // Check parameters
    if (!handle) err = &err_bad_parms;
    else
    {
        printing_job *job;

        // Initially no next job found
        *handle = psifs_PRINT_JOB_INVALID;

        // Find the oldest print job that has not been accepted
        for (job = printing_job_head; job; job = job->next)
        {
            if (job->status == psifs_PRINT_JOB_START) *handle = job->handle;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the print job.
                  status        - Variable to receive the status.
                  received      - Variable to receive the number of complete
                                  pages received.
                  read          - Variable to receive the number of pages read.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Poll the status of a print job.
*/
os_error *printing_poll(psifs_print_job_handle handle,
                        psifs_print_job_status *status,
                        bits *received, bits *read)
{
    os_error *err = NULL;
    printing_job *job;

    // Check parameters
    if (!status && !received && !read) err = &err_bad_parms;
    else
    {
        // Find the specified print job
        job = printing_find(handle);

        // Return the status
        if (status) *status = job ? job->status : psifs_PRINT_JOB_IDLE;
        if (received) *received = job ? job->received : 0;
        if (read) *read = job ? job->read : 0;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the print job.
                  name          - Name of the file to write the page data to.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Read a page from a print job.
*/
os_error *printing_read(psifs_print_job_handle handle, const char *name)
{
    os_error *err = NULL;
    printing_job *job;

    // Find the specified print job
    job = printing_find(handle);

    // Check whether a page is available
    if (!job || (job->read == job->received)) err = &err_print_job_no_page;

    // Accept the print job
    if (!err && (job->status == psifs_PRINT_JOB_START))
    {
        job->status = psifs_PRINT_JOB_RECEIVING;
        err = wprt_continue();
    }

    // Either move or delete the page data
    if (!err)
    {
        const char *page = printing_name(job->base + job->read);
        if (name)
        {
            err = xosfscontrol_copy(page, name,
                                    osfscontrol_COPY_RECURSE
                                    | osfscontrol_COPY_FORCE
                                    | osfscontrol_COPY_DELETE
                                    | osfscontrol_COPY_LOOK,
                                    0, 0, 0, 0, NULL);
        }
        else
        {
            err = xosfscontrol_wipe(page, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
        }
    }

    // Update the number of pages read
    if (!err)
    {
        job->read++;
        if (((job->status == psifs_PRINT_JOB_COMPLETE)
             || (job->status == psifs_PRINT_JOB_CANCELLED))
            && (job->read == job->received))
        {
            // Delete the print job when complete and all pages read
            err = printing_delete(job);
        }
    }

    // Update any pollwords
    if (!err) err = pollword_update(psifs_MASK_PRINT_JOB_STATUS);

    // Return any error produced
    return err;
}

/*
    Parameters  : handle        - Handle of the print job.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Cancel a print job.
*/
os_error *printing_cancel(psifs_print_job_handle handle)
{
    os_error *err = NULL;
    printing_job *job;

    // Find the specified print job
    job = printing_find(handle);

    // Cancel the print job if still being received
    if (job && (job->status != psifs_PRINT_JOB_COMPLETE)
        && (job->status != psifs_PRINT_JOB_CANCELLED))
    {
        err = wprt_cancel();
    }

    // Delete the print job
    if (!err && job) err = printing_delete(job);

    // Update any pollwords
    if (!err) err = pollword_update(psifs_MASK_PRINT_JOB_STATUS);

    // Return any error produced
    return err;
}

/*
    Parameters  : data          - Pointer to the received data, or NULL if
                                  none.
                  size          - Size of received data.
                  last_packet   - Is this the end of the current page.
                  last_page     - Is this part of the last page in the job.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Process data received from the remote printing server.
*/
os_error *printing_data(const byte *data, bits size,
                        bool last_packet, bool last_page)
{
    os_error *err = NULL;
    printing_job *job = printing_job_head;

    // Reset the idle timeout
    idle_kick();

    // Create a new print job if appropriate
    if (!job || (job->status == psifs_PRINT_JOB_COMPLETE)
        || (job->status == psifs_PRINT_JOB_CANCELLED))
    {
        if (!err) err = printing_new(&job);
    }

    // Start a new page if appropriate
    if (!err && !job->file)
    {
        err = xosfind_openoutw(osfind_NO_PATH | osfind_ERROR_IF_DIR,
                               printing_name(printing_next_page++),
                               NULL, &job->file);
        if (!err && !job->file) err = &err_not_found;
        if (err) job->file = 0;
    }

    // Write the data to the file
    if (!err) err = xosgbpb_writew(job->file, data, size, NULL);

    // Handle end of page
    if (!err && last_packet)
    {
        // Close the file
        xosfind_closew(job->file);
        job->file = 0;

        // End receive of the current page
        job->received++;

        // Handle start of print job
        if (job->status == psifs_PRINT_JOB_IDLE)
        {
            job->status = psifs_PRINT_JOB_START;
        }

        // Handle end of print job
        if (last_page) job->status = psifs_PRINT_JOB_COMPLETE;

        // Update any pollwords
        if (!err) err = pollword_update(psifs_MASK_PRINT_JOB_STATUS);
    }

    // Enable print job to continue if appropriate
    if (!err && (job->status != psifs_PRINT_JOB_START))
    {
        err = wprt_continue();
    }

    // Cancel the print job if there was an error
    if (err) wprt_cancel();

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : The print job was cancelled before completion.
*/
os_error *printing_cancelled(void)
{
    os_error *err = NULL;
    printing_job *job = printing_job_head;

    // No action unless there is an active print job
    if (job)
    {
        // Action depends on the current state of the job
        if ((job->status == psifs_PRINT_JOB_IDLE)
            || (job->status == psifs_PRINT_JOB_START))
        {
            // Discard the print job
            err = printing_delete(job);
        }
        else
        {
            // Set the status to cancelled
            job->status = psifs_PRINT_JOB_CANCELLED;
        }

        // Update any pollwords
        if (!err) err = pollword_update(psifs_MASK_PRINT_JOB_STATUS);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the print job services layer after the multiplexor
                  layer has been started.
*/
os_error *printing_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting print job layer"))

    // No action if already active
    if (!printing_active)
    {
        // Set the active flag if successful
        printing_active = TRUE;

        // Update any pollwords
        if (!err) err = pollword_update(psifs_MASK_PRINT_JOB_STATUS);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the print job services layer before the multiplexor
                  layer has been closed.
*/
os_error *printing_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending print job layer now=%u", now))

    // No action unless active
    if (printing_active)
    {
        // Kill any active print jobs
        while (!err && printing_job_head)
        {
            err = printing_delete(printing_job_head);
        }

        // Clear the active flag
        if (!err) printing_active = FALSE;

        // Update any pollwords
        if (!err) err = pollword_update(psifs_MASK_PRINT_JOB_STATUS);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the print job services layer.
*/
os_error *printing_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying print job layer status"))

    // Display the current print job status
    if (printing_job_head)
    {
        printing_job *job;
        for (job = printing_job_head; job; job = job->next)
        {
            // Display the line start
            printf("Print job %010u ", job->handle);

            // Display the current status
            switch (job->status)
            {
                case psifs_PRINT_JOB_IDLE:      printf("idle");      break;
                case psifs_PRINT_JOB_START:     printf("starting");  break;
                case psifs_PRINT_JOB_RECEIVING: printf("receiving"); break;
                case psifs_PRINT_JOB_COMPLETE:  printf("complete");  break;
                case psifs_PRINT_JOB_CANCELLED: printf("cancelled"); break;
                default:
                    printf("unknown status %u", job->status);
            }

            // Display the page count
            printf(", %u of %u pages processed", job->read, job->received);

            // Extra information about current page
            if (job->file) printf(", receiving next page");

            // End the details
            printf(".\n");
        }
    }
    else if (printing_active)
    {
        printf("No print jobs.\n");
    }
    else printf("Print job handling not active.\n");

    // Return any error produced
    return err;
}
