<*
    File        : icon/drive.hsc
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

<PAGE TITLE="Iconbar Icons" SUBTITLE="SIBO or EPOC drive" PARENT=":icon/index.html" KEYWORDS="Icon,Iconbar,Status,Disconnect,Remote Link,Drive,Disc Name,Disk Name,Help,Backup,Free Space,Menu,Quit,Filer Window,Info,Connect,File Converters">

<HEADING><SIBO> or <EPOC> Drive Iconbar Icon</HEADING>
<PARA>
<CENTER><IMG SRC=":graphics/e16.gif" ALT=""> <IMG SRC=":graphics/e32.gif" ALT=""></CENTER>
<P>
A <SIBO> or <EPOC> drive iconbar icon is displayed for each accessible remote drive when <PSIFS> is connected to a remote <SIBO> or <EPOC> device using the <PLP>. The name of each drive is shown below the corresponding icon. If the connection is lost then the drive icons are replaced by the <A HREF=":icon/act.html" TITLE="Remote Link Icon">remote link icon</A>.
<P>
Dragging a file to the iconbar icon will open a window allowing a <A HREF=":menu/conv.html" TITLE="File Conversion">file conversion</A> to be performed.
</PARA>

<HEADING>Select</HEADING>
<PARA>
Clicking on the iconbar icon with <MOUSE BUTTON=SELECT> opens a filer window for the selected drive. 
</PARA>

<HEADING>Menu</HEADING>
<PARA>
Clicking on the iconbar icon with <MOUSE BUTTON=MENU> displays the following menu:
<MEN>
    <MENI TITLE="Info" HREF=":menu/info.html">
    <MENI TITLE="Help" HREF=":menu/help.html">
    <MENI TITLE="Name disc" HREF=":menu/name.html">
    <MENI TITLE="Backup..." HREF=":menu/backu.html">
    <MENI TITLE="Free..." HREF=":menu/free.html">
    <MENI TITLE="Disconnect" HREF=":menu/disco.html">
    <MENI TITLE="Quit" HREF=":menu/quit.html">
</MEN>
</PARA>

<HEADING>Adjust</HEADING>
<PARA>
Clicking on the iconbar icon with <MOUSE BUTTON=ADJUST> disables the remote link. The <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A> is displayed to indicate the new state.
<P>
This has the same effect as selecting <MENL TITLE="Disconnect" HREF=":menu/disco.html"> from the iconbar icon menu.
</PARA>

</PAGE>
