<*
    File        : start/restr.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Restoring Backups" PARENT=":start/index.html" KEYWORDS="Backup,Restore,Configuration,Tar,!SparkFS,David Pilling,*PsiFSTar,Drive,Disc Name,Disk Name,Frank Lancaster,Email">

<HEADING>Restoring Backups</HEADING>
<PARA>
<PSIFS> does not offer a restore option for backup files. Instead, it stores the backups in <ARG>tar</ARG> files, using the <ARG>fltar</ARG> <RISCOS> variant to preserve attribute and date stamps accurately. This allows any software that can handle these files to be used for restoring a backup.
</PARA>

<HEADING>Finding the Backup Files</HEADING>
<PARA>
The directory containing the backup files is specified within the main <PSIFS> <A HREF=":start/confg.html" TITLE="Configuration">configuration</A>. This directory contains sub-directories based on the original name given to the backup.
<P>
The easiest way to find the appropriate directory is to ensure that a <A HREF=":start/conct.html" TITLE="Connecting">connection</A> has been established, and that the <A HREF=":icon/drive.html" TITLE="SIBO or EPOC Drive Icon"><SIBO> or <EPOC> drive icon</A> for the appropriate drive is visible. Open the backup control window by selecting <MENL TITLE="Backup..." HREF=":menu/backu.html"> from the iconbar icon menu, and then click on the <ARG>Open</ARG> button. This will open a filer window showing all of the backup files for that disc.
<P>
The most recent backup file is called <ARG>Backup</ARG>, with older backups named either <ARG>Full-1</ARG>, <ARG>Full-2</ARG> etc, or <ARG>Changes-1</ARG>, <ARG>Changes-2</ARG> etc. The number and type of files depend on the backup options set for the disc.
</PARA>

<HEADING>Extracting Files</HEADING>
<PARA>
Any tool that can correctly handle <ARG>tar</ARG> files using the <ARG>fltar</ARG> <RISCOS> variant may be used to extract files from a <PSIFS> backup. To restore a backup simply copy the files back to their original drive.
<P>
The recommended tool is <A HREF="mailto:david@pilling.demon.co.uk" TITLE="Send Email to David Pilling">David Pilling</A>'s <ARG>!SparkFS</ARG>. However, only recent versions are suitable; early versions interpret the filenames incorrectly. Another good tool is <A HREF="mailto:fl@tools.de" TITLE="Send Email to Frank Lancaster">Frank Lancaster</A>'s public domain command line <A HREF="http://www.tools.de/fl/tar.arc"><ARG>tar</ARG></A> utility.
<P>
As a last resort, <PSIFS> provides a <CMDL CMD="PsiFSTar" HREF=":command/tar.html"> command to allow files to be listed or extracted from backups. However, this is not very friendly or versatile, so other options should be tried first.
</PARA>

</PAGE>
