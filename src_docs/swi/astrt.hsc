<*
    File        : swi/astrt.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_AsyncStart" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_AsyncStart,Asynchronous Remote Operation,Backup,Action Windows">

<SWI NAME="PsiFS_AsyncStart" NUM="520C5" DESC="Start an asynchronous remote operation">
    <SWIE REG="R0">reason code</SWIE>
    <SWIE MORE>Other registers depend on reason code</SWIE>
    <SWIO REG="R0">handle for the operation</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call starts an asynchronous remote operation, depending on the reason code in R0. Valid reason codes are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>0</TD><TD>Close the specified open files</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>1</TD><TD>Open the specified files</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>2</TD><TD>Read a single file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>3</TD><TD>Write a single file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>4</TD><TD>Backup a single directory tree</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>5</TD><TD>Write and start a single file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>6</TD><TD>Write and install a file</TD></TR>
        </TABLE>
        <P>
        These operations may only be performed if the remote link is enabled and a connection has been established. If a connection has not been established, then the operation will complete with an error status.
        <P>
        Any <PSIFS> file names supplied as parameters must be of the form <ARG>:<ARGU>drive</ARGU>.$.<ARGU>file/directory</ARGU></ARG> where <ARG><ARGU>drive</ARGU></ARG> is a single character drive letter (<ARG>A</ARG> to <ARG>Z</ARG>), and <ARG><ARGU>file/directory</ARGU></ARG> is the path to the object. Other file names may be specified in any convenient form, although use of canonicalised paths is recommended.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_AsyncEnd" HREF=":swi/aend.html">, <SWIL SWI="PsiFS_AsyncPoll" HREF=":swi/apoll.html">, <SWIL SWI="PsiFS_AsyncControl" HREF=":swi/actrl.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncStart 0" NUM="520C5" DESC="Close the specified open files">
    <SWIE REG="R0">0</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated pattern to match (0 for none)</SWIE>
    <SWIE REG="R2" MORE>pointer to control-character terminated file name</SWIE>
    <SWIE REG="R3" MORE>append to end of file (0 overwrites)</SWIE>
    <SWIO REG="R0">handle for the operation</SWIO>
    <SWIU>
        Close all tasks with open files that match the specified wildcarded pattern. If no pattern is specified then all tasks are closed. A list of the files closed is written to the specified file in a suitable format for subsequently restarting the tasks.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncStart 1" NUM="520C5" DESC="Open the specified files">
    <SWIE REG="R0">1</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated file name</SWIE>
    <SWIE REG="R2" MORE>delete file when finished (0 leaves it)</SWIE>
    <SWIO REG="R0">handle for the operation</SWIO>
    <SWIU>
        Restart the tasks listed in the specified file. Each line of the file should contain two file names separated by a space; the first is the application to start, and the second file is the file to open. If an application should be started without opening a file then the second file name should be left blank, but the space must still be included.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncStart 2" NUM="520C5" DESC="Read a single file">
    <SWIE REG="R0">2</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated source file name</SWIE>
    <SWIE REG="R2" MORE>pointer to control-character terminated destination file name</SWIE>
    <SWIO REG="R0">handle for the operation</SWIO>
    <SWIU>
        Read the specified file from the <SIBO> or <EPOC> device.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncStart 3" NUM="520C5" DESC="Write a single file">
    <SWIE REG="R0">3</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated source file name</SWIE>
    <SWIE REG="R2" MORE>pointer to control-character terminated destination file name</SWIE>
    <SWIE REG="R3" MORE>delete file when finished (0 leaves it)</SWIE>
    <SWIO REG="R0">handle for the operation</SWIO>
    <SWIU>
        Write the specified file to the <SIBO> or <EPOC> device.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncStart 4" NUM="520C5" DESC="Backup a single directory tree">
    <SWIE REG="R0">4</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated source directory</SWIE>
    <SWIE REG="R2" MORE>pointer to control-character terminated destination backup file name</SWIE>
    <SWIE REG="R3" MORE>pointer to control-character terminated previous backup file name (0 for none)</SWIE>
    <SWIE REG="R4" MORE>pointer to control-character terminated scrap backup file name (0 for none)</SWIE>
    <SWIE REG="R5" MORE>pointer to control-character terminated temporary file name</SWIE>
    <SWIO REG="R0">handle for the operation</SWIO>
    <SWIU>
        Perform a backup of all files starting from the specified <SIBO> or <EPOC> directory. The output is written to a <NAME>tar</NAME> file to preserve long filenames, using the <NAME>fltar</NAME> (<ARG>Archie</ARG>) extensions to store datestamp and attribute information. If a previous backup file is specified then unmodified files are simply copied rather than being retrieved from the <SIBO> or <EPOC> device. If a scrap backup file is also specified then files not used from the previous backup file are copied to keep a record of previous versions.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncStart 5" NUM="520C5" DESC="Write and start a single file">
    <SWIE REG="R0">5</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated source file name</SWIE>
    <SWIE REG="R2" MORE>pointer to control-character terminated destination file name</SWIE>
    <SWIE REG="R3" MORE>pointer to control-character terminated executable file name (0 for none)</SWIE>
    <SWIE REG="R4" MORE>delete file when finished (0 leaves it)</SWIE>
    <SWIO REG="R0">handle for the operation</SWIO>
    <SWIU>
        Write the specified file to the <SIBO> or <EPOC> device and start it. If an executable file name is specified then that it used to open the file, otherwise the copied file is itself assumed to be executable.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncStart 6" NUM="520C5" DESC="Write and install a file">
    <SWIE REG="R0">6</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated installer executable file name</SWIE>
    <SWIE REG="R2" MORE>pointer to control-character terminated installer source file name</SWIE>
    <SWIE REG="R3" MORE>pointer to control-character terminated installer destination file name</SWIE>
    <SWIE REG="R4" MORE>delete installer file when finished (0 leaves it)</SWIE>
    <SWIE REG="R5" MORE>pointer to control-character terminated package source file name</SWIE>
    <SWIE REG="R6" MORE>pointer to control-character terminated package destination file name</SWIE>
    <SWIE REG="R7" MORE>delete package file when finished (0 leaves it)</SWIE>
    <SWIO REG="R0">handle for the operation</SWIO>
    <SWIU>
        Ensure that the specified installer is present on the <SIBO> or <EPOC> device, and then use it to open the specified package.
        <P>
        This checks if the specified executable is present, and if not the installer file is copied and executed. Note that all drives are searched for the executable; the drive specified is only used if the installer file is executed. The package file itself is then copied, and the installer executable used to open it.
        <P>
        The typical use of this operation is to install SIS files on <EPOC> devices, ensuring first that the <ARG>Add/remove</ARG> icon is present on the <NAME>control&nbsp;panel</NAME>.
    </SWIU>
</SWI>

</PAGE>
