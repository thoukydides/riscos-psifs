<*
    File        : faq/baud.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Baud Rate Too High" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Serial Port,Block Driver,Remote Link,Baud Rate,Configuration">

<HEADING>Why is the remote link very slow or unreliable?</HEADING>
<PARA>
The most likely cause of the link being slow or unreliable is that the baud rate has been set too high. The PsiFS link statistics window, opened by selecting <MENL TITLE="Link..." HREF=":menu/info.html#link"> from the <MENL TITLE="Info" HREF=":menu/info.html"> menu, or <CMDL CMD="PsiFSStatus" HREF=":command/stats.html"> command may be used to monitor the error rate. If the baud rate is set correctly <PSIFS> should be able to maintain a connection without any retries or invalid protocol frames.
<P>
If problems are experienced, then try <A HREF=":start/confg.html" TITLE="Configuration">configuring</A> both <PSIFS> and the <SIBO> or <EPOC> device to use the <A HREF=":start/baud.html" TITLE="Baud Rate">highest recommended baud rate</A>. The standard serial port of older <RISCOS> computers is only intended to operate at 19200&nbsp;baud or lower. Higher speeds will probably be offered as options, but they may not operate satisfactorily.
<P>
Care must be taken when trying baud rates above the recommended maximum; the computer may become unresponsive, or even appear to hang. Ensure that all important documents are saved first.
</PARA>

</PAGE>
