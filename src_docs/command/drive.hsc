<*
    File        : command/drive.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*Drive" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*Drive,Current Drive">

<CMD CMD="Drive" DESC="Sets the current drive">
<CMDS><ARGU>drive</ARGU></CMDS>
<CMDP PARAM="<ARGU>drive</ARGU>">the letter of the disc drive, from <ARG>A</ARG> to <ARG>Z</ARG></CMDP>
<CMDU>
    <CMDN> sets the current drive if <ARG>NoDir</ARG> is set. Otherwise, <CMDN> has no meaning. The command is provided for compatibility with early versions of <ARG>ADFS</ARG>.
</CMDU>
<CMDES CMD="*Drive&nbsp;C">
<CMDR><CMDL CMD="Dir">, <CMDL CMD="NoDir"></CMDR>
</CMD>

</PAGE>
