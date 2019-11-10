<*
    File        : swi/get.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_Get" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_Get,Status,Configuration,Block Driver,Baud Rate,Auto Baud Rate,Disc Name,Disk Name,EPOC,SIBO,Printer Mirror,Remote Link,Drive">

<SWI NAME="PsiFS_Get" NUM="520C3" DESC="Read a <PSIFS> option or status value">
    <SWIE REG="R0">option or status value to read</SWIE>
    <SWIE MORE>If the value specified by R0 is a string</SWIE>
    <SWIE REG="R1" INDENT MORE>pointer to buffer to contain null terminated string value</SWIE>
    <SWIE REG="R2" INDENT MORE>size of buffer</SWIE>
    <SWIO>If the value specified by R0 is numeric</SWIO>   
    <SWIO REG="R1" INDENT MORE>value of the option</SWIO>
    <SWIO MORE>If the value specified by R0 is a string</SWIO>
    <SWIO REG="R2" INDENT MORE>
        number of spare bytes in the buffer <B>including</B> the null terminator, i.e.:
        <TABLE ALIGN=LEFT>
            <TR ALIGN=TOP VALIGN=TOP><TD NOWRAP>R2 &gt; 0</TD><TD>implies there are (R2-1) completely unused bytes in the buffer; so R2=1 implies there are 0 unused bytes in the buffer, and therefore the terminator just fitted</TD></TR>
            <TR ALIGN=TOP VALIGN=TOP><TD NOWRAP>R2 &lt; 1</TD><TD>implies there are (1-R2) too many bytes to fit in the buffer, which has consequently not been filled in; so R2=0 implies there is 1 byte too many - the terminator - to fit in the buffer</TD></TR>
        </TABLE>
    </SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call reads a <PSIFS> option or status value. An error may be returned if an attempt is made to read the value of an option that is meaningless for the current mode of operation.
        <P>
        The values that may be read are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>R0</TH><TH>Type</TH><TH>Option</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0</TD><TD ALIGN=CENTER>numeric</TD><TD>mode of operation</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;100</TD><TD ALIGN=CENTER>string</TD><TD>name of the serial block driver</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;101</TD><TD ALIGN=CENTER>numeric</TD><TD>serial block driver port number</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;102</TD><TD ALIGN=CENTER>numeric</TD><TD>serial block driver baud rate</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;103</TD><TD ALIGN=CENTER>string</TD><TD>optional extra serial block driver options</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;104</TD><TD ALIGN=CENTER>numeric</TD><TD>automatic baud rate identification mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;108</TD><TD ALIGN=CENTER>numeric</TD><TD>active serial block driver baud rate</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;110</TD><TD ALIGN=CENTER>numeric</TD><TD>clock synchronization mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;120</TD><TD ALIGN=CENTER>numeric</TD><TD>remote link idle disconnect time</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;121</TD><TD ALIGN=CENTER>numeric</TD><TD>printer mirror idle disconnect time</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;122</TD><TD ALIGN=CENTER>numeric</TD><TD>idle disconnect external power mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;128</TD><TD ALIGN=CENTER>numeric</TD><TD>idle background operations mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;200</TD><TD ALIGN=CENTER>numeric</TD><TD>number of bytes of serial data received</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;201</TD><TD ALIGN=CENTER>numeric</TD><TD>number of bytes of serial data transmitted</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;210</TD><TD ALIGN=CENTER>numeric</TD><TD>number of valid protocol frames received</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;211</TD><TD ALIGN=CENTER>numeric</TD><TD>number of invalid protocol frames received</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;212</TD><TD ALIGN=CENTER>numeric</TD><TD>number of retries for received protocol frames</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;214</TD><TD ALIGN=CENTER>numeric</TD><TD>number of protocol frames transmitted</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;216</TD><TD ALIGN=CENTER>numeric</TD><TD>number of retries for transmitted protocol frames</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1000</TD><TD ALIGN=CENTER>numeric</TD><TD>status of the remote link</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1010</TD><TD ALIGN=CENTER>numeric</TD><TD>type of the remote machine</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1011</TD><TD ALIGN=CENTER>string</TD><TD>description of the remote machine type</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1012</TD><TD ALIGN=CENTER>numeric</TD><TD>language of the remote machine</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1018</TD><TD ALIGN=CENTER>numeric</TD><TD>major version of the remote machine operating system</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1019</TD><TD ALIGN=CENTER>numeric</TD><TD>minor version of the remote machine operating system</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;101A</TD><TD ALIGN=CENTER>numeric</TD><TD>build of remote machine operating system</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1020</TD><TD ALIGN=CENTER>numeric</TD><TD>low word of the unique identifier of the remote machine</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1021</TD><TD ALIGN=CENTER>numeric</TD><TD>high word of the unique identifier of the remote machine</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1030</TD><TD ALIGN=CENTER>string</TD><TD>owner information of the remote machine</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;11xx</TD><TD ALIGN=CENTER>numeric</TD><TD>status of the specified remote drive</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;12xx</TD><TD ALIGN=CENTER>string</TD><TD>name of the specified remote drive</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;13xx</TD><TD ALIGN=CENTER>numeric</TD><TD>unique identifier of the specified remote drive</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1800</TD><TD ALIGN=CENTER>numeric</TD><TD>status of the main battery</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1801</TD><TD ALIGN=CENTER>numeric</TD><TD>voltage of the main battery</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1802</TD><TD ALIGN=CENTER>numeric</TD><TD>maximum voltage of the main battery</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1810</TD><TD ALIGN=CENTER>numeric</TD><TD>status of the backup battery</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1811</TD><TD ALIGN=CENTER>numeric</TD><TD>voltage of the backup battery</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1812</TD><TD ALIGN=CENTER>numeric</TD><TD>maximum voltage of the backup battery</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1820</TD><TD ALIGN=CENTER>numeric</TD><TD>status of external power</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;2000</TD><TD ALIGN=CENTER>numeric</TD><TD>status of the printer mirror</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;2001</TD><TD ALIGN=CENTER>string</TD><TD>name of the printer mirror destination</TD></TR>
        </TABLE>
        <P>
        If a string value is specified this may be used as a two-pass process:
        <OL>
            <LI>On entry, set R0 to specify the value to read, and R1 and R2 (the pointer to, and the size of, the buffer) to zero. On exit, R2 = -(length of string)
            <LI>Claim a buffer of the right size (1-R2, not just -R2, as a space is needed for the terminator). On entry, ensure that R0 still specifies the value to read, that R1 is set to point to the buffer, and R2 contains the lenght of the buffer. On exit the buffer should be filled in, and R2 should be 1; but check to make sure.
        </OL>
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_Set" HREF=":swi/set.html">, <SWIL SWI="PsiFS_Mode" HREF=":swi/mode.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;0" NUM="520C3" DESC="Get the mode of operation">
    <SWIE REG="R0">&amp;0</SWIE>
    <SWIO REG="R1">mode of operation</SWIO>
    <SWIU>
        This call reads the current mode of operation. The possible values are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD>inactive</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>1</TD><TD>remote link mode</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>2</TD><TD>printer mirror mode</TD></TR>
        </TABLE>
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;100" NUM="520C3" DESC="Get the serial block driver name">
    <SWIE REG="R0">&amp;100</SWIE>
    <SWIE REG="R1" MORE>pointer to buffer to contain null terminated serial block driver name</SWIE>
    <SWIE REG="R2" MORE>size of buffer</SWIE>
    <SWIO REG="R2">number of spare bytes in the buffer <B>including</B> the null terminator</SWIO>
    <SWIU>
        This call reads the name of the configured serial block driver.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;101" NUM="520C3" DESC="Get the serial block driver port number">
    <SWIE REG="R0">&amp;101</SWIE>
    <SWIO REG="R1">port number (0 for the first)</SWIO>
    <SWIU>
        This call reads the configured serial block driver port number.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;102" NUM="520C3" DESC="Get the serial block driver baud rate">
    <SWIE REG="R0">&amp;102</SWIE>
    <SWIO REG="R1">baud rate</SWIO>
    <SWIU>
        This call reads the configured serial block driver baud rate. The actual baud rate used may be different if either the requested baud rate is not available or automatic baud rate identification is active.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;103" NUM="520C3" DESC="Get the serial block driver options">
    <SWIE REG="R0">&amp;103</SWIE>
    <SWIE REG="R1" MORE>pointer to buffer to contain null terminated serial block driver options</SWIE>
    <SWIE REG="R2" MORE>size of buffer</SWIE>
    <SWIO REG="R2">number of spare bytes in the buffer <B>including</B> the null terminator</SWIO>
    <SWIU>
        This call reads the optional parameter string of the configured serial block driver.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;104" NUM="520C3" DESC="Get the automatic baud rate identification mode">
    <SWIE REG="R0">&amp;104</SWIE>
    <SWIO REG="R1">automatic baud rate identification (0 disables)</SWIO>
    <SWIU>
        This call reads whether automatic baud rate identification is enabled. Only the remote link is affected; the printer mirror always uses the configured baud rate.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;108" NUM="520C3" DESC="Get the active serial block driver baud rate">
    <SWIE REG="R0">&amp;108</SWIE>
    <SWIO REG="R1">active baud rate</SWIO>
    <SWIU>
        This call reads the serial block driver baud rate actively being used. This may be different to the configured baud rate if either the requested baud rate is not available or automatic baud rate identification is active.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;110" NUM="520C3" DESC="Get the clock synchronization mode">
    <SWIE REG="R0">&amp;110</SWIE>
    <SWIO REG="R1">clock synchronization mode (0 disables)</SWIO>
    <SWIU>
        This call reads whether synchronization of clocks on connection of the remote link is enabled. This only affects <EPOC> devices; it is not possible to synchronize the clock of <SIBO> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;120" NUM="520C3" DESC="Get the remote link idle disconnect time">
    <SWIE REG="R0">&amp;120</SWIE>
    <SWIO REG="R1">idle disconnect time in seconds (0 disables)</SWIO>
    <SWIU>
        This call reads whether automatic disconnection of the remote link after a period of activity is enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;121" NUM="520C3" DESC="Get the printer mirror idle disconnect time">
    <SWIE REG="R0">&amp;121</SWIE>
    <SWIO REG="R1">idle disconnect time in seconds (0 disables)</SWIO>
    <SWIU>
        This call reads whether automatic disconnection of the printer mirror after a period of activity is enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;122" NUM="520C3" DESC="Get the idle disconnect external power mode">
    <SWIE REG="R0">&amp;122</SWIE>
    <SWIO REG="R1">idle disconnect external power mode (0 disables)</SWIO>
    <SWIU>
        This call reads whether automatic disconnection after a period of inactivity if running on external power is enabled. This only affects <EPOC> devices; it is not possible to determine the status of external power for <SIBO> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;128" NUM="520C3" DESC="Get the idle background operations mode">
    <SWIE REG="R0">&amp;128</SWIE>
    <SWIO REG="R1">idle background operations mode (0 disables)</SWIO>
    <SWIU>
        This call reads whether throttling back of background operations when idle is enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;200" NUM="520C3" DESC="Get the number of bytes of serial data received">
    <SWIE REG="R0">&amp;200</SWIE>
    <SWIO REG="R1">number of bytes of serial data received</SWIO>
    <SWIU>
        This call reads the number of bytes of serial data received since the remote link or printer mirror was last enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;201" NUM="520C3" DESC="Get the number of bytes of serial data transmitted">
    <SWIE REG="R0">&amp;200</SWIE>
    <SWIO REG="R1">number of bytes of serial data trasmitted</SWIO>
    <SWIU>
        This call reads the number of bytes of serial data transmitted since the remote link or printer mirror was last enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;210" NUM="520C3" DESC="Get the number of valid protocol frames received">
    <SWIE REG="R0">&amp;210</SWIE>
    <SWIO REG="R1">number of valid protocol frames received</SWIO>
    <SWIU>
        This call reads the number of valid protocol frames received since the remote link was last enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;211" NUM="520C3" DESC="Get the number of invalid protocol frames received">
    <SWIE REG="R0">&amp;211</SWIE>
    <SWIO REG="R1">number of invalid protocol frames received</SWIO>
    <SWIU>
        This call reads the number of invalid protocol frames received since the remote link was last enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;212" NUM="520C3" DESC="Get the number of retries for received protocol frames">
    <SWIE REG="R0">&amp;212</SWIE>
    <SWIO REG="R1">number of retries for received protocol frames</SWIO>
    <SWIU>
        This call reads the number of retries for received protocol frames since the remote link was last enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;214" NUM="520C3" DESC="Get the number of protocol frames transmitted">
    <SWIE REG="R0">&amp;214</SWIE>
    <SWIO REG="R1">number of protocol frames transmitted</SWIO>
    <SWIU>
        This call reads the number of protocol frames transmitted since the remote link was last enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;216" NUM="520C3" DESC="Get the number of retries for transmitted protocol frames">
    <SWIE REG="R0">&amp;216</SWIE>
    <SWIO REG="R1">number of retries for transmitted protocol frames</SWIO>
    <SWIU>
        This call reads the number of retries for transmitted protocol frames since the remote link was last enabled.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1000" NUM="520C3" DESC="Get the status of the remote link">
    <SWIE REG="R0">&amp;1000</SWIE>
    <SWIO REG="R1">remote link status</SWIO>
    <SWIU>
        This call reads the status of the remote link. The bits of the result have the following meaning:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Bit</TH><TH>Meaning when set</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD ALIGN=LEFT>remote link active</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>4</TD><TD ALIGN=LEFT>connected to a <SIBO> device</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>5</TD><TD ALIGN=LEFT>connected to an <EPOC> device</TD></TR>
        </TABLE>
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1010" NUM="520C3" DESC="Get the type of the remote machine">
    <SWIE REG="R0">&amp;1010</SWIE>
    <SWIO REG="R1">type of the remote machine</SWIO>
    <SWIU>
        This call reads the type of the remote machine. The possible values are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning when set</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;00</TD><TD ALIGN=LEFT>Unknown</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;01</TD><TD ALIGN=LEFT>PC</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;02</TD><TD ALIGN=LEFT>MC</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;03</TD><TD ALIGN=LEFT>HC</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;04</TD><TD ALIGN=LEFT>Series 3</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;05</TD><TD ALIGN=LEFT>Series 3a, 3c, or 3mx</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;06</TD><TD ALIGN=LEFT>Workabout</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;07</TD><TD ALIGN=LEFT>Sienna</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;08</TD><TD ALIGN=LEFT>Series 3c</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;20</TD><TD ALIGN=LEFT>Series 5</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;21</TD><TD ALIGN=LEFT>WinC</TD></TR>
        </TABLE>
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1011" NUM="520C3" DESC="Get the description of the remote machine type">
    <SWIE REG="R0">&amp;1011</SWIE>
    <SWIE REG="R1" MORE>pointer to buffer to contain null terminated description</SWIE>
    <SWIE REG="R2" MORE>size of buffer</SWIE>
    <SWIO REG="R2">number of spare bytes in the buffer <B>including</B> the null terminator</SWIO>
    <SWIU>
        This call reads the description of the remote machine type.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1012" NUM="520C3" DESC="Get the language of the remote machine">
    <SWIE REG="R0">&amp;1012</SWIE>
    <SWIO REG="R1">language</SWIO>
    <SWIU>
        This call reads the language of the remote machine. The possible values are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;00</TD><TD ALIGN=LEFT>Test</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;01</TD><TD ALIGN=LEFT>English</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;02</TD><TD ALIGN=LEFT>French</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;03</TD><TD ALIGN=LEFT>German</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;04</TD><TD ALIGN=LEFT>Spanish</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;05</TD><TD ALIGN=LEFT>Italian</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;06</TD><TD ALIGN=LEFT>Swedish</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;07</TD><TD ALIGN=LEFT>Danish</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;08</TD><TD ALIGN=LEFT>Norwegian</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;09</TD><TD ALIGN=LEFT>Finnish</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0A</TD><TD ALIGN=LEFT>American</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0B</TD><TD ALIGN=LEFT>Swiss French</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0C</TD><TD ALIGN=LEFT>Swiss German</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0D</TD><TD ALIGN=LEFT>Portuguese</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0E</TD><TD ALIGN=LEFT>Turkish</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;0F</TD><TD ALIGN=LEFT>Icelandic</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;10</TD><TD ALIGN=LEFT>Russian</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;11</TD><TD ALIGN=LEFT>Hungarian</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;12</TD><TD ALIGN=LEFT>Dutch</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;13</TD><TD ALIGN=LEFT>Belgian Flemish</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;14</TD><TD ALIGN=LEFT>Australian</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;15</TD><TD ALIGN=LEFT>Belgian French</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;16</TD><TD ALIGN=LEFT>Austrian</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;17</TD><TD ALIGN=LEFT>New Zealand</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;18</TD><TD ALIGN=LEFT>International French</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;19</TD><TD ALIGN=LEFT>Czech</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1A</TD><TD ALIGN=LEFT>Slovak</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1B</TD><TD ALIGN=LEFT>Polish</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>&amp;1C</TD><TD ALIGN=LEFT>Slovenian</TD></TR>
        </TABLE>
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1018" NUM="520C3" DESC="Get the major version of the remote machine operating system">
    <SWIE REG="R0">&amp;1018</SWIE>
    <SWIO REG="R1">major version</SWIO>
    <SWIU>
        This call reads the major version of the remote machine operating system.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1019" NUM="520C3" DESC="Get the minor version of the remote machine operating system">
    <SWIE REG="R0">&amp;1019</SWIE>
    <SWIO REG="R1">minor version</SWIO>
    <SWIU>
        This call reads the minor version of the remote machine operating system.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;101A" NUM="520C3" DESC="Get the build of the remote machine operating system">
    <SWIE REG="R0">&amp;101A</SWIE>
    <SWIO REG="R1">build</SWIO>
    <SWIU>
        This call reads the build of the remote machine operating system.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1020" NUM="520C3" DESC="Get the low word of the unique identifier of the remote machine">
    <SWIE REG="R0">&amp;1020</SWIE>
    <SWIO REG="R1">low word of unique identifier</SWIO>
    <SWIU>
        This call reads the low word of the unique identifier of the remote machine.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1021" NUM="520C3" DESC="Get the high word of the unique identifier of the remote machine">
    <SWIE REG="R0">&amp;1021</SWIE>
    <SWIO REG="R1">high word of unique identifier</SWIO>
    <SWIU>
        This call reads the high word of the unique identifier of the remote machine.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1030" NUM="520C3" DESC="Get the owner information of the remote machine">
    <SWIE REG="R0">&amp;1030</SWIE>
    <SWIE REG="R1" MORE>pointer to buffer to contain null terminated owner information</SWIE>
    <SWIE REG="R2" MORE>size of buffer</SWIE>
    <SWIO REG="R2">number of spare bytes in the buffer <B>including</B> the null terminator</SWIO>
    <SWIU>
        This call reads the owner information of the remote machine.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;11xx" NUM="520C3" DESC="Get the status of the specified remote drive">
    <SWIE REG="R0">&amp;1100 + drive</SWIE>
    <SWIO REG="R1">remote drive status</SWIO>
    <SWIU>
        This call reads the status of a single remote drive. The bits of the result have the following meaning:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Bit</TH><TH>Meaning when set</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD ALIGN=LEFT>drive present</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>8</TD><TD ALIGN=LEFT>ROM drive</TD></TR>
        </TABLE>
        <P>
        The drive may be specified in one of three ways:
        <UL>
            <LI>a zero based index (0 for drive A, through to 25 for drive Z)
            <LI>the ASCII code of the drive letter in upper case (65 for drive A, through to 90 for drive Z)
            <LI>the ASCII code of the drive letter in lower case (97 for drive A, through to 122 for drive Z)
        </UL>
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;12xx" NUM="520C3" DESC="Get the name of the specified remote drive">
    <SWIE REG="R0">&amp;1200 + drive</SWIE>
    <SWIE REG="R1" MORE>pointer to buffer to contain null terminated drive name</SWIE>
    <SWIE REG="R2" MORE>size of buffer</SWIE>
    <SWIO REG="R2">number of spare bytes in the buffer <B>including</B> the null terminator</SWIO>
    <SWIU>
        This call reads the name of a single remote drive.
        <P>
        The drive may be specified in one of three ways:
        <UL>
            <LI>a zero based index (0 for drive A, through to 25 for drive Z)
            <LI>the ASCII code of the drive letter in upper case (65 for drive A, through to 90 for drive Z)
            <LI>the ASCII code of the drive letter in lower case (97 for drive A, through to 122 for drive Z)
        </UL>
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;13xx" NUM="520C3" DESC="Get the unique identifier of the specified remote drive">
    <SWIE REG="R0">&amp;1300 + drive</SWIE>
    <SWIO REG="R1">remote drive unique identifier</SWIO>
    <SWIU>
        This call reads the unique identifier of a single remote drive.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1800" NUM="520C3" DESC="Get the status of the main battery">
    <SWIE REG="R0">&amp;1800</SWIE>
    <SWIO REG="R1">main battery status</SWIO>
    <SWIU>
        This call reads the status of the main battery. The possible values are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD>dead</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>1</TD><TD>very low</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>2</TD><TD>low</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>3</TD><TD>good</TD></TR>
        </TABLE>
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1801" NUM="520C3" DESC="Get the voltage of the main battery">
    <SWIE REG="R0">&amp;1801</SWIE>
    <SWIO REG="R1">main battery voltage</SWIO>
    <SWIU>
        This call reads the voltage of the main battery in milli-volts.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1802" NUM="520C3" DESC="Get the maximum voltage of the main battery">
    <SWIE REG="R0">&amp;1802</SWIE>
    <SWIO REG="R1">main battery maximum voltage</SWIO>
    <SWIU>
        This call reads the maximum voltage of the main battery in milli-volts.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1810" NUM="520C3" DESC="Get the status of the backup battery">
    <SWIE REG="R0">&amp;1810</SWIE>
    <SWIO REG="R1">backup battery status</SWIO>
    <SWIU>
        This call reads the status of the backup battery. The possible values are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Value</TH><TH>Meaning</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD>dead</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>1</TD><TD>very low</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>2</TD><TD>low</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>3</TD><TD>good</TD></TR>
        </TABLE>                     
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1811" NUM="520C3" DESC="Get the voltage of the backup battery">
    <SWIE REG="R0">&amp;1811</SWIE>
    <SWIO REG="R1">backup battery voltage</SWIO>
    <SWIU>
        This call reads the voltage of the backup battery in milli-volts.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1812" NUM="520C3" DESC="Get the maximum voltage of the backup battery">
    <SWIE REG="R0">&amp;1812</SWIE>
    <SWIO REG="R1">backup battery maximum voltage</SWIO>
    <SWIU>
        This call reads the maximum voltage of the backup battery in milli-volts.
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;1820" NUM="520C3" DESC="Get the status of external power">
    <SWIE REG="R0">&amp;1820</SWIE>
    <SWIO REG="R1">status of external power (0 not present)</SWIO>
    <SWIU>
        This call reads the status of external power.   
        <P>
        This value is only available for <EPOC> devices.
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;2000" NUM="520C3" DESC="Get the status of the printer mirror">
    <SWIE REG="R0">&amp;2000</SWIE>
    <SWIO REG="R1">printer mirror status</SWIO>
    <SWIU>
        This call reads the status of the printer mirror. The bits of the result have the following meaning:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Bit</TH><TH>Meaning when set</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD ALIGN=LEFT>printer mirror active</TD></TR>
        </TABLE>
    </SWIU>
</SWI>

<HR>

<SWI NAME="PsiFS_Get &amp;2001" NUM="520C3" DESC="Get the name of the printer mirror destination">
    <SWIE REG="R0">&amp;2001</SWIE>
    <SWIE REG="R1" MORE>pointer to buffer to contain null terminated printer mirror destination</SWIE>
    <SWIE REG="R2" MORE>size of buffer</SWIE>
    <SWIO REG="R2">number of spare bytes in the buffer <B>including</B> the null terminator</SWIO>
    <SWIU>
        This call reads the name of the printer mirror destination.
    </SWIU>
</SWI>

</PAGE>
