<*
    File        : faq/dst.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="Summer Time" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,Summer Time,Date Stamp,Backup">

<HEADING>Why does <PSIFS> copy all the files in a backup when summer time starts or ends?</HEADING>
<PARA>
<RISCOS> and <EPOC> treat file time stamps differently. Under <RISCOS> time stamps are stored in Universal Time Coordinated (UTC) and converted to local time for display, and <EPOC> always uses local time. This results in the displayed time changing under <RISCOS> but not under <EPOC> when the time zone or summer time setting changes.
<P>
When <PSIFS> converts time stamps between the two formats it attempts to keep the displayed times the same. Hence, files that have been copied to <RISCOS> prior to changing the time zone or summer time settings will have different time stamps to those copied after the change. This has the unfortunate side-effect of making any files in a backup appear either older or newer than those on the <EPOC> device, requiring all of the files to be copied again even if they are unchanged.
<P>
When summer time ends, i.e. the clocks go back, <PSIFS> automatically copies all of the files to refresh the backup. However, when summer time starts, i.e. the clocks go forward, <PSIFS> displays the message <ARG>Previous backup is newer</ARG>. In this case clicking on <ARG>Quiet</ARG> results in all the files being copied.
</PARA>

</PAGE>
