#include "ServerInfo.h"
#include "putty_proxy.h"

void CopyServerInfo(SftpServerAccountInfo* ServerInfos, int SourceIndex, int DestIndex)
{
  //cached part isn't important now
  //imported connections aren't here, so imported fields aren't relevant
  ServerInfos[DestIndex].id = DestIndex;
  _snprintf(ServerInfos[DestIndex].title, MAX_Server_INFO, "copy of %s", 
    ServerInfos[SourceIndex].title);
  strncpy(ServerInfos[DestIndex].username, ServerInfos[SourceIndex].username, MAX_Server_INFO);
  strncpy(ServerInfos[DestIndex].password, ServerInfos[SourceIndex].password, MAX_Server_INFO);
  strncpy(ServerInfos[DestIndex].passphrase, ServerInfos[SourceIndex].passphrase, MAX_Server_INFO);
  strncpy(ServerInfos[DestIndex].host, ServerInfos[SourceIndex].host, MAX_Server_INFO);
  strncpy(ServerInfos[DestIndex].home_dir, ServerInfos[SourceIndex].home_dir, MAX_Server_INFO);
  strncpy(ServerInfos[DestIndex].keyfilename, 
    ServerInfos[SourceIndex].keyfilename, MAX_Server_INFO);
  ServerInfos[DestIndex].port = ServerInfos[SourceIndex].port;
  ServerInfos[DestIndex].chmod_value_put = ServerInfos[SourceIndex].chmod_value_put;
  ServerInfos[DestIndex].chmod_value_mkdir = ServerInfos[SourceIndex].chmod_value_mkdir;
  ServerInfos[DestIndex].compression = ServerInfos[SourceIndex].compression;
  ServerInfos[DestIndex].use_key_auth = ServerInfos[SourceIndex].use_key_auth;
  ServerInfos[DestIndex].set_chmod_after_put = ServerInfos[SourceIndex].set_chmod_after_put;
  ServerInfos[DestIndex].set_chmod_after_mkdir = ServerInfos[SourceIndex].set_chmod_after_mkdir;
  ServerInfos[DestIndex].set_mtime_after_put = ServerInfos[SourceIndex].set_mtime_after_put;
  ServerInfos[DestIndex].dont_ask4_username = ServerInfos[SourceIndex].dont_ask4_username;
  ServerInfos[DestIndex].dont_ask4_password = ServerInfos[SourceIndex].dont_ask4_password;
  ServerInfos[DestIndex].dont_ask4_passphrase = ServerInfos[SourceIndex].dont_ask4_passphrase;
  ServerInfos[DestIndex].proxy_type = ServerInfos[SourceIndex].proxy_type;
  strncpy(ServerInfos[DestIndex].proxy_host, ServerInfos[SourceIndex].proxy_host, MAX_Server_INFO);
  ServerInfos[DestIndex].proxy_port = ServerInfos[SourceIndex].proxy_port;
  strncpy(ServerInfos[DestIndex].proxy_username, 
    ServerInfos[SourceIndex].proxy_username, MAX_Server_INFO);
  strncpy(ServerInfos[DestIndex].proxy_password, 
    ServerInfos[SourceIndex].proxy_password, MAX_Server_INFO);
  strncpy(ServerInfos[DestIndex].proxy_telnet_command, 
    ServerInfos[SourceIndex].proxy_telnet_command, MAX_Server_INFO);
}

