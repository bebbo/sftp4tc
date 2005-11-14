#include "stdafx.h"
#include "fsplugin.h"
#include "sftpmap.h"

extern "C" {
#include "../shared/sftp4tc_share.h"
}

#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------

typedef struct
{
  PsftpConnectProcType Connect;
  PsftpDoSftpProcType DoSftp;
  PsftpGetCurrentDirStructProcType GetCurrentDirStruct;
  FARPROC Disconnect;
  FARPROC SFTP_DLL_FNCT_psftp_memory_hole__stopfen;
  PsftpGetLastErrorMessageProcType GetLastErrorMessage;
  PsftpInitProgressProcProcType InitProgressProc;
  PsftpSetSftpServerAccountInfoProcType SetSftpServerAccountInfo;
  PsftpSetTransferModeProcType SetTransferMode;
  PsftpDisconnectedProcType Disconnected;
  HMODULE hDll;
} PsftpWrapperType;

PsftpWrapperType PsftpWrapper[MAX_SERVER_COUNT];

//---------------------------------------------------------------------

bool gAlreadyConnected;
char gDefaultTransferMode[2048];
int gDisableRunTimeLogging = 0;
int gDisableRunTimeLoggingOnce = 0;

//---------------------------------------------------------------------

extern RequestProcType gRequestProc;
extern ProgressProcType gProgressProc;
extern int gPluginNumber;
extern HMODULE ghThisDllModule;

//---------------------------------------------------------------------

bool IsAlreadyConnected()
{
  return gAlreadyConnected;
}

//---------------------------------------------------------------------

void ResetAlreadyConnected()
{
  gAlreadyConnected = false;
}

//---------------------------------------------------------------------

inline void InitPsftpWrapperStruct(int serverId)
{
  PsftpWrapper[serverId].hDll                                     = NULL;
  PsftpWrapper[serverId].Connect                                  = NULL;
  PsftpWrapper[serverId].Disconnect                               = NULL;
  PsftpWrapper[serverId].DoSftp                                   = NULL;
  PsftpWrapper[serverId].GetCurrentDirStruct                      = NULL;
  PsftpWrapper[serverId].GetLastErrorMessage                      = NULL;
  PsftpWrapper[serverId].InitProgressProc                         = NULL;
  PsftpWrapper[serverId].SFTP_DLL_FNCT_psftp_memory_hole__stopfen = NULL;
  PsftpWrapper[serverId].SetSftpServerAccountInfo                 = NULL;
  PsftpWrapper[serverId].SetTransferMode                          = NULL;
  PsftpWrapper[serverId].Disconnected                             = NULL;
}                                   

//---------------------------------------------------------------------

void UnloadPsftpDll(int serverId)
{
  if (PsftpWrapper[serverId].hDll) {
    FreeLibrary(PsftpWrapper[serverId].hDll);
    UnlinkTemporaryDllFile(serverId);
    
    InitPsftpWrapperStruct(serverId);
  }
}

//---------------------------------------------------------------------

void SetTransferMode(int id, char *mode)
{
  if (id>=0)
    PsftpWrapper[id].SetTransferMode(mode);
  else
    strncat(gDefaultTransferMode, mode, 2048);
}

//---------------------------------------------------------------------

