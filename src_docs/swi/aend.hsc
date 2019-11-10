<*
    File        : swi/aend.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="PsiFS_AsyncEnd" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,PsiFS_AsyncEnd,Asynchronous Remote Operation,Backup,Action Windows">

<SWI NAME="PsiFS_AsyncEnd" NUM="520C6" DESC="End an asynchronous remote operation">
    <SWIE REG="R0">the previously allocated handle for this operation</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        End a previously started asynchronous remote operation. If the operation is still in progress, then this may either terminate the operation or allow it to complete depending on the operation being performed.
    </SWIU>
    <SWIS><SWIL SWI="PsiFS_AsyncStart" HREF=":swi/astrt.html">, <SWIL SWI="PsiFS_AsyncPoll" HREF=":swi/apoll.html">, <SWIL SWI="PsiFS_AsyncControl" HREF=":swi/actrl.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
