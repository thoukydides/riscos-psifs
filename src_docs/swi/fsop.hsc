<*
    File        : swi/fsop.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_FileOp" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_FileOp">

<SWI NAME="PsiFS_FileOp" NUM="520C9" DESC="Perform a miscellaneous file system operation">
    <SWIE REG="R0">reason code</SWIE>
    <SWIE MORE>Other registers depend on reason code</SWIE>
    <SWIO>Registers depend on reason code</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call performs a miscellaneous file system operation, depending on the reason code in R0. Valid reason codes are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>0</TD><TD>change the name of a disc</TD></TR>
        </TABLE>
        <P>
        This call extends the facilities available via the standard FileSwitch interface.
    </SWIU>
    <SWIS><SWIL SWI="OS_FSControl"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

<HR>

<SWI NAME="PsiFS_FileOp 0" NUM="520C9" DESC="Change the name of a disc">
    <SWIE REG="R0">0</SWIE>
    <SWIE REG="R1" MORE>drive</SWIE>
    <SWIE REG="R2" MORE>pointer to control-character terminated disc name string</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call changes the name of the specified remote drive, avpoiding the 10 character limit imposed by FileSwitch. The drive may be specified in one of three ways:
        <UL>
            <LI>a zero based index (0 for drive A, through to 25 for drive Z)
            <LI>the ASCII code of the drive letter in upper case (65 for drive A, through to 90 for drive Z)
            <LI>the ASCII code of the drive letter in lower case (97 for drive A, through to 122 for drive Z)
        </UL>
    </SWIU>
</SWI>

</PAGE>
