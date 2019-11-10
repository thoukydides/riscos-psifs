<*
    File        : command/disbl.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*PsiFSDisable" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*PsiFSDisable,Disconnect,Block Driver,Serial Port,Printer Mirror,Remote Link,Printing">

<CMD CMD="PsiFSDisable" DESC="Disable use of the configured block driver">
<CMDS>[-abort]</CMDS>
<CMDP PARAM="-abort">disconnect immediately without flushing data</CMDP>
<CMDU>
    <CMDN> closes any active connection and ends use of the configured block driver. This allows other programs to use the serial port.
    <P>
    The <ARG>-abort</ARG> switch does not wait for data to be flushed, so may result in data being lost. It should only be used as a last resort.
    <P>
    No action is taken if the block driver is already disabled.
</CMDU>
<CMDES CMD="*PsiFSDisable">
<CMDR><CMDL CMD="PsiFSDriver" HREF=":command/drivr.html">, <CMDL CMD="PsiFSEnable" HREF=":command/enabl.html">, <CMDL CMD="PsiFSListDrivers" HREF=":command/list.html">, <CMDL CMD="PsiFSStatus" HREF=":command/stats.html"></CMDR>
</CMD>

</PAGE>
