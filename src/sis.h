/*
    File        : sis.h
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : SIS file handling for the PsiFS module.

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
#ifndef SIS_H
#define SIS_H

// Include oslib header files
#include "oslib/os.h"
#include "oslib/osgbpb.h"

// Include project header files
#include "epoc32.h"
#include "fs.h"

// A sis file handle
typedef struct sis_handle *sis_handle;

// Types of file
typedef bits sis_file_type;
#define SIS_FILE_STANDARD ((sis_file_type) 0x00)
#define SIS_FILE_TEXT ((sis_file_type) 0x01)
#define SIS_FILE_SIS ((sis_file_type) 0x02)
#define SIS_FILE_RUN ((sis_file_type) 0x03)
#define SIS_FILE_NONE ((sis_file_type) 0x04)

// Types of buttons for a text file
typedef bits sis_file_buttons;
#define SIS_BUTTONS_CONTINUE ((sis_file_buttons) 0x00)
#define SIS_BUTTONS_SKIP ((sis_file_buttons) 0x01)
#define SIS_BUTTONS_ABORT ((sis_file_buttons) 0x02)

// Types of run action for an executable file
typedef bits sis_file_run;
#define SIS_RUN_INSTALL ((sis_file_run) 0x00)
#define SIS_RUN_REMOVE ((sis_file_run) 0x01)
#define SIS_RUN_BOTH ((sis_file_run) 0x02)

// Selectable drive letter
#define SIS_SELECTABLE_DRIVE '!'

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Parameters  : src           - The name of the SIS file to open.
                  name          - The original name of the SIS file.
                  handle        - Variable to receive a pointer to the handle.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Open a SIS file.
*/
os_error *sis_open(const char *src, const char *name, sis_handle *handle);

/*
    Parameters  : src           - The SIS file handle to clone.
                  dest          - Variable to receive the cloned handle.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Clone a SIS file handle. The cloned handle references the
                  same SIS file.
*/
os_error *sis_clone(sis_handle src, sis_handle *dest);

/*
    Parameters  : handle        - Handle of the SIS file to close.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Close a previously opened SIS file. This should be called the
                  same number of times as sis_open and sis_clone.
*/
os_error *sis_close(sis_handle *handle);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  num           - Variable to receive the number of languages.
                  languages     - Variable to receive a pointer to the list of
                                  languages.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the languages supported by the SIS file.
*/
os_error *sis_get_languages(sis_handle handle, bits *num,
                            const psifs_language **languages);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  language      - Variable to receive the selected language.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the selected language for this SIS file.
*/
os_error *sis_get_language(sis_handle handle, psifs_language *language);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  language      - The language to use.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write the selected language for this SIS file.
*/
os_error *sis_set_language(sis_handle handle, psifs_language language);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  component     - Variable to receive a pointer to the component
                                  name.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the component name for this SIS file.
*/
os_error *sis_get_component(sis_handle handle, const char **component);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  major         - Variable to receive the major version number.
                  minor         - Variable to receive the minor version number.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the version number for this SIS file.
*/
os_error *sis_get_version(sis_handle handle, bits *major, bits *minor);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  variant       - Variable to receive the variant number.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the variant number for this SIS file.
*/
os_error *sis_get_variant(sis_handle handle, bits *variant);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  uid           - Variable to receive the UID.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the UID for this SIS file.
*/
os_error *sis_get_uid(sis_handle handle, epoc32_file_uid *uid);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  drive         - Variable to receive the current installation
                                  drive.
                  selectable    - Variable to receive whether the installation
                                  drive is selectable.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the installation drive for this SIS file.
*/
os_error *sis_get_drive(sis_handle handle, psifs_drive *drive,
                        bool *selectable);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  drive         - The required installation drive.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Write the installation drive for this SIS file.
*/
os_error *sis_set_drive(sis_handle handle, psifs_drive drive);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  num           - Variable to receive the number of files.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the number of files in this SIS file.
*/
os_error *sis_get_files(sis_handle handle, bits *num);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  index         - Index of the file to manipulate.
                  name          - Variable to receive the name of the file.
                  type          - Variable to receive the type of the file.
                  detail        - Variable to receive any associated detail
                                  for the file.
                  size          - Variable to receive the size of the file.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read about a file from this SIS file.
*/
os_error *sis_get_file(sis_handle handle, bits index, const char **name,
                       sis_file_type *type, bits *detail, bits *size);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  index         - Index of the file to manipulate.
                  name          - Name of the file to create.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Extract a file from this SIS file.
*/
os_error *sis_save_file(sis_handle handle, bits index, const char *name);

/*
    Parameters  : src           - Handle of the SIS file to manipulate.
                  index         - Index of the file to manipulate.
                  dest          - Variable to receive the new handle.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Obtain a handle for a component SIS file.
*/
os_error *sis_get_sis(sis_handle src, bits index, sis_handle *dest);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  num           - Variable to receive the number of requisites.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the number of requisites in this SIS file.
*/
os_error *sis_get_requisites(sis_handle handle, bits *num);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  index         - Index of the requisite to manipulate.
                  name          - Variable to receive the name of the requisite.
                  uid           - Variable to receive the UID of the requisite.
                  major         - Variable to receive the major version number
                                  of the variant.
                  minor         - Variable to receive the minor version number
                                  of the variant.
                  variant       - Variable to receive the variant of the
                                  requisite.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the number of requisites in this SIS file.
*/
os_error *sis_get_requisite(sis_handle handle, bits index, const char **name,
                            bits *uid, bits *major, bits *minor, bits *variant);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  name          - Variable to receive the residual SIS file
                                  name.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Read the name for the residual version of this SIS file.
*/
os_error *sis_get_residual(sis_handle handle, const char **name);

/*
    Parameters  : handle        - Handle of the SIS file to manipulate.
                  name          - Name of the file to create.
    Returns     : os_error *    - NULL for success, or pointer to a standard
                                  error block.
    Description : Create the residual version of this SIS file.
*/
os_error *sis_save_residual(sis_handle handle, const char *name);

/*
    Select language for installation:
        Obtain list of languages
        Set language
    Check requisites:
        Read residual SIS files from target
        Check if all requisites are satisfied
        Get details of non-satisfied requisites
    Check previous version:
        Read residual SIS files from target
        Check details of previous installation
    Copy files:
        Check if drive is selectable
        Set drive
        Extract files
            Read next file details
            Extract file
            Get embedded SIS file handle
    Residual SIS file:
        Generate truncated file with updated header
    Removal:
        Read residual SIS files from target
        Check if any components are required
*/

#ifdef __cplusplus
    }
#endif

#endif
