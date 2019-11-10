<*
    File        : start/conv.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Performing File Format Conversions" PARENT=":start/index.html" KEYWORDS="File Converters">

<HEADING>Performing File Format Conversions</HEADING>
<PARA>
There are two main techniques for performing a file format conversion: dragging files to any <PSIFS> iconbar icon, or intercepting files double-clicked or dragged within the desktop. These methods are explained within the following sections.
<P>
Whatever method is used to start the conversion, the behaviour is similar, with a standard <A HREF=":menu/conv.html" TITLE="File Conversion">file conversion</A> window being used to select the required conversion.
</PARA>

<HEADING>Drag to Iconbar</HEADING>
<PARA>
The simplest way to perform a conversion is to drag the icon for the file to be converted to one of the <PSIFS> iconbar icons. This opens a window from which the required conversion can be selected.
<P>
Select the required conversion from those available via the pop-up menu; only file format converters appropriate to the file being converted are offered as options. Either drag the file icon to a filer window or type a filename into the writable icon and click <B>Convert</B> to perform the selected conversion.
<P>
<PSIFS> remembers the last conversion performed for each file type, and automatically selects the same file format converter when the same type of file is dragged to an iconbar icon.
</PARA>

<HEADING>File Intercept</HEADING>
<PARA>
<PSIFS> can also intercept files double-clicked or dragged within the desktop. Holding down the <ARG>Left&nbsp;Alt</ARG> key while double-clicking on a file, or at the end of a drag, intercepts the file and allows a conversion to be performed.
<P>
Select the required conversion from those available via the pop-up menu, and then click <B>Load</B>, <B>Run</B> or <B>Save</B> (as appropriate) to perform the conversion. As with files dragged to an iconbar icon, only file format converters appropriate to the file being converted are offered as options.
<P>
If file format conversions are frequently required then it may be easier to <A HREF=":start/confg.html" TITLE="Configuring PsiFS">configure</A> automatic intercepts for specific types of file transfer. It is possible to override these settings at any time by pressing <ARG>Left&nbsp;Alt</ARG> to force an intercept and allow a converter to be selected or <ARG>Right&nbsp;Alt</ARG> to prevent an intercept.
<P>
<PSIFS> remembers the last conversion performed for each combination of file type, type of file transfer intercept, and source and destination applications. This allows the appropriate file format converter to be selected for other files matching the same conditions. This is especially useful if automatic conversions are enabled.
</PARA>

</PAGE>
