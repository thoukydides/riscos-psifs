/*
    File        : util.c
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

// Include header file for this module
#include "util.h"

// Include system header files
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// Include oslib header files
#include "oslib/macros.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"

// Include project header files
#include "ctrl.h"
#include "err.h"
#include "fs.h"
#include "sis.h"
#include "tar.h"
#include "wildcard.h"

// Language mappings
typedef struct
{
    char code[3];
    psifs_language language;
} util_language;
static const util_language util_languages[] =
{
    {"EN", psifs_LANGUAGE_UK_ENGLISH},
    {"FR", psifs_LANGUAGE_FRENCH},
    {"GE", psifs_LANGUAGE_GERMAN},
    {"SP", psifs_LANGUAGE_SPANISH},
    {"IT", psifs_LANGUAGE_ITALIAN},
    {"SW", psifs_LANGUAGE_SWEDISH},
    {"DA", psifs_LANGUAGE_DANISH},
    {"NO", psifs_LANGUAGE_NORWEGIAN},
    {"FI", psifs_LANGUAGE_FINNISH},
    {"AM", psifs_LANGUAGE_AMERICAN_ENGLISH},
    {"SF", psifs_LANGUAGE_SWISS_FRENCH},
    {"SG", psifs_LANGUAGE_SWISS_GERMAN},
    {"PO", psifs_LANGUAGE_PORTUGUESE},
    {"TU", psifs_LANGUAGE_TURKISH},
    {"IC", psifs_LANGUAGE_ICELANDIC},
    {"RU", psifs_LANGUAGE_RUSSIAN},
    {"HU", psifs_LANGUAGE_HUNGARIAN},
    {"DU", psifs_LANGUAGE_DUTCH},
    {"BL", psifs_LANGUAGE_BELGIAN_FLEMISH},
    {"AU", psifs_LANGUAGE_AUSTRALIAN_ENGLISH},
    {"BG", psifs_LANGUAGE_BELGIAN_FRENCH},
    {"AS", psifs_LANGUAGE_AUSTRIAN_GERMAN},
    {"NZ", psifs_LANGUAGE_NEW_ZEALAND_ENGLISH},
    {"IF", psifs_LANGUAGE_INTERNATIONAL_FRENCH},
    {"CS", psifs_LANGUAGE_CZECH},
    {"SK", psifs_LANGUAGE_SLOVAK},
    {"PL", psifs_LANGUAGE_POLISH},
    {"SL", psifs_LANGUAGE_SLOVENIAN},
    {"TC", psifs_LANGUAGE_TAIWAN_CHINESE},
    {"HK", psifs_LANGUAGE_HONG_KONG_CHINESE},
    {"ZH", psifs_LANGUAGE_PRC_CHINESE},
    {"JA", psifs_LANGUAGE_JAPANESE},
    {"TH", psifs_LANGUAGE_THAI}
};
#define UTIL_LANGUAGES (sizeof(util_languages) / sizeof(util_language))

/*
    Parameters  : void
    Returns     : os_t  - The current monotonic time.
    Description : Read the monotonic time, ignoring any error produced.
*/
os_t util_time(void)
{
    os_error *err = NULL;
    os_t time;

    // Attempt to read the time
    err = xos_read_monotonic_time(&time);

    // Return the value read
    return err ? 0 : time;
}

/*
    Parameters  : a     - The first task handle.
                  b     - The second task handle.
    Returns     : bool  - Are the tasks the same.
    Description : Compare two task handles for equality.
*/
bool util_eq_task(wimp_t a, wimp_t b)
{
    // Return the result of the comparison
    return (((bits) a) & UTIL_TASK_MASK) == (((bits) b) & UTIL_TASK_MASK);
}

/*
    Parameters  : value         - The value to convert.
    Returns     : const char *  - Pointer to the resulting string.
    Description : Convert the specified value to a left aligned string using
                  commas to separate each group of three digits.
*/
const char *util_int_comma(bits value)
{
    static char buffer[14];
    char *ptr = buffer + sizeof(buffer);
    bits digits = 0;

    // Ensure that the string is terminated
    *--ptr = '\0';

    // Process the next digit
    do
    {
        // Add a comma if required
        if (digits == 3)
        {
            *--ptr = ',';
            digits = 0;
        }

        // Add the next digit
        *--ptr = '0' + (value % 10);
        value /= 10;
        digits++;
    } while (value);

    // Return a pointer to the result
    return ptr;
}

