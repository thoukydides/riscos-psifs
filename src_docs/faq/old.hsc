<*
    File        : faq/old.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Old Serial Block Driver" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Block Driver,InternalPC,SP_DualPC,!SerialDev">

<HEADING>Why is the error <ARG>Internal error, no stack for trap handler</ARG>, <ARG>Illegal instruction</ARG> or <ARG>Abort on data transfer</ARG> displayed when attempting to enable the remote link?</HEADING>
<PARA>
Versions of the <ARG>InternalPC</ARG> block driver earlier than 1.12 are incompatible with <PSIFS>. Use of these versions may lead to unpredictable behaviour and fatal errors.
<P>
<PSIFS> is supplied with version 1.12 of the <ARG>InternalPC</ARG> block driver in <ARG>!PsiFS.Resources.Modules.InternalPC</ARG>. However, if <ARG>!SerialDev</ARG> has been seen by the filer then the version inside <ARG>!SerialDev</ARG> is used instead.
<P>
The versions of all available serial block drivers may be displayed using <CMDL CMD="PsiFSListDrivers" HREF=":command/list.html"> <ARG>-verbose</ARG>. If the version of <ARG>InternalPC</ARG> is listed as being earlier than 1.12 then it is strongly recommended that the copy inside <ARG>!SerialDev</ARG> is replaced by the copy inside <PSIFS>.
</PARA>

</PAGE>
