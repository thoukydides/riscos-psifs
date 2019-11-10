<*
    File        : swi/cpste.hsc
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 2001-2002, 2019
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_ClipboardPaste" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_ClipboardPaste,Clipboard,File Converters">

<SWI NAME="PsiFS_ClipboardPaste" NUM="520D0" DESC="Read from the remote clipboard">
    <SWIE REG="R0">pointer to null terminated filename, or 0 for none</SWIE>
    <SWIO REG="R0">flags</SWIO>
    <SWIO REG="R1" MORE>time stamp of last update to local clipboard</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Paste the contents of the remote clipboard to the specified file. This is only appropriate for <EPOC> devices.
        <P>
        If no filename is specified then this just returns the status of the clipboard.
        <P>
        See <SWIL SWI="PsiFS_ClipboardCopy"> for a description of the return values.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_ClipboardCopy" HREF=":swi/ccopy.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
