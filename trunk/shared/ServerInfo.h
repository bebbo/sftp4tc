#ifndef _SERVER_INFO_H
#define _SERVER_INFO_H

#include "share.h"

#define INI_CONFIG_IMPORT_PUTTY_SSH_SESS "import_putty_ssh_sessions"
#define INI_CONFIG_IMPORT_SSHCOM_SSH_SESS "import_sshcom_ssh_sessions"
#define INI_CONFIG_SECTION_NAME "config"
#define INI_CONFIG_USE_PASSWORD_CRYPTER "passwd_crypter"

void CopyServerInfo(SftpServerAccountInfo* ServerInfos, int SourceIndex, int DestIndex);
void SetDefaultsToServerInfo(SftpServerAccountInfo* ServerInfo);
void SaveServerInfo(int SectionNumber, struct SftpServerAccountInfo *ServerInfo, 
                    struct config_properties *aProperties);

#endif //_SERVER_INFO_H
