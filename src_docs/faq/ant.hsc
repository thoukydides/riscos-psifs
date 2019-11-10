<*
    File        : faq/ant.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Serial Port Clashes" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,,Problems,!InetSuite,Serial Port,Device Claim Protocol">

<HEADING>Why does the computer hang when <PSIFS> is enabled?</HEADING>
<PARA>
Many applications do not fully support the device claim protocol. This means that another task may start using the serial port while <PSIFS> is using it, or a task already using the serial port may not object when <PSIFS> attempts to claim it.
<P>
If a clash over use of the serial port occurs then the computer may appear to hang. Press <ARG>Escape</ARG> to regain control, then save any work and reboot the computer.
<P>
A common cause of this kind of problem is if the ANT Internet Suite is being used. <ARG>!InetSuite</ARG> accesses the serial port even when it is not using the modem, so it must be quit before enabling <PSIFS>. It is recommended that <ARG>!InetSuite</ARG> and <PSIFS> are never loaded at the same time.
</PARA>

</PAGE>
