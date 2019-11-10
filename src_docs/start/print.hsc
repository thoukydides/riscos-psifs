<*
    File        : start/print.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Printing via the Remote Link" PARENT=":start/index.html" KEYWORDS="Printing,Configuration">

<HEADING>Printing via the Remote Link</HEADING>
<PARA>
<PSIFS> supports printing from <EPOC> devices via the remote link. This uses the normal <RISCOS> printer drivers, so any printer with a suitable <RISCOS> driver can be used, even if no driver exists for <EPOC>.
<P>
This is provided as an alternative to the <A HREF=":start/enabl.html" TITLE="Enabling the Printer Mirror">printer mirror</A>, which allows the <SIBO> or <EPOC> printer drivers to be used.
</PARA>

<HEADING>Default Printer</HEADING>
<PARA>
The default printer on <EPOC> devices is configured from the <NAME>Control&nbsp;panel</NAME>. This may be opened from the <NAME>System</NAME> screen by clicking on the <ARG>Control&nbsp;panel</ARG> toolbar icon, selecting the <ARG>Control&nbsp;panel...</ARG> option from the <ARG>Tools</ARG> menu, or by pressing <ARG>Ctrl+S</ARG>. Double-click on the <ARG>Printer</ARG> icon to open the <ARG>Standard&nbsp;printer&nbsp;setup</ARG> dialogue box.
<P>
The <B>Print&nbsp;to</B> option should be set to <ARG>Printer&nbsp;via&nbsp;PC</ARG>. There are no other settings for this printer driver. Click on the <ARG>OK</ARG> icon or press <ARG>Enter</ARG> to confirm the standard printer setup and close the dialogue box. The <NAME>Control&nbsp;panel</NAME> may be closed either by clicking outside its window or by pressing <ARG>Escape</ARG>.
<P>
The default printer should now be configured correctly for use via <PSIFS> and the remote link.
</PARA>

<HEADING>Printing</HEADING>
<PARA>
Ideally, having set the default printer, it would be sufficient to simply select the <ARG>Print...</ARG> option from any application. However, some applications maintain their own individual printer options, either globally or for individual files. Hence, it is necessary to verify the settings each time a file is printed.
<P>
The following instructions describe how to print from <NAME>Word</NAME>; the procedure for other applications should be similar.
<P>
The two machines should be connected, and the remote link <A HREF=":start/conct.html" TITLE="Connecting">connected</A>, if not already done. <NAME>Word</NAME> should be started and a suitable document opened, such as the <ARG>Welcome&nbsp;to&nbsp;Series&nbsp;5</ARG> file that comes with <SERIES VER="5"> machines.
<P>
Open the <ARG>Print</ARG> dialogue box either by selecting <ARG>Print...</ARG> from the <ARG>Printing</ARG> sub-menu of the <ARG>File</ARG> menu, or by pressing <ARG>Ctrl+P</ARG>. Next, open the <ARG>Printer setup</ARG> dialogue box either by clicking on the <ARG>Printer...</ARG> icon or by pressing <ARG>Ctrl+P</ARG>. Set the <B>To</B> option as described above, i.e. to <ARG>Printer&nbsp;via&nbsp;PC</ARG>, and then click on the <ARG>OK</ARG> icon or press <ARG>Enter</ARG> to confirm the standard printer setup and close the dialogue box. The document may now be printed either by clicking on the <ARG>Print</ARG> icon or by pressing <ARG>Enter</ARG>.
<P>
By default this will open a window on the <RISCOS> computer showing the status of the print job, and offering the following options:
<UL>
    <LI><B>Print</B> - Start printing, if a suitable printer driver is loaded.
    <LI><B>Preview</B> - Display a preview of the print job. This also allows the pages to be exported in a form suitable for loading into the <NAME>Draw</NAME> application.
    <LI><B>Cancel</B> - Discard the print job.
</UL>
However, <PSIFS> can be <A HREF=":start/confg.html" TITLE="Configuring PsiFS">configured</A> to automatically start printing or display the preview when a print job starts.
</PARA>

</PAGE>
