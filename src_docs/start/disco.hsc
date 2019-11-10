<*
    File        : start/disco.hsc
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

<PAGE TITLE="Getting Started" SUBTITLE="Disconnecting" PARENT=":start/index.html" KEYWORDS="Remote Link,Disconnect,Serial Port,Quit,Idle">

<HEADING>Disconnecting</HEADING>
<PARA>
It is not necessary to perform any special action to terminate a <A HREF=":start/conct.html" TITLE="Connecting">connected</A> link; simply disconnect the cable or disable the software on either or both machines. However, this will not allow any other application to use the serial port. Hence, it is better to either disable or quit <PSIFS> when it is no longer required.
<P>
The remote link increases the power consumption of <SIBO> or <EPOC> devices. Hence, to maximize battery life, it is recommended that the remote link is disabled whenever it is not being used:
<UL>
    <LI>On <SIBO> devices the link is disabled using the <ARG>Remote Link</ARG> or <ARG>Communications</ARG> option from the <ARG>Special</ARG> menu, or by pressing <ARG>Psion+L</ARG> or <ARG>Acorn+L</ARG>.
    <LI>On <EPOC> devices the link is disabled using the <ARG>Remote link...</ARG> option from the <ARG>Tools</ARG> menu, or by pressing <ARG>Ctrl+L</ARG>.
</UL>
</PARA>

<HEADING>Disabling PsiFS</HEADING>
<PARA>
To disable <PSIFS> click on the iconbar icon with <MOUSE BUTTON=ADJUST> or select <MENL TITLE="Disconnect" HREF=":menu/disco.html"> from the iconbar icon menu. The <A HREF=":icon/idle.html" TITLE="Idle Icon">idle icon</A> will be displayed to indicate that the serial port is no longer being used:
<CENTER><A HREF=":icon/idle.html" TITLE="Idle Icon"><IMG SRC=":graphics/idle.gif" ALT="" BORDER=0></A></CENTER>
</PARA>

<HEADING>Quitting PsiFS</HEADING>
<PARA>
To quit <PSIFS> simply click on the iconbar icon with <MOUSE BUTTON=MENU> and select <MENL TITLE="Quit"> from the menu. This will remove both the filer and file system module from memory.
</PARA>

</PAGE>
