<*
    File        : conv/index.hsc
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

<PAGE TITLE="File Format Converters" PARENT=":index.html" KEYWORDS="File Converters,File Formats">

<HEADING>File Format Converters</HEADING>
<PARA>
<PSIFS> provides an interface to allow the integration of third party file format converters with a unified user interface. Both stand-alone and embedded file format converters are supported.
<P>
The documentation accompanying file format converters should describe any necessary installation procedure. However, in general, it will be sufficient for the file format converter to be either seen by the filer or copied into the <ARG>!PsiFS.Converters</ARG> directory.
</PARA>

<HEADING>Programmer's Interface</HEADING>
<PARA>
When <PSIFS> is loaded it first boots any applications contained within the <ARG>!PsiFS.Converters</ARG> directory. <SWIL SWI="Wimp_StartTask"> is used to run any <A HREF=":conv/boot.html" TITLE="!Boot"><ARG>!Boot</ARG></A> files found within sub-directories.
<P>
Next, all system variables starting with <ARG>PsiFSConverter$</ARG> are enumerated, and any files called <A HREF=":conv/psifs.html" TITLE="!PsiFS"><ARG>!PsiFS</ARG></A> within the referenced directories are parsed to build a list of all available file format converters and their characteristics. If successful any toolbox resource file called <A HREF=":conv/res.html" TITLE="!PsiFSRes"><ARG>!PsiFSRes</ARG></A> within the same directories are also loaded.
<P>
Two simple file format converters have been included as examples. The first is based around <A HREF=":conv/exfsi.html" TITLE="!ChangeFSI Example"><ARG>!ChangeFSI</ARG></A>, and the second is a simple wrapper for commands provided by the <A HREF=":conv/expsi.html" TITLE="PsiFS Module Example"><PSIFS> module</A>.
</PARA>

</PAGE>
