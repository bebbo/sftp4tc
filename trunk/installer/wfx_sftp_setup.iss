[Setup]
InternalCompressLevel=ultra
SourceDir=..\tc_plugin\release
OutputBaseFilename=wfx_sftp_1_1_56_2_setup
SolidCompression=true
VersionInfoVersion=1.1.56.2
VersionInfoDescription=SFTP FS-plugin for TotalCommander setup
Compression=lzma/ultra
AppName=SFTP FS-plugin for TotalCommander
AppVerName=1.1.56.2
DefaultDirName={pf}\TC Plugins\SFTP4TC
UsePreviousGroup=false
AppendDefaultGroupName=false
AlwaysShowComponentsList=false
DisableReadyPage=true
ShowLanguageDialog=no
LanguageDetectionMethod=none
AppPublisherURL=http://developer.berlios.de/projects/sftp4tc/
AppVersion=1.1.56.2
AppID={{C52CD225-E994-499C-89B8-457E846E0CF5}
UninstallDisplayName=SFTP for TotalCommander
OutputDir=..\..\installer
SetupIconFile=..\SFTP.ICO
UninstallIconFile=..\SFTP.ICO

[Files]
Source: plugin_sftp.wfx; DestDir: {app}; Components: SFTP_Plugin
Source: psftp.dll; DestDir: {app}; Components: SFTP_Plugin
Source: wfx_sftp_cfg.dll; DestDir: {app}; Components: SFTP_CFG
Source: ..\..\properties_dlg\wfx_sftp_cfg.xrc; DestDir: {app}\rc; Components: SFTP_CFG
Source: ..\..\..\password_crypter\src\Static_Release\password_crypter.dll; DestDir: {app}; Components: PASSWORD_CRYPTER
Source: \wxWidgets\lib\wxmsw24.dll; DestDir: {sys}; Flags: promptifolder; Components: support_files
Source: ..\..\..\vcredist\msvcr71.dll; DestDir: {sys}; Flags: restartreplace uninsneveruninstall sharedfile; Components: support_files
Source: ..\..\..\vcredist\msvcrt.dll; DestDir: {sys}; Flags: restartreplace uninsneveruninstall sharedfile; Components: support_files
Source: C:\eclipse3\workspace\TC Plugin Installer\bin\TC_Plug_Installer.exe; DestDir: {app}; Components: SFTP_Plugin

[Components]
Name: SFTP_Plugin; Description: SFTP Plugin files; Flags: fixed; Types: custom compact full
Name: SFTP_CFG; Description: Configuration dialog; Types: custom full
Name: PASSWORD_CRYPTER; Description: Password crypter; Types: full custom
Name: support_files; Description: Runtime support files; Types: full

[Run]
Filename: {app}\plugin_sftp.wfx; Description: "You need ""TC Plugman"" to handle .wfx files, to integrate this plugin with Total Commander ""automaticaly"""; StatusMsg: running SFTP Plugin; Flags: postinstall unchecked shellexec
Filename: {app}\TC_Plug_Installer.exe; Parameters: "-user ""Secure FTP Connections"" ""{app}\plugin_sftp.wfx"""; Description: Install for this user (depends on Total Commander's installation); Flags: skipifdoesntexist postinstall runhidden; Components: SFTP_Plugin
Filename: {app}\TC_Plug_Installer.exe; Parameters: "-common ""Secure FTP Connections"" ""{app}\plugin_sftp.wfx"""; Description: Install for any user (depends on Total Commander's installation); Flags: skipifdoesntexist postinstall runhidden; Components: SFTP_Plugin
