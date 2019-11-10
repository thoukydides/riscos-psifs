<*
    File        : faq/drvrs.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="!SerialDev Not Seen" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Block Driver,InternalPC,SP_DualPC,!SerialDev">

<HEADING>Why are <ARG>InternalPC</ARG> and <ARG>SP_DualPC</ARG> the only serial blockdrivers offered?</HEADING>
<PARA>
<PSIFS> is supplied with copies of the <ARG>InternalPC</ARG> and <ARG>SP_DualPC</ARG> serial block drivers. These two blockdrivers support most of the common configurations.
<P>
To use other blockdrivers, for example if an unusual serial port card is being used, then <ARG>!SerialDev</ARG> must be seen by the filer before <PSIFS> is loaded. The easiest way to do this on a machine running <RISCOS VER="3.5"> or later is to place it in the <ARG>!Boot.Resources</ARG> directory.
<P>
Please note that <ARG>!SerialDev</ARG> is not distributed with <PSIFS>; it must be obtained from another source.
</PARA>

</PAGE>
