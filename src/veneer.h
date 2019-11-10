/*
    File        : veneer.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Filing system entry point veneers for the PsiFS module.

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
#ifndef VENEER_H
#define VENEER_H

#ifdef __cplusplus
    extern "C" {
#endif

// Dummy prototypes for the filing system entry point veneers
void fsentry_open(void);
void fsentry_getbytes(void);
void fsentry_putbytes(void);
void fsentry_args(void);
void fsentry_close(void);
void fsentry_file(void);
void fsentry_func(void);
void fsentry_gbpb(void);
void fsentry_free(void);

#ifdef __cplusplus
    }
#endif

#endif
