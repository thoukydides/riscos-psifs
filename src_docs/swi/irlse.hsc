<*
    File        : swi/irlse.hsc
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Part of the PsiFS documentation.
 
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_InterceptRelease" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_InterceptRelease,File Converters">

<SWI NAME="PsiFS_InterceptRelease" NUM="520CB" DESC="Release all intercepted file types">
    <SWIE REG="R0">the previously allocated pollword for this client</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call releases intercepts for all file types. The same pollword as returned by <SWIL SWI="PsiFS_Register"> must be specified.
        <P>
        Any active intercepts for this client are cancelled.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_Register" HREF=":swi/reg.html">, <SWIL SWI="PsiFS_Unregister" HREF=":swi/unreg.html">, <SWIL SWI="PsiFS_InterceptClaim" HREF=":swi/iclam.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
