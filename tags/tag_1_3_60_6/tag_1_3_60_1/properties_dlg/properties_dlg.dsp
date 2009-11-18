# Microsoft Developer Studio Project File - Name="properties_dlg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=properties_dlg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "properties_dlg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "properties_dlg.mak" CFG="properties_dlg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "properties_dlg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "properties_dlg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "properties_dlg - Win32 Static Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "properties_dlg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "properties_dlg___Win32_Release"
# PROP BASE Intermediate_Dir "properties_dlg___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "properties_dlg___Win32_Release"
# PROP Intermediate_Dir "properties_dlg___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PROPERTIES_DLG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\shared\\" /I "$(WXWIDGETS)\lib\msw" /I "$(WXWIDGETS)\contrib\include" /I "..\PuttyLib\putty" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "PROPERTIES_DLG_EXPORTS" /D "__WXMSW__" /D WXUSINGDLL=1 /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxxrc24.lib wxmsw24.lib comctl32.lib WS2_32.Lib /nologo /dll /machine:I386 /out:"../tc_plugin/Release/wfx_sftp_cfg.dll"

!ELSEIF  "$(CFG)" == "properties_dlg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "properties_dlg___Win32_Debug"
# PROP BASE Intermediate_Dir "properties_dlg___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "properties_dlg___Win32_Debug"
# PROP Intermediate_Dir "properties_dlg___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PROPERTIES_DLG_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\shared\\" /I "$(WXWIDGETS)\lib\mswd" /I "$(WXWIDGETS)\contrib\include" /I "..\PuttyLib\putty" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "PROPERTIES_DLG_EXPORTS" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "__WXMSW__" /D WXUSINGDLL=1 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxxrc24d.lib wxmsw24d.lib comctl32.lib WS2_32.Lib RpcRT4.Lib /nologo /dll /debug /machine:I386 /out:"../tc_plugin/Debug/wfx_sftp_cfg.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "properties_dlg - Win32 Static Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "properties_dlg___Win32_Static_Release"
# PROP BASE Intermediate_Dir "properties_dlg___Win32_Static_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "properties_dlg___Win32_Static_Release"
# PROP Intermediate_Dir "properties_dlg___Win32_Static_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\shared\\" /I "$(WXWIDGETS)\lib\msw" /I "$(WXWIDGETS)\contrib\include" /I "..\PuttyLib\putty" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "PROPERTIES_DLG_EXPORTS" /D "__WXMSW__" /D WXUSINGDLL=1 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\shared\\" /I "$(WXWIDGETS)\lib\msw" /I "$(WXWIDGETS)\contrib\include" /I "..\PuttyLib\putty" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "PROPERTIES_DLG_EXPORTS" /D "__WXMSW__" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxxrc24.lib wxmsw24.lib comctl32.lib WS2_32.Lib /nologo /dll /machine:I386 /out:"../tc_plugin/Release/wfx_sftp_cfg.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxxrc.lib wxmsw.lib comctl32.lib WS2_32.Lib /nologo /dll /machine:I386 /out:"../tc_plugin/Release/wfx_sftp_cfg.dll"

!ENDIF 

# Begin Target

# Name "properties_dlg - Win32 Release"
# Name "properties_dlg - Win32 Debug"
# Name "properties_dlg - Win32 Static Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\shared\ConfigProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\PreferencesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\properties_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\properties_dlg.def
# End Source File
# Begin Source File

SOURCE=.\properties_dlg.rc
# End Source File
# Begin Source File

SOURCE=..\shared\ServerInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionConfigDialog.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\PreferencesDialog.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SessionConfigDialog.h
# End Source File
# Begin Source File

SOURCE=.\wxmyapp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
