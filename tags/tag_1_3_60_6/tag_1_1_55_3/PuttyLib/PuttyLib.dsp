# Microsoft Developer Studio Project File - Name="PuttyLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PuttyLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PuttyLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PuttyLib.mak" CFG="PuttyLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PuttyLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PuttyLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PuttyLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PUTTYLIB_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\PuttyLib" /I "..\PuttyLib\putty" /I "..\tc_plugin" /I "..\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PUTTYLIB_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 WSOCK32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:1.0 /dll /machine:I386 /out:"../tc_plugin/Release/psftp.dll"

!ELSEIF  "$(CFG)" == "PuttyLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PUTTYLIB_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gi /GX /Zi /Od /I "..\PuttyLib" /I "..\PuttyLib\putty" /I "..\tc_plugin" /I "..\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PUTTYLIB_EXPORTS" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 WSOCK32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:1.0 /dll /map /debug /machine:I386 /out:"../tc_plugin/Debug/psftp.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "PuttyLib - Win32 Release"
# Name "PuttyLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\putty\BE_ALL.C
# End Source File
# Begin Source File

SOURCE=.\putty\CMDLINE.C
# End Source File
# Begin Source File

SOURCE=.\putty\CONSOLE.C
# End Source File
# Begin Source File

SOURCE=.\putty\IMPORT.C
# End Source File
# Begin Source File

SOURCE=.\putty\INT64.C
# End Source File
# Begin Source File

SOURCE=.\putty\LOGGING.C
# End Source File
# Begin Source File

SOURCE=.\putty\MISC.C
# End Source File
# Begin Source File

SOURCE=.\putty\NOISE.C
# End Source File
# Begin Source File

SOURCE=.\putty\PAGEANTC.C
# End Source File
# Begin Source File

SOURCE=.\pl_misc.c
# End Source File
# Begin Source File

SOURCE=.\pl_psftp.c
# End Source File
# Begin Source File

SOURCE=.\putty\PORTFWD.C
# End Source File
# Begin Source File

SOURCE=.\putty\pproxy.c
# End Source File
# Begin Source File

SOURCE=.\putty\PROXY.C
# End Source File
# Begin Source File

SOURCE=.\putty\PSFTP.C
# End Source File
# Begin Source File

SOURCE=.\PuttyLib.cpp
# End Source File
# Begin Source File

SOURCE=.\PuttyLib.def
# End Source File
# Begin Source File

SOURCE=.\PuttyLib.rc
# End Source File
# Begin Source File

SOURCE=.\putty\RAW.C
# End Source File
# Begin Source File

SOURCE=.\putty\RLOGIN.C
# End Source File
# Begin Source File

SOURCE=.\putty\SETTINGS.C
# End Source File
# Begin Source File

SOURCE=.\putty\SFTP.C
# End Source File
# Begin Source File

SOURCE=..\shared\share.c
# End Source File
# Begin Source File

SOURCE=.\putty\SSH.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHAES.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHBLOWF.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHBN.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHCRC.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHCRCDA.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHDES.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHDH.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHDSS.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHDSSG.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHMD5.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHPRIME.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHPUBK.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHRAND.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHRSA.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHRSAG.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHSH512.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHSHA.C
# End Source File
# Begin Source File

SOURCE=.\putty\SSHZLIB.C
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\putty\TELNET.C
# End Source File
# Begin Source File

SOURCE=.\putty\TREE234.C
# End Source File
# Begin Source File

SOURCE=.\putty\UNICODE.C
# End Source File
# Begin Source File

SOURCE=.\putty\VERSION.C
# End Source File
# Begin Source File

SOURCE=.\putty\WCWIDTH.C
# End Source File
# Begin Source File

SOURCE=.\putty\WILDCARD.C
# End Source File
# Begin Source File

SOURCE=.\putty\windefs.c
# End Source File
# Begin Source File

SOURCE=.\putty\winmisc.c
# End Source File
# Begin Source File

SOURCE=.\putty\WINNET.C
# End Source File
# Begin Source File

SOURCE=.\putty\winsftp_cutdown.c
# End Source File
# Begin Source File

SOURCE=.\putty\WINSTORE.C
# End Source File
# Begin Source File

SOURCE=.\putty\WINUTILS.C
# End Source File
# Begin Source File

SOURCE=.\putty\X11FWD.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\putty\INT64.H
# End Source File
# Begin Source File

SOURCE=.\putty\MISC.H
# End Source File
# Begin Source File

SOURCE=.\putty\NETWORK.H
# End Source File
# Begin Source File

SOURCE=.\pl_consts.h
# End Source File
# Begin Source File

SOURCE=.\pl_misc.h
# End Source File
# Begin Source File

SOURCE=.\putty\PROXY.H
# End Source File
# Begin Source File

SOURCE=.\putty\PUTTY.H
# End Source File
# Begin Source File

SOURCE=.\PuttyLib.h
# End Source File
# Begin Source File

SOURCE=.\putty\PUTTYMEM.H
# End Source File
# Begin Source File

SOURCE=.\putty\RESOURCE.H
# End Source File
# Begin Source File

SOURCE=.\putty\SFTP.H
# End Source File
# Begin Source File

SOURCE=..\shared\share.h
# End Source File
# Begin Source File

SOURCE=.\putty\SSH.H
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\putty\STORAGE.H
# End Source File
# Begin Source File

SOURCE=.\putty\TREE234.H
# End Source File
# Begin Source File

SOURCE=.\putty\WIN_RES.H
# End Source File
# Begin Source File

SOURCE=.\putty\WINSTUFF.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
