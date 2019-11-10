<*
    File        : contact/form.hsc
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

<$include FILE=(FILEREL)>
<PAGE TITLE="Contact Details" SUBTITLE="Fault Report Form" PARENT=":contact/index.html" KEYWORDS="Email,Web Page,Fault Report,Bugs,Problems">

<* Output bug report files *>
<$define FORMFILE:string/CONST="../!PsiFS/Docs/Contact/form">

<$export FILE=(FORMFILE) DATA="">
<$macro FORMOUT LINE:str>
    <$if COND=(set LINE)>
        <$export FILE=(FORMFILE) DATA=(LINE + hsc.lf) APPEND>
    <$else>
        <$export FILE=(FORMFILE) DATA=(hsc.lf) APPEND>
    </$if>
</$macro>

<FORMOUT LINE="Please complete all of the fields before submitting the form.">
<FORMOUT LINE="Send a separate report for each bug or problem.">
<FORMOUT>
<FORMOUT LINE=("Email to " + EMAIL)>
<FORMOUT>
<FORMOUT LINE="<--------------------------------- Cut here -------------------------------->">
<FORMOUT>
<FORMOUT LINE="PSIFS FAULT REPORT FORM">
<FORMOUT>

<FORM METHOD=POST ACTION=("http://www.btinternet.com/cgi-bin/userform.cgi")> 
<INPUT TYPE=HIDDEN NAME="Form title" VALUE="PsiFS fault report form">
<INPUT TYPE=HIDDEN NAME="SendMailTo" VALUE=(EMAIL)>
<INPUT TYPE=HIDDEN NAME="redirect" VALUE="http://www.thouky.co.uk/software/psifs/">

<PRM>

<PRMD>
    Please complete all of the fields before submitting the form. Send a separate report for each bug or problem.
</PRMD>
<PRMD>
    If <NAME><B>w</B>orld <B>w</B>ide <B>w</B>eb</NAME> access is not available, please email the <A HREF=":contact/form" TITLE="Textual Fault Report Form">textual version</A> of this form to <A HREF=("mailto:" + EMAIL) TITLE=("Send Email to " + AUTHOR)><ARG><(EMAIL)></ARG></A>.
</PRMD>

<FORMOUT LINE="Your name:                      []">
<PRMD HEAD="Your name">
    <INPUT TYPE=TEXT NAME="realname" VALUE="" SIZE=40 MAXLENGTH=255>
</PRMD>

<FORMOUT LINE="Email address:                  []">
<PRMD HEAD="Email address">
    <INPUT TYPE=TEXT NAME="email" VALUE="" SIZE=40 MAXLENGTH=255>
</PRMD>

<FORMOUT LINE="Priority:                       [Low | Medium | Urgent]">
<PRMD HEAD="Importance" SUB="Priority">
    <INPUT TYPE=RADIO NAME="priority" VALUE="Low" CHECKED>Low&nbsp;
    <INPUT TYPE=RADIO NAME="priority" VALUE="Medium">Medium&nbsp;
    <INPUT TYPE=RADIO NAME="priority" VALUE="Urgent">Urgent
</PRMD>

<FORMOUT LINE="Repeatability:                  [Always | Once | Occassional | Once]">
<PRMD SUB="Repeatability">
    <INPUT TYPE=RADIO NAME="repeatability" VALUE="Always">Always&nbsp;
    <INPUT TYPE=RADIO NAME="repeatability" VALUE="Often">Often&nbsp;
    <INPUT TYPE=RADIO NAME="repeatability" VALUE="Occasional">Occasional&nbsp;
    <INPUT TYPE=RADIO NAME="repeatability" VALUE="Once" CHECKED>Once
</PRMD>

<FORMOUT>
<FORMOUT LINE=("PsiFS module version:           [" + RELVER + " (" + RELDATE + ")]")>
<PRMD HEAD="PsiFS module" SUB="Version">
    <INPUT TYPE=TEXT NAME="psifs_module_ver" VALUE=(RELVER) SIZE=4 MAXLENGTH=12>
