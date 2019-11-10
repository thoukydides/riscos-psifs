<*
    File        : vers.hsc
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

<PAGE TITLE="Version History" PARENT=":index.html" KEYWORDS="Version,History,Bugs,Problems" NORULE>

<REL>
    <RELI VER="1.60" DATE="19-Sep-02" RELEASE>
        <B>Seventh official release version.</B>
        <BR>
        Added ability to reload raw print job files.
        <BR>
        Print jobs can be rendered with references to the source data.
    </RELI>
    
    <RELI VER="1.53" DATE="26-Aug-02">
        Improved handling of uncommon print primitives.
        <BR>
        Corrected format of frame used to disconnect from remote servers.
        <BR>
        Updated link for <ARG>Web site</ARG> buttons.
    </RELI>
    
    <RELI VER="1.52" DATE="21-Mar-02">
        Improved error handling during printing.
    </RELI>
    
    <RELI VER="1.51" DATE="17-Mar-02">
        Added experimental remote printing server support.
        <BR>
        Improved clean-up when filer quit.
    </RELI>
    
    <RELI VER="1.50" DATE="21-Jul-01">
        <B>Sixth official release version.</B>
        <BR>
        Disabled experimental support for <EPOC> based mobile phones.
    </RELI>
    
    <RELI VER="1.44" DATE="11-Jul-01">
        Multiple frames now transmitted before waiting for acknowledgement with <EPOC> devices.
        <BR>
        Reduced retry interval before connection established to speed up automatic baud rate identification.
        <BR>
        Clipboard server only started after normal connection complete.
        <BR>
        Suppressed negative timing estimates near completion of asynchronous operations.
    </RELI>
    
    <RELI VER="1.43" DATE="08-Jul-01">
        More efficient background operations, resulting in faster incremental backups.
        <BR>
        Added automatic disconnection after a configurable period of inactivity.
        <BR>
        The device claim protocol is now supported for the serial port, but not for the printer mirror destination.
        <BR>
        Corrected calculation of <EPOC> UID checksums.
        <BR>
        Corrected decoding of requisite names in SIS files with multiple language variants.
        <BR>
        Very experimental support for <EPOC> based mobile phones.
        <BR>
        Version 1.02 of the <ARG>SerialBuffer</ARG> module included.
    </RELI>
    
    <RELI VER="1.42" DATE="28-Jun-01">
        Corrected re-transmission timeout for data frame received while waiting for acknowledgement.
        <BR>
        Closing NCP channels no longer loses more recent channels.
        <BR>
        Failure to start the remote clipboard server is handled properly.
        <BR>
        The remote clipboard server is automatically downloaded to <EPOC> devices if not already present.
    </RELI>
    
    <RELI VER="1.41" DATE="26-Jun-01">
        Added preliminary remote clipboard support.
        <BR>
        Corrected poll word handling within the filer.
        <BR>
        Modified quoting of file redirection within aliases to avoid clash with the <ARG>SquigglyPipes</ARG> module.
        <BR>
        Improved detection of SIS file installations in progress.
        <BR>
        Improved file name handling for converters.
    </RELI>
    
    <RELI VER="1.40" DATE="04-Mar-01">
        <B>Fifth official release version.</B>
        <BR>
        The SIS file installer now sets the file attributes sensibly.
        <BR>
        Excessively long lines of owner information are now accepted.
        <BR>
        BASIC files distributed in textual form to allow easier manipulation by tools.
    </RELI>
    
    <RELI VER="1.32" DATE="31-Aug-00">
        Added configurable button text and interactive help for file format converter windows.
        <BR>
        Installation of a SIS file can now be started by double-clicking on the file.
        <BR>
        The SIS file Installer no longer replaces any ROM version of the <ARG>Add/remove</ARG> software.
    </RELI>
    
    <RELI VER="1.31" DATE="22-Aug-00">
        Support added for installing SIS files on <EPOC> devices using the Add/remove control panel.
        <BR>
        WIMP message added to allow third party software to use asynchronous operations easily.
        <BR>
        Action windows are not now automatically closed if an error occurred.
        <BR>
        <ARG>SWP</ARG> no longer used to implement semaphores, allowing use on ARM2 processors.
        <BR>
        Name disc option disabled for virtual drives.
        <BR>
        Backup option not disabled for virtual drives.
        <BR>
        File transfer intercepts disabled if <ARG>Shift</ARG> pressed.
    </RELI>
    
    <RELI VER="1.30" DATE="04-Jun-00">
        <B>Fourth official release version.</B>
        <BR>
        <B>Released under version 2 of the GNU General Public License.</B>
        <BR>
        Rebuilt using OSLib 6.01 and CathLibCPP issue 2.
    </RELI>
    
    <RELI VER="1.26" DATE="29-May-00">
        Improved help text for *commands after using <ARG>cmunge</ARG> instead of <ARG>cmhg</ARG>.
        <BR>
        File format converters can supply windows to allow additional options to be selected.
        <BR>
        File format converters can optionally be supplied with the original leafname.
        <BR>
        File format converters can specify the <EPOC> file UIDs or wildcarded names to match.
        <BR>
        Corrected premature deletion of files captured during file transfer intercepts.
        <BR>
        Corrected processing of non-absolute filenames in <EPOC> SIS installation files.
        <BR>
        File format converter for <EPOC> SIS files automatically produces residual SIS files.
        <BR>
        All file transfer intercepts now claim the input focus if they open a window.
    </RELI>
    
    <RELI VER="1.25" DATE="09-May-00">
        Made protocol handling more flexible to cope with <SERIES VER="3"> machines.
        <BR>
        Added support for custom battery warning voltages.
        <BR>
        Added file type and icon sprites for <EPOC> SIS installation files.
    </RELI>
    
    <RELI VER="1.24" DATE="30-Apr-00">
        Added a command to extract files and information from <EPOC> SIS installation files.
        <BR>
        Added a simple file format converter interface for the <PSIFS> module commands.
        <BR>
        Added <RISCOS VER="4"> style sprites.
        <BR>
        Corrected operation of Quiet button during backup.
        <BR>
        File format converters that produce directories are hidden for file transfer intercepts.
        <BR>
        Naming of temporary scrap files made more friendly for file format converters.
        <BR>
        Improved syntax of command to extract contents of <ARG>tar</ARG> files.
        <BR>
        Incorrect display of <SIBO> operating system version suppressed.
    </RELI>
    
    <RELI VER="1.23" DATE="08-Mar-00">
        Corrected sizes of wimp messages sent by file transfer intercepts again.
        <BR>
        Unrecognised file types ignored while parsing file format converter configuration files.
    </RELI>
    
    <RELI VER="1.22" DATE="05-Mar-00">
        Clicking on the iconbar icon with <MOUSE BUTTON=SELECT> no longer disables any link.
        <BR>
        Added drag and drop support for saving the result of a conversion.
        <BR>
        Rationalised handling of errors produced by third party file format converters.
        <BR>
        Error handling during file transfer intercepts corrected.
        <BR>
        Added delayed deletion of the results from file format conversions.
        <BR>
        Added support for stand-alone file format converters.
        <BR>
        Corrected sizes of wimp messages sent by file transfer intercepts.
        <BR>
        Corrected handling of backups with deleted directories.
        <BR>
        Shorter file names passed as arguments to file format converters.
    </RELI>
    
    <RELI VER="1.21" DATE="20-Feb-00">
        Preliminary support added for third party file format converters.
    </RELI>
    
    <RELI VER="1.20" DATE="11-Feb-00">
        <B>Third official release version.</B>
        <BR>
        Corrected incremental backup behaviour.
        <BR>
        Prevented module being killed if clients are registered.
        <BR>
        Added SWIs to allow file transfers within the desktop to be intercepted.
    </RELI>
    
    <RELI VER="1.15" DATE="04-Feb-00">
        Sophisticated selection of backup options and files for specific discs.
        <BR>
        Made button to open filer window for backup files more accessible.
        <BR>
        Enabled descriptive names to be entered for backups.
        <BR>
        Added battery status monitoring to the filer.
        <BR>
        Added display of information about the connected device to the filer.
        <BR>
        Added a new SWI to allow discs to be renamed without FileSwitch imposing a ten character limit.
        <BR>
        Skipping over objects in <ARG>tar</ARG> files more efficient.
        <BR>
        Hard spaces supported in disc names.
    </RELI>
    
    <RELI VER="1.14" DATE="23-Jan-00">
        Corrected initial update of backup options windows.
        <BR>
        Corrected closing of <SIBO> and <EPOC> files when backup operations aborted.
        <BR>
        Added collection of protocol error rate statistics.
        <BR>
        Background operations on <ARG>tar</ARG> files dynamically adjust buffer sizes to keep processor usage uniform.
        <BR>
        Added a help option to the iconbar icon menus.
        <BR>
        Added option to synchronize clock of <EPOC> devices on connection.
        <BR>
        Added interactive help to the buttons in asynchronous remote operation windows.
        <BR>
        Further improved estimates of remaining time for asynchronous remote operations.
        <BR>
        Documentation improved.
    </RELI>
    
    <RELI VER="1.13" DATE="15-Jan-00">
        Added a full user interface to control backup operations.
        <BR>
        Improved efficiency of <ARG>tar</ARG> file operations.
        <BR>
        Made <ARG>tar</ARG> file operations non-atomic to prevent the machine becoming unresponsive for long periods.
        <BR>
        Reduced probability of remote link disconnecting during backup operations.
        <BR>
        Made display of file names during backup operations consistent with other asynchronous remote operations.
        <BR>
        More accurate status indication during backup operations.
        <BR>
        Improved estimates of remaining time for asynchronous remote operations.
        <BR>
        Added icon sprites for <ARG>tar</ARG> files.
        <BR>
        Corrected positioning of the asynchronous remote operations window if an error produced immediately.
        <BR>
        Made filer more <NAME>Style&nbsp;Guide</NAME> compliant.
    </RELI>
    
    <RELI VER="1.12" DATE="04-Jan-00">
        Corrected the maximum number of characters that are valid in a disc name within the filer.
        <BR>
        Increased the minimum toolbox module version numbers to the latest available for download, to avoid bug with iconbar icon spacings.
        <BR>
        Corrected accesses to <SIBO> files larger than 64kB.
        <BR>
        Made shutdown of tasks consistent on <SIBO> and <EPOC> devices.
        <BR>
        Improved error reporting and recovery for restarting tasks.
        <BR>
        Dynamic area used for memory allocation if possible.
        <BR>
        Reduced memory consumption when link is idle.
        <BR>
        Implemented asynchronous remote operation to perform an incremental backup of a single directory.
        <BR>
        Improved estimates of timings for asynchronous remote operations.
    </RELI>
    
    <RELI VER="1.11" DATE="27-Oct-99">
        Corrected virtual drive handling for invalid drive names.
        <BR>
        Added asynchronous remote operations for reading and writing files.
        <BR>
        Started adding support for performing incremental backups.
        <BR>
        Implemented a generic window for managing asynchronous remote operations.
        <BR>
        Suppressed errors generating while closing the printer mirror destination.
        <BR>
        Automatically patches version 0.99 of the <ARG>MirrorSupport</ARG> module to ignore <PSIFS>.
    </RELI>
    
    <RELI VER="1.10" DATE="12-Jun-99">
        <B>Second official release version.</B>
        <BR>
        Disabled the backup option on the virtual drive's iconbar menu.
    </RELI>
    
    <RELI VER="1.04" DATE="30-May-99">
        Added a virtual drive that represents each of the physical drives as a subdirectory, allowing a single icon to provide access to all drives.
        <BR>
        Reduced the number of serial stop bits from 2 to 1, and amended the documentation concerning the serial port configuration for the printer mirror.
        <BR>
        Some errors ignored when attempting to restart applications, allowing OPL tasks to be skipped.
        <BR>
        Corrected the decoding of replies from <EPOC16> devices during a task shutdown.
        <BR>
        Changed references to <EPOC16> and <EPOC32> to <SIBO> and <EPOC> respectively within the software and documentation. The earlier version history entries have not been modified.
        <BR>
        Patched round bug in OSLib SWI veneer to allow preservation of the block driver configuration.
        <BR>
        Added mappings for some file name extensions longer than three characters.
    </RELI>
    
    <RELI VER="1.03" DATE="13-May-99">
        Added options to automatically open and close filer windows when the remote link is connected and disconnected.
        <BR>
        Added option to suppress display of ROM drives.
        <BR>
        Prevented unnecessary deletion and recreation of iconbar icons while a connection is being established.
        <BR>
        Configuration options not reset if the <ARG>Configure...</ARG> menu option is selected with the window already open.
        <BR>
        Rearranged the desktop filer menus.
        <BR>
        The printer mirror destination is now selectable from the desktop filer.
        <BR>
        Number of bytes received displayed under the printer mirror icon.
        <BR>
        Lowered minimum toolbox module version numbers to match those in the original release of the new <ARG>!Boot</ARG> structure.
        <BR>
        Restricted the range of baud rates offered to those likely to be useful.
        <BR>
        Improved configuration file search algorithm.
        <BR>
        Improved scrap file and directory determination.
        <BR>
        The <ARG>MimeMap</ARG> module may optionally be used (if available) to map any unrecognised file extensions to file types.
        <BR>
        Added extra default file type mappings.
        <BR>
        Removed the <ARG>Internal</ARG> and added the <ARG>SP_DualPC</ARG> serial block drivers.
        <BR>
        Better documentation, especially with regard to using the printer mirror for the first time.
        <BR>
        The desktop save protocol is supported.
        <BR>
        Started implementing support for asynchronous remote operations.
        <BR>
        Experimental shutdown and restart of <EPOC> applications in preparation for backup option.
        <BR>
        Simplified serial block driver handling.
    </RELI>
    
    <RELI VER="1.02" DATE="24-Apr-99">
        Automatic baud rate identification added.
        <BR>
        Corrected canonicalisation of paths with zero length disc names to allow unnamed SSDs to be accessed.
        <BR>
        The printer mirror attempts to set the file type of output files to <ARG>Printout</ARG> (&amp;FF4).
        <BR>
        Changed the default printer mirror destination from <ARG>printer#parallel:</ARG> to <ARG>printer:</ARG>.
        <BR>
        The module now counts the number of characters received and transmitted over the serial link, and makes the information available via the SWI interface.
    </RELI>
    
    <RELI VER="1.01" DATE="09-Apr-99">
        Added timeout to serial port handling under CallBacks to prevent the machine locking-up if the serial port misbehaves.
        <BR>
        Automatically patches version 4.02 of the <ARG>BlackHole</ARG> module to ignore <PSIFS>, thereby preventing problems if Singularity mode is selected.
    </RELI>
    
    <RELI VER="1.00" DATE="27-Mar-99">
        <B>First official release version.</B>
        <BR>
        Minor changes to the help text only.
    </RELI>
    
    <RELI VER="0.14" DATE="20-Mar-99">
        Added configuration support to the desktop filer.
        <BR>
        Removed the configuration option from menus when the serial block driver is active.
        <BR>
        Processor usage of desktop filer reduced by filtering out null polls.
        <BR>
        Unused key presses are now passed to other tasks.
    </RELI>
    
    <RELI VER="0.13" DATE="21-Feb-99">
        Added SWIs to support desktop filer and front-end.
        <BR>
        Basic desktop filer included.
        <BR>
        Rearranged the start-up Obey files to improve loading in different contexts.
        <BR>
        Major overhaul of the documentation.
        <BR>
        Corrected behaviour of file system if a remote drive has a single character name.
    </RELI>
    
    <RELI VER="0.12" DATE="05-Jan-99">
        Reconnection to remote servers following a brief disconnection made more reliable.
        <BR>
        More recent version of the remote command server used.
        <BR>
        Removed special development version *commands.
        <BR>
        Added the ability to mirror data received from the serial port to the parallel port.
        <BR>
        Completely new file and application icons following <PSION>'s refusal to allow use of those from <NAME>PsiWin</NAME>.
        <BR>
        Configuration of the serial block driver separated from enabling and disabling the link.
        <BR>
        Shortened help text for *commands to allow the SWI handler veneer to be included within <ARG>cmhg</ARG>'s limit of 4kB.
    </RELI>
    
    <RELI VER="0.11" DATE="29-Nov-98">
        The configured <ARG>Truncate</ARG> option is fully supported.
        <BR>
        Removed alternative names for the remote command server.
        <BR>
        The remote command server is automatically downloaded to <EPOC16> devices if not already present.
    </RELI>
    
    <RELI VER="0.10" DATE="26-Nov-98">
        Corrected conversion of <RISCOS> to <EPOC16> root directory paths.
        <BR>
        Replies to load server commands checked more carefully for validity.
    </RELI>
    
    <RELI VER="0.09" DATE="25-Nov-98">
        &quot;Unused&quot; parts of the load server command are cleared to zeros.
        <BR>
        Alternative server names are tried in a different order.
    </RELI>
    
    <RELI VER="0.08" DATE="25-Nov-98">
        Write operations past the end of an open file now work correctly.
        <BR>
        Corrected update of sequential file pointer when a file is truncated.
        <BR>
        Improved the mapping of attributes to prevent <RISCOS> files with ordinary (<ARG>WR/</ARG>) attributes becoming hidden when copied to an <EPOC> device.
        <BR>
        Added support for all of the necessary filing system entry points.
        <BR>
        Error checking added to handle failed attempts to load custom servers.
        <BR>
        Alternative name added for the remote processing server.
    </RELI>
    
    <RELI VER="0.07" DATE="17-Nov-98">
        UpCalls are generated to update filer windows when changes are made to files or directories on the <EPOC> device.
        <BR>
        Interactive free space display now correctly updated.
    </RELI>
    
    <RELI VER="0.06" DATE="15-Nov-98">
        Directories with lots of entries are now read correctly.
        <BR>
        Files opened in read only mode have sharing enabled to allow simultaneous access by <EPOC> applications.
        <BR>
        The sequential file pointer of open files is only moved if necessary.
        <BR>
        Corrected the modes used to open files to allow ROM files to be read from an <EPOC32> machine.
        <BR>
        Reading of file UIDs made more efficient.
    </RELI>
    
    <RELI VER="0.05" DATE="14-Nov-98">
        Recognition of empty drives made more tolerant of strange <EPOC16> status codes.
        <BR>
        Errors ignored while converting disc names to drive letters.
    </RELI>
    
    <RELI VER="0.04" DATE="13-Nov-98">
        Background update of the directory cache is delayed following foreground operations to speed up subsequent commands.
        <BR>
        Delayed changing date and time stamps and attributes of open files until after the files have been closed.
        <BR>
        Load and execution addresses are preserved while files are kept open. In addition, values of <ARG>&amp;DEADDEAD</ARG> are treated specially to improve the appearance of interactive filer copies.
        <BR>
        Improved error checking of disc name to drive letter conversions.
    </RELI>
    
    <RELI VER="0.03" DATE="12-Nov-98">
        Cached drive and directory details are used to speed up most operations.
        <BR>
        Amended mapping of object attributes to prevent directories from being read only by default.
        <BR>
        Added <ARG>Drive</ARG>, <ARG>Free</ARG> and <ARG>NameDisc</ARG>/<ARG>NameDisk</ARG> commands.
        <BR>
        Included support for the Free module to allow interactive free space display.
        <BR>
        Corrected the detection of connection to <EPOC16> servers.
        <BR>
        Replaced <NAME>Jeroen Wessels</NAME>'s modified version of the <ARG>Serial&nbsp;Buffer</ARG> module with <NAME>David Pilling</NAME>'s original.
        <BR>
        Removed the <ARG>Internal</ARG> serial block driver from the standard distribution.
        <BR>
        Improved conversion of dates from <RISCOS> to <EPOC32> format.
        <BR>
        Delayed date and time stamping of open files until after the files have been closed.
    </RELI>
    
    <RELI VER="0.02" DATE="21-Oct-98">
        Corrected problem with creating and deleting directories on <EPOC32> devices.
        <BR>
        Details of the remote machine and drives are cached. However, the cached values are not yet used.
        <BR>
        The serial link no longer times out due to inactivity.
        <BR>
        Improved Obey files to allow operation from a command line.
    </RELI>
    
    <RELI VER="0.01" DATE="18-Oct-98">
        Improved the recognition of NCP version numbers to treat the <SERIES VER="3mx"> and <SERIES VER="3a"> as <EPOC16> machines.
        <BR>
        Reorganised Obey files and directory structure.
        <BR>
        Designed a logo and added a start-up banner.
    </RELI>
    
    <RELI VER="0.00" DATE="10-Oct-98">
        Original development version.
    </RELI>
</REL>

</PAGE>
