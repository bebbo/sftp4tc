#include "ConfigProperties.h"
#include "ServerInfo.h"

void SaveProperties(struct config_properties *aProperties)
{
  char buf[MAX_Server_INFO];
  sprintf(buf, "%d", aProperties->DoImportPuttySessions-'0');
  WritePrivateProfileString(INI_CONFIG_SECTION_NAME, INI_CONFIG_IMPORT_PUTTY_SSH_SESS, buf,
    aProperties->ConfigIniFile);
  WritePrivateProfileString(INI_CONFIG_SECTION_NAME, INI_CONFIG_IMPORT_SSHCOM_SSH_SESS, 
    aProperties->DoImportSSHcomSessions, aProperties->ConfigIniFile);

  int aid=1;
  for(int i=0; i < aProperties->ServerCount; i++) {
    if (!aProperties->ServerInfos[i].is_imported_from_any_datasrc)
    {
      if (aProperties->ServerInfos[i].id!=-1)
        SaveServerInfo(aid++, &aProperties->ServerInfos[i], 
          aProperties->ConfigIniFile);
    }
  }
  for(int i=0; i < aProperties->ServerCount; i++) {
    if (!aProperties->ServerInfos[i].is_imported_from_any_datasrc)
    {
      if (aProperties->ServerInfos[i].id==-1) {
        sprintf(buf, "%d", aid++);
        WritePrivateProfileSection(buf, "", aProperties->ConfigIniFile);
      }
    }
  }
}