</PRMD>

<PRMD SUB="Date">
    <INPUT TYPE=TEXT NAME="psifs_module_date" VALUE=(RELDATE) SIZE=9 MAXLENGTH=9>
</PRMD>

<FORMOUT LINE=("PsiFS filer version:            [" + RELVER + " (" + RELDATE + ")]")>
<PRMD HEAD="PsiFS filer" SUB="Version">
    <INPUT TYPE=TEXT NAME="psifs_filer_ver" VALUE=(RELVER) SIZE=4 MAXLENGTH=12>
</PRMD>

<PRMD SUB="Date">
    <INPUT TYPE=TEXT NAME="psifs_filer_date" VALUE=(RELDATE) SIZE=9 MAXLENGTH=9>
</PRMD>

<FORMOUT>
<FORMOUT LINE="RISC OS computer model:         []">
<PRMD HEAD="RISC OS computer" SUB="Model">
    <SELECT NAME="riscos_model">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="Acorn A4">Acorn A4
        <OPTION VALUE="Acorn A305">Acorn A305
        <OPTION VALUE="Acorn A310">Acorn A310
        <OPTION VALUE="Acorn A410">Acorn A410
        <OPTION VALUE="Acorn A410/1">Acorn A410/1
        <OPTION VALUE="Acorn A420">Acorn A420
        <OPTION VALUE="Acorn A420/1">Acorn A420/1
        <OPTION VALUE="Acorn A440">Acorn A440
        <OPTION VALUE="Acorn A440/1">Acorn A440/1
        <OPTION VALUE="Acorn A500">Acorn A500
        <OPTION VALUE="Acorn A540">Acorn A540
        <OPTION VALUE="Acorn A3000">Acorn A3000
        <OPTION VALUE="Acorn A3010">Acorn A3010
        <OPTION VALUE="Acorn A3020">Acorn A3020
        <OPTION VALUE="Acorn A4000">Acorn A4000
        <OPTION VALUE="Acorn A5000">Acorn A5000
        <OPTION VALUE="Acorn A7000">Acorn A7000
        <OPTION VALUE="Acorn A7000+ Classic">Acorn A7000+ Classic
        <OPTION VALUE="Acorn A7000+ Odyssey">Acorn A7000+ Odyssey
        <OPTION VALUE="Acorn Network Ccomputer">Acorn Network Computer
        <OPTION VALUE="Acorn R140">Acorn R140    
        <OPTION VALUE="Acorn R225">Acorn R225    
        <OPTION VALUE="Acorn R260">Acorn R260
        <OPTION VALUE="Acorn RiscPC 233T">Acorn RiscPC 233T
        <OPTION VALUE="Acorn RiscPC 600">Acorn RiscPC 600
        <OPTION VALUE="Acorn RiscPC 700">Acorn RiscPC 700
        <OPTION VALUE="Acorn RiscPC SA233">Acorn RiscPC SA233
        <OPTION VALUE="Acorn RiscPC J233">Acorn RiscPC J233
        <OPTION VALUE="Acorn Set Top Box 1">Acorn Set Top Box 1
        <OPTION VALUE="Acorn Set Top Box 20">Acorn Set Top Box 20
        <OPTION VALUE="Acorn Set Top Box 22">Acorn Set Top Box 22
        <OPTION VALUE="Acorn Sprinter">Acorn Sprinter
        <OPTION VALUE="Acorn Web Wizard">Acorn Web Wizard
        <OPTION VALUE="MicroDigital Mico">MicroDigital Mico
        <OPTION VALUE="RiscStation NetWORX">RiscStation NetWORX
        <OPTION VALUE="RiscStation R7500 Lite">RiscStation R7500 Lite
        <OPTION VALUE="...">Other (please specify)
    </SELECT>
    <BR>
    <INPUT TYPE=TEXT NAME="riscos_model_other" VALUE="" SIZE=25 MAXLENGTH=255>
