<*
    File        : icon/print.hsc
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

<PAGE TITLE="Iconbar Icons" SUBTITLE="Printer Mirror" PARENT=":icon/index.html" KEYWORDS="Icon,Iconbar,Printer Mirror,Printing,Menu,Disconnect,Status,Help,Quit,Info,Connect,File Converters">

<HEADING>Printer Mirror Iconbar Icon</HEADING>
<PARA>
<CENTER><IMG SRC=":graphics/print.gif" ALT=""></CENTER>
<P>
The printer mirror iconbar icon is displayed when <PSIFS> is copying all characters received over the serial link to the printer. This allows a <SIBO> or <EPOC> device to print to a parallel printer without needing a special cable.
<P>
The text under the icon changes to indicate the number of bytes received over the serial link. This can be useful to confirm that the printer mirror is operating as expected.
<P>
More information is available on <A HREF=":start/enabl.html" TITLE="Enabling the Printer Mirror">enabling</A> and <A HREF=":start/disbl.html" TITLE="Disabling the Printer Mirror">disabling</A> the printer mirror. This includes details specific to printing from <A HREF=":start/prn16.html" TITLE="Printing from SIBO Devices"><SIBO></A> and <A HREF=":start/prn32.html" TITLE="Printing from EPOC Devices"><EPOC></A> devices.
<P>
Dragging a file to the iconbar icon will open a window allowing a <A HREF=":menu/conv.html" TITLE="File Conversion">file conversion</A> to be performed.
</PARA>

<HEADING>Select</HEADING>
<PARA>
Clicking on the iconbar icon with <MOUSE BUTTON=SELECT> has no effect.
<P>
This has the same effect as selecting <MENL TITLE="Disconnect" HREF=":menu/disco.html"> from the iconbar icon menu.
</PARA>

<HEADING>Menu</HEADING>
<PARA>
Clicking on the iconbar icon with <MOUSE BUTTON=MENU> displays the following menu:
<MEN>
    <MENI TITLE="Info" HREF=":menu/info.html">
    <MENI TITLE="Help" HREF=":menu/help.html">
    <MENI TITLE="Disconnect" HREF=":menu/disco.html">
    <MENI TITLE="Quit" HREF=":menu/quit.html">
</MEN>
</PARA>

<HEADING>Adjust</HEADING>
<PARA>
Clicking on the iconbar icon with <MOUSE BUTTON=ADJUST> disables the printer mirror. The <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A> is displayed to indicate the new state.
<P>
This has the same effect as selecting <MENL TITLE="Disconnect" HREF=":menu/disco.html"> from the iconbar icon menu.
</PARA>

</PAGE>
