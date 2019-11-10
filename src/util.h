/*
    File        : util.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Utility functions for the PsiFS module.

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
#ifndef UTIL_H
#define UTIL_H

// Include oslib header files
#include "oslib/os.h"
#include "oslib/wimp.h"

// Include project header files
#include "fs.h"

// Mask for comparing wimp task handles
#define UTIL_TASK_MASK (0xffff)

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : void
    Returns     : os_t  - The current monotonic time.
    Description : Read the monotonic time, ignoring any error produced.
*/
os_t util_time(void);

/*
    Parameters  : a     - The first task handle.
                  b     - The second task handle.
    Returns     : bool  - Are the tasks the same.
    Description : Compare two task handles for equality.
*/
bool util_eq_task(wimp_t a, wimp_t b);

/*
    Parameters  : value         - The value to convert.
    Returns     : const char *  - Pointer to the resulting string.
    Description : Convert the specified value to a left aligned string using
                  commas to separate each group of three digits.
*/
const char *util_int_comma(bits value);

/*
    Parameters  : spec          - Disc specification, or NULL for the current
                                  drive.
                  path          - Variable to receive a pointer to the
                                  corresponding path.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a disc specification into a path.
*/
os_error *util_disc_spec(const char *spec, const char **path);

/*
    Parameters  : drive         - The drive specifier.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the default drive to use if none specified.
*/
os_error *util_set_drive(const char *drive);

/*
    Parameters  : void
    Returns     : bool  - Should long filenames be quietly truncated.
    Description : Read the configured truncate option. If set then long
                  filenames should be quietly truncated, otherwise an error
                  should be generated.
*/
bool util_truncate(void);

/*
    Parameters  : dir           - The name of the directory to create.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Ensure that the specified directory exists. Parent directories
                  are created if necessary.
*/
os_error *util_cdir(const char *dir);

/*
    Parameters  : src           - Source tar file.
                  pattern       - Wildcarded pattern to match.
                  dest          - Optional destination directory.
                  verbose       - Should verbose output be produced.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Extract the contents of the specified tar file.
*/
os_error *util_read_tar(const char *src, const char *pattern, const char *dest,
                        bool verbose);

/*
    Parameters  : src           - Source SIS file.
                  dest          - Destination directory or tar file.
                  tar           - Should a tar file be produced.
                  scrap         - Temporary file.
                  recurse       - Recurse through component SIS files.
                  residual      - Name of residual file to create.
                  language      - Language to use, or psifs_LANGUAGE_UNKNOWN
                                  for the default.
                  drive         - Destination drive.
                  verbose       - Should verbose output be produced.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Extract or list the contents of the specified SIS file.
*/
os_error *util_read_sis(const char *src, const char *dest, bool tar,
                        const char *scrap, bool recurse, const char *residual,
                        psifs_language language, psifs_drive drive,
                        bool verbose);

/*
    Parameters  : language      - The language number.
                  code          - Variable to receive a pointer to the
                                  corresponding code.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a language number into the corresponding code.
*/
os_error *util_language_to_code(psifs_language language, const char **code);

/*
    Parameters  : code          - The language code.
                  language      - Variable to receive the corresponding number.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a language code into the corresponding number.
*/
os_error *util_language_from_code(const char *code, psifs_language *language);

#ifdef __cplusplus
    }
#endif

#endif
