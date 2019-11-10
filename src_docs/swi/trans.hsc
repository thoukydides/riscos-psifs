<*
    File        : swi/trans.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_GetTranslationTable" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_GetTranslationTable,Character Mapping,Latin1,Code Page 850,Windows ANSI">

<SWI NAME="PsiFS_Register" NUM="520D1" DESC="Obtain a character translation table">
    <SWIE REG="R0">source character set</SWIE>
    <SWIE REG="R1" MORE>destination character set</SWIE>
    <SWIE REG="R2" MORE>pointer to 256 buffer to receive the translation table</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call construcst a translation table that can be used to convert between the two specified character sets.
        <P>
        The character sets can be any combination of:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Character set</TH><TH>Description</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>0</TD><TD><RISCOS> (Latin1)</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>1</TD><TD><EPOC> (code page 850)</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT NOWRAP>2</TD><TD><SIBO> (<WINDOWS> ANSI character set)</TD></TR>
        </TABLE>
        <P>
        Indexing the resulting table by a character code from the source character set yields the corresponding character from the destination character set or 0 if the character is unmappable.
    </SWIU>
    <SWIS NONE></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
