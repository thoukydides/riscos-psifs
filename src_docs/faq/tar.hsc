<*
    File        : faq/tar.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Unsuitable Tar File Extractor" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Tar,!SparkFS,David Pilling,*PsiFSTar,Backup,Restore,Frank Lancaster,Email">

<HEADING>Why are the file names in a backup corrupt?</HEADING>
<PARA>
Backup files produced by <PSIFS> use the <ARG>fltar</ARG> <RISCOS> variant of <ARG>tar</ARG> files. This stores filenames using <RISCOS> rather than <NAME>UNIX</NAME> conventions. Unfortunately, some tools expect the opposite convention, so end up with corrupt the filenames.
<P>
Recent versions of <ARG>!SparkFS</ARG> operate correctly, but earlier versions get the filenames wrong. If problems are experienced then contact <A HREF="mailto:david@pilling.demon.co.uk" TITLE="Send Email to David Pilling">David Pilling</A> for an upgrade to the latest version.
<P>
Alternatively, <A HREF="mailto:fl@tools.de" TITLE="Send Email to Frank Lancaster">Frank Lancaster</A>'s public domain command line <A HREF="http://www.tools.de/fl/tar.arc"><ARG>tar</ARG></A> utility, or the <CMDL CMD="PsiFSTar" HREF=":command/tar.html"> command may be used without problems.
</PARA>

</PAGE>
