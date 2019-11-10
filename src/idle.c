/*
    File        : idle.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Idle detection and timing for the PsiFS module.

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
#include "idle.h"

// Include system header files
#include <stdio.h>

// Include project header files
#include "cache.h"
#include "debug.h"
#include "util.h"

// Idle configuration
bits idle_disconnect_link = 0;
bits idle_disconnect_printer = 0;
bool idle_disconnect_external = FALSE;
bool idle_background_throttle = FALSE;

// Number of active operations
static bits idle_operations = 0;

// End of the last operation
static os_t idle_last = 0;

// Minimum idle time for displaying status
#define IDLE_MIN_STATUS (300)

// Minimum idle time for throttling background operations
#define IDLE_MIN_THROTTLE (500)

/*
    Parameters  : void
    Returns     : bits      - Number of centi-seconds idle.
    Description : Return the length of time that the remote link has been idle.
*/
static bits idle_time(void)
{
    // Return the idle time
    return idle_operations ? 0 : (util_time() - idle_last);
}

/*
    Parameters  : void
    Returns     : void
    Description : Mark the start of an operation using the remote link. The
                  idle timeout is disabled for the duration of the operation.
                  A count of the number of active operations is maintained,
                  so calls to idle_start and idle_end must be matched.
*/
void idle_start(void)
{
    // Increment the number of operations
    idle_operations++;
}

/*
    Parameters  : void
    Returns     : void
    Description : Mark the end of an operation using the remote link. The
                  idle timeout is re-enabled if there are no other active
                  operations.
*/
void idle_end(void)
{
    // No action unless an operation is in progress
    if (idle_operations)
    {
        // Decrement the number of operations and update the last activity time
        if (!--idle_operations) idle_last = util_time();
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Reset the idle timeout. This is equivalent to calling
                  idle_start followed immediately by idle_end.
*/
void idle_kick(void)
{
    // Update the last activity time
    idle_last = util_time();
}

/*
    Parameters  : time  - Required idle timeout in seconds.
    Returns     : bool  - Should a disconnection be performed.
    Description : Check if an automatic idle disconnection should occur.
*/
bool idle_check_disconnect(bits time)
{
    // Check if a disconnection should occur
    return (0 < time) && ((time * 100) <= idle_time());
}

/*
    Parameters  : void
    Returns     : bool  - Should background operations be throttled back.
    Description : Check if background operations should be reduced.
*/
bool idle_check_throttle(void)
{
    // Check if background operations should be throttled
    return idle_background_throttle && (IDLE_MIN_THROTTLE < idle_time());
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the idle timeout.
*/
os_error *idle_status(void)
{
    os_error *err = NULL;
    bits idle;

    DEBUG_PRINTF(("Displaying idle timeout status"))

    // No action unless idle for a significant period
    idle = idle_time();
    if (IDLE_MIN_STATUS < idle)
    {
        printf("The remote link has been idle for %u seconds.\n", idle / 100);
    }

    // Return any error produced
    return err;
}
