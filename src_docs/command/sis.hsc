<*
    File        : command/sis.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*PsiFSSIS" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*PsiFSSIS,SIS File">

<CMD CMD="PsiFSSIS" DESC="Read the contents of an <EPOC> SIS file">
<CMDS><ARGU>sis_file</ARGU> [[-dir]&nbsp;<ARGU>directory</ARGU>&nbsp;|&nbsp;-tar&nbsp;<ARGU>tar_file</ARGU>] [-scrap&nbsp;<ARGU>temp_file</ARGU>] [-language&nbsp;<ARGU>language</ARGU>] [-recurse] [-drive&nbsp;<ARGU>drive</ARGU>] [-residual&nbsp;<ARGU>residual_file</ARGU>] [-verbose]</CMDS>
<CMDP PARAM="<ARGU>sis_file</ARGU>">a valid pathname specifying an <EPOC> SIS file</CMDP>
<CMDP PARAM="<ARGU>directory</ARGU>" MORE>a valid pathname specifying an output directory</CMDP>
<CMDP PARAM="<ARGU>tar_file</ARGU>" MORE>a valid pathname specifying an output <ARG>tar</ARG> file</CMDP>
<CMDP PARAM="<ARGU>temp_file</ARGU>" MORE>a valid pathname specifying a temporary file</CMDP>
<CMDP PARAM="<ARGU>language</ARGU>" MORE>a two character language code</CMDP>
<CMDP PARAM="-recurse" MORE>recursively process component SIS files</CMDP>
<CMDP PARAM="<ARGU>drive</ARGU>" MORE>the letter of the installation disc drive, from <ARG>A</ARG> to <ARG>Z</ARG></CMDP>
<CMDP PARAM="<ARGU>residual_file</ARGU>" MORE>leafname for the residual SIS file</CMDP>
<CMDP PARAM="-verbose" MORE>display a description of contents</CMDP>
<CMDU>
    <CMDN> reads the contents of an <EPOC> SIS file. This can either extract the files to a standard directory structure or to a <ARG>tar</ARG> file in <ARG>fltar</ARG> format; the latter is recommended to ensure that filenames are preserved. Note that this cannot be used for installing applications directly to a connected device.
    <P>
    A temporary file is required while processing the contents of the SIS file. If the <ARG>-scrap</ARG> switch is not specified then <ARG>&lt;Wimp$Scrap&gt;</ARG> is used.
    <P>
    The language code allows the installation language to be selected from those supported by the SIS file. If it is not specified then <PSIFS> attempts to make an intelligent choice. The possible codes are:
    <TABLE BORDER=1 ALIGN=CENTER>
        <TR><TH>Code</TH><TH>Language</TH></TR>
        <TR><TD ALIGN=CENTER><ARG>EN</ARG></TD><TD>English</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>FR</ARG></TD><TD>French</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>GE</ARG></TD><TD>German</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>SP</ARG></TD><TD>Spanish</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>IT</ARG></TD><TD>Italian</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>SW</ARG></TD><TD>Swedish</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>DA</ARG></TD><TD>Danish</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>NO</ARG></TD><TD>Norwegian</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>FI</ARG></TD><TD>Finnish</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>AM</ARG></TD><TD>American</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>SF</ARG></TD><TD>Swiss French</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>SG</ARG></TD><TD>Swiss German</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>PO</ARG></TD><TD>Portuguese</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>TU</ARG></TD><TD>Turkish</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>IC</ARG></TD><TD>Icelandic</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>RU</ARG></TD><TD>Russian</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>HU</ARG></TD><TD>Hungarian</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>DU</ARG></TD><TD>Dutch</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>BL</ARG></TD><TD>Belgian Flemish</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>AU</ARG></TD><TD>Australian</TD></TR>
        <TR><TD ALIGN=CENTER><ARG>BG</ARG></TD><TD>Belgian French</TD></TR>
    </TABLE>
    <P>
    SIS files may contain nested SIS files for shared components. These may be extracted recursively by specifying the <ARG>-recurse</ARG> switch. If this is not done then the nested files are extracted unmodified. The same <ARG><ARGU>language</ARGU></ARG> and <ARG><ARGU>drive</ARGU></ARG> options are used for all components.
    <P>
    Some SIS files allow the installation drive to be selected. If no installation drive is specified using the <ARG>-drive</ARG> switch then <PSIFS> will default to drive <ARG>C</ARG>.
    <P>
    When a SIS file is installed on an <EPOC> device a residual SIS file is created in the <ARG>C:\System\Install</ARG> directory. <CMDN> will generate the residual SIS if the <ARG>-residual</ARG> switch is used. The leafname of the residual SIS file and installation drive must both be specified.
    <P>
    The <ARG>-verbose</ARG> switch enables display of detailed information about the SIS file. This is recommended to enable checking of requisites prior to attempting an installation.
</CMDU>
<CMDE CMD="*PsiFSSIS File/sis<BR>-verbose">Describe contents of <ARG>file/sis</ARG></CMDE>
<CMDE CMD="*PsiFSSIS File/sis<BR>-tar&nbsp;Output<BR>-scrap&nbsp;ScrapFile<BR>-language&nbsp;FR<BR>-recurse -drive&nbsp;D<BR>-residual&nbsp;File/sis" MORE>Recursively extract to a tar file for installation on drive <ARG>D</ARG> in French</CMDE>
<CMDR><CMDL CMD="PsiFSTar" HREF=":command/tar.html"></CMDR>
</CMD>

</PAGE>
