<*
    File        : swi/unreg.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_Unregister" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_Unregister,Status">

<SWI NAME="PsiFS_Unregister" NUM="520C1" DESC="Unregister a previously registered client">
    <SWIE REG="R0">the previously allocated pollword for this client</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call unregisters a previously registered client and releases any claimed file intercepts. The same pollword as returned by <SWIL SWI="PsiFS_Register"> must be specified.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_Register" HREF=":swi/reg.html">, <SWIL SWI="PsiFS_InterceptRelease" HREF=":swi/irlse.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
