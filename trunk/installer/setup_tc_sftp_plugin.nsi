; **********************************
; * Setup for SFTP-Plugin for TotalCommander
; * by Wolfram Esser (www.derwok.de)
; * 
; * get TotalCommander at http://www.ghisler.com
; *
; * get SFTP-Plugin at  http://developer.berlios.de/projects/sftp4tc/
; *
; * created with Nullsoft's Installer System NSIS
; * http://nsis.sourceforge.net
; * 
; **********************************
; v0.1 WOK - initial release
; v0.2 WOK - fixed problem with $WINDIR vs. Registry source for ini-location
; v0.3 MK  - changes to version 1.1.56.2
; ---------------------------------

SetCompressor LZMA
XPStyle on

VIAddVersionKey FileDescription "SFTP FS-plugin for TotalCommander setup"
VIAddVersionKey ProductName SFTP4TC
VIAddVersionKey ProductVersion 1.1.56.2
VIAddVersionKey FileVersion 1.1.56.2
VIAddVersionKey OriginalFilename wfx_sftp_1_1_56_2_setup.exe
VIAddVersionKey LegalCopyright ""

VIProductVersion 1.1.56.2

; The name of the installer
Name "SFTP FS-Plugin for TotalCommander(tm)"

; The setup-file to write
OutFile "wfx_sftp_1_1_56_2_setup.exe"


; The default installation directory
InstallDir c:\totalcmd\sftp_plugin

PageEx directory getdir
PageExEnd

Function getdir
FunctionEnd

; *************************************************************** DEFAULT-SETUP-SECTION
; The stuff to install
Section "" ;No components page, name is not important

  SetDetailsView show
  DetailPrint "SFTP FS-Plugin for TotalCommander Setup"

;TryRegistry:
  ;Get TC install direcory from Registry into $1 variable
  ClearErrors
  ReadRegStr $1 HKCU "Software\Ghisler\Total Commander" InstallDir
  IfErrors 0 RegGetIni
	; No luck in registry, so try to find wincmd.ini in Windows-Directory
  	Goto TryWinDir

  Goto TryWinDir
 
RegGetIni:
  ;Get TC ini location from Registry into $2 variable
  ClearErrors
  ReadRegStr $2 HKCU "Software\Ghisler\Total Commander" IniFileName
  IfErrors 0 MakeIniPath
	; No luck in registry, so try to find wincmd.ini in Windows-Directory 
  	Goto TryWinDir
MakeIniPath:
  ; Absolute path of ini-file from registry?
  IfFileExists $2 OKINIExists
  ; No, try to build absolute path from relative path into $2
    StrCpy $3 $1\$2
    StrCpy $2 $3
	Goto CheckIniExists
 
  

TryWinDir:
	StrCpy $2 $WINDIR\wincmd.ini
    ReadINIStr $1 $2 Configuration InstallDir
    StrCmp $1 "" 0 CheckIniExists

;    ; Also no luck to find wincmd.ini in $WINDIR...
;    MessageBox MB_OK|MB_ICONSTOP "ERROR: could not read from registry$\n     HKEY_CURRENT_USER\Software\Ghisler\Total Commander$\r$\nand could not read from $WINDIR\wincmd.ini$\n     [Configuration]InstallDir=$\n$\nNo TotalCommander installed?$\r$\nExiting!"
;	DetailPrint "Fatal Error: Exiting"
;    Goto ErrorExit
  


CheckIniExists:
  ; check, if wincmd.ini was found correctly
  IfFileExists $2 OKINIExists
  	MessageBox MB_OK|MB_ICONSTOP "ERROR: could not find wincmd.ini in registry or $WINDIR\wincmd.ini ($2)$\n$\nNo TotalCommander installed?$\r$\nExiting!"
	DetailPrint "Fatal Error: Exiting"
    Goto ErrorExit
 
OKINIExists:
  ; finally: check if TOTALCMD.EXE is there...
  IfFileExists $1\TOTALCMD.EXE OKTCExists
  	MessageBox MB_OK|MB_ICONSTOP "ERROR: could not find $1\TOTALCMD.EXE$\r$\nNo TotalCommander installed?$\r$\nExiting!"
	DetailPrint "Fatal Error: Exiting" 
  	Goto ErrorExit

OKTCExists:
  ; Last chance to STOP!
	MessageBox MB_YESNO|MB_ICONQUESTION 'Would you really like to install the SFTP-Plugin to directory$\r$\n"$1" (patch ini: $2)?$\r$\n(No warranty for this setup-program!)' IDYES YesInstall
      DetailPrint "User abort. Nothing installed. Exiting."
      Goto ErrorExit


; OK - let's do it now...
YesInstall:
  ; Are there any TC windows open?
  FindWindow $5 "TTOTAL_CMD"
  ; MessageBox MB_OK|MB_ICONEXCLAMATION $5
  StrCmp $5 "0" 0 NeedClosing
    ; MessageBox MB_OK|MB_ICONEXCLAMATION "NO Window!!"
    Goto ClosedAllTCs
 
NeedClosing:
  ; Automatic closing of TC?
  MessageBox MB_YESNO|MB_ICONQUESTION 'Closing of all TotalCommanders is needed.$\nShould all TotalCommanders be closed NOW (one will be restarted later)?' IDYES AutomaticTCclosing
      DetailPrint "Fatal Error: TotalCommander still running. Exiting."
      Goto ErrorExit

AutomaticTCclosing:
  DetailPrint "Automatic closing of TotalCommanders"

CloseAllTCWindows:
  FindWindow $5 "TTOTAL_CMD"
  ; MessageBox MB_OK|MB_ICONEXCLAMATION $5
  StrCmp $5 "0" 0 FoundWindow
    ; MessageBox MB_OK|MB_ICONEXCLAMATION "NO Window!!"
    Goto ClosedAllTCs
 
