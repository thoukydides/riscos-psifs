<*
    File        : faq/print.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Inaccurate Printing" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Printing">

<HEADING>Why are print jobs rendered inaccurately?</HEADING>
<PARA>
There are two main reasons why <EPOC> print jobs are not rendered accurately by <PSIFS>:
<UL>
    <LI>The <EPOC> print job file format is not entirely understood. In particular the purpose of some common primitives is not known, so they are simply ignored.
    <LI>The <RISCOS> graphics support does not match that of <EPOC>, especially when constrained to the features supported by <NAME>Draw</NAME> files. The most deficiency is the poor support for clipping.
</UL>
<P>
The <PLP> document, available from the <PSIFS> <A HREF=":contact/web.html" TITLE="Web Page">web page</A>, contains a description of the <EPOC> print job file format as currently understood. Please <A HREF=":contact/auth.html" TITLE="Contacting the Author">contact the author</A> with any additions or corrections.
<P>
Print jobs can be annotated by adding the line <ARG>PrintJobLogDebug=True</ARG> to the <PSIFS> configuration file (either <ARG>!Boot.Choices.PsiFS.Config</ARG> or <ARG>!PsiFS.Config.Config</ARG>, depending upon the computer's configuration). This tags each rendered primitive with a list of the primitives that immediately precede it. The name of each primitive is prefixed by the file offset to that primitive in hexadecimal.
</PARA>

</PAGE>
