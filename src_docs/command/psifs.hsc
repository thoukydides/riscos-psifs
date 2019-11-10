<*
    File        : command/psifs.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*PsiFS" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*PsiFS,File System,Current File System">

<CMD CMD="PsiFS" DESC="Selects the <PSIFS LONG> as the current filing system">
<CMDS></CMDS>
<CMDP></CMDP>
<CMDU>
    <CMDN> selects the <PSIFS LONG> as the filing system for subsequent operations. Remember that it is not necessary to switch filing systems if you use the full pathnames of objects. For example, you can refer to <ARG>ADFS</ARG> objects (on a local harddisc, say) when <PSIFS> is the current filing system.
</CMDU>
<CMDES CMD="*PsiFS">
<CMDR><CMDL CMD="ADFS">, <CMDL CMD="Net">, <CMDL CMD="RAM">, <CMDL CMD="ResourceFS">, <CMDL CMD="SCSI"></CMDR>
</CMD>

</PAGE>
