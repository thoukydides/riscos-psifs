<*
    File        : command/enabl.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*PsiFSEnable" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*PsiFSEnable,Block Driver,Printer Mirror,Remote Link,Connect,Serial Port,Printing">

<CMD CMD="PsiFSEnable" DESC="Enable use of the configured block driver">
<CMDS>[-link | [-print] [<ARGU>device</ARGU>]]</CMDS>
<CMDP PARAM="-link">connect remote link (the default)</CMDP>
<CMDP PARAM="-print" MORE>connect remote device to a printer</CMDP>
<CMDP PARAM="<ARGU>device</ARGU>" MORE>the name of a file or device to send data to</CMDP>
<CMDU>
    <CMDN> activates the configured block driver for either the remote link or connecting to a printer. 
    <P>
    The <ARG>-link</ARG> switch attempts to initiate a connection to a remote <SIBO> or <EPOC> device using the <PLP>. This option is assumed if no parameters are specified.
    <P>
    The <ARG>-print</ARG> switch results in all characters received over the serial link being copied to <ARG><ARGU>device</ARGU></ARG>. Any valid file or output device may be specified. The default is to use <ARG>printer:</ARG> which allows a <SIBO> or <EPOC> device to print to the active <RISCOS> printer without needing a special cable. Useful values for <ARG><ARGU>device</ARGU></ARG> include:
    <TABLE BORDER=1 ALIGN=CENTER>
        <TR><TH>Device</TH><TH>Description</TH></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER NOWRAP><ARG>printer:</ARG></TD><TD>The currently selected printer (the default behaviour).</TD></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER NOWRAP><ARG>null:</ARG>,<BR><ARG>printer#null:</ARG>,<BR><ARG>printer#sink:</ARG><BR>or <ARG>printer#0:</ARG></TD><TD>The null device (discards all data).</TD></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER NOWRAP><ARG>parallel:</ARG>,<BR><ARG>fastparallel:</ARG>,<BR><ARG>printer#parallel:</ARG>,<BR><ARG>printer#centronics:</ARG><BR>or <ARG>printer#1:</ARG></TD><TD>The parallel port.</TD></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER NOWRAP><ARG>serial:</ARG>,<BR><ARG>printer#serial:</ARG>,<BR><ARG>printer#rs423:</ARG><BR>or <ARG>printer#2:</ARG></TD><TD>The internal serial port (only available if <PSIFS> is configured to use a different port).</TD></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER NOWRAP><ARG>printer#user:</ARG><BR>or <ARG>printer#3:</ARG></TD><TD>The user printer device.</TD></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER NOWRAP><ARG>netprint:</ARG></TD><TD>A network printer.</TD></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER NOWRAP><ARG>vdu:</ARG></TD><TD>The computer's VDU stream, filtered using the configured <ARG>DumpFormat</ARG>.</TD></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER NOWRAP><ARG>rawvdu:</ARG></TD><TD>The computer's VDU stream, unfiltered.</TD></TR>
    </TABLE>
    <P>
    If the block driver is already enabled then either no action is taken or an error is generated depending on whether the current and requested modes are the same.
</CMDU>
<CMDES CMD="*PsiFSEnable -link">
<CMDR><CMDL CMD="PsiFSDisable" HREF=":command/disbl.html">, <CMDL CMD="PsiFSDriver" HREF=":command/drivr.html">, <CMDL CMD="PsiFSListDrivers" HREF=":command/list.html">, <CMDL CMD="PsiFSStatus" HREF=":command/stats.html"></CMDR>
</CMD>

</PAGE>
