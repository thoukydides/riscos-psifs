/*
    File        : stats.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Statistics collection for the PsiFS module.

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
#include "stats.h"

// Serial data
bits stats_rx_bytes = 0;
bits stats_tx_bytes = 0;

// Link layer frames
bits stats_rx_frame = 0;
bits stats_rx_err_frame = 0;
bits stats_rx_retry_frame = 0;
bits stats_tx_frame = 0;
bits stats_tx_retry_frame = 0;

/*
    Parameters  : void
    Returns     : void
    Description : Reset the statistics.
*/
void stats_reset(void)
{
    // Reset all of the values
    stats_rx_bytes = 0;
    stats_tx_bytes = 0;
    stats_rx_frame = 0;
    stats_rx_err_frame = 0;
    stats_rx_retry_frame = 0;
    stats_tx_frame = 0;
    stats_tx_retry_frame = 0;
}
