/*
    File        : claim.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
    Description : Support for the device claim protocol for the PsiFS filer.

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
#include "claim.h"

// Include oslib header files
#include "oslib/macros.h"

// Include project header files
#include "blockdrive.h"
#include "config.h"
#include "filer.h"
#include "psifs.h"

// Major device number for a serial port
#define CLAIM_MAJOR_DEVICE_SERIAL (2)

// Mode to start after device claimed
static psifs_mode claim_start_mode = psifs_MODE_INACTIVE;
static string claim_start_device;

/*
    Parameters  : major - Variable to receive the major device number.
                  minor - Variable to receive the minor device number.
    Returns     : void
    Description : Read the major and minor device numbers for the configured
                  block driver.
*/
static void claim_get_blockdriver(int &major, int &minor)
{
    // Read the current block driver configuration
    config_store config;
    config.read();

    // Attempt to load the selected block driver
    blockdrive_driver driver;
    bool found = !blockdrive_load(config.get_str(config_tag_driver).c_str(),
                                  &driver, 0);

    // Construct the device number
    major = CLAIM_MAJOR_DEVICE_SERIAL;
    minor = found ? (driver->number + config.get_num(config_tag_port)) : 0;

    // Unload the driver
    if (found) blockdrive_unload(driver);
}

/*
    Parameters  : mode      - The mode to be started.
                  device    - An optional device for the printer mirror.
    Returns     : void
    Description : Send a Message_DeviceClaim wimp message to claim the serial
                  port.
*/
static void claim_start(psifs_mode mode, const string device)
{
    // Store the details of the mode to start
    claim_start_mode = mode;
    claim_start_device = device;

    // Attempt to claim the serial port
    wimp_message message;
    message.action = message_DEVICE_CLAIM;
    message.your_ref = 0;
    claim_get_blockdriver(message.data.device.major, message.data.device.minor);
    filer_msgtrans(message.data.device.info, sizeof(message.data.device.info), mode == psifs_MODE_LINK ? "ClmClLk" : "ClmClPr", filer_msgtrans("_TaskName").c_str());
    message.size = ALIGN(strchr(message.data.device.info, '\0')
                          - (char *) &message + 1);
    wimp_send_message(wimp_USER_MESSAGE_RECORDED, &message, wimp_BROADCAST);
}

/*
    Parameters  : void
    Returns     : void
    Description : Start the remote link after claiming the serial port.
*/
void claim_start_link()
{
    claim_start(psifs_MODE_LINK, "");
}

/*
    Parameters  : device    - An optional device for the printer mirror.
    Returns     : void
    Description : Start the printer mirror after claiming the serial port.
*/
void claim_start_printer(const string device)
{
    claim_start(psifs_MODE_PRINTER, device);
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DeviceClaim wimp message events.
*/
int claim_device_claim(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Only interested in messages from other tasks if PsiFS is active
    psifs_mode mode;
    if ((message->sender != filer_task_handle)
        && !xpsifsget_mode(&mode) && (mode != psifs_MODE_INACTIVE))
    {
        // Read the number of the serial port device being used by PsiFS
        int major;
        int minor;
        claim_get_blockdriver(major, minor);

        // Compare with the devive being claimed
        if ((message->data.device.major == major)
            && (message->data.device.minor == minor))
        {
            // Prevent the device from being claimed
            message->action = message_DEVICE_IN_USE;
            message->your_ref = message->my_ref;
            filer_msgtrans(message->data.device.info, sizeof(message->data.device.info), mode == psifs_MODE_LINK ? "ClmUsLk" : "ClmUsPr");
            message->size = ALIGN(strchr(message->data.device.info, '\0')
                                  - (char *) &message + 1);
            wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
        }
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DeviceInUse wimp message events.
*/
int claim_device_in_use(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Construct an error message
    os_error err;
    err.errnum = 0;
    filer_msgtrans(err.errmess, sizeof(err.errmess), claim_start_mode == psifs_MODE_LINK ? "ClmErLk" : "ClmErPr", message->data.device.info);

    // Cancel starting the remote link or printer mirror
    claim_start_mode = psifs_MODE_INACTIVE;

    // Enable simple error handling
    filer_error_allowed++;

    // Generate the error
    os_generate_error(&err);

    // Restore normal error handling
    filer_error_allowed--;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle unacknowledged messages.
*/
bool claim_acknowledge(wimp_event_no event_code, wimp_block *block,
                       toolbox_block *id_block, void *handle)
{
    bool claimed = FALSE;
    wimp_message *message = &block->message;

    NOT_USED(event_code);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Only interested in Message_DeviceClaim
    if (message->action == message_DEVICE_CLAIM)
    {
        // Enable simple error handling
        filer_error_allowed++;

        // Start the remote link or printer mirror
        switch (claim_start_mode)
        {
            case psifs_MODE_LINK:
                // Enable the remote link
                psifsmode_link();
                break;

            case psifs_MODE_PRINTER:
                // Enable the printer mirror
                psifsmode_printer(claim_start_device.empty() ? 0 : claim_start_device.c_str());
                break;

            default:
                // Not interested in anything else
                break;
        }

        // Restore normal error handling
        filer_error_allowed--;

        // Claim the message
        claimed = TRUE;
    }

    // Claim the event
    return claimed;
}
