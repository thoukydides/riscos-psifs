<*
    File        : faq/port.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Unable To Open Port For Printing" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Printer Mirror,Serial Port,Printing,Unable To Open Port For Printing">

<HEADING>Why is the error <ARG>Cannot open port</ARG> displayed when attempting to print?</HEADING>
<PARA>
If the <SIBO> or <EPOC> computer gives an error such as <ARG>Cannot open port</ARG> when attempting to print via the <PSIFS> printer mirror, then the serial port is probably already being used.
<P>
The most important point to note is that the <PLP> is <B>not</B> used, unless the <ARG>Printer&nbsp;via&nbsp;PC</ARG> option is used with <EPOC> devices. Hence, the remote link must be disabled from the <NAME>System</NAME> screen of the <SIBO> or <EPOC> device before attempting to use the printer mirror:
<UL>
    <LI>On <SIBO> devices the link is disabled using the <ARG>Remote Link</ARG> or <ARG>Communications</ARG> option from the <ARG>Special</ARG> menu, or by pressing <ARG>Psion+L</ARG> or <ARG>Acorn+L</ARG>.
    <LI>On <EPOC> devices the link is disabled using the <ARG>Remote link...</ARG> option from the <ARG>Tools</ARG> menu, or by pressing <ARG>Ctrl+L</ARG>.
</UL>
<P>
If another application, such as <ARG>Comms</ARG>, is using the serial port then it must also be disabled.
</PARA>

</PAGE>
