<*
    File        : brand.hsc
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Supplementary macro definitions for the PsiFS
                  documentation.
 
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
*>

<* Colours *>
<$define COLORBG:color/CONST="#FFFFFF">
<$define COLORBGHI:color/CONST="#EEEEBB">
<$define COLORBGM:color/CONST="#CCCCCC">
<$define COLORFG:color/CONST="#000000">
<$define COLORFGSP:color/CONST="#004499">
<$define COLORFGLO:color/CONST="#666666">
<$define COLORFGHI:color/CONST="#DD0000">
<$define COLORFGL:color/CONST="#0000FF">
<$define COLORFGAL:color/CONST="#FF0000">
<$define COLORFGVL:color/CONST=(COLORFGSP)>

<* A copyright link *>
<$macro COPYRIGHTLINK>
    <A HREF=":legal/copy.html" TITLE="Copyright">Copyright</A> &copy; <A HREF=("mailto:" + EMAIL) TITLE=("Send Email to " + AUTHOR)><(AUTHOR)></A>, <(COPYRIGHT)>
</$macro>

<* A promotion of OSLib *>
<$macro OSLIBMENTION>
    A comprehensive <NAME>OSLib</NAME> interface to all of the <PSIFS> SWIs and WIMP messages is included as part of the source code release.
</$macro>