int Connect(char* user, char* password, char* host, int port,
        SftpServerAccountInfo* allServers, int currentServerId)
{
  // are we connected ?
  if (PsftpWrapper[currentServerId].hDll != NULL) 
  { 
    // we allready have a connection running
    return SFTP_SUCCESS;        
  }

  int real_port = 22;
  char real_host[MAX_CMD_BUFFER];
  char real_port_string[MAX_CMD_BUFFER];
  SftpServerAccountInfo* current_server = &allServers[currentServerId];
  PsftpWrapperType *currect_psftp = &PsftpWrapper[currentServerId];
  real_host[0] = '\0';

  // ok lets load the DLL and connect to the sftp server 

  InitPsftpWrapperStruct(currentServerId);

  char dll_2_load[MAX_CMD_BUFFER];
  char dll_2_copy[MAX_CMD_BUFFER];
  char dll_with_server_id[100];

  sprintf(dll_with_server_id, "psftp_%i.dll", currentServerId);

  GetTempPath(MAX_CMD_BUFFER, dll_2_load);
  strcat(dll_2_load, dll_with_server_id);

  GetPsftpDllPath(dll_2_copy);
  if (!FileExists(dll_2_copy)) {
    char msg[MAX_CMD_BUFFER];
    sprintf(msg, "%s not exists", dll_2_copy);
    dbg(msg);
    return SFTP_FAILED;
  }

  int fcret = FileCopy(dll_2_copy, dll_2_load);

#ifdef _DEBUG
  currect_psftp->hDll = LoadLibrary(dll_2_copy);
#else
  currect_psftp->hDll = LoadLibrary(dll_2_load);
#endif

  HMODULE dll = currect_psftp->hDll;

  if (dll == NULL) {
    char msg[MAX_CMD_BUFFER];
    sprintf(msg, "Can't load \"%s\" (code %d)", dll_2_load, fcret);
    dbg(msg);
    return SFTP_FAILED;         // DLL couldn't be loaded, write a warn message
  };

  currect_psftp->Connect =
    (PsftpConnectProcType) GetProcAddress(dll, "__map__wcplg_open_sftp_session");
  currect_psftp->Disconnect =
    GetProcAddress(dll, "__map__wcplg_close_sftp_session");
  currect_psftp->DoSftp =
    (PsftpDoSftpProcType) GetProcAddress(dll, "__map__wcplg_do_sftp");
  currect_psftp->GetCurrentDirStruct =
    (PsftpGetCurrentDirStructProcType) GetProcAddress(dll, "__map__wcplg_get_current_dir_struct");
  currect_psftp->GetLastErrorMessage =
    (PsftpGetLastErrorMessageProcType) GetProcAddress(dll, "__map__wcplg_get_last_error_msg");
  currect_psftp->InitProgressProc =
    (PsftpInitProgressProcProcType) GetProcAddress(dll, "__map__init_ProgressProc");
  currect_psftp->SFTP_DLL_FNCT_psftp_memory_hole__stopfen =
    GetProcAddress(dll, "__map__psftp_memory_hole__stopfen");
  currect_psftp->SetSftpServerAccountInfo =
    (PsftpSetSftpServerAccountInfoProcType) GetProcAddress(dll, "__map__set_Server_config_Struct");
  currect_psftp->SetTransferMode =
    (PsftpSetTransferModeProcType) GetProcAddress(dll, "__map__setTransferMode");
  currect_psftp->Disconnected =
    (PsftpDisconnectedProcType) GetProcAddress(dll, "__map__disconnected");

  if (currect_psftp->Connect == NULL
      || currect_psftp->Disconnect == NULL
      || currect_psftp->DoSftp == NULL
      || currect_psftp->GetCurrentDirStruct == NULL
      || currect_psftp->GetLastErrorMessage == NULL
      || currect_psftp->InitProgressProc == NULL
      || currect_psftp->SFTP_DLL_FNCT_psftp_memory_hole__stopfen == NULL
      || currect_psftp->SetSftpServerAccountInfo == NULL
      || currect_psftp->SetTransferMode == NULL
      || currect_psftp->Disconnected == NULL) {
    // DLL loaded successfuly, but not all functions could be imported
    dbg("Can't load all of PSFTP.DLL's functions!");
    UnloadPsftpDll(currentServerId);
    return SFTP_FAILED;
  }


  /* 
     DLL-loading and functionsimport successed,
     now try to connect to server
   */

  InitProgressProc(gProgressProc, gPluginNumber, currentServerId);

  if ((current_server->keyfilename[0]) & (!current_server->dont_ask4_passphrase)) //get passphrase for private key
  {
    gRequestProc(
        gPluginNumber, 
        RT_Password,
        "Secure FTP: Passphrase for your key", 
        "",
        current_server->passphrase, 
        MAX_CMD_BUFFER);
  }

  currect_psftp->SetSftpServerAccountInfo(*current_server);
  // Logging
  char buf_log[MAX_CMD_BUFFER];

  strcpy(real_host, host);
  strcpy(real_port_string, host);
  trim_host_from_hoststring(real_host);
  trim_port_from_hoststring(real_port_string);

  if (strlen(real_port_string) < 1) {
    real_port = port;
  } else {
    real_port = atoi(real_port_string);
    if (real_port == 0)
      real_port = port;
  }

  if (real_port == 0)
    real_port = 22;              //last check if port set 2 a valid number

  sprintf(buf_log, "CONNECT \\");
  if (gAlreadyConnected)
    LogProc_(MSGTYPE_DETAILS, buf_log);
  else {
    LogProc_(MSGTYPE_CONNECT, buf_log);
    gAlreadyConnected = true;
  }

  sprintf(
      buf_log, 
      "[%s] (connecting to %s@%s:%i using %s)",
      current_server->title, 
      user, 
      real_host, 
      real_port,
      current_server->use_key_auth 
          ? "public key authentication" 
          : "password authentication");
  LogProc_(MSGTYPE_DETAILS, buf_log);

  currect_psftp->SetTransferMode(gDefaultTransferMode);

  if (currect_psftp->Connect(user, password, real_host, real_port) == 1) {
    char sftp_cmd[MAX_CMD_BUFFER];
    
    sprintf(sftp_cmd, "pwd");
    if (ExecuteCommand(sftp_cmd, current_server->base_dir, currentServerId) != SFTP_SUCCESS) {
      strcpy(current_server->base_dir, current_server->home_dir);
    }

    if (current_server->home_dir != NULL) {
      if ((strlen(current_server->home_dir) > 0)
          && (strcmp(current_server->home_dir, ".") != 0)) {
        sprintf(sftp_cmd, "cd \"%s\"", current_server->home_dir);
        convert_slash_windows_to_unix(sftp_cmd);

        if (ExecuteCommand(sftp_cmd, NULL, currentServerId) == SFTP_SUCCESS) {
          strcpy(current_server->base_dir, current_server->home_dir);
        } else {
          sprintf(buf_log, "WARNING: couldn't change to home directory!");
          current_server->home_dir[0] = 0;
          LogProc_(MSGTYPE_DETAILS, buf_log);
        }
      }
    }
    LogProc_(MSGTYPE_CONNECTCOMPLETE, "");
    return SFTP_SUCCESS;
  }

  strcpy(buf_log, currect_psftp->GetLastErrorMessage());
  if (strcmp(buf_log, "") != 0)
    LogProc_(MSGTYPE_IMPORTANTERROR, buf_log);

  //lets look if this is a imported session && use_key_auth=1  - and if this case then change 
  //use_key_auth=0 that the user can give a password
  if (current_server->is_imported_from_any_datasrc == 1
      && current_server->use_key_auth == 1) {
    current_server->use_key_auth = 0;
  }

  UnloadPsftpDll(currentServerId);

  return SFTP_FAILED;
}

