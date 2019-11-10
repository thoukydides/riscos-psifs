<*
    File        : conv/res.hsc
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

<PAGE TITLE="File Format Converters" SUBTITLE="!PsiFSRes" PARENT=":conv/index.html" KEYWORDS="File Converters,File Formats">

<HEADING><ARG>!PsiFSRes</ARG></HEADING>
<PARA>
A toolbox resource file called <ARG>!PsiFSRes</ARG> can be provided to allow options for the file format converter to be selected.
<P>
The <ARG>!PsiFSRes</ARG> file is loaded by <PSIFS> immediately after processing the associated <A HREF=":conv/psifs.html" TITLE="!PsiFS"><ARG>!PsiFS</ARG></A> file. The templates within the file are merged with those belonging to <PSIFS> itself, as well as those belonging to other file format converters, so it is essential that unique names are used for all templates. Hence, the <ARG><ARGU>converter</ARGU></ARG> part of the <ARG>PsiFSConverter$<ARGU>converter</ARGU></ARG> variable should be used as a prefix for all template names.
<P>
Any number of toolbox templates may be included within the resource file, and any template may be used for any combination of file format converters. However, there are some restrictions; these are described in the following sections.
</PARA>

<HEADING>Object Flags</HEADING>
<PARA>
The object flags for each toolbox template must be set as follows:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Flag</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Auto-create</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>No</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Auto-show</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>No</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Shared object</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>No</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Ancestor object</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>No</ARG></TD></TR>
</TABLE>
</PARA>

<HEADING>Window Properties</HEADING>
<PARA>
All toolbox templates must be window objects. The <B>Main window properties</B> should be set as follows:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Title</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER>&quot;<ARG>PsiFS conversion options</ARG>&quot;</TD></TR>
    <TR VALIGN=TOP><TD>Justify title</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Centre</ARG></TD>
    <TR VALIGN=TOP><TD>Back</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>On</ARG></TD>
    <TR VALIGN=TOP><TD>Close</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>On</ARG></TD>
    <TR VALIGN=TOP><TD>Toggle</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Hscroll</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Vscroll</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Size</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Show menu</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Default input focus</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Invisible caret</ARG> or <ARG>In gadget</ARG></TD>
    <TR VALIGN=TOP><TD>Auto-open</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>On</ARG></TD>
    <TR VALIGN=TOP><TD>Auto-close</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>On</ARG></TD>
    <TR VALIGN=TOP><TD>Deliver event before showing</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Default</ARG></TD>
    <TR VALIGN=TOP><TD>Deliver event when hidden</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Default</ARG></TD>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER>&quot;<ARG>This window allows options for the file conversion to be selected.</ARG>&quot;</TD>
</TABLE>
<P>
The <B>Other window properties</B> should also be set as follows:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Pane</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Hot keys</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Moveable</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>On</ARG></TD>
    <TR VALIGN=TOP><TD>Auto-redraw</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>On</ARG></TD>
    <TR VALIGN=TOP><TD>Backdrop</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Real colours</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Allow off-screen</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Force on-screen</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Button type</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>0</ARG></TD>
    <TR VALIGN=TOP><TD>Extendable X</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Extendable Y</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>User scroll</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
    <TR VALIGN=TOP><TD>Sprite area</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Wimp</ARG></TD>
    <TR VALIGN=TOP><TD>Pointer</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD>
</TABLE>
<P>
The <B>Window colours</B> should be set at the defaults as follows:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH COLSPAN=2>Property</TH><TH>Type</TH><TH>Colour</TH></TR>
    <TR VALIGN=TOP><TD ROWSPAN=3>Title bar</TD><TD>Foreground</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>7</ARG></TD>
    <TR VALIGN=TOP><TD>Background</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>2</ARG></TD>
    <TR VALIGN=TOP><TD>Input focus</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>12</ARG></TD>
    <TR VALIGN=TOP><TD ROWSPAN=2>Work area</TD><TD>Foreground</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>7</ARG></TD>
    <TR VALIGN=TOP><TD>Background</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>1</ARG></TD>
    <TR VALIGN=TOP><TD ROWSPAN=2>Scroll bars</TD><TD>Foreground</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>1</ARG></TD>
    <TR VALIGN=TOP><TD>Background</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>3</ARG></TD>
</TABLE>
<P>
The <B>Minimum size</B> and <B>Work area</B> should normally both be set to the normal size of the window. No toolbars should be attached to the window.
</PARA>

<HEADING>Gadgets</HEADING>
<PARA>
Any number of gadgets may be included within each window, although there are restrictions on the types and settings that can be used. These restrictions are described within the following sections.
<P>
All gadgets types other than Label and Labelled Box should have appropriate help text specified. Also, all gadget types other than Action Button should have delivery of events disabled; they are not required by <PSIFS>. The initial values specified for each gadget within the resource file are used as a defaults if no previously used options are available.
<P>
There are no specific requirements concerning component IDs, other than that they need to agree with the values specified in the <ARG>!PsiFS</ARG> file.
</PARA>

<HEADING>Gadgets - Action Button</HEADING>
<PARA>
Exactly two Action Button gadgets should be included in the window. The first is used by the user to accept the selected options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>&amp;0</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Text</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER>&quot;<ARG>Set</ARG>&quot;</TD></TR>
    <TR VALIGN=TOP><TD>Show object</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Deliver event</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Default</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Default</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>On</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Cancel</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Local</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>On</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER>&quot;<ARG>Click SELECT (or press RETURN) to apply the changes.|MClick ADJUST to apply changes without closing the box.</ARG>&quot;</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
