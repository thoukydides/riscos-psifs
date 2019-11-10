<*
    File        : swi/apoll.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_AsyncPoll" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_AsyncPoll,Asynchronous Remote Operation,Backup,Action Windows">

<SWI NAME="PsiFS_AsyncPoll" NUM="520C7" DESC="Poll the status of an asynchronous remote operation">
    <SWIE REG="R0">the previously allocated handle for this operation</SWIE>
    <SWIO REG="R0">status of the operation</SWIO>
    <SWIO REG="R1" MORE>pointer to null terminated description of status</SWIO>
    <SWIO REG="R2" MORE>pointer to null terminated details, or 0 if none</SWIO>
    <SWIO REG="R3" MORE>pointer to null terminated error text, or 0 if none</SWIO>
    <SWIO REG="R4" MORE>operational time taken in centi-seconds</SWIO>
    <SWIO REG="R5" MORE>estimated time to complete operation in centi-seconds, or 0 if no estimate available</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Poll the status of a previously started asynchronous remote operation. The possible values for the status are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH><TH>Description</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0000</TD><TD>completed successfully</TD><TD ROWSPAN=3 VALIGN=MIDDLE>The operation has terminated</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0001</TD><TD>completed with error</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0002</TD><TD>aborted</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0100</TD><TD>operation starting</TD><TD ROWSPAN=3 VALIGN=MIDDLE>Internal status codes; these should not normally be returned</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0101</TD><TD>operation in progress</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0102</TD><TD>operation delegated</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0200</TD><TD>about to copy a file: waiting for <NAME>yes</NAME> or <NAME>no</NAME></TD><TD ROWSPAN=4 VALIGN=MIDDLE>A response is required via <SWIL SWI="PsiFS_AsyncControl"> to continue the operation</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0201</TD><TD>file failed to open: waiting for <NAME>skip</NAME> or <NAME>retry</NAME></TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0202</TD><TD>previous backup contains newer file: waiting for <NAME>skip</NAME>, <NAME>copy</NAME> or <NAME>quiet</NAME></TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0202</TD><TD>file failed to read: waiting for <NAME>skip</NAME> or <NAME>retry</NAME></TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0300</TD><TD>operation paused</TD><TD VALIGN=MIDDLE> The operation should be resumed using <SWIL SWI="PsiFS_AsyncControl"></TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1000</TD><TD>reading list of open files</TD><TD ROWSPAN=17 VALIGN=MIDDLE>The operation is progressing; the status code and description indicates the current activity</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1001</TD><TD>reading command line for an open file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1002</TD><TD>closing a task</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1003</TD><TD>opening a task</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1100</TD><TD>opening a file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1101</TD><TD>closing a file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1102</TD><TD>reading from a file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1103</TD><TD>writing to a file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1104</TD><TD>creating a directory</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1105</TD><TD>deleting a file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1200</TD><TD>reading catalogue entry</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1201</TD><TD>writing catalogue entry</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1300</TD><TD>keeping file from previous backup</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1301</TD><TD>scrapping file from previous backup</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1302</TD><TD>skipping over file from previous backup</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1303</TD><TD>adding file to backup</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1304</TD><TD>extracting file from backup</TD></TR>
        </TABLE>
        <P>
        The description pointed to be R1 is a summary of the current status (typically the operation being performed), or the question that requires a response. The details optionally pointed to by R2 is typically set to the name of the file or application being operated upon. Any error message or additional details of a problem are pointed to by R3.
        <P>
        The returned pointers only remain valid until the next call of this SWI. If the values need to be kept then they must be copied immediately.
        <P>
        The timings returned in R4 and R5 are estimates for guidance only; they are unlikely to be particularly accurate. 
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_AsyncStart" HREF=":swi/astrt.html">, <SWIL SWI="PsiFS_AsyncEnd" HREF=":swi/aend.html">, <SWIL SWI="PsiFS_AsyncControl" HREF=":swi/actrl.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