//---------------------------------------------------------------------

int Disconnect(int serverId, bool logMessage)
{

  struct SftpServerAccountInfo* all_servers;

  if (serverId == -1) {
    dbg("wcplg_sftp_disconnect: ServerId == -1 FIX ME!");
    return SFTP_FAILED;
  }

  all_servers = GetServerInfos();
  SftpServerAccountInfo* current_server = &all_servers[serverId];

  if (PsftpWrapper[serverId].hDll != NULL)  // are we really connected ?
  {
    PsftpWrapper[serverId].Disconnect();

    UnloadPsftpDll(serverId);

    if (serverId > -1) {
      if (strcmp(current_server->title, QUICK_CONNECTION) == 0) {
        current_server->host_cached[0] = '\0';
      }

      current_server->password_cached[0] = '\0';

      if (!current_server->dont_ask4_passphrase)
      {
        unsigned int xx = 0;
        unsigned int passphrase_length = strlen(current_server->passphrase);
        for (; xx < passphrase_length; xx++)
          current_server->passphrase[xx] = 0;
      }
          
      if (!current_server->dont_ask4_password)
        current_server->password[0] = '\0';
      if (!current_server->dont_ask4_username)
        current_server->username_cached[0] = '\0';
    }

    char message_buffer[MAX_CMD_BUFFER];
    strcpy(message_buffer, current_server->title);
    strcat(message_buffer, ": connection lost, disconnected!");

    if (logMessage)
      LogProc_(MSGTYPE_DISCONNECT, message_buffer);
  } else {
    char message_buffer[] = "sorry! there's no server for disconnection.";
    if (logMessage)
      LogProc_(MSGTYPE_DISCONNECT, message_buffer);
  }

  return SFTP_SUCCESS;
}