</PRMD>

<FORMOUT LINE="RISC OS version:                [v.vv]">
<PRMD SUB="OS version">
    <SELECT NAME="riscos_ver">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="RISC OS 2.00">RISC OS 2.00
        <OPTION VALUE="RISC OS 3.00">RISC OS 3.00
        <OPTION VALUE="RISC OS 3.10">RISC OS 3.10
        <OPTION VALUE="RISC OS 3.11">RISC OS 3.11
        <OPTION VALUE="RISC OS 3.50">RISC OS 3.50
        <OPTION VALUE="RISC OS 3.51">RISC OS 3.51
        <OPTION VALUE="RISC OS 3.60">RISC OS 3.60
        <OPTION VALUE="RISC OS 3.70">RISC OS 3.70
        <OPTION VALUE="RISC OS 4.00">RISC OS 4.00
        <OPTION VALUE="RISC OS 4.01">RISC OS 4.01
        <OPTION VALUE="RISC OS 4.02">RISC OS 4.02
        <OPTION VALUE="RISC OS 4.03">RISC OS 4.03
        <OPTION VALUE="...">Other (please specify)
    </SELECT>
    <BR>
    <INPUT TYPE=TEXT NAME="riscos_ver_other" VALUE="" SIZE=4 MAXLENGTH=4>
</PRMD>

<FORMOUT LINE="Main processor type:            []">
<PRMD SUB="Processor">
    <SELECT NAME="riscos_proc">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="ARM2">ARM2
        <OPTION VALUE="ARM250">ARM250
        <OPTION VALUE="ARM3">ARM3
        <OPTION VALUE="ARM610">ARM610
        <OPTION VALUE="ARM710">ARM710
        <OPTION VALUE="ARM7500">ARM7500
        <OPTION VALUE="ARM7500FE">ARM7500FE
        <OPTION VALUE="StrongARM">StrongARM
        <OPTION VALUE="Kinetic">Kinetic
        <OPTION VALUE="...">Other (please specify)
    </SELECT>
    <BR>
    <INPUT TYPE=TEXT NAME="riscos_proc_other" VALUE="" SIZE=25 MAXLENGTH=255>
</PRMD>

<FORMOUT LINE="Memory size:                    [xxxMB]">
<PRMD SUB="Memory size">
    <SELECT NAME="riscos_ram">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="1MB">1MB
        <OPTION VALUE="2MB">2MB
        <OPTION VALUE="3MB">3MB
        <OPTION VALUE="4MB">4MB
        <OPTION VALUE="5MB">5MB
        <OPTION VALUE="6MB">6MB
        <OPTION VALUE="7MB">7MB
        <OPTION VALUE="8MB">8MB
        <OPTION VALUE="9MB">9MB
        <OPTION VALUE="10MB">10MB
        <OPTION VALUE="11MB">11MB
        <OPTION VALUE="12MB">12MB
        <OPTION VALUE="13MB">13MB
        <OPTION VALUE="14MB">14MB
        <OPTION VALUE="16MB">16MB
        <OPTION VALUE="17MB">17MB
        <OPTION VALUE="18MB">18MB
        <OPTION VALUE="19MB">19MB
        <OPTION VALUE="20MB">20MB
        <OPTION VALUE="21MB">21MB
        <OPTION VALUE="22MB">22MB
        <OPTION VALUE="24MB">24MB
        <OPTION VALUE="25MB">25MB
        <OPTION VALUE="26MB">26MB
        <OPTION VALUE="32MB">32MB
        <OPTION VALUE="33MB">33MB
        <OPTION VALUE="34MB">34MB
        <OPTION VALUE="35MB">35MB
        <OPTION VALUE="36MB">36MB
        <OPTION VALUE="37MB">37MB
        <OPTION VALUE="38MB">38MB
        <OPTION VALUE="40MB">40MB
        <OPTION VALUE="41MB">41MB
        <OPTION VALUE="42MB">42MB
        <OPTION VALUE="48MB">48MB
        <OPTION VALUE="49MB">49MB
        <OPTION VALUE="50MB">50MB
        <OPTION VALUE="61MB">61MB
        <OPTION VALUE="62MB">62MB
        <OPTION VALUE="63MB">63MB
        <OPTION VALUE="64MB">64MB
        <OPTION VALUE="65MB">65MB
        <OPTION VALUE="66MB">66MB
        <OPTION VALUE="67MB">67MB
        <OPTION VALUE="68MB">68MB
        <OPTION VALUE="69MB">69MB
        <OPTION VALUE="70MB">70MB
        <OPTION VALUE="71MB">71MB
        <OPTION VALUE="72MB">72MB
        <OPTION VALUE="73MB">73MB
        <OPTION VALUE="74MB">74MB
        <OPTION VALUE="76MB">76MB
        <OPTION VALUE="77MB">77MB
        <OPTION VALUE="78MB">78MB
        <OPTION VALUE="79MB">79MB
        <OPTION VALUE="80MB">80MB
        <OPTION VALUE="81MB">81MB
        <OPTION VALUE="82MB">82MB
        <OPTION VALUE="84MB">84MB
        <OPTION VALUE="85MB">85MB
        <OPTION VALUE="86MB">86MB
        <OPTION VALUE="92MB">92MB
        <OPTION VALUE="93MB">93MB
        <OPTION VALUE="94MB">94MB
        <OPTION VALUE="95MB">95MB
        <OPTION VALUE="96MB">96MB
        <OPTION VALUE="97MB">97MB
        <OPTION VALUE="98MB">98MB
        <OPTION VALUE="100MB">100MB
        <OPTION VALUE="101MB">101MB
        <OPTION VALUE="102MB">102MB
        <OPTION VALUE="108MB">108MB
        <OPTION VALUE="109MB">109MB
        <OPTION VALUE="110MB">110MB
        <OPTION VALUE="124MB">124MB
        <OPTION VALUE="125MB">125MB
        <OPTION VALUE="126MB">126MB
        <OPTION VALUE="127MB">127MB
        <OPTION VALUE="128MB">128MB
        <OPTION VALUE="129MB">129MB
        <OPTION VALUE="130MB">130MB
        <OPTION VALUE="131MB">131MB
        <OPTION VALUE="132MB">132MB
        <OPTION VALUE="133MB">133MB
        <OPTION VALUE="134MB">134MB
        <OPTION VALUE="135MB">135MB
        <OPTION VALUE="136MB">136MB
        <OPTION VALUE="137MB">137MB
        <OPTION VALUE="138MB">138MB
        <OPTION VALUE="140MB">140MB
        <OPTION VALUE="141MB">141MB
        <OPTION VALUE="142MB">142MB
        <OPTION VALUE="143MB">143MB
        <OPTION VALUE="144MB">144MB
        <OPTION VALUE="145MB">145MB
        <OPTION VALUE="146MB">146MB
        <OPTION VALUE="148MB">148MB
        <OPTION VALUE="149MB">149MB
        <OPTION VALUE="150MB">150MB
        <OPTION VALUE="156MB">156MB
        <OPTION VALUE="157MB">157MB
        <OPTION VALUE="158MB">158MB
        <OPTION VALUE="159MB">159MB
        <OPTION VALUE="160MB">160MB
        <OPTION VALUE="161MB">161MB
        <OPTION VALUE="162MB">162MB
        <OPTION VALUE="164MB">164MB
        <OPTION VALUE="165MB">165MB
        <OPTION VALUE="166MB">166MB
        <OPTION VALUE="172MB">172MB
        <OPTION VALUE="173MB">173MB
        <OPTION VALUE="174MB">174MB
        <OPTION VALUE="188MB">188MB
        <OPTION VALUE="189MB">189MB
        <OPTION VALUE="190MB">190MB
        <OPTION VALUE="191MB">191MB
        <OPTION VALUE="192MB">192MB
        <OPTION VALUE="193MB">193MB
        <OPTION VALUE="194MB">194MB
        <OPTION VALUE="196MB">196MB
        <OPTION VALUE="197MB">197MB
        <OPTION VALUE="198MB">198MB
        <OPTION VALUE="204MB">204MB
        <OPTION VALUE="205MB">205MB
        <OPTION VALUE="206MB">206MB
        <OPTION VALUE="220MB">220MB
        <OPTION VALUE="221MB">221MB
        <OPTION VALUE="222MB">222MB
        <OPTION VALUE="252MB">252MB
        <OPTION VALUE="253MB">253MB
        <OPTION VALUE="254MB">254MB
        <OPTION VALUE="255MB">255MB
        <OPTION VALUE="256MB">256MB
        <OPTION VALUE="257MB">257MB
        <OPTION VALUE="258MB">258MB
        <OPTION VALUE="260MB">260MB
        <OPTION VALUE="261MB">261MB
        <OPTION VALUE="262MB">262MB
        <OPTION VALUE="268MB">268MB
        <OPTION VALUE="269MB">269MB
        <OPTION VALUE="270MB">270MB
        <OPTION VALUE="284MB">284MB
        <OPTION VALUE="285MB">285MB
        <OPTION VALUE="286MB">286MB
        <OPTION VALUE="316MB">316MB
        <OPTION VALUE="317MB">317MB
        <OPTION VALUE="318MB">318MB
        <OPTION VALUE="380MB">380MB
        <OPTION VALUE="381MB">381MB
        <OPTION VALUE="382MB">382MB
    </SELECT>
