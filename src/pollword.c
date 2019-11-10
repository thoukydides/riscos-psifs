/*
    File        : pollword.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Client pollword handler for the PsiFS module.

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
#include "pollword.h"

// Include clib header files
#include <stddef.h>

// Include project header files
#include "ctrl.h"
#include "debug.h"
#include "err.h"
#include "mem.h"
#include "wimpfilt.h"

// List of pollwords
typedef struct pollword_details
{
    struct pollword_details *next;
    struct pollword_details *prev;
    char *name;
    psifs_mask mask;
    int pollword;
} pollword_details;
static pollword_details *pollword_head = NULL;

/*
    Parameters  : name          - The name of the client.
                  mask          - The mask of changes of interest.
                  pollword      - Variable to receive a pointer to the pollword.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Register a client to be informed of changes.
*/
os_error *pollword_register(const char *name, psifs_mask mask, int **pollword)
{
    os_error *err = NULL;

    // Check parameters
    if (!name || !pollword) err = &err_bad_parms;
    else
    {
        pollword_details *ptr;

        DEBUG_PRINTF(("pollword_register name='%s', mask=0x%08x", name, mask))

        // Allocate memory for the details
        ptr = MEM_MALLOC(sizeof(pollword_details));
        if (!ptr) err = &err_buffer;

        // Duplicate the client name
        if (!err)
        {
            ptr->name = ctrl_strdup(name);
            if (!ptr->name)
            {
                err = &err_buffer;
                MEM_FREE(ptr);
            }
        }

        // Complete the remaining details
        if (!err)
        {
            // Copy the mask and clear the pollword initially
            ptr->mask = mask;
            ptr->pollword = 0;

            // Link the details in
            ptr->next = pollword_head;
            ptr->prev = NULL;
            if (pollword_head) pollword_head->prev = ptr;
            pollword_head = ptr;
        }

        // Set the return value
        *pollword = !err ? &ptr->pollword : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : pollword      - The previously allocated pollword pointer.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Unregister the specified client.
*/
os_error *pollword_unregister(int *pollword)
{
    os_error *err = NULL;

    // Check parameters
    if (!pollword) err = &err_bad_parms;
    else
    {
        pollword_details *ptr;

        DEBUG_PRINTF(("pollword_unregister pollword=%p", pollword))

        // Inform the WIMP filters
        err = wimpfilt_release(pollword);
        if (!err)
        {
            // Convert the pollword to a details pointer
            ptr = (pollword_details *) (((char *) pollword)
                                        - offsetof(pollword_details, pollword));

            // Unlink from the list of pollwords
            if (ptr->next) ptr->next->prev = ptr->prev;
            if (ptr->prev) ptr->prev->next = ptr->next;
            else pollword_head = ptr->next;

            // Free the memory
            MEM_FREE(ptr->name);
            MEM_FREE(ptr);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : mask          - The mask of changes to notify to clients.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Update any pollwords affected by the specified mask.
*/
os_error *pollword_update(psifs_mask mask)
{
    os_error *err = NULL;
    pollword_details *ptr = pollword_head;

    // Loop through all pollwords
    while (ptr)
    {
        // Update this pollword if necessary
        if (ptr->mask & mask) ptr->pollword |= mask;

        // Advance to the next pollword
        ptr = ptr->next;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check if any clients are registered.
*/
os_error *pollword_pre_finalise(void)
{
    os_error *err = NULL;

    // Check if any clients are registered
    if (pollword_head) err = &err_kill_clients;

    // Return any error produced
    return err;
}
