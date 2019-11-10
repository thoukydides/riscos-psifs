<*
    File        : menu/confg.hsc
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

<PAGE TITLE="Menu Entries" SUBTITLE="Configure..." PARENT=":menu/index.html" KEYWORDS="Menu,Configuration,Block Driver,Serial Port,Printer Mirror,Backup,Filer Window,File Converters,*PsiFSEnable,*PsiFSDriver,Auto Baud Rate,Virtual Drive">

<HEADING>Configure...</HEADING>
<PARA>
Selecting the <MENL TITLE="Configure..."> menu entry opens the <PSIFS> configuration window. There are three buttons at the bottom of the window:
<UL>
    <LI><B>Save</B> - Save the displayed options for use during subsequent sessions. Note that this does not change the options for the current session.
    <LI><B>Cancel</B> - Close the window without applying any changes. Clicking with <MOUSE BUTTON=ADJUST> leaves the window open, but resets the options to their previous values.
    <LI><B>Set</B> - Apply the displayed changes. Clicking with <MOUSE BUTTON=ADJUST> applies the changes without closing the window.
</UL>
<P>
The rest of the window allows various aspects of <PSIFS> to be configured.
</PARA>

<HEADING>Serial block driver</HEADING>
<PARA>
The top section of the window allows the serial block driver to be configured for communication with a <SIBO> or <EPOC> device. The options are:
<UL>
    <LI><B>Driver name</B> - The name of the serial block driver to use. This should normally be <ARG>InternalPC</ARG> if the standard serial port is being used. Use the appropriate driver if extra serial ports are fitted to the computer.
    <LI><B>Port number</B> - The number of the serial port to use. For each block driver the port numbering starts from <ARG>0</ARG>.
    <LI><B>Baud rate</B> - The serial baud rate to use. This should normally be set to the <A HREF=":start/baud.html" TITLE="Baud Rate">highest baud rate</A> supported by both machines.
    <LI><B>Automatic</B> - Enable or disable automatic baud rate identification. This option may be useful if different <SIBO> or <EPOC> devices are used, although establishing a connection will take longer. This only affects operation of the remote link; the printer mirror always uses the specified baud rate.
    <LI><B>Other options</B> - Any other options required for the selected block driver. This should normally be left blank.