</PRMD>

<FORMOUT LINE="Any patches: (please list all)  []">
<PRMD SUB="Any patches<BR>(please list all)">
    <TEXTAREA NAME="riscos_patches" ROWS=3 COLS=25></TEXTAREA>
</PRMD>

<FORMOUT>
<FORMOUT LINE="EPOC or SIBO computer model:    []">
<PRMD HEAD="EPOC or SIBO computer" SUB="Model">
    <SELECT NAME="epoc_model">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="Acorn PocketBook I">Acorn PocketBook I
        <OPTION VALUE="Acorn PocketBook II">Acorn PocketBook II
        <OPTION VALUE="Ericsson MC218">Ericsson MC218
        <OPTION VALUE="Ericsson R380">Ericsson R380
        <OPTION VALUE="Geofox-One">Geofox-One
        <OPTION VALUE="Nokia 9210">Nokia 9210
        <OPTION VALUE="Nokia 9290">Nokia 9290
        <OPTION VALUE="Oregon Scientific Osaris">Oregon Scientific Osaris
        <OPTION VALUE="Philips Ilium Accent">Philips Ilium Accent
        <OPTION VALUE="Philips Synergy">Philips Synergy
        <OPTION VALUE="Psion Netbook">Psion Netbook
        <OPTION VALUE="Psion Revo">Psion Revo
        <OPTION VALUE="Psion Revo Plus">Psion Revo Plus
        <OPTION VALUE="Psion Series 3">Psion Series 3
        <OPTION VALUE="Psion Series 3a">Psion Series 3a
        <OPTION VALUE="Psion Series 3c">Psion Series 3c
        <OPTION VALUE="Psion Series 3mx">Psion Series 3mx
        <OPTION VALUE="Psion Series 5">Psion Series 5
        <OPTION VALUE="Psion Series 5mx">Psion Series 5mx
        <OPTION VALUE="Psion Series 7">Psion Series 7
        <OPTION VALUE="Psion Siena">Psion Siena
        <OPTION VALUE="Psion Series Workabout">Psion Workabout
        <OPTION VALUE="Xemplar PocketBook III">Xemplar PocketBook III
        <OPTION VALUE="...">Other (please specify)
    </SELECT>
    <BR>
    <INPUT TYPE=TEXT NAME="epoc_model_other" VALUE="" SIZE=25 MAXLENGTH=255>
