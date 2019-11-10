<*
    File        : macros.hsc
    Date        : 19-Sep-02
    Author      : © A.Thoukydides, 1998-2002, 2019
    Description : Main macro definitions for the PsiFS documentation.
 
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

<* Include details that may be changed for different builds *>
<$include FILE="brand.hsc">

<* Author details *>
<$define AUTHOR:string/CONST="Alexander Thoukydides">
<$define EMAIL:string/CONST="psifs@thouky.co.uk">

<* The current date *>
<$let HSC.FORMAT.TIME="%d-%b-%y">
<$define DATE:string/CONST=(GetTime())>
<$let HSC.FORMAT.TIME="%Y">
<$define COPYRIGHT:string/CONST="1998, 1999, 2000, 2001, 2002">

<* Special files *>
<$define FILEREL:string/CONST="Release">

<* The frameword for all pages *>
<$macro PAGE/CLOSE/ONLYONCE TITLE:string SUBTITLE:string PARENT:uri/STRIPEXT NORULE:bool KEYWORDS:string CONTENTS:bool INDEX:bool>
    <$define EXTTITLE:string="PsiFS Documentation">
    <$if COND=(set TITLE)>
        <$let EXTTITLE=(EXTTITLE + " - " + TITLE)>
    </$if>
    <$if COND=(set SUBTITLE)>
        <$let EXTTITLE=(EXTTITLE + " [" + SUBTITLE + "]")>
    </$if>
    <$define XREF:string=("xref -url " + HSC.DOCUMENT.URI + " -title ")>
    <$if COND=(set TITLE)>
        <$let XREF=(XREF + TITLE)>
        <$if COND=(set SUBTITLE)>
            <$let XREF=(XREF + ": " + SUBTITLE)>
        </$if>
    <$else>
        <$let XREF=(XREF + "PsiFS Documentation")>
    </$if>
    <$if COND=(set PARENT)>
        <$let XREF=(XREF + " -parent " + PARENT)>
    </$if>
    <$if COND=(set KEYWORDS)>
        <$let XREF=(XREF + " -keywords " + KEYWORDS)>
    </$if>
    <$if COND=(set CONTENTS)>
        <$depend ON="database" FILE>
        <$let XREF=(XREF + " -contents")>
    </$if>
    <$if COND=(set INDEX)>
        <$depend ON="database" FILE>
        <$let XREF=(XREF + " -index")>
    </$if>
    <$exec COMMAND=(XREF + " { > tmp }") TEMPORARY INCLUDE FILE="tmp">
    <!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2//EN">
    <("<!-- File        : " + HSC.DOCUMENT.URI + " -->")>
    <("<!-- Date        : " + DATE + " -->")>
    <("<!-- Description : Part of the PsiFS documentation. -->")>
    <("<!-- -->")>
    <("<!-- Copyright Â© " + COPYRIGHT + " " + AUTHOR +" -->")>
    <("<!-- -->")>
    <("<!-- This program is free software; you can redistribute it and/or -->")>
    <("<!-- modify it under the terms of the GNU General Public License -->")>
    <("<!-- as published by the Free Software Foundation; either version 2 -->")>
    <("<!-- of the License, or (at your option) any later version. -->")>
    <("<!-- -->")>
    <("<!-- This program is distributed in the hope that it will be useful, -->")>
    <("<!-- but WITHOUT ANY WARRANTY; without even the implied warranty of -->")>
    <("<!-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the -->")>
    <("<!-- GNU General Public License for more details. -->")>
    <("<!-- -->")>
    <("<!-- You should have received a copy of the GNU General Public License -->")>
    <("<!-- along with this program; if not, write to the Free Software -->")>
    <("<!-- Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA -->")>
    <HTML>
    <HEAD>
        <TITLE><(EXTTITLE)></TITLE>
        <LINK REV=MADE HREF=("mailto:" + EMAIL)>
        <$if COND=(set PARENT)>
            <LINK REL=PARENT HREF=(PARENT)>
        </$if>
        <META NAME=DESCRIPTION CONTENT=(EXTTITLE)>
    </HEAD>
        <BODY TEXT=(COLORFG) BGCOLOR=(COLORBG) LINK=(COLORFGL) VLINK=(COLORFGVL) ALINK=(COLORFGAL)>
            <CENTER>
                <H1><$if COND=(set TITLE)><PSIFS><$stripws>: <(TITLE)><$else><IMG SRC=":graphics/psifsl.gif" ALT="PsiFS"></$if></H1>
                <$if COND=(set SUBTITLE)><B><(SUBTITLE)></B></$if>
            </CENTER>
            <$if COND=(not set NORULE)><HR></$if>
            <$content>
            <$if COND=(not set NORULE)><HR></$if>
            <TABLE WIDTH="100%">
                <TR VALIGN=TOP>
                    <TD>
                        <$if COND=(set PARENT)>
                            [<A HREF=":index.html" TITLE="Contents">Contents</A>]
                            <$if COND=(not (PARENT = ":index.html"))>
                                [<A HREF=(PARENT) TITLE="Up">Up</A>]
                            </$if>
                        </$if>
                    </TD>
                    <TD ALIGN=RIGHT>
                        <FONT SIZE=-1><I><COPYRIGHTLINK></I></FONT>
                    </TD>
                </TR>
            </TABLE>
        </BODY>
    </HTML>
