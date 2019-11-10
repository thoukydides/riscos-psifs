<*
    File        : swi/pdata.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_PrintJobData" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_PrintJobData,Printing">

<SWI NAME="PsiFS_PrintJobData" NUM="520D3" DESC="Read a page from a print job">
    <SWIE REG="R0">handle of print job</SWIE>
    <SWIE REG="R1" MORE>pointer to null terminated filename, or 0 for none</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Write the data for the next page of a print job to the specified file. This is only appropriate for <EPOC> devices.
        <P>
        If no filename is specified then this discards the data. Note that this is not the same as cancelling the print job, as performed by <SWIL SWI="PsiFS_PrintJobCancel">.
        <P>
        The first page of a print job, number 0, contains dummy data. Reading that data via this call enables reception of the rest of the print job.
        <P>
        Reading the data for the last page of a print job ends the job.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_PrintJobPoll" HREF=":swi/ppoll.html">, <SWIL SWI="PsiFS_PrintJobCancel" HREF=":swi/pcncl.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
