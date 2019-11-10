<*
    File        : fs/date.hsc
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

<PAGE TITLE="File System" SUBTITLE="Date and Times" PARENT=":fs/index.html" KEYWORDS="File System,Date Stamp">

<HEADING>Dates and Times</HEADING>
<PARA>
A direct mapping is performed between <RISCOS> and <SIBO> or <EPOC> dates and times. However, <SIBO> and <EPOC> only store file date stamps as an exact multiple of two seconds. Hence, copying a <RISCOS> file may result in the file date stamp being rounded down to give an even number of seconds. Date stamps are always accurately converted in the opposite direction.
</PARA>

</PAGE>
