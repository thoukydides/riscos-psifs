<*
    File        : start/prn16.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Printing from SIBO Devices" PARENT=":start/enabl.html" KEYWORDS="Printer Mirror,Printing,Configuration,SIBO">

<HEADING>Printer Configuration</HEADING>
<PARA>
The default printer on <SIBO> devices is configured from the <NAME>System</NAME> screen by selecting the <ARG>Printer</ARG> option from the <ARG>Control</ARG> menu, or by pressing <ARG>Psion+Y</ARG>. This opens the <ARG>Printer&nbsp;configuration</ARG> window:
<CENTER><IMG SRC=":graphics/pc16.gif" ALT="" BORDER=0></CENTER>
<P>
There are three important options:
<UL>
    <LI><B>Printer device</B> - This must be set to <ARG>Serial</ARG>.
    <LI><B>Serial characteristics</B> - The settings must be set as shown below:
        <CENTER><IMG SRC=":graphics/ps16.gif" ALT="" BORDER=0></CENTER>
    <LI><B>Serial handshaking</B> - The settings must be set as shown below:
        <CENTER><IMG SRC=":graphics/ph16.gif" ALT="" BORDER=0></CENTER>
</UL>
<P>
Press <ARG>Enter</ARG> to confirm all selections.
<P>
The default printer should now be configured correctly for use via the <PSIFS> printer mirror.
</PARA>

<HEADING>Printing</HEADING>
<PARA>
Ideally, having set the printer options, it would be sufficient to simply select the <ARG>Print</ARG> option from any application. However, some applications maintain their own individual printer options, either globally or for individual files. Hence, it is necessary to verify the settings each time a file is printed.
<P>
The following instructions describe how to print from <NAME>Word</NAME>; the procedure for other applications should be similar.
<P>
The two machines should be connected, and the printer mirror <A HREF=":start/enabl.html" TITLE="Enabling the Printer Mirror">enabled</A>, if not already done. <NAME>Word</NAME> should be started and a suitable document opened.
<P>
Open the <ARG>Printer&nbsp;configuration</ARG> dialogue box either by selecting <ARG>Print&nbsp;setup</ARG> from the <ARG>Special</ARG> menu, or by pressing <ARG>Psion+Y</ARG>. Ensure that <ARG>Serial</ARG> is selected, and verify that the <ARG>Serial&nbsp;characteristics</ARG> and <ARG>Serial&nbsp;handshaking</ARG> are set as described above. Press <ARG>Enter</ARG> to confirm the selections.
<P>
Open the <ARG>Print</ARG> dialogue box either by selecting <ARG>Print</ARG> from the <ARG>Special</ARG> menu, or by pressing <ARG>Psion+P</ARG>. Adjust any options as required, and then press <ARG>Enter</ARG> to print the document.
</PARA>

</PAGE>
