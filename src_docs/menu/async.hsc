<*
    File        : menu/async.hsc
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

<PAGE TITLE="Menu Entries" SUBTITLE="Action Windows" PARENT=":menu/index.html" KEYWORDS="Action Windows,Asynchronous Remote Operation,Backup">

<HEADING>Actions</HEADING>
<PARA>
Many operations within <PSIFS> are controlled via windows that resemble the <NAME>Filer&nbsp;action</NAME> windows used for copying files normally. A short description of the operation is displayed in the title bar of the window:
<UL>
    <LI><ARG>PsiFS closing files</ARG> - Closing open files to prepare for a backup.
    <LI><ARG>PsiFS opening files</ARG> - Re-opening previously closed files after a backup has been completed.
    <LI><ARG>PsiFS backup <ARGU>drive</ARGU></ARG> - Performing a backup of the files from <ARG><ARGU>drive</ARGU></ARG>.
</UL>
<P>
The top section of the window displays details of the current activity, including the name of the file being manipulated. Below this is displayed the total time taken by the operation and an estimate of the time remaining.
<P>
Up to four different buttons may appear at the bottom of the window:
<UL>
    <LI><B>OK</B> - Acknowledge the displayed information and close the window.
    <LI><B>Abort</B> - Abort the operation. This attempts to tidy up first, for example by deleting any temporary files.
    <LI><B>Pause</B> - Pause the operation. Click <ARG>Continue</ARG> to resume the operation.
    <LI><B>Continue</B> - Resume an operation after clicking <ARG>Pause</ARG>.
    <LI><B>Yes</B> - Give an affirmative answer to a question.
    <LI><B>No</B> - Give a negative answer to a question.
    <LI><B>Quiet</B> - Automatically selects the default answer to this and future questions at the current or lower levels.
    <LI><B>Skip</B> - Skip over the current object.
    <LI><B>Restart</B> - Restart the operation on this object.
    <LI><B>Retry</B> - Retry the activity that failed.
    <LI><B>Copy</B> - Copy the current file from the <SIBO> or <EPOC> device.
</UL>
The available buttons depend on the status of the operation being performed.
</PARA>

</PAGE>