</$macro>

<* A heading *>
<$macro HEADING/CLOSE NAME:string><H2><$if COND=(set NAME)><A NAME=(NAME)></$if><FONT COLOR=(COLORFGHI)><$content></FONT><$if COND=(set NAME)></A></$if></H2></$macro>

<* A body region *>
<$macro PARA/CLOSE ALIGN:enum("left|right|center|justify")>
    <TABLE WIDTH="100%"><TR><TD WIDTH=30></TD><TD ALIGN?=ALIGN><$stripws><$content><$stripws></TD></TR></TABLE>
</$macro>

<* A contents menu *>
<$macro CONTENTS/CLOSE TITLE:string>
    <$if COND=(set TITLE)>
        <HEADING><(TITLE)></HEADING>
    </$if>
    <CENTER>
        <$content>
    </CENTER>
</$macro>
<$macro CONTENTSITEM/CLOSE/MBI="CONTENTS" HREF:uri/STRIPEXT/REQUIRED TITLE:string>
    <FONT SIZE="+1"><A HREF=(HREF) TITLE=(TITLE)><$content></A></FONT><BR>
</$macro>

<* A version history *>
<$macro REL/CLOSE/NAW="REL">
    <TABLE BORDER=1 WIDTH="100%">
        <TR><TH NOWRAP>Version</TH><TH NOWRAP>Date</TH><TH>Description</TH></TR>
        <$content>
    </TABLE>
</$macro>
<$macro RELIF/CLOSE/MBI="RELI"/NAW="RELIF" SHADE:bool><$if COND=(set SHADE)><FONT COLOR=(COLORFGLO)><$content></FONT><$else><$content></$if></$macro>
<$macro RELI/CLOSE/MBI="REL"/NAW="RELI" VER:string/REQUIRED DATE:string/REQUIRED RELEASE:bool>
    <$define SHADE:bool=(VER > "1")>
    <$define BGCOLOR:color>
    <$if COND=(set RELEASE)>
        <$let BGCOLOR=(COLORBGHI)>
        <$export FILE=(FILEREL) DATA=("<$define RELVER:string/CONST='" + VER + "'><$define RELDATE:string/CONST='" + DATE + "'>")>
    </$if>
    <TR VALIGN=TOP>
        <TD NOWRAP WIDTH=70 ALIGN=CENTER BGCOLOR?=BGCOLOR><RELIF SHADE=(SHADE)><(VER)></RELIF></TD>
        <TD NOWRAP WIDTH=70 ALIGN=CENTER BGCOLOR?=BGCOLOR><RELIF SHADE=(SHADE)><(DATE)></RELIF></TD>
        <TD BGCOLOR?=BGCOLOR><RELIF SHADE=(SHADE)><$content></RELIF></TD>
    <TR>
</$macro>

