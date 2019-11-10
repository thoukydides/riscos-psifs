/*
    File        : frame.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Frame manipulation for the link layer of the PsiFS
                  module.

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
#include "frame.h"

// Include clib header files
#include <stdio.h>

// Include project header files
#include "connect.h"
#include "crc.h"
#include "debug.h"
#include "err.h"
#include "escape.h"
#include "link.h"
#include "stats.h"
#include "util.h"

// Uncomment the next line to display frame contents
//#define FRAME_SHOW

// Special characters
#define FRAME_STX (0x02)
#define FRAME_ETX (0x03)
#define FRAME_EOT (0x04)
#define FRAME_DLE (0x10)
#define FRAME_DC1 (0x11)
#define FRAME_DC3 (0x13)
#define FRAME_SYN (0x16)
#define FRAME_ETB (0x17)
#define FRAME_SPC (0x20)
#define FRAME_PNG (0x21)

// Frame transmit and receive states
typedef enum
{
    FRAME_STATE_IDLE,
    FRAME_STATE_START_SYN,
    FRAME_STATE_START_DLE,
    FRAME_STATE_START_STX,
    FRAME_STATE_DATA,
    FRAME_STATE_DATA_STUFF,
    FRAME_STATE_END_ETX,
    FRAME_STATE_END_CRC_HIGH,
    FRAME_STATE_END_CRC_LOW
} frame_states;

// Special data pointers for CONT/SEQ bytes
#define FRAME_PTR_CONT_SEQ (-2)
#define FRAME_PTR_CONT_SEQ_EXT (-1)

// Is the frame handler active
static bool frame_active = FALSE;
static bool frame_polled = FALSE;

// Status of connection to EPOC mobile phone
bool frame_phone = FALSE;
static const char frame_phone_start[] = ""; /* Should be "AT*ESYN=1\r" */
static const char *frame_phone_start_ptr;

// Current transmitter state
static frame_states frame_tx_state = FRAME_STATE_IDLE;
static frame_data frame_tx_data;
static crc_state frame_tx_crc;
static int frame_tx_ptr;
static byte frame_tx_stuff;

// Current receiver state
static frame_states frame_rx_state = FRAME_STATE_IDLE;
static frame_data frame_rx_data;
static crc_state frame_rx_crc;

