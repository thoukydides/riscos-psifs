<*
    File        : swi/cpoll.hsc
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_ClipboardCopy" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_ClipboardCopy,Clipboard,File Converters">

<SWI NAME="PsiFS_ClipboardCopy" NUM="520CF" DESC="Write to the remote clipboard">
    <SWIE REG="R0">pointer to null terminated filename</SWIE>
    <SWIO REG="R0">flags</SWIO>
    <SWIO REG="R1" MORE>time stamp of last update to local clipboard</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Copy the contents of the specified file to the remote clipboard. This is only appropriate for <EPOC> devices.
        <P>
        The bits of the flags returned in R0 have the following meanings:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Bit</TH><TH>Meaning when set</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>0</TD><TD>clipboard server active</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>1 ~ 7</TD><TD>reserved</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>8</TD><TD>clipboard changed by remote machine</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>9</TD><TD>local clipboard is up to date</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>10 ~ 15</TD><TD>reserved</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>16</TD><TD>clipboard changed by local machine</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>17</TD><TD>remote clipboard is up to date</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>18 ~ 31</TD><TD>reserved</TD></TR>
        </TABLE>
        <P>
        The time stamp returned in R1 is the value of <SWIL SWI="OS_ReadMonotonicTime"> when the clipboard was updated.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_ClipboardPaste" HREF=":swi/cpste.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
