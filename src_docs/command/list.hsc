<*
    File        : command/list.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*PsiFSListDrivers" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*PsiFSListDrivers,Block Driver,Version,Baud Rate">

<CMD CMD="PsiFSListDrivers" DESC="List the available serial block drivers">
<CMDS>[-verbose]</CMDS>
<CMDP PARAM="-verbose">display more information about each driver, including the version number and the baud rates supported</CMDP>
<CMDU>
    <CMDN> lists the serial block drivers available for use by <PSIFS>.
</CMDU>
<CMDES CMD="<B>*PsiFSListDrivers</B><BR>Driver&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Information&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Manufacturer<BR>----------&nbsp;-------------------------------&nbsp;-------------------------------<BR>Internal&nbsp;&nbsp;&nbsp;Internal&nbsp;(Acorn&nbsp;wiring)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The&nbsp;Serial&nbsp;Port<BR>InternalPC&nbsp;Internal&nbsp;(PC&nbsp;wiring)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The&nbsp;Serial&nbsp;Port">
<CMDES CMD="<B>*PsiFSListDrivers&nbsp;-verbose</B><BR>Internal:<BR>&nbsp;&nbsp;&nbsp;&nbsp;Information&nbsp;&nbsp;:&nbsp;Internal&nbsp;(Acorn&nbsp;wiring)<BR>&nbsp;&nbsp;&nbsp;&nbsp;Manufacturer&nbsp;:&nbsp;The&nbsp;Serial&nbsp;Port<BR>&nbsp;&nbsp;&nbsp;&nbsp;Driver&nbsp;number:&nbsp;0<BR>&nbsp;&nbsp;&nbsp;&nbsp;Version&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;:&nbsp;1.12<BR>&nbsp;&nbsp;&nbsp;&nbsp;Ports&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;:&nbsp;1<BR>&nbsp;&nbsp;&nbsp;&nbsp;Baud&nbsp;rates&nbsp;&nbsp;&nbsp;:&nbsp;50, 75, 110, 134, 150, 300, 600, 1200, 1800, 2400, 3600, 4800, 7200, 9600, 19200, 38400, 57600 and 115200<BR><BR>InternalPC:<BR>&nbsp;&nbsp;&nbsp;&nbsp;Information&nbsp;&nbsp;:&nbsp;Internal&nbsp;(PC&nbsp;wiring)<BR>&nbsp;&nbsp;&nbsp;&nbsp;Manufacturer&nbsp;:&nbsp;The&nbsp;Serial&nbsp;Port<BR>&nbsp;&nbsp;&nbsp;&nbsp;Driver&nbsp;number:&nbsp;0<BR>&nbsp;&nbsp;&nbsp;&nbsp;Version&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;:&nbsp;1.12<BR>&nbsp;&nbsp;&nbsp;&nbsp;Ports&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;:&nbsp;1<BR>&nbsp;&nbsp;&nbsp;&nbsp;Baud&nbsp;rates&nbsp;&nbsp;&nbsp;:&nbsp;50, 75, 110, 134, 150, 300, 600, 1200, 1800, 2400, 3600, 4800, 7200, 9600, 19200, 38400, 57600 and 115200" MORE>
<CMDR><CMDL CMD="PsiFSDisable" HREF=":command/disbl.html">, <CMDL CMD="PsiFSDriver" HREF=":command/drivr.html">, <CMDL CMD="PsiFSEnable" HREF=":command/enabl.html">, <CMDL CMD="PsiFSStatus" HREF=":command/stats.html"></CMDR>
</CMD>

</PAGE>
