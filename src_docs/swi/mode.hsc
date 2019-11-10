<*
    File        : swi/mode.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_Mode" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_Mode,Connect,Disconnect,Printer Mirror,Remote Link,*PsiFSEnable">

<SWI NAME="PsiFS_Mode" NUM="520C4" DESC="Set the mode of operation">
    <SWIE REG="R0">reason code</SWIE>
    <SWIE MORE>Other registers depend on reason code</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call changes <PSIFS>'s mode of operation, depending on the reason code in R0. Valid reason codes are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>0</TD><TD>Inactive</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>1</TD><TD>Remote link mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>2</TD><TD>Printer mirror mode</TD></TR>
        </TABLE>
        <P>
        These are described separately below.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_Set" HREF=":swi/set.html">, <SWIL SWI="PsiFS_Get" HREF=":swi/get.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

<HR>

<SWI NAME="PsiFS_Mode 0" NUM="520C4" DESC="Set <PSIFS> to inactive mode">
    <SWIE REG="R0">0</SWIE>
    <SWIE REG="R1" MORE>non-zero to disconnect immediately</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call closes any active connection and ends use of the configured block driver. This allows other programs to use the serial port. It is equivalent to using <CMDL CMD="PsiFSDisable" HREF=":command/disbl.html">.
        <P>
        If R1 is set to a non-zero value then <PSIFS> does not wait for data to be flushed, so may result in data being lost. It should only be used as a last resort.
        <P>
        No action is taken if the block driver is already disabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Mode 1" NUM="520C4" DESC="Set <PSIFS> to remote link mode">
    <SWIE REG="R0">1</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call activates the configured block driver and attempts to initiate a connection to a remote <SIBO> or <EPOC> device using <PLP>. It is equivalent to using <CMDL CMD="PsiFSEnable" HREF=":command/enabl.html"> with the <ARG>-link</ARG> switch.
        <P>
        If the block driver is already enabled then either no action is taken or an error is generated depending on whether the remote link is the current mode.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Mode 2" NUM="520C4" DESC="Set <PSIFS> to printer mirror mode">
    <SWIE REG="R0">2</SWIE>
    <SWIE REG="R1" MORE>control-terminated device name (0 for default)</SWIE>
    <SWIO NONE></SWIO>
    <SWIU>
        This call activates the configured block driver and starts copying any characters received over the serial link to the device specified by R1. Any valid file or output device may be specified. The default is to use <ARG>printer:</ARG> which allows a <SIBO> or <EPOC> device to print to the active <RISCOS>  printer without needing a special cable. It is equivalent to using <CMDL CMD="PsiFSEnable" HREF=":command/enabl.html"> with the <ARG>-print</ARG> switch.
        <P>
        If the block driver is already enabled then either no action is taken or an error is generated depending on whether the printer mirror is the current mode.
    </SWIU>
</SWI>

</PAGE>
