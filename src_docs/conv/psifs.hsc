<*
    File        : conv/psifs.hsc
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

<PAGE TITLE="File Format Converters" SUBTITLE="!PsiFS" PARENT=":conv/index.html" KEYWORDS="File Converters,File Formats">

<HEADING><ARG>!PsiFS</ARG></HEADING>
<PARA>
A file called <ARG>!PsiFS</ARG> is required to describe the characteristics of each file format converter.
<P>
The <ARG>!PsiFS</ARG> file is split into several sections, each terminated by a line containing <ARG>&lt;EOF&gt;</ARG> on its own. Within each section, lines of the form <ARG><ARGU>tag</ARGU>=<ARGU>value</ARGU></ARG> are used to associate a value with a tag. Any other lines are treated as comments which are ignored; no special prefix character is required.
<P>
Note that both case and spaces are significant. In particular, trailing spaces prevent recognition of the <ARG>&lt;EOF&gt;</ARG> token, so if problems are encountered it is worth checking that there are no invisible trailing spaces.
</PARA>

<HEADING>Defaults</HEADING>
<PARA>
The first section of the <ARG>!PsiFS</ARG> file specifies default options. These are automatically copied to the <PSIFS> options file (either <ARG>!Boot.Choices.PsiFS.Options</ARG> or <ARG>!PsiFS.Config.Options</ARG>, depending upon the computer's configuration) if the associated tag is not already defined.
<P>
The tags currently associated with selecting file format converters are:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Tag</TH><TH>Description</TH></TR>
    <TR VALIGN=TOP><TD><ARG>Convert</ARG></TD><TD>Any conversion.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU></ARG></TD><TD>Any conversion of <ARG><ARGU>type</ARGU></ARG> file.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU>_Standard</ARG></TD><TD>Stand-alone conversion of <ARG><ARGU>type</ARGU></ARG> file.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU>_InterceptLoad</ARG></TD><TD ROWSPAN=2>Loading a <ARG><ARGU>type</ARGU></ARG> file into <ARG><ARGU>receiver</ARGU></ARG>.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU>_InterceptLoad_<ARGU>receiver</ARGU></ARG></TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU>_InterceptRun</ARG></TD><TD ROWSPAN=2>Running a <ARG><ARGU>type</ARGU></ARG> file from <ARG><ARGU>sender</ARGU></ARG>.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU>_InterceptRun_<ARGU>sender</ARGU></ARG></TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU>_InterceptSave</ARG></TD><TD ROWSPAN=3>Saving a <ARG><ARGU>type</ARGU></ARG> file from <ARG><ARGU>sender</ARGU></ARG> to <ARG><ARGU>receiver</ARGU></ARG>.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU>_InterceptSave_<ARGU>sender</ARGU></ARG></TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert_<ARGU>type</ARGU>_InterceptSave_<ARGU>sender</ARGU>_<ARGU>receiver</ARGU></ARG></TD></TR>
</TABLE>
<P>
In these tags <ARG><ARGU>type</ARGU></ARG> is the file type as a three digit upper-case hexadecimal number, <ARG><ARGU>sender</ARGU></ARG> is the name of the task sending the file, and <ARG><ARGU>receiver</ARGU></ARG> is the name of the task receiving the file. Both <ARG><ARGU>sender</ARGU></ARG> and <ARG><ARGU>receiver</ARGU></ARG> are the task names displayed by the Task Manager, but with any spaces or equals signs removed.
<P>
The value associated with each of these tags is the <ARG>Tag</ARG> of the file format converter to use, or blank if no conversion should be applied.
<P>
When a conversion is required, <PSIFS> uses these options to select the default file format converter, starting from the most specific matching option and continuing until a selectable file format converter is found. This behaviour is especially important if automatic conversions are enabled.
<P>
The following additional tags are associated with the options for the file format converters:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Tag</TH><TH>Description</TH></TR>
    <TR VALIGN=TOP><TD><ARG>ConvertOptions_<ARGU>Tag</ARGU></ARG></TD><TD>The most recently used options.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>ConvertOptions_<ARGU>Tag</ARGU>_Version</ARG></TD><TD>The <ARG><ARGU>Options</ARGU></ARG> value of the converter, used to detect changes and prevent use of incompatible options.</TD></TR>
</TABLE>
<P>
It should not be necessary to include these tags within the <ARG>!PsiFS</ARG> file; they are described here for completeness. It is more effective to set default options within the <A HREF=":conv/res.html" TITLE="!PsiFSRes"><ARG>!PsiFSRes</ARG></A> toolbox resource file.
<P>
Each time a conversion is performed, <PSIFS> automatically updates the appropriate defaults to ensure that the most suitable conversion and options are selected for successive operations.
</PARA>

<HEADING>Shared Characteristics</HEADING>
<PARA>
The second section of the <ARG>!PsiFS</ARG> file defines tags that apply to all converters. These definitions are overridden by a tag of the same name specified for an individual file format converter.
<P>
See the individual characteristics defined below for details of the available options.
</PARA>

<HEADING>Individual Characteristics</HEADING>
<PARA>
The following tags may be defined for each converter:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Tag</TH><TH>Type</TH><TH>Description</TH></TR>
    <TR VALIGN=TOP><TD><ARG>Tag</ARG></TD><TD ALIGN=CENTER>Required</TD><TD>Name to use for the converter in <PSIFS> configuration files. This is currently only used to specify the default converter, but in the future it may also be used to store converter specific options.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Name</ARG></TD><TD ALIGN=CENTER>Recommended</TD><TD>Text to display in the Name field of the Info window. This should normally be the name of the tool used to perform the conversion.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Author</ARG></TD><TD ALIGN=CENTER>Recommended</TD><TD>Text to display in the Author field of the Info window. This should normally include any copyright message.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Version</ARG></TD><TD ALIGN=CENTER>Recommended</TD><TD>Text to display in the Version field of the Info window.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Purpose</ARG></TD><TD ALIGN=CENTER>Recommended</TD><TD>Text to display in the Purpose field of the Info window. This should normally describe the individual file format conversion.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>WebSite</ARG></TD><TD ALIGN=CENTER>Recommended</TD><TD>URL to launch if the Web site button in the Info window is clicked. This should normally be the site associated with the converter.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Description</ARG></TD><TD ALIGN=CENTER>Recommended</TD><TD>Menu entry for this file format converter. This should normally include both the source and destination file types.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>ButtonText</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>Text to display on the button used to start the conversion. This should normally only be specified if the main purpose of the file format converter is a side-effect.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>ButtonHelp</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>Interactive help text for the button used to start the conversion. This should normally only be specified if the main purpose of the file format converter is a side-effect.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Input</ARG></TD><TD ALIGN=CENTER>Required</TD><TD>List of supported input file types.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>InputUID</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>Space separated list of supported input file UIDs. The UIDs should be specified in the same format as for <CMDL CMD="PsiFSMap" HREF=":command/map.html">, i.e. as 24 digit hexadecimal numbers. If this tag is left blank then no restrictions are placed on the input UID.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>InputName</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>Space separated list of supported input leaf names. Standard wildcards may be used, for example to check for specific extensions. If this tag is left blank then no restrictions are placed on the input file name.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Output</ARG></TD><TD ALIGN=CENTER>Required</TD><TD>Single file type produced by the file format converter.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>InterceptLoad</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>List of file types to intercept when loaded by dragging within the desktop.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>InterceptRun</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>List of file types to intercept when run by double-clicking within the desktop.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>InterceptSave</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>List of file types to intercept when saved or transferred between applications by dragging within the desktop.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Quality</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>The quality of the conversion specified as an integer between 0 (for very bad) to 100 (for perfect). This is used to help choose the most appropriate converter for invisible conversions such as global clipboard support. If this tag is not specified then a value of 25 is assumed.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Options</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>Definition of user selectable options.</TD></TR>
    <TR VALIGN=TOP><TD><ARG>Convert</ARG></TD><TD ALIGN=CENTER>Required</TD><TD>The <ARG>Convert</ARG> tag specifies the command used to perform the conversion. The source and destination filenames and the original leafname are substituted for the character sequences <ARG>&lt;src&gt;</ARG>, <ARG>&lt;dest&gt;</ARG> and <ARG>&lt;orig&gt;</ARG> respectively. Any options specified using the <ARG>Options</ARG> tag are substituted for the character sequence <ARG>&lt;opt&gt;</ARG></TD></TR>
    <TR VALIGN=TOP><TD><ARG>ConvertSilent</ARG></TD><TD ALIGN=CENTER>Optional</TD><TD>The <ARG>ConvertSilent</ARG> tag specifies an alternative command for performing the conversion. This is similar to the <ARG>Convert</ARG> tag, except that the converter should not report any errors and no options will be substituted for any <ARG>&lt;opt&gt;</ARG> character sequence. This is used for invisible conversions such as global clipboard support.</TD></TR>
</TABLE>
<P>
File type lists consist of a space separated list of <RISCOS> file types, specified in either numeric or textual form. The following special file types are also supported: <ARG>Directory</ARG>, <ARG>Application</ARG> and <ARG>Untyped</ARG>. However, although they are fully supported, it is recommended that these special file types are not used to trigger file transfer intercepts. An additional special file type of <ARG>NoConversion</ARG> is also supported for the <ARG>Output</ARG> tag to indicate that no output file will be produced; some other side-effect will occur instead.
<P>
The list of files to intercept only apply if enabled within the <PSIFS> configuration, and may be overridden by the user pressing <ARG>Left&nbsp;Alt</ARG> to force an intercept or <ARG>Right&nbsp;Alt</ARG> to prevent an intercept.
<P>
The <ARG>Options</ARG> value starts with the name of a toolbox template from the <ARG>!PsiFSRes</ARG> file, followed by a space separated list of &quot;<ARG>-<ARGU>keyword</ARGU>&nbsp;<ARGU>gadget</ARGU></ARG>&quot; pairs. The <ARG>-<ARGU>keyword</ARGU></ARG> is the option that will be substituted for the <ARG>&lt;opt&gt;</ARG> character sequence within the <ARG>Convert</ARG> string, and <ARG><ARGU>gadget</ARGU></ARG> consists of a single letter gadget type followed by the component ID of the gadget in hexadecimal. Note that the options are sorted before use; ordering is not preserved.
</PARA>

<HEADING>Running Converter</HEADING>
<PARA>
The file format converter is run using <SWIL SWI="Wimp_StartTask">, so almost any command may be used. However, <PSIFS> assumes that the conversion will have been performed when control is returned, so multi-tasking converters are not supported.
<P>
The behaviour of the converter if unable to handle the supplied file depends on whether the command specified by the <ARG>Convert</ARG> or <ARG>ConvertSilent</ARG> tag is used. In both cases the converter must exit without producing an output file. If the <ARG>Convert</ARG> was used then the converter should generate an error using either <SWIL SWI="OS_GenerateError"> or <SWIL SWI="Wimp_ReportError"> before exiting, but if the <ARG>ConvertSilent</ARG> was used then no error should be reported. Error messages should not be output via the VDU drivers (<ARG>stdout</ARG>/<ARG>stderr</ARG> for a C program); if necessary output should be redirected to a file and post-processed to convert any messages to <RISCOS> errors.
<P>
Future versions may support converters running in a TaskWindow; this will be controlled by additional tags specifying the capabilities of individual converters.
<P>
</PARA>

</PAGE>
