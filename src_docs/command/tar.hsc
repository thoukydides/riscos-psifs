<*
    File        : command/tar.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*PsiFSTar" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*PsiFSTar,Tar,Backup,Restore,!SparkFS">

<CMD CMD="PsiFSTar" DESC="Read the contents of a tar file">
<CMDS><ARGU>tar_file</ARGU> [<ARGU>object_spec</ARGU> [<ARGU>directory</ARGU>]] [-verbose]</CMDS>
<CMDP PARAM="<ARGU>tar_file</ARGU>">a valid pathname specifying a <ARG>tar</ARG> file</CMDP>
<CMDP PARAM="<ARGU>object_spec</ARGU>" MORE>a wildcard specification for matching against</CMDP>
<CMDP PARAM="<ARGU>directory</ARGU>" MORE>a valid pathname specifying a directory</CMDP>
<CMDP PARAM="-verbose" MORE>display a list of matching objects</CMDP>
<CMDU>
    <CMDN> reads the contents of a <ARG>tar</ARG> file. All of the variants supported by <NAME>SparkFS</NAME> are handled, including <ARG>arctar</ARG>, <ARG>comma</ARG>, <ARG>fltar</ARG> and <ARG>unix</ARG>. However, this command is intended solely to allow restoration of backup files produced by <PSIFS>.
    <P>
    The default pattern is <ARG>*</ARG>, which will match all objects within the source file. The default directory is <ARG>@</ARG>, which corresponds to the current directory.
    <P>
    The <ARG>-verbose</ARG> switch displays a listing of the matching files contained within the source file. If this switch is used without specifically specifying a directory then no files will be extracted from the <ARG>tar</ARG> file.
</CMDU>
<CMDE CMD="*PsiFSTar Backup<BR>-verbose">Display a listing of all files in <ARG>Backup</ARG></CMDE>
<CMDE CMD="*PsiFSTar Backup<BR>*/txt PsiFS::Internal.$" MORE>Restore all files with a <ARG>txt</ARG> extension to their original directories on the <ARG>Internal</ARG>drive</CMDE>
<CMDR><CMDL CMD="PsiFSSIS" HREF=":command/sis.html"></CMDR>
</CMD>

</PAGE>
