<*
    File        : start/index.hsc
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

<PAGE TITLE="Getting Started" PARENT=":index.html" KEYWORDS="Remote Link,Printer Mirror,Printing,Configuration,Connect,Disconnect,Backup,Restore">

<HEADING>Getting Started</HEADING>
<PARA>
<PSIFS> should be loaded by double-clicking on the <ARG>!PsiFS</ARG> icon in a filer window. This will add the <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A> to the left-hand side of the iconbar:
<CENTER><A HREF=":icon/idle.html" TITLE="Idle Icon"><IMG SRC=":graphics/idle.gif" ALT="" BORDER=0></A></CENTER>
<P>
If the standard internal serial port is being used, then <PSIFS> can be used in remote link mode without changing the default configuration. However, to use the printer mirror mode, or if a third-party serial port card is being used, then it will be necessary to <A HREF=":start/confg.html" TITLE="Configuring PsiFS">configure</A> the serial block driver first.
<P>
Connect a suitable serial cable between the two machines. The <SERIES VER="3/3a"> and <PB VER="I"> require a special <3LINK> or <ALINK> cable; other machines use a simple serial cable like that supplied with the <SERIES VER="5"> or <OSARIS>. Note that only the <3LINK> will work unmodified on early <ARC> computers.
</PARA>

<HEADING>Remote Link</HEADING>
<PARA>
The main use of <PSIFS> is to access files stored on a <SIBO> or <EPOC> device as if they were on a local drive. A remote link can be <A HREF=":start/conct.html" TITLE="Connecting">connected</A> or <A HREF=":start/disco.html" TITLE="Disconnecting">disconnected</A> as required by simply clicking on the <A HREF=":icon/index.html" TITLE="Iconbar Icons">icon</A>. The icon will change to reflect the current status.
<P>
The remote link can be used to perform a manual or automatic <A HREF=":start/backu.html" TITLE="Backup">backup</A> of any available drive. <A HREF=":start/restr.html" TITLE="Restoring Backups">Restoring backups</A> is slightly more complicated, but should rarely be required.
<P>
<EPOC> devices can also use the remote link to <A HREF=":start/print.html" TITLE="Printing via the Remote Link">print</A> via the <RISCOS> printer drivers using the <ARG>Printer&nbsp;via&nbsp;PC</ARG> option.
</PARA>

<HEADING>Printer Mirror</HEADING>
<PARA>
As an alternative to using the remote link, <PSIFS> can also be used to print via the <RISCOS> computer using the <SIBO> or <EPOC> device's own printer drivers. The printer mirror can also be <A HREF=":start/enabl.html" TITLE="Enabling the Printer Mirror">enabled</A> or <A HREF=":start/disbl.html" TITLE="Disabling the Printer Mirror">disabled</A> by simply clicking on the <A HREF=":icon/index.html" TITLE="Iconbar Icons">icon</A>. The icon will change to reflect the current status.
<P>
<EPOC> users are advised to use the <ARG>Printer&nbsp;via&nbsp;PC</ARG> driver to <A HREF=":start/print.html" TITLE="Printing via the Remote Link">print</A> via the remote link in preference to the printer mirror.
</PARA>

<HEADING>File Format Converters</HEADING>
<PARA>
<SIBO> and <EPOC> devices use different file formats to common <RISCOS> applications. Hence, it is necessary to convert between different file formats to enable data to be usefully shared between the platforms.
<P>
<PSIFS> allows third party tools to be integrated within a simple and consistent interface. These need to be <A HREF=":start/seen.html" TITLE="Installing File Format Converters">seen</A> before they can be used to perform <A HREF=":start/conv.html" TITLE="Performing File Format Conversions">conversions</A> or <A HREF=":start/sis.html" TITLE="Install EPOC SIS Files">install <EPOC> SIS files</A>.
</PARA>

<HEADING>Clipboard</HEADING>
<PARA>
<PSIFS> allows the clipboard of <EPOC> devices to be integrated into <RISCOS>'s global clipboard. This makes it possible to copy data from an application running on either device, and then to paste the data into an application on the other device. Third party file format converters will be used if available.
<P>
Once the clipboard support has been enabled in the <A HREF=":start/confg.html" TITLE="Configuring PsiFS">configuration</A> it is possible to copy and paste between applications whenever there is an active connection.
</PARA>

</PAGE>