<P>
The second is used by the user to discard the selected options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>&amp;1</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Text</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER>&quot;<ARG>Cancel</ARG>&quot;</TD></TR>
    <TR VALIGN=TOP><TD>Show object</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Deliver event</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Default</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Default</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Cancel</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>On</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Local</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>On</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER>&quot;<ARG>Click SELECT (or press ESCAPE) to close the box without applying any changes.|MClick ADJUST to reset the contents of the box.</ARG>&quot;</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
<P>
Additional Action Button gadgets are not supported.
</PARA>

<HEADING>Gadgets - Label</HEADING>
<PARA>
Label gadgets may be freely used to annotate options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Justify</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Display border</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
</PARA>

<HEADING>Gadgets - Labelled Box</HEADING>
<PARA>
Labelled Box gadgets may be freely used to group or annotate options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Text</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Sprite</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
</PARA>

<HEADING>Gadgets - Number Range</HEADING>
<PARA>
Number Range gadgets may be used to allow numeric options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Deliver events when value changes</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Minimum</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER>&gt;= <ARG>0</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Maximum</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER>&lt;= <ARG>2,147,483,647</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Initial</TD><TD ALIGN=CENTER COLSPAN=2>Set to desired default</TD></TR>
    <TR VALIGN=TOP><TD>Precision</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>0</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Step size</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Has numerical display</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Writable</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Justify</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Display width</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Has adjusters</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Has slider</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Slider colour</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Link to gadgets</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
<P>
The &quot;<ARG>N</ARG>&quot; specification is used for Number Range options. The value returned is an unscaled interger in the range 0 to 2,147,483,647; any <B>Precision</B> specification is ignored. If a fractional value is required then it must be post-processed to scale the value as appropriate.
</PARA>

<HEADING>Gadgets - Option Button</HEADING>
<PARA>
Option Button gadgets may be used to allow binary options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Text</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Deliver event</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>None</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Selected</TD><TD ALIGN=CENTER COLSPAN=2>Set to desired default</TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
<P>
The &quot;<ARG>O</ARG>&quot; specification is used for Option Button options. No value is returned; the state of the gadget is indicated by the presence of the specified switch.
</PARA>

<HEADING>Gadgets - Radio Button</HEADING>
<PARA>
Radio Button gadgets may be used to allow a single choice from a set of options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Group</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Text</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Deliver event</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>None</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Selected</TD><TD ALIGN=CENTER COLSPAN=2>Set to desired default</TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
<P>
The &quot;<ARG>R</ARG>&quot; specification is used for Radio Button options. No value is returned; the state of the gadget is indicated by the presence of the specified switch. Note that exactly one switch will be returned for a particular radio group.
</PARA>

<HEADING>Gadgets - String Set</HEADING>
<PARA>
String Set gadgets may be used to allow a single choice from a set of string options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Title</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Strings</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Initial</TD><TD ALIGN=CENTER COLSPAN=2>Set to desired default</TD></TR>
    <TR VALIGN=TOP><TD>Has display field</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Writable</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Justify</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Value changed</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>About to be shown</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Specify allowed characters</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER>Disable &quot;<ARG>&nbsp;</ARG>&quot;, &quot;<ARG>&quot;</ARG>&quot; and &quot;<ARG>-</ARG>&quot; to prevent quoted values</TD></TR>
    <TR VALIGN=TOP><TD>Link to gadgets</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
<P>
The &quot;<ARG>S</ARG>&quot; specification is used for String Set options. The value returned is the selected string. Note that the value will be quoted if it contains &quot;<ARG>&nbsp;</ARG>&quot; (space) or &quot;<ARG>&quot;</ARG>&quot; (quotation mark) characters, or if it starts with a &quot;<ARG>-</ARG>&quot; (hyphen-minus) character. If values different from the displayed strings are required then post-processing must be used to map the value as appropriate.
</PARA>

<HEADING>Gadgets - Writable Field</HEADING>
<PARA>
Writable Field gadgets may be used to allow user specified text options:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Property</TH><TH>Type</TH><TH>Setting</TH></TR>
    <TR VALIGN=TOP><TD>Component ID</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Text</TD><TD ALIGN=CENTER COLSPAN=2>Set to desired default</TD></TR>
    <TR VALIGN=TOP><TD>Justify</TD><TD ALIGN=CENTER COLSPAN=2>No requirement</TD></TR>
    <TR VALIGN=TOP><TD>Specify allowed characters</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER>Disable &quot;<ARG>&nbsp;</ARG>&quot;, &quot;<ARG>&quot;</ARG>&quot; and &quot;<ARG>-</ARG>&quot; to prevent quoted values</TD></TR>
    <TR VALIGN=TOP><TD>Password behaviour</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Link to gadgets</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Deliver events when value changes</TD><TD ALIGN=CENTER>Required</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
    <TR VALIGN=TOP><TD>Help text</TD><TD ALIGN=CENTER COLSPAN=2>Set appropriately</TD></TR>
    <TR VALIGN=TOP><TD>Faded</TD><TD ALIGN=CENTER>Recommended</TD><TD ALIGN=CENTER><ARG>Off</ARG></TD></TR>
</TABLE>
<P>
The &quot;<ARG>W</ARG>&quot; specification is used for Writable Field options. The value returned is the entered string. Note that the value will be quoted if it contains &quot;<ARG>&nbsp;</ARG>&quot; (space) or &quot;<ARG>&quot;</ARG>&quot; (quotation mark) characters, or if it starts with a &quot;<ARG>-</ARG>&quot; (hyphen-minus) character.
</PARA>

<HEADING>Gadgets - Other</HEADING>
<PARA>
No other gadget types are currently supported by <PSIFS>, so they must not be used. These include, but are not restricted to, Adjuster, Button, Display Field, Draggable, Popup Menu, Scroll List, Slider, Text Area and Tool Action gadgets.
</PARA>

</PAGE>
