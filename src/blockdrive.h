/*
    File        : blockdrive.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Interface to Hugo Fiennes's Block Drivers for serial access.

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

// Only include header file once
#ifndef BLOCKDRIVE_H
#define BLOCKDRIVE_H

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/os.h"

// Control lines
typedef bits blockdrive_control;
#define BLOCKDRIVE_CONTROL_DTR ((blockdrive_control) (1 << 0))
#define BLOCKDRIVE_CONTROL_RTS ((blockdrive_control) (1 << 1))
#define BLOCKDRIVE_CONTROL_BREAK ((blockdrive_control) (1 << 2))

// Modem control lines
typedef bits blockdrive_modem;
#define BLOCKDRIVE_MODEM_CTS ((blockdrive_modem) (1 << 0))
#define BLOCKDRIVE_MODEM_DSR ((blockdrive_modem) (1 << 1))
#define BLOCKDRIVE_MODEM_RI ((blockdrive_modem) (1 << 2))
#define BLOCKDRIVE_MODEM_DCD ((blockdrive_modem) (1 << 3))

// Receive errors
typedef bits blockdrive_errors;
#define BLOCKDRIVE_ERRORS_OVERRUN ((blockdrive_errors) (1 << 0))
#define BLOCKDRIVE_ERRORS_PARITY ((blockdrive_errors) (1 << 1))
#define BLOCKDRIVE_ERRORS_FRAMING ((blockdrive_errors) (1 << 2))
#define BLOCKDRIVE_ERRORS_BREAK ((blockdrive_errors) (1 << 3))

// Flow control
typedef bits blockdrive_flow;
#define BLOCKDRIVE_FLOW_HARDWARE ((blockdrive_flow) (1 << 0))
#define BLOCKDRIVE_FLOW_XONXOFF ((blockdrive_flow) (1 << 1))

// Word format
typedef bits blockdrive_format;
#define BLOCKDRIVE_FORMAT_BITS_MASK ((blockdrive_format) (3 << 0))
#define BLOCKDRIVE_FORMAT_BITS_8 ((blockdrive_format) (0 << 0))
#define BLOCKDRIVE_FORMAT_BITS_7 ((blockdrive_format) (1 << 0))
#define BLOCKDRIVE_FORMAT_BITS_6 ((blockdrive_format) (2 << 0))
#define BLOCKDRIVE_FORMAT_BITS_5 ((blockdrive_format) (3 << 0))
#define BLOCKDRIVE_FORMAT_STOP_MASK ((blockdrive_format) (1 << 2))
#define BLOCKDRIVE_FORMAT_STOP_1 ((blockdrive_format) (0 << 2))
#define BLOCKDRIVE_FORMAT_STOP_2 ((blockdrive_format) (1 << 2))
#define BLOCKDRIVE_FORMAT_PARITY_MASK ((blockdrive_format) (1 << 3))
#define BLOCKDRIVE_FORMAT_PARITY_DISABLED ((blockdrive_format) (0 << 3))
#define BLOCKDRIVE_FORMAT_PARITY_ENABLED ((blockdrive_format) (1 << 3))
#define BLOCKDRIVE_FORMAT_PARITY_TYPE_MASK ((blockdrive_format) (3 << 4))
#define BLOCKDRIVE_FORMAT_PARITY_TYPE_ODD ((blockdrive_format) (0 << 4))
#define BLOCKDRIVE_FORMAT_PARITY_TYPE_EVEN ((blockdrive_format) (1 << 4))
#define BLOCKDRIVE_FORMAT_PARITY_TYPE_1 ((blockdrive_format) (2 << 4))
#define BLOCKDRIVE_FORMAT_PARITY_TYPE_0 ((blockdrive_format) (3 << 4))

// Block driver flags
typedef bits blockdrive_flags;
#define BLOCKDRIVE_FLAGS_MULTIPLE_PORTS ((blockdrive_flags) (1 << 0))
#define BLOCKDRIVE_FLAGS_SPLIT_RATES ((blockdrive_flags) (1 << 1))
#define BLOCKDRIVE_FLAGS_HAS_FIFO ((blockdrive_flags) (1 << 2))
#define BLOCKDRIVE_FLAGS_MULTITASK_BREAK ((blockdrive_flags) (1 << 3))
#define BLOCKDRIVE_FLAGS_REQUIRES_POLL ((blockdrive_flags) (1 << 4))
#define BLOCKDRIVE_FLAGS_WONT_EMPTY ((blockdrive_flags) (1 << 5))
#define BLOCKDRIVE_FLAGS_BLOCK_OPERATIONS ((blockdrive_flags) (1 << 6))
#define BLOCKDRIVE_FLAGS_NOT_OVERLAP ((blockdrive_flags) (1 << 7))
#define BLOCKDRIVE_FLAGS_INQUIRY_INITIALISE ((blockdrive_flags) (1 << 8))
#define BLOCKDRIVE_FLAGS_HIGHEST_PORT_SHIFT (24)
#define BLOCKDRIVE_FLAGS_HIGHEST_PORT_MASK ((blockdrive_flags) (0xff << 24))

// Block driver number
#define BLOCKDRIVE_NUMBER_SHIFT (8)

// List of baud rates (0 word ends list)
#define BLOCKDRIVE_SPEEDS (32)
typedef bits blockdrive_speeds[BLOCKDRIVE_SPEEDS];

// Block driver layout
typedef struct
{
    bits entry[32];                     // Entry point to call routines
    char information[32];               // Driver information string
    char manufacturer[32];              // Manufacturer information string
    bits version;                       // Version number XX.XX
    blockdrive_flags flags;             // Driver flags
    bits number;                        // Driver number
    bits reserved[13];                  // Reserved
    blockdrive_speeds speeds;           // Supported speeds (0 word ends list)
    bits code[928];                     // Driver code
    bits port;                          // The active port (not part of driver)
} blockdrive_layout;
typedef blockdrive_layout *blockdrive_driver;
#define BLOCKDRIVE_DRIVER_NONE ((blockdrive_driver) NULL)

// Special value to indicate no port
#define BLOCKDRIVE_PORT_NONE ((bits) -1)

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : context       - Set to 0 for the first call and preserve
                                  between successive calls.
                  name          - Buffer to receive the next block driver name.
                  size          - Size of the buffer.
                  found         - Variable to receive whether a block driver
                                  was found.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Enumerate the available block drivers.
*/
os_error *blockdrive_enumerate(int *context, char *name, size_t size,
                               bool *found);

