<*
    File        : start/prn32.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Printing from EPOC Devices" PARENT=":start/enabl.html" KEYWORDS="Printer Mirror,Printing,Configuration,EPOC">

<HEADING>Printer via PC</HEADING>
<PARA>
<EPOC> users as advised to <A HREF=":start/print.html" TITLE="Printing via the Remote Link">print</A> via the <RISCOS> printer drivers rather than use the printer mirror. The following instructions explain how to use the printer mirror.
</PARA>

<HEADING>Default Printer</HEADING>
<PARA>
The default printer on <EPOC> devices is configured from the <NAME>Control&nbsp;panel</NAME>. This may be opened from the <NAME>System</NAME> screen by clicking on the <ARG>Control&nbsp;panel</ARG> toolbar icon, selecting the <ARG>Control&nbsp;panel...</ARG> option from the <ARG>Tools</ARG> menu, or by pressing <ARG>Ctrl+S</ARG>. Double-click on the <ARG>Printer</ARG> icon to open the <ARG>Standard&nbsp;printer&nbsp;setup</ARG> dialogue box.
<P>
There are two important options:
<UL>
    <LI><B>Print to</B> - This should be set to a suitable value for the type of printer actually being used. It must <B>not</B> be set to <ARG>Printer&nbsp;via&nbsp;PC</ARG>, even though output will be redirected via the <RISCOS> computer.
    <LI><B>Print via</B> - This must be set to <ARG>Serial&nbsp;port</ARG>.
</UL>
<P>
Next, click on the <ARG>Settings...</ARG> icon or press <ARG>Ctrl+S</ARG> to open the <ARG>Serial&nbsp;port&nbsp;settings</ARG> dialogue box. The <A HREF=":start/baud.html" TITLE="Baud Rate">baud rate</A> must be set to the same value as <A HREF=":start/confg.html" TITLE="Configuring PsiFS">configured</A> within <PSIFS>. The other settings should be set as shown below:
<CENTER><IMG SRC=":graphics/pf32.gif" ALT="" BORDER=0> <IMG SRC=":graphics/ph32.gif" ALT="" BORDER=0></CENTER>
<P>
Click on the <ARG>OK</ARG> icon or press <ARG>Enter</ARG> to confirm the serial port settings and close the dialogue box, then click on the <ARG>OK</ARG> icon or press <ARG>Enter</ARG> to confirm the standard printer setup and close the dialogue box. The <NAME>Control&nbsp;panel</NAME> may be closed either by clicking outside its window or by pressing <ARG>Escape</ARG>.
<P>
The default printer should now be configured correctly for use via the <PSIFS> printer mirror.
</PARA>

<HEADING>Printing</HEADING>
<PARA>
Ideally, having set the default printer, it would be sufficient to simply select the <ARG>Print...</ARG> option from any application. However, some applications maintain their own individual printer options, either globally or for individual files. Hence, it is necessary to verify the settings each time a file is printed.
<P>
The following instructions describe how to print from <NAME>Word</NAME>; the procedure for other applications should be similar.
<P>
The two machines should be connected, and the printer mirror <A HREF=":start/enabl.html" TITLE="Enabling the Printer Mirror">enabled</A>, if not already done. <NAME>Word</NAME> should be started and a suitable document opened, such as the <ARG>Welcome&nbsp;to&nbsp;Series&nbsp;5</ARG> file that comes with <SERIES VER="5"> machines.
<P>
Open the <ARG>Print</ARG> dialogue box either by selecting <ARG>Print...</ARG> from the <ARG>Printing</ARG> sub-menu of the <ARG>File</ARG> menu, or by pressing <ARG>Ctrl+P</ARG>. Next, open the <ARG>Printer setup</ARG> dialogue box either by clicking on the <ARG>Printer...</ARG> icon or by pressing <ARG>Ctrl+P</ARG>. Set the <B>To</B> and <B>Via</B> options as described above, i.e. to the appropriate printer type and <ARG>Serial&nbsp;port</ARG> respectively.
<P>
Open the <ARG>Serial&nbsp;port&nbsp;settings</ARG> dialogue box either by clicking on the <ARG>Settings...</ARG> icon or by presing <ARG>Ctrl+S</ARG>. Verify that the options are set as described above; they should be correct, but it is worth checking.
<P>
Click on the <ARG>OK</ARG> icon or press <ARG>Enter</ARG> to confirm the serial port settings and close the dialogue box, then click on the <ARG>OK</ARG> icon or press <ARG>Enter</ARG> to confirm the standard printer setup and close the dialogue box. The document may now be printed either by clicking on the <ARG>Print</ARG> icon or by pressing <ARG>Enter</ARG>.
</PARA>

</PAGE>