FoundWindow:
  ; MessageBox MB_OK|MB_ICONEXCLAMATION "Found Window!!"
  ; Send a WM_CLOSE to this instance
  SendMessage $5 16 0 0
  Sleep 250
  Goto CloseAllTCWindows

ClosedAllTCs:
 
  ; Set output path to the TC-installation directory.
  SetOutPath $1

  ; Put all files (recursively) there
  File ..\tc_plugin\Release\plugin_sftp.wfx
  File ..\tc_plugin\Release\psftp.dll
  File .\Licence.putty

; promote Plugin to TC via TC's INI file
IfFileExists $2 IniAbsolutePath
	; we need to prepend the TC directory
	WriteINIStr $OUTDIR\$2 FileSystemPlugins "Secure FTP Connections" $OUTDIR\plugin_sftp.wfx
	FlushINI $OUTDIR\$2
	Goto AfterIniChange
IniAbsolutePath:
	; we have an absolute path to TC's ini file
	WriteINIStr $2 FileSystemPlugins "Secure FTP Connections" $OUTDIR\plugin_sftp.wfx
	FlushINI $2
AfterIniChange:

 ; write uninstall strings
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TotalCommanderSFTP" "DisplayName" "TotalCommander SFTP-Plugin (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TotalCommanderSFTP" "UninstallString" '"$OUTDIR\tc_sftp_uninstaller.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TotalCommanderSFTP" "DisplayIcon" '"$OUTDIR\TCUNINST.EXE"'
  WriteUninstaller "$OUTDIR\sftp4tc_uninstaller.exe"


  ExecShell "" $OUTDIR\TOTALCMD.EXE
  Sleep 1500
  BringToFront
  MessageBox MB_OK|MB_ICONEXCLAMATION "You can access SFTP via 'Network Neighborhood / Secure FTP Connections'$\r$\n$\r$\nHave fun!$\r$\n$\r$\n                Setup by Wolfram Esser (www.derwok.de)$\r$\n                SFTP-Plugin by Hans-Juergen Petrich (petrich@tronic-media.com)$\n                and by Martin Kanich (kanci@users.berlios.de)$\n                TotalCommander by Christian Ghisler (www.ghisler.com)"
  Goto ShowNotes

ShowNotes:
;  DetailPrint "Showing install notes"
;  ExecShell "" notepad.exe $OUTDIR\sftp_plugin\install.txt SW_SHOWNORMAL

ErrorExit:

SectionEnd ; end the section



; *************************************************************** UNINSTALL-SECTION
Section "Uninstall"
  SetDetailsView show
  DetailPrint "Uninstall SFPT-Plugin Setup for TotalCommander"
 
;TryRegistry:
  ;Get TC install direcory from Registry into $1 variable
  ClearErrors
  ReadRegStr $1 HKCU "Software\Ghisler\Total Commander" InstallDir
  IfErrors 0 RegGetIni
	; No luck in registry, so try to find wincmd.ini in Windows-Directory
  	Goto TryWinDir
 
RegGetIni:
  ;Get TC ini location from Registry into $2 variable
  ClearErrors
  ReadRegStr $2 HKCU "Software\Ghisler\Total Commander" IniFileName
  IfErrors 0 MakeIniPath
	; No luck in registry, so try to find wincmd.ini in Windows-Directory 
  	Goto TryWinDir
MakeIniPath:
  ; Absolute path of ini-file from registry?
  IfFileExists $2 Ask2Uninstall
  ; No, try to build absolute path from relative path into $2
    StrCpy $3 $1\$2
    StrCpy $2 $3
	Goto Ask2Uninstall

TryWinDir:
    ; Maybe we find the wincmd.ini in $WINDIR?
	StrCpy $2 $WINDIR\wincmd.ini
    ReadINIStr $1 $2 Configuration InstallDir
    StrCmp $1 "" 0 Ask2Uninstall

    ; Also no luck to find wincmd.ini in $WINDIR...
    MessageBox MB_OK|MB_ICONSTOP "ERROR: could not read from registry$\n     HKEY_CURRENT_USER\Software\Ghisler\Total Commander$\r$\nand could not read from $WINDIR\wincmd.ini$\n     [Configuration]InstallDir=$\n$\nNo TotalCommander installed?$\r$\nExiting!"
	DetailPrint "Fatal Error: Exiting"
    Goto ErrorExit

Ask2Uninstall:
  ; Last chance to STOP!
	MessageBox MB_YESNO|MB_ICONQUESTION 'Would you really like to UNINSTALL the SFTP-Plugin from directory$\r$\n$1?$\nALL TotalCommanders WILL BE CLOSED!!!' IDYES YesUnInstall
      DetailPrint "No uninstalling. User abort."
      Goto ErrorExit
YesUnInstall:

;AutomaticTCclosing:
  DetailPrint "Automatic closing of all TotalCommanders"
CloseAllTCWindows:
  FindWindow $5 "TTOTAL_CMD"
  StrCmp $5 "0" 0 FoundWindow
    ; No window found
    Goto ClosedAllTCs
 
FoundWindow:
  ; Send a WM_CLOSE to this instance
  SendMessage $5 16 0 0
  Sleep 250
  Goto CloseAllTCWindows

ClosedAllTCs:
  ; Now do cleanup!
  Delete $1\plugin_sftp.wfx
  Delete $1\psftp.dll
  Delete $1\licence.putty
  Delete $1\sftp4tc_uninstaller.exe

  DeleteINIStr $2 FileSystemPlugins "Secure FTP Connections"
  FlushINI $2
 
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TotalCommanderSFTP"

ErrorExit:

SectionEnd

