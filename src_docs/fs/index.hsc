<*
    File        : fs/index.hsc
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

<PAGE TITLE="File System" PARENT=":index.html" KEYWORDS="File System,File Name,File Type,Attributes,Date Stamp,Disc Name,Disk Name,Virtual Drive,Character Mapping">

<HEADING>Paths</HEADING>
<PARA>
All paths under <PSIFS> are of the form:
<CENTER><ARG>PsiFS::<ARGU>discname</ARGU>.$.<ARGU>file/directory</ARGU></ARG></CENTER>
where <ARG><ARGU>discname</ARGU></ARG> is either a disc name or a single character drive letter (<ARG>A</ARG> to <ARG>Z</ARG> or <ARG>@</ARG>), and <ARG><ARGU>file/directory</ARGU></ARG> is the path to the object being accessed.
<P>
The special drive letter <ARG>@</ARG> refers to a virtual drive with all of the physical drives represented as subdirectories. The special disc name <ARG>AllDrives</ARG> also refers to this virtual drive.
</PARA>

<HEADING>Mappings</HEADING>
<PARA>
<RISCOS> and <SIBO> or <EPOC> use different formats for <A HREF=":fs/chars.html" TITLE="Character Mappings">filenames</A>, <A HREF=":fs/type.html" TITLE="File Types">file types</A>, <A HREF=":fs/date.html" TITLE="Dates and Times">dates</A>, and <A HREF=":fs/attr.html" TITLE="Atributes">attributes</A>. <PSIFS> attempts to perform sensible mappings for all of these values when transferring files.
<P>
Copying a file from a <SIBO> or <EPOC> device to <RISCOS> and back again should not result in any loss of information, providing that the intermediate <RISCOS> file system supports long filenames and all necessary attributes. This ensures that it is possible to use <RISCOS> backup tools to preserve the contents of <SIBO> or <EPOC> drives.
</PARA>

</PAGE>
