<*
    File        : faq/sis.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="SIS File Attributes" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,SIS File,Installation,Attributes,File System,EPOC,Access Denied">

<HEADING>Why is the error <ARG>Access denied</ARG> displayed when attempting to install a <ARG>SIS</ARG> file?</HEADING>
<PARA>
The <ARG>Add/remove</ARG> function used to install <ARG>SIS</ARG> files on <EPOC> devices needs to be able to modify the file being installed. Hence, it is vital that the <ARG>SIS</ARG> file is not set to be <ARG>read&nbsp;only</ARG>, otherwise the <ARG>Access denied</ARG> error will be produced.
<P>
<PSIFS> attempts to preserve file <A HREF=":fs/attr.html" TITLE="Attributes">attributes</A> when they are copied. In particular, a file with <ARG>L/</ARG>, <ARG>R/</ARG>, or <ARG>LR/</ARG> attributes under <RISCOS> will be set to <ARG>read&nbsp;only</ARG> when copied to an <EPOC> device. This is especially likely to occur if the file is copied from a read-only media such as CD-ROM.
<P>
Hence, it may be necesary to change the file's attributes once it has been copied. This may be done either from the <RISCOS> or <EPOC> computer:
<UL>
    <LI>Under <RISCOS> the appropriate attributes may be set by pressing <MOUSE BUTTON=MENU> over the file in a filer window, moving over the <ARG>File</ARG> sub-menu, <ARG>Access</ARG> sub-sub-menu, and then selecting the <ARG>Unprotected</ARG> option.
    <LI>Under <EPOC> the file properties are displayed by selecting <ARG>Properties...</ARG> from the <ARG>File</ARG> menu, or by pressing <ARG>Ctrl-P</ARG>. The <ARG>Read-only</ARG> option should be un-ticked, and the change confirmed by clicking on the <ARG>OK</ARG> icon or pressing <ARG>Enter</ARG>.
</UL>
<P>
Alternatively, using <PSIFS>'s own <A HREF=":start/sis.html" TITLE="Installing EPOC SIS Files">SIS file installer</A> will avoid these problems.
</PARA>

</PAGE>
