#ifndef _SHARE_H
#define _SHARE_H

#include <stdio.h>
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>
#include "passwd_crypter.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_Server_Count 500
#define MAX_Server_INFO 255     //this ain't so nice!!!
#define RESULT_OK	1
#define	RESULT_ERR	0

#define UNEXPECTED_OK_MSG "unexpected OK response"

//from ACE
#define UNUSED_ARG(a) do {/* null */} while (&a == 0);

typedef int PROXY_TYPE;

int get_tmp_file_name(char *buf);

struct SftpServerAccountInfo {
  int id;
  char title[MAX_Server_INFO];
  char username[MAX_Server_INFO];
  char username_cached[MAX_Server_INFO];
  char password[MAX_Server_INFO];
  char passphrase[MAX_Server_INFO]; //always ask
  char password_cached[MAX_Server_INFO];
  char host[MAX_Server_INFO];
  char host_cached[MAX_Server_INFO];
  char home_dir[MAX_Server_INFO];
  char keyfilename[MAX_Server_INFO];
  unsigned int port;
  unsigned int chmod_value_put;
  unsigned int chmod_value_mkdir;
  unsigned char compression;
  unsigned char use_key_auth;
  unsigned char set_chmod_after_put;
  unsigned char set_chmod_after_mkdir;
  unsigned char set_mtime_after_put;
  unsigned char dont_ask4_username;
  unsigned char dont_ask4_password;
  unsigned char dont_ask4_passphrase;
  unsigned char is_imported_from_any_datasrc;  //private
  unsigned char is_imported_from_putty_registry; //private
  unsigned char is_imported_from_sshcom_registry;  //private
  //Experimental - just a copy from Putty.h:Config struct
  PROXY_TYPE proxy_type;
  char proxy_host[MAX_Server_INFO];
  unsigned int proxy_port;
  char proxy_username[MAX_Server_INFO];
  char proxy_password[MAX_Server_INFO];
  char proxy_telnet_command[MAX_Server_INFO];
};

struct config_properties {
  struct SftpServerAccountInfo ServerInfos[MAX_Server_Count];
  int ServerCount;
  int ImportedSessions;
  char DoImportSSHcomSessions[MAX_Server_INFO];
  char DoImportPuttySessions;
  char ConfigIniFile[MAX_PATH];
  HWND MainWindow;
  char PasswordCrypterPath[MAX_Server_INFO];
  char PasswordCrypterPassword[MAX_Server_INFO];
  tEncryptPassword EncryptPassword;
  tDecryptPassword DecryptPassword;
};

struct SftpServerAccountInfo *get_Server_config_Struct(void);
void set_Server_config_Struct(struct SftpServerAccountInfo ServerAccountInfo);

#ifdef __cplusplus
}
#endif

#endif //_SHARE_H
