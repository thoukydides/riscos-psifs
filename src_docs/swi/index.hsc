<*
    File        : swi/index.hsc
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

<PAGE TITLE="SWI Calls" PARENT=":index.html" KEYWORDS="SWI,Software Interrupt,OSLib Interface,C Veneer,Assembler Header File">

<HEADING>SWI Calls</HEADING>
<PARA>
The following SWIs are implemented by the <PSIFS> module. These are mainly intended for use by the desktop filer, but may be used by other clients.
<UL>
    <LI><SWIL SWI="PsiFS_Register" HREF=":swi/reg.html">
    <LI><SWIL SWI="PsiFS_Unregister" HREF=":swi/unreg.html">
    <LI><SWIL SWI="PsiFS_Set" HREF=":swi/set.html">
    <LI><SWIL SWI="PsiFS_Get" HREF=":swi/get.html">
    <LI><SWIL SWI="PsiFS_Mode" HREF=":swi/mode.html">
    <LI><SWIL SWI="PsiFS_AsyncStart" HREF=":swi/astrt.html">
    <LI><SWIL SWI="PsiFS_AsyncEnd" HREF=":swi/aend.html">
    <LI><SWIL SWI="PsiFS_AsyncPoll" HREF=":swi/apoll.html">
    <LI><SWIL SWI="PsiFS_AsyncControl" HREF=":swi/actrl.html">
    <LI><SWIL SWI="PsiFS_FileOp" HREF=":swi/fsop.html">
    <LI><SWIL SWI="PsiFS_InterceptClaim" HREF=":swi/iclam.html">
    <LI><SWIL SWI="PsiFS_InterceptRelease" HREF=":swi/irlse.html">
    <LI><SWIL SWI="PsiFS_InterceptPoll" HREF=":swi/ipoll.html">
    <LI><SWIL SWI="PsiFS_InterceptControl" HREF=":swi/ictrl.html">
    <LI><SWIL SWI="PsiFS_CheckUID" HREF=":swi/cuid.html">
    <LI><SWIL SWI="PsiFS_ClipboardCopy" HREF=":swi/ccopy.html">
    <LI><SWIL SWI="PsiFS_ClipboardPaste" HREF=":swi/cpste.html">
    <LI><SWIL SWI="PsiFS_GetTranslationTable" HREF=":swi/trans.html">
    <LI><SWIL SWI="PsiFS_PrintJobPoll" HREF=":swi/ppoll.html">
    <LI><SWIL SWI="PsiFS_PrintJobData" HREF=":swi/pdata.html">
    <LI><SWIL SWI="PsiFS_PrintJobCancel" HREF=":swi/pcncl.html">
</UL>
<P>
<OSLIBMENTION>
</PARA>

</PAGE>