/*
    Parameters  : value         - Variable to receive the character to
                                  transmit.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Generate the next character to transmit, if any.
*/
static os_error *frame_tx_byte(byte *value)
{
    os_error *err = NULL;

    // Action depends on the current transmitter state
    switch (frame_tx_state)
    {
        case FRAME_STATE_IDLE:
            // No frame to send, so send a dummy NULL
            *value = 0;
            break;

        case FRAME_STATE_START_SYN:
            // Send SYN at start of the frame or AT sequence for EPOC phone
            if (*frame_phone_start_ptr) *value = *frame_phone_start_ptr++;
            else
            {
                *value = frame_phone ? FRAME_ETB : FRAME_SYN;
                frame_tx_state = FRAME_STATE_START_DLE;
            }
            break;

        case FRAME_STATE_START_DLE:
            // Send DLE at start of the frame
            *value = FRAME_DLE;
            frame_tx_state = FRAME_STATE_START_STX;
            break;

        case FRAME_STATE_START_STX:
            // Send STX at start of the frame
            *value = FRAME_STX;
            crc_reset(&frame_tx_crc);
            frame_tx_ptr = FRAME_PTR_CONT_SEQ;
            frame_tx_state = FRAME_STATE_DATA;
            break;

        case FRAME_STATE_DATA:
            // Send CONT/SEQ or data byte, or DLE at end of frame
            if (frame_tx_ptr < frame_tx_data.size)
            {
                if (frame_tx_ptr == FRAME_PTR_CONT_SEQ)
                {
                    *value = frame_tx_data.cont << 4
                             | frame_tx_data.seq & 0x07;
                    if (frame_tx_data.seq < 8) frame_tx_ptr++;
                    else *value |= 0x08;
                }
                else if (frame_tx_ptr == FRAME_PTR_CONT_SEQ_EXT)
                {
                    *value = (frame_tx_data.seq & 0x7F8) >> 3;
                }
                else *value = frame_tx_data.data[frame_tx_ptr];
                frame_tx_ptr++;
                crc_update(&frame_tx_crc, *value);
                if ((*value == FRAME_DLE)
                    || (connect_era && (*value == FRAME_ETX))
                    || (frame_phone && ((*value == FRAME_DC1)
                                        || (*value == FRAME_DC3))))
                {
                    frame_tx_stuff = *value;
                    *value = FRAME_DLE;
                    frame_tx_state = FRAME_STATE_DATA_STUFF;
                }
            }
            else
            {
                *value = FRAME_DLE;
                frame_tx_state = FRAME_STATE_END_ETX;
            }
            break;

        case FRAME_STATE_DATA_STUFF:
            // Stuff the current CONT/SEQ or data byte
            if (frame_tx_stuff == FRAME_ETX) *value = FRAME_EOT;
            else if (frame_tx_stuff == FRAME_DC1) *value = FRAME_SPC;
            else if (frame_tx_stuff == FRAME_DC3) *value = FRAME_PNG;
            else *value = frame_tx_stuff;
            frame_tx_state = FRAME_STATE_DATA;
            break;

        case FRAME_STATE_END_ETX:
            // Send the ETX at end of the frame
            *value = FRAME_ETX;
            frame_tx_state = FRAME_STATE_END_CRC_HIGH;
            break;

        case FRAME_STATE_END_CRC_HIGH:
            // Send the high byte of the CRC value
            *value = crc_msb(&frame_tx_crc);
            frame_tx_state = FRAME_STATE_END_CRC_LOW;
            break;

        case FRAME_STATE_END_CRC_LOW:
            // Send the low byte of the CRC value
            *value = crc_lsb(&frame_tx_crc);
            frame_tx_state = FRAME_STATE_IDLE;
            stats_tx_frame++;
            break;

        default:
            // These states should never occur
            *value = NULL;
            frame_tx_state = FRAME_STATE_IDLE;
            err = &err_bad_frame_state;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : value - The unstuffed character.
    Returns     : void
    Description : Process a single received byte after any unstuffing has been
                  performed.
*/
static void frame_rx_byte_data(byte value)
{
    // Update the CRC value with this byte
    crc_update(&frame_rx_crc, value);

    // Decode the CONT/SEQ byte or data
    if (frame_rx_data.size == FRAME_PTR_CONT_SEQ)
    {
        frame_rx_data.cont = (value & 0xf0) >> 4;
        frame_rx_data.seq = value & 0x07;
        if (!(value & 0x08)) frame_rx_data.size++;
    }
    else if (frame_rx_data.size == FRAME_PTR_CONT_SEQ_EXT)
    {
        frame_rx_data.seq |= value << 3;
    }
    else frame_rx_data.data[frame_rx_data.size] = value;

    // Increase the size of the received frame
    frame_rx_data.size++;
}

/*
    Parameters  : value         - The received character.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Update the frame receiver state machine with the specified
                  character.
*/
static os_error *frame_rx_byte(byte value)
{
    os_error *err = NULL;

    // Action depends on the current receiver state
    switch (frame_rx_state)
    {
        case FRAME_STATE_IDLE:
            // A received frame is waiting to be processed, so no action
            break;

        case FRAME_STATE_START_SYN:
            // Only interested in SYN to start a frame
            if (value == FRAME_SYN)
            {
                frame_rx_state = FRAME_STATE_START_DLE;
                frame_phone = FALSE;
            }
            else if (value == FRAME_ETB)
            {
                frame_rx_state = FRAME_STATE_START_DLE;
                frame_phone = TRUE;
            }
            break;

        case FRAME_STATE_START_DLE:
            // Expecting DLE at start of frame
            if (value == FRAME_DLE) frame_rx_state = FRAME_STATE_START_STX;
            else if (value == FRAME_SYN) frame_phone = FALSE;
            else if (value == FRAME_ETB) frame_phone = TRUE;
            else
            {
                frame_rx_state = FRAME_STATE_START_SYN;
                stats_rx_err_frame++;
            }
            break;

        case FRAME_STATE_START_STX:
            // Expecting STX to start transmission
            crc_reset(&frame_rx_crc);
            frame_rx_data.size = FRAME_PTR_CONT_SEQ;
            if (value == FRAME_STX) frame_rx_state = FRAME_STATE_DATA;
            else
            {
                stats_rx_err_frame++;
                if (value == FRAME_SYN)
                {
                    frame_rx_state = FRAME_STATE_START_DLE;
                    frame_phone = FALSE;
                }
                else if (value == FRAME_ETB)
                {
                    frame_rx_state = FRAME_STATE_START_DLE;
                    frame_phone = TRUE;
                }
                else frame_rx_state = FRAME_STATE_START_SYN;
            }
            break;

        case FRAME_STATE_DATA:
            // Expecting CONT/SEQ or data byte, or DLE for stuffing/frame end
            if (value == FRAME_DLE) frame_rx_state = FRAME_STATE_DATA_STUFF;
            else if (frame_rx_data.size < FRAME_MAX_DATA_RX)
            {
                frame_rx_byte_data(value);
            }
            else
            {
                frame_rx_state = FRAME_STATE_START_SYN;
                stats_rx_err_frame++;
            }
            break;

        case FRAME_STATE_DATA_STUFF:
            // Expecting stuffed CONT/SEQ or data byte, or ETX for frame end
            if ((0 <= frame_rx_data.size) && (value == FRAME_ETX))
            {
                frame_rx_state = FRAME_STATE_END_CRC_HIGH;
            }
            else
            {
                if (value == FRAME_EOT) value = FRAME_ETX;
                else if (value == FRAME_SPC) value = FRAME_DC1;
                else if (value == FRAME_PNG) value = FRAME_DC3;
                if (frame_rx_data.size < FRAME_MAX_DATA_RX)
                {
                    frame_rx_byte_data(value);
                    frame_rx_state = FRAME_STATE_DATA;
                }
                else
                {
                    frame_rx_state = FRAME_STATE_START_SYN;
                    stats_rx_err_frame++;
                }
            }
            break;

        case FRAME_STATE_END_CRC_HIGH:
            // Expecting high byte of CRC value
            if (value == crc_msb(&frame_rx_crc))
            {
                frame_rx_state = FRAME_STATE_END_CRC_LOW;
            }
            else
            {
                frame_rx_state = FRAME_STATE_START_SYN;
                stats_rx_err_frame++;
            }
            break;

        case FRAME_STATE_END_CRC_LOW:
            // Expecting low byte of CRC value
            if (value == crc_lsb(&frame_rx_crc))
            {
#ifdef FRAME_SHOW
                DEBUG_PRINTF(("Rx frame CONT=%u, SEQ=%u, size=%i", frame_rx_data.cont, frame_rx_data.seq, frame_rx_data.size))
                {
                    bits i;

                    printf("(Rx");
                    for (i = 0; i < frame_rx_data.size; i++)
                    {
                        printf(" %02x", frame_rx_data.data[i]);
                    }
                    printf(")\n");
                }
#endif
                frame_rx_state = FRAME_STATE_IDLE;
                stats_rx_frame++;
            }
            else
            {
                frame_rx_state = FRAME_STATE_START_SYN;
                stats_rx_err_frame++;
            }
            break;

        case FRAME_STATE_END_ETX:
        default:
            // These states should never occur
            frame_rx_state = FRAME_STATE_START_SYN;
            err = &err_bad_frame_state;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : all           - Should all status be reset, or just the
                                  transmit status.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Reset the frame handler status. This should be called if the
                  connection handler times out, especially if the baud rate is
                  changed.
*/
os_error *frame_reset(bool all)
{
    os_error *err = NULL;

    // Always reset the transmitter status
    frame_phone = FALSE;
    frame_phone_start_ptr = frame_phone_start;
    frame_tx_state = FRAME_STATE_IDLE;

    // Only reset the receive status if necessary
    if (all)
    {
        // Reset the receiver state machine
        frame_rx_state = FRAME_STATE_START_SYN;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : active        - Is the remote device present and active.
                  rx            - The next received character, or negative if
                                  none.
                  tx            - Variable to receive a character to transmit,
                                  or NULL if transmit buffer is full.
                  idle          - Should idle polling of higher layers be
                                  performed.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any polled actions required. This must only be called
                  when a block driver is active and usable.
*/
os_error *frame_poll(bool active, int rx, int *tx, bool idle)
{
    os_error *err = NULL;

    // No action unless active
    if (frame_active)
    {
        static bool prev_tx_ready = FALSE;
        bool tx_ready;
        bool rx_ready;

        // Set the polled status
        frame_polled = TRUE;

        // Special case if not active
        if (!active)
        {
            // Reset the state machines
            err = frame_reset(TRUE);
        }
        else
        {
            // Handle any received character
            if (0 <= rx) err = frame_rx_byte(rx);

            // Transmit a pending character if possible
            if (!err && tx && (frame_tx_state != FRAME_STATE_IDLE))
            {
                byte value;

                err = frame_tx_byte(&value);
                if (!err) *tx = value;
            }
        }

        // Check the transmit and receive status
        tx_ready = active && (frame_tx_state == FRAME_STATE_IDLE);
        rx_ready = frame_rx_state == FRAME_STATE_IDLE;

        // Poll the connection manager
        if (!err && (rx_ready || (tx_ready && !prev_tx_ready) || idle))
        {
            // Perform the poll
            err = connect_poll(active, rx_ready ? &frame_rx_data : NULL,
                               tx_ready);

            // Re-enable the receiver if necessary
            if (rx_ready) frame_rx_state = FRAME_STATE_START_SYN;

            // Remember the transmitter status (which may have changed)
            prev_tx_ready = active && (frame_tx_state == FRAME_STATE_IDLE);
        }

        // Clear the polled status
        frame_polled = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - The message to send.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Abort transmission of any active frame, and transmit the
                  specified message. The message is copied, so the original
                  may be modified or transient.
*/
os_error *frame_send(const frame_data *frame)
{
    os_error *err = NULL;

    // Check the parameters
    if (!frame) err = &err_bad_parms;
    else if (!frame_active) err = &err_no_frame;
    else if (!frame_polled) err = &err_not_poll;
    else
    {
#ifdef FRAME_SHOW
        DEBUG_PRINTF(("Tx frame CONT=%u, SEQ=%u, size=%i", frame->cont, frame->seq, frame->size))
        {
            bits i;

            printf("(Tx");
            for (i = 0; i < frame->size; i++)
            {
                printf(" %02x", frame->data[i]);
            }
            printf(")\n");
        }
#endif

        // Copy the message data
        frame_tx_data = *frame;

        // Start the state machine
        frame_tx_state = FRAME_STATE_START_SYN;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Wait for any current frame being transmitted to complete.
*/
static os_error *frame_wait_idle(void)
{
    os_error *err = NULL;
    escape_config esc;

    DEBUG_PRINTF(("Waiting for frame handler to become idle"))

    // Enable escape conditions
    err = escape_store(&esc);
    if (!err) err = escape_enable();
    if (!err)
    {
        // Keep polling until idle
        while (!err && (frame_tx_state != FRAME_STATE_IDLE))
        {
            err = link_poll(TRUE);
        }

        // Restore the previous escape state
        escape_restore(&esc);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start the frame handler for the link. This resets both the
                  transmitter and receiver.
*/
os_error *frame_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting frame handler"))

    // No action if already active
    if (!frame_active)
    {
        // Reset the status
        err = frame_reset(TRUE);

        // Start the connection manager
        if (!err) err = connect_start();

        // Set the active flag if successful
        if (!err) frame_active = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the frame handler and any clients for the link. This will
                  normally attempt a tidy shutdown, but may be forced to
                  terminate immediately.
*/
os_error *frame_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending frame handler now=%u", now))

    // No action unless active
    if (frame_active)
    {
        // End the connection manager
        err = connect_end(now);

        // Wait for any active frame to complete
        if (!err && !now) err = frame_wait_idle();

        // Clear the active flag if successful
        if (!err) frame_active = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the frame handler and any
                  clients.
*/
os_error *frame_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying frame handler status"))

    // Display the error rate
    printf("%u valid frames", stats_rx_frame);
    if (stats_rx_err_frame)
    {
        printf(" and %u invalid frames", stats_rx_err_frame);
    }
    printf(" received");
    if (stats_rx_retry_frame)
    {
        printf(", including %u retries", stats_rx_retry_frame);
    }
    printf(".\n");
    printf("%u frames transmitted", stats_tx_frame);
    if (stats_tx_retry_frame)
    {
        printf(", including %u retries", stats_tx_retry_frame);
    }
    printf(".\n");

    // Just pass to the connection handler
    err = connect_status();

    // Return any error produced
    return err;
}
