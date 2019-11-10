<*
    File        : menu/print.hsc
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

<PAGE TITLE="Menu Entries" SUBTITLE="Printer Mirror" PARENT=":menu/index.html" KEYWORDS="Menu,Printer Mirror,Printing,Data Format,Connect,Idle">

<HEADING>Printer mirror</HEADING>
<PARA>
Selecting the <MENL TITLE="Printer mirror"> menu entry enables the printer mirror with received data redirected to the <A HREF=":menu/confg.html" TITLE="Configure...">configured</A> default destination. The <A HREF=":icon/print.html" TITLE="Printer Mirror Icon">printer mirror icon</A> is displayed to indicate the new state.
<P>
This has the same effect as clicking on the iconbar icon with <MOUSE BUTTON=ADJUST>.
<P>
Moving the mouse pointer to the right over the <MENL TITLE="Printer mirror"> sub-menu icon displays the following menu:
<MEN TITLE="Printer mirror">
    <MENI TITLE="Parallel printer" HREF="#parallel">
    <MENI TITLE="Serial printer" HREF="#serial">
    <MENI TITLE="----------------">
    <MENI TITLE="Print to file" HREF="#file">
</MEN>
<P>
More information is available on <A HREF=":start/enabl.html" TITLE="Enabling the Printer Mirror">enabling</A> and <A HREF=":start/disbl.html" TITLE="Disabling the Printer Mirror">disabling</A> the printer mirror. This includes details specific to printing from <A HREF=":start/prn16.html" TITLE="Printing from SIBO Devices"><SIBO></A> and <A HREF=":start/prn32.html" TITLE="Printing from EPOC Devices"><EPOC></A> devices.
</PARA>

<HEADING NAME="settings">Serial settings</HEADING>
<PARA>
The printer mirror uses the <A HREF=":menu/confg.html" TITLE="Configure...">configured</A> serial block driver and baud rate. The <SIBO> or <EPOC> device must be configured to use the same baud rate, and the following data format and handshaking:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH COLSPAN=2 NOWRAP>Data format</TH><TH COLSPAN=2 NOWRAP>Handshaking</TH></TR>
    <TR VALIGN=TOP><TD NOWRAP>Data bits</TD><TD ALIGN=RIGHT>8</TD><TD NOWRAP>XON/XOFF</TD><TD ALIGN=RIGHT>No</TD></TR>
    <TR VALIGN=TOP><TD NOWRAP>Stop bits</TD><TD ALIGN=RIGHT>1</TD><TD NOWRAP>RTS/CTS</TD><TD ALIGN=RIGHT>Yes</TD></TR>
    <TR VALIGN=TOP><TD NOWRAP>Parity</TD><TD ALIGN=RIGHT>None</TD><TD NOWRAP>DSR/DTR</TD><TD ALIGN=RIGHT>Yes</TD></TR>
    <TR VALIGN=TOP><TD NOWRAP>Ignore parity</TD><TD ALIGN=RIGHT>Yes</TD><TD NOWRAP>DCD</TD><TD ALIGN=RIGHT>No</TD></TR>
</TABLE>
<P>
Note that the remote link must be switched off on the <SIBO> or <EPOC> device before attempting to use the printer mirror feature. Failure to do this will result in garbage being printed.
</PARA>

<HEADING NAME="parallel">Printer mirror &gt; Parallel printer</HEADING>
<PARA>
Selecting the <MENL TITLE="Parallel printer"> menu entry, from the <MENL TITLE="Printer mirror"> menu, enables the printer mirror with received data redirected to the parallel printer port. The <A HREF=":icon/print.html" TITLE="Printer Mirror Icon">printer mirror icon</A> is displayed to indicate the new state.
<P>
This is equivalent to using the <MENL TITLE="Print to file" HREF="#file"> option with <ARG>printer#parallel:</ARG> selected as the filename.
</PARA>

<HEADING NAME="serial">Printer mirror &gt; Serial printer</HEADING>
<PARA>
Selecting the <MENL TITLE="Serial printer"> menu entry, from the <MENL TITLE="Printer mirror"> menu, enables the printer mirror with received data redirected to the internal serial port. The <A HREF=":icon/print.html" TITLE="Printer Mirror Icon">printer mirror icon</A> is displayed to indicate the new state.
<P>
This is equivalent to using the <MENL TITLE="Print to file" HREF="#file"> option with <ARG>printer#serial:</ARG> selected as the filename. The current serial port settings are used; not those specified in the <PSIFS> configuration. This option can only be used if <PSIFS> is configured to use a different serial port for connecting to the <SIBO> or <EPOC> device.
</PARA>

<HEADING NAME="file">Printer mirror &gt; Print to file</HEADING>
<PARA>
Selecting the <MENL TITLE="Print to file"> menu entry or moving the mouse pointer to the right, from the <MENL TITLE="Printer mirror"> menu, opens a <ARG>Print&nbsp;to</ARG> window allowing a filename to be specified. The printer mirror will be enabled with received data redirected to the specified file or device. The <A HREF=":icon/print.html" TITLE="Printer Mirror Icon">printer mirror icon</A> is displayed to indicate the new state.
</PARA>

</PAGE>
