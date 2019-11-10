<*
    File        : conv/expsi.hsc
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

<PAGE TITLE="File Format Converters" SUBTITLE="PsiFS Module Example" PARENT=":conv/index.html" KEYWORDS="File Converters,File Formats,Tar,SIS File,*PsiFSTar,*PsiFSSIS">

<HEADING><PSIFS> Module Example</HEADING>
<PARA>
An example file format converter configuration for the <CMDL CMD="PsiFSTar" HREF=":command/tar.html"> and <CMDL CMD="PsiFSSIS" HREF=":command/sis.html"> commands has been included in the directory <ARG>!PsiFS.Converters.!PsiFS</ARG>. This allows the contents of <ARG>tar</ARG> and SIS files to be extracted or listed. A print job launcher and a very simple converter for <EPOC> clipboard files have also been included.
<P>
Please note that since the file format converter interface uses the <ARG>!Scrap</ARG> directory to store temporary files, filenames may be truncated unless long filenames are supported on the drive where <ARG>!Scrap</ARG> is located.
<P>
The following sections describe each of the files within the sub-directory.
</PARA>

<HEADING><ARG>!PsiFS</ARG></HEADING>
<PARA>
The <A HREF=":conv/psifs.html" TITLE="!PsiFS"><ARG>!PsiFS</ARG></A> file tells <PSIFS> all of the important characteristics of the file format converter.
<P>
The first section of the file sets default conversion options, and this is followed by the converter configurations. Multiple converter configurations with different options are specified for each command; this is necessary if different output file types are produced.
<P>
The <ARG>PsiFSCapture</ARG> alias defined within the <ARG>!Boot</ARG> file is used to capture the output from a command to a specified file. The <ARG>PsiFSInstall</ARG>, <ARG>PsiFSConvert</ARG> and <ARG>PsiFSPrint</ARG> aliases are used to start BASIC programs to perform other special operations.
</PARA>

<HEADING><ARG>!PsiFSRes</ARG></HEADING>
<PARA>
The <A HREF=":conv/res.html" TITLE="!PsiFSRes"><ARG>!PsiFSRes</ARG></A> file contains the toolbox template used to select conversion options for SIS files, and launch options for print jobs. There are no user selectable options for the conversion of <ARG>tar</ARG> files.
<P>
A String Set gadget is used to allow selection of the installation drive. It is not possible to generate the list of available options dynamically when the conversion is performed, so a static list offering just drives <ARG>C</ARG> and <ARG>D</ARG> is specified in the toolbox resource file.
</PARA>

<HEADING><ARG>!Boot</ARG></HEADING>
<PARA>
The <A HREF=":conv/boot.html" TITLE="!Boot"><ARG>!Boot</ARG></A> file is run by <PSIFS> at start-up using <SWIL SWI="Wimp_StartTask">.
<P>
This starts by defining the textual version of the supported file types to ensure that the <ARG>!PsiFS</ARG> file can be correctly parsed. The file format converter is then registered by defining the system variable <ARG>PsiFSConverter$PsiFS</ARG> to point to the current directory. Another set of system variables (<ARG>PsiFS$ConverterScrap</ARG>, <ARG>PsiFS$ConverterInstall</ARG> and <ARG>PsiFS$ConverterScrap</ARG>) are defined to specify the names of temporary files. Finally, aliases for <ARG>PsiFSCapture</ARG>, <ARG>PsiFSInstall</ARG>, <ARG>PsiFSConvert</ARG> and <ARG>PsiFSPrint</ARG> are defined to run the <ARG>Capture</ARG>, <ARG>Install</ARG>, <ARG>Convert</ARG> and <ARG>Print</ARG> programs respectively, enabling simpler commands to be used within the <ARG>!PsiFS</ARG> file.
</PARA>

<HEADING><ARG>!Run</ARG></HEADING>
<PARA>
The <ARG>!Run</ARG> file is not normally run; it is included solely to provide a friendly error message if someone does try to run it.
</PARA>

<HEADING><ARG>Capture</ARG></HEADING>
<PARA>
The <ARG>Capture</ARG> file is a simple BASIC program that executes a command with the output captured to a text file.
<P>
This program is used instead of a simple command alias to ensure that the output file is deleted if an error occurs. This behaviour is required by the <PSIFS> file format converter interface.
</PARA>

<HEADING><ARG>Convert</ARG></HEADING>
<PARA>
The <ARG>Convert</ARG> file is a simple BASIC program that performs a straightforward bidirectional conversion between <EPOC> clipboard and text files.
<P>
<SWIL SWI="PsiFS_GetTranslationTable" HREF=":swi/trans.html"> is used to obtain a character translation table between the <WINDOWS> ANSI character set used by <EPOC> and the Latin1 character set normally used by <RISCOS>. A few additional translations are then added to handle some peculiarities of <EPOC> files.
</PARA>

<HEADING><ARG>Install</ARG></HEADING>
<PARA>
The <ARG>Install</ARG> file is a simple BASIC program that starts an asynchronous remote operation to install a SIS file.
<P>
The first step is to make a copy of the SIS file to ensure that it is still available after the file format converter interface has deleted any temporary files. <SWIL SWI="PsiFS_AsyncStart" HREF=":swi/astrt.html"> is then used to start the installation, and <WIMPL MSG="Message_PsifsAsyncStart" HREF=":wimp/astrt.html"> is broadcast to open an action window.
<P>
Finally, a dummy output file is created to signal to <PSIFS> that the &quot;conversion&quot; has been successful.
</PARA>

<HEADING><ARG>Print</ARG></HEADING>
<PARA>
The <ARG>Print</ARG> file is a simple BASIC program that sends a message to the <PSIFS> filer to process a print job file.
<P>
Like the <ARG>Install</ARG> program, the first step is to make a copy of the print job file to ensure that it is still available after the file format converter interface has deleted any temporary files. <WIMPL MSG="Message_PsifsPrint" HREF=":wimp/print.html"> is then broadcast to open a print job window.
<P>
A dummy output file is created to signal to <PSIFS> that the &quot;conversion&quot; has been successful, and the temporary copy of the print job file is deleted.
</PARA>

<HEADING><ARG>AddRemove</ARG></HEADING>
<PARA>
The <ARG>AddRemove</ARG> file is a copy of the <ARG>InstExe.exe</ARG> executable used to install the <ARG>Add/remove</ARG> <NAME>control&nbsp;panel</NAME> icon. This is copied to an <EPOC> device and run to install the necessary files.
</PARA>

</PAGE>
