/*
    File        : ctrl.c
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

// Include header file for this module
#include "ctrl.h"

// Include oslib header files
#include "oslib/macros.h"

// Include project header files
#include "mem.h"

// Check for a control character
#define CTRL_TERM(c) ((0 <= (c)) && ((c) < 32))

/*
    Parameters  : to        - Variable to receive a '\0' terminated copy of the
                              string.
                  from      - The control character terminated string to copy.
    Returns     : char *    - Pointer to the destination string.
    Description : Copy the source string into the destination string. The
                  behaviour is undefined if the objects overlap.
*/
char *ctrl_strcpy(char *to, const char *from)
{
    char *ptr = to;

    // Copy the string
    while (!CTRL_TERM(*from)) *ptr++ = *from++;

    // Terminate the result
    *ptr = '\0';

    // Return a pointer to the result
    return to;
}

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
char *ctrl_strncpy(char *to, const char *from, size_t n)
{
    char *ptr = to;

    // Copy n characters
    while (n)
    {
        // Copy the next character
        if (CTRL_TERM(*from)) *ptr++ = '\0';
        else *ptr++ = *from++;

        // Decrement the number of characters left to copy
        n--;
    }

    // Return a pointer to the result
    return to;
}

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
char *ctrl_strcat(char *to, const char *from)
{
    char *ptr = to;

    // Find the destination string terminator
    while (!CTRL_TERM(*ptr)) ptr++;

    // Append the source string
    ctrl_strcpy(ptr, from);

    // Return a pointer to the result
    return to;
}

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
char *ctrl_strncat(char *to, const char *from, size_t n)
{
    char *ptr = to;

    // Find the destination string terminator
    while (!CTRL_TERM(*ptr)) ptr++;

    // Append at most n characters from the source string
    while (!CTRL_TERM(*from) && n)
    {
        *ptr++ = *from++;
        n--;
    }

    // Terminate the result
    *ptr = '\0';

    // Return a pointer to the result
    return to;
}

/*
    Parameters  : str1  - The first string.
                  n1    - The length of the first string.
                  str2  - The second string.
                  n2    - The length of the second string.
    Returns     : int   - The result of the comparison:
                            < 0 if str1 < str2
                             0  if str1 == str2
                            > 0 if str1 > str2
    Description : Compare the two strings up to the specified lengths. No
                  checks are performed for terminators.
*/
static int ctrl_cmp(const char *str1, size_t n1, const char *str2, size_t n2)
{
    int result = 0;
    size_t n = MIN(n1, n2);

    // Compare the initial segments
    while (!result && n)
    {
        if (*str1 != *str2) result = *str1 < *str2 ? -1 : +1;
        else
        {
            str1++;
            str2++;
            n--;
        }
    }

    // Compare the lengths if the initial segments match
    if (!result && (n1 != n2)) result = n1 < n2 ? -1 : +1;

    // Return the result of the comparison
    return result;
}

/*
    Parameters  : str1  - The first control character terminated string.
                  str2  - The second control character terminated string.
    Returns     : int   - The result of the comparison:
                            < 0 if str1 < str2
                             0  if str1 == str2
                            > 0 if str1 > str2
    Description : Compare the two strings.
*/
int ctrl_strcmp(const char *str1, const char *str2)
{
    // Return the result of the comparison
    return ctrl_cmp(str1, ctrl_strlen(str1), str2, ctrl_strlen(str2));
}

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
int ctrl_strncmp(const char *str1, const char *str2, size_t n)
{
    // Return the result of the comparison
    return ctrl_cmp(str1, MIN(n, ctrl_strlen(str1)),
                    str2, MIN(n, ctrl_strlen(str2)));
}

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
char *ctrl_strchr(const char *str, int ch)
{
    char *ptr = (char *) str;

    // Find the first match
    while (!CTRL_TERM(*ptr) && (*ptr != ch)) ptr++;

    // Return the result
    return CTRL_TERM(ch) || !CTRL_TERM(*ptr) ? ptr : NULL;
}

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
char *ctrl_strrchr(const char *str, int ch)
{
    char *ptr = NULL;
    char *next = ctrl_strchr(str, ch);

    // Find the last match
    while (next)
    {
        ptr = next;
        next = CTRL_TERM(*ptr) ? NULL : ctrl_strchr(ptr + 1, ch);
    }

    // Return the result
    return ptr;
}

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
size_t ctrl_strspn(const char *str, const char *set)
{
    const char *ptr = str;

    // Find the first character not from the set
    while (!CTRL_TERM(*ptr) && ctrl_strchr(set, *ptr)) ptr++;

    // Return the length of the initial segment
    return ptr - str;
}

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
size_t ctrl_strcspn(const char *str, const char *set)
{
    const char *ptr = str;

    // Find the first character from the set
    while (!CTRL_TERM(*ptr) && !ctrl_strchr(set, *ptr)) ptr++;

    // Return the length of the initial segment
    return ptr - str;
}

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
char *ctrl_strpbrk(const char *str, const char *set)
{
    char *ptr = (char *) str;

    // Find the first character from the set
    while (!CTRL_TERM(*ptr) && !ctrl_strchr(set, *ptr)) ptr++;

    // Return the length of the initial segment
    return CTRL_TERM(*ptr) ? NULL : ptr;
}

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
char *ctrl_strstr(const char *str, const char *sub)
{
    char *ptr = (char *) str;
    size_t n_str = ctrl_strlen(str);
    size_t n_sub = ctrl_strlen(sub);

    // Find the first matching sequence
    while ((n_sub <= n_str) && ctrl_strncmp(ptr, sub, n_sub))
    {
        ptr++;
        n_str--;
    }

    // Return the result
    return n_str < n_sub ? NULL : ptr;
}

/*
    Parameters  : str       - A control character terminated string.
    Returns     : size_t    - The number of characters that precede the
                              terminator character.
    Description : Compute the length of the specified string.
*/
size_t ctrl_strlen(const char *str)
{
    const char *ptr = str;

    // Find the terminator character
    while (ptr && !CTRL_TERM(*ptr)) ptr++;

    // Return the length of the string
    return ptr - str;
}

/*
    Parameters  : str       - The control character terminated string to copy.
    Returns     : char *    - Pointer to the copy of the string, or NULL if
                              failed.
    Description : Allocate memory and copy the specified string. The memory
                  should be released when finished with by calling free with
                  the returned pointer value.
*/
char *ctrl_strdup(const char *str)
{
    char *ptr = NULL;

    // Attempt to allocate the required memory
    if (str) ptr = (char *) MEM_MALLOC(ctrl_strlen(str) + 1);

    // Copy the string if memory allocation successful
    if (ptr) ctrl_strcpy(ptr, str);

    // Return a pointer to the result
    return ptr;
}
