/*
    File        : wildcard.h
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

// Only include header file once
#ifndef WILDCARD_H
#define WILDCARD_H

#ifdef __cplusplus
    extern "C" {
#endif

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
int wildcard_cmp(const char *pattern, const char *str);

#ifdef __cplusplus
    }
#endif

#endif
