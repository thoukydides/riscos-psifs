<*
    File        : compt.hsc
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

<PAGE TITLE="Compatibility" PARENT=":index.html" KEYWORDS="Compatibility,Serial Port,RISC OS,Psion,Xemplar,The Serial Port,Philips,Oregon Scientific,EPOC,SIBO,Atomwide,Geofox,Acorn,Ericsson">

<HEADING>Host Computer</HEADING>
<PARA>
<PSIFS> should work on any computer running <RISCOS VER="3.1"> or higher.
<P>
It has been successfully used on:
<UL>
    <LI><B><ARC VER="A410/1"></B> fitted with <RISCOS VER="3.1">.
    <LI><B><ARC VER="A4"></B> fitted with <RISCOS VER="3.1">.
    <LI><B><ARC VER="A5000"></B> fitted with <RISCOS VER="3.11">.
    <LI><B><RPC VER="600/700/J233"></B> fitted with an <ARM TYPE="610/710">/<SA>/<KINETIC> and <RISCOS VER="3.5/3.6/3.7/4.0">.
</UL>
<P>
Early <ARC> computers had faulty serial ports that required cables wired differently to operate correctly. The <3LINK> and simple <RS232> serial cable will not work unmodified with these machines, but the <ALINK> should work without problems.
</PARA>

<HEADING>Remote Computer</HEADING>
<PARA>
<PSIFS> should work with any computer running either <SIBO> (<SERIES VER="3/3a/3c/3mx">, <WORKABOUT>, <SIENA>, <PB VER="I/II"> or <XPB VER="III">) or <EPOC> (<SERIES VER="5/5mx/7">, <NETBOOK>, <REVO>, <REVO PLUS>, <GEOFOXONE>, <MC218>, <OSARIS>, <ILIUM> or <SYNERGY>) connected via a serial link.
<P>
It has been successfully used with:
<UL>                            
    <LI><B><SERIES VER="3"></B> fitted with <ROM VER="1.80(F)">, <RAM KB="256">.
    <LI><B><SERIES VER="3a"></B> fitted with <ROM VER="3.40(F)">, <RAM MB="2">.
    <LI><B><SERIES VER="3c"></B> fitted with <RAM MB="2">.
    <LI><B><SERIES VER="3mx"></B>.
    <LI><B><PB VER="II"></B> fitted with <ROM VER="1.20(F)">.
    <LI><B><SERIES VER="5"></B> release 1 fitted with <ROM VER="1.00(113)/1.01(114)/1.01(145)"> and <RAM MB="8">.
    <LI><B><SERIES VER="5mx"></B> fitted with <ROM VER="1.05(250)">.
    <LI><B><REVO></B>.
    <LI><B><REVO PLUS></B>.
    <LI><B><NETBOOK></B>.
    <LI><B><WORKABOUT></B>.
    <LI><B><GEOFOXONE></B> fitted with <RAM MB="16">.
    <LI><B><MC218></B>.
    <LI><B><OSARIS></B>.
</UL>
<P>
Unfortunately, some <EPOC> based mobile phones use a modified version of <PLP> that is not currently supported by <PSIFS>. This includes the <R380>, <9210> and <9290>.
</PARA>

<HEADING>Serial Link</HEADING>
<PARA>
<PSIFS> should work over any serial link that can be used with <PSIWIN> (<3LINK>, <ALINK> or a simple <RS232> serial cable for more recent machines).
<P>
It has been successfully used with the following cables:
<UL>
    <LI><B><3LINK> cable</B> (does not work unmodified on early <ARC> machines).
    <LI><B><ALINK> cable</B>.
    <LI><B><SERIES VER="5"> serial cable</B> (does not work unmodified on early <ARC> machines).
</UL>
<P>
It has been successfully used with the following serial ports:
<UL>
    <LI><B>Internal</B> serial port.
    <LI><B><SERIALPORT> dual</B> serial port card.
    <LI><B><ATOMWIDE> triple</B> serial port card.
    <LI><B><INTELLIGENT> dual</B> serial port card.
</UL>
<P>
Although it has not been tested, <PSIFS> should also work via an <IRDA> infrared link if both machines are fitted with suitable interfaces and protocol stacks.
</PARA>

</PAGE>
