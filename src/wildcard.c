/*
    File        : wildcard.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Wildcarded string comparisons for the PsiFS module.

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
#include "wildcard.h"

// Include clib header files
#include <ctype.h>

// Include project header files
#include "fs.h"

/*
    Parameters  : pattern   - The wildcarded pattern to match.
                  str       - The string to compare to.
    Returns     : int       - The result of the comparison:
                                < 0 if pattern < str
                                 0  if pattern == str
                                > 0 if pattern > str
    Description : Perform a wildcarded, case-insensitive comparison of the two
                  strings.
*/
int wildcard_cmp(const char *pattern, const char *str)
{
    int result = 0;

    // Compare characters in sequence
    while (!result && (*pattern || *str))
    {
        // Check for special conditions
        if (*pattern == FS_CHAR_WILD_SINGLE)
        {
            // Single character wildcard does not match the terminator
            pattern++;
            if (*str++ == '\0') result = 1;
        }
        else if (*pattern == FS_CHAR_WILD_ANY)
        {
            // Try all string lengths for multiple character wildcard
            pattern++;
            result = wildcard_cmp(pattern, str);
            while (*str)
            {
                str++;
                if (result) result = wildcard_cmp(pattern, str);
            }
            pattern = str;
        }
        else result = toupper(*pattern++) - toupper(*str++);
    }

    // Return the result
    return result;
}
