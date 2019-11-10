<*
    File        : swi/reg.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_Register" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_Register,Status">

<SWI NAME="PsiFS_Register" NUM="520C0" DESC="Register a client to be informed of changes">
    <SWIE REG="R0">pointer to control-character terminated name of the client</SWIE>
    <SWIE REG="R1" MORE>mask specifying changes of interest</SWIE>
    <SWIO REG="R0">pointer to the pollword for this client</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call registers a client to be informed of changes in the status of the <PSIFS> module or filing system.
        <P>
        The changes of interest are specified by setting bits in the mask passed in R1:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Bit</TH><TH>Changes reported when set</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>0</TD><TD>mode of operation</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>1</TD><TD>serial block driver configuration</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>2 ~ 3</TD><TD>must be 0</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>4</TD><TD>number of bytes or frames transmitted or received</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>5 ~ 7</TD><TD>must be 0</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>8</TD><TD>remote link configuration</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>9</TD><TD>remote link connection status</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>10</TD><TD>remote drive details</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>11</TD><TD>must be 0</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>12</TD><TD>asynchronous operation completed</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>13</TD><TD>asynchronous operation status</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>14 ~ 15</TD><TD>must be 0</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>16</TD><TD>printer mirror configuration</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>17 ~ 19</TD><TD>must be 0</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>20</TD><TD>file transfer intercept status</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>21</TD><TD>clipboard transfer status</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>22</TD><TD>print job status</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>23 ~ 31</TD><TD>must be 0</TD></TR>
        </TABLE>
        <P>
        The returned pollword is suitable for passing to <SWIL SWI="Wimp_Poll">. It should also be passed to <SWIL SWI="PsiFS_Unregister"> to unregister the client. <PSIFS> will set the pollword to a non-zero value whenever there is any relevant change in status. It is the responsibility of the client task to clear the pollword back to zero and to discover what has changed.
        <P>
        The recommended sequence of events is:
        <OL>
            <LI>Wait for the pollword to become non-zero, e.g. by calling <SWIL SWI="Wimp_Poll">.
            <LI>Clear the pollword back to zero.
            <LI>Check for changes to any options of interest by calling <SWIL SWI="PsiFS_Get">, <SWIL SWI="PsiFS_InterceptPoll"> and <SWIL SWI="PsiFS_ClipboardPaste">.
        </OL>
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_Unregister" HREF=":swi/unreg.html">, <SWIL SWI="PsiFS_Get" HREF=":swi/get.html">, <SWIL SWI="PsiFS_InterceptClaim" HREF=":swi/iclam.html">, <SWIL SWI="PsiFS_InterceptPoll" HREF=":swi/ipoll.html">, <SWIL SWI="Wimp_Poll"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
