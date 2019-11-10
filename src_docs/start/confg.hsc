<*
    File        : start/confg.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Configuration" PARENT=":start/index.html" KEYWORDS="Configuration,Baud Rate,Block Driver,Serial Port,Backup,Printer Mirror,Printing,Connect,Disconnect,File Converters">

<HEADING>Configuration</HEADING>
<PARA>
The procedure for configuring <PSIFS> depends on whether it is started within the desktop or from the command line.
<P>
If <PSIFS> is loaded within the desktop, for example by double-clicking on the <ARG>!PsiFS</ARG> icon, then the configuration should be modified using the desktop filer. Ensure that the <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A> is visible, and then select <MENL TITLE="Configure..." HREF=":menu/confg.html"> from the iconbar icon menu to open the configuration window. Make any required changes to the configuration, and then click <ARG>Save</ARG> to preserve the changes for future sessions, followed by <ARG>Set</ARG> to apply the changes for the current session.
<P>
If <PSIFS> is loaded outside the desktop or from a command line, then the configuration is taken from the <ARG>!PsiFS.Configure</ARG> file. This file also contains the mappings used to convert <SIBO> file extensions and <EPOC> UIDs to <RISCOS> file types. See the documentation for the <CMDL CMD="PsiFSDriver" HREF=":command/drivr.html"> and <CMDL CMD="PsiFSMap" HREF=":command/map.html"> commands for more details.
</PARA>

<HEADING>Serial Block Driver</HEADING>
<PARA>
The most important configuration options are for the serial block driver used to communicate with the remote <SIBO> or <EPOC> device. The default settings should be adequate if only the remote link and internal serial port are being used.
<P>
The first option to select is the <B>Driver name</B>. This should normally be set to <ARG>InternalPC</ARG> if the internal serial port is being used, <ARG>SP_DualPC</ARG> if either an <ATOMWIDE> or <SERIALPORT> serial port card is being used, or <ARG>IIDualPC</ARG> if an <INTELLIGENT> serial port card is being used. Other serial port cards may require different serial block drivers.
<P>
The second option to configure is the <B>Port number</B>. The first port of each block driver is numbered <ARG>0</ARG>. The internal serial port it is always number <ARG>0</ARG>, and ports on <ATOMWIDE> or <SERIALPORT> triple serial port cards are numbered <ARG>0</ARG>, <ARG>1</ARG> or <ARG>2</ARG>. Do not attempt to select a port for which no hardware is present.
<P>
The third option to adjust is the <B>Baud rate</B>. This should normally be set to the <A HREF=":start/baud.html" TITLE="Baud Rate">highest baud rate</A> supported by both machines. The same baud rate must be selected on the <SIBO> or <EPOC> device.
<P>
If the <B>Automatic</B> option is selected then <PSIFS> will automatically identify the required baud rate for the remote link, but will take longer to establish a connection. This option can be useful if different <SIBO> or <EPOC> devices are used with a single <RISCOS> computer. The printer mirror ignores this setting and always uses the configured baud rate.
<P>
Finally, the <B>Other options</B> icon should normally be left blank. This feature allows extra options to be passed to serial block drivers that support them.
</PARA>

<HEADING>Remote Link Options</HEADING>
<PARA>
Many options specific to connections to <EPOC> devices can be configured.
<P>
The feature that is likely to be of most interest is the integration of the <EPOC> clipboard with the <RISCOS> global clipboard. To enable this feature ensure that the <B>Integrate with the RISC OS global clipboard</B> option is selected. The <B>Poll for changes</B> option should be left as <B>using clipboard size only</B> initially; this can be modified later if changes to the clipboard are missed or the desktop runs too slowly.
</PARA>

<HEADING>Backup Options</HEADING>
<PARA>
Backups are stored by default in a directory called <ARG>Backups</ARG> in the same directory as <ARG>!PsiFS</ARG>.
<P>
An alternative directory may be used by dragging the directory icon and dropping it over a filer window, or dragging a directory icon from a filer window and dropping it over the arrow. Alternatively, any valid path may be typed directly into the <B>Directory containing backup files</B> icon.
<P>
Options specific to a single disc are set by selecting <MENL TITLE="Backup..." HREF=":menu/backu.html"> from the menu of the appropriate <A HREF=":icon/drive.html" TITLE="SIBO or EPOC Drive Icon"><SIBO> or <EPOC> drive icon</A>. If the <B>Open backup options window for unrecognised discs</B> option is selected then <PSIFS> will automatically open the backup options window when connected to unrecognised discs.
</PARA>

<HEADING>Print Job Options</HEADING>
<PARA>
<EPOC> devices can print via the remote link using <RISCOS> printer drivers. The action taken when a print job starts can be selected, which by default is to open a window displaying the status of the print job, allowing the job to be previewed or printed. The <B>When job starts</B> option allows this default action to be changed.
<P>
Normally, <PSIFS> will start printing the first page of the print job as soon as possible. This can result in the print job completing quicker but may result in the job being split, which could cause problems with network or shared printers. The <B>Start printing</B> option can be used to force <PSIFS> to wait for the whole print job to be received before starting to print.
</PARA>

<HEADING>Printer Mirror</HEADING>
<PARA>
The default destination for the printer mirror is <ARG>printer:</ARG>. This results in output being sent to the active <RISCOS> printer, which may be changed using <ARG>!Printers</ARG>.
<P>
It is possible to redirect output to any other <RISCOS> file or device. The pop-up menu to the right of the <B>Default destination</B> icon provides a list of useful alternatives. Alternatively, any valid filename may be typed directly into the icon.
<P>
The configured destination is used when the <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A> is clicked on with <MOUSE BUTTON=ADJUST> or <MENL TITLE="Printer mirror" HREF=":menu/print.html"> is selected from the iconbar icon menu. However, it is possible to select an alternative destination from the <MENL TITLE="Printer mirror" HREF=":menu/print.html"> sub-menu.
</PARA>

<HEADING>File Conversion Options</HEADING>
<PARA>
By default files are only intercepted to allow conversions to be performed if the <ARG>Left&nbsp;Alt</ARG> key is pressed.
<P>
It is possible to enable different types of intercept to occur automatically for supported file types. The pop-up menus to the right of <B>Intercept run</B>, <B>Intercept load</B> and <B>Intercept save</B> allow the various options to be selected. If the <B>Automatic</B> options are selected then the file will be converted without prompting for the conversion to perform.
</PARA>

</PAGE>
