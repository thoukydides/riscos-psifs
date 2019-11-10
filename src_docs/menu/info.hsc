<*
    File        : menu/info.hsc
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

<PAGE TITLE="Menu Entries" SUBTITLE="Info" PARENT=":menu/index.html" KEYWORDS="Menu,Info,Version,Help,Batteries,Power status">

<HEADING>Info</HEADING>
<PARA>
Moving the mouse pointer to the right over the <MENL TITLE="Info"> sub-menu icon displays either information about this version of <PSIFS> or the following menu:
<MEN TITLE="Info">
    <MENI TITLE="PsiFS" HREF="#psifs">
    <MENI TITLE="----------">
    <MENI TITLE="Machine..." HREF="#machine">
    <MENI TITLE="Owner..." HREF="#owner">
    <MENI TITLE="Power..." HREF="#power">
    <MENI TITLE="----------">
    <MENI TITLE="Link..." HREF="#link">
    <MENI TITLE="Clipboard..." HREF="#clipboard">
</MEN>
</PARA>

<HEADING NAME="psifs">Info &gt; PsiFS</HEADING>
<PARA>
Selecting the <MENL TITLE="PsiFS"> menu entry, from the <MENL TITLE="Info"> menu, displays information about this version of <PSIFS>. The most important information is the version number shown at the bottom of the window; this should be quoted in all correspondence.
</PARA>

<HEADING NAME="machine">Info &gt; Machine...</HEADING>
<PARA>
Selecting the <MENL TITLE="Machine..."> menu entry, from the <MENL TITLE="Info"> menu, opens a window displaying information about the connected device. Some details are only available with <EPOC> devices; the <SIBO> version of the <PLP> does not provide access to this information.
</PARA>

<HEADING NAME="owner">Info &gt; Owner...</HEADING>
<PARA>
Selecting the <MENL TITLE="Owner..."> menu entry, from the <MENL TITLE="Info"> menu, opens a window displaying information about the owner of the connected device.
</PARA>

<HEADING NAME="power">Info &gt; Power...</HEADING>
<PARA>
Selecting the <MENL TITLE="Power..."> menu entry, from the <MENL TITLE="Info"> menu, opens a window displaying the battery and external power status. This is only available with <EPOC> devices; the <SIBO> version of the <PLP> does not provide access to this information.
</PARA>

<HEADING NAME="link">Info &gt; Link...</HEADING>
<PARA>
Selecting the <MENL TITLE="Link..."> menu entry, from the <MENL TITLE="Info"> menu, opens a window displaying statistics about the remote link. The top section of the window shows the current transfer rate in both graphical and textual form, and lower section of the window shows the total number of valid and invalid packets that have been sent and received.
</PARA>

<HEADING NAME="clipboard">Info &gt; Clipboard...</HEADING>
<PARA>
Selecting the <MENL TITLE="Clipboard..."> menu entry, from the <MENL TITLE="Info"> menu, opens a window displaying the shared clipboard status. This is only available with <EPOC> devices; the <SIBO> version of the <PLP> does not provide access to the clipboard.
<P>
Files can be copied to the <EPOC> clipboard by dragging their icons from file windows and dropping them within this window. Similarly, the current <EPOC> clipboard can be exported by clicking on the <ARG>Save</ARG> button.
<P>
Clipboard data is automatically converted to and from <EPOC> format using any file format converters that have been installed.
</PARA>

</PAGE>
