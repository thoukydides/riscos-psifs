<*
    File        : fs/chars.hsc
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

<PAGE TITLE="File System" SUBTITLE="Character Mappings" PARENT=":fs/index.html" KEYWORDS="File System,Character Mapping,File Name,Latin1,Code Page 850,Windows ANSI">

<HEADING>Character Mappings</HEADING>
<PARA>
<PSIFS> attempts to preserve special characters in filenames. Unfortunately, the standard character sets of <RISCOS> (Latin1), <SIBO> (code page 850) and <EPOC> (<WINDOWS> ANSI character set) are all different, so not all filenames can be preserved unchanged.
<P>
If a <RISCOS> file is copied to a <SIBO> or <EPOC> device then some unmappable characters may be converted to &quot;<ARG>_</ARG>&quot; (underscore) symbols. In the opposite direction, from a <SIBO> or <EPOC> device to <RISCOS>, characters without a direct mapping are converted to a unique sequence of characters. This ensures that <SIBO> or <EPOC> files can be copied and restored without any change of name.
<P>
It is beyond the scope of this documentation to fully describe the mapping between the various character sets. However, the following special symbols are mapped in a similar manner to <DOSFS>:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR VALIGN=TOP ALIGN=CENTER><TH><RISCOS></TH><TD NOWRAP>hard space</TD><TD><ARG>&lt;</ARG></TD><TD><ARG>&gt;</ARG></TD><TD><ARG>#</ARG></TD><TD><ARG>?</ARG></TD><TD><ARG>.</ARG></TD><TD><ARG>/</ARG></TD></TR>
    <TR VALIGN=TOP ALIGN=CENTER><TH><SIBO>&nbsp;or&nbsp;<EPOC></TH><TD>space</TD><TD><ARG>$</ARG></TD><TD><ARG>^</ARG></TD><TD><ARG>?</ARG></TD><TD><ARG>#</ARG></TD><TD><ARG>\</ARG></TD><TD><ARG>.</ARG></TD></TR>
</TABLE>
<P>
Finally, <SIBO> only supports upper-case filenames. <PSIFS> converts such names so that the first character is upper-case and all other characters are lower-case. This is similar to the approach used by <WINDOWS VER="95/98"> to display old <MSDOS> style filenames.
</PARA>

</PAGE>
