<*
    File        : swi/iclam.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_InterceptClaim" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_InterceptClaim,File Converters">

<SWI NAME="PsiFS_InterceptClaim" NUM="520CA" DESC="Claim an intercepted file type">
    <SWIE REG="R0">the previously allocated pollword for this client</SWIE>
    <SWIE REG="R1" MORE>file type to intercept</SWIE>
    <SWIE REG="R2" MORE>mask of intercept types</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call allows file transfers within the desktop for the specified file type to be intercepted. The same pollword as returned by <SWIL SWI="PsiFS_Register"> must be specified. The types of intercept to claim are specified by setting bits in the mask passed in R2:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Bit</TH><TH>Intercept claimed when set</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>0</TD><TD>forced by user</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>1</TD><TD>load from filer</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>2</TD><TD>save to filer</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>3</TD><TD>transfer between applications</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>4</TD><TD>all opens from filer</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>5</TD><TD>unclaimed opens from filer</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>6 ~ 31</TD><TD>must be 0</TD></TR>
        </TABLE>
        <P>
        If no bits are set in the mask then the intercept is released. Alternatively, all intercepts may be released by calling <SWIL SWI="PsiFS_InterceptRelease"> or <SWIL SWI="PsiFS_Unregister">.
        <P>
        The user may force a file transfer intercept by pressing <ARG>Left&nbsp;Alt</ARG> or disable an intercept by pressing <ARG>Right&nbsp;Alt</ARG>.
        <P>
        <SWIL SWI="PsiFS_InterceptPoll"> should be used to check for new intercepted file transfers when the poll word is non-zero.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_Register" HREF=":swi/reg.html">, <SWIL SWI="PsiFS_Unregister" HREF=":swi/unreg.html">, <SWIL SWI="PsiFS_InterceptRelease" HREF=":swi/irlse.html">, <SWIL SWI="PsiFS_InterceptPoll" HREF=":swi/ipoll.html">, <SWIL SWI="PsiFS_InterceptControl" HREF=":swi/ictrl.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
