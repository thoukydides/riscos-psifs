<*
    File        : probs.hsc
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

<PAGE TITLE="Known Problems" PARENT=":index.html" KEYWORDS="Problems,Bugs,Version,Missing Features,Serial Port,Device Claim Protocol">

<HEADING>Problems With This Version</HEADING>
<PARA>
See the <A TITLE="Frequently Asked Questions" HREF=":faq/index.html">frequently asked questions</A> for details of other problems that may be experienced.
<UL>
    <LI>Under <RISCOS VER="4"> enabling the printer mirror without an active printer connected can result in an infinite error loop due to the new parallel port error detection. There probably isn't much that can be done about this.
    <LI><RISCOS> attributes such as <ARG>LR/</ARG>, <ARG>LW/</ARG> or <ARG>LWR/</ARG> result in the <EPOC> <ARG>system</ARG> attribute being set. Unfortunately, this makes it impossible to delete the file from an <EPOC> device.
    <LI>Corrupt <ARG>tar</ARG> files can result in confusing error messages.
    <LI>Some print jobs, notably those from <NAME>Sheet</NAME>, have spurious lines plotted.
</UL>
</PARA>

<HEADING>Missing Features</HEADING>
<PARA>
<UL>
    <LI>It may be useful to allow just part of a disc to be backed up.
</UL>
</PARA>

</PAGE>
