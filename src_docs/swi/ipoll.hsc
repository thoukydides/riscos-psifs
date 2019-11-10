<*
    File        : swi/ipoll.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_InterceptPoll" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_InterceptPoll,File Converters">

<SWI NAME="PsiFS_InterceptPoll" NUM="520CC" DESC="Poll for any new intercepted file transfers">
    <SWIE REG="R0">the previously allocated pollword for this client</SWIE>
    <SWIO REG="R0">handle for the intercepted file transfer, or 0 if none</SWIO>
    <SWIO REG="R1" MORE>file type intercepted</SWIO>
    <SWIO REG="R2" MORE>intercept type</SWIO>
    <SWIO REG="R3" MORE>pointer to null terminated original filename</SWIO>
    <SWIO REG="R4" MORE>pointer to null terminated path for the copy</SWIO>
    <SWIO REG="R5" MORE>task handle of the sender</SWIO>
    <SWIO REG="R6" MORE>task handle of the receiver</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Poll for new intercepted file transfers. The same pollword as returned by <SWIL SWI="PsiFS_Register"> must be specified. This should be called repeatedly until the returned handle is 0 indicating no more files.
        <P>
        Each intercepted file transfer will have been saved to a temporary file. <SWIL SWI="PsiFS_InterceptControl"> should be used to specify the action to perform. The intercept types are the same as used for the mask supplied to <SWIL SWI="PsiFS_InterceptClaim">. More than one intercept type may be specified, for example if the intercept was forced by the user.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_Register" HREF=":swi/reg.html">, <SWIL SWI="PsiFS_InterceptClaim" HREF=":swi/iclam.html">, <SWIL SWI="PsiFS_InterceptControl" HREF=":swi/ictrl.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
