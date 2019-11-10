<*
    File        : swi/ppoll.hsc
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2002, 2019
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_PrintJobPoll" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_PrintJobPoll,Printing">

<SWI NAME="PsiFS_PrintJobPoll" NUM="520D2" DESC="Poll the status of a print job">
    <SWIE REG="R0">handle of print job, or 0 for next</SWIE>
    <SWIO REG="R0">handle of print job, or 0 for none</SWIO>
    <SWIO REG="R1" MORE>status of job</SWIO>
    <SWIO REG="R2" MORE>number of pages received</SWIO>
    <SWIO REG="R3" MORE>number of pages read</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Poll the status of the specified print job. This is only appropriate for <EPOC> devices.
        <P>
        If R0 is 0 on entry then this returns information about the next unclaimed print job. The handle of the print job is returned in R0.
        <P>
        The possible values for the status are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Description</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD>no print job active</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>1</TD><TD>start of print job received but not accepted</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>2</TD><TD>print job accepted</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>3</TD><TD>print job complete</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>4</TD><TD>print job cancelled</TD></TR>
        </TABLE>
        <P>
        The values in R2 and R3 are the counts of the number of complete pages received and the number of pages read via <SWIL SWI="PsiFS_PrintJobData">. Each print job starts with a dummy page, so these counts are both one higher than the number of real pages.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_PrintJobData" HREF=":swi/pdata.html">, <SWIL SWI="PsiFS_PrintJobCancel" HREF=":swi/pcncl.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