</UL>
These options may also be changed using the <CMDL CMD="PsiFSDriver" HREF=":command/drivr.html"> command.
<P>
The list of baud rates is normally restricted to those that are likely to be useful. It is possible to enable display of all available baud rates by adding the line <ARG>BlockdriverRestrictBaud=False</ARG> to the <PSIFS> configuration file (either <ARG>!Boot.Choices.PsiFS.Config</ARG> or <ARG>!PsiFS.Config.Config</ARG>, depending upon the computer's configuration).
</PARA>

<HEADING>Remote link options (EPOC devices only)</HEADING>
<PARA>
The second section of the window controls special behaviour related to the remote link when used with <EPOC> devices. These features have no effect with <SIBO> devices. The options are:
<UL>
    <LI><B>Synchronize clocks when remote link connected</B> - Enable or disable automatic synchronization of an <EPOC> device's clock to that of the <RISCOS> computer when the remote link is connected.
    <LI><B>Custom battery warning voltages</B> - Enable or disable use of user defined threshold voltages for detecting low batteries. If this option is not selected then the standard <EPOC> thresholds are used. The following levels are suggested for NiMH or NiCd batteries used in a <SERIES VER="5"> or similar:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TD></TD><TH NOWRAP>Dead</TH><TH NOWRAP>Very low</TH><TH NOWRAP>Low</TH></TR>
            <TR><TH NOWRAP>Main batteries</TH><TD ALIGN=CENTER>2.00</TD><TD ALIGN=CENTER>2.15</TD><TD ALIGN=CENTER>2.30</TD></TR>
            <TR><TH NOWRAP>Backup battery</TH><TD ALIGN=CENTER>2.00</TD><TD ALIGN=CENTER>2.50</TD><TD ALIGN=CENTER>3.00</TD></TR>
        </TABLE>
    <LI><B>Monitor batteries</B> - This controls whether a warning window is automatically opened if the batteries of the remote device are low:
        <UL>
            <LI><B>never</B> - monitoring disabled.
            <LI><B>when no external power</B> - monitoring disabled if remote device is operating on external power.
            <LI><B>always</B> - monitoring always active, even if external power is being used.
        </UL>
    <LI><B>Integrate with the RISC OS global clipboard</B> - enable or disable sharing of the <EPOC> and <RISCOS> clipboards. When selected this allows data copied from either an <EPOC> or <RISCOS> application to be pasted into application running in the other environment.
    <LI><B>Poll for changes</B> - Applications do not announce changes to the global clipboard under <RISCOS>, so <PSIFS> has to detect changes by polling the current clipboard owner. This option controls how this takes place:
        <UL>
            <LI><B>never</B> - no polling. The <EPOC> clipboard will only be updated when an application claims the clipboard. With most applications this only occurs if a different application previously had the clipboard.
            <LI><B>using clipboard size</B> - poll for changes to the size of the clipboard. This is the preferred option; it consumes only a small amount of processing time, but works with most applications.
            <LI><B>using clipboard content</B> - poll for changes to the content of the clipboard. This can slow down the desktop if large amounts of data are copied to the clipboard but is the most reliable option.
        </UL>
</UL>
</PARA>

<HEADING>Filer options</HEADING>
<PARA>
The third section of the window controls the behaviour of the desktop filer. The options are:
<UL>
    <LI><B>Show drive icon</B> - This controls which drive icons are shown on the iconbar:
        <UL>
            <LI><B>for non-ROM drives</B> - an icon is shown for each non-ROM drive that is accessible.
            <LI><B>for every drive</B> - an icon is shown for all accessible drives.
            <LI><B>for virtual drive only</B> - a single icon is shown for a special virtual drive that has all of the accessible drives represented by subdirectories.
        </UL>
    <LI><B>Open filer windows</B> - This controls when filer windows are opened:
        <UL>
            <LI><B>manually only</B> - filer windows are never opened automatically.
            <LI><B>when remote link connected</B> - filer windows are automatically opened for all available drives when a remote link is connected.
        </UL>
    <LI><B>Close filer windows</B> - This controls when filer windows are closed:
        <UL>
            <LI><B>manually only</B> - filer windows are never closed automatically.
            <LI><B>when file system killed</B> - filer windows are only closed automatically when the <PSIFS> file system is killed at the same time as the desktop filer.
            <LI><B>when remote link disconnected</B> - filer windows are closed whenever a drive becomes unavailable, usually when a remote link is disconnected.
        </UL>
</UL>
</PARA>

<HEADING>Backup options</HEADING>
<PARA>
The fourth section of the window allows backup operations to be configured. The options are:
<UL>
    <LI><B>Directory containing backup files</B> - The path of the directory to contain all backup files. Sub-directories are automatically created to contain the configuration and backup files for each disc. Set the path by dragging the directory icon and dropping it over a filer window, or dragging a directory icon from a filer window and dropping it over the arrow. Alternatively, any valid path may be typed directly into the icon. If the directory containing backup files is changed, then <PSIFS> will attempt to move any previous backup files when the changes are applied.
    <LI><B>Open backup options window for unrecognised discs</B> - Enable or disable the automatic opening of the backup options window when connected to unrecognised discs. This makes it easy to set the initial backup options for each disc.
</UL>
<P>
Options specific to a single disc are set by selecting <MENL TITLE="Backup..." HREF=":menu/backu.html"> from the menu of the appropriate <A HREF=":icon/drive.html" TITLE="SIBO or EPOC Drive Icon"><SIBO> or <EPOC> drive icon</A>.
</PARA>

<HEADING>Print job options (EPOC devices only)</HEADING>
<PARA>
The fifth section section of the window allows the print job options for <EPOC> devices to be configured. These features have no effect with <SIBO> devices of if the printer mirror is used. The options are:
<UL>
    <LI><B>When job starts</B> - The default action when a print job starts:
        <UL>
            <LI><ARG>open control window</ARG> - a window is opened displaying the number of pages received, and allowing the print job to be previewed or printed.
            <LI><ARG>display preview</ARG> - a preview of the print job is displayed.
            <LI><ARG>start printing</ARG> - the print job is printed automatically.
        </UL>
    <LI><B>Start printing</B> - When should printing begin after a print job starts:
        <UL>
            <LI><ARG>after first page received</ARG> - start printing as soon as possible. This may result in the print job being split, which could be undesirable with network or shared printers.
            <LI><ARG>when print job complete</ARG> - wait for the whole job to be received before starting to print. This ensures that the print job will not be split, but it may take longer for the print job to finish.
        </UL>
    <LI><B>Preview scale</B> - The default magnification factor for the print job preview.
    <LI><B>Anti-alias</B> - Should vector graphics in the print job be anti-aliased. This uses the ArtWorks rendering modules to improve the quality of the on-screen image by using grey edge pixels to achieve smoother graphics.
</UL>
</PARA>

<HEADING>Printer mirror</HEADING>
<PARA>
The sixth section of the window allows the printer mirror to be configured. The options are:
<UL>
    <LI><B>Default destination</B> - The default destination for data received over the serial link in printer mirror mode:
        <UL>
            <LI><ARG>parallel:</ARG> - the parallel port.
            <LI><ARG>fastparallel:</ARG> - the parallel port in fast Centronics mode.
            <LI><ARG>serial:</ARG> - the internal serial port (only available if <PSIFS> is configured to use a different port).
            <LI><ARG>netprint:</ARG> - a network printer.
            <LI><ARG>null:</ARG> - the null device (all data is discarded).
            <LI><ARG>printer:</ARG> - the active printer (selected using <ARG>!Printers</ARG>).
            <LI><ARG>printer#parallel:</ARG> - a printer connected to the parallel port.
            <LI><ARG>printer#serial:</ARG> - a printer connected to the serial port.
            <LI><ARG>printer#user:</ARG> - a user printer device.
            <LI><ARG>printer#null:</ARG> - a null printer device (all data is discarded).
            <LI><ARG>&lt;wimp$scrapdir&gt;.printout</ARG> - a temporary file (useful for trouble shooting).
            <LI><ARG>pipe:$.printout</ARG> - a pipe (useful for trouble shooting).
        </UL>
        Alternatively, any valid filename may be typed directly into the icon. This setting is used when the <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A> is clicked on with <MOUSE BUTTON=ADJUST> or <MENL TITLE="Printer mirror" HREF=":menu/print.html"> is selected from the iconbar icon menu.
</UL>
<P>
The <CMDL CMD="PsiFSEnable" HREF=":command/enabl.html"> command can also be used to start the printer mirror and set the destination filename.
</PARA>

<HEADING>Idle behaviour</HEADING>
<PARA>
The seventh section of the window allows configuration of how <PSIFS> behaves when a remote link or printer mirror connection is not being actively used. The options are:
<UL>
    <LI><B>Disconnect remote link</B> - This controls whether the remote link will be automatically disconnected after the specified length of idle time.
    <LI><B>Disconnect printer mirror</B> - This controls whether the printer mirror will be automatically disconnected after the specified length of idle time.
    <LI><B>Reduce background activity when idle</B> - This controls whether background operations, such as checking for changes to files, should be throttled back when the remote link is not being actively used. This is likely to make the remote machine more responsive, but has the disadvantage that <PSIFS> will take longer to detect any changes.
</UL>
</PARA>

<HEADING>File conversion options</HEADING>
<PARA>
The final section of the window allows the operation of file conversion to be configured. The options are:
<UL>
    <LI><B>Intercept run</B> - This controls when files being run by double-clicking are intercepted to allow a conversion to be performed:
        <UL>
            <LI><B>never</B> - file runs are never intercepted.
            <LI><B>if not claimed</B> - file runs are only intercepted if not claimed by any active application.
            <LI><B>always</B> - file runs are always intercepted.
        </UL>
        If the <B>Automatic</B> option is also selected then a conversion is automatically performed without prompting.
    <LI><B>Intercept load</B> - This controls when files being loaded by dragging are intercepted to allow a conversion to be performed:
        <UL>
            <LI><B>never</B> - file loads are never intercepted.
            <LI><B>always</B> - file loads are always intercepted.
        </UL>
        If the <B>Automatic</B> option is also selected then a conversion is automatically performed without prompting.
    <LI><B>Intercept save</B> - This controls when files being saved or transferred between applications by dragging are intercepted to allow a conversion to be performed:
        <UL>
            <LI><B>never</B> - file saves are never intercepted.
            <LI><B>to filer only</B> - file saves are only intercepted if they are to the filer.
            <LI><B>always</B> - files saves and transfers between applications are always intercepted.
        </UL>
        If the <B>Automatic</B> option is also selected then a conversion is automatically performed without prompting.
</UL>
<P>
These options may be overridden at any time by pressing <ARG>Left&nbsp;Alt</ARG> to force an intercept and allow a converter to be selected or <ARG>Right&nbsp;Alt</ARG> to prevent an intercept. Unless an intercept if forced by pressing <ARG>Left&nbsp;Alt</ARG>, the intercept will only occur if it is both enabled within the configuration and supported by at least one of the installed file converters.
<P>
The list of file format converters for intercepted files is normally restricted to those that do not produce a directory. It is possible to enable display of all appropriate file format converters by adding the line <ARG>InterceptFileDirectory=True</ARG> to the <PSIFS> configuration file (either <ARG>!Boot.Choices.PsiFS.Config</ARG> or <ARG>!PsiFS.Config.Config</ARG>, depending upon the computer's configuration).
</PARA>

</PAGE>
