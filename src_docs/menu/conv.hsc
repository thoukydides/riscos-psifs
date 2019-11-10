<*
    File        : menu/conv.hsc
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

<PAGE TITLE="Menu Entries" SUBTITLE="File Conversion" PARENT=":menu/index.html" KEYWORDS="File Converters">

<HEADING>File Conversion</HEADING>
<PARA>
A file conversion window is opened whenever a file is dragged to a <PSIFS> iconbar icon, or a file transfer within the desktop is intercepted. The exact appearance and behaviour of the window depends on the source of the file, but the basic behaviour is the same.
</PARA>

<HEADING>Drag to Iconbar</HEADING>
<PARA>
The simplest way to perform a conversion is to drag the icon for the file to be converted to one of the <PSIFS> iconbar icons. This opens a window from which the required conversion can be selected.
<P>
At the top of the window there is an icon for the converted file. This may be dragged to a filer window or another application to perform the conversion and save the result. Next to the file icon there is a writable field allowing the required file name to be typed.
<P>
In the middle of the window there is a field allowing the type of conversion to be selected. <PSIFS> attempts to select the most appropriate conversion based on the type of file and past usage. When a new converter is selected the file icon changes to indicate the type of the resulting file.
<P>
There are four buttons at the bottom of the window:
<UL>
    <LI><B>Info</B> - Open a window displaying information about the selected converter.
    <LI><B>Options</B> - Open a window allowing options to be set for the selected converter.
    <LI><B>Cancel</B> - Close the window without performing a conversion. Clicking with <MOUSE BUTTON=ADJUST> leaves the window open, but resets the options to their original values.
    <LI><B>Convert</B> - Perform a conversion using the selected options. Clicking with <MOUSE BUTTON=ADJUST> performs a conversion without closing the window.
</UL>
</PARA>

<HEADING>File Intercept</HEADING>
<PARA>
<PSIFS> can also intercept files double-clicked or dragged within the desktop. Only the <A HREF=":menu/confg.html" TITLE="Configure...">configured</A> intercepts are performed automatically, but this can be overridden by pressing <ARG>Left&nbsp;Alt</ARG> to force an intercept and allow a converter to be selected or <ARG>Right&nbsp;Alt</ARG> to prevent an intercept.
<P>
A the top of the window there is an icon for the converted file. The original filename and task names are also shown as appropriate.
<P>
In the middle of the window there is a field allowing the type of conversion to be selected. <PSIFS> attempts to select the most appropriate conversion based on the type of file and past usage. When a new converter is selected the file icon changes to indicate the type of the resulting file.
<P>
There are four buttons at the bottom of the window:
<UL>
    <LI><B>Info</B> - Open a window displaying information about the selected converter.
    <LI><B>Options</B> - Open a window allowing options to be set for the selected converter. This feature is not currently supported.
    <LI><B>Cancel</B> - Close the window without performing a conversion. Clicking with <MOUSE BUTTON=ADJUST> leaves the window open, but resets the options to their original values.
    <LI><B>Load</B>, <B>Run</B> or <B>Save</B> - Perform a conversion using the selected options and close the window.
</UL>
</PARA>

<HEADING>Installing Converters</HEADING>
<PARA>
It is not normally necessary to install a file format converter; ensuring that the file format converter is seen by the filer before <PSIFS> is loaded is usually sufficient. Alterntatively, the converter may be copied to the <ARG>!PsiFS.Converters</ARG> directory to ensure that it is always available. However, additional steps may be required for some file format converters; see the documentation supplied with the individual file format converter for details.
<P>
If <PSIFS> is already running then it will need to be restarted before any new converters are available.
</PARA>

</PAGE>
