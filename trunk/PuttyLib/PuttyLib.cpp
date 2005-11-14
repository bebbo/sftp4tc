// PuttyLib.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "PuttyLib.h"
#include "windows.h"
#include "fsplugin.h"


#pragma warning( disable : 4786 )
#include <string>
#include <vector>

extern "C" {
#include "sftp4tc_share.h"
#include "pl_misc.h"
#include "pl_psftp.h"

int fxp_disconnected();

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

int __map__disconnected()
{
  return fxp_disconnected();
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

#define ASCII_MODE 0
#define BINARY_MODE 1
#define SMART_MODE 2

int mMode = BINARY_MODE;
std::vector<std::string> mModeDetails;

void CALLBACK __map__setTransferMode(char *mode)
{
  switch (mode[0])
  {
    case 'X':
      {
        mModeDetails.clear();
        std::string smode(mode+1);
        std::string::size_type i = smode.find_first_not_of('*');
        std::string::size_type j;
        while (i!=std::string::npos) {
          j = smode.find_first_of(' ', i);
          if (j==std::string::npos)
            j = smode.length();
          mModeDetails.push_back(smode.substr(i, j-i));
          if (j == smode.length())
            i = std::string::npos;
          else {
            i = smode.find_first_not_of(' ', j);
            i = smode.find_first_not_of('*', i);
          }
        }
        mMode = SMART_MODE;
        break;
      }
    case 'A':
      mMode = ASCII_MODE;
      break;
    default:
      mMode = BINARY_MODE;
  }
  
}

int getTransferMode(char *filename)
{
  if (mMode==SMART_MODE) {
    int i=0;
    int len=strlen(filename);
    while ((i<mModeDetails.size()) && ((len<mModeDetails[i].length()) || (stricmp(filename+(len-mModeDetails[i].length()), mModeDetails[i].c_str())!=0))) i++;
    if (i<mModeDetails.size())
      return ASCII_MODE;
    else
      return BINARY_MODE;
  } else
    return mMode;
}
}