//---------------------------------------------------------------------

int ExecuteCommand(char *command, char *serverOutput, int serverId)
{
  char log_buf[MAX_CMD_BUFFER];
  struct SftpServerAccountInfo* all_servers = GetServerInfos();

  if (serverId == -1) {
    dbg("ExecuteCommand: ServerId == -1, FIX ME");
    return SFTP_FAILED;
  }

  sprintf(log_buf, "%s: %s", all_servers[serverId].title, command);

  LogProc_(MSGTYPE_DETAILS, log_buf);

  if (PsftpWrapper[serverId].DoSftp(command, serverOutput) != 1) {
    sprintf(
      log_buf, 
      "%s: !%s", 
      all_servers[serverId].title,
      PsftpWrapper[serverId].GetLastErrorMessage()
    );
    LogProc_(MSGTYPE_IMPORTANTERROR, log_buf);
    if (PsftpWrapper[serverId].Disconnected() == 1) {
      UnloadPsftpDll(serverId);
      return SFTP_DISCONNECTED;
      //reconnect would be usefull...
    }
    return SFTP_FAILED;
  }

  return SFTP_SUCCESS;
}

//---------------------------------------------------------------------

struct fxp_names* GetCurrentDirectoryStruct(int serverId)
{
  fxp_names* current_directory_struct = NULL;

  // MUST be copied!!!
  my_fxp_names* temp_directory_struct = PsftpWrapper[serverId].GetCurrentDirStruct();

  if (temp_directory_struct != NULL) 
  {
    current_directory_struct = new fxp_names;
    current_directory_struct->nnames = temp_directory_struct->nnames;
    if (current_directory_struct->nnames>=0) 
    {
      current_directory_struct->names =
        (fxp_name*) malloc(sizeof(fxp_name) * temp_directory_struct->nnames);

      for (int i = 0; i < temp_directory_struct->nnames; ++i) 
      {
        current_directory_struct->names[i].attrs =
            temp_directory_struct->names[i]->attrs;
        current_directory_struct->names[i].filename =
            (char*) malloc(sizeof(char) *
                          (strlen(temp_directory_struct->names[i]->filename) +
                           1));
        current_directory_struct->names[i].longname =
            (char *) malloc(sizeof(char) *
                          (strlen(temp_directory_struct->names[i]->longname) +
                           1));
        strcpy(current_directory_struct->names[i].filename,
               temp_directory_struct->names[i]->filename);
        strcpy(current_directory_struct->names[i].longname,
               temp_directory_struct->names[i]->longname);
      }
    }
  }
  return current_directory_struct;
}

//---------------------------------------------------------------------

inline void convert_slash_windows_to_unix(char* string)
{
  unsigned int i = 0;
  unsigned int length = strlen(string);
  for (; i < length; i++) 
  {
    if (string[i] == '\\') string[i] = '/';
  }
}

//---------------------------------------------------------------------

void convert_slash_unix_to_windows(char* string)
{
  unsigned int i = 0;
  unsigned int length = strlen(string);

  for (; i < length; i++) 
  {
    if (string[i] == '/') string[i] = '\\';
  }
}

//---------------------------------------------------------------------

int InitProgressProc(ProgressProcType progressProc, int pluginNr, int serverId)
{
  if (PsftpWrapper[serverId].InitProgressProc != NULL) 
  {
    return PsftpWrapper[serverId].InitProgressProc(progressProc, pluginNr);
  }

  return 0;
}

//---------------------------------------------------------------------

//this function is obsolete. the reason for her existence should be fixed!
int psftp_memory_hole__stopfen(int serverId)
{
  if ((serverId!=-1) && (PsftpWrapper[serverId].SFTP_DLL_FNCT_psftp_memory_hole__stopfen != NULL)) 
  {
    return PsftpWrapper[serverId].SFTP_DLL_FNCT_psftp_memory_hole__stopfen();
  }

  return -1;
}

//---------------------------------------------------------------------

