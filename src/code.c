/*
    File        : code.c
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

// Include header file for this module
#include "code.h"

// Include clib header files
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

// Include project header files
#include "debug.h"
#include "err.h"
#include "fs.h"

// A partial character mapping
typedef struct
{
    char from;
    char to;
} code_map;

// Mapping between Latin1 and ANSI
static const code_map code_map_latin1_ansi[] =
{
    {0x00,  0x80},
    {0x00,  0x81},
    {0x00,  0x82},
    {0x00,  0x83},
    {0x00,  0x88},
    {0x00,  0x8a},
    {0x00,  0x8d},
    {0x00,  0x8e},
    {0x00,  0x8f},
    {0x00,  0x90},
    {0x00,  0x98},
    {0x00,  0x9a},
    {0x00,  0x9d},
    {0x00,  0x9e},
    {0x00,  0x9f},
    {0x80,  0x00},
    {0x81,  0x00},  // Ŵ
    {0x82,  0x00},  // ŵ
    {0x83,  0x00},
    {0x84,  0x00},
    {0x85,  0x00},  // Ŷ
    {0x86,  0x00},  // ŷ
    {0x87,  0x00},
    {0x88,  0x00},
    {0x89,  0x00},
    {0x8a,  0x00},
    {0x8b,  0x00},
    {0x8c,  0x85},  // …
    {0x8d,  0x99},  // ™
    {0x8e,  0x89},  // ‰
    {0x8f,  0x95},  // •
    {0x90,  0x91},  // ‘
    {0x91,  0x92},  // ’
    {0x92,  0x8b},  // ‹
    {0x93,  0x9b},  // ›
    {0x94,  0x93},  // “
    {0x95,  0x94},  // ”
    {0x96,  0x84},  // „
    {0x97,  0x96},  // –
    {0x98,  0x97},  // —
    {0x99,  0x10},  // − mapped to hard space
    {0x9a,  0x8c},  // Œ
    {0x9b,  0x9c},  // œ
    {0x9c,  0x86},  // †
    {0x9d,  0x87},  // ‡
    {0x9e,  0x00},  // ﬁ
    {0x9f,  0x00},  // ﬂ
};
#define CODE_MAP_LATIN1_ANSI_NUM (sizeof(code_map_latin1_ansi) / sizeof(code_map))

// Mapping between code page 850 and ANSI
static const code_map code_map_850_ansi[] =
{
    {0x05,  0x86},
    {0x80,  0xc7},
    {0x81,  0xfc},
    {0x82,  0xe9},
    {0x83,  0xe2},
    {0x84,  0xe4},
    {0x85,  0xe0},
    {0x86,  0xe5},
    {0x87,  0xe7},
    {0x88,  0xea},
    {0x89,  0xeb},
    {0x8a,  0xe8},
    {0x8b,  0xef},
    {0x8c,  0xee},
    {0x8d,  0xec},
    {0x8e,  0xc4},
    {0x8f,  0xc5},
    {0x90,  0xc9},
    {0x91,  0xe6},
    {0x92,  0xc6},
    {0x93,  0xf4},
    {0x94,  0xf6},
    {0x95,  0xf2},
    {0x96,  0xfb},
    {0x97,  0xf9},
    {0x98,  0xff},
    {0x99,  0xd6},
    {0x9a,  0xdc},
    {0x9b,  0xf8},
    {0x9c,  0xa3},
    {0x9d,  0xd8},
    {0x9e,  0xd7},
    {0x9f,  0x83},
    {0xa0,  0xe1},
    {0xa1,  0xed},
    {0xa2,  0xf3},
    {0xa3,  0xfa},
    {0xa4,  0xf1},
    {0xa5,  0xd1},
    {0xa6,  0xaa},
    {0xa7,  0xba},
    {0xa8,  0xbf},
    {0xa9,  0xae},
    {0xaa,  0xac},
    {0xab,  0xbd},
    {0xac,  0xbc},
    {0xad,  0xa1},
    {0xae,  0xab},
    {0xaf,  0xbb},
    {0xb0,  0x80},
    {0xb1,  0x81},
    {0xb2,  0x82},
    {0xb3,  0x9c},
    {0xb4,  0x84},
    {0xb5,  0xc1},
    {0xb6,  0xc2},
    {0xb7,  0xc0},
    {0xb8,  0xa9},
    {0xb9,  0x85},
    {0xba,  0x05},
    {0xbb,  0x87},
    {0xbc,  0x88},
    {0xbd,  0xa2},
    {0xbe,  0xa5},
    {0xbf,  0x89},
    {0xc0,  0x8a},
    {0xc1,  0x8b},
    {0xc2,  0x8c},
    {0xc3,  0x8d},
    {0xc4,  0xad},
    {0xc5,  0x8e},
    {0xc6,  0xe3},
    {0xc7,  0xc3},
    {0xc8,  0x8f},
    {0xc9,  0x90},
    {0xca,  0x91},
    {0xcb,  0x92},
    {0xcc,  0x93},
    {0xcd,  0x97},
    {0xce,  0x94},
    {0xcf,  0xa4},
    {0xd0,  0xf0},
    {0xd1,  0xd0},
    {0xd2,  0xca},
    {0xd3,  0xcb},
    {0xd4,  0xc8},
    {0xd5,  0x96},
    {0xd6,  0xcd},
    {0xd7,  0xce},
    {0xd8,  0xcf},
    {0xd9,  0x98},
    {0xda,  0x99},
    {0xdb,  0x9a},
    {0xdc,  0x9b},
    {0xdd,  0xa6},
    {0xde,  0xcc},
    {0xdf,  0x9d},
    {0xe0,  0xd3},
    {0xe1,  0xdf},
    {0xe2,  0xd4},
    {0xe3,  0xd2},
    {0xe4,  0xf5},
    {0xe5,  0xd5},
    {0xe6,  0xb5},
    {0xe7,  0xfe},
    {0xe8,  0xde},
    {0xe9,  0xda},
    {0xea,  0xdb},
    {0xeb,  0xd9},
    {0xec,  0xfd},
    {0xed,  0xdd},
    {0xee,  0xaf},
    {0xef,  0xb4},
    {0xf0,  0x9e},
    {0xf1,  0xb1},
    {0xf2,  0x9f},
    {0xf3,  0xbe},
    {0xf4,  0xb6},
    {0xf5,  0xa7},
    {0xf6,  0xf7},
    {0xf7,  0xb8},
    {0xf8,  0xb0},
    {0xf9,  0xa8},
    {0xfa,  0xb7},
    {0xfb,  0xb9},
    {0xfc,  0xb3},
    {0xfd,  0xb2},
    {0xfe,  0x95},
    {0xff,  0xa0}
};
#define CODE_MAP_850_ANSI_NUM (sizeof(code_map_850_ansi) / sizeof(code_map))

// Mapping between RISC OS and ERA filenames
static const code_map code_map_riscos_era[] =
{
    {0x00,  0x01},
    {0x00,  0x02},
    {0x00,  0x03},
    {0x00,  0x04},
    {0x00,  0x05},
    {0x00,  0x06},
    {0x00,  0x07},
    {0x00,  0x08},
    {0x00,  0x09},
    {0x00,  0x0a},
    {0x00,  0x0b},
    {0x00,  0x0c},
    {0x00,  0x0d},
    {0x00,  0x0e},
    {0x00,  0x0f},
    {0x00,  0x11},
    {0x00,  0x12},
    {0x00,  0x13},
    {0x00,  0x14},
    {0x00,  0x15},
    {0x00,  0x16},
    {0x00,  0x17},
    {0x00,  0x18},
    {0x00,  0x19},
    {0x00,  0x1a},
    {0x00,  0x1b},
    {0x00,  0x1c},
    {0x00,  0x1d},
    {0x00,  0x1e},
    {0x00,  0x1f},
    {0x00,  '\"'},
    {'<',   '$'},
    {0x00,  '%'},
    {0x00,  '&'},
    {0x00,  '/'},   // Parameter separator
    {0x00,  '<'},   // Input redirection
    {0x00,  '>'},   // Output redirection
    {0x00,  '@'},
    {'>',   '^'},
    {0x00,  '|'},   // Pipe
    {0x00,  0x7f},  // Delete
    {0x00,  0xa0},
    {0x01,  0x00},
    {0x02,  0x00},
    {0x03,  0x00},
    {0x04,  0x00},
    {0x05,  0x00},
    {0x06,  0x00},
    {0x07,  0x00},
    {0x08,  0x00},
    {0x09,  0x00},
    {0x0a,  0x00},
    {0x0b,  0x00},
    {0x0c,  0x00},
    {0x0d,  0x00},
    {0x0e,  0x00},
    {0x0f,  0x00},
    {0x11,  0x00},
    {0x12,  0x00},
    {0x13,  0x00},
    {0x14,  0x00},
    {0x15,  0x00},
    {0x16,  0x00},
    {0x17,  0x00},
    {0x18,  0x00},
    {0x19,  0x00},
    {0x1a,  0x00},
    {0x1b,  0x00},
    {0x1c,  0x00},
    {0x1d,  0x00},
    {0x1e,  0x00},
    {0x1f,  0x00},
    {' ',   0x00},
    {'\"',  0x00},
    {'#',   '?'},   // Wildcard for a single character
    {'$',   0x00},  // Root directory of the disc
    {'%',   0x00},  // Currently selected library directory (CSL)
    {'&',   0x00},  // User root directory (URD)
    {'*',   '*'},   // Wildcard for 0 or more characters
    {'.',   '\\'},  // Directory separator
    {'/',   '.'},   // Extension separator
    {':',   ':'},   // Drive or disc specification and end of filing system name
    {'?',   '#'},
    {'@',   0x00},  // Currently selected directory (CSD)
    {'\\',  0x00},  // Previously selected directory (PSD)
    {'^',   0x00},  // Parent directory
    {'|',   0x00},  // Pipe
    {0x7f,  0x00},  // Delete
    {0xa0,  ' '}    // Hard space under RISC OS, normal space under EPOC
};
#define CODE_MAP_RISCOS_ERA_NUM (sizeof(code_map_riscos_era) / sizeof(code_map))

// The cached conversion tables
typedef char code_cached[256];
static code_cached code_cached_latin1_to_ansi;
static code_cached code_cached_ansi_to_latin1;
static code_cached code_cached_ansi_to_850;
static code_cached code_cached_850_to_ansi;
static code_cached code_cached_riscos_to_ansi;
static code_cached code_cached_ansi_to_riscos;

// Quoting and unmappable character substitution
#define CODE_UNMAPPABLE '_'
#define CODE_QUOTE_START 0x9E
#define CODE_QUOTE_END 0x9F

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Perform any required module initialisation.
*/
static os_error *code_initialise(void)
{
    os_error *err = NULL;
    static bool initialised = FALSE;

    // Initialise the conversion tables once only
    if (!initialised)
    {
        int i;

        // Start with standard mappings
        for (i = 0; i < 256; i++)
        {
            code_cached_latin1_to_ansi[i] = i;
            code_cached_ansi_to_latin1[i] = i;
            code_cached_ansi_to_850[i] = i;
            code_cached_850_to_ansi[i] = i;
        }

        // Process each of the tables
        for (i = 0; i < CODE_MAP_LATIN1_ANSI_NUM; i++)
        {
            code_cached_latin1_to_ansi[code_map_latin1_ansi[i].from] = code_map_latin1_ansi[i].to;
            code_cached_ansi_to_latin1[code_map_latin1_ansi[i].to] = code_map_latin1_ansi[i].from;
        }
        for (i = 0; i < CODE_MAP_850_ANSI_NUM; i++)
        {
            code_cached_850_to_ansi[code_map_850_ansi[i].from] = code_map_850_ansi[i].to;
            code_cached_ansi_to_850[code_map_850_ansi[i].to] = code_map_850_ansi[i].from;
        }
        for (i = 0; i < 256; i++)
        {
            code_cached_riscos_to_ansi[i] = code_cached_latin1_to_ansi[i];
            code_cached_ansi_to_riscos[i] = code_cached_ansi_to_latin1[i];
        }
        for (i = 0; i < CODE_MAP_RISCOS_ERA_NUM; i++)
        {
            code_cached_riscos_to_ansi[code_map_riscos_era[i].from] = code_cached_latin1_to_ansi[code_map_riscos_era[i].to];
            code_cached_ansi_to_riscos[code_cached_latin1_to_ansi[code_map_riscos_era[i].to]] = code_map_riscos_era[i].from;
        }

        // Set the initialised flag
        initialised = TRUE;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : src           - The source string.
                  dest          - The variable to hold the result.
                  pre           - The first mapping to apply, or NULL for none.
                  post          - The second mapping to apply, or NULL for none.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a string using the specified mapping. The source and
                  destination strings may be the same to perform an in-place
                  conversion.
*/
static os_error *code_string(const char *src, char *dest,
                             const code_cached pre, const code_cached post)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest || (!pre && !post)) err = &err_bad_parms;
    else
    {
        // Perform any required initialisation
        err = code_initialise();

        // Perform the conversion
        while (!err && *src)
        {
            char ch = *src++;
            if (pre) ch = pre[ch];
            if (post) ch = post[ch];
            *dest++ = ch;
        }

        // Terminate the result
        if (!err) *dest = '\0';
    }

    // Return any error produced
    return err;
}

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
os_error *code_latin1_to_ansi(const char *src, char *dest)
{
    // Perform the required mapping
    return code_string(src, dest, code_cached_latin1_to_ansi, NULL);
}

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
os_error *code_latin1_to_850(const char *src, char *dest)
{
    // Perform the required mapping
    return code_string(src, dest, code_cached_latin1_to_ansi,
                       code_cached_ansi_to_850);
}

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
os_error *code_ansi_to_latin1(const char *src, char *dest)
{
    // Perform the required mapping
    return code_string(src, dest, code_cached_ansi_to_latin1, NULL);
}

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
os_error *code_850_to_latin1(const char *src, char *dest)
{
    // Perform the required mapping
    return code_string(src, dest, code_cached_850_to_ansi,
                       code_cached_ansi_to_latin1);
}

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
os_error *code_ansi_to_850(const char *src, char *dest)
{
    // Perform the required mapping
    return code_string(src, dest, code_cached_ansi_to_850, NULL);
}

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
os_error *code_850_to_ansi(const char *src, char *dest)
{
    // Perform the required mapping
    return code_string(src, dest, code_cached_850_to_ansi, NULL);
}

