<*
    File        : wimp/astrt.hsc
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

<PAGE TITLE="WIMP Messages" SUBTITLE="Message_PsifsAsyncStart" PARENT=":wimp/index.html" KEYWORDS="WIMP Messages,PsiFS_AsyncStart,Asynchronous Remote Operation,Action Windows">

<WIMP NAME="Message_PsifsAsyncStart" NUM="520C0">
    <WIMPB OFFSET="20">handle of asynchronous remote operation</WIMPB>
    <WIMPB OFFSET="24">initial position x coordinate (-1 for automatic)</WIMPB>
    <WIMPB OFFSET="28">initial position y coordinate (-1 for automatic)</WIMPB>
    <WIMPB OFFSET="32">display <ARG>Close</ARG> button (0 closes window automatically)</WIMPB>
    <WIMPB OFFSET="36">display <ARG>Abort</ARG> button (0 disables aborting)</WIMPB>
    <WIMPB OFFSET="40">display <ARG>Pause</ARG> button (0 disables pausing)</WIMPB>
    <WIMPB OFFSET="44">title for action window, zero-terminated</WIMPB>
    <WIMPU>
        This message is broadcast by a task that has started an asynchronous remote operation using <SWIL SWI="PsiFS_AsyncStart" HREF=":swi/astrt.html"> to request that the <PSIFS> filer accept ownership of the operation and open an action window for it.
        <P>
        If <PSIFS> accepts the transfer of ownership then it will acknowledge the message, and will take responsibility for calling <SWIL SWI="PsiFS_AsyncEnd" HREF=":swi/aend.html"> at the appropriate time.
    </WIMPU>
</WIMP>

</PAGE>