void SetDefaultsToServerInfo(SftpServerAccountInfo* ServerInfo)
{
  ServerInfo->compression = 0;
  ServerInfo->dont_ask4_username = 0;
  ServerInfo->dont_ask4_passphrase = 0;
  ServerInfo->dont_ask4_password = 0;
  ServerInfo->home_dir[0] = '\0';
  ServerInfo->host[0] = '\0';
  ServerInfo->host_cached[0] = '\0';
  //
  ServerInfo->is_imported_from_any_datasrc = 0;
  ServerInfo->is_imported_from_putty_registry = 0;
  ServerInfo->is_imported_from_sshcom_registry = 0;
  //
  ServerInfo->keyfilename[0] = '\0';
  ServerInfo->passphrase[0] = '\0';
  ServerInfo->proxy_host[0] = '\0';
  ServerInfo->proxy_password[0] = '\0';
  ServerInfo->proxy_telnet_command[0] = '\0';
  ServerInfo->proxy_type = PROXY_NONE;
  ServerInfo->proxy_username[0] = '\0';
  ServerInfo->password[0] = '\0';
  ServerInfo->password_cached[0] = '\0';
  ServerInfo->use_key_auth = 0;
  ServerInfo->username[0] = '\0';
  ServerInfo->username_cached[0] = '\0';
  ServerInfo->set_chmod_after_put = 0;
  ServerInfo->set_chmod_after_mkdir = 0;
  ServerInfo->set_mtime_after_put = 0;
  //
  ServerInfo->port = 22;
  ServerInfo->proxy_port = 1080;
  ServerInfo->chmod_value_put = 700;
  ServerInfo->chmod_value_mkdir = 700;
}

#define WriteServerInfo(key, field) WritePrivateProfileString(section_name, key, \
  ServerInfo->field, aProperties->ConfigIniFile); 
#define WriteServerInfoBool(key, field) WritePrivateProfileString(section_name, key, \
  ServerInfo->field ? "1" : "0", aProperties->ConfigIniFile); 
#define WriteServerInfoInt(key, field) sprintf(buf, "%d", ServerInfo->field); \
  WritePrivateProfileString(section_name, key, \
  buf, aProperties->ConfigIniFile); 
#define WriteServerInfoValue(key, value) WritePrivateProfileString(section_name, key, \
  value, aProperties->ConfigIniFile); 

void SaveServerInfo(int SectionNumber, struct SftpServerAccountInfo *ServerInfo, 
                    struct config_properties *aProperties)
{
  char section_name[MAX_Server_INFO];
  char buf[MAX_Server_INFO];
  sprintf(section_name, "%i", SectionNumber);
  WriteServerInfo("title", title)
  WriteServerInfo("host", host)
  WriteServerInfo("username", username)
  if ((aProperties->EncryptPassword) && (strlen(aProperties->PasswordCrypterPassword)>0)) {
    char buf[4000];
    memset(buf, 0, sizeof(buf));
    aProperties->EncryptPassword(ServerInfo->password, buf);
    WriteServerInfoValue("password", buf)
  } else {
    WriteServerInfo("password", password)
  }
  WriteServerInfoInt("port", port)
  WriteServerInfo("home_dir", home_dir)
  WriteServerInfoBool("compression", compression)
  WriteServerInfoBool("use_key_auth", use_key_auth)
  WriteServerInfo("keyfilename", keyfilename)
  WriteServerInfoBool("dont_ask4_passphrase", dont_ask4_passphrase)
  WriteServerInfoBool("dont_ask4_username", dont_ask4_username)
  WriteServerInfoBool("dont_ask4_password", dont_ask4_password)
  WriteServerInfoInt("proxy_type", proxy_type)
  WriteServerInfo("proxy_host", proxy_host)
  WriteServerInfoInt("proxy_port", proxy_port)
  WriteServerInfo("proxy_username", proxy_username)
  WriteServerInfo("proxy_password", proxy_password)
  WriteServerInfo("proxy_telnet_command", proxy_telnet_command)
  WriteServerInfoInt("chmod_value_put", chmod_value_put)
  WriteServerInfoInt("chmod_value_mkdir", chmod_value_mkdir)
  WriteServerInfoBool("set_chmod_after_put", set_chmod_after_put)
  WriteServerInfoBool("set_chmod_after_mkdir", set_chmod_after_mkdir)
  WriteServerInfoBool("set_mtime_after_put", set_mtime_after_put)
}