<* PRM style descriptions *>
<$macro PRM/CLOSE/NAW="PRM"><TABLE WIDTH="100%"><$content></TABLE></$macro>
<$macro PRMT/CLOSE/MBI="PRM"/NAW="PRMT|PRMD"><TR><TD COLSPAN=4 ALIGN=RIGHT><FONT COLOR=(COLORFGHI)><$content></FONT></TD></TR></$macro>
<$macro PRMD/CLOSE/MBI="PRM"/NAW="PRMT|PRMD" HEAD:string SUB:string INDENT:bool MORE:bool><TR VALIGN=TOP><$if COND=((set HEAD) AND (not set MORE))><TD COLSPAN=4><BR></TD></TR><TR VALIGN=TOP><TD NOWRAP><FONT COLOR=(COLORFGHI)><B><(HEAD)>:</B></FONT></TD><$else><TD></TD></$if><$if COND=(set INDENT)><TD></TD><$if COND=(set SUB)><TD><(SUB)></TD><TD><$content></TD><$else><TD COLSPAN=2><$content></TD></$if><$else><$if COND=(set SUB)><TD><(SUB)></TD><TD COLSPAN=2><$content></TD><$else><TD COLSPAN=3><$content></TD></$if></$if></TR></$macro>

<* Command descriptions *>
<$macro ARG/CLOSE/NAW="ARG"><TT><$content></TT></$macro>
<$macro ARGU/CLOSE/MBI="ARG"/NAW="ARGU"><I><$content></I></$macro>
<*<$macro ARGU/CLOSE/MBI="ARG"/NAW="ARGU">&lt;</TT><I><$content></I><TT>&gt;</$macro>*>
<$define CMDT:string>
<$macro CMD/CLOSE/NAW="CMD" CMD:string/REQUIRED DESC:string/REQUIRED><$let CMDT=("*" + CMD)><PRM><PRMT><ARG><B><(CMDT)></B></ARG></PRMT><PRMD><(DESC)></PRMD><$content></PRM></$macro>
<$macro CMDN/MBI="CMD"><ARG><(CMDT)></ARG></$macro>
<$macro CMDS/CLOSE/MBI="CMD"/NAW="CMDS|CMDP|CMDU|CMDE|CMDR"><PRMD HEAD="Syntax"><ARG><(CMDT)> <$content></ARG></PRMD></$macro>
<$macro CMDP/CLOSE/MBI="CMD"/NAW="CMDS|CMDP|CMDU|CMDE|CMDR" PARAM:string MORE:bool><$if COND=(set PARAM)><$let PARAM=("<ARG>" + PARAM + "&nbsp;</ARG>")><$else><$let PARAM="None"></$if><PRMD HEAD="Parameters" SUB=(PARAM) MORE?=MORE><$content></PRMD></$macro>
<$macro CMDU/CLOSE/MBI="CMD"/NAW="CMDS|CMDP|CMDU|CMDE|CMDR"><PRMD HEAD="Use"><$content></PRMD></$macro>
<$macro CMDE/CLOSE/MBI="CMD"/NAW="CMDS|CMDP|CMDU|CMDE|CMDR" CMD:string/REQUIRED MORE:bool SIMPLE:bool><$define SUB:string=(CMD)><$let SUB=("<ARG>" + SUB + "</ARG>")><$if COND=(set SIMPLE)><PRMD HEAD="Example" MORE?=MORE><(SUB)></PRMD><$else><PRMD HEAD="Example" SUB=(SUB) MORE?=MORE><I><$content></I></PRMD></$if></$macro>
<$macro CMDES/MBI="CMD"/NAW="CMDS|CMDP|CMDU|CMDE|CMDR" CMD:string/REQUIRED MORE:bool><CMDE CMD=(CMD) MORE?=MORE SIMPLE></CMDE></$macro>
<$macro CMDR/CLOSE/MBI="CMD"/NAW="CMDS|CMDP|CMDU|CMDE|CMDR" NONE:bool><PRMD HEAD="Related&nbsp;commands"><$if COND=(set NONE)>None<$else><$content></$if></PRMD></$macro>
<$macro CMDL CMD:string/REQUIRED HREF:uri/STRIPEXT><ARG>*<$if COND=(set HREF)><A HREF=(HREF) TITLE=("*" + CMD)></$if><(CMD)><$if COND=(set HREF)></A></$if></ARG></$macro>

