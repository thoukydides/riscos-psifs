<*
    File        : start/baud.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Baud Rate" PARENT=":start/index.html" KEYWORDS="Baud Rate,Remote Link,Printer Mirror,Auto Baud Rate,Configuration">

<HEADING>Baud Rate</HEADING>
<PARA>
The <RISCOS> and <SIBO> or <EPOC> machines both need to be <A HREF=":start/confg.html" TITLE="Configuring PsiFS">configured</A> to use the same baud rate. This should normally be the highest speed supported by both devices:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR>
        <TH COLSPAN=2>Machine type</TH>
        <TH><ARC VER="A4">,<BR><ARC VER="A3<I>xx</I>">,<BR><ARC VER="A4<I>x</I>0">,<BR><ARC VER="A4<I>x</I>0/1">,<BR><ARC VER="A5<I>x</I>0">,<BR><ARC VER="A30<I>x</I>0">,<BR><ARC VER="A4000">,<BR><ARC VER="A5000"><BR>or&nbsp;<ARC VER="R<I>xxx</I>"></TH>
        <TH><RPC>,<BR><ARC VER="A7000"><BR>or&nbsp;<ARC VER="A7000+"></TH>
    </TR>
    <TR><TH ROWSPAN=4><SIBO></TH><TH NOWRAP><SERIES VER="3"></TH><TD ALIGN=RIGHT>9600&nbsp;baud</TD><TD ALIGN=RIGHT>9600&nbsp;baud</TD></TR>
    <TR><TH NOWRAP><SERIES VER="3a">,<BR><SIENA>,<BR><WORKABOUT><BR>or&nbsp;<PB VER="I"></TH><TD ROWSPAN=3 ALIGN=RIGHT>19200&nbsp;baud</TD><TD ALIGN=RIGHT>19200&nbsp;baud</TD></TR>
    <TR><TH NOWRAP><SERIES VER="3c">,<BR><PB VER="II"><BR>or&nbsp;<XPB VER="III"></TH><TD ALIGN=RIGHT>57600&nbsp;baud</TD></TR>
    <TR><TH NOWRAP><SERIES VER="3mx"></TH><TD ALIGN=RIGHT>115200&nbsp;baud</TD></TR>
    <TR><TH><EPOC></TH><TH NOWRAP><SERIES VER="5">,<BR><SERIES VER="5mx">,<BR><SERIES VER="7">,<BR><NETBOOK>,<BR><REVO>,<BR><REVO PLUS>,<BR><GEOFOXONE>,<BR><MC218>,<BR><R380>,<BR><OSARIS>,<BR><ILIUM><BR> or <SYNERGY></TH><TD ALIGN=RIGHT>19200&nbsp;baud</TD><TD ALIGN=RIGHT>115200&nbsp;baud</TD></TR>
</TABLE>
<P>
If a high speed serial interface is fitted to the <RISCOS> computer, then the higher baud rates listed in the right-hand column will be available with older machines.
<P>
<PSIFS> may be configured to automatically identify the required baud rate for the remote link. With this option it is not necessary to explicitly specify the baud rate unless the printer mirror is being used.
</PARA>

</PAGE>
