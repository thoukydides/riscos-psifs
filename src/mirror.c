/*
    File        : mirror.c
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Automatically patch the support module of Mirror for the
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
#include "mirror.h"

// Include clib header files
#include <stdio.h>

// Include oslib header files
#include "oslib/fileswitch.h"
#include "oslib/osmodule.h"

// Include project header files
#include "debug.h"
#include "err.h"
#include "psifs.h"

// Name of the Mirror support module
#define MIRROR_MODULE "MirrorSupport"

// Has the patch been applied
static bool mirror_patched = FALSE;

/*
    Parameters  : start         - The start address of the module.
                  end           - The first byte after the end of the module.
                  patch         - Variable to receive the address of the
                                  instruction to patch, or NULL if no patch is
                                  required.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Patch the Mirror support module at the specified address.
*/
os_error *mirror_search(bits *start, bits *end, bits **patch)
{
    os_error *err = NULL;

    // Just test the single instruction to be patched
    *patch = start[0x1c4c / 4] == 0xe3390004 ? start + (0x1c4c / 4) : NULL;

    // Return any error produced
    return err;
}

/*
    Parameters  : patch         - Pointer to the instruction to patch.
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Patch the Mirror support module at the specified address.
*/
os_error *mirror_patch(bits *patch)
{
    os_error *err = NULL;
    typedef struct
    {
        bits and;
        bits teq;
        bits teqne;
        bits movs;
    } code_block;
    code_block *code;

    DEBUG_PRINTF(("Applying Mirror patch"))

    // Allocate a block of memory (in the RMA) for the extra code
    err = xosmodule_alloc(sizeof(code_block), (void **) &code);
    if (!err && !code) err = &err_buffer;

    // Patch the module if successful
    if (!err)
    {
        // Record the patch
        mirror_patched = TRUE;

        // AND r0, r8, #&ff
        code->and = 0xe20800ff;

        // TEQ r0, #psifs_FS_NUMBER_PSIFS
        code->teq = 0xe3300000 | psifs_FS_NUMBER_PSIFS;

        // TEQNE r9, #4
        code->teqne = 0x13390004;

        // MOVS pc, r14
        code->movs = 0xe1b0f00e;

        // Modify the module to use the new code
        *patch = 0xeb000000 | (((int) (((bits *) code) - patch) - 2) & 0xffffff);

        // Synchronize StrongARM caches
        xos_synchronise_code_areas(1, patch, patch);
        xos_synchronise_code_areas(1, code, ((byte *) code) + sizeof(code) - 4);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Check whether a copy of Mirror is loaded, and patch it if
                  appropriate.
*/
os_error *mirror_check(void)
{
    os_error *err = NULL;
    byte *module;

    DEBUG_PRINTF(("Checking for Mirror"))

    // Attempt to find the module
    if (!xosmodule_lookup(MIRROR_MODULE, NULL, NULL, &module, NULL, NULL)
        && module)
    {
        bits *patch = NULL;

        // Check the module data
        err = mirror_search((bits *) module,
                            (bits *) (module + ((bits *) module)[-1]),
                            &patch);

        // Patch the module if necessary
        if (!err && patch) err = mirror_patch(patch);
    }

    // Return any error produced
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to a corresponding error block, or
                                  NULL if no error.
    Description : Display the current status of the Mirror patch.
*/
os_error *mirror_status(void)
{
    os_error *err = NULL;

    DEBUG_PRINTF(("Displaying Mirror patch status"))

    // No action unless the patch applied
    if (mirror_patched)
    {
        printf("%s module patched.\n", MIRROR_MODULE);
    }

    // Return any error produced
    return err;
}
