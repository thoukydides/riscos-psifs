<*
    File        : start/enabl.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Enabling the Printer Mirror" PARENT=":start/index.html" KEYWORDS="Printer Mirror,Printing,Configuration,Connect,Data Format,Idle">

<HEADING>Printer Mirror</HEADING>
<PARA>
The printer mirror is the simplest feature in <PSIFS>, but unfortunately it is also probably the most difficult to use.
<P>
The most important point to note is that the <PLP> is <B>not</B> used. Hence, the remote link must be disabled from the <NAME>System</NAME> screen of the <SIBO> or <EPOC> device before attempting to use the printer mirror:
<UL>
    <LI>On <SIBO> devices the link is disabled using the <ARG>Remote Link</ARG> or <ARG>Communications</ARG> option from the <ARG>Special</ARG> menu, or by pressing <ARG>Psion+L</ARG> or <ARG>Acorn+L</ARG>.
    <LI>On <EPOC> devices the link is disabled using the <ARG>Remote link...</ARG> option from the <ARG>Tools</ARG> menu, or by pressing <ARG>Ctrl+L</ARG>.
</UL>
<P>
The printer mirror simply copies data received via the serial port to another device or file; no processing of the data is performed. It is provided mainly for use with <SIBO> devices; users of <EPOC> devices are advised to <A HREF=":start/print.html" TITLE="Printing via the Remote Link">print</A> via the <RISCOS> printer drivers instead.
</PARA>

<HEADING>Serial Configuration</HEADING>
<PARA>
Unlike the remote link, it is not possible for <PSIFS> to automatically identify the baud rate to use for the printer mirror. Hence, the <A HREF=":start/baud.html" TITLE="Baud Rate">baud rate</A> must be manually <A HREF=":start/confg.html" TITLE="Configuring PsiFS">configured</A>. This should normally be set to the highest baud rate supported by both machines.
</PARA>

<HEADING>Enabling the Printer Mirror</HEADING>
<PARA>
Click on the <PSIFS> iconbar icon with <MOUSE BUTTON=ADJUST>, or select <MENL TITLE="Printer mirror" HREF=":menu/print.html"> from the iconbar icon menu, to enable the printer mirror. The <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A> will be replaced by the <A HREF=":icon/print.html" TITLE="Printer Mirror Icon">printer mirror icon</A> to indicate that the printer mirror is active:
<CENTER><A HREF=":icon/print.html" TITLE="Printer Mirror Icon"><IMG SRC=":graphics/print.gif" ALT="" BORDER=0></A></CENTER>
<P>
By default, the printer mirror will redirect output to the currently selected <RISCOS> printer. It is possible to select an alternative destination, either within the <A HREF=":start/confg.html" TITLE="Configuring PsiFS">configuration</A> or from the <MENL TITLE="Printer mirror" HREF=":menu/print.html"> sub-menu.
<P>
The printer mirror may be <A HREF=":start/disbl.html" TITLE="Disabling the Printer Mirror">disabled</A> at any time.
</PARA>

<HEADING>Printing</HEADING>
<PARA>
Once the two machines have been conected and printer mirror mode enabled, it is simply a matter of selecting the print option from a <SIBO> or <EPOC> application. However, the output must be sent via the serial port with the correct baud rate, data format and handshaking:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH COLSPAN=2 NOWRAP>Data format</TH><TH COLSPAN=2 NOWRAP>Handshaking</TH></TR>
    <TR VALIGN=TOP><TD NOWRAP>Data bits</TD><TD ALIGN=RIGHT>8</TD><TD NOWRAP>XON/XOFF</TD><TD ALIGN=RIGHT>No</TD></TR>
    <TR VALIGN=TOP><TD NOWRAP>Stop bits</TD><TD ALIGN=RIGHT>1</TD><TD NOWRAP>RTS/CTS</TD><TD ALIGN=RIGHT>Yes</TD></TR>
    <TR VALIGN=TOP><TD NOWRAP>Parity</TD><TD ALIGN=RIGHT>None</TD><TD NOWRAP>DSR/DTR</TD><TD ALIGN=RIGHT>Yes</TD></TR>
    <TR VALIGN=TOP><TD NOWRAP>Ignore parity</TD><TD ALIGN=RIGHT>Yes</TD><TD NOWRAP>DCD</TD><TD ALIGN=RIGHT>No</TD></TR>
</TABLE>
<P>
The details for how to do this are different for <A HREF=":start/prn16.html" TITLE="Printing from SIBO Devices"><SIBO></A> and <A HREF=":start/prn32.html" TITLE="Printing from EPOC Devices"><EPOC></A> devices.
<P>
Remember that the <PSIFS> printer mirror simply copies data received over the serial port to the selected destination; the <RISCOS> printer driver is <B>not</B> used. Hence, it is necessary to select the appropriate printer driver on the <SIBO> or <EPOC> device for the printer being used.
</PARA>

</PAGE>