/*
    Parameters  : spec          - Disc specification, or NULL for the current
                                  drive.
                  path          - Variable to receive a pointer to the
                                  corresponding path.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a disc specification into a path.
*/
os_error *util_disc_spec(const char *spec, const char **path)
{
    os_error *err = NULL;

    // Check parameters
    if (!path) err = &err_bad_parms;
    else
    {
        static fs_pathname str;
        char *ptr;

        // Construct the required path
        strcpy(str, FS_NAME);
        ptr = strchr(str, '\0');
        *ptr++ = FS_CHAR_DISC;
        *ptr++ = FS_CHAR_DISC;
        if (!spec || !*spec)
        {
            // No disc specified
            *ptr++ = fs_default_drive;
            *ptr++ = FS_CHAR_SEPARATOR;
            *ptr++ = FS_CHAR_ROOT;
            *ptr++ = '\0';
        }
        else
        {
            // Append the specified disc
            if (*spec == FS_CHAR_DISC) spec++;
            if ((sizeof(str) + str - ptr) <= strlen(spec)) err = &err_bad_drive;
            else strcpy(ptr, spec);
        }

        // Return a pointer to the result
        *path = !err ? str : NULL;
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : drive         - The drive specifier.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Set the default drive to use if none specified.
*/
os_error *util_set_drive(const char *drive)
{
    os_error *err = NULL;

    // Check parameters
    if (!drive) err = &err_bad_parms;
    else
    {
        // Check if it is a valid drive letter
        if (!isalpha(drive[0]) || drive[1]) err = &err_bad_drive;
        else fs_default_drive = toupper(drive[0]);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : bool  - Should long filenames be quietly truncated.
    Description : Read the configured truncate option. If set then long
                  filenames should be quietly truncated, otherwise an error
                  should be generated.
*/
bool util_truncate(void)
{
    static bool truncate = TRUE;
    static bool read = FALSE;

    // No action required if already read
    if (!read)
    {
        int value;

        // Read the configured truncate option
        if (!xosbyte2(osbyte_READ_CMOS, osbyte_CONFIGURE_TRUNCATE, 0, &value))
        {
            truncate = BOOL(value & osbyte_CONFIGURE_TRUNCATE_MASK);
        }

        // Set the read flag
        read = TRUE;
    }

    // Return whether the truncate bit is set
    return truncate;
}

/*
    Parameters  : dir           - The name of the directory to create.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Ensure that the specified directory exists. Parent directories
                  are created if necessary.
*/
os_error *util_cdir(const char *dir)
{
    os_error *err = NULL;

    // Check parameters
    if (!dir) err = &err_bad_parms;
    else
    {
        static fs_pathname path;
        char *dot = NULL;

        // Copy the path if required
        if (dir != path)
        {
            if (sizeof(path) <= ctrl_strlen(dir)) err = &err_bad_name;
            else ctrl_strcpy(path, dir);
        }

        // Attempt to remove the leafname
        if (!err) dot = ctrl_strrchr(path, FS_CHAR_SEPARATOR);
        if (dot)
        {
            // Remove the leafname
            *dot = '\0';

            // Create the parent directory
            err = util_cdir(path);

            // Restore the directory separator
            if (!err) *dot = FS_CHAR_SEPARATOR;

            // Create this directory
            if (!err) err = xosfile_create_dir(dir, 0);
        }
    }

    // Return any error produced
    return err;
}

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
                        bool verbose)

{
    os_error *err = NULL;

    // Check parameters
    if (!src || !pattern || (!dest && !verbose)) err = &err_bad_parms;
    else
    {
        tar_handle handle;

        // Attempt to open the tar file
        err = tar_open_in(src, &handle);
        if (!err)
        {
            const fs_info *info;

            // Loop through all of the files
            err = tar_info(handle, &info);
            while (!err && info)
            {
                bool done = FALSE;

                // Check if the file matches
                if (!wildcard_cmp(pattern, info->name))
                {
                    // Display the file details
                    if (verbose) printf("%s\n", info->name);

                    // Extract the file if appropriate
                    if (dest)
                    {
                        static fs_pathname path;

                        // Mark the file as extracted
                        done = TRUE;

                        // Generate the destination path
                        if (sizeof(path) <= ctrl_strlen(dest) + ctrl_strlen(info->name) + 1) err = &err_bad_name;
                        else sprintf(path, "%s%c%s", dest, FS_CHAR_SEPARATOR, info->name);

                        // Ensure that parent directories exist
                        if (!err)
                        {
                            // Remove the leafname
                            char *dot = ctrl_strrchr(path, FS_CHAR_SEPARATOR);
                            *dot = '\0';

                            // Ensure that the directory exists
                            err = util_cdir(path);

                            // Restore the leafname
                            if (!err) *dot = FS_CHAR_SEPARATOR;
                        }

                        // Extract the file
                        if (!err) err = tar_extract(handle, path);
                    }
                }

                // Skip over the file if not extracted
                if (!done) err = tar_skip(handle);

                // Complete the current operation
                if (!err) err = tar_complete(handle);

                // Read the details of the next file
                if (!err) err = tar_info(handle, &info);
            }

            // Close the tar file, ignoring any error produced
            tar_close(&handle);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : dir           - Name of destination directory.
                  tar           - Handle of destination tar file.
                  src           - Name of source file.
                  dest          - Name of destination file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Save a file to the specified directory or tar file.
*/
static os_error *util_save_file(const char *dir, tar_handle tar,
                                const char *src, const char *dest)
{
    os_error *err = NULL;

    // Check parameters
    if ((!dir && !tar) || !src || !dest) err = &err_bad_parms;
    else
    {
        static fs_pathname path;
        char *ptr;

        // Stub of path depends on the destination
        if (dir)
        {
            // Stub is based on the target directory
            if (sizeof(path) <= (ctrl_strlen(dir) + 1)) err = &err_bad_name;
            else
            {
                // Start with the specified directory
                ctrl_strcpy(path, dir);

                // Append a directory separator
                ptr = ctrl_strchr(path, '\0');
                *ptr++ = FS_CHAR_SEPARATOR;
                *ptr = '\0';
            }
        }
        else
        {
            // No stub for tar file
            *path = '\0';
            ptr = path;
        }

        // Special case for fully specified destination file name
        if (!err && (dest[0] == FS_CHAR_DISC))
        {
            // Check that the format is suitable
            if (!isalpha(dest[1]) || (dest[2] != FS_CHAR_SEPARATOR) || (dest[3] != FS_CHAR_ROOT) || (dest[4] != FS_CHAR_SEPARATOR)) err = &err_bad_name;
            else if (sizeof(path) <= (ctrl_strlen(path) + 2)) err = &err_bad_name;
            else
            {
                // Add the directory to the path
                *ptr++ = toupper(dest[1]);
                *ptr++ = FS_CHAR_SEPARATOR;
                *ptr = '\0';

                // Skip over the drive specification
                dest += 5;
            }
        }

        // Append the destination name to the path
        if (!err)
        {
            // Check that the path fits
            if (sizeof(path) <= (ctrl_strlen(path) + ctrl_strlen(dest))) err = &err_bad_name;
            else ctrl_strcat(ptr, dest);
        }

        // Action depends on whether target is a directory or tar file
        if (dir)
        {
            // Ensure that parent directories exist
            if (!err)
            {
                // Remove the leafname
                char *dot = ctrl_strrchr(path, FS_CHAR_SEPARATOR);
                *dot = '\0';

                // Ensure that the directory exists
                err = util_cdir(path);

                // Restore the leafname
                if (!err) *dot = FS_CHAR_SEPARATOR;
            }

            // Move the file
            if (!err)
            {
                err = xosfscontrol_copy(src, path,
                                        osfscontrol_COPY_RECURSE
                                        | osfscontrol_COPY_FORCE
                                        | osfscontrol_COPY_DELETE
                                        | osfscontrol_COPY_LOOK,
                                        0, 0, 0, 0, NULL);
            }
        }
        else
        {
            // Add to tar file
            if (!err) err = tar_add(src, path, tar);
            if (!err) err = tar_complete(tar);
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : sis           - Handle of SIS file to process.
                  dir           - Name of destination directory.
                  tar           - Handle of destination tar file.
                  scrap         - Temporary file.
                  recurse       - Recurse through component SIS files.
                  residual      - Should a residual file be created.
                  language      - Language to use, or psifs_LANGUAGE_UNKNOWN
                                  for the default.
                  drive         - Destination drive.
                  verbose       - Should verbose output be produced.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Extract or list the contents of the specified SIS file.
*/
static os_error *util_process_sis(sis_handle sis, const char *dir,
                                  tar_handle tar, const char *scrap,
                                  bool recurse, bool residual,
                                  psifs_language language, psifs_drive drive,
                                  bool verbose)
{
    os_error *err = NULL;

    // Check parameters
    if (!sis || (tar && dir) || (!scrap && (tar || dir)) || (residual && !tar && !dir) || (!verbose && !tar && !dir)) err = &err_bad_parms;
    else
    {
        bits index;
        bits requisites;
        bits files;

        // Set the language
        if (language != psifs_LANGUAGE_UNKNOWN)
        {
            err = sis_set_language(sis, language);
        }

        // Set the drive
        if (!err && (drive != SIS_SELECTABLE_DRIVE))
        {
            err = sis_set_drive(sis, drive);
        }

        // Display the component name and version
        if (verbose)
        {
            const char *component;
            bits major;
            bits minor;
            bits variant;
            epoc32_file_uid uid;

            // Read the component name and version number
            if (!err) err = sis_get_component(sis, &component);
            if (!err) err = sis_get_version(sis, &major, &minor);
            if (!err) err = sis_get_variant(sis, &variant);
            if (!err) err = sis_get_uid(sis, &uid);
            if (!err)
            {
                // Display the component name and version number
                printf("Start         : %s\n", component);
                printf("Version       : %u.%02u (%u)\n", major, minor, variant);
                printf("UID           : %08X%08X%08X\n", uid.uid1, uid.uid2, uid.uid3);
            }
        }

        // Display the language information
        if (verbose)
        {
            bits num;
            const psifs_language *languages;
            psifs_language selected;

            // Read the list of supported languages
            if (!err) err = sis_get_languages(sis, &num, &languages);
            if (!err) err = sis_get_language(sis, &selected);
            if (!err) printf("Languages     : ");
            for (index = 0; !err && (index < num); index++)
            {
                const char *code;

                // Display the code for this language
                err = util_language_to_code(languages[index], &code);
                if (!err)
                {
                    if (index) printf(", ");
                    if (languages[index] == selected) printf("[");
                    printf("%s", code);
                    if (languages[index] == selected) printf("]");
                }
            }
            if (!err) printf("\n");
        }

        // Display the installation drive
        if (verbose)
        {
            psifs_drive selected;
            bool selectable;

            // Read the selected drive
            if (!err) err = sis_get_drive(sis, &selected, &selectable);
            if (!err)
            {
                // Display the drive information
                printf("Drive         : ");
                if (!selectable) printf("Not selectable\n");
                else printf("%c\n", selected);
            }
        }

        // Process requisites
        if (!err) err = sis_get_requisites(sis, &requisites);
        for (index = 0; !err && (index < requisites); index++)
        {
            const char *name;
            bits uid;
            bits major;
            bits minor;
            bits variant;

            // Read the requisite details
            err = sis_get_requisite(sis, index, &name, &uid, &major, &minor, &variant);

            // Display the requisite details
            if (!err && verbose)
            {
                printf("Requisite     : %s (", name);
                printf("UID %08X, ", uid);
                printf("Version %u.%02u (%u)", major, minor, variant);
                printf(")\n");
            }
        }

        // Process files
        if (!err) err = sis_get_files(sis, &files);
        for (index = 0; !err && (index < files); index++)
        {
            const char *name;
            sis_file_type type;
            bits detail;
            bits size;

            // Read the file details
            err = sis_get_file(sis, index, &name, &type, &detail, &size);

            // Display the file details
            if (!err && verbose)
            {
                switch (type)
                {
                    case SIS_FILE_STANDARD:
                        // Standard file
                        printf("File          : %s (%u bytes)\n", name, size);
                        break;

                    case SIS_FILE_TEXT:
                        // Text file
                        printf("File (text)   : %s (%u bytes, ", name, size);
                        if (detail == SIS_BUTTONS_CONTINUE) printf("Continue button only");
                        else if (detail == SIS_BUTTONS_SKIP) printf("Yes - continue, No - skip next file");
                        else if (detail == SIS_BUTTONS_ABORT) printf("Yes - continue, No - abort installation");
                        else printf(" buttons %u", detail);
                        printf(")\n");
                        break;

                    case SIS_FILE_SIS:
                        // Component SIS file
                        printf("File (SIS)    : %s (%u bytes)\n", name, size);
                        break;

                    case SIS_FILE_RUN:
                        // Run file on installation or removal
                        printf("File (run)    : %s (%u bytes, run ", name, size);
                        if (detail == SIS_RUN_INSTALL) printf("on installation");
                        else if (detail == SIS_RUN_REMOVE) printf("on removal");
                        else if (detail == SIS_RUN_BOTH) printf("on installation and removal");
                        else printf(" type %u", detail);
                        printf(")\n");
                        break;

                    case SIS_FILE_NONE:
                        // Virtual file
                        printf("File (virtual): %s\n", name);
                        break;

                    default:
                        // Unknown file type
                        err = &err_bad_sis_type;
                        break;
                }
            }

            // Take appropriate action
            if (type == SIS_FILE_NONE)
            {
                // No action required for virtual files
            }
            else if ((type == SIS_FILE_SIS) && recurse)
            {
                sis_handle component = NULL;

                // Recurse through component SIS files
                if (!err) err = sis_get_sis(sis, index, &component);
                if (!err)
                {
                    // Need to specify a different residual file
                    err = util_process_sis(component, dir, tar, scrap,
                                           recurse, residual,
                                           language, drive, verbose);
                }
                if (component) sis_close(&component);
            }
            else if (dir || tar)
            {
                // Extract the file
                if (!err) err = sis_save_file(sis, index, scrap);

                // Move the file
                if (!err) err = util_save_file(dir, tar, scrap, name);
            }
        }

        // Create residual file
        if (!err && residual)
        {
            const char *name;

            // Create the residual file
            err = sis_save_residual(sis, scrap);

            // Generate the target filename
            if (!err) err = sis_get_residual(sis, &name);

            // Move the residual file
            if (!err) err = util_save_file(dir, tar, scrap, name);
        }

        // Display the component name again
        if (verbose)
        {
            const char *component;

            // Read the component name
            if (!err) err = sis_get_component(sis, &component);
            if (!err)
            {
                // Display the component name
                printf("End           : %s\n", component);
            }
        }
    }

    // Return any error produced
    return err;
}

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
                        bool verbose)
{
    os_error *err = NULL;

    // Check parameters
    if (!src || (tar && !dest) || (!scrap && dest) || (residual && !dest) || (!verbose && !dest)) err = &err_bad_parms;
    else
    {
        sis_handle in = NULL;
        tar_handle out = NULL;

        // Open the SIS file
        err = sis_open(src, residual ? residual : src, &in);

        // Open the tar file
        if (!err && tar) err = tar_open_out(dest, &out, FALSE);

        // Process the contents
        if (!err)
        {
            err = util_process_sis(in, tar ? NULL : dest, out, scrap,
                                   recurse, residual ? TRUE : FALSE, language,
                                   drive, verbose);
        }

        // Close the tar file
        if (out) tar_close(&out);

        // Close the SIS file
        if (in) sis_close(&in);

        // Ensure that any scrap file is deleted
        if (scrap) xosfscontrol_wipe(scrap, osfscontrol_WIPE_RECURSE | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : language      - The language number.
                  code          - Variable to receive a pointer to the
                                  corresponding code.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a language number into the corresponding code.
*/
os_error *util_language_to_code(psifs_language language, const char **code)
{
    os_error *err = NULL;

    // Check parameters
    if (!code) err = &err_bad_parms;
    else
    {
        bits i;

        // Search for a matching language number
        *code = NULL;
        for (i = 0; !*code && (i < UTIL_LANGUAGES); i++)
        {
            if (util_languages[i].language == language)
            {
                *code = util_languages[i].code;
            }
        }

        // Use the language number if not found
        if (!*code)
        {
            static char str[DEC_WIDTH + 1];
            sprintf(str, "%02d", language);
            *code = str;
        }
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : code          - The language code.
                  language      - Variable to receive the corresponding number.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Convert a language code into the corresponding number.
*/
os_error *util_language_from_code(const char *code, psifs_language *language)
{
    os_error *err = NULL;

    // Check parameters
    if (!code || !language) err = &err_bad_parms;
    else
    {
        bits i;

        // Check if a number was specified
        for (i = 0; code[i] && isdigit(code[i]); i++) {}
        if (i && !code[i])
        {
            // A number was specified so convert directly
            *language = atoi(code);
        }
        else
        {
            // Attempt to find a matching code
            *language = psifs_LANGUAGE_UNKNOWN;
            for (i = 0; (*language == psifs_LANGUAGE_UNKNOWN) && (i < UTIL_LANGUAGES); i++)
            {
                if (!wildcard_cmp(util_languages[i].code, code))
                {
                    *language = util_languages[i].language;
                }
            }

            // Return an error if not recognised
            if (*language == psifs_LANGUAGE_UNKNOWN) err = &err_lang_unknown;
        }
    }

    // Return any error produced
    return err;
}
