<*
    File        : wimp/print.hsc
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

<PAGE TITLE="WIMP Messages" SUBTITLE="Message_PsifsPrint" PARENT=":wimp/index.html" KEYWORDS="WIMP Messages,Printing">

<WIMP NAME="Message_PsifsPrint" NUM="520C1">
    <WIMPB OFFSET="20">preferred action</WIMPB>
    <WIMPB OFFSET="24">initial position x coordinate (-1 for automatic)</WIMPB>
    <WIMPB OFFSET="28">initial position y coordinate (-1 for automatic)</WIMPB>
    <WIMPB OFFSET="32">filename, zero-terminated</WIMPB>
    <WIMPU>
        This message is broadcast by a task that wants the <PSIFS> filer to process a page from a print job.
        <P>
        The possible actions are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Action</TH><TH>Description</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD>open control window</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>1</TD><TD>display preview</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>2</TD><TD>start printing</TD></TR>
        </TABLE>
        <P>
        If <PSIFS> accepts the print job then it will take a copy of the file and acknowledge the message.
    </WIMPU>
</WIMP>

</PAGE>