<* SWI descriptions *>
<$macro SWI/CLOSE/NAW="SWI" NAME:string/REQUIRED NUM:string/REQUIRED DESC:string/REQUIRED><PRM><PRMT><ARG><B><(NAME)></B></ARG><BR><ARG>(SWI&nbsp;&amp;<(NUM)>)</ARG></PRMT><PRMD><(DESC)></PRMD><$content></PRM></$macro>
<$macro SWIE/CLOSE/MBI="SWI"/NAW="SWIE|SWIO|SWII|SWIP|SWIR|SWIU|SWIS|SWIV" REG:string INDENT:bool MORE:bool NONE:bool><$define SUB:string><$if COND=(set REG)><$let SUB=(REG + "&nbsp;=")></$if><PRMD HEAD="On&nbsp;entry" SUB?=SUB INDENT?=INDENT MORE?=MORE><$if COND=(set NONE)>No parameters passed in registers<$else><$content></$if></PRMD></$macro>
<$macro SWIO/CLOSE/MBI="SWI"/NAW="SWIE|SWIO|SWII|SWIP|SWIR|SWIU|SWIS|SWIV" REG:string INDENT:bool MORE:bool NONE:bool><$define SUB:string><$if COND=(set REG)><$let SUB=(REG + "&nbsp;=")></$if><PRMD HEAD="On&nbsp;exit" SUB?=SUB INDENT?=INDENT MORE?=MORE><$if COND=(set NONE)>Registers preserved<$else><$content></$if></PRMD></$macro>
<$macro SWII/MBI="SWI"/NAW="SWIE|SWIO|SWII|SWIP|SWIR|SWIU|SWIS|SWIV"><PRMD HEAD="Interrupts">Interrupt status is undefined<BR>Fast interrupts are enabled</PRMD></$macro>
<$macro SWIP/MBI="SWI"/NAW="SWIE|SWIO|SWII|SWIP|SWIR|SWIU|SWIS|SWIV" MODE:enum("svc")="svc"><PRMD HEAD="Processor&nbsp;mode">Processor is in <$if COND=(MODE="svc")>SVC<$else>undefined</$if> mode</PRMD></$macro>
<$macro SWIR/MBI="SWI"/NAW="SWIE|SWIO|SWII|SWIP|SWIR|SWIU|SWIS|SWIV" REENTRANT:enum("undefined|yes|no")="undefined"><PRMD HEAD="Re-entrancy"><$if COND=(REENTRANT="yes")>SWI is re-entrant<$else><$if COND=(REENTRANT="no")>SWI is not re-entrant<$else>Not defined</$if></$if></PRMD></$macro>
<$macro SWIU/CLOSE/MBI="SWI"/NAW="SWIE|SWIO|SWII|SWIP|SWIR|SWIU|SWIS|SWIV"><PRMD HEAD="Use"><$content></PRMD></$macro>
<$macro SWIS/CLOSE/MBI="SWI"/NAW="SWIE|SWIO|SWII|SWIP|SWIR|SWIU|SWIS|SWIV" NONE:bool><PRMD HEAD="Related&nbsp;SWIs"><$if COND=(set NONE)>None<$else><$content></$if></PRMD></$macro>
<$macro SWIV/CLOSE/MBI="SWI"/NAW="SWIE|SWIO|SWII|SWIP|SWIR|SWIU|SWIS|SWIV" NONE:bool><PRMD HEAD="Related&nbsp;vectors"><$if COND=(set NONE)>None<$else><$content></$if></PRMD></$macro>
<$macro SWIL SWI:string/REQUIRED HREF:uri/STRIPEXT><ARG><$if COND=(set HREF)><A HREF=(HREF) TITLE=(SWI)></$if><(SWI)><$if COND=(set HREF)></A></$if></ARG></$macro>

