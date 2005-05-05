#include "stdafx.h"
#include "fsplugin.h"
#include "sftpmap.h"

extern "C" {
#include "../shared/share.h"
}

#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------

TD_SFTP_DLL_FNCT_CONNECT SFTP_DLL_FNCT_CONNECT[MAX_Server_Count];
TD_SFTP_DLL_FNCT_DO_SFTP SFTP_DLL_FNCT_DO_SFTP[MAX_Server_Count];
TD_SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT
  SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT[MAX_Server_Count];
FARPROC SFTP_DLL_FNCT_DISCONNECT[MAX_Server_Count];
FARPROC SFTP_DLL_FNCT_psftp_memory_hole__stopfen[MAX_Server_Count];
TD_SFTP_DLL_FNCT_GLASTERRMSG
  SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[MAX_Server_Count];
TD_SFTP_DLL_FNCT_init_ProgressProc
  SFTP_DLL_FNCT_init_ProgressProc[MAX_Server_Count];
TD_SFTP_DLL_FNCT_SetSftpServerAccountInfo
  SFTP_DLL_FNCT_SetSftpServerAccountInfo[MAX_Server_Count];
TD_SFTP_DLL_FNCT_SetTransferMode
  SFTP_DLL_FNCT_SetTransferMode[MAX_Server_Count];
TD_SFTP_DLL_FNCT_Disconnected
  SFTP_DLL_FNCT_Disconnected[MAX_Server_Count];
HMODULE PSFTP_DLL_HANDLER[MAX_Server_Count];

//---------------------------------------------------------------------

char *LI_user = NULL;
char *LI_password = NULL;
char *LI_host = NULL;
int LI_port = 22;
bool already_connected;
char mDefaultTransferMode[2048];
int disable_run_time_logging = 0;
int disable_run_time_logging_ONCE = 0;

//---------------------------------------------------------------------

extern tRequestProc RequestProc;
extern int PluginNumber;
extern HMODULE hDllModule;

//---------------------------------------------------------------------

bool IsAlreadyConnected()
{
  return already_connected;
}

void ResetAlreadyConnected()
{
  already_connected = false;
}

//---------------------------------------------------------------------

void Unload_PSFTP_DLL_HANDLER(int ServerId)
{
  if (PSFTP_DLL_HANDLER[ServerId]) {
    FreeLibrary(PSFTP_DLL_HANDLER[ServerId]);
    unlink_dll_tmp_file(ServerId);
    PSFTP_DLL_HANDLER[ServerId] = NULL;
    SFTP_DLL_FNCT_CONNECT[ServerId] = NULL;
    SFTP_DLL_FNCT_DISCONNECT[ServerId] = NULL;
    SFTP_DLL_FNCT_DO_SFTP[ServerId] = NULL;
    SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT[ServerId] = NULL;
    SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[ServerId] = NULL;
    SFTP_DLL_FNCT_init_ProgressProc[ServerId] = NULL;
    SFTP_DLL_FNCT_psftp_memory_hole__stopfen[ServerId] = NULL;
    SFTP_DLL_FNCT_SetSftpServerAccountInfo[ServerId] = NULL;
    SFTP_DLL_FNCT_SetTransferMode[ServerId] = NULL;
    SFTP_DLL_FNCT_Disconnected[ServerId] = NULL;
  }
}

//---------------------------------------------------------------------

void wcplg_sftp_transfermode(int id, char *mode)
{
  if (id>=0)
    SFTP_DLL_FNCT_SetTransferMode[id](mode);
  else
    strncat(mDefaultTransferMode, mode, 2048);
}

//---------------------------------------------------------------------