</PRMD>

<FORMOUT LINE="EPOC or SIBO version:           [v.vv(rrr)]">
<PRMD SUB="OS version">
    <INPUT TYPE=TEXT NAME="epoc_ver" VALUE="v.vv(rrr)" SIZE=9 MAXLENGTH=15>
</PRMD>

<FORMOUT>
<FORMOUT LINE="Serial port interface type:     [Internal | Atomwide | The Serial Port]">
<PRMD HEAD="Serial" SUB="Interface">
    <SELECT NAME="serial_port">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="Internal">Internal
        <OPTION VALUE="Atomwide (Single)">Atomwide (Single)
        <OPTION VALUE="Atomwide (Dual)">Atomwide (Dual)
        <OPTION VALUE="Atomwide (Triple)">Atomwide (Triple)
        <OPTION VALUE="Brainsoft Multipod">Brainsoft Multipod
        <OPTION VALUE="The Serial Port (Dual)">The Serial Port (Dual)
        <OPTION VALUE="...">Other (please specify)
    </SELECT>
    <BR>
    <INPUT TYPE=TEXT NAME="serial_port_other" VALUE="" SIZE=25 MAXLENGTH=255>
</PRMD>

<FORMOUT LINE="Serial block driver:            [InternalPC | SP_DualPC]">
<PRMD SUB="Block driver">
    <SELECT NAME="serial_block">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="Internal">Internal
        <OPTION VALUE="InternalPC">InternalPC
        <OPTION VALUE="SP_Dual">SP_Dual
        <OPTION VALUE="SP_DualPC">SP_DualPC
        <OPTION VALUE="...">Other (please specify)
    </SELECT>
    <BR>
    <INPUT TYPE=TEXT NAME="serial_block_other" VALUE="" SIZE=25 MAXLENGTH=255>
</PRMD>

<FORMOUT LINE="Baud rate:                      [9600 | 19200 | 38400 | 57600 | 115200]">
<PRMD SUB="Baud rate">
    <SELECT NAME="serial_baud">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="9600">9600 baud
        <OPTION VALUE="19200">19200 baud
        <OPTION VALUE="38400">38400 baud
        <OPTION VALUE="57600">57600 baud
        <OPTION VALUE="115200">115200 baud
        <OPTION VALUE="...">Other (please specify)
    </SELECT>
    <BR>
    <INPUT TYPE=TEXT NAME="serial_baud_other" VALUE="" SIZE=6 MAXLENGTH=7>
</PRMD>

<FORMOUT LINE="Serial cable type:              [3-Link | A-Link | RS-232]">
<PRMD SUB="Cable">
    <SELECT NAME="serial_cable">
        <OPTION VALUE="" SELECTED>
        <OPTION VALUE="Acorn A-Link">Acorn A-Link
        <OPTION VALUE="Psion 3-Link">Psion 3-Link
        <OPTION VALUE="RS-232 serial cable">RS-232 serial cable
        <OPTION VALUE="...">Other (please specify)
    </SELECT>
    <BR>
    <INPUT TYPE=TEXT NAME="serial_cable_other" VALUE="" SIZE=25 MAXLENGTH=255>
</PRMD>

<FORMOUT>
<FORMOUT LINE="Short summary: []">
<PRMD HEAD="Short summary">
    <INPUT TYPE=TEXT NAME="summary" VALUE="" SIZE=40 MAXLENGTH=255>
</PRMD>

<FORMOUT>
<FORMOUT LINE="Full details:">
<PRMD HEAD="Full details">
    <TEXTAREA NAME="details" ROWS=10 COLS=40></TEXTAREA>
</PRMD>

<PRMD HEAD="Post it">
    <INPUT TYPE=SUBMIT VALUE="Send the form">&nbsp;or&nbsp;<INPUT TYPE=RESET VALUE="Clear the form">
</PRMD>

</PRM>
</FORM>

</PAGE>