void InitPsftpWrappers()
{
  for (int i = 0; i < MAX_SERVER_COUNT; i++) {
    PsftpWrapper[i].Connect = NULL;
    PsftpWrapper[i].DoSftp = NULL;
    PsftpWrapper[i].GetCurrentDirStruct = NULL;
    PsftpWrapper[i].Disconnect = NULL;
    PsftpWrapper[i].SFTP_DLL_FNCT_psftp_memory_hole__stopfen = NULL;
    PsftpWrapper[i].GetLastErrorMessage = NULL;
    PsftpWrapper[i].InitProgressProc = NULL;
    PsftpWrapper[i].SetSftpServerAccountInfo = NULL;
    PsftpWrapper[i].Disconnected = NULL;
    PsftpWrapper[i].SetTransferMode = NULL;
    PsftpWrapper[i].hDll = NULL;
  }
}

//---------------------------------------------------------------------

void GetPsftpDllPath(char* buf)
{
  char plugin_dir[MAX_CMD_BUFFER];
  GetModuleFileName(ghThisDllModule, plugin_dir, MAX_CMD_BUFFER - 10);

  strcpy(buf, plugin_dir);

  char *p = strrchr(buf, '\\');
  if (!p) p = buf;

  strcpy(p, "\\psftp.dll");
}

//---------------------------------------------------------------------

int FileCopy(char* src, char* dest)
{
  FILE* infp;
  FILE* outfp;
  int c;

  if (strcmp(src, dest) == 0) return 1;

  infp = fopen(src, "rb");

  if (infp == NULL) return 2;

  outfp = fopen(dest, "w+b");
  if (outfp == NULL) 
  {
    fclose(infp);
    return 3;
  }

  //what about buffered copy, or system copy?
  while ((c = getc(infp)) != EOF) putc(c, outfp);

  fclose(infp);
  fclose(outfp);

  return 0;
}

//---------------------------------------------------------------------

void UnlinkTemporaryDllFile(int id)
{
  if (id == -1) return;

  char dll_2_load[MAX_CMD_BUFFER];
  char dll_with_server_id[100];

  sprintf(dll_with_server_id, "psftp_%i.dll", id);
  GetTempPath(MAX_CMD_BUFFER, dll_2_load);
  strcat(dll_2_load, dll_with_server_id);

  remove(dll_2_load);
}

//---------------------------------------------------------------------

void UnlinkAllTemporaryDllFiles()
{
  int i = 0;

  for (; i < MAX_SERVER_COUNT; i++) 
  {
    UnlinkTemporaryDllFile(i);
  }
}

//---------------------------------------------------------------------

int FileExists(char* fname)
{
  FILE *fp;
  int ret = 0;

  fp = fopen(fname, "r");

  if (fp != NULL) 
  {
    ret = 1;
    fclose(fp);
  }

  return ret;
}

//---------------------------------------------------------------------

void GetLastPsftpError(int id, char* buf)
{
  char *msg;
  if (PsftpWrapper[id].GetLastErrorMessage != NULL)
  {
    msg = PsftpWrapper[id].GetLastErrorMessage();

    if (msg == NULL)
    {
      strcpy(buf, "");
    }
    else
    {
      strcpy(buf, msg);
    }
  }
  else
  {
    strcpy(buf, "");
  }
}

//---------------------------------------------------------------------

void DisableLogging()
{
  gDisableRunTimeLogging = 1;
}

//---------------------------------------------------------------------

void DisableLoggingOnce()
{
  gDisableRunTimeLoggingOnce = 1;
}

//---------------------------------------------------------------------

void EnableLogging()
{
  gDisableRunTimeLogging = 0;
}

//---------------------------------------------------------------------

int DoLogging()
{
  if (gDisableRunTimeLoggingOnce == 1) {
    gDisableRunTimeLoggingOnce = 0;
    return 0;
  }
  if (gDisableRunTimeLogging == 1)
    return 0;
  return 1;
}

//---------------------------------------------------------------------

void trim_host_from_hoststring(char* hoststring)
{
  char *buf;
  if (hoststring == NULL)
    return;
  buf = strstr(hoststring, ":");
  if (buf == NULL)
    return;                     //Kein Port angegeben
  char* buf2=strstr(buf+1, ":");
  if (buf2 != NULL)
    return;                     //Kein Port angegeben
  hoststring[strlen(hoststring) - strlen(buf)] = '\0';
}

//---------------------------------------------------------------------

void trim_port_from_hoststring(char* hoststring)
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