/*
    Parameters  : name          - The name of the block driver to load.
                  driver        - Variable to receive the driver handle.
                  ports         - Variable to receive the number of ports
                                  available.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Allocate a block of memory, load the specified block driver,
                  and perform an inquiry initialise (if supported). This also
                  selects the driver for future operations.
*/
os_error *blockdrive_load(const char *name, blockdrive_driver *driver,
                          bits *ports);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : Deallocate the block of memory used by the specified block
                  driver. This does not close down the driver first.
*/
void blockdrive_unload(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
                  value     - The byte to send.
    Returns     : bool      - Was the byte inserted into the transmit queue.
    Description : Insert a byte into the transmit queue.
*/
bool blockdrive_put_byte(blockdrive_driver driver, byte value);

/*
    Parameters  : driver    - The handle of the block driver.
                  value     - Variable to receive the byte read.
    Returns     : bool      - Was a byte available.
    Description : Remove a byte from the receive queue.
*/
bool blockdrive_get_byte(blockdrive_driver driver, byte *value);

/*
    Parameters  : driver    - The handle of the block driver.
                  block     - Pointer to block containing the bytes to transmit.
                  bytes     - Number of bytes to insert.
    Returns     : bits      - The number of bytes successfully inserted.
    Description : Insert a block of bytes into the transmit queue.
*/
bits blockdrive_put_block(blockdrive_driver driver, const byte *block,
                          size_t bytes);

/*
    Parameters  : driver    - The handle of the block driver.
                  block     - Pointer to block to hold the received bytes.
                  bytes     - Maximum number of bytes to place in the buffer.
    Returns     : bits      - Number of bytes placed in the buffer.
    Description : Remove a block of bytes from the receive queue.
*/
bits blockdrive_get_block(blockdrive_driver driver, byte *block, size_t bytes);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : bits      - Number of bytes free in the transmit buffer.
    Description : Check the available space in the transmit buffer.
*/
bits blockdrive_check_tx(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : bits      - Number of bytes used in the receive buffer.
    Description : Check the number of bytes in the receive buffer.
*/
bits blockdrive_check_rx(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : Flush the transmit buffer and hardware FIFO (if applicable).
*/
void blockdrive_flush_tx(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : Flush the receive buffer and hardware FIFO (if applicable).
*/
void blockdrive_flush_rx(blockdrive_driver driver);

/*
    Parameters  : driver                - The handle of the block driver.
    Returns     : blockdrive_control    - The current state of the control
                                          lines.
    Description : Read the control line settings.
*/
blockdrive_control blockdrive_control_read(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
                  control   - The new control line settings.
    Returns     : void
    Description : Set the state of the control lines.
*/
void blockdrive_control_write(blockdrive_driver driver,
                              blockdrive_control control);

/*
    Parameters  : driver            - The handle of the block driver.
    Returns     : blockdrive_modem  - The current state of the modem control
                                      lines.
    Description : Read the modem control line status.
*/
blockdrive_modem blockdrive_modem_read(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : blockdrive_errors - The receive errors seen.
    Description : Read the receive errors seen since the last call of this
                  function.
*/
blockdrive_errors blockdrive_errors_read(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
                  time      - The length of the break in centiseconds.
    Returns     : void
    Description : Send a break. This function does not return until the break
                  has been sent.
*/
void blockdrive_break(blockdrive_driver driver, bits time);

/*
    Parameters  : driver    - The handle of the block driver.
                  value     - Variable to receive the byte read.
    Returns     : bool  - Was a byte available.
    Description : Read the next byte from the receive queue, leaving it in the
                  buffer.
*/
bool blockdrive_examine_byte(blockdrive_driver driver, byte *value);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : bits      - The current transmit speed.
    Description : Read the current transmit baud rate.
*/
bits blockdrive_tx_speed_read(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
                  speed     - The new transmit speed.
    Returns     : void
    Description : Set a new transmit baud rate.
*/
void blockdrive_tx_speed_write(blockdrive_driver driver, bits speed);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : bits  - The current receive speed.
    Description : Read the current receive baud rate.
*/
bits blockdrive_rx_speed_read(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
                  speed     - The new receive speed.
    Returns     : void
    Description : Set a new receive baud rate.
*/
void blockdrive_rx_speed_write(blockdrive_driver driver, bits speed);

/*
    Parameters  : driver            - The handle of the block driver.
    Returns     : blockdrive_format - The word format.
    Description : Read the current word format.
*/
blockdrive_format blockdrive_word_format_read(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
                  format    - The required word format.
    Returns     : void
    Description : Set a new word format.
*/
void blockdrive_word_format_write(blockdrive_driver driver,
                                  blockdrive_format format);

/*
    Parameters  : driver            - The handle of the block driver.
    Returns     : blockdrive_flow   - The current flow control method.
    Description : Read the current flow control method.
*/
blockdrive_flow blockdrive_flow_control_read(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
                  method    - The required flow control method.
    Returns     : void
    Description : Set a new flow control method.
*/
void blockdrive_flow_control_write(blockdrive_driver driver,
                                   blockdrive_flow method);

/*
    Parameters  : driver        - The handle of the block driver.
                  port          - The port to initialise.
                  options       - Parameter string specifying device options,
                                  or NULL if none.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Initialise the block driver for the specified port. The baud
                  rate tables and driver flags should not be read until this
                  function has been called. This also selects the port for
                  future options.
*/
os_error *blockdrive_initialise(blockdrive_driver driver, bits port,
                                const char *options);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : Close down the block driver.
*/
void blockdrive_close_down(blockdrive_driver driver);

/*
    Parameters  : driver    - The handle of the block driver.
    Returns     : void
    Description : This should be called regularly, such as in a polling loop,
                  for the driver to perform polling tasks. Polls can be as
                  infrequent as three or four times a second.
*/
void blockdrive_poll(blockdrive_driver driver);

#ifdef __cplusplus
    }
#endif

#endif
