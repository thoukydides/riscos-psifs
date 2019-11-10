<*
    File        : fs/attr.hsc
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

<PAGE TITLE="File System" SUBTITLE="Attributes" PARENT=":fs/index.html" KEYWORDS="File System,Attributes">

<HEADING>Attributes</HEADING>
<PARA>
<RISCOS> <B>owner</B> attributes are mapped to and from <SIBO> or <EPOC> <ARG>read&nbsp;only</ARG> (or not <ARG>writable</ARG>) and <ARG>system</ARG> attributes as follows:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH NOWRAP ROWSPAN=2><RISCOS></TH><TH NOWRAP COLSPAN=2><SIBO>&nbsp;or&nbsp;<EPOC></TH></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD NOWRAP><ARG>read only</ARG></TD><TD><ARG>system</ARG></TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>&nbsp;&nbsp;&nbsp;/&nbsp;&nbsp;</ARG></TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>&nbsp;&nbsp;R/&nbsp;&nbsp;</ARG></TD><TD>Yes</TD><TD>&nbsp;</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>&nbsp;W&nbsp;/&nbsp;&nbsp;</ARG></TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>&nbsp;WR/&nbsp;&nbsp;</ARG></TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>L&nbsp;&nbsp;/&nbsp;&nbsp;</ARG></TD><TD>Yes</TD><TD>&nbsp;</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>L&nbsp;R/&nbsp;&nbsp;</ARG></TD><TD>Yes</TD><TD>Yes</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>LW&nbsp;/&nbsp;&nbsp;</ARG></TD><TD>&nbsp;</TD><TD>Yes</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>LWR/&nbsp;&nbsp;</ARG></TD><TD>&nbsp;</TD><TD>Yes</TD></TR>
</TABLE>
<P>
<RISCOS> <B>public</B> attributes are mapped to and from <SIBO> or <EPOC> <ARG>archive</ARG> (or <ARG>modified</ARG>) and <ARG>hidden</ARG> attributes as follows:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH NOWRAP ROWSPAN=2><RISCOS></TH><TH NOWRAP COLSPAN=2><SIBO>&nbsp;or&nbsp;<EPOC></TH></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>archive</ARG></TD><TD><ARG>hidden</ARG></TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>&nbsp;&nbsp;&nbsp;/&nbsp;&nbsp;</ARG></TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>&nbsp;&nbsp;&nbsp;/&nbsp;r</ARG></TD><TD>&nbsp;</TD><TD>Yes</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>&nbsp;&nbsp;&nbsp;/w&nbsp;</ARG></TD><TD>Yes</TD><TD>Yes</TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TD><ARG>&nbsp;&nbsp;&nbsp;/wr</ARG></TD><TD>Yes</TD><TD>&nbsp;</TD></TR>
</TABLE>
<P>
This mapping preserves all <SIBO> or <EPOC> attributes except for the <ARG>system</ARG> attribute of directories, which is converted to <ARG>read&nbsp;only</ARG> due to limitations of <RISCOS> filesystems.
</PARA>

</PAGE>
