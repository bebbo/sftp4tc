// PuttyLib.cpp : Defines the entry point for the DLL application.
// Here come only few version notices. Everything should be noted in plugin source.
/*
VERSION 1.1.56.1
[27.10.2004]
/UPDATED to PuTTY 0.56 (first beta)
+FIXED Text Transfer Mode (1 character on a line was skipped)

VERSION 1.1.55.3
[19.10.2004]
*ADDED Dumb transfer mode. If one set "Text mode" he should be sure, that it's the best choice 
       for him. There's no check for host's default mode. Assuming *nix.

VERSION 1.1.55.1 
[2.9. 2004]
/UPDATED TO PuTTY 0.55 (first beta)
*/

#include "stdafx.h"
#include "PuttyLib.h"
#include "windows.h"
#include "fsplugin.h"

extern "C" {
#include "share.h"
#include "pl_misc.h"
#include "pl_psftp.h"
} 

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}


int __stdcall __map__wcplg_open_sftp_session(char *user, char *password,
                                             char *host, int port)
{
  ISinitT = 0;
  CurrentDirStruct.nnames = 0;
  names_to_freeing_num = 0;

  return wcplg_open_sftp_session(host, user, password, port);
}

int __stdcall __map__wcplg_close_sftp_session()
{
  return wcplg_close_sftp_session();
}

int __stdcall __map__wcplg_do_sftp(char *_cmd, char *_server_output)
{
  return wcplg_do_sftp(_cmd, _server_output);
}

struct my_fxp_names *__map__wcplg_get_current_dir_struct()
{
  return wcplg_get_current_dir_struct();
}

char *__map__wcplg_get_last_error_msg()
{
  return wcplg_get_last_error_msg();
}

int __stdcall __map__init_ProgressProc(tProgressProc AP_ProgressProc,
                                       int Awc_PluginNr)
{
  return init_ProgressProc(AP_ProgressProc, Awc_PluginNr);
}

int __stdcall __map__psftp_memory_hole__stopfen(void)
{
  return psftp_memory_hole__stopfen();
}

void __stdcall __map__set_Server_config_Struct(struct SftpServerAccountInfo
                                               ServerAccountInfo)
{
  set_Server_config_Struct(ServerAccountInfo);
}

bool mBinary=true;

void CALLBACK __map__setTransferMode(bool binary)
{
  mBinary = binary;
}

extern "C" {
int getTransferMode()
{
  if (mBinary)
    return 1;
  else
    return 0;
}
}