<*
    File        : conv/exfsi.hsc
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

<PAGE TITLE="File Format Converters" SUBTITLE="!ChangeFSI Example" PARENT=":conv/index.html" KEYWORDS="File Converters,File Formats">

<HEADING><ARG>!ChangeFSI</ARG> Example</HEADING>
<PARA>
An example file format converter configuration for <ARG>!ChangeFSI</ARG> has been included in the directory <ARG>!PsiFS.Converters.!ChangeFSI</ARG>. This allows common graphics formats (GIF, PCX, BMP, PBM, JPEG, TIFF, and Sprite files) to be converted to either Sprite or JPEG files with a selectable number of bits per pixel and processing options.
<P>
The following sections describe each of the files within the sub-directory.
</PARA>

<HEADING><ARG>!PsiFS</ARG></HEADING>
<PARA>
The <A HREF=":conv/psifs.html" TITLE="!PsiFS"><ARG>!PsiFS</ARG></A> file tells <PSIFS> all of the important characteristics of the file format converter.
<P>
The first section of the file sets default conversion options for all of the supported file types. This is followed by a separate converter configuration for each output format. Note that both of the converters in this example support the same input file types, so these are defined once within the shared characteristics section.
<P>
The <ARG>PsiFSChangeFSI</ARG> alias defined within the <ARG>!Boot</ARG> file is used to perform all of the conversions, with additional parameters specifying the required behaviour.
<P>
The command line argument to <ARG>!ChangeFSI</ARG> used to specify the output file format is not prefixed by a &quot;<ARG>-</ARG>&quot; (minus-hyphen), so the arguments supplied by <PSIFS> need to be modified. To allow easy manipulation within the <ARG>!Run</ARG> file requires this argument to be in a fixed position. This is achieved by relying on the sorting order: options starting with numbers or upper case letters are always placed before options starting with lower case letters.
</PARA>

<HEADING><ARG>!PsiFSRes</ARG></HEADING>
<PARA>
The <A HREF=":conv/res.html" TITLE="!PsiFSRes"><ARG>!PsiFSRes</ARG></A> file contains the two toolbox templates used to select the conversion options.
<P>
Radio buttons are used to allow selection of a single output format. A String Set gadget could have been used for the same purpose, but the returned arguments would have required greater pre-processing to convert them into a suitable form for passing to <ARG>!ChangeFSI</ARG>.
</PARA>

<HEADING><ARG>!Boot</ARG></HEADING>
<PARA>
The <A HREF=":conv/boot.html" TITLE="!Boot"><ARG>!Boot</ARG></A> file is run by <PSIFS> at start-up using <SWIL SWI="Wimp_StartTask">.
<P>
This starts by defining the textual version of all the supported file types to ensure that the <ARG>!PsiFS</ARG> file can be correctly parsed. Next, the file format converter is registered by defining the system variable <ARG>PsiFSConverter$ChangeFSI</ARG> to point to the current directory. Finally, an alias for <ARG>PsiFSChangeFSI</ARG> is defined to run the <ARG>!Run</ARG> file, enabling simpler commands to be used within the <ARG>!PsiFS</ARG> file.
</PARA>

<HEADING><ARG>!Run</ARG></HEADING>
<PARA>
The <ARG>!Run</ARG> file is run by <PSIFS> using <SWIL SWI="Wimp_StartTask"> when the converter is called via the <ARG>PsiFSChangeFSI</ARG> alias.
<P>
It starts by checking that it has been supplied with parameters and ensuring that there is sufficient memory to run <ARG>ChangeFSI</ARG>. If necessary, the <ARG>FindFSI</ARG> BASIC program is then used to locate <ARG>!ChangeFSI</ARG> if has not already been seen by the filer.
<P>
A temporary alias <ARG>Alias$PsiFSChangeFSITemp</ARG> is used to allow easy manipulation of the arguments to remove the &quot;<ARG>-</ARG>&quot; (minus-hyphen) from the output format specifier. Finally, the <ARG>ChangeFSI</ARG> executable is run to perform the required conversion, and any error is reported.
</PARA>

<HEADING><ARG>FindFSI</ARG></HEADING>
<PARA>
The <ARG>FindFSI</ARG> file is a simple BASIC program that searches for <ARG>!ChangeFSI</ARG> in likely locations. If a copy is found then its <ARG>!Boot</ARG> file is run to set the necessary system variables.
<P>
This program is only required because the file format converter is not contained within the same directory as the configuration files.
</PARA>

</PAGE>
