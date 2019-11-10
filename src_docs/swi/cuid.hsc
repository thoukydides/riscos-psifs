<*
    File        : swi/cuid.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_CheckUID" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_CheckUID,UID">

<SWI NAME="PsiFS_CheckUID" NUM="520CE" DESC="Calculate the checksum for a UID">
    <SWIE REG="R0">UID1</SWIE>
    <SWIE REG="R1" MORE>UID2</SWIE>
    <SWIE REG="R2" MORE>UID3</SWIE>
    <SWIO REG="R0">calculated checksum, UID4</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Calculate the checksum for an <EPOC> UID. The resulting value should be used for UID4.
    </SWIU>
    <SWIS NONE></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