<* WIMP message descriptions *>
<$macro WIMP/CLOSE/NAW="WIMP" NAME:string/REQUIRED NUM:string/REQUIRED><HEADING><(NAME)> (&amp;<(NUM)>)</HEADING><PARA><TABLE WIDTH="100%"><$stripws><$content><$stripws></TABLE></PARA></$macro>
<$macro WIMPB/CLOSE/MBI="WIMP"/NAW="WIMPB|WIMPU" OFFSET:string/REQUIRED><TR><TD WIDTH=30></TD><TD>R1+<(OFFSET)></TD><TD><$content></TD></TR></$macro>
<$macro WIMPU/CLOSE/MBI="WIMP"/NAW="WIMPB|WIMPU"><TR><TD COLSPAN=3><$content></TD></TR></$macro>
<$macro WIMPL MSG:string/REQUIRED HREF:uri/STRIPEXT><ARG><$if COND=(set HREF)><A HREF=(HREF) TITLE=(MSG)></$if><(MSG)><$if COND=(set HREF)></A></$if></ARG></$macro>

<* Menus *>
<$macro MEN/CLOSE/NAW="MEN" TITLE:string><TABLE ALIGN=CENTER BORDER=1><TR><TD BGCOLOR=(COLORBGM) ALIGN=CENTER><ARG><$if COND=(set TITLE)><(TITLE)><$else>PsiFS</$if></ARG></TD></TR><TR><TD><$content></TD></TR></TABLE></$macro>
<$macro MENI/MBI="MEN"/NAW="MENI" TITLE:string/REQUIRED HREF:uri/STRIPEXT><ARG><$if COND=(set HREF)><A HREF=(HREF) TITLE=(TITLE)></$if><(TITLE)><$if COND=(set HREF)></A></$if></ARG><BR></$macro>
<$macro MENL TITLE:string/REQUIRED HREF:uri/STRIPEXT><ARG><$if COND=(set HREF)><A HREF=(HREF) TITLE=(TITLE)></$if><(TITLE)><$if COND=(set HREF)></A></$if></ARG></$macro>

