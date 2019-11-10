<*
    File        : faq/seral.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Early Archimedes Serial Ports" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Serial Port,Block Driver,Toby Smith,Jonathan Allin">

<HEADING>Why does <PSIFS> not work on early Archimedes?</HEADING>
<PARA>
Early <ARC> computers had faulty serial ports that required cables wired differently to operate correctly. The <ALINK> should work reliably with these machines, but the <3LINK> and simple <RS232> serial cable require an adaptor wired as follows:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH NOWRAP><ARC> connector<BR>(9 pin female)</TH><TH NOWRAP><3LINK> or <RS232> serial cable<BR>(9 pin male)</TH></TR>
    <TR><TD ALIGN=CENTER NOWRAP>2 <NAME>(RxD)</NAME></TD><TD ALIGN=CENTER NOWRAP>2 <NAME>(RxD)</NAME></TD></TR>
    <TR><TD ALIGN=CENTER NOWRAP>3 <NAME>(TxD)</NAME></TD><TD ALIGN=CENTER NOWRAP>3 <NAME>(TxD)</NAME></TD></TR>
    <TR><TD ALIGN=CENTER NOWRAP>1 <NAME>(DCD)</NAME> + 4 <NAME>(DTR)</NAME></TD><TD ALIGN=CENTER NOWRAP>4 <NAME>(DTR)</NAME></TD></TR>
    <TR><TD ALIGN=CENTER NOWRAP>5 <NAME>(GND)</NAME></TD><TD ALIGN=CENTER NOWRAP>5 <NAME>(GND)</NAME></TD></TR>
    <TR><TD ALIGN=CENTER NOWRAP>8 <NAME>(CTS)</NAME></TD><TD ALIGN=CENTER NOWRAP>6 <NAME>(DSR)</NAME></TD></TR>
    <TR><TD ALIGN=CENTER NOWRAP>7 <NAME>(RTS)</NAME></TD><TD ALIGN=CENTER NOWRAP>7 <NAME>(RTS)</NAME></TD></TR>
    <TR><TD ALIGN=CENTER NOWRAP>6 <NAME>(DSR)</NAME></TD><TD ALIGN=CENTER NOWRAP>8 <NAME>(CTS)</NAME></TD></TR>
</TABLE>
<P>
To summarise, pins 6 and 8 should be swapped, pins 1 and 4 should be joined at the <ARC> end, and pins 1 and 9 are not used. The <ARG>Internal</ARG> block driver should then be used rather than <ARG>InternalPC</ARG>.
<P>
This wiring is by Toby Smith and Jonathan Allin, but has not been tested. No resposibility is taken for any damage that may result from use of this information.
</PARA>

</PAGE>