/*
    Parameters  : src           - The source string.
                  dest          - Variable to hold the result.
                  size          - Maximum number of characters to write to the
                                  destination buffer, including the terminator.
                  pre           - The first character mapping to use, or NULL
                                  for none.
                  post          - The second character mapping to use, or NULL
                                  for none.
                  quote         - Should unmappable characters be quoted,
                                  otherwise unquoting is performed.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Map a file or directory leafname using the specified mapping.
*/
static os_error *code_file(const char *src, char *dest, size_t size,
                           const code_cached pre, const code_cached post,
                           bool quote)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || !dest || !size || (!pre && !post)) err = &err_bad_parms;
    else
    {
        // Perform any required initialisation
        err = code_initialise();

        // Process characters from the source string until finished
        while (!err && *src)
        {
            char ch = *src++;
            bool quoted = FALSE;

            // Perform the first mapping
            if (pre)
            {
                // Unquote the character if appropriate
                if (!quote && (ch == CODE_QUOTE_START)
                    && isxdigit(src[0]) && isxdigit(src[1])
                    && (src[2] == CODE_QUOTE_END))
                {
                    ch = (char) strtoul(src, NULL, 16);
                    src += 3;
                }
                else ch = pre[ch];
            }

            // Perform the second mapping
            if (post)
            {
                // Quote the character if appropriate
                if (quote && (!post[ch] || (post[ch] == CODE_QUOTE_START)
                              || (post[ch] == CODE_QUOTE_END)))
                {
                    quoted = TRUE;
                }
                else ch = post[ch];
            }

            // Write the result to the destination buffer
            if (size < (quoted ? 4 : 1)) err = &err_bad_name;
            else if (quoted)
            {
                dest += sprintf(dest, "%c%02X%c",
                                CODE_QUOTE_START, ch, CODE_QUOTE_END);
                size -= 4;
            }
            else
            {
                *dest++ = ch ? ch : CODE_UNMAPPABLE;
                size--;
            }
        }

        // Add a terminator to the result
        if (!err && !size) err = &err_bad_name;
        if (!err) *dest = '\0';
    }

    // Return any error produced
    return err;
}

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
os_error *code_riscos_to_era(const char *src, char *dest, size_t size)
{
    // Perform the required mapping
    return code_file(src, dest, size, code_cached_riscos_to_ansi, NULL, FALSE);
}

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
os_error *code_riscos_to_sibo(const char *src, char *dest, size_t size)
{
    os_error *err = NULL;

    // Perform the required mapping
    err = code_file(src, dest, size, code_cached_riscos_to_ansi,
                    code_cached_ansi_to_850, FALSE);

    // Tidy up the case
    if (!err) for (; *dest; dest++) *dest = toupper(*dest);

    // Return any error produced
    return err;
}

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
os_error *code_era_to_riscos(const char *src, char *dest, size_t size)
{
    // Perform the required mapping
    return code_file(src, dest, size, NULL, code_cached_ansi_to_riscos, TRUE);
}

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
os_error *code_sibo_to_riscos(const char *src, char *dest, size_t size)
{
    os_error *err = NULL;

    // Perform the required mapping
    err = code_file(src, dest, size, code_cached_850_to_ansi,
                    code_cached_ansi_to_riscos, TRUE);

    // Tidy up the case
    if (!err)
    {
        bool first = TRUE;

        while (*dest)
        {
            *dest = first ? toupper(*dest) : tolower(*dest);
            first = *dest++ == FS_CHAR_SEPARATOR;
        }
    }

    // Return any error produced
    return err;
}

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
                         psifs_translation_table *table)
{
    os_error *err = NULL;

    // Check parameters
    if (!table) err = &err_bad_parms;
    else
    {
        const char *map_from;
        const char *map_to;

        // Perform any required initialisation
        err = code_initialise();

        // Choose conversion from the source character set to ANSI
        if (!err)
        {
            switch (from)
            {
                case psifs_LATIN1:
                    map_from = code_cached_latin1_to_ansi;
                    break;

                case psifs_WINDOWS_ANSI:
                    map_from = NULL;
                    break;

                case psifs_CODE_PAGE_850:
                    map_from = code_cached_850_to_ansi;
                    break;

                default:
                    // No other character sets are supported
                    err = &err_bad_parms;
            }
        }

        // Choose conversion from ANSI to the destination character set
        if (!err)
        {
            switch (to)
            {
                case psifs_LATIN1:
                    map_to = code_cached_ansi_to_latin1;
                    break;

                case psifs_WINDOWS_ANSI:
                    map_to = NULL;
                    break;

                case psifs_CODE_PAGE_850:
                    map_to = code_cached_ansi_to_850;
                    break;

                default:
                    // No other character sets are supported
                    err = &err_bad_parms;
            }
        }

        // Build the translation table
        if (!err)
        {
            int i;

            // No translation if the character sets are the same
            if (from == to) map_from = map_to = NULL;

            // Fill in all the table entries
            for (i = 0; i < 256; i++)
            {
                char mapping = i;
                if (map_from) mapping = map_from[mapping];
                if (map_to) mapping = map_to[mapping];
                table->mapping[i] = mapping;
            }

            // Force preservation of null characters
            table->mapping[0] = 0;
        }
    }

    // Return any error produced
    return err;
}
