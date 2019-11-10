<*
    File        : start/backu.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Backup" PARENT=":start/index.html" KEYWORDS="Backup,Configuration,Connect,Action Windows,Drive,Disc Name,Disk Name">

<HEADING>Backup</HEADING>
<PARA>
The backup feature is probably the most complicated part of <PSIFS>, but once setup it can operate completely autonomously, with no manual intervention required.
<P>
The most important point to grasp is that backups are disc orientated, using the disc name and media unique identifier (if available) to uniquely associate each disc with its options and backup files. This means that every disc should be given a different name.
<P>
Unfortunately, <A HREF=":start/restr.html" TITLE="Restoring Backups">restoring a backup</A> is slightly more complicated.
</PARA>

<HEADING>Configuring Backups</HEADING>
<PARA>
The configuration of backups is split into two parts: global settings that apply to all discs, and options specific to a single disc.
<P>
The global settings are set with the general <A HREF=":start/confg.html" TITLE="Configuration">configuration</A> accessed by selecting <MENL TITLE="Configure..." HREF=":menu/confg.html"> from the iconbar icon menu of the <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A>. The main item to set is the <B>Directory containing backup files</B>, which defaults to a directory called <ARG>Backups</ARG> in the same directory as <ARG>!PsiFS</ARG>.
<P>
To set the disc specific options, ensure that a <A HREF=":start/conct.html" TITLE="Connecting">connection</A> has been established, and that the <A HREF=":icon/drive.html" TITLE="SIBO or EPOC Drive Icon"><SIBO> or <EPOC> drive icon</A> for the drive to configure is visible. Open the backup control window by selecting <MENL TITLE="Backup..." HREF=":menu/backu.html"> from the iconbar icon menu, and then click on the <ARG>Options</ARG> button. Set the required options, and then click <ARG>Set</ARG> to save and apply the changes.
</PARA>

<HEADING NAME="manual">Manual Backup</HEADING>
<PARA>
Ensure that a <A HREF=":start/conct.html" TITLE="Connecting">connection</A> has been established, and that the <A HREF=":icon/drive.html" TITLE="SIBO or EPOC Drive Icon"><SIBO> or <EPOC> drive icon</A> for the drive to backup is visible. Open the backup control window by selecting <MENL TITLE="Backup..." HREF=":menu/backu.html"> from the iconbar icon menu.
<P>
Click on the <ARG>Backup</ARG> icon to start a backup manually. This would be selected automatically after a five second countdown if the <B>Always use these settings</B> option is set for this disc.
</PARA>

<HEADING NAME="automatic">Automatic Backup</HEADING>
<PARA>
The <B>Perform automatic backup</B> option must be selected to enable automatic backups. With this option set, <PSIFS> will check the date of the last backup each time a connection is established. If the configured interval has elapsed then the backup control window will be opened to prompt for a backup to be started.
<P>
For a fully automatic backup, the <B>Always use these settings</B> option should also be set. This will cause a five second countdown to be started at the same time as opening the backup control window.
</PARA>

<HEADING>Controlling a Backup</HEADING>
<PARA>
The progress of a backup operation is displayed by an <A HREF=":menu/async.html" TITLE="Action Windows">action window</A>. Most of the window is used to describe the current operation, and the timing information. At the bottom of the window up to four buttons, depending on context, allow the operation to be controlled. The two most important buttons are <ARG>Abort</ARG> which cancels a backup, and <ARG>Pause</ARG> which suspends a backup.
</PARA>

</PAGE>
