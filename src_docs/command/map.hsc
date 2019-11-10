<*
    File        : command/map.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*PsiFSMap" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*PsiFSMap,File Type,File Extension,UID,MimeMap,File System">

<CMD CMD="PsiFSMap" DESC="Specifies a mapping between a <SIBO> file extension or <EPOC> UID and a <RISCOS> file type">
<CMDS>[-ext&nbsp;<ARGU>extension</ARGU> | -uid&nbsp;<ARGU>uid</ARGU> | -other] [[-type]&nbsp;<ARGU>filetype</ARGU>] [-mimemap | -nomimemap]</CMDS>
<CMDP PARAM="-ext&nbsp;<ARGU>extension</ARGU>">a <SIBO> file extension of up to three characters</CMDP>
<CMDP PARAM="-uid&nbsp;<ARGU>uid</ARGU>" MORE>an <EPOC> UID (24 digit hexadecimal number)</CMDP>
<CMDP PARAM="-other" MORE>used to specify file type for all unrecognised or unmapped files</CMDP>
<CMDP PARAM="<ARGU>filetype</ARGU>" MORE>a number (in hexadecimal by default) or text description of the file type to be mapped. The command <ARG>*Show&nbsp;File$Type*</ARG> displays a list of valid file types.</CMDP>
<CMDP PARAM="-mimemap" MORE>enables use of the <ARG>MimeMap</ARG> module</CMDP>
<CMDP PARAM="-nomimemap" MORE>disables use of the <ARG>MimeMap</ARG> module</CMDP>
<CMDU>
    <CMDN> specifies a mapping between either a <SIBO> extension or <EPOC> UID and a <RISCOS> file type. Any <SIBO> or <EPOC> file with either the given extension or UID will be treated by <RISCOS> as having the given file type, rather than being of type <ARG>Psion</ARG>. Mappings based on a UID take priority over those based on an extension.
    <P>
    The <ARG>MimeMap</ARG> module, if available, may also be used to convert extensions to file types. This is only tried if there is no match for the internal UID or extension mappings.
    <P>
    If no <RISCOS> filetype is given, then the existing mapping (if any) for that extension or UID is cancelled. If no parameter is given, then all current mappings are listed.
    <P>
    The UID is constructed from the first 3 words of the file represented in hexadecimal (padded with leading zeros to 8 digits if necessary) concatenated to form a single 24 digit value. For example, for an <EPOC> <ARG>Sheet</ARG> file:
    <TABLE BORDER=1 ALIGN=CENTER>
        <TR VALIGN=TOP ALIGN=CENTER><TH NOWRAP>First 12 bytes</TH><TD><ARG>37</ARG></TD><TD><ARG>00</ARG></TD><TD><ARG>00</ARG></TD><TD><ARG>10</ARG></TD><TD><ARG>6D</ARG></TD><TD><ARG>00</ARG></TD><TD><ARG>00</ARG></TD><TD><ARG>10</ARG></TD><TD><ARG>88</ARG></TD><TD><ARG>00</ARG></TD><TD><ARG>00</ARG></TD><TD><ARG>10</ARG></TD></TR>
        <TR VALIGN=TOP ALIGN=CENTER><TH NOWRAP>First 3 words</TH><TD COLSPAN=4><ARG>10000037</ARG></TD><TD COLSPAN=4><ARG>1000006D</ARG></TD><TD COLSPAN=4><ARG>10000088</ARG></TD></TR>
        <TR VALIGN=TOP ALIGN=CENTER><TH NOWRAP>UID</TH><TD COLSPAN=12><ARG>100000371000006D10000088</ARG></TD></TR>
    </TABLE>
    <P>
    The mappings are only retained until the next reset. More permanent changes can be made by modifying the <ARG>!PsiFS.Configure</ARG> file that specifies the default mappings used by <PSIFS>.
</CMDU>
<CMDE CMD="PsiFSMap&nbsp;-ext&nbsp;SHT&nbsp;-type&nbsp;PsiSheet<BR>PsiFSMap&nbsp;-ext&nbsp;SPR&nbsp;-type&nbsp;PsiSheet<BR>PsiFSMap -uid&nbsp;100000371000006D10000088 -type&nbsp;PsiSheet">Treat all files with an SHR or SPR extension or the specified UID as <ARG>PsiSheet</ARG> files. It is not necessary for all of the extensions or UIDs to match; any single match will set the file type.</CMDE>
<CMDE CMD="PsiFSMap&nbsp;-other&nbsp;-type&nbsp;Psion" MORE>Treat all files that have no other mapping defined as <ARG>Psion</ARG> files.</CMDE>
<CMDE CMD="PsiFSMap -uid&nbsp;000000000000000000000000 -type&nbsp;Data" MORE>Treat all files without an <EPOC> UID as <ARG>Data</ARG> files. This disables mapping by extension for <EPOC> files. Exactly 24 zeros must be specified.</CMDE>
<CMDE CMD="PsiFSMap&nbsp;-ext&nbsp;&quot;&quot;&nbsp;-type&nbsp;Text" MORE>Treat all files without a <SIBO> extension as <RISCOS> <ARG>Text</ARG> files. For example, they will have <ARG>Text</ARG> file icons, and load into a text editor when double-clicked on.</CMDE>
<CMDR><CMDL CMD="DOSMap"></CMDR>
</CMD>

</PAGE>
