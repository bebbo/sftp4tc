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
Source: ..\..\..\password_crypter\Static_Release\password_crypter.dll; DestDir: {app}; Components: PASSWORD_CRYPTER
Source: \wxWidgets\lib\wxmsw24.dll; DestDir: {sys}; Flags: promptifolder; Components: support_files
Source: ..\..\..\vcredist\msvcr71.dll; DestDir: {sys}; Flags: restartreplace uninsneveruninstall sharedfile; Components: support_files
Source: ..\..\..\vcredist\msvcrt.dll; DestDir: {sys}; Flags: restartreplace uninsneveruninstall sharedfile; Components: support_files
[Components]
Name: SFTP_Plugin; Description: SFTP Plugin files; Flags: fixed; Types: custom compact full
Name: SFTP_CFG; Description: Configuration dialog; Types: custom full
Name: PASSWORD_CRYPTER; Description: Password crypter; Types: full custom
Name: support_files; Description: Runtime support files; Types: full
