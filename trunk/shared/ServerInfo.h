#ifndef _SERVER_INFO_H
#define _SERVER_INFO_H

#include "sftp4tc_share.h"

#define INI_CONFIG_IMPORT_PUTTY_SSH_SESS "import_putty_ssh_sessions"
#define INI_CONFIG_IMPORT_SSHCOM_SSH_SESS "import_sshcom_ssh_sessions"
#define INI_CONFIG_CACHE_FS "cache_fs"
#define INI_CONFIG_SECTION_NAME "config"
#define INI_CONFIG_USE_PASSWORD_CRYPTER "passwd_crypter"
#define INI_CONFIG_TEST_PASSWORD "passwd_crypter_testpwd"
#define INI_CONFIG_TEST_PASSWORD_TEST "wef ie3wf2b jfbiu sdk%fb6sd#fhe67lkajlc,sm"

void CopyServerInfo(SftpServerAccountInfo* ServerInfos, int SourceIndex, int DestIndex);
void SetDefaultsToServerInfo(SftpServerAccountInfo* ServerInfo);
void SaveServerInfo(int SectionNumber, struct SftpServerAccountInfo *ServerInfo, 
                    ConfigPropertiesType *aProperties);

#endif //_SERVER_INFO_H
