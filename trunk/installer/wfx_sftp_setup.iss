[Setup]
InternalCompressLevel=ultra
SourceDir=..\tc_plugin\release
OutputBaseFilename=wfx_sftp_1_2_56_1_setup
SolidCompression=true
VersionInfoVersion=1.2.56.1
VersionInfoDescription={cm:VersionInfoDesc}
Compression=lzma/ultra
AppName={cm:AppName}
AppVerName=1.2.56.1
DefaultDirName={pf}\TC Plugins\SFTP4TC
UsePreviousGroup=false
AppendDefaultGroupName=false
AlwaysShowComponentsList=false
DisableReadyPage=true
ShowLanguageDialog=no
LanguageDetectionMethod=none
AppPublisherURL=http://developer.berlios.de/projects/sftp4tc/
AppVersion=1.2.56.1
AppID={{C52CD225-E994-499C-89B8-457E846E0CF5}
UninstallDisplayName=SFTP for TotalCommander
OutputDir=..\..\installer
SetupIconFile=..\SFTP.ICO
ChangesEnvironment=yes

[Languages]
Name: en; MessagesFile: compiler:Default.isl

[Messages]
en.BeveledLabel=English

[CustomMessages]
en.all_users_setup_tc=for all users
en.curr_user_setup_tc=for current user
en.setup_tc_group_desc=Install with Total Commander
en.VersionInfoDesc=SFTP FS-plugin for TotalCommander setup
en.AppName=SFTP FS-plugin for TotalCommander
en.PluginFilesDesc=SFTP Plugin files
en.ConfigDialog=Configuration dialog
en.PasswordCrypter=Password crypter
en.RuntimeSupportFiles=Runtime support files
en.Error=Total Commander's INI file (common for all users) was not found!%nPlease install this plugin manually.

[Files]
Source: plugin_sftp.wfx; DestDir: {app}; Components: SFTP_Plugin; AfterInstall: AfterPluginInstall
Source: psftp.dll; DestDir: {app}; Components: SFTP_Plugin
Source: wfx_sftp_cfg.dll; DestDir: {app}; Components: SFTP_CFG
Source: ..\..\properties_dlg\wfx_sftp_cfg.xrc; DestDir: {app}\rc; Components: SFTP_CFG
Source: ..\..\..\password_crypter\src\Static_Release\password_crypter.dll; DestDir: {app}; Components: PASSWORD_CRYPTER; AfterInstall: AfterPWDCrypterInstall
Source: \wxWidgets\lib\wxmsw24.dll; DestDir: {sys}; Flags: promptifolder; Components: support_files
Source: ..\..\..\vcredist\msvcr71.dll; DestDir: {sys}; Flags: restartreplace uninsneveruninstall sharedfile; Components: support_files
Source: ..\..\..\vcredist\msvcrt.dll; DestDir: {sys}; Flags: restartreplace uninsneveruninstall sharedfile; Components: support_files

[Components]
Name: SFTP_Plugin; Description: {cm:PluginFilesDesc}; Flags: fixed; Types: custom compact full
Name: SFTP_CFG; Description: {cm:ConfigDialog}; Types: custom full
Name: PASSWORD_CRYPTER; Description: {cm:PasswordCrypter}; Types: full custom
Name: support_files; Description: {cm:RuntimeSupportFiles}; Types: full

[Tasks]
Name: all_users_setup_tc; Description: {cm:all_users_setup_tc}; GroupDescription: setup_tc_group_desc; Flags: checkedonce; Components: SFTP_Plugin
Name: curr_user_setup_tc; Description: {cm:curr_user_setup_tc}; GroupDescription: setup_tc_group_desc; Flags: checkedonce; Components: SFTP_Plugin

[Code]
const
  FS_PLUGIN_SECTION = 'FileSystemPlugins';
  PLUGIN_NAME = 'Secure FTP Connections';
  TC_KEY = 'SOFTWARE\Ghisler\Total Commander';
  INI_KEY = 'IniFileName';

  SFTP_INI = 'wcx_sftp.ini';
  SFTP_KEY = 'Software\petrich\tc_sftp_plugin';
  SFTP_INI_KEY = 'SFtpIniName';
  PWD_SECTION = 'config';
  PWD_INI_KEY = 'passwd_crypter';

procedure AfterPluginInstall;
var
  user_ini, ini: string;
  found: boolean;
begin
  if IsTaskSelected('all_users_setup_tc') then begin
   	if RegQueryStringValue(HKEY_LOCAL_MACHINE, TC_KEY, INI_KEY, ini) then begin
	  found := true;
	  ini := ExpandFileName(ini);
    end else begin
      found := false;
    end;

    if found then begin
      SetIniString(FS_PLUGIN_SECTION, PLUGIN_NAME, ExpandConstant('{app}')+'\plugin_sftp.wfx', ini);
    end else begin
      MsgBox(ExpandConstant('{cm:Error}'), mbError, MB_OK);
    end;
  end else begin
    ini := '';
  end;

  if IsTaskSelected('curr_user_setup_tc') then begin
   	if RegQueryStringValue(HKEY_CURRENT_USER, TC_KEY, INI_KEY, user_ini) then begin
 	  found := true;
	  ini := ExpandFileName(user_ini);
    end else begin
      found := false;
    end;

    if ini<>user_ini then begin
      if found then begin
        SetIniString(FS_PLUGIN_SECTION, PLUGIN_NAME, ExpandConstant('{app}')+'\plugin_sftp.wfx', user_ini);
      end else begin
        MsgBox(ExpandConstant('{cm:Error}'), mbError, MB_OK);
      end;
    end;
  end;
end;

procedure AfterPWDCrypterInstall;
var
  user_ini, ini: string;
begin
  if RegQueryStringValue(HKEY_CURRENT_USER, SFTP_KEY, SFTP_INI_KEY, user_ini) then begin
  	user_ini := ExpandFileName(user_ini);
    SetIniString(PWD_SECTION, PWD_INI_KEY, ExpandConstant('{app}')+'\password_crypter.dll', user_ini);
  end else begin
    user_ini := '';
  end;

  ini := ExpandFileName(ExpandConstant('{app}')+'\'+SFTP_INI);
  if ini<>user_ini then
      SetIniString(PWD_SECTION, PWD_INI_KEY, ExpandConstant('{app}')+'\password_crypter.dll', ini);
end;
