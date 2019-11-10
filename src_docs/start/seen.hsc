<*
    File        : start/seen.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Installing File Format Converters" PARENT=":start/index.html" KEYWORDS="File Converters,Tar,SIS File">

<HEADING>Installing File Format Converters</HEADING>
<PARA>
The simplest way to use a third party file format converter with <PSIFS> is to ensure that it has been seen by the filer before <PSIFS> is loaded. Alternatively, the file format converter can be placed within the <ARG>!PsiFS.Converters</ARG> directory to ensure that it is always available.
<P>
See the documentation supplied with individual file format converters for more details or any additional requirements. The <A HREF=":faq/conv.html" TITLE="File Format Converters">file format converters</A> section of the FAQ lists the most common third party tools.
<P>
Once file format converters have been installed, they may be easily used to perform <A HREF=":start/conv.html" TITLE="Performing File Format Conversions">conversions</A>.
</PARA>

<HEADING><ARG>!ChangeFSI</ARG></HEADING>
<PARA>
<PSIFS> includes a simple file format converter that uses <ARG>!ChangeFSI</ARG> to convert common graphics formats (GIF, PCX, BMP, PBM, JPEG, TIFF, and Sprite files) to either Sprite or JPEG files with a selectable number of bits per pixel.
<P>
No installation is required to use this file format converter. If necessary, <PSIFS> automatically searches for a copy of <ARG>!ChangeFSI</ARG>, but if this fails the message <ARG>Please open a directory containing !ChangeFSI and then repeat the operation</ARG> is displayed. In this event simply open a filer window for the appropriate directory.
</PARA>

<HEADING><ARG>!PsiFS</ARG></HEADING>
<PARA>
<PSIFS> includes another simple file format converter that uses the <PSIFS> module to extract the contents of <ARG>tar</ARG> and SIS files. No installation is required to use this file format converter.
<P>
Please note that long filenames will only be preserved if <ARG>!Scrap</ARG> is located on a drive that supports long filenames.
</PARA>

</PAGE>
