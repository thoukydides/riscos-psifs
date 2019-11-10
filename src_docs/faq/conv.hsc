<*
    File        : faq/conv.hsc
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

<PAGE TITLE="Frequently Asked Questions" SUBTITLE="File Format Converters" PARENT=":faq/index.html" KEYWORDS="Frequently Asked Questions,Problems,File Converters,File Formats,Gert-Jan de Vos,Frodo Looijaard,Thomas Milius,Paul Vigay,Email">

<HEADING>How do I convert <SIBO> or <EPOC> files for use under <RISCOS>?</HEADING>
<PARA>
<PSIFS> does not include any file format converters for <SIBO> or <EPOC> files, but it does allow integration of third party extensions. See the documentation supplied with the individual file format converters for details.
<P>
Most <SIBO> and <EPOC> applications can import and export data in standard file formats, such as plain ASCII or CSV; these are suitable for exchanging data with <RISCOS> applications. Alternatively, a range of conversion software is available:
<UL>
    <LI><B><A HREF="http://www.mountside.nl/people/gertjan/changepsi/" TITLE="ChangePSI">ChangePSI</A></B> by <A HREF="mailto:gertjanv@mountside.nl" TITLE="Send Email to Gert-Jan de Vos">Gert-Jan de Vos</A> is a <I>freeware</I> file format conversion utility running under <RISCOS> that integrates with <PSIFS>.
    <LI><B><A HREF="http://home.t-online.de/home/thomas-milius/indexE.htm" TITLE="Psionconv">Psionconv</A></B> by <A HREF="mailto:Thomas-Milius@t-online.de" TITLE="Send Email to Thomas Milius">Thomas Milius</A> is another <I>freeware</I> <EPOC> file format conversion utility running under <RISCOS> that integrates with <PSIFS>.
    <LI><B><A HREF="http://home.t-online.de/home/thomas-milius/indexE.htm" TITLE="SIBOConv">SIBOConv</A></B> by <A HREF="mailto:Thomas-Milius@t-online.de" TITLE="Send Email to Thomas Milius">Thomas Milius</A> is a <I>freeware</I> <SIBO> file format conversion utility running under <RISCOS> that integrates with <PSIFS>.
    <LI><B><A HREF="http://www.vigay.com/riscos/psion.html" TITLE="ArcLink5">ArcLink5</A></B> by <A HREF="mailto:paul@vigay.com" TITLE="Send Email to Paul Vigay">Paul Vigay</A> is a <I>shareware</I> <EPOC> to <RISCOS> connectivity application that includes the ability to perform a few file format conversions.
    <LI><B><A HREF="http://www.interconnex.co.uk/riscos/psirisc/index.html" TITLE="PsiRisc">PsiRisc</A></B> by <A HREF="mailto:sales@interconnex.com" TITLE="Send Email to Interconnex">Interconnex</A> is a <I>commercial</I> <SIBO> and <EPOC> to <RISCOS> connectivity application that can also perform some file format conversions.
    <LI><B><PSIWIN></B>, supplied with most <EPOC> devices, can be run on a PC (or a PC card) to convert between a wider variety of common formats.
</UL>
<P>
Third parties interested in implementing additional file format converters may be interested in <A HREF="http://huizen.dds.nl/~frodol" TITLE="EPOC File Formats">details of <EPOC> file formats</A> produced by <A HREF="mailto:frodol@dds.nl" TITLE="Send Email to Frodo Looijaard">Frodo Looijaard</A>.
</PARA>

</PAGE>
