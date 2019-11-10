<*
    File        : faq/modem.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Online Media Internal Modem" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Online Media,Internal Modem,SWI &amp;83901 not known,Block Driver,!SerialDev">

<HEADING>Why is the error <ARG>SWI &amp;83901 not known</ARG> displayed when using <ARG>*PsiFSListDrivers</ARG>?</HEADING>
<PARA>
The serial block driver for <NAME>Online Media</NAME> internal modem cards is badly written. Any attempt to initialise the block driver on a machine without the card fitted results in an error that bypasses the normal error handling mechanism. This prevents <CMDL CMD="PsiFSListDrivers" HREF=":command/list.html"> from listing all of the available drivers.
<P>
To avoid this problem, open the <ARG>!SerialDev.Modules</ARG> directory, and remove the <ARG>IntModem</ARG> directory.
</PARA>

</PAGE>
