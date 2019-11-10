<*
    File        : swi/pcncl.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_PrintJobCancel" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_PrintJobCancel,Printing">

<SWI NAME="PsiFS_PrintJobCancel" NUM="520D4" DESC="Cancel a print job">
    <SWIE REG="R0">handle of print job</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Cancel a print job. This is only appropriate for <EPOC> devices.
        <P>
        If this is used while the print job is still being received, the original source is cancelled. Otherwise, this just discards the data that has been queued.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_PrintJobPoll" HREF=":swi/ppoll.html">, <SWIL SWI="PsiFS_PrintJobData" HREF=":swi/pdata.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
