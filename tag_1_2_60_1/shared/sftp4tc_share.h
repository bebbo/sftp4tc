#ifndef _SHARE_H
#define _SHARE_H

#include <stdio.h>
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>
#include "passwd_crypter.h"

#define MAX_SERVER_COUNT 500
#define MAX_SERVER_INFO 255     //this ain't so nice!!!
#define RESULT_OK	1
#define	RESULT_ERR	0

#define UNEXPECTED_OK_MSG "unexpected OK response"

#ifdef __cplusplus
extern "C" {
#endif

//from ACE
#define UNUSED_ARG(a) do {/* null */} while (&a == 0);

typedef int PROXY_TYPE;

int get_tmp_file_name(char *buf);

typedef struct SftpServerAccountInfo {
  int id;
  char title[MAX_SERVER_INFO];
  char username[MAX_SERVER_INFO];
  char username_cached[MAX_SERVER_INFO];
  char password[MAX_SERVER_INFO];
  char passphrase[MAX_SERVER_INFO]; //always ask
  char password_cached[MAX_SERVER_INFO];
  char host[MAX_SERVER_INFO];
  char host_cached[MAX_SERVER_INFO];
  char home_dir[MAX_SERVER_INFO];
  char keyfilename[MAX_SERVER_INFO];
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
  //internal
  unsigned char is_imported_from_any_datasrc;  //private
  unsigned char is_imported_from_putty_registry; //private
  unsigned char is_imported_from_sshcom_registry;  //private
  char base_dir[MAX_SERVER_INFO];
  char show_hidden_files;
  //Experimental - just a copy from Putty.h:Config struct
  PROXY_TYPE proxy_type;
  char proxy_host[MAX_SERVER_INFO];
  unsigned int proxy_port;
  char proxy_username[MAX_SERVER_INFO];
  char proxy_password[MAX_SERVER_INFO];
  char proxy_telnet_command[MAX_SERVER_INFO];
} SftpServerAccountInfoType;

typedef struct ConfigProperties {
  struct SftpServerAccountInfo ServerInfos[MAX_SERVER_COUNT];
  int ServerCount;
  int ImportedSessions;
  int SelectedSession;
  char DoImportSSHcomSessions[MAX_SERVER_INFO];
  char DoImportPuttySessions;
  char CacheFS;
  char ConfigIniFile[MAX_PATH];
  HWND MainWindow;
  char PasswordCrypterPath[MAX_SERVER_INFO];
  char PasswordCrypterPassword[MAX_SERVER_INFO];
  tEncryptPassword EncryptPassword;
  tDecryptPassword DecryptPassword;
} ConfigPropertiesType;

struct SftpServerAccountInfo *get_Server_config_Struct(void);
void set_Server_config_Struct(struct SftpServerAccountInfo ServerAccountInfo);

#ifdef __cplusplus
}
#endif

#endif //_SHARE_H
