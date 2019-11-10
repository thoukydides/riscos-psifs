<*
    File        : swi/set.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_Set" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_Set,Configuration,Block Driver,Baud Rate,Auto Baud Rate,Synchronize Clocks">

<SWI NAME="PsiFS_Set" NUM="520C2" DESC="Modify a <PSIFS> option">
    <SWIE REG="R0">option to modify</SWIE>
    <SWIE MORE>If the option specified by R0 requires a numeric value</SWIE>
    <SWIE REG="R1" INDENT MORE>value to set</SWIE>
    <SWIE MORE>If the option specified by R0 requires a string value</SWIE>
    <SWIE REG="R1" INDENT MORE>pointer to control-character terminated string</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call sets the value of a <PSIFS> option. An error may be returned if an attempt is made to modify an option that is meaningless for the current mode of operation.
        <P>
        The options that may be set are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>R0</TH><TH>Type</TH><TH>Option</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;100</TD><TD ALIGN=CENTER>string</TD><TD>name of the serial block driver</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;101</TD><TD ALIGN=CENTER>numeric</TD><TD>serial block driver port number</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;102</TD><TD ALIGN=CENTER>numeric</TD><TD>serial block driver baud rate</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;103</TD><TD ALIGN=CENTER>string</TD><TD>optional extra serial block driver options</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;104</TD><TD ALIGN=CENTER>numeric</TD><TD>automatic baud rate identification mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;110</TD><TD ALIGN=CENTER>numeric</TD><TD>clock synchronization mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;120</TD><TD ALIGN=CENTER>numeric</TD><TD>remote link idle disconnect time</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;121</TD><TD ALIGN=CENTER>numeric</TD><TD>printer mirror idle disconnect time</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;122</TD><TD ALIGN=CENTER>numeric</TD><TD>idle disconnect external power mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;128</TD><TD ALIGN=CENTER>numeric</TD><TD>idle background operations mode</TD></TR>
        </TABLE>
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_Get" HREF=":swi/get.html">, <SWIL SWI="PsiFS_Mode" HREF=":swi/mode.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;100" NUM="520C2" DESC="Set the serial block driver name">
    <SWIE REG="R0">&amp;100</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated serial block driver name string</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call sets the name of the configured serial block driver.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;101" NUM="520C2" DESC="Set the serial block driver port number">
    <SWIE REG="R0">&amp;101</SWIE>
    <SWIO REG="R1">port number (0 for the first)</SWIO>
    <SWIO NONE></SWIO>
    <SWIU>
        This call sets the configured serial block driver port number.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;102" NUM="520C2" DESC="Set the serial block driver baud rate">
    <SWIE REG="R0">&amp;102</SWIE>
    <SWIO REG="R1">baud rate</SWIO>
    <SWIO NONE></SWIO>
    <SWIU>
        This call sets the configured serial block driver baud rate. If the requested baud rate is not available then a valid alternative is selected.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;103" NUM="520C2" DESC="Set the serial block driver options">
    <SWIE REG="R0">&amp;103</SWIE>
    <SWIE REG="R1" MORE>pointer to control-character terminated serial block driver options string</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call sets the optional parameter string of the configured serial block driver.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;104" NUM="520C2" DESC="Set the automatic baud rate identification mode">
    <SWIE REG="R0">&amp;104</SWIE>
    <SWIE REG="R1" MORE>automatic baud rate identification (0 disables)</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call enables or disables automatic baud rate identification. Only the remote link is affected; the printer mirror always uses the configured baud rate.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;110" NUM="520C2" DESC="Set the clock synchronization mode">
    <SWIE REG="R0">&amp;110</SWIE>
    <SWIE REG="R1" MORE>clock synchronization mode (0 disables)</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call enables or disables synchronization of clocks on connection of the remote link. This only affects <EPOC> devices; it is not possible to synchronize the clock of <SIBO> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;120" NUM="520C2" DESC="Set the remote link idle disconnect time">
    <SWIE REG="R0">&amp;120</SWIE>
    <SWIE REG="R1" MORE>idle disconnect time in seconds (0 disables)</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call enables or disables automatic disconnection of the remote link after a period of activity.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;121" NUM="520C2" DESC="Set the printer mirror idle disconnect time">
    <SWIE REG="R0">&amp;121</SWIE>
    <SWIE REG="R1" MORE>idle disconnect time in seconds (0 disables)</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call enables or disables automatic disconnection of the printer mirror after a period of activity.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;122" NUM="520C2" DESC="Set the idle disconnect external power mode">
    <SWIE REG="R0">&amp;122</SWIE>
    <SWIE REG="R1" MORE>idle disconnect external power mode (0 disables)</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call enables or disables automatic disconnection after a period of inactivity if running on external power. This only affects <EPOC> devices; it is not possible to determine the status of external power for <SIBO> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Set &amp;128" NUM="520C2" DESC="Set the idle background operations mode">
    <SWIE REG="R0">&amp;128</SWIE>
    <SWIE REG="R1" MORE>idle background operations mode (0 disables)</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call enables or disables throttling back of background operations when idle.
    </SWIU>
</SWI>

</PAGE>
