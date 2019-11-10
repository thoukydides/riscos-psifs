# PsiFS #

:warning: **The author is no longer developing for RISC OS, so this code is provided as-is without support.** :warning:

PsiFS is a filing system that provides access via a serial link to files stored on a SIBO or EPOC computer, such as a Psion Series 5, allowing them to be accessed just like files on a native disc. The standard Psion Link Protocol (PLP) is used; there is no extra software to install or run on the remote machine. It also supports printing from EPOC devices, using either the EPOC or RISC OS printer drivers. Other services provided include clipboard integration and battery monitoring. A plug-in interface enables third-party conversion utilities to be transparently invoked.

These source files have been rearranged to make them more convenient to navigate and view on non RISC OS platforms. Most significantly, instead of the RISC OS convention of using separate directories for different source file types, more traditional file extensions have been used. BBC BASIC files have also been detokenised to plain text. The Makefile has not been updated to match these changes, so the code will not build without some modification.

***
<sup> Copyright 1998-2002, 2019
