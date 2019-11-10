/*
    File        : connect.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Connection manager for the link layer of the PsiFS
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
#include "connect.h"

// Include clib header files
#include <stdio.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "escape.h"
#include "link.h"
#include "mux.h"
#include "pollword.h"
#include "stats.h"
#include "util.h"

// Frame types
#define CONNECT_CONT_ACK_PDU (0)
#define CONNECT_CONT_DISC_PDU (1)
#define CONNECT_CONT_REQ_PDU (2)
#define CONNECT_CONT_DATA_PDU (3)
#define CONNECT_SEQ_DISC_PDU (0)
#define CONNECT_SEQ_REQ_PDU (0)
#define CONNECT_SEQ_REQ_REQ_PDU (1)
#define CONNECT_SEQ_REQ_CON_PDU (4)
#define CONNECT_SEQ_DATA_PDU_NUM_SIBO (8)
#define CONNECT_SEQ_DATA_PDU_NUM_ERA (2048)

// Connection states
typedef enum
{
    CONNECT_IDLE,
    CONNECT_IDLE_REQ,
    CONNECT_IDLE_ACK,
    CONNECT_DATA,
    CONNECT_DATA_ACK
} connect_states;

// The connection state
bool connect_active = FALSE;
static bool connect_enable = FALSE;
static bool connect_polled = FALSE;
static connect_states connect_state;
bool connect_connected = FALSE;
bool connect_era = FALSE;

// The timer
#define CONNECT_TIMEOUT_IDLE (100 * 60)
#define CONNECT_TIMEOUT_RETRY_BYTES_SCALE (4)
#define CONNECT_TIMEOUT_RETRY_OFFSET (20)
static bool connect_timer;
static int connect_timeout;

// Retry counter
#define CONNECT_REQ_RETRIES (4)
#define CONNECT_DATA_RETRIES (8)
#define CONNECT_DISC_RETRIES (4)
static bits connect_retries;

// Sequence numbers
static bits connect_seq_tx;
static bits connect_seq_rx;

// Pending data and supervisory frames
#define CONNECT_MAX_WINDOW_ERA (5) // Can be up to (32) if useful
#define CONNECT_MAX_WINDOW_SIBO (1)
#define CONNECT_MAX_WINDOW (CONNECT_MAX_WINDOW_ERA)
static bool connect_ctrl_pending;
static frame_data connect_ctrl_frame;
static bits connect_tx_data_pending;
static bits connect_tx_data_head;
static bits connect_tx_data_tail;
static frame_data connect_tx_data_frame[CONNECT_MAX_WINDOW + 1];
static bool connect_rx_data_pending;
static frame_data connect_rx_data_frame;

// Magic number for connection confirm
static bits connect_magic;

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : The remote link has connected so start the multiplexor layer
                  if appropriate.
*/
static os_error *connect_mux_connected(void)
{
    os_error *err = NULL;

    // No action if already connected
    if (!connect_connected)
    {
        // Start by updating any relevant pollwords
        err = pollword_update(psifs_MASK_LINK_STATUS);

        // Start the multiplexor layer if required
        if (!err) err = mux_start();

        // Set the connection state if successful
        if (!err) connect_connected = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : The remote link has been disconnected so end the multiplexor
                  layer if appropriate.
*/
static os_error *connect_mux_disconnected(bool now)
{
    os_error *err = NULL;

    // No action unless already connected
    if (connect_connected)
    {
        // Start by updating any relevant pollwords
        err = pollword_update(psifs_MASK_LINK_STATUS);

        // End the multiplexor layer if required
        if (!err) err = mux_end(now);

        // Clear the connection state if successful
        if (!err) connect_connected = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : void
    Description : Disable the timer.
*/
static void connect_timer_stop(void)
{
    // Stop the timer
    connect_timer = FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Configure and reset the idle timeout timer.
*/
static void connect_timer_idle(void)
{
    // Start the timer
    connect_timeout = util_time() + CONNECT_TIMEOUT_IDLE;
    connect_timer = TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Configure and reset the retry timeout timer.
*/
static void connect_timer_retry(void)
{
    int timeout;

    // Calculate the required timeout
    timeout = CONNECT_TIMEOUT_RETRY_OFFSET;
    if (connect_connected) timeout += link_time((frame_phone ? FRAME_MAX_DATA_RX : FRAME_MAX_DATA_TX) * CONNECT_TIMEOUT_RETRY_BYTES_SCALE);

    // Start the timer
    connect_timeout = util_time() + timeout;
    connect_timer = TRUE;
}

/*
    Parameters  : seq   - The sequence number to use.
    Returns     : void
    Description : Queue an acknowledge frame.
*/
static void connect_tx_ack(bits seq)
{
    // Queue an acknowledge frame
    connect_ctrl_frame.cont = CONNECT_CONT_ACK_PDU;
    connect_ctrl_frame.seq = seq;
    connect_ctrl_frame.size = 0;
    connect_ctrl_pending = TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Queue a disconnect frame.
*/
static void connect_tx_disc(void)
{
    // Queue a disconnect frame
    connect_ctrl_frame.cont = CONNECT_CONT_DISC_PDU;
    connect_ctrl_frame.seq = CONNECT_SEQ_DISC_PDU;
    connect_ctrl_frame.size = 0;
    connect_ctrl_pending = TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Queue a connection request frame.
*/
static void connect_tx_req_req(void)
{
    // Queue a connection request frame
    connect_ctrl_frame.cont = CONNECT_CONT_REQ_PDU;
    connect_ctrl_frame.seq = CONNECT_SEQ_REQ_REQ_PDU;
    connect_ctrl_frame.size = 0;
    connect_ctrl_pending = TRUE;
}

/*
    Parameters  : magic - The magic number to include.
    Returns     : void
    Description : Queue a connection confirm frame.
*/
static void connect_tx_req_con(bits magic)
{
    // Queue a connection confirm frame
    connect_ctrl_frame.cont = CONNECT_CONT_REQ_PDU;
    connect_ctrl_frame.seq = CONNECT_SEQ_REQ_CON_PDU;
    connect_ctrl_frame.size = sizeof(bits);
    *(bits *) connect_ctrl_frame.data = magic;
    connect_ctrl_pending = TRUE;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Reset the state machine and associated status. This does not
                  perform any other actions associated with a disconnection,
                  such as informing the multiplexor layer.
*/
static os_error *connect_reset(void)
{
    os_error *err = NULL;

    // Reset the state machine
    connect_state = CONNECT_IDLE;
    connect_era = FALSE;

    // Enable the timer
    connect_timer_retry();

    // Reset the sequence numbers
    connect_seq_tx = 0;
    connect_seq_rx = 0;

    // No pending transfers
    connect_tx_data_pending = 0;
    connect_tx_data_head = 0;
    connect_tx_data_tail = 0;
    connect_rx_data_pending = FALSE;

    // Choose a magic number for connections
    connect_magic = util_time();

    // Ensure that the multiplexor is ended
    err = connect_mux_disconnected(TRUE);

    // Return any error produced
    return err;
}

/*
    Parameters  : seq   - A sequence number.
    Returns     : bits  - The next sequence number.
    Description : Increment a sequence number.
*/
static bits connect_inc_seq(bits seq)
{
    // Return the incremented sequence number
    return (seq + 1)
           % (connect_era
              ? CONNECT_SEQ_DATA_PDU_NUM_ERA
              : CONNECT_SEQ_DATA_PDU_NUM_SIBO);
}

/*
    Parameters  : ptr   - A transmit window pointer.
    Returns     : bits  - The next transmit window pointer.
    Description : Increment a transmit window pointer.
*/
static bits connect_inc_tx_window(bits ptr)
{
    // Return the incremented sequence number
    return (ptr + 1) % (CONNECT_MAX_WINDOW + 1);
}

/*
    Parameters  : void
    Returns     : bits  - The number of free frames.
    Description : Calculate the number of free frames in the transmit window.
*/
static bits connect_free_tx_window(void)
{
    // Return the number of free frames
    return connect_connected
           ? (connect_era ? CONNECT_MAX_WINDOW_ERA : CONNECT_MAX_WINDOW_SIBO)
             - ((connect_tx_data_head + CONNECT_MAX_WINDOW + 1
                 - connect_tx_data_tail) % (CONNECT_MAX_WINDOW + 1))
           : 0;
}

/*
    Parameters  : frame         - The received frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received acknowledge frame.
*/
static os_error *connect_poll_rx_ack(const frame_data *frame)
{
    os_error *err = NULL;

    // Action depends on the current state
    switch (connect_state)
    {
        case CONNECT_IDLE_REQ:
        case CONNECT_IDLE_ACK:
            // Connection completed by acknowledge
            if (connect_enable)
            {
                connect_seq_tx = frame->seq;
                connect_seq_rx = 0;
                connect_timer_idle();
                connect_state = CONNECT_DATA;
                err = connect_mux_connected();
            }
            break;

        case CONNECT_DATA_ACK:
            // Acknowledge of a data frame
            {
                bits tx = connect_tx_data_tail;
                bool pending = FALSE;

                // Find a transmitted frame with a matching sequence number
                while (tx != connect_tx_data_head)
                {
                    if (tx == connect_tx_data_pending) pending = TRUE;
                    tx = connect_inc_tx_window(tx);
                    if (frame->seq == connect_tx_data_frame[tx].seq)
                    {
                        connect_tx_data_tail = tx;
                        if (pending) connect_tx_data_pending = tx;
                    }
                }

                // Check if all frames have been acknowledged
                if (connect_tx_data_tail == connect_tx_data_head)
                {
                    // All queued frames have been acknowledged
                    connect_state = CONNECT_DATA;
                    connect_timer_idle();
                }
                else
                {
                    // Unacknowledged frames remain
                    connect_timer_retry();
                }
            }
            break;

        case CONNECT_IDLE:
        case CONNECT_DATA:
            // Ignore the acknowledge in other states
            break;

        default:
            // No other states should occur
            err = &err_bad_connect_state;
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - The received frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received disconnect frame.
*/
static os_error *connect_poll_rx_disc(const frame_data *frame)
{
    os_error *err = NULL;

    // Action depends on the current state
    switch (connect_state)
    {
        case CONNECT_IDLE_REQ:
        case CONNECT_IDLE_ACK:
        case CONNECT_DATA:
        case CONNECT_DATA_ACK:
            // Disconnected so reset the state machine
            err = connect_reset();
            break;

        case CONNECT_IDLE:
            // Ignore the disconnect in other states
            break;

        default:
            // No other states should occur
            err = &err_bad_connect_state;
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - The received frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received request frame.
*/
static os_error *connect_poll_rx_req(const frame_data *frame)
{
    os_error *err = NULL;

    // Action depends on the current state
    switch (connect_state)
    {
        case CONNECT_IDLE:
            // SIBO connection request
            if (connect_enable)
            {
                connect_era = FALSE;
                connect_tx_req_con(connect_magic);
                connect_timer_retry();
                connect_retries = CONNECT_REQ_RETRIES;
                connect_state = CONNECT_IDLE_ACK;
            }
            break;

        case CONNECT_IDLE_REQ:
        case CONNECT_IDLE_ACK:
            // SIBO connection request
            if (connect_enable)
            {
                connect_era = FALSE;
                connect_seq_tx = 0;
                connect_seq_rx = 0;
                connect_tx_ack(connect_seq_rx);
                connect_timer_idle();
                connect_state = CONNECT_DATA;
                err = connect_mux_connected();
            }
            break;

        case CONNECT_DATA:
        case CONNECT_DATA_ACK:
            // Should not receive when already connected, so disconnect
            err = connect_reset();
            break;

        default:
            // No other states should occur
            err = &err_bad_connect_state;
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - The received frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received request request frame.
*/
static os_error *connect_poll_rx_req_req(const frame_data *frame)
{
    os_error *err = NULL;

    // Action depends on the current state
    switch (connect_state)
    {
        case CONNECT_IDLE:
        case CONNECT_IDLE_REQ:
        case CONNECT_IDLE_ACK:
            // ERA connection request
            if (connect_enable)
            {
                connect_era = TRUE;
                connect_tx_req_con(connect_magic);
                connect_timer_retry();
                connect_retries = CONNECT_REQ_RETRIES;
                connect_state = CONNECT_IDLE_ACK;
            }
            break;

        case CONNECT_DATA:
        case CONNECT_DATA_ACK:
            // Should not receive when already connected, so disconnect
            err = connect_reset();
            break;

        default:
            // No other states should occur
            err = &err_bad_connect_state;
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - The received frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received request confirm frame.
*/
static os_error *connect_poll_rx_req_con(const frame_data *frame)
{
    os_error *err = NULL;

    // Action depends on the current state
    switch (connect_state)
    {
        case CONNECT_IDLE_ACK:
            // ERA connection confirmed
            if (connect_enable
                && (sizeof(bits) <= frame->size)
                && (*((bits *) frame->data) != connect_magic))
            {
                // Acknowledge the received frame
                connect_era = TRUE;
                connect_seq_tx = 0;
                connect_seq_rx = 0;
                connect_tx_ack(connect_seq_rx);
                connect_timer_idle();
                connect_state = CONNECT_DATA;
                err = connect_mux_connected();
            }
            break;

        case CONNECT_DATA:
        case CONNECT_DATA_ACK:
            // Should not receive when already connected, so disconnect
            err = connect_reset();
            break;

        case CONNECT_IDLE:
        case CONNECT_IDLE_REQ:
            // Ignore the connection confirm in other states
            break;

        default:
            // No other states should occur
            err = &err_bad_connect_state;
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - The received frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received data frame.
*/
static os_error *connect_poll_rx_data(const frame_data *frame)
{
    os_error *err = NULL;

    // Action depends on the current state
    switch (connect_state)
    {
        case CONNECT_DATA:
        case CONNECT_DATA_ACK:
            // Data received
            if (frame->seq == connect_inc_seq(connect_seq_rx))
            {
                // The sequence number matches so data is valid
                connect_seq_rx = frame->seq;
                connect_rx_data_frame = *frame;
                connect_rx_data_pending = TRUE;
                connect_tx_ack(connect_seq_rx);
                if (connect_state == CONNECT_DATA) connect_timer_idle();
                else connect_timer_retry();
            }
            else
            {
                // Sequence number does not match so not valid
                connect_tx_ack(connect_seq_rx);
                stats_rx_retry_frame++;
            }
            break;

        case CONNECT_IDLE:
        case CONNECT_IDLE_REQ:
        case CONNECT_IDLE_ACK:
            // Ignore the data in other states
            break;

        default:
            // No other states should occur
            err = &err_bad_connect_state;
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : rx_frame      - The received frame.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a received frame.
*/
static os_error *connect_poll_rx(const frame_data *rx_frame)
{
    os_error *err = NULL;

    // Process the received frame
    switch (rx_frame->cont)
    {
        case CONNECT_CONT_ACK_PDU:
            // Acknowledge of a data or request/confirm frame
            err = connect_poll_rx_ack(rx_frame);
            break;

        case CONNECT_CONT_DISC_PDU:
            // Disconnect frame
            err = connect_poll_rx_disc(rx_frame);
            break;

        case CONNECT_CONT_REQ_PDU:
            // Request or request request/confirm frame
            if (rx_frame->seq  == CONNECT_SEQ_REQ_PDU)
            {
                // SIBO request
                err = connect_poll_rx_req(rx_frame);
            }
            else if (rx_frame->seq == CONNECT_SEQ_REQ_REQ_PDU)
            {
                // ERA request request
                err = connect_poll_rx_req_req(rx_frame);
            }
            else if (rx_frame->seq == CONNECT_SEQ_REQ_CON_PDU)
            {
                // ERA request confirm
                err = connect_poll_rx_req_con(rx_frame);
            }
            break;

        case CONNECT_CONT_DATA_PDU:
            // Data frame
            err = connect_poll_rx_data(rx_frame);
            break;

        default:
            // No other frames are of interest
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Start transmitting any waiting frame.
*/
static os_error *connect_poll_tx(void)
{
    os_error *err = NULL;

    // Supervisory frames take priority
    if (connect_ctrl_pending)
    {
        connect_ctrl_pending = FALSE;
        err = frame_send(&connect_ctrl_frame);
    }
    else if (connect_tx_data_pending != connect_tx_data_head)
    {
        connect_tx_data_pending = connect_inc_tx_window(connect_tx_data_pending);
        err = frame_send(&connect_tx_data_frame[connect_tx_data_pending]);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle a timer timeout event. This normally results in either
                  a retry or a disconnect.
*/
static os_error *connect_poll_timeout(void)
{
    os_error *err = NULL;

    // Action depends on the current state
    switch (connect_state)
    {
        case CONNECT_IDLE:
            // No action unless connection enabled
            if (connect_enable)
            {
                connect_tx_req_req();
                connect_timer_retry();
                connect_state = CONNECT_IDLE_REQ;
            }
            break;

        case CONNECT_IDLE_REQ:
            // Retry the connection request
            if (connect_enable)
            {
                bool changed;
                err = link_next_baud(&changed);
                if (!err) err = frame_reset(changed);
                if (!err)
                {
                    connect_tx_req_req();
                    connect_timer_retry();
                }
            }
            else
            {
                err = connect_reset();
            }
            break;

        case CONNECT_IDLE_ACK:
            // Retry the connection confirm
            if (connect_enable && --connect_retries)
            {
                connect_tx_req_con(connect_magic);
                connect_timer_retry();
            }
            else
            {
                err = connect_reset();
            }
            break;

        case CONNECT_DATA:
            // Inactivity timeout
            connect_tx_disc();
            err = connect_reset();
            break;

        case CONNECT_DATA_ACK:
            // Retry the data transmission if possible
            if (--connect_retries)
            {
                connect_tx_data_pending = connect_tx_data_tail;
                connect_timer_retry();
                stats_tx_retry_frame++;
            }
            else
            {
                connect_tx_disc();
                err = connect_reset();
            }
            break;

        default:
            // No other states should occur
            err = &err_bad_connect_state;
            break;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check the timer and perform any actions required.
*/
static os_error *connect_poll_timer(void)
{
    os_error *err = NULL;

    // Check for expiry of the timer
    if (connect_timer && (0 < (util_time() - connect_timeout)))
    {
        // Disable the timer
        connect_timer_stop();

        // Perform any actions required
        err = connect_poll_timeout();
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Reset the state if a disconnection is detected.
*/
static os_error *connect_poll_disconnected(void)
{
    os_error *err = NULL;

    // Cancel any pending transfers
    connect_ctrl_pending = FALSE;

    // Reset the state machine
    err = connect_reset();

    // Return any error produced
    return err;
}

/*
    Parameters  : active        - Is the remote device present and active.
                  rx_frame      - Pointer to a received message, or NULL if
                                  none.
                  tx_idle       - Is the frame transmitter idle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Handle any received frame and perform any other polled
                  actions required.
*/
os_error *connect_poll(bool active, const frame_data *rx_frame, bool tx_idle)
{
    os_error *err = NULL;

    // No action unless active
    if (connect_active)
    {
        // Set the polled status
        connect_polled = TRUE;

        // Special case if disconnected
        if (active)
        {
            // Handle any received frame
            if (rx_frame) err = connect_poll_rx(rx_frame);

            // Check for timeouts
            if (!err) err = connect_poll_timer();

            // Poll the multiplexor layer if appropriate
            if (!err && connect_connected)
            {
                // Perform the poll
                err = mux_poll(connect_rx_data_pending
                               ? &connect_rx_data_frame
                               : NULL,
                               0 < connect_free_tx_window());

                // Clear any pending received frame
                connect_rx_data_pending = FALSE;
            }

            // Handle any data to transmit
            if (!err && tx_idle) err = connect_poll_tx();
        }
        else err = connect_poll_disconnected();

        // Clear the polled status
        connect_polled = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : frame         - The data to send.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Transmit the specified data. An error is returned if either
                  there is no connection or the transmit queue is full.
*/
os_error *connect_tx(const frame_data *frame)
{
    os_error *err = NULL;

    // Check parameters
    if (!frame) err = &err_bad_parms;
    else if (!connect_active) err = &err_no_connect;
    else if (!connect_connected) err = &err_not_connected;
    else if (!connect_polled) err = &err_not_poll;
    else if (connect_free_tx_window() == 0) err = &err_connection_busy;
    else
    {
        // Queue the specified frame
        connect_seq_tx = connect_inc_seq(connect_seq_tx);
        connect_tx_data_head = connect_inc_tx_window(connect_tx_data_head);
        connect_tx_data_frame[connect_tx_data_head] = *frame;
        connect_tx_data_frame[connect_tx_data_head].cont = CONNECT_CONT_DATA_PDU;
        connect_tx_data_frame[connect_tx_data_head].seq = connect_seq_tx;
        connect_timer_retry();
        connect_retries = CONNECT_DATA_RETRIES;

        // Change state
        connect_state = CONNECT_DATA_ACK;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Trigger a disconnection and wait for it to complete. This
                  attempts to perform a fully handshaked disconnection, but
                  reverts to an abortive disconnect if necessary.
*/
static os_error *connect_disconnect(void)
{
    os_error *err = NULL;
    escape_config esc;

    // Enable escape conditions
    err = escape_store(&esc);
    if (!err) err = escape_enable();
    if (!err)
    {
        // Send any pending data frames
        while (!err && (connect_state == CONNECT_DATA_ACK))
        {
            err = link_poll(TRUE);
        }

        // Send a disconnection frame
        if (!err)
        {
            connect_tx_disc();
            err = connect_reset();
        }

        // Keep polling until frame transmitted
        while (!err && connect_ctrl_pending) err = link_poll(TRUE);

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
    Description : Start the connection managers and any clients for the link.
*/
os_error *connect_start(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Starting connection handler"))

    // No action if already active
    if (!connect_active)
    {
        // Start by updating any relevant pollwords
        err = pollword_update(psifs_MASK_LINK_STATUS);

        // Reset the state machine
        if (!err) err = connect_reset();

        // Set the active flag and enable connections if successful
        if (!err)
        {
            connect_active = TRUE;
            connect_enable = TRUE;
        }
    }

    DEBUG_ERR(err)

    // Return any error produced
    return err;
}

/*
    Parameters  : now           - Should the link usage terminate immediately.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : End the connection manager and any clients for the link.
                  This will normally attempt a tidy shutdown, but may be forced
                  to terminate immediately.
*/
os_error *connect_end(bool now)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Ending connection handler now=%u", now))

    // No action unless active
    if (connect_active)
    {
        // Disable any reconnection attempts
        connect_enable = FALSE;

        // Start by updating any relevant pollwords
        err = pollword_update(psifs_MASK_LINK_STATUS);

        // Ensure that the multiplexor is disconnected
        if (!err) err = connect_mux_disconnected(now);

        // Perform a disconnect
        if (!now) err = connect_disconnect();

        // Clear the active flag if successful
        if (!err) connect_active = FALSE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the connection manager and any
                  clients.
*/
os_error *connect_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying connection handler status"))

    // Display the current connection status
    if (connect_connected)
    {
        printf("Connected to %s device.\n", connect_era ? "an EPOC" : "a SIBO");
        err = mux_status();
    }
    else if (connect_active)
    {
         printf("Not connected to a remote device.\n"
                "The remote link may be disabled,"
                " or the settings could be incorrect.\n");
    }
    else printf("Connection disabled.\n");

    // Return any error produced
    return err;
}
