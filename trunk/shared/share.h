#ifndef _SHARE_H
#define _SHARE_H

#include <stdio.h>
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>

#define MAX_Server_INFO 255     //this ain't so nice!!!
#define RESULT_OK	1
#define	RESULT_ERR	0

#define UNEXPECTED_OK_MSG "unexpected OK response"

typedef int PROXY_TYPE;

int get_tmp_file_name(char *buf);

struct SftpServerAccountInfo {
  unsigned int id;
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
  unsigned int compression;
  unsigned int use_key_auth;
  unsigned int dont_ask4_username;
  unsigned int dont_ask4_password;
  unsigned int dont_ask4_passphrase;
  unsigned int is_imported_from_any_datasrc;  //private
  unsigned int is_imported_from_putty_registry; //private
  unsigned int is_imported_from_sshcom_registry;  //private
  //Experimental - just a copy from Putty.h:Config struct
  PROXY_TYPE proxy_type;
  char proxy_host[512];
  int proxy_port;
  char proxy_username[32];
  char proxy_password[32];
  char proxy_telnet_command[512];
};


struct SftpServerAccountInfo get_Server_config_Struct(void);
void set_Server_config_Struct(struct SftpServerAccountInfo
                              ServerAccountInfo);
void DISABLE_LOGGING();
void ENABLE_LOGGING();
void DISABLE_LOGGING_ONCE();
int do_logging();
void trim_host_from_hoststring(char *str_host);
void trim_port_from_hoststring(char *str_host);

#endif                          //_SHARE_H
