<*
    File        : conv/boot.hsc
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

<PAGE TITLE="File Format Converters" SUBTITLE="!Boot" PARENT=":conv/index.html" KEYWORDS="File Converters,File Formats">

<HEADING><ARG>!Boot</ARG></HEADING>
<PARA>
Any files called <ARG>!PsiFS.Converters.<ARGU>converter</ARGU>.!Boot</ARG> are run before <PSIFS> checks for <ARG>PsiFSConverter$</ARG> system variables. This allows stand-alone file converters to be installed simply by copying them to the <ARG>!PsiFS.Converters</ARG> directory.
<P>
It is recommended that the following tasks are performed within the <ARG>!Boot</ARG> file:
<UL>
    <LI>Loading appropriate file type sprites.
    <LI>Setting a system variable <ARG><ARGU>converter</ARGU>$Dir</ARG> to reference the current directory.
    <LI>Setting system variables <ARG>File$Type_<ARGU>xxx</ARGU></ARG> giving the texual version of each supported file type.
    <LI>Setting a system variable <ARG>PsiFSConverter$<ARGU>converter</ARGU></ARG> to register the file format converter with <PSIFS>.
    <LI>Setting a system variable <ARG>Alias$<ARGU>converter</ARGU></ARG> to define an alias for running the converter.
</UL>
<P>
Most of these actions are reasonably standard for <ARG>!Boot</ARG> files anyway. Other initialisation may also be performed, such as dynamically generating a custom <ARG>!PsiFS</ARG> file based on the available resources.
<P>
The <ARG>PsiFSConverter$<ARGU>converter</ARGU></ARG> variable is especially important, as it is used by <PSIFS> to enumerate all of the available file format converters. The <ARG><ARGU>converter</ARGU></ARG> part of the variable name should be the same as the directory name containing the converter, but without any leading pling (<ARG>!</ARG>) character. The value of the variable should be the path of the directory containing the <A HREF=":conv/psifs.html" TITLE="!PsiFS"><ARG>!PsiFS</ARG></A> file that describes the file format converter.
</PARA>

</PAGE>