int wcplg_sftp_connect(char *user, char *password, char *host, int port,
                       SftpServerAccountInfo * allServers,
                       int CurrentServerId)
{

  if (PSFTP_DLL_HANDLER[CurrentServerId] != NULL) // are we connected ?
  {
    return SFTP_SUCCESS;        // we allready have a connection running
  }

  /*
     char log_buf[MAX_CMD_BUFFER];
     sprintf(log_buf,"connect:%d",CurrentServerId);
     LogProc_(MSGTYPE_DETAILS,log_buf);
   */

  int realPort = 22;
  char realHost[MAX_CMD_BUFFER];
  char realPortStr[MAX_CMD_BUFFER];
  realHost[0] = '\0';

  // ok lets load the DLL and connect to the sftp server 

  SFTP_DLL_FNCT_CONNECT[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_DISCONNECT[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_DO_SFTP[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_init_ProgressProc[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_psftp_memory_hole__stopfen[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_SetSftpServerAccountInfo[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_SetTransferMode[CurrentServerId] = NULL;
  SFTP_DLL_FNCT_Disconnected[CurrentServerId] = NULL;
  PSFTP_DLL_HANDLER[CurrentServerId] = NULL;

  char dll_2_load[MAX_CMD_BUFFER];
  char dll_2_copy[MAX_CMD_BUFFER];
  char dll_with_CSid[100];

  sprintf(dll_with_CSid, "psftp_%i.dll", CurrentServerId);

  GetTempPath(MAX_CMD_BUFFER, dll_2_load);
  strcat(dll_2_load, dll_with_CSid);

  get_psftpDll_path(dll_2_copy);
  if (!file_exists(dll_2_copy)) {
    char msg[MAX_CMD_BUFFER];
    sprintf(msg, "%s not exists", dll_2_copy);
    dbg(msg);
    return SFTP_FAILED;
  }

  int fcret = fileCopy(dll_2_copy, dll_2_load);

#ifdef _DEBUG
  PSFTP_DLL_HANDLER[CurrentServerId] = LoadLibrary(dll_2_copy);
#else
  PSFTP_DLL_HANDLER[CurrentServerId] = LoadLibrary(dll_2_load);
#endif

  if (PSFTP_DLL_HANDLER[CurrentServerId] == NULL) {
    char msg[MAX_CMD_BUFFER];
    sprintf(msg, "Can't load \"%s\" (code %d)", dll_2_load, fcret);
    dbg(msg);
    return SFTP_FAILED;         // DLL nicht ladbar, warnung ausgeben 
  };

  SFTP_DLL_FNCT_CONNECT[CurrentServerId] =
    (TD_SFTP_DLL_FNCT_CONNECT)
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__wcplg_open_sftp_session");
  SFTP_DLL_FNCT_DISCONNECT[CurrentServerId] =
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__wcplg_close_sftp_session");
  SFTP_DLL_FNCT_DO_SFTP[CurrentServerId] =
    (TD_SFTP_DLL_FNCT_DO_SFTP)
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__wcplg_do_sftp");
  SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT[CurrentServerId] =
    (TD_SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT)
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__wcplg_get_current_dir_struct");
  SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[CurrentServerId] =
    (TD_SFTP_DLL_FNCT_GLASTERRMSG)
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__wcplg_get_last_error_msg");
  SFTP_DLL_FNCT_init_ProgressProc[CurrentServerId] =
    (TD_SFTP_DLL_FNCT_init_ProgressProc)
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__init_ProgressProc");
  SFTP_DLL_FNCT_psftp_memory_hole__stopfen[CurrentServerId] =
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__psftp_memory_hole__stopfen");
  SFTP_DLL_FNCT_SetSftpServerAccountInfo[CurrentServerId] =
    (TD_SFTP_DLL_FNCT_SetSftpServerAccountInfo)
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__set_Server_config_Struct");
  SFTP_DLL_FNCT_SetTransferMode[CurrentServerId] =
    (TD_SFTP_DLL_FNCT_SetTransferMode)
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__setTransferMode");

  SFTP_DLL_FNCT_Disconnected[CurrentServerId] =
    (TD_SFTP_DLL_FNCT_Disconnected)
    GetProcAddress(PSFTP_DLL_HANDLER[CurrentServerId],
                   "__map__disconnected");

  if (SFTP_DLL_FNCT_CONNECT[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_DISCONNECT[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_DO_SFTP[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_init_ProgressProc[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_psftp_memory_hole__stopfen[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_SetSftpServerAccountInfo[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_SetTransferMode[CurrentServerId] == NULL
      || SFTP_DLL_FNCT_Disconnected[CurrentServerId] == NULL) {
    // DLL konnte zwar geladen werden aber die funktionen konnten nicht vollständig 
    // importiert werden 
    dbg("Can't load all of PSFTP.DLL's functions!");
    Unload_PSFTP_DLL_HANDLER(CurrentServerId);
    return SFTP_FAILED;
  }


  /* 
     OK DLL Ladevorgang und Funktionsimport hat supi geklappt, 
     Jetzt zum Server connecten mit den Userdaten und gucken ob der connect geklappt hat
   */

  init_ProgressProc(get_ProgressProc(), get_PluginNumber(),
                    CurrentServerId);

  if ((allServers[CurrentServerId].keyfilename[0]) & (!allServers[CurrentServerId].dont_ask4_passphrase)) //get passphrase for private key
  {
    RequestProc(PluginNumber, RT_Password,
                "Secure FTP: Passphrase for your key", "",
                allServers[CurrentServerId].passphrase, MAX_CMD_BUFFER);
  }

  SFTP_DLL_FNCT_SetSftpServerAccountInfo[CurrentServerId] (allServers
                                                           [CurrentServerId]);
  // Logging
  char buf_log[MAX_CMD_BUFFER];

  strcpy(realHost, host);
  strcpy(realPortStr, host);
  trim_host_from_hoststring(realHost);
  trim_port_from_hoststring(realPortStr);

  if (strlen(realPortStr) < 1) {
    realPort = port;
  } else {
    realPort = atoi(realPortStr);
    if (realPort == 0)
      realPort = port;
  }

  if (realPort == 0)
    realPort = 22;              //last check if port set 2 a valid number

  sprintf(buf_log, "CONNECT \\");
  if (already_connected)
    LogProc_(MSGTYPE_DETAILS, buf_log);
  else {
    LogProc_(MSGTYPE_CONNECT, buf_log);
    already_connected = true;
  }

  sprintf(buf_log, "[%s] (connecting to %s@%s:%i using %s)",
          allServers[CurrentServerId].title, user, realHost, realPort,
          allServers[CurrentServerId].
          use_key_auth ? "public key authentication" :
          "password authentication");
  LogProc_(MSGTYPE_DETAILS, buf_log);

  SFTP_DLL_FNCT_SetTransferMode[CurrentServerId](mDefaultTransferMode);

  if (SFTP_DLL_FNCT_CONNECT[CurrentServerId]
      (user, password, realHost, realPort) == 1) {
    char sftp_cmd[MAX_CMD_BUFFER];
    
    sprintf(sftp_cmd, "pwd");
    if (wcplg_sftp_do_commando(sftp_cmd, allServers[CurrentServerId].base_dir, CurrentServerId) != SFTP_SUCCESS) {
      strcpy(allServers[CurrentServerId].base_dir, allServers[CurrentServerId].home_dir);
    }

    if (allServers[CurrentServerId].home_dir != NULL) {
      if ((strlen(allServers[CurrentServerId].home_dir) > 0)
          && (strcmp(allServers[CurrentServerId].home_dir, ".") != 0)) {
        sprintf(sftp_cmd, "cd \"%s\"", allServers[CurrentServerId].home_dir);
        winSlash2unix(sftp_cmd);

        if (wcplg_sftp_do_commando(sftp_cmd, NULL, CurrentServerId) == SFTP_SUCCESS) {
          strcpy(allServers[CurrentServerId].base_dir, allServers[CurrentServerId].home_dir);
        } else {
          sprintf(buf_log, "WARNING: couldn't change to home directory!");
          allServers[CurrentServerId].home_dir[0] = 0;
          LogProc_(MSGTYPE_DETAILS, buf_log);
        }
      }
    }
    LogProc_(MSGTYPE_CONNECTCOMPLETE, "");
    return SFTP_SUCCESS;
  }

  strcpy(buf_log,
         SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[CurrentServerId] ());
  if (strcmp(buf_log, "") != 0)
    LogProc_(MSGTYPE_IMPORTANTERROR, buf_log);


  //wcplg_sftp_disconnect(CurrentServerId,true);

  //lets look if this is a imported session && use_key_auth=1  - and if this case then change 
  //use_key_auth=0 that the user can give a password
  if (allServers[CurrentServerId].is_imported_from_any_datasrc == 1
      && allServers[CurrentServerId].use_key_auth == 1) {
    allServers[CurrentServerId].use_key_auth = 0;
  }

  Unload_PSFTP_DLL_HANDLER(CurrentServerId);

  return SFTP_FAILED;
}

//---------------------------------------------------------------------

int wcplg_sftp_disconnect(int ServerId, bool log_message)
{

  int cS_ID;
  struct SftpServerAccountInfo *_allServer;

  if (ServerId == -1) {
    dbg("wcplg_sftp_disconnect: ServerId == -1 FIX ME!");
    return SFTP_FAILED;
  }

  cS_ID = ServerId;
  _allServer = GetServerInfos();

  if (PSFTP_DLL_HANDLER[ServerId] != NULL)  // are we really connected ?
  {
    SFTP_DLL_FNCT_DISCONNECT[ServerId] ();

    Unload_PSFTP_DLL_HANDLER(ServerId);

    if (cS_ID > -1) {
      if (strcmp(_allServer[cS_ID].title, QUICK_CONNECTION) == 0) {
        _allServer[cS_ID].host_cached[0] = '\0';
      }
      _allServer[cS_ID].password_cached[0] = '\0';
      if (!_allServer[cS_ID].dont_ask4_passphrase)
        for (unsigned int xx = 0;
             xx < strlen(_allServer[cS_ID].passphrase); xx++)
          _allServer[cS_ID].passphrase[xx] = 0;
      if (!_allServer[cS_ID].dont_ask4_password)
        _allServer[cS_ID].password[0] = '\0';
      if (!_allServer[cS_ID].dont_ask4_username)
        _allServer[cS_ID].username_cached[0] = '\0';
    }

    char bufDmsg[MAX_CMD_BUFFER];
    strcpy(bufDmsg, _allServer[cS_ID].title);
    strcat(bufDmsg, ": connection lost, disconnected!");

    if (log_message)
      LogProc_(MSGTYPE_DISCONNECT, bufDmsg);
  } else {
    char bufDmsg[] = "sorry!";
    if (log_message)
      LogProc_(MSGTYPE_DISCONNECT, bufDmsg);
  }

  return SFTP_SUCCESS;
}

//---------------------------------------------------------------------

int wcplg_sftp_do_commando(char *commando, char *server_output,
                           int ServerId)
{
  char log_buf[MAX_CMD_BUFFER];
  int _CurrentServer_ID = ServerId;
  struct SftpServerAccountInfo *_allServer;
  _allServer = GetServerInfos();

  if (_CurrentServer_ID == -1) {
    dbg("wcplg_sftp_do_commando: _CurrentServer_ID == -1, FIX ME");
    return SFTP_FAILED;
  }

  sprintf(log_buf, "%s: %s", _allServer[_CurrentServer_ID].title,
          commando);

  LogProc_(MSGTYPE_DETAILS, log_buf);

  if (SFTP_DLL_FNCT_DO_SFTP[ServerId] (commando, server_output) != 1) {
    sprintf(log_buf, "%s: !%s", _allServer[ServerId].title,
            SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[ServerId] ());
    LogProc_(MSGTYPE_IMPORTANTERROR, log_buf);
    if (SFTP_DLL_FNCT_Disconnected[ServerId]() == 1) {
      Unload_PSFTP_DLL_HANDLER(ServerId);
      return SFTP_DISCONNECTED;
      //reconnect would be usefull...
    }
    return SFTP_FAILED;
  }

  return SFTP_SUCCESS;
}

//---------------------------------------------------------------------

struct fxp_names *wcplg_sftp_get_current_dir_struct(int ServerId)
{
  fxp_names *CurrentDirStruct = NULL;

  // MUST be copied!!!
  my_fxp_names *CurrentDirStruct_TMP =
    SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT[ServerId] ();

  if (CurrentDirStruct_TMP != NULL) {
    CurrentDirStruct = new fxp_names;
    CurrentDirStruct->nnames = CurrentDirStruct_TMP->nnames;
    if (CurrentDirStruct->nnames>=0) {
      CurrentDirStruct->names =
        (fxp_name *) malloc(sizeof(fxp_name) * CurrentDirStruct_TMP->nnames);

      for (int i = 0; i < CurrentDirStruct_TMP->nnames; ++i) {
        CurrentDirStruct->names[i].attrs =
          CurrentDirStruct_TMP->names[i]->attrs;
        CurrentDirStruct->names[i].filename =
          (char *) malloc(sizeof(char) *
                          (strlen(CurrentDirStruct_TMP->names[i]->filename) +
                           1));
        CurrentDirStruct->names[i].longname =
          (char *) malloc(sizeof(char) *
                          (strlen(CurrentDirStruct_TMP->names[i]->longname) +
                           1));
        strcpy(CurrentDirStruct->names[i].filename,
               CurrentDirStruct_TMP->names[i]->filename);
        strcpy(CurrentDirStruct->names[i].longname,
               CurrentDirStruct_TMP->names[i]->longname);
      }
    }
  }
  return CurrentDirStruct;
}

//---------------------------------------------------------------------

void winSlash2unix(char *s)
{
  unsigned int i;
  for (i = 0; i < strlen(s); i++) {
    if (s[i] == '\\')
      s[i] = '/';
  }
}

//---------------------------------------------------------------------

void UnixSlash2Win(char *s)
{
  unsigned int i;
  for (i = 0; i < strlen(s); i++) {
    if (s[i] == '/')
      s[i] = '\\';
  }
}

//---------------------------------------------------------------------

int init_ProgressProc(tProgressProc AP_ProgressProc, int Awc_PluginNr,
                      int ServerId)
{
  if (SFTP_DLL_FNCT_init_ProgressProc[ServerId] != NULL) {
    return SFTP_DLL_FNCT_init_ProgressProc[ServerId] (AP_ProgressProc,
                                                      Awc_PluginNr);
  }
  return 0;
}

//---------------------------------------------------------------------

int psftp_memory_hole__stopfen(int ServerId)
{
  if ((ServerId!=-1) && (SFTP_DLL_FNCT_psftp_memory_hole__stopfen[ServerId] != NULL)) {
    return SFTP_DLL_FNCT_psftp_memory_hole__stopfen[ServerId] ();
  }
  return -1;
}

//---------------------------------------------------------------------

void init_server_dll_handlers()
{
  int i = 0;
  for (; i < MAX_Server_Count; i++) {
    SFTP_DLL_FNCT_CONNECT[i] = NULL;
    SFTP_DLL_FNCT_DO_SFTP[i] = NULL;
    SFTP_DLL_FNCT_GET_CURRENT_DIR_STRUCT[i] = NULL;
    SFTP_DLL_FNCT_DISCONNECT[i] = NULL;
    SFTP_DLL_FNCT_psftp_memory_hole__stopfen[i] = NULL;
    SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[i] = NULL;
    SFTP_DLL_FNCT_init_ProgressProc[i] = NULL;
    SFTP_DLL_FNCT_SetSftpServerAccountInfo[i] = NULL;
    SFTP_DLL_FNCT_Disconnected[i] = NULL;
    SFTP_DLL_FNCT_SetTransferMode[i] = NULL;
    PSFTP_DLL_HANDLER[MAX_Server_Count] = NULL;
  }
}

//---------------------------------------------------------------------

void get_psftpDll_path(char *buf)
{
  char cDir[MAX_CMD_BUFFER];
  GetModuleFileName(hDllModule, cDir, MAX_CMD_BUFFER - 10);

  strcpy(buf, cDir);

  char *p = strrchr(buf, '\\');
  if (!p)
    p = buf;

  strcpy(p, "\\psftp.dll");
}

//---------------------------------------------------------------------

int fileCopy(char *src, char *dest)
{
  FILE *infp, *outfp;
  int c;
  if (strcmp(src, dest) == 0)
    return 1;

  infp = fopen(src, "rb");
  if (infp == NULL)
    return 2;

  outfp = fopen(dest, "w+b");
  if (outfp == NULL) {
    fclose(infp);
    return 3;
  }

  while ((c = getc(infp)) != EOF) //what about buffered copy, or system copy?
    putc(c, outfp);

  fclose(infp);
  fclose(outfp);
  return 0;
}

//---------------------------------------------------------------------

void unlink_dll_tmp_file(int id)
{
  if (id == -1)
    return;
  char dll_2_load[MAX_CMD_BUFFER];
  char dll_with_CSid[100];

  sprintf(dll_with_CSid, "psftp_%i.dll", id);
  GetTempPath(MAX_CMD_BUFFER, dll_2_load);
  strcat(dll_2_load, dll_with_CSid);

  remove(dll_2_load);
}

//---------------------------------------------------------------------

void unlink_ALL_dll_tmp_files()
{
  int i;
  for (i = 0; i < MAX_Server_Count; i++) {
    unlink_dll_tmp_file(i);
  }
}

//---------------------------------------------------------------------

int file_exists(char *fname)
{
  FILE *fp;
  int ret = 0;

  fp = fopen(fname, "r");

  if (fp != NULL) {
    ret = 1;
    fclose(fp);
  }
  return ret;
}

//---------------------------------------------------------------------

void wcplg_sftp_getLastError(int id, char *buf)
{
  char *msg;
  if (SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[id] != NULL) {
    msg = SFTP_DLL_FNCT_GET_LAST_ERROR_MESSAGE[id] ();
    if (msg == NULL)
      strcpy(buf, "");
    else
      strcpy(buf, msg);
  } else
    strcpy(buf, "");
}

//---------------------------------------------------------------------

void DISABLE_LOGGING()
{
  disable_run_time_logging = 1;
}

//---------------------------------------------------------------------

void DISABLE_LOGGING_ONCE()
{
  disable_run_time_logging_ONCE = 1;
}

//---------------------------------------------------------------------

void ENABLE_LOGGING()
{
  disable_run_time_logging = 0;
}

//---------------------------------------------------------------------

int do_logging()
{
  if (disable_run_time_logging_ONCE == 1) {
    disable_run_time_logging_ONCE = 0;
    return 0;
  }
  if (disable_run_time_logging == 1)
    return 0;
  return 1;
}

//---------------------------------------------------------------------

void trim_host_from_hoststring(char *hoststring)
{
  char *buf;
  if (hoststring == NULL)
    return;
  buf = strstr(hoststring, ":");
  if (buf == NULL)
    return;                     //Kein Port angegeben
  hoststring[strlen(hoststring) - strlen(buf)] = '\0';
}

//---------------------------------------------------------------------

void trim_port_from_hoststring(char *hoststring)
{
  char *buf;
  if (hoststring == NULL)
    return;
  buf = strstr(hoststring, ":");
  if (buf == NULL) {
    strcpy(hoststring, "");
    return;                     //Kein Port angegeben
  }
  buf++;                        // : überspringen
  strcpy(hoststring, buf);
  //hoststring+=(strlen(hoststring)-strlen(buf)+1);
}

