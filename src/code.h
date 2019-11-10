/*
    File        : code.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Code page conversions for the PsiFS module.

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
#ifndef CODE_H
#define CODE_H

// Include clib header files
#include <stdlib.h>

// Include oslib header files
#include "oslib/os.h"

// Include project header files
#include "psifs.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : src           - The source string.
                  dest          - The variable to hold the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a string from the Latin1 character set to the Windows
                  ANSI code page as used by ERA. This is not an exact inverse
                  of code_ansi_to_latin1, i.e. the operation is not generally
                  reversible. The source and destination strings may be the same
                  to perform an in-place conversion.
*/
os_error *code_latin1_to_ansi(const char *src, char *dest);

/*
    Parameters  : src           - The source string.
                  dest          - The variable to hold the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a string from the Latin1 character set to the 850 code
                  page as used by SIBO. This is not an exact inverse of
                  code_850_to_latin1, i.e. the operation is not generally
                  reversible. The source and destination strings may be the same
                  to perform an in-place conversion.
*/
os_error *code_latin1_to_850(const char *src, char *dest);

/*
    Parameters  : src           - The source string.
                  dest          - The variable to hold the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a string from the Windows ANSI code page as used by ERA
                  to the Latin1 character set. This is not an exact inverse of
                  code_latin1_to_ansi, i.e. the operation is not generally
                  reversible. The source and destination strings may be the
                  same to perform an in-place conversion.
*/
os_error *code_ansi_to_latin1(const char *src, char *dest);

/*
    Parameters  : src           - The source string.
                  dest          - The variable to hold the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a string from the 850 code page as used by SIBO to the
                  Latin1 character set. This is not an exact inverse of
                  code_latin1_to_850, i.e. the operation is not generally
                  reversible. The source and destination strings may be the same
                  to perform an in-place conversion.
*/
os_error *code_850_to_latin1(const char *src, char *dest);

/*
    Parameters  : src           - The source string.
                  dest          - The variable to hold the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a string from the Windows ANSI code page as used by ERA
                  to the 850 code page as used by SIBO. This is an exact
                  inverse of code_850_to_ansi, i.e. the operation is fully
                  reversible. The source and destination strings may be the
                  same to perform an in-place conversion.
*/
os_error *code_ansi_to_850(const char *src, char *dest);

/*
    Parameters  : src           - The source string.
                  dest          - The variable to hold the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a string from the 850 code page as used by SIBO to the
                  Windows ANSI code page used by ERA. This is an exact inverse
                  of code_ansi_to_850, i.e. the operation is fully reversible.
                  The source and destination strings may be the same to perform
                  an in-place conversion.
*/
os_error *code_850_to_ansi(const char *src, char *dest);

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Maximum number of characters to write to the
                                  destination buffer, including the terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory leafname from the format used by
                  RISC OS to that used by ERA. This is almost an exact inverse
                  of code_era_to_riscos, with the exception of incorrectly
                  recognised escape sequences, i.e. the operation is generally
                  reversible.
*/
os_error *code_riscos_to_era(const char *src, char *dest, size_t size);

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Maximum number of characters to write to the
                                  destination buffer, including the terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory leafname from the format used by
                  RISC OS to that used by SIBO. This is almost an exact inverse
                  of code_sibo_to_riscos, with the exception of incorrectly
                  recognised escape sequences, i.e. the operation is generally
                  reversible.
*/
os_error *code_riscos_to_sibo(const char *src, char *dest, size_t size);

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Maximum number of characters to write to the
                                  destination buffer, including the terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory leafname from the format used by
                  ERA to that used by RISC OS. This is almost an exact inverse
                  of code_riscos_to_era, with the exception of incorrectly
                  recognised escape sequences, i.e. the operation is generally
                  reversible.
*/
os_error *code_era_to_riscos(const char *src, char *dest, size_t size);

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Maximum number of characters to write to the
                                  destination buffer, including the terminator.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory leafname from the format used by
                  SIBO to that used by RISC OS. This is almost an exact inverse
                  of code_riscos_to_sibo, with the exception of incorrectly
                  recognised escape sequences, i.e. the operation is generally
                  reversible.
*/
os_error *code_sibo_to_riscos(const char *src, char *dest, size_t size);

/*
    Parameters  : from          - Source character set.
                  to            - Destination character set.
                  table         - Variable to hold the result.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Construct a character translation table between the two
                  character sets. Indexing the resulting table by a character
                  code from the source character set will yield the
                  corresponding character in the destination character set or
                  0 if the character is unmappable.
*/
os_error *code_get_table(psifs_character_set from, psifs_character_set to,
                         psifs_translation_table *table);

#ifdef __cplusplus
    }
#endif

#endif
