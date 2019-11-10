<*
    File        : swi/ictrl.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_InterceptControl" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_InterceptControl,File Converters">

<SWI NAME="PsiFS_InterceptControl" NUM="520CD" DESC="Control an intercepted file transfer">
    <SWIE REG="R0">reason code</SWIE>
    <SWIE REG="R1" MORE>the previously allocated handle for this intercept</SWIE>
    <SWIE MORE>Other registers depend on reason code</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call controls an intercepted file transfer, depending on the reason code in R0. Valid reason codes are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>0</TD><TD>Restart the transfer</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>1</TD><TD>Replace the transfer with a different file</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>2</TD><TD>Cancel the transfer</TD></TR>
        </TABLE>
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_InterceptClaim" HREF=":swi/iclam.html">, <SWIL SWI="PsiFS_InterceptPoll" HREF=":swi/ipoll.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

<HR>

<SWI NAME="PsiFS_InterceptControl 0" NUM="520CD" DESC="Restart the transfer">
    <SWIE REG="R0">0</SWIE>  
    <SWIE REG="R1" MORE>the previously allocated handle for this intercept</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        Restart the transfer without any changes. The temporary file will be deleted.
        <P>
        This should not normally be called if any tasks have closed or any windows have been moved.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_InterceptControl 1" NUM="520CD" DESC="Replace the transfer with a different file">
    <SWIE REG="R0">1</SWIE>  
    <SWIE REG="R1" MORE>the previously allocated handle for this intercept</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        Restart the transfer with the temporary file replaced by a different file. The replacement file will be deleted.
        <P>
        This should not normally be called if any tasks have closed or any windows have been moved.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_InterceptControl 2" NUM="520CD" DESC="Cancel the transfer">
    <SWIE REG="R0">0</SWIE>  
    <SWIE REG="R1" MORE>the previously allocated handle for this intercept</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        Cancel the transfer. The temporary file will be deleted.
    </SWIU>
</SWI>

</PAGE>
