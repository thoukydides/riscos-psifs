<*
    File        : menu/backu.hsc
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

<PAGE TITLE="Menu Entries" SUBTITLE="Backup..." PARENT=":menu/index.html" KEYWORDS="Menu,Backup,Restore,Tar,!SparkFS,Configuration,Drive,Disc Name,Disk Name,*PsiFSTar,Action Windows">

<HEADING>Backup...</HEADING>
<PARA>
Selecting the <MENL TITLE="Backup..."> menu entry opens the <PSIFS> backup window. This allows a full or incremental <A HREF=":start/backu.html" TITLE="Backup">backup</A> to be performed of a single <SIBO> or <EPOC> disc.
<P>
The <PSIFS> backup feature uses the disc name and media unique identifier (if available) to identify each disc, maintain status and configuration details. Hence, it is recommended that each disc is given a unique name. <PSIFS> attempts to use other methods of identification to avoid problems with multiple <SIBO> or <EPOC> devices, but these are not always reliable, so default disc names like <ARG>Internal</ARG> should be changed before the backup options are set.
<P>
The top of the window shows the name of the backup. Below this is the drive containing the disc, and the time and date of the last backup of this disc. If the previous backup is incomplete, for example if it was aborted or an error occurred, then <ARG>(P)</ARG> is appended after the date and time. Finally, the current status or activity is displayed.
<P>
There are five buttons in the window:
<UL>
    <LI><B>Open</B> - Open a filer window for the directory containing the <A HREF="#files" TITLE="Backup Files">backup files</A>.
    <LI><B>Abort</B> - Abort the current backup operation for this disc. Clicking with <MOUSE BUTTON=ADJUST> leaves the window open.
    <LI><B>Backup</B> - Start a backup operation for this disc. Clicking with <MOUSE BUTTON=ADJUST> leaves the window open.
    <LI><B>Options</B> - Display the <A HREF="#options" TITLE="Backup Options">backup options</A> for this disc. Clicking with <MOUSE BUTTON=ADJUST> leaves the window open.
    <LI><B>Close</B> - Close the window.
</UL>
<P>
The backup operation uses <A HREF=":menu/async.html" TITLE="Action Windows">action windows</A> to display progress information and to allow interaction when required.
</PARA>

<HEADING NAME="options">Backup Options</HEADING>
<PARA>
The <PSIFS> backup options window allows the options for a single disc to be set. The name of the disc is displayed at the top of the window. There are three buttons at the bottom of the window:
<UL>
    <LI><B>Advanced</B> - Open a window showing the <A HREF="#advanced" TITLE="Advanced Backup Options">advanced backup options</A>. It should not normally be necessary to adjust these settings.
    <LI><B>Cancel</B> - Close the window without applying any changes. Clicking with <MOUSE BUTTON=ADJUST> leaves the window open, but resets the options to their previous values.
    <LI><B>Set</B> - Save and apply the displayed options. Clicking with <MOUSE BUTTON=ADJUST> applies the changes without closing the window.
</UL>
Immediately above these buttons there is a single option:
<UL>
    <B>Always use these settings</B> - Enable or disable backup operations from starting automatically when the backup window is opened or an automatic backup is due. If this option is enabled then a five second countdown is provided to enable the backup to be aborted or the options to be changed.
</UL>
</PARA>

<HEADING>Backup Options - Automatic backup</HEADING>
<PARA>
The top section of the window allows configuration of automatic backups. The options are:
<UL>
    <LI><B>Perform automatic backup</B> - Enable or disable automatic backups of this disc at pre-defined intervals. If this option is selected then the following additional options are available to control the backup interval:
        <UL>
            <LI><B>Each time connected</B> - start a backup each time the disc is connected.
            <LI><B>Every day</B> - start a backup the first time each day the disc is connected.
            <LI><B>Every week</B> - start a backup the first time each week the disc is connected.
        </UL>
</UL>
</PARA>

<HEADING>Backup Options - Archive options</HEADING>
<PARA>
The bottom section of the window controls how previous backups are kept.
<UL>
    <LI><B>Keep previous versions</B> - This sets the number of backup versions that are kept. If this is set to 1 then only the most recent backup is stored. Higher values improve security, but require more disc space.
    <LI><B>Store changes only</B> - Enable or disable incremental backups. If this option is selected then only the files deleted or changed are stored for previous backups, otherwise all files are stored. The most recent backup always contains all files from the disc.
</UL>
</PARA>

<HEADING NAME="advanced">Advanced Backup Options</HEADING>
<PARA>
The advanced backup options control how discs are recognised and associated with backup files and configurations. The settings are:
<UL>
    <LI><B>Disc name</B> - the name of the disc. It is not possible to configure this value to be ignored if not media unique identifier is available.
    <LI><B>Drive letter</B> - the drive letter used to access the disc.
    <LI><B>Media unique identifier</B> - an eight digit hexadecimal value uniquely identifying a disc. This value may not be available for the internal disc of <SIBO> devices or for discs not formatted by a <SIBO> or <EPOC> device. It is not possible to configure this value to be ignored.
    <LI><B>Machine unique identifier</B> - a sixteen digit hexadecimal value uniquely identifying <EPOC> devices.
</UL>
<P>
If an <B>Ignore</B> option is set then the corresponding value is not used when checking for a match. Changing these options may not take immediate effect.
</PARA>

<HEADING NAME="files">Backup files</HEADING>
<PARA>
The backup files for each disc are stored in separate sub-directories with the same name as the disc. Each directory contains one or more of the following:
<UL>
    <LI><ARG>Options</ARG> - The options and status for the backup of this disc. This file must not be edited by hand.
    <LI><ARG>Backup</ARG> - The most recent backup of this disc.
    <LI><ARG>Full-<ARGU>x</ARGU></ARG> - Previous complete backups of this disc. The most recent previous backup is called <ARG>Full-1</ARG>, with older backups called <ARG>Full-2</ARG>, <ARG>Full-3</ARG> etc.
    <LI><ARG>Changes-<ARGU>x</ARGU></ARG> - Previous incremental backups of this disc. The most recent previous backup is called <ARG>Changes-1</ARG>, with older backups called <ARG>Changes-2</ARG>, <ARG>Changes-3</ARG> etc. These files only contain the files removed or changed between backups.
</UL>
<P>
All backup files are written in <ARG>tar</ARG> format, using the <ARG>fltar</ARG> <RISCOS> specific variant to preserve attributes and date stamps accurately. However, <PSIFS> can reliably read all of the <ARG>tar</ARG> file variants supported by <ARG>!SparkFS</ARG>, including <ARG>arctar</ARG>, <ARG>comma</ARG> and <ARG>unix</ARG>.
<P>
Any tool that supports the <ARG>fltar</ARG> variant of <ARG>tar</ARG> files can be used for <A HREF=":start/restr.html" TITLE="Restoring Backups">restoring backups</A>. Alternatively,the <CMDL CMD="PsiFSTar" HREF=":command/tar.html"> command may be used.
</PARA>

</PAGE>
