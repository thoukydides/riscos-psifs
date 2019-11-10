/*
    File        : inst.c++
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Template instantiation for the PsiFS filer.

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

// Include cathlibcpp header files
#include "functional.h"
#include "string.h"

// Include project header files
#include "asyncwin.h"
#include "backcfg.h"
#include "backobj.h"
#include "backop.h"
#include "convobj.h"
#include "convwin.h"
#include "drawobj.h"
#include "fontobj.h"
#include "printpgobj.h"
#include "printjbobj.h"
#include "printctwin.h"
#include "printjbwin.h"
#include "printprwin.h"

// Instantiate a list of asynchronous windows
#include "hlist.c++"
inline void destroy(asyncwin_win **) {}
INSTANTIATE_LIST(asyncwin_win *)

// Instantiate a list of backup objects
inline void destroy(backobj_obj **) {}
INSTANTIATE_LIST(backobj_obj *)

// Instantiate a list of backup operations
inline void destroy(backop_op **) {}
INSTANTIATE_LIST(backop_op *)

// Instantiate a list of backup configuration objects
inline void destroy(backcfg_obj **) {}
INSTANTIATE_LIST(backcfg_obj *)

// Instantiate a list of conversion base windows
inline void destroy(convwin_base **) {}
INSTANTIATE_LIST(convwin_base *)

// Instantiate a list of conversion windows
inline void destroy(convwin_win **) {}
INSTANTIATE_LIST(convwin_win *)

// Instantiate a list of converter objects
inline void destroy(convobj_obj **) {}
INSTANTIATE_LIST(convobj_obj *)

// Instantiate a list of font handles
inline void destroy(fontobj_base **) {}
INSTANTIATE_LIST(fontobj_base *)

// Instantiate a list of draw file objects
inline void destroy(drawobj_base **) {}
INSTANTIATE_LIST(drawobj_base *)

// Instantiate a list of print job objects
inline void destroy(printjbobj_obj **) {}
INSTANTIATE_LIST(printjbobj_obj *)

// Instantiate a list of print job page objects
inline void destroy(printpgobj_obj **) {}
INSTANTIATE_LIST(printpgobj_obj *)

// Instantiate a list of print job control windows
inline void destroy(printctwin_win **) {}
INSTANTIATE_LIST(printctwin_win *)

// Instantiate a list of print job status windows
inline void destroy(printjbwin_win **) {}
INSTANTIATE_LIST(printjbwin_win *)

// Instantiate a list of print job preview windows
inline void destroy(printprwin_win **) {}
INSTANTIATE_LIST(printprwin_win *)

// Instantiate a vector of converter objects
#include "hvector.c++"
INSTANTIATE_VECTOR(convobj_obj *)

// Instantiate a vector of coordinates
inline void destroy(os_coord *) {}
INSTANTIATE_VECTOR(os_coord);

// Instantiate a vector of print job page objects
INSTANTIATE_DESTROY_VECTOR(printpgobj_obj)

// Instantiate a map of strings to strings
#include "hmap.c++"
INSTANTIATE_MAP(string, string, less<string> )

// Instantiate a map of bits to strings
INSTANTIATE_MAP(bits, string, less<bits> )

// Instantiate a map of bits to bits
INSTANTIATE_MAP(bits, bits, less<bits> )

// Instantiate a set of EPOC UIDs
#include "hset.c++"
inline void destroy(epoc32_file_uid *) {}
INSTANTIATE_SET(epoc32_file_uid, less<epoc32_file_uid> )

// Instantiate a set of strings
INSTANTIATE_SET(string, less<string> )