<* Common terms *>
<$macro NAME/CLOSE/NAW="NAME" SPECIAL:bool><$if COND=(set SPECIAL)><FONT COLOR=(COLORFGSP)><B></$if><I><$content></I><$if COND=(set SPECIAL)></B></FONT></$if></$macro>
<$macro MOUSE BUTTON:enum("select|menu|adjust")><$if COND=(set BUTTON)><NAME><$if COND=(BUTTON="select")>Select<$else><$if COND=(BUTTON="Menu")>Menu<$else>Adjust</$if></$if></NAME><$else>the mouse</$if></$macro>
<$macro PSIFS LONG:bool><NAME SPECIAL><$if COND=(set LONG)>Psion&nbsp;Filing&nbsp;System<$else>PsiFS</$if></NAME></$macro>
<$macro PSION><NAME>Psion</NAME></$macro>
<$macro SYMBIAN><NAME>Symbian</NAME></$macro>
<$macro XEMPLAR><NAME>Xemplar</NAME></$macro>
<$macro ERICSSON><NAME>Ericsson</NAME></$macro>
<$macro NOKIA><NAME>Nokia</NAME></$macro>
<$macro ACORN><NAME>Acorn</NAME></$macro>
<$macro ATOMWIDE><NAME>Atomwide</NAME></$macro>
<$macro INTELLIGENT><NAME>Intelligent&nbsp;Interfaces</NAME></$macro>
<$macro SABIN><NAME>Miles&nbsp;Sabin</NAME></$macro>
<$macro SERIALPORT><NAME>The&nbsp;Serial&nbsp;Port</NAME></$macro>
<$macro RISCOS VER:string><NAME>RISC&nbsp;OS<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>
<$macro DOSFS VER:string><NAME>DOSFS</NAME></$macro>
<$macro ARM TYPE:string/REQUIRED MHZ:string><NAME><$if COND=(set MHZ)><(MHZ)>MHz&nbsp;</$if>ARM<(TYPE)></NAME></$macro>
<$macro SA MHZ:string><NAME><$if COND=(set MHZ)><(MHZ)>MHz&nbsp;</$if>StrongARM</NAME></$macro>
<$macro KINETIC MHZ:string><NAME><$if COND=(set MHZ)><(MHZ)>MHz&nbsp;</$if>Kinetic</NAME></$macro>
<$macro RAM KB:string MB:string><NAME><$if COND=(set KB)><(KB)>kB&nbsp;</$if><$if COND=(set MB)><(MB)>MB&nbsp;</$if>RAM</NAME></$macro>
<$macro ROM VER:string/REQUIRED><NAME>ROM&nbsp;version&nbsp;<(VER)></NAME></$macro>
<$macro 3LINK><NAME>Psion&nbsp;3-Link</NAME></$macro>
<$macro ALINK><NAME>Acorn&nbsp;A-Link</NAME></$macro>
<$macro RS232><NAME>RS-232</NAME></$macro>
<$macro IRDA><NAME>IrDA</NAME></$macro>
<$macro SIBO FLA:bool><NAME>SIBO</NAME><$if COND=(set FLA)> (<B>SI</B>xteen-<B>B</B>it <B>O</B>rganizer)</$if></$macro>
<$macro EPOC><NAME>EPOC</NAME></$macro>
<$macro EPOC16><NAME>EPOC16</NAME></$macro>
<$macro EPOC32><NAME>EPOC32</NAME></$macro>
<$macro PSIWIN><NAME>PsiWin</NAME></$macro>
<$macro POCKETFS><NAME>PocketFS</NAME></$macro>
<$macro SIENA><NAME>Psion&nbsp;Siena</NAME></$macro>
<$macro REVO PLUS:bool><NAME>Psion&nbsp;Revo<$if COND=(set PLUS)>&nbsp;Plus</$if></NAME></$macro>
<$macro WORKABOUT><NAME>Psion&nbsp;Workabout</NAME></$macro>
<$macro GEOFOX><NAME>Geofox</NAME></$macro>
<$macro GEOFOXONE><NAME>Geofox-One</NAME></$macro>
<$macro ILIUM><NAME>Philips&nbsp;Ilium&nbsp;Accent</NAME></$macro>
<$macro OSARIS><NAME>Oregon&nbsp;Scientific&nbsp;Osaris</NAME></$macro>
<$macro R380><NAME>Ericsson&nbsp;R380</NAME></$macro>
<$macro MC218><NAME>Ericsson&nbsp;MC218</NAME></$macro>
<$macro 9210><NAME>Nokia&nbsp;9210</NAME></$macro>
<$macro 9290><NAME>Nokia&nbsp;9290</NAME></$macro>
<$macro SYNERGY><NAME>Philips&nbsp;Synergy</NAME></$macro>
<$macro SERIES VER:string="3"><NAME>Psion&nbsp;Series&nbsp;<(VER)></NAME></$macro>
<$macro NETBOOK><NAME>Psion&nbsp;Netbook</NAME></$macro>
<$macro ARC VER:string><NAME>Acorn&nbsp;<$if COND=(set VER)><(VER)><$else>Archimedes</$if></NAME></$macro>
<$macro MICRODIGITAL><NAME>MicroDigital</NAME></$macro>
<$macro MICO VER:string><NAME>MicroDigital&nbsp;Mico<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>
<$macro RPC VER:string><NAME>Acorn&nbsp;RiscPC<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>
<$macro SPRINTER VER:string><NAME>Acorn&nbsp;Sprinter<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>
<$macro RISCSTATION VER:string><NAME>RiscStation<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>
<$macro PB VER:string><NAME>Acorn&nbsp;PocketBook<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>
<$macro XPB VER:string><NAME>Xemplar&nbsp;PocketBook<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>
<$macro PLP TLA:bool><$if COND=(set TLA)><NAME><B>P</B>sion&nbsp;<B>L</B>ink&nbsp;<B>P</B>rotocol</NAME> (PLP)<$else><NAME>Psion&nbsp;Link&nbsp;Protocol</NAME></$if></$macro>
<$macro MSDOS VER:string><NAME>MS-DOS<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>
<$macro WINDOWS VER:string><NAME>Windows<$if COND=(set VER)>&nbsp;<(VER)></$if></NAME></$macro>