<*
    File        : command/drivr.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*PsiFSDriver" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*PsiFSDriver,Block Driver,Serial Port,Baud Rate,Auto Baud Rate,Printer Mirror,Data Format,Configuration">

<CMD CMD="PsiFSDriver" DESC="Configure the serial block driver to use">
<CMDS>[[-driver]&nbsp;<ARGU>driver</ARGU>] [[-port]&nbsp;<ARGU>port</ARGU>] [[-baud]&nbsp;<ARGU>baud_rate</ARGU>] [-autobaud&nbsp;|&nbsp;-noautobaud] [[-options]&nbsp;<ARGU>options</ARGU>]</CMDS>
<CMDP PARAM="<ARGU>driver</ARGU>">the block driver name</CMDP>
<CMDP PARAM="<ARGU>port</ARGU>" MORE>serial port number (<ARG>0</ARG> for the first)</CMDP>
<CMDP PARAM="<ARGU>baud_rate</ARGU>" MORE>serial baud rate</CMDP>
<CMDP PARAM="<ARGU>-autobaud</ARGU>" MORE>enable automatic baud rate identification</CMDP>
<CMDP PARAM="<ARGU>-noautobaud</ARGU>" MORE>disable automatic baud rate identification</CMDP>
<CMDP PARAM="<ARGU>options</ARGU>" MORE>an optional parameter string specifying extra block driver options</CMDP>
<CMDU>
    <CMDN> configures the serial block driver used by <PSIFS> for both the remote link and connecting to a printer. The current values of any omitted parameters are preserved. If the block driver is active then an error is generated.
    <P>
    The baud rate must be configured the same on both the <RISCOS> and <SIBO> or <EPOC> machines. It should normally be set to the fastest rate supported by both sets of hardware.
    <P>
    If automatic baud rate identification is enabled then the following baud rates are tried in sequence until a connection is established: 9600, 19200, 38400, 57600 and 115200 baud. Automatic baud rate identification is only used for the remote link; the printer mirror always uses the specified baud rate.
    <P>
    The data format and handshaking are always set as:
    <TABLE BORDER=1 ALIGN=CENTER>
        <TR><TH COLSPAN=2 NOWRAP>Data format</TH><TH COLSPAN=2 NOWRAP>Handshaking</TH></TR>
        <TR VALIGN=TOP><TD NOWRAP>Data bits</TD><TD ALIGN=RIGHT>8</TD><TD NOWRAP>XON/XOFF</TD><TD ALIGN=RIGHT>No</TD></TR>
        <TR VALIGN=TOP><TD NOWRAP>Stop bits</TD><TD ALIGN=RIGHT>1</TD><TD NOWRAP>RTS/CTS</TD><TD ALIGN=RIGHT>Yes</TD></TR>
        <TR VALIGN=TOP><TD NOWRAP>Parity</TD><TD ALIGN=RIGHT>Odd</TD><TD NOWRAP>DSR/DTR</TD><TD ALIGN=RIGHT>Yes</TD></TR>
        <TR VALIGN=TOP><TD NOWRAP>Ignore parity</TD><TD ALIGN=RIGHT>Yes</TD><TD NOWRAP>DCD</TD><TD ALIGN=RIGHT>No</TD></TR>
    </TABLE>
    <P>
    These settings are automatically used for the <PLP>, but should be manually configured on the <SIBO> or <EPOC> device when printing via <PSIFS>.
    <P>
    The <ARG>-options</ARG> parameter is not normally required, but may be used to configure special features with some block drivers. If spaces or other special characters need to be included then the string must be enclosed in quotes.
    <P>
    If no parameters are given then the current settings are displayed.
</CMDU>
<CMDE CMD="PsiFSDriver<BR>-driver&nbsp;InternalPC<BR>-port&nbsp;0 -baud&nbsp;115200<BR>-autobaud&nbsp;-options&nbsp;&quot;&quot;">Configure automatic baud rate identification with the internal serial port</CMDE>
<CMDR><CMDL CMD="PsiFSDisable" HREF=":command/disbl.html">, <CMDL CMD="PsiFSEnable" HREF=":command/enabl.html">, <CMDL CMD="PsiFSListDrivers" HREF=":command/list.html">, <CMDL CMD="PsiFSStatus" HREF=":command/stats.html"></CMDR>
</CMD>

</PAGE>
