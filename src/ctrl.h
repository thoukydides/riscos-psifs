/*
    File        : ctrl.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Control character terminated string manipulation for the
                  PsiFS module.

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
#ifndef CTRL_H
#define CTRL_H

// Include clib header files
#include <stdlib.h>

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : to        - Variable to receive a '\0' terminated copy of the
                              string.
                  from      - The control character terminated string to copy.
    Returns     : char *    - Pointer to the destination string.
    Description : Copy the source string into the destination string. The
                  behaviour is undefined if the objects overlap.
*/
char *ctrl_strcpy(char *to, const char *from);

/*
    Parameters  : to        - Variable to receive a copy of the string, padded
                              to n characters with '\0's.
                  from      - The control character terminated string to copy.
                  n         - The maximum number of characters to copy.
    Returns     : char *    - Pointer to the destination string.
    Description : Copy not more than n characters from the source string into
                  the destination string. The behaviour is undefined if the
                  objects overlap.
*/
char *ctrl_strncpy(char *to, const char *from, size_t n);

/*
    Parameters  : to        - String to be extended. The result will be
                              terminated with a '\0'.
                  from      - The control character terminated string to
                              append.
    Returns     : char *    - Pointer to the destination string.
    Description : Append a copy of the source string to the end of the
                  destination string. The initial character of the source
                  string overwrites the terminator of the destination string.
*/
char *ctrl_strcat(char *to, const char *from);

/*
    Parameters  : to        - String to be extended. The result will always be
                              terminated with a '\0'.
                  from      - The control character terminated string to
                              append.
                  n         - The maximum number of characters to copy.
    Returns     : char *    - Pointer to the destination string.
    Description : Append not more than n characters from the source string to
                  the end of the destination string. The initial character of
                  the source string overwrites the terminator of the
                  destination string.
*/
char *ctrl_strncat(char *to, const char *from, size_t n);

/*
    Parameters  : str1  - The first control character terminated string.
                  str2  - The second control character terminated string.
    Returns     : int   - The result of the comparison:
                            < 0 if str1 < str2
                             0  if str1 == str2
                            > 0 if str1 > str2
    Description : Compare the two strings.
*/
int ctrl_strcmp(const char *str1, const char *str2);

/*
    Parameters  : str1  - The first control character terminated string.
                  str2  - The second control character terminated string.
                  n     - The maximum number of characters to compare.
    Returns     : int   - The result of the comparison:
                            < 0 if str1 < str2
                             0  if str1 == str2
                            > 0 if str1 > str2
    Description : Compare not more than n characters of the two strings.
*/
int ctrl_strncmp(const char *str1, const char *str2, size_t n);

/*
    Parameters  : str       - The control character terminated string to
                              search.
                  ch        - The character to search for. If this is a control
                              character then it will match the terminator
                              regardless of its value.
    Returns     : char *    - A pointer to the located character, or NULL if
                              the character does not occur in the string.
    Description : Locate the first occurrence of the specified character.
*/
char *ctrl_strchr(const char *str, int ch);

/*
    Parameters  : str       - The control character terminated string to
                              search.
                  ch        - The character to search for. If this is a control
                              character then it will match the terminator
                              regardless of its value.
    Returns     : char *    - A pointer to the located character, or NULL if
                              the character does not occur in the string.
    Description : Locate the last occurrence of the specified character.
*/
char *ctrl_strrchr(const char *str, int ch);

/*
    Parameters  : str       - The control character terminated string to
                              search.
                  set       - The control character terminated set of
                              characters to compare against.
    Returns     : size_t    - The length of the initial segment consisting
                              entirely of the specified characters.
    Description : Compute the length of the initial segment of the string which
                  consists entirely of characters from the specified set.
*/
size_t ctrl_strspn(const char *str, const char *set);

/*
    Parameters  : str       - The control character terminated string to
                              search.
                  set       - The control character terminated set of
                              characters to compare against.
    Returns     : size_t    - The length of the initial segment not containing
                              any of the specified characters.
    Description : Compute the length of the initial segment of the string which
                  consists entirely of characters not from the specified set.
*/
size_t ctrl_strcspn(const char *str, const char *set);

/*
    Parameters  : str       - The control character terminated string to
                              search.
                  set       - The control character terminated set of
                              characters to compare against.
    Returns     : char *    - A pointer to the first matching character, or
                              NULL if none are present.
    Description : Locate the first occurrence in the string of any character
                  from the specified set.
*/
char *ctrl_strpbrk(const char *str, const char *set);

/*
    Parameters  : str       - The control character terminated string to
                              search.
                  sub       - The control character terminated string to
                              search for.
    Returns     : char *    - Pointer to the first occurence of the search
                              string, or NULL if not present.
    Description : Locate the first occurence in the string of the specified
                  sequence of characters.
*/
char *ctrl_strstr(const char *str, const char *sub);

/*
    Parameters  : str       - A control character terminated string.
    Returns     : size_t    - The number of characters that precede the
                              terminator character.
    Description : Compute the length of the specified string.
*/
size_t ctrl_strlen(const char *str);

/*
    Parameters  : str       - The control character terminated string to copy.
    Returns     : char *    - Pointer to the copy of the string, or NULL if
                              failed.
    Description : Allocate memory and copy the specified string. The memory
                  should be released when finished with by calling free with
                  the returned pointer value.
*/
char *ctrl_strdup(const char *str);

#ifdef __cplusplus
    }
#endif

#endif
