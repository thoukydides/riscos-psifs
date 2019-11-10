<*
    File        : swi/actrl.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_AsyncControl" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_Async_Control,Asynchronous Remote Operation,Backup,Action Windows">

<SWI NAME="PsiFS_AsyncControl" NUM="520C8" DESC="Control an asynchronous remote operation">
    <SWIE REG="R0">reason code</SWIE>
    <SWIE REG="R1" MORE>the previously allocated handle for this operation</SWIE>
    <SWIE MORE>Other registers depend on reason code</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call controls a previously started asynchronous remote operation, depending on the reason code in R0. Valid reason codes are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>0</TD><TD>Simple response to query</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>1</TD><TD>Pause operation</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>2</TD><TD>Resume operation</TD></TR>
        </TABLE>
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_AsyncStart" HREF=":swi/astrt.html">, <SWIL SWI="PsiFS_AsyncEnd" HREF=":swi/aend.html">, <SWIL SWI="PsiFS_AsyncPoll" HREF=":swi/apoll.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncControl 0" NUM="520C8" DESC="Simple response to query">
    <SWIE REG="R0">0</SWIE>
    <SWIE REG="R1" MORE>the previously allocated handle for this operation</SWIE>
    <SWIE REG="R2" MORE>the response code</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        Supply a simple response to a query returned previously by <SWIL SWI="PsiFS_AsyncPoll">. Valid response codes are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>0</TD><TD>Continue</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>1</TD><TD>Yes</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>2</TD><TD>No</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>3</TD><TD>Quiet</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>4</TD><TD>Skip</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>5</TD><TD>Restart</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>6</TD><TD>Retry</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>7</TD><TD>Copy</TD></TR>
        </TABLE>
        <P>
        The response must be appropriate to the query.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncControl 1" NUM="520C8" DESC="Pause operation">
    <SWIE REG="R0">1</SWIE>
    <SWIE REG="R1" MORE>the previously allocated handle for this operation</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        Pause the specified asynchronous remote operation. This may not have immediate effect; <PSIFS> will wait for a convenient time if necessary.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_AsyncControl 2" NUM="520C8" DESC="Resume operation">
    <SWIE REG="R0">2</SWIE>
    <SWIE REG="R1" MORE>the previously allocated handle for this operation</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        Resume a previously paused asynchronous remote operation.
    </SWIU>
</SWI>

</PAGE>
