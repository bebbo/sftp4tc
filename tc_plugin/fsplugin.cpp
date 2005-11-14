//disabling that ugly warnings from vc++6.0 about cutting identifiers from 
//template types, like std::map<std::string, fxp_names*>
#pragma warning(disable:4786)

#include "stdafx.h"
#include "fsplugin.h"
#include "sftpmap.h"
#include <direct.h>
#include <stdio.h>
#include "putty_proxy.h"
#include "properties_dlg.h"
#include "share.h"
#include "ServerInfo.h"
#include "passwd_crypter.h"
#include "ConfigProperties.h"
#include "resource.h"

#include <map>
#include <string>

//Plugin's caption, shown in TC's list
#define FSPLUGIN_CAPTION "Secure FTP Connections"

//Registry keys for this plugin
#define PETRICH_TC_REGP	"Software\\petrich\\tc_sftp_plugin"
#define PETRICH_TC_REGP_SFTP_K	"SFtpIniName"

//Registry keys for PuTTY - Session importing
#define PUTTY_REG_POS "Software\\SimonTatham\\PuTTY"
#define PUTTY_REG_PARENT "Software\\SimonTatham"
#define PUTTY_REG_PARENT_CHILD "PuTTY"
#define PUTTY_REG_GPARENT "Software"
#define PUTTY_REG_GPARENT_CHILD "SimonTatham"
static const char *const puttystr = PUTTY_REG_POS "\\Sessions";
//
static const char hex[17] = "0123456789ABCDEF";  //17 because of terminating 0-character
static const char *const DefaultIniFileName = "wcx_sftp.ini";
const int NO_SERVER_ID = -1;
const DWORD S_IFLNK = 0x0A000;

typedef struct {
  char mPath[MAX_PATH];
  HANDLE mhSearch;
  int mSumIndex;
  int mCurrentIndex;
  int mSearchMode;
  fxp_names *mCurrentDirStruct;
} LastFindStuctType;

struct EnumSettingsType {
  HKEY mKey;
  int i;
};

typedef std::map<std::string, fxp_names*> DirCacheType;
typedef DirCacheType::iterator DirCacheIteratorType;

DirCacheType DirCache;
char gLastPath[MAX_PATH];

ConfigPropertiesType gSftpConfig;

HMODULE ghThisDllModule;
bool gDeleteOnlyConnection = false;
bool gAcceptRefresh = false;

//Plugin's initialization values (TC FS Plugin Parameters)
int gPluginNumber;
ProgressProcType gProgressProc;
LogProcType gLogProc;
RequestProcType gRequestProc;

int gCurrentServerId= -1;
static int gDllInitialised = 0;
static HICON gConnectionIcon = NULL;

//config DLL
HMODULE ghDialogDLL = 0;
tProperties ghProperties = 0;
tFreeCfgDLL ghFreeCfgDLL = 0;

//password crypter DLL
HMODULE ghPasswdCrypterDLL = 0;
tSetupPasswordCrypter ghSetupPasswordCrypter = 0;

// session imports
int import_one_putty_session(SftpServerAccountInfoType *serverAccountInfo, 
                       char *puttySectionName);
int import_putty_sessions(int currentId, char *importMode);
int import_ssh_com_sessions(int lastInsertId, char *dirLocation);

// support functions
int octal_permissions_2_tc_integral(unsigned long octalVal);
bool set_local_file_time(char *fullFilePath, FILETIME * lastWriteTime);
void force_valid_server_title(char *title);

// server functions
bool any_connection_active();
void check_concurrent_connection(char *path);
int file_exists_on_remote_server(char *remoteFile);
int get_basename_from_path(char *buf, char *path);
unsigned int get_id_by_path(char *path);
int get_sftp_server_id_by_title(char *title);
int get_sftp_server_id_by_path(char *path);
void load_servers();
int wcplg_sftp_connect_by_id(int id);
int wcplg_sftp_do_commando_by_id(char *sftpCmd, char *serverOutput, int ID);

// dir strcut functions
void free_current_dir_struct(fxp_names * pDirStruct, int id);

const char *SimpleCaption = "PasswordCrypter's master-password";
const char *CaptionAskFirst = "PasswordCrypter's master-password (enter new one)";
const char *CaptionAskSecond = "PasswordCrypter's master-password (verification)";

//---------------------------------------------------------------------

char *local_fs_path_to_remote_path(char* path)
{
  return path + (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
}

//---------------------------------------------------------------------

//copy maximum of maxlen characters
static char *strlcpy(char *p, char *p2, size_t maxlen)
{
  if (strlen(p2) >= maxlen) {
    strncpy(p, p2, maxlen);
    p[maxlen] = 0;
  } else
    strcpy(p, p2);
  return p;
}

//---------------------------------------------------------------------

int get_custom_users_sftp_inifile_from_reg(char *configIniFile)
{
  int ret = -1;
  HKEY subkey1;
  DWORD type;
  DWORD buflen = MAX_PATH;

  if (RegOpenKey(HKEY_CURRENT_USER, PETRICH_TC_REGP, &subkey1) ==
      ERROR_SUCCESS) {
    if (RegQueryValueEx
        (subkey1, PETRICH_TC_REGP_SFTP_K, 0, &type,
         (unsigned char *) configIniFile, &buflen) == ERROR_SUCCESS) {
      ret = 0;
    }

  }

  RegCloseKey(subkey1);

  return ret;
}

//---------------------------------------------------------------------

void __stdcall dbg(char *msg)
{
  if (msg == NULL)
    return;
  gRequestProc(gPluginNumber, RT_MsgOK, "DBG", msg, NULL, 0);
}

//---------------------------------------------------------------------

void dbg_v(char *msg, char *param)
{
  char buf[MAX_MSG_BUFFER];
  _snprintf(buf, MAX_MSG_BUFFER, msg, param);
  dbg(buf);
}

//---------------------------------------------------------------------

void err_v(char *msg, char *param)
{
  char buf[MAX_MSG_BUFFER];
  _snprintf(buf, MAX_MSG_BUFFER, msg, param);

  gRequestProc(gPluginNumber, RT_MsgOK, "Error", buf, NULL, 0);
}

//---------------------------------------------------------------------

void free_cache(int serverId)
{
  for (DirCacheIteratorType dci=DirCache.begin(); dci!=DirCache.end(); dci++) {
    fxp_names* dir_struct = dci->second;
    free_current_dir_struct(dir_struct, serverId);
  }
  DirCache.clear();
}

//---------------------------------------------------------------------

int sftp_disconnect(int serverId, bool logMessage)
{
  if (gSftpConfig.CacheFS) {
    free_cache(gCurrentServerId);
  }

  return Disconnect(serverId, logMessage);
}

//---------------------------------------------------------------------

int remote_name_is_directory(char *path)
{
  char sftp_cmd[MAX_CMD_BUFFER];
  int server_id;

  server_id = get_sftp_server_id_by_path(path);
  if (server_id == NO_SERVER_ID)
    return 0;

  strcpy(sftp_cmd, "ls \"./");
  strcat(sftp_cmd, path + (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title) + 1));
  strcat(sftp_cmd, "\"");

  convert_slash_windows_to_unix(sftp_cmd);

  //DISABLE_LOGGING();  
  if (wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, server_id) !=
      SFTP_SUCCESS) {
    //ENABLE_LOGGING();
    return 0;
  }
  //ENABLE_LOGGING();

  return 1;
}

//---------------------------------------------------------------------

extern "C" BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  UNUSED_ARG(lpReserved);
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    {
      gDllInitialised++;
      //Get the ini's filename
      if (get_custom_users_sftp_inifile_from_reg(gSftpConfig.ConfigIniFile) == -1) {
        ghThisDllModule = (HMODULE) hModule;
        GetModuleFileName(ghThisDllModule, gSftpConfig.ConfigIniFile, 
          sizeof(gSftpConfig.ConfigIniFile) - 1);
        char *p = strrchr(gSftpConfig.ConfigIniFile, '\\');
        if (p)
          p++;
        else
          p = gSftpConfig.ConfigIniFile;
        strcpy(p, DefaultIniFileName);
      }
      break;
    }
  case DLL_PROCESS_DETACH:
    {
      gDllInitialised--;
      if (gDllInitialised == 0) try {
        if (ghDialogDLL) {
          ghFreeCfgDLL();
          FreeLibrary(ghDialogDLL);
          ghDialogDLL = NULL;
          ghProperties = NULL;
          ghFreeCfgDLL = NULL;
        }
        if (ghPasswdCrypterDLL) {
          FreeLibrary(ghPasswdCrypterDLL);
          ghPasswdCrypterDLL = 0;
          gSftpConfig.PasswordCrypterPath[0] = 0;
        }
      } catch (...) {
      }
    }
  }
  return TRUE;
}
//---------------------------------------------------------------------

int server_title_exists(int numServer, int currentServerId, char *title)
{
  int i;
  for (i = 0; i < numServer; i++) {
    if (i != currentServerId && strcmp(gSftpConfig.ServerInfos[i].title, title) == 0) {
      return 1;
    }
  }
  return 0;
}

//---------------------------------------------------------------------

void make_server_titles_unique(int serverCount)
{
  char buf[MAX_SERVER_INFO];
  char buf2[MAX_SERVER_INFO];

  for (int i = 0; i < serverCount; i++) {
    int dbl = 0;
    strcpy(buf, gSftpConfig.ServerInfos[i].title);
    while (1)                   // :-)
    {
      if (server_title_exists(serverCount, i, buf) == 1) {
        dbl++;
        sprintf(buf2, "%d", (dbl + 1));
        strcpy(buf, gSftpConfig.ServerInfos[i].title);
        strcat(buf, " (");
        strcat(buf, buf2);
        strcat(buf, ")");
        continue;
      }
      if (dbl > 0) {
        strcpy(gSftpConfig.ServerInfos[i].title, buf);
      }
      break;
    }
  }
}

//---------------------------------------------------------------------

int __stdcall FsInit(int PluginNr, ProgressProcType pProgressProc,
                     LogProcType pLogProc, RequestProcType pRequestProc)
{
  //initialise global variables
  gLastPath[0] = '\0';
  gConnectionIcon = LoadIcon(ghThisDllModule, MAKEINTRESOURCE(IDI_ICON_CONNECTION));

  //remember all those values
  gProgressProc = pProgressProc;
  gLogProc = pLogProc;
  gRequestProc = pRequestProc;
  gPluginNumber = PluginNr;

  //initialize all servers
  load_servers();
  InitPsftpWrappers();
  UnlinkAllTemporaryDllFiles();
  ResetAlreadyConnected();

  //dialog dll present?
  char cDir[MAX_CMD_BUFFER], cDLL[MAX_CMD_BUFFER];
  GetModuleFileName(ghThisDllModule, cDir, MAX_CMD_BUFFER);
  char *p = strrchr(cDir, '\\');
  p[1] = 0;

  _snprintf(cDLL, MAX_CMD_BUFFER, "%s%s", cDir, DIALOG_DLL);

  ghDialogDLL = LoadLibrary(cDLL);
  if (ghDialogDLL) {
    
    ghProperties = (tProperties)GetProcAddress(ghDialogDLL, PROPERTIES_FUNCTION);
    tInitialize hInitialize = (tInitialize)GetProcAddress(ghDialogDLL, INITIALIZE_FUNCTION);
    ghFreeCfgDLL = (tFreeCfgDLL)GetProcAddress(ghDialogDLL, FREE_FUNCTION);
    if ((ghProperties==NULL) ||(hInitialize==NULL) || (ghFreeCfgDLL==NULL))
    {
      FreeLibrary(ghDialogDLL);
      ghDialogDLL=0;
    } else {
      hInitialize(ghDialogDLL);
    }
  }
  return 0;
}
//---------------------------------------------------------------------

bool UnixTimeToLocalTime(long *mtime, LPFILETIME ft)
{
  time_t lmtime = *mtime;
  struct tm *fttm = localtime(&lmtime);
  SYSTEMTIME st;
  FILETIME ft2;

  st.wYear = fttm->tm_year + 1900;

  /*
     // M$ Help says something different :-(
     if ((st.wYear>=30 && st.wYear<=99) || (st.wYear<200 && st.wYear>=100))
     st.wYear+=1900;
     else if (st.wYear<100)
     st.wYear+=2000;
   */
  st.wMonth = fttm->tm_mon + 1; // 0-11 in struct tm*
  st.wDay = fttm->tm_mday;
  st.wHour = fttm->tm_hour;
  st.wMinute = fttm->tm_min;
  st.wSecond = fttm->tm_sec;
  st.wDayOfWeek = 0;
  st.wMilliseconds = 0;
  if (SystemTimeToFileTime(&st, &ft2)) {
    return (LocalFileTimeToFileTime(&ft2, ft)==TRUE); // Wincmd expects system time!
  } else
    return false;
}
//---------------------------------------------------------------------

HANDLE __stdcall FsFindFirst(char *Path, WIN32_FIND_DATA * FindData)
{
  // user is trying to delete a connection!
  if (gDeleteOnlyConnection) {
    SetLastError(ERROR_NO_MORE_FILES);
    return INVALID_HANDLE_VALUE;
  }

  LastFindStuctType* lf;
  lf = (LastFindStuctType*) malloc(sizeof(LastFindStuctType));
  lf->mCurrentDirStruct = NULL;

  memset(FindData, 0, sizeof(WIN32_FIND_DATA));

  if (strcmp(Path, "\\") == 0) {
    // Connection selected
    if (!any_connection_active()) //Load Servers if there is no active connection
    {
      load_servers();
    }
   
    lf->mCurrentIndex = 0;
    lf->mSumIndex = gSftpConfig.ServerCount;
    if (!ghDialogDLL)
      lf->mSumIndex++;
    strcpy(FindData->cFileName, gSftpConfig.ServerInfos[lf->mCurrentIndex].title);

    FindData->dwFileAttributes = /*FILE_ATTRIBUTE_DIRECTORY |*/ FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000;
    FindData->dwReserved0 |= S_IFLNK; // Wincmd uses only this one!
    FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
    FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
    lf->mhSearch = INVALID_HANDLE_VALUE;
    lf->mSearchMode = HANDLE__SHOW_SFTP_SERVER;
    return (HANDLE) lf;
  } else {
    // So we have a connection - list selected directory
    lf->mSearchMode = HANDLE__SHOW_SFTP_DIR;
    // check, which server is requested
    check_concurrent_connection(Path);

    //it's fatal if it doesn't even exists!
    if (gCurrentServerId == NO_SERVER_ID) {
      dbg("fsfindfirst: gCurrentServerId == -1, FIX ME!");
      SetLastError(ERROR_INVALID_ACCESS);
      return INVALID_HANDLE_VALUE;
    }

    char sftp_cmd[MAX_CMD_BUFFER];
    char *lPath = Path;
    fxp_names* CurrentDirStruct=NULL;
    std::string lFullPath(lPath);
    if (lFullPath[lFullPath.length()-1]=='\\')
      lFullPath = lFullPath.substr(0, lFullPath.length()-1);

    strlcpy(lf->mPath, Path, MAX_PATH);
    lPath += (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title)); //skip backslash and title ('\CONNECTION')

    if (gSftpConfig.CacheFS) 
    {
      lFullPath = Path;
      if (lFullPath[lFullPath.length()-1]=='\\')
        lFullPath = lFullPath.substr(0, lFullPath.length()-1);
      if ((!gAcceptRefresh) || (strcmp(lFullPath.c_str(), gLastPath)!=0)) {
        CurrentDirStruct = DirCache[lFullPath];
      }
      strncpy(gLastPath, lFullPath.c_str(), MAX_PATH);
    }    

    if (!CurrentDirStruct) {
      int ret;

      //prepare command line
      if (strcmp(lPath, "") != 0) {
        _snprintf(sftp_cmd, MAX_CMD_BUFFER, "cd \"%s\"", lPath);
      } else {
        _snprintf(sftp_cmd, MAX_CMD_BUFFER, "cd /");
      }

      //change that command line to unix style
      convert_slash_windows_to_unix(sftp_cmd);

      //execute commando
      ret = wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId);
      if (ret == SFTP_DISCONNECTED) {
        ret = wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId);
      }
      if (ret != SFTP_SUCCESS) {
        LogProc_(MSGTYPE_CONNECTCOMPLETE, "Access denied!");
        SetLastError(ERROR_ACCESS_DENIED);
        return INVALID_HANDLE_VALUE;
      }

      strcpy(sftp_cmd, "ls");
      //execute commando
      if (wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId) !=
          SFTP_SUCCESS) {
        LogProc_(MSGTYPE_CONNECTCOMPLETE, "Access denied!");
        SetLastError(ERROR_ACCESS_DENIED);
        return INVALID_HANDLE_VALUE;
      }

      //get the directory listing
      CurrentDirStruct = GetCurrentDirectoryStruct(gCurrentServerId);

      if ((gSftpConfig.CacheFS) && (CurrentDirStruct)) {
        fxp_names* old_dir_struct = DirCache[lFullPath];
        if ((old_dir_struct) && (CurrentDirStruct!=old_dir_struct)) {
          DirCache.erase(lFullPath);
          free_current_dir_struct(old_dir_struct, gCurrentServerId);
        }
        DirCache[lFullPath] = CurrentDirStruct;
      }
    }

    if (CurrentDirStruct == NULL) {
      //this should come if you were disconnected. just a badness for 
      //reconnection :-) maybe we should read the ini for the right password :-) 
      //or better don't erase it if it was in .ini 
      char buf_log[MAX_CMD_BUFFER];
      GetLastPsftpError(gCurrentServerId, buf_log);
      if (strcmp(buf_log, UNEXPECTED_OK_MSG) == 0) {
        sprintf(buf_log, "permission denied");
        LogProc_(MSGTYPE_DETAILS, buf_log);
        SetLastError(ERROR_INVALID_ACCESS);
        return (HANDLE) INVALID_HANDLE_VALUE;
      } else {
        sftp_disconnect(gCurrentServerId, false);
        return FsFindFirst(Path, FindData);
      }

    }

    lf->mCurrentDirStruct = CurrentDirStruct;
    lf->mSumIndex = CurrentDirStruct->nnames;
    lf->mCurrentIndex = 0;

    //skip some directories
    while ((lf->mCurrentIndex < lf->mSumIndex) &&
           ((strlen(lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename)==0)
            || 
            (strcmp
             (lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename,
              ".") == 0)
            ||
            (strcmp
             (lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename,
              "..") == 0)
            || ((gSftpConfig.ServerInfos[gCurrentServerId].show_hidden_files=='0') && (lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename[0]=='.'))
           )
          )
      (lf->mCurrentIndex)++;

    //is there anything?
    if (lf->mCurrentIndex >= lf->mCurrentDirStruct->nnames) {
      if (lf->mCurrentDirStruct->nnames<0) {
        LogProc_(MSGTYPE_DETAILS, "Access denied(*)!");
        SetLastError(ERROR_ACCESS_DENIED);
      } else
        SetLastError(ERROR_NO_MORE_FILES);

      if (!gSftpConfig.CacheFS) {
        free_current_dir_struct(CurrentDirStruct, gCurrentServerId);
      }
      return INVALID_HANDLE_VALUE;
    }

    //Copy that info to TotalCommander's structure
    char FileTyp;
    FileTyp = lf->mCurrentDirStruct->names[lf->mCurrentIndex].longname[0];
    FindData->dwReserved0 =
      octal_permissions_2_tc_integral(lf->mCurrentDirStruct->names[lf->mCurrentIndex].
                          attrs.permissions);

    if (FileTyp == 'd') {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | 0x80000000;
    } else if (FileTyp == 'l') {
      FindData->dwFileAttributes =
        FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000;
      FindData->dwReserved0 |= S_IFLNK; // Wincmd uses only this one!
    } else {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL | 0x80000000;
    }

    if (!UnixTimeToLocalTime
        ((long *) &lf->mCurrentDirStruct->names[lf->mCurrentIndex].attrs.
         mtime, &FindData->ftLastWriteTime)) {
      FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
      FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
    }

    FindData->nFileSizeHigh =
      (DWORD) lf->mCurrentDirStruct->names[lf->mCurrentIndex].attrs.size.hi;
    FindData->nFileSizeLow =
      (DWORD) lf->mCurrentDirStruct->names[lf->mCurrentIndex].attrs.size.lo;
    strcpy(FindData->cFileName,
           CurrentDirStruct->names[lf->mCurrentIndex].filename);

    return (HANDLE) lf;
  }
  return INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------

BOOL __stdcall FsFindNext(HANDLE Hdl, WIN32_FIND_DATA * FindData)
{
  LastFindStuctType* lf;
  lf = (LastFindStuctType*) Hdl;

  //is it a connection listing?
  if (lf->mSearchMode == HANDLE__SHOW_SFTP_SERVER) {
    if (lf->mCurrentIndex >= lf->mSumIndex)
      return false;             //this is the last entry 

    //copy entry
    if (lf->mCurrentIndex == lf->mSumIndex - 1) {
      lf->mCurrentIndex++;
      strcpy(FindData->cFileName, DefineEditConnection_define);
      FindData->dwFileAttributes = 0;
    } else {
      if ((lf->mCurrentIndex == lf->mSumIndex - 2) && (!ghDialogDLL)) {
        lf->mCurrentIndex++;
        strcpy(FindData->cFileName, DefineAddConnection_define);
        FindData->dwFileAttributes = 0;
      } else {
        lf->mCurrentIndex++;
        strcpy(FindData->cFileName, gSftpConfig.ServerInfos[lf->mCurrentIndex].title);
        FindData->dwFileAttributes = /*FILE_ATTRIBUTE_DIRECTORY |*/ FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000;
        FindData->dwReserved0 |= S_IFLNK; // Wincmd uses only this one!
      }
    }
    FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
    FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
    return true;
  }

  //it's not a connection listing, it's real directory list
  if (lf->mSearchMode == HANDLE__SHOW_SFTP_DIR) {
    lf->mCurrentIndex++;
    while ((lf->mCurrentIndex < lf->mSumIndex) &&
           ((strlen(lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename)==0)
            || 
            (strcmp
             (lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename,
              ".") == 0)
            ||
            (strcmp
             (lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename,
              "..") == 0)
            || ((gSftpConfig.ServerInfos[gCurrentServerId].show_hidden_files=='0') && (lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename[0]=='.'))
            ))
      (lf->mCurrentIndex)++;

    if (lf->mCurrentIndex >= lf->mSumIndex)
      return false;             //it's the last entry
    strcpy(FindData->cFileName,
           lf->mCurrentDirStruct->names[lf->mCurrentIndex].filename);

    FindData->dwReserved0 =
      octal_permissions_2_tc_integral(lf->mCurrentDirStruct->names[lf->mCurrentIndex].
                          attrs.permissions);

    char FileTyp;
    FileTyp = lf->mCurrentDirStruct->names[lf->mCurrentIndex].longname[0];
    if (FileTyp == 'd')         //directory
    {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | 0x80000000;
    } else if (FileTyp == 'l')  //link
    {
      FindData->dwFileAttributes =
        FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000;
      FindData->dwReserved0 |= S_IFLNK; //TotalCommander uses only this one!
    } else                      //anything else :-)
    {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL | 0x80000000;
    }

    if (!UnixTimeToLocalTime
        ((long *) &lf->mCurrentDirStruct->names[lf->mCurrentIndex].attrs.
         mtime, &FindData->ftLastWriteTime)) {
      FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
      FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
    }

    FindData->nFileSizeHigh =
      (DWORD) lf->mCurrentDirStruct->names[lf->mCurrentIndex].attrs.size.hi;
    FindData->nFileSizeLow =
      (DWORD) lf->mCurrentDirStruct->names[lf->mCurrentIndex].attrs.size.lo;

    return true;
  }

  dbg("FIX ME: this is not a regular listing mode!");
  return false;
}
//---------------------------------------------------------------------

int __stdcall FsFindClose(HANDLE Hdl)
{
  LastFindStuctType* lf;
  lf = (LastFindStuctType*) Hdl;

  if (lf != INVALID_HANDLE_VALUE) {
    if (!gSftpConfig.CacheFS)
      free_current_dir_struct(lf->mCurrentDirStruct, gCurrentServerId);
    else
      lf->mCurrentDirStruct = NULL;
    free(lf);                   //now we can free this
  }

  return 0;
}
//---------------------------------------------------------------------

//free all buffers
void free_current_dir_struct(fxp_names * P_DirStruct, int ServerID)
{
  if (P_DirStruct != NULL) {
    for (int i = 0; i < P_DirStruct->nnames; i++) {
      free(P_DirStruct->names[i].filename);
      free(P_DirStruct->names[i].longname);
    }
    free(P_DirStruct);
  }

  int n;
  n = psftp_memory_hole__stopfen(ServerID); //well, this seems not to be used and what about n? :-(
}
//---------------------------------------------------------------------

bool addnewserver(char *serverTitle)
{
  char host[MAX_PATH], user[MAX_PATH], pwd[MAX_PATH], port[MAX_PATH];
  char home_dir[MAX_PATH];
  char section_name[MAX_SERVER_INFO];
  char server_title[MAX_SERVER_INFO];
  bool wantcompression;

  server_title[0] = '\0';

  host[0] = 0;
  user[0] = 0;
  pwd[0] = 0;
  home_dir[0] = '/';
  home_dir[1] = 0;

  if (serverTitle == NULL) {
    while (strlen(server_title) <= 0) {
      if (!gRequestProc
          (gPluginNumber, RT_Other, "Server Title", "Server Title:",
           server_title, sizeof(server_title) - 1))
        return false;
    }
  } else {
    strcpy(server_title, serverTitle);
  }

  force_valid_server_title(server_title);

  if (!gRequestProc
      (gPluginNumber, RT_Other, "New Connection", "Host[:port]", host,
       sizeof(host) - 1))
    return false;

  strcpy(port, host);
  trim_host_from_hoststring(host);
  trim_port_from_hoststring(port);
  if (!strlen(port))
    strcpy(port, "22");

  if (!gRequestProc
      (gPluginNumber, RT_UserName, "New Connection", NULL, user,
       sizeof(user) - 1))
    return false;

  if (!gRequestProc
      (gPluginNumber, RT_Password, "New Connection", NULL, pwd,
       sizeof(user) - 1))
    return false;

  if (!gRequestProc
      (gPluginNumber, RT_TargetDir, "New Connection", NULL, home_dir,
       sizeof(user) - 1))
    return false;

  // RequestProc mit RT_MsgYesNo Funzt nicht, gibt immer FALSE zurück

  wantcompression =
    gRequestProc(
        gPluginNumber, 
        RT_MsgOKCancel,
        "Add connection - compression",
        "Do you want to compress the data connection (only recommended for slow connections)?",
        NULL, 
        0) == TRUE;

  sprintf(section_name, "%i", gSftpConfig.ServerCount - gSftpConfig.ImportedSessions);
  WritePrivateProfileString(section_name, "title", server_title, gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "host", host, gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "username", user, gSftpConfig.ConfigIniFile);
  if ((gSftpConfig.EncryptPassword) && (strlen(gSftpConfig.PasswordCrypterPassword)>0)) {
    char buf[4000];
    memset(buf, 0, sizeof(buf));
    gSftpConfig.EncryptPassword(pwd, buf);
    WritePrivateProfileString(section_name, "password", buf, gSftpConfig.ConfigIniFile);
  } else {
    WritePrivateProfileString(section_name, "password", pwd, gSftpConfig.ConfigIniFile);
  }
  WritePrivateProfileString(section_name, "port", port, gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "home_dir", home_dir, gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "compression",
                            wantcompression ? "1" : "0", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "use_key_auth", "0", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "keyfilename", "", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "dont_ask4_passphrase", "0",
                            gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "dont_ask4_username",
                            strcmp(user, "") == 0 ? "0" : "1", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "dont_ask4_password",
                            strcmp(pwd, "") == 0 ? "0" : "1", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_type", "0", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_host", "", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_port", "0", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_username", "", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_password", "", gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_telnet_command", "",
                            gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "chmod_value_put", "",
                            gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "chmod_value_mkdir", "",
                            gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "set_chmod_after_put", "0",
                            gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "set_chmod_after_mkdir", "0",
                            gSftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "set_mtime_after_put", "1",
                            gSftpConfig.ConfigIniFile);

  load_servers();
  return true;
}
//---------------------------------------------------------------------

#define MOVE_INFO(KEY) GetPrivateProfileString(section_name_old, KEY, "", copybuf, \
                                MAX_SERVER_INFO, gSftpConfig.ConfigIniFile); \
        WritePrivateProfileString(section_name_new, KEY, copybuf, \
                                  gSftpConfig.ConfigIniFile);

#define MOVE_INFO_PARAM(KEY, PARAM) GetPrivateProfileString(section_name_old, KEY, PARAM, copybuf, \
                                MAX_SERVER_INFO, gSftpConfig.ConfigIniFile); \
        WritePrivateProfileString(section_name_new, KEY, copybuf, \
                                  gSftpConfig.ConfigIniFile); 

#define MOVE_INFO_WRITEPARAM(KEY, PARAM) GetPrivateProfileString(section_name_old, KEY, "", copybuf, \
                                MAX_SERVER_INFO, gSftpConfig.ConfigIniFile); \
        WritePrivateProfileString(section_name_new, KEY, PARAM, \
                                  gSftpConfig.ConfigIniFile); 
//---------------------------------------------------------------------

bool deletethisconnection(char *serverTitle)
{
  int i, j, Sid;
  char section_name_old[MAX_SERVER_INFO];
  char section_name_new[MAX_SERVER_INFO];

  Sid = get_sftp_server_id_by_title(serverTitle);
  if (Sid == -1) {
    gRequestProc(gPluginNumber, RT_MsgOK, "Delete connection",
                "Delete Error: deletethisconnection(): Sid=-1 FIX me plz - FAST",
                NULL, 0);
    return false;
  }
  if (gSftpConfig.ServerInfos[Sid].is_imported_from_any_datasrc == 1) {
    if (gSftpConfig.ServerInfos[Sid].is_imported_from_putty_registry == 1) {
      gRequestProc(gPluginNumber, RT_MsgOK, "Delete connection",
                  "This connection is imported from the PuTTY Session Database - can not delete",
                  NULL, 0);
      return false;
    }
    if (gSftpConfig.ServerInfos[Sid].is_imported_from_sshcom_registry == 1) {
      gRequestProc(gPluginNumber, RT_MsgOK, "Delete connection",
                  "This connection is imported from the ssh.com Session Database - can not delete",
                  NULL, 0);
      return false;
    }
    gRequestProc(gPluginNumber, RT_MsgOK, "Delete connection",
                "This connection is imported  - can not delete", NULL, 0);
    return false;
  }

  for (i = 0; i < gSftpConfig.ServerCount - 1 - gSftpConfig.ImportedSessions; i++) {
    if (strcmp(gSftpConfig.ServerInfos[i].title, serverTitle) == 0) {
      if (gSftpConfig.ServerInfos[i].is_imported_from_any_datasrc == 1) {
        gRequestProc(gPluginNumber, RT_MsgOK, "Delete connection",
                    "This connection is imported from the PuTTY Session Database - can not delete",
                    NULL, 0);
        return false;
      }
      // Delete server i by moving all sections 1 up
      // What about Section rename?
      i++;                      // It's 1-based in the ini file!
      for (j = i; j < gSftpConfig.ServerCount - gSftpConfig.ImportedSessions; j++) {
        sprintf(section_name_old, "%i", j + 1);
        sprintf(section_name_new, "%i", j);
        char copybuf[MAX_SERVER_INFO];

        MOVE_INFO("title");
        MOVE_INFO("host")
        MOVE_INFO("username")
        MOVE_INFO_WRITEPARAM("port", copybuf[0] ? copybuf : NULL)
        MOVE_INFO_WRITEPARAM("home_dir", copybuf[0] ? copybuf : NULL)
        MOVE_INFO_PARAM("compression", "0")
        MOVE_INFO("keyfilename")
        MOVE_INFO("dont_ask4_passphrase")
        MOVE_INFO("dont_ask4_password")
        MOVE_INFO_PARAM("use_key_auth", "0")
        MOVE_INFO_PARAM("dont_ask4_username", "0")
        MOVE_INFO("password")
        MOVE_INFO_PARAM("proxy_type", "0")
        MOVE_INFO("proxy_host")
        MOVE_INFO("proxy_port")
        MOVE_INFO("proxy_username")
        MOVE_INFO("proxy_password")
        MOVE_INFO("proxy_telnet_command")
        MOVE_INFO("chmod_value_put")
        MOVE_INFO("chmod_value_mkdir")
        MOVE_INFO_PARAM("set_chmod_after_put", "0")
        MOVE_INFO_PARAM("set_chmod_after_mkdir", "0")
        MOVE_INFO_PARAM("set_mtime_after_put", "1")
      }
      // delete last section
      sprintf(section_name_old, "%i", gSftpConfig.ServerCount - 1 - gSftpConfig.ImportedSessions);
      WritePrivateProfileString(section_name_old, NULL, NULL, gSftpConfig.ConfigIniFile);
      load_servers();
      return TRUE;
    }
  }
  return FALSE;
}
//---------------------------------------------------------------------

int remote_chmod(char *RemoteName, unsigned int value)
{
  check_concurrent_connection(RemoteName);
  char *lRemoteName = RemoteName + (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title) + 1);
  char sftp_cmd[MAX_CMD_BUFFER];

  _snprintf(sftp_cmd, MAX_CMD_BUFFER, "chmod %d \"%s\"", value, lRemoteName);

  convert_slash_windows_to_unix(sftp_cmd);

  if (wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId) == SFTP_SUCCESS) {
    return FS_EXEC_OK;
  }
    
  return FS_EXEC_ERROR;
}
//---------------------------------------------------------------------

int remote_mtime(char *RemoteName, time_t value)
{
  check_concurrent_connection(RemoteName);
  char *lRemoteName = RemoteName + (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
  char sftp_cmd[MAX_CMD_BUFFER];

  _snprintf(sftp_cmd, MAX_CMD_BUFFER, "mtime %d \"%s\"", value, lRemoteName);

  convert_slash_windows_to_unix(sftp_cmd);

  if (wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId) == SFTP_SUCCESS) {
    return FS_EXEC_OK;
  }
    
  return FS_EXEC_ERROR;
}
//---------------------------------------------------------------------

BOOL __stdcall FsMkDir(char *Path)
{
  char cmd_buf[MAX_CMD_BUFFER];
  char *lPath = Path;

  if (strchr(Path + 1, '\\') == NULL) { // Create new server!
    return addnewserver(Path + 1);
  }

  check_concurrent_connection(Path);

  lPath += (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));

  sprintf(cmd_buf, "mkdir \"%s\"", lPath);

  convert_slash_windows_to_unix(cmd_buf);

  if (wcplg_sftp_do_commando_by_id(cmd_buf, NULL, gCurrentServerId) == SFTP_SUCCESS)
  {
    if (gSftpConfig.ServerInfos[gCurrentServerId].set_chmod_after_mkdir)
      remote_chmod(Path, gSftpConfig.ServerInfos[gCurrentServerId].chmod_value_mkdir); 
    return true;
  }
  return false;
}
//---------------------------------------------------------------------

void DefineAndAddConnection()
{
  HANDLE myFile;

  myFile = CreateFile(gSftpConfig.ConfigIniFile, // file name
    GENERIC_READ,     // access mode
    FILE_SHARE_READ,  // share mode
    NULL,             // security descriptor
    OPEN_ALWAYS,      // how to create
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
    NULL); // handle to template file

  if (myFile == INVALID_HANDLE_VALUE)
  {
    err_v("Error opening '%s'", gSftpConfig.ConfigIniFile);
  } else {
    CloseHandle(myFile);
  }

  ShellExecute(0, "open", gSftpConfig.ConfigIniFile, NULL, "c:\\", SW_SHOW);
}
//---------------------------------------------------------------------

char *strlcat(char *p, char *p2, size_t maxlen)
{
  size_t restlen = maxlen - strlen(p);
  if (restlen > 0)
    strlcpy(p + strlen(p), p2, restlen);
  return p;
}
//---------------------------------------------------------------------

void formcorrectpath(char *completepath)
{
  int j, j0, i;
  char *p, *p1, *pEnd;
  char ch;
  char *searchpos = completepath;

  do {
    j = 0;
    p = strstr(searchpos, "\\.");
    if (p)
      j = 2;
    else {                      // Detect also \ ..\ or \ ..#0}
      p = strstr(searchpos, "\\ .");
      if (p)
        j = 3;
    }
    if (p) {
      j0 = j + 1;
      while (p[j] == '.')
        j++;                    // cd ...
      ch = p[j];
      pEnd = &p[j + 1];
      if (ch == 0 || ch == '\\') {  //cd ..
        p[1] = 0;
        for (i = j0; i <= j; i++) //For each additional '.'
        {
          //Delete previous path
          p[0] = 0;
          p1 = strrchr(completepath, '\\');
          if (p1) {
            p1[1] = 0;
            if (ch == '\\')     //Copy rest of path!
              memmove(p1 + 1, pEnd, strlen(pEnd) + 1);
          } else
            p[0] = '\\';
          p = p1;
        }
        searchpos = completepath;
      } else                    //.. belongs to long name!
        searchpos = &p[3];
    }
  } while (p);
}
//---------------------------------------------------------------------

void SetMTime(char* LocalName, char* RemoteName)
{
  if (gSftpConfig.ServerInfos[gCurrentServerId].set_mtime_after_put)
  {
    bool f;
    HANDLE myFile;
    FILETIME LastWriteTime, LastWriteTimeUTC;

    myFile = CreateFile(LocalName, // file name
      GENERIC_READ,     // access mode
      FILE_SHARE_READ,  // share mode
      NULL,             // security descriptor
      OPEN_EXISTING,    // how to create
      FILE_ATTRIBUTE_NORMAL | // file attributes
      FILE_FLAG_SEQUENTIAL_SCAN, NULL); // handle to template file

    f = (myFile != INVALID_HANDLE_VALUE);

    if (f)
    {
      f = (GetFileTime(myFile,       // gets last-write time for file
         NULL, NULL, &LastWriteTimeUTC)==TRUE);
      CloseHandle(myFile);
    }

    if (f) {
      SYSTEMTIME stLocal;
      TIME_ZONE_INFORMATION tzinfo;
      DWORD info = GetTimeZoneInformation(&tzinfo);
      tm atm;
      FileTimeToLocalFileTime(&LastWriteTimeUTC, &LastWriteTime);
      FileTimeToSystemTime(&LastWriteTime, &stLocal);

      atm.tm_year = stLocal.wYear - 1900;
      atm.tm_mon = stLocal.wMonth - 1;
      atm.tm_mday = stLocal.wDay;
      atm.tm_hour = stLocal.wHour;
      atm.tm_min = stLocal.wMinute;
      atm.tm_sec = stLocal.wSecond;
      atm.tm_isdst = -1;
      time_t new_mtime = mktime(&atm);
      
      remote_mtime(RemoteName, new_mtime);
    }
  }
}

//---------------------------------------------------------------------

//This function needs a real revision
int __stdcall FsExecuteFile(HWND MainWin, char *RemoteName, char *Verb)
{
  char buf[MAX_CMD_BUFFER];
  char sftp_cmd[MAX_CMD_BUFFER];
  char log_sftp_cmd[MAX_CMD_BUFFER];
  char *lVerb = Verb;
  char *lRemoteName = RemoteName;

  gSftpConfig.MainWindow = MainWin;

  if (stricmp(Verb, "open") == 0) {
    if (strcmp(DefineEditConnection_selected, RemoteName) == 0) {
      if (ghDialogDLL) {
        if (ghProperties(0, &gSftpConfig)) {
          RemoteName[1] = '\0';
          return FS_EXEC_SYMLINK;
        }
      } else {
        DefineAndAddConnection();
      }
      
      return FS_EXEC_OK;
    }

    if (strcmp(DefineAddConnection_selected, RemoteName) == 0) {
      bool ret = false;
      if (ghDialogDLL) {
        ret = ghProperties(1, &gSftpConfig);
      } else {
        ret = addnewserver(NULL);
      }
      if (ret) {
        RemoteName[1] = '\0';
        return FS_EXEC_SYMLINK;
      }
      return FS_EXEC_OK;
    }

    check_concurrent_connection(RemoteName);

    if ((strchr(RemoteName+1, '\\') == NULL) && (!IsAlreadyConnected()) && (wcplg_sftp_connect_by_id(gCurrentServerId)) == SFTP_SUCCESS) {
      strcat(RemoteName, gSftpConfig.ServerInfos[gCurrentServerId].base_dir);
      convert_slash_unix_to_windows(RemoteName);
      return FS_EXEC_SYMLINK;
    } else if (remote_name_is_directory(RemoteName)) {
      return FS_EXEC_SYMLINK;
    }
  }

  //explicit REPUT SUPPORT
  if (stricmp(Verb, "quote reput") == 0 || stricmp(Verb, "quote reput ") == 0
      || (strlen(Verb) >= strlen("quote reput  ")
          && strncmp(Verb, "quote reput  ", 13) == 0)) {
    gRequestProc(gPluginNumber, RT_MsgOK, "Usage for reput", "Usage: reput <file>", NULL, 0);
    return FS_EXEC_ERROR;
  }

  if (strlen(Verb) > strlen("quote reput") + 1) {
    //check if connected and inside of a connection
    if ((RemoteName != NULL) && (strlen(RemoteName) == 1) && (RemoteName[0] == '\\')) {
      gRequestProc(gPluginNumber, RT_MsgOK, "Reput", "Can not use reput here", NULL, 0);
      return FS_EXEC_ERROR;
    }

    strcpy(buf, Verb);
    if (_strnicmp(buf, "quote reput ", 12) == 0) {
      char remote_users_current_dir[MAX_CMD_BUFFER];
      char remote_filename[MAX_CMD_BUFFER];
      int sftp_ret;

      if ((RemoteName == NULL) || (strlen(RemoteName) <= 0)
          || (strchr(RemoteName, '?'))) {
        dbg_v("Invalid RemoteName ('%s') FIX ME PLZ", RemoteName);
        return FS_EXEC_ERROR;
      }

      check_concurrent_connection(RemoteName);

      strncpy(remote_users_current_dir, local_fs_path_to_remote_path(RemoteName), MAX_CMD_BUFFER);

      convert_slash_windows_to_unix(remote_users_current_dir);

      char *local_name = buf + strlen("quote reput") + 1;

      //Log msg
      _snprintf(sftp_cmd, MAX_CMD_BUFFER, "reput \"%s\" \"%s\"", local_name, remote_users_current_dir);

      gProgressProc(gPluginNumber, local_name, remote_filename, 0);

      LogProc_(MSGTYPE_DETAILS, sftp_cmd);
      DisableLoggingOnce();

      sftp_ret = wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId);
      //Put done
      gProgressProc(gPluginNumber, local_name, remote_filename, 100);

      //any error?
      if (sftp_ret == SFTP_SUCCESS) {
        SetMTime(local_name, RemoteName);
        return FS_EXEC_OK;
      }

      return FS_EXEC_ERROR;
    }
  }

  //explicit REGET SUPPORT
  if (strcmp(Verb, "quote reget") == 0 || strcmp(Verb, "quote reget ") == 0
      || (strlen(Verb) >= strlen("quote reget  ")
          && strncmp(Verb, "quote reget  ", 13) == 0)) {
    gRequestProc(gPluginNumber, RT_MsgOK, "Usage for reget",
                "Usage: reget <file>", NULL, 0);
    return FS_EXEC_ERROR;
  }

  if (strlen(Verb) > strlen("quote reget") + 1) {
    //check if connected 
    if (RemoteName != NULL && strlen(RemoteName) == 1
        && RemoteName[0] == '\\') {
      gRequestProc(gPluginNumber, RT_MsgOK, "reget", "Can not use reget here", NULL, 0);
      return FS_EXEC_ERROR;
    }

    strcpy(buf, Verb);
    if (_strnicmp(buf, "quote reget ", 12) == 0) {
      char remote_users_current_dir[MAX_CMD_BUFFER];
      int sftp_ret;

      if (RemoteName == NULL || strlen(RemoteName) <= 0
          || strchr(RemoteName, '?')) {
        dbg_v("Invalid RemoteName('%s') FIX ME PLZ", RemoteName);
        return FS_EXEC_ERROR;
      }

      check_concurrent_connection(RemoteName);

      char *local_name = buf + strlen("quote reget") + 1;

      strcpy(remote_users_current_dir, local_fs_path_to_remote_path(RemoteName));

      convert_slash_windows_to_unix(remote_users_current_dir);

      _snprintf(sftp_cmd, MAX_CMD_BUFFER, "reget \"%s\" \"%s\"", remote_users_current_dir, local_name);

      gProgressProc(gPluginNumber, RemoteName, local_name, 0);

      LogProc_(MSGTYPE_DETAILS, sftp_cmd);
      DisableLoggingOnce();

      sftp_ret = wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId);
      //Done
      gProgressProc(gPluginNumber, RemoteName, local_name, 100);

      //Any errors?
      if (sftp_ret == SFTP_SUCCESS) {
        return FS_EXEC_OK;
      }
      return FS_EXEC_ERROR;

    }
  }

  if (_strnicmp(Verb, "quote cd ", 9) == 0) {
    strcpy(buf, Verb);
    check_concurrent_connection(RemoteName);

    //cd command: Change dir to what user wanted!
    // two cases: absolute path with / at start, relative with backslash

    if (buf[9] == '/' || buf[9] == '\\') {
      _snprintf(RemoteName, MAX_PATH - 1, "\\%s%s", gSftpConfig.ServerInfos[gCurrentServerId].title, buf + 9);
    } else {

      if (RemoteName[strlen(RemoteName) - 1] != '\\')
        _snprintf(buf, MAX_PATH - 1, "%s\\%s", RemoteName, Verb + 9);
      else
        _snprintf(buf, MAX_PATH - 1, "%s%s", RemoteName, Verb + 9);
      strlcpy(RemoteName, buf, MAX_PATH - 1);

    }

    for (unsigned int i = 0; i < strlen(RemoteName); i++)
      if (RemoteName[i] == '/')
        RemoteName[i] = '\\';

    formcorrectpath(RemoteName);

    return FS_EXEC_SYMLINK;
  }

  //we could implement a property dialog here; changing some connections setting could be usefull
  if (_strnicmp(Verb, "properties", 10)==0) {
    if (_strnicmp(RemoteName+1, gSftpConfig.ServerInfos[gCurrentServerId].title, 
      strlen(gSftpConfig.ServerInfos[gCurrentServerId].title))==0) {
      if (ghDialogDLL) {
        gSftpConfig.SelectedSession = gCurrentServerId;
        if (ghProperties(2, &gSftpConfig)) {
          return FS_EXEC_OK;
        }
      }
      return FS_EXEC_OK;
    }
  }

  //CHMOD
  if ((_strnicmp(lVerb, "quote chmod ", 12)==0) || (_strnicmp(lVerb, "chmod ", 6)==0))
  {
    if (lVerb[0]=='q')
      lVerb += 6;
    lVerb += 6;
    check_concurrent_connection(lRemoteName);
    lRemoteName += (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));

    strcpy(sftp_cmd, "chmod ");
    strcat(sftp_cmd, lVerb);
    if (Verb[0]!='q')
    {
      strcat(sftp_cmd, " \"");
      strcat(sftp_cmd, lRemoteName);
      strcat(sftp_cmd, "\"");
    }

    convert_slash_windows_to_unix(sftp_cmd);

    if (wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId) ==
        SFTP_SUCCESS) {
      return FS_EXEC_OK;
    } else
      return FS_EXEC_ERROR;
  }

  //CHOWN
  if (_strnicmp(Verb, "quote chown ", 12)==0) {
    strcpy(sftp_cmd, &Verb[6]);

    convert_slash_windows_to_unix(sftp_cmd);

    if (wcplg_sftp_do_commando_by_id(sftp_cmd, NULL, gCurrentServerId) ==
        SFTP_SUCCESS) {
      return FS_EXEC_OK;
    } else
      return FS_EXEC_ERROR;
  }

  //MODE: Setting the transfer mode
  if (_strnicmp(Verb, "MODE ", 5)==0) {
    if (strlen(Verb)>=6) {
      //the first character means the mode itself - A for ASCII, I for image=binary; 
      //X is an extension to standard which means, the real transfer mode depends on 
      //transfered file's extension.
      switch (Verb[5])
      {
        case 'A':
        case 'X':
          SetTransferMode(gCurrentServerId, Verb+5);
          break;
        default:
          SetTransferMode(gCurrentServerId, "I\0");
      }
    }
    return FS_EXEC_OK;
  }

  _snprintf(log_sftp_cmd, MAX_CMD_BUFFER, "Command Received: ['%s' - '%s']", Verb, RemoteName);
  LogProc_(MSGTYPE_DETAILS, log_sftp_cmd);

  return FS_EXEC_ERROR;
}
//---------------------------------------------------------------------

int __stdcall FsRenMovFile(char *OldName, char *NewName, BOOL Move,
                           BOOL OverWrite, RemoteInfoStruct * ri)
{
  int retv;
  char cmd_buf[MAX_CMD_BUFFER];

  int server_2_server_move = 0;
  int serverid_from_oldname;
  int serverid_from_newname;

  ri=ri;

  if (strchr(OldName + 1, '\\') == NULL)
    return 0;

  serverid_from_oldname = get_sftp_server_id_by_path(OldName);
  serverid_from_newname = get_sftp_server_id_by_path(NewName);
  if (serverid_from_oldname == NO_SERVER_ID || serverid_from_newname == NO_SERVER_ID) {
    dbg("FsRenMovFile(): get_sftpServer_ID_by_Path == -1 FIX me plz!!!");
    return FS_FILE_READERROR;
  }

  if (Move && serverid_from_oldname != serverid_from_newname) {
    server_2_server_move = 1;
  }

  if (!Move || server_2_server_move == 1) {
    // mhhhh Server2Server copy... okay :->
    char tmp_dir_file[MAX_PATH];
    int cp_flags;

    RemoteInfoStruct *BlindStrukt = NULL;
    get_tmp_file_name(tmp_dir_file);

    FsGetFile(OldName, tmp_dir_file, FS_COPYFLAGS_OVERWRITE, BlindStrukt);
    // if file exsits ?

    if (OverWrite) {
      cp_flags = FS_COPYFLAGS_OVERWRITE;
    } else {
      cp_flags = 0;
    }

    retv = FsPutFile(tmp_dir_file, NewName, cp_flags);
    _unlink(tmp_dir_file);

    if (retv == FS_FILE_OK && server_2_server_move == 1) {
      FsDeleteFile(OldName);
    }

    return retv;
  } else {                      // its a rename/move on the same server
    int err = gProgressProc(gPluginNumber, OldName, NewName, 0);
    check_concurrent_connection(OldName);

    if (!OverWrite) {
      int R_fexists = file_exists_on_remote_server(NewName);
      if (R_fexists == FS_FILE_EXISTS) {
        return FS_FILE_EXISTS;
      }
    }
    // Rename!
    OldName += (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
    NewName += (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));

    strcpy(cmd_buf, "rename \"");
    strcat(cmd_buf, OldName);
    strcat(cmd_buf, "\" \"");
    strcat(cmd_buf, NewName);
    strcat(cmd_buf, "\"");

    OldName -= (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
    NewName -= (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));

    convert_slash_windows_to_unix(cmd_buf);

    err = gProgressProc(gPluginNumber, OldName, NewName, 50);
    if (wcplg_sftp_do_commando_by_id(cmd_buf, NULL, gCurrentServerId) ==
        SFTP_SUCCESS) {
      err = gProgressProc(gPluginNumber, OldName, NewName, 100);
      return FS_FILE_OK;
    }

    err = gProgressProc(gPluginNumber, OldName, NewName, 100);

    return FS_FILE_WRITEERROR;

  }
  return FS_FILE_WRITEERROR;
}
//---------------------------------------------------------------------

int __stdcall FsGetFile(char *RemoteName, char *LocalName, int CopyFlags,
                        RemoteInfoStruct * ri)
{
  char cmd_buf[MAX_CMD_BUFFER];
  int err;
  bool ok = true;

  if ((CopyFlags & FS_COPYFLAGS_OVERWRITE) == 0 && CopyFlags != FS_COPYFLAGS_RESUME)  //want to get the file
  {
    //local
    FILE *fp;
    fp = fopen(LocalName, "r");
    if (fp != NULL) {
      //file exists
      fclose(fp);
      //reget ?
      return FS_FILE_EXISTSRESUMEALLOWED; //FS_FILE_EXISTS;
    }
  }

  if (CopyFlags & FS_COPYFLAGS_RESUME) {
    //reget
    int ret = FS_EXEC_ERROR;
    char wd_saved[MAX_CMD_BUFFER];
    char wd_curr[MAX_CMD_BUFFER];

    strcpy(cmd_buf, "quote reget ");
    strcat(cmd_buf, LocalName);

    if (getcwd(wd_saved, MAX_PATH) == NULL) {
      return FS_FILE_WRITEERROR;
    }
    get_basename_from_path(wd_curr, LocalName);
    if (chdir(wd_curr) != 0) {
      return FS_FILE_WRITEERROR;
    }

    ret = FsExecuteFile((HWND) NULL, RemoteName, cmd_buf);
    chdir(wd_saved);

    if (ret == FS_EXEC_OK) {
      return FS_FILE_OK;
    }
    return FS_FILE_WRITEERROR;
  }

  check_concurrent_connection(RemoteName);

  strcpy(cmd_buf, "get \"");

  RemoteName += (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
  strcat(cmd_buf, RemoteName);
  RemoteName -= (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
  convert_slash_windows_to_unix(cmd_buf);
  strcat(cmd_buf, "\"");

  strcat(cmd_buf, " \"");
  strcat(cmd_buf, LocalName);
  strcat(cmd_buf, "\"");

  err = gProgressProc(gPluginNumber, RemoteName, LocalName, 0);

  if (err)
    return FS_FILE_USERABORT;

  ok =
    (wcplg_sftp_do_commando_by_id(cmd_buf, NULL, gCurrentServerId) ==
     SFTP_SUCCESS);

  if (ok) {
    if (ri != NULL) {
      //is RemoteInfo  set?
      set_local_file_time(LocalName, &ri->LastWriteTime);
    }

    if (CopyFlags & FS_COPYFLAGS_MOVE) {
      cmd_buf[1] = 'r';
      cmd_buf[2] = 'm';         // send rm command to move!
      ok =
        (wcplg_sftp_do_commando_by_id(cmd_buf + 1, NULL, gCurrentServerId)
         == SFTP_SUCCESS);
    }

    err = gProgressProc(gPluginNumber, RemoteName, LocalName, 100);
    if (err)
      return FS_FILE_USERABORT;

    if (ok)
      return FS_FILE_OK;
  }
  return FS_FILE_READERROR;
}
//---------------------------------------------------------------------

int __stdcall FsPutFile(char *LocalName, char *RemoteName, int CopyFlags)
{
  char cmd_buf[MAX_CMD_BUFFER];
  char buf2[MAX_CMD_BUFFER];

  int err = 0;
  int ok = true;

  if ((CopyFlags & FS_COPYFLAGS_OVERWRITE) == 0)
  {
    //ok check Server exists
    if (get_sftp_server_id_by_path(RemoteName) == -1) {
      //local
      //shouldn't happen
      dbg("Unknown Target [error 1]");
      return FS_FILE_WRITEERROR;
    } else {
      if (CopyFlags & FS_COPYFLAGS_EXISTS_SAMECASE) //optimized: use hint from TotalCommander
        return FS_FILE_EXISTSRESUMEALLOWED;
    }
  }

  if (CopyFlags & FS_COPYFLAGS_RESUME) {
    //reput
    int ret = FS_EXEC_ERROR;
    char wd_saved[MAX_CMD_BUFFER];
    char wd_curr[MAX_CMD_BUFFER];

    strcpy(cmd_buf, "quote reput ");
    strcat(cmd_buf, LocalName);

    if (getcwd(wd_saved, MAX_PATH) == NULL) {
      return FS_FILE_WRITEERROR;
    }

    get_basename_from_path(wd_curr, LocalName);

    if (chdir(wd_curr) != 0) {
      return FS_FILE_WRITEERROR;
    }

    ret = FsExecuteFile((HWND) NULL, RemoteName, cmd_buf);
    chdir(wd_saved);

    if (ret == FS_EXEC_OK) {
      return FS_FILE_OK;
    }

    return FS_FILE_WRITEERROR;
  }

  check_concurrent_connection(RemoteName);

  strcpy(cmd_buf, "put \"");
  strcat(cmd_buf, LocalName);
  strcat(cmd_buf, "\" \"");

  char *lRemoteName = RemoteName + (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
  strcpy(buf2, lRemoteName);
  convert_slash_windows_to_unix(buf2);
  strcat(cmd_buf, buf2);
  strcat(cmd_buf, "\"");

  err = gProgressProc(gPluginNumber, LocalName, RemoteName, 0);
  if (err)
    return FS_FILE_USERABORT;

  ok =
    (wcplg_sftp_do_commando_by_id(cmd_buf, NULL, gCurrentServerId) ==
     SFTP_SUCCESS);

  if (ok) {
    if (CopyFlags & FS_COPYFLAGS_MOVE) {
      ok = (0 == _unlink(LocalName)); // delete source after successful upload
    }
    err = gProgressProc(gPluginNumber, RemoteName, LocalName, 100);
    if (err)
      return FS_FILE_USERABORT;
    if (ok)
    {
      if (gSftpConfig.ServerInfos[gCurrentServerId].set_chmod_after_put)
      {
        remote_chmod(RemoteName, gSftpConfig.ServerInfos[gCurrentServerId].chmod_value_put);
      }
      SetMTime(LocalName, RemoteName);
      return FS_FILE_OK;
    }
  }

  return FS_FILE_WRITEERROR;
}

//---------------------------------------------------------------------

BOOL __stdcall FsDeleteFile(char *RemoteName)
{
  char cmd_buf[MAX_CMD_BUFFER];

  // user is trying to delete a connection!
  if (gDeleteOnlyConnection) {
    SetLastError(ERROR_NO_MORE_FILES);
    return false;
  }

  if (RemoteName[0] != '\\')
    return false;

  strcpy(cmd_buf, "rm \"");

  gProgressProc(gPluginNumber, RemoteName, RemoteName, 0);
  check_concurrent_connection(RemoteName);

  RemoteName += (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
  strcat(cmd_buf, RemoteName);
  RemoteName -= (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
  strcat(cmd_buf, "\"");

  convert_slash_windows_to_unix(cmd_buf);

  gProgressProc(gPluginNumber, RemoteName, RemoteName, 50);
  if (wcplg_sftp_do_commando_by_id(cmd_buf, NULL, gCurrentServerId) ==
      SFTP_SUCCESS) {
    gProgressProc(gPluginNumber, RemoteName, RemoteName, 100);
    return true;
  }
  return false;
}

//---------------------------------------------------------------------

BOOL __stdcall FsRemoveDir(char *RemoteName)
{
  // user is trying to delete a connection!
  if (RemoteName != NULL
      && strlen(RemoteName) == strlen(QUICK_CONNECTION) + 1
      && strcmp(1 + RemoteName, QUICK_CONNECTION) == 0) {
    gRequestProc(gPluginNumber, RT_MsgOK, "Delete Connection",
                "Can not delete " QUICK_CONNECTION, NULL, 0);
    return false;
  }

  if (gDeleteOnlyConnection) {
    deletethisconnection(RemoteName + 1);
    return true;
  }

  char cmd_buf[MAX_CMD_BUFFER];

  check_concurrent_connection(RemoteName);

  if (gSftpConfig.CacheFS) {
    std::string lFullPath = RemoteName;
    fxp_names* tmp_dir = DirCache[lFullPath];
    if (tmp_dir) {
      DirCache.erase(lFullPath);
      free_current_dir_struct(tmp_dir, gCurrentServerId);
    }
    std::string::size_type pos = lFullPath.find_last_of('\\');
    if (pos!=std::string::npos) {
      lFullPath = lFullPath.substr(0, pos);
      fxp_names* tmp_dir = DirCache[lFullPath];
      if (tmp_dir) {
        DirCache.erase(lFullPath);
        free_current_dir_struct(tmp_dir, gCurrentServerId);
      }
    }
  }

  strcpy(cmd_buf, "rmdir \"");

  RemoteName += (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));
  strcat(cmd_buf, RemoteName);
  RemoteName -= (1 + strlen(gSftpConfig.ServerInfos[gCurrentServerId].title));

  strcat(cmd_buf, "\"");

  convert_slash_windows_to_unix(cmd_buf);

  if (wcplg_sftp_do_commando_by_id(cmd_buf, NULL, gCurrentServerId) ==
      SFTP_SUCCESS)
    return true;
  return false;
}

//---------------------------------------------------------------------

int wcplg_sftp_do_commando_by_id(char *sftpCmd, char *serverOutput, int id)
{
  if (wcplg_sftp_connect_by_id(id) == SFTP_SUCCESS) {
    return ExecuteCommand(sftpCmd, serverOutput, id);
  }
  return SFTP_FAILED;
}

//---------------------------------------------------------------------

int wcplg_sftp_connect_by_id(int id)
{
  bool server_entered = false;
  bool user_entered = false;
  char buf[MAX_CMD_BUFFER];

  /*
     sprintf(buf,"severid:%d",ID);
     LogProc_(MSGTYPE_DETAILS,buf);
   */

  if (id == NO_SERVER_ID)
    return SFTP_FAILED;         //who knows, what happend before :-(

  SftpServerAccountInfo &r_server_info = gSftpConfig.ServerInfos[id];

  if (strlen(r_server_info.host_cached) < 1) {
    strcpy(r_server_info.host_cached, r_server_info.host);
    if (!gRequestProc(gPluginNumber, RT_Other, "Secure FTP", "Host[:port]",
      r_server_info.host_cached, MAX_CMD_BUFFER)) {
        r_server_info.host_cached[0] = '\0';
      return SFTP_FAILED;
    }

    server_entered = true;
    strcpy(r_server_info.host, r_server_info.host_cached);
  }

  if (strlen(r_server_info.username_cached) < 1) {
    strcpy(buf, "Username for ");
    strcat(buf, r_server_info.host_cached);

    strcpy(r_server_info.username_cached, r_server_info.username);

    // Do not ask 4 username if dont_ask4_username==1
    if (r_server_info.dont_ask4_username != 1) {
      if (!gRequestProc
          (gPluginNumber, RT_UserName, "Secure FTP", buf,
           r_server_info.username_cached, MAX_CMD_BUFFER)) {
        // For quick connection, clear host if failed
        r_server_info.username_cached[0] = 0;
        if (server_entered)
          r_server_info.host_cached[0] = 0;
        return SFTP_FAILED;
      }
    }
    user_entered = true;
    strcpy(r_server_info.username, r_server_info.username_cached);
  }

  if (strlen(r_server_info.password_cached) < 1) {
    strcpy(buf, "Password for ");
    strcat(buf, r_server_info.username_cached);
    strcat(buf, "@");
    strcat(buf, r_server_info.host_cached);

    strcpy(r_server_info.password_cached, r_server_info.password);

    if (r_server_info.use_key_auth != 1) {
      if (!r_server_info.dont_ask4_password) {
        if (!gRequestProc
            (gPluginNumber, RT_Password, "Secure FTP", buf,
             r_server_info.password_cached, MAX_CMD_BUFFER)) {
          // For quick connection, clear host+user if failed
          if (server_entered)
            r_server_info.host_cached[0] = 0;
          if (user_entered)
            r_server_info.username_cached[0] = 0;
          return SFTP_FAILED;
        }
      }
    }

    if (r_server_info.dont_ask4_password)
      strcpy(r_server_info.password_cached, r_server_info.password);
    else
      strcpy(r_server_info.password, r_server_info.password_cached);
  }

  if (Connect(r_server_info.username, 
          r_server_info.password, 
          r_server_info.host, 
          r_server_info.port, 
          gSftpConfig.ServerInfos, id) != SFTP_SUCCESS) {
    r_server_info.password_cached[0] = '\0';
    r_server_info.host_cached[0] = 0;
    r_server_info.username_cached[0] = 0;
    return SFTP_FAILED;
  }
  //OK connect done
  return SFTP_SUCCESS;
}

//---------------------------------------------------------------------

int get_sftp_server_id_by_path(char *path)
{
  char *server_path = path + 1;
  char server_title[MAX_SERVER_INFO];

  if (strchr(server_path, '\\') != NULL) {
    // OK, it's a piece like this:
    // server n\foo\bla\etc
    // have 2 cut the rest after server n
    int pos;
    pos = (int) (strchr(server_path, '\\') - server_path);
    strncpy(server_title, server_path, pos);
    server_title[pos] = '\0';
  } else {
    // OK, it's a piece like this:
    // server n
    // we have the Server Title
    strcpy(server_title, server_path);
  }

  return get_sftp_server_id_by_title(server_title);
}

//---------------------------------------------------------------------

int get_sftp_server_id_by_title(char *title)
{
  int i;
  for (i = 0; i < gSftpConfig.ServerCount; i++) {
    if (strcmp(gSftpConfig.ServerInfos[i].title, title) == 0) {
      return i;
    }
  }
  return NO_SERVER_ID;
}

//---------------------------------------------------------------------

bool any_connection_active()
{
  for (int i = 0; i < gSftpConfig.ServerCount; i++) {
    if (gSftpConfig.ServerInfos[i].username_cached[0])
      return true;
  }
  return false;
}

//---------------------------------------------------------------------

void load_servers()
{
  int max_sections = MAX_SERVER_COUNT;
  int i;
  int ID = 0;
  int imported_num = 0;
  char buf_divers[4000];
  char section_name[MAX_SERVER_INFO];
  char buf_title[MAX_SERVER_INFO];
  char buf_host[MAX_SERVER_INFO];
  char buf_username[MAX_SERVER_INFO];
  char buf_password[4000];
  char buf_port[MAX_SERVER_INFO];
  char buf_home_dir[MAX_SERVER_INFO];
  char buf_compression[MAX_SERVER_INFO];
  char buf_use_key_auth[MAX_SERVER_INFO];
  char buf_dont_ask4_username[MAX_SERVER_INFO];
  char buf_dont_ask4_password[MAX_SERVER_INFO];
  char buf_keyfilename[MAX_SERVER_INFO];
  char buf_dont_ask4_passphrase[MAX_SERVER_INFO];
  char buf_proxy_type[MAX_SERVER_INFO];
  char buf_proxy_host[MAX_SERVER_INFO];
  char buf_proxy_port[MAX_SERVER_INFO];
  char buf_proxy_username[MAX_SERVER_INFO];
  char buf_proxy_password[MAX_SERVER_INFO];
  char buf_proxy_telnet_command[MAX_SERVER_INFO];
  char buf_chmod_value_put[MAX_SERVER_INFO];
  char buf_chmod_value_mkdir[MAX_SERVER_INFO];
  char buf_set_chmod_after_put[MAX_SERVER_INFO];
  char buf_set_chmod_after_mkdir[MAX_SERVER_INFO];
  char buf_set_mtime_after_put[MAX_SERVER_INFO];
  BOOL retry, load, new_passwd=false;
    
  char caption[4000];

  do {
    char buf_test[4000];
    retry = false;
    load = false;
    //Check for password crypter library
    GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_USE_PASSWORD_CRYPTER, "", buf_divers,
                          MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
    GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_TEST_PASSWORD, "", buf_test,
                          4000, gSftpConfig.ConfigIniFile);

    if (strlen(buf_divers)>0) {
      if (stricmp(buf_divers, gSftpConfig.PasswordCrypterPath)!=0) {
        strncpy(gSftpConfig.PasswordCrypterPath, buf_divers, MAX_SERVER_INFO);
        ghPasswdCrypterDLL = LoadLibrary(buf_divers);
        if (ghPasswdCrypterDLL) {
          new_passwd=false;
          ghSetupPasswordCrypter = (tSetupPasswordCrypter)
            GetProcAddress(ghPasswdCrypterDLL, "SetupPasswordCrypter");
          gSftpConfig.EncryptPassword = (tEncryptPassword)
            GetProcAddress(ghPasswdCrypterDLL, "EncryptPassword");
          gSftpConfig.DecryptPassword = (tDecryptPassword)
            GetProcAddress(ghPasswdCrypterDLL, "DecryptPassword");
          if ((ghSetupPasswordCrypter) && (gSftpConfig.EncryptPassword) && 
            (gSftpConfig.DecryptPassword)) {
            buf_divers[0]=0;
            //what to do, if user cancel this?
            if (strlen(buf_test)>0) 
              strcpy(caption, SimpleCaption);
            else {
              strcpy(caption, CaptionAskFirst);
              new_passwd = true;
            }
            load = gRequestProc(gPluginNumber, RT_Password, caption, 
              NULL, buf_divers, sizeof(buf_divers) - 1);
            if (load) {
              if (new_passwd) { 
                load = gRequestProc(gPluginNumber, RT_Password, strcpy(caption, CaptionAskSecond), 
                   NULL, buf_test, sizeof(buf_test) - 1);
                if (strcmp(buf_divers,buf_test)==0) {
                  strcpy(gSftpConfig.PasswordCrypterPassword, buf_divers);
                  ghSetupPasswordCrypter(buf_divers);
                } else {
                  retry = gRequestProc(gPluginNumber, RT_MsgYesNo, 
                    "Passwordcrypter's master-password verification failed!", "Retry?", NULL, 0);
                  load = false;
                  gSftpConfig.PasswordCrypterPath[0] = 0;
                }
              }
            } else {
              gSftpConfig.PasswordCrypterPath[0] = 0;
              gSftpConfig.PasswordCrypterPassword[0] = 0;
            }
          } else {
            gSftpConfig.PasswordCrypterPath[0] = 0;
            load = false;
          }

          if (load && (!new_passwd)) {
            char buf_test2[4000];

            memset(buf_test, 0, sizeof(buf_test));

            strncpy(gSftpConfig.PasswordCrypterPassword, buf_divers, MAX_SERVER_INFO);
            ghSetupPasswordCrypter(gSftpConfig.PasswordCrypterPassword);

            if (strlen(buf_test)>0) {
              memset(buf_test2, 0, sizeof(buf_test2));

              gSftpConfig.DecryptPassword(buf_test, buf_test2);
              load = strcmp(buf_test2, INI_CONFIG_TEST_PASSWORD_TEST)==0;
              if (!load) {
                retry = gRequestProc(gPluginNumber, RT_MsgYesNo, 
                  "Passwordcrypter's master-password verification failed!", "Retry?", NULL, 0);
                gSftpConfig.PasswordCrypterPath[0] = 0;
                gSftpConfig.PasswordCrypterPassword[0] = 0;
              }
            }
          }
          
          if (!load) {
            FreeLibrary(ghPasswdCrypterDLL);
            ghPasswdCrypterDLL=0;
            gSftpConfig.EncryptPassword = 0;
            gSftpConfig.DecryptPassword = 0;
          }
        }
      } else {
        if (!ghPasswdCrypterDLL)
          err_v("Passwordcrypter module failed to load!\n(%s)", gSftpConfig.PasswordCrypterPath);
      }

      if (!ghPasswdCrypterDLL) {
        //gSftpConfig.PasswordCrypterPath[0] = 0;
        //give a message
      }
    }
  } while (retry);

  for (i = 0; i < max_sections; i++) {
    buf_title[0] = '\0';
    buf_host[0] = '\0';
    buf_username[0] = '\0';
    buf_password[0] = '\0';
    buf_port[0] = '\0';
    buf_home_dir[0] = '\0';
    buf_compression[0] = '\0';
    buf_keyfilename[0] = '\0';
    buf_dont_ask4_passphrase[0] = '\0';
    buf_dont_ask4_password[0] = '\0';
    buf_use_key_auth[0] = '\0';
    buf_dont_ask4_username[0] = '\0';
    buf_proxy_type[0] = '\0';
    buf_proxy_host[0] = '\0';
    buf_proxy_port[0] = '\0';
    buf_proxy_username[0] = '\0';
    buf_proxy_password[0] = '\0';
    buf_proxy_telnet_command[0] = '\0';
    buf_chmod_value_put[0] = '\0';
    buf_chmod_value_mkdir[0] = '\0';
    buf_set_chmod_after_put[0] = '\0';
    buf_set_chmod_after_mkdir[0] = '\0';
    buf_set_mtime_after_put[0] = '\0';

    sprintf(section_name, "%i", i);

    GetPrivateProfileString(section_name, "title", "", buf_title,
                            MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
    GetPrivateProfileString(section_name, "host", "", buf_host,
                            MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);

    if (strlen(buf_host) && strlen(buf_title)) {
      GetPrivateProfileString(section_name, "username", "", buf_username,
                              MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "password", "", buf_password,
                              MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "port", "", buf_port,
                              MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "home_dir", "", buf_home_dir,
                              MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "compression", "",
                              buf_compression, MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "use_key_auth", "",
                              buf_use_key_auth, MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "keyfilename", "",
                              buf_keyfilename, MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "dont_ask4_passphrase", "",
                              buf_dont_ask4_passphrase, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "dont_ask4_password", "",
                              buf_dont_ask4_password, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "dont_ask4_username", "",
                              buf_dont_ask4_username, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_type", "",
                              buf_proxy_type, MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_host", "",
                              buf_proxy_host, MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_port", "",
                              buf_proxy_port, MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_username", "",
                              buf_proxy_username, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_password", "",
                              buf_proxy_password, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_telnet_command", "",
                              buf_proxy_telnet_command, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "chmod_value_put", "",
                              buf_chmod_value_put, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "chmod_value_mkdir", "",
                              buf_chmod_value_mkdir, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "set_chmod_after_put", "",
                              buf_set_chmod_after_put, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "set_chmod_after_mkdir", "",
                              buf_set_chmod_after_mkdir, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "set_mtime_after_put", "",
                              buf_set_mtime_after_put, MAX_SERVER_INFO,
                              gSftpConfig.ConfigIniFile);
      // Check title for / && 
      force_valid_server_title(buf_title);

      // TODO check if title already exists, in such case we change it a little, e.g. append '[1]'

      gSftpConfig.ServerInfos[ID].show_hidden_files='1';
      strcpy(gSftpConfig.ServerInfos[ID].title, buf_title);
      strcpy(gSftpConfig.ServerInfos[ID].host, buf_host);
      strcpy(gSftpConfig.ServerInfos[ID].host_cached, buf_host);
      strcpy(gSftpConfig.ServerInfos[ID].username, buf_username);
      if ((ghPasswdCrypterDLL) && (!new_passwd) && (strlen(buf_password)>0)) {
        gSftpConfig.DecryptPassword(buf_password, gSftpConfig.ServerInfos[ID].password);
      } else {
        strncpy(gSftpConfig.ServerInfos[ID].password, buf_password, 
          sizeof(gSftpConfig.ServerInfos[ID].password));
      }
      strcpy(gSftpConfig.ServerInfos[ID].keyfilename, buf_keyfilename);

      //use_key_auth optional, default=0
      if (strlen(buf_use_key_auth)) {
        gSftpConfig.ServerInfos[ID].use_key_auth = 
          (unsigned char)strtoul(buf_use_key_auth, NULL, 10);
        if (gSftpConfig.ServerInfos[ID].use_key_auth > 1)
          gSftpConfig.ServerInfos[ID].use_key_auth = 1;
      } else {
        gSftpConfig.ServerInfos[ID].use_key_auth = 0;
      }

      //dont_ask4_username optional, default=0
      if (strlen(buf_dont_ask4_username)) {
        gSftpConfig.ServerInfos[ID].dont_ask4_username = 
          (unsigned char)strtoul(buf_dont_ask4_username, NULL, 10);
        if (gSftpConfig.ServerInfos[ID].dont_ask4_username > 1)
          gSftpConfig.ServerInfos[ID].dont_ask4_username = 1;
      } else {
        gSftpConfig.ServerInfos[ID].dont_ask4_username = 0;
      }

      //dont_ask4_passphrase optional, default=0
      if (strlen(buf_dont_ask4_passphrase)) {
        gSftpConfig.ServerInfos[ID].dont_ask4_passphrase =
          (unsigned char)strtoul(buf_dont_ask4_passphrase, NULL, 10);
        if (gSftpConfig.ServerInfos[ID].dont_ask4_passphrase > 1)
          gSftpConfig.ServerInfos[ID].dont_ask4_passphrase = 1;
      } else {
        gSftpConfig.ServerInfos[ID].dont_ask4_passphrase = 0;
      }

      //dont_ask4_password optional, default=0
      if (strlen(buf_dont_ask4_password)) {
        gSftpConfig.ServerInfos[ID].dont_ask4_password =
          (unsigned char)strtoul(buf_dont_ask4_password, NULL, 10);
        if (gSftpConfig.ServerInfos[ID].dont_ask4_password > 1)
          gSftpConfig.ServerInfos[ID].dont_ask4_password = 1;
      } else {
        gSftpConfig.ServerInfos[ID].dont_ask4_password = 0;
      }

      //set_chmod_after_put optional, default=0
      if (strlen(buf_set_chmod_after_put)) {
        gSftpConfig.ServerInfos[ID].set_chmod_after_put =
          (unsigned char)strtoul(buf_set_chmod_after_put, NULL, 10);
        if (gSftpConfig.ServerInfos[ID].set_chmod_after_put > 1)
          gSftpConfig.ServerInfos[ID].set_chmod_after_put = 1;
      } else {
        gSftpConfig.ServerInfos[ID].set_chmod_after_put = 0;
      }

      //set_chmod_after_mkdir optional, default=0
      if (strlen(buf_set_chmod_after_mkdir)) {
        gSftpConfig.ServerInfos[ID].set_chmod_after_mkdir =
          (unsigned char)strtoul(buf_set_chmod_after_mkdir, NULL, 10);
        if (gSftpConfig.ServerInfos[ID].set_chmod_after_mkdir > 1)
          gSftpConfig.ServerInfos[ID].set_chmod_after_mkdir = 1;
      } else {
        gSftpConfig.ServerInfos[ID].set_chmod_after_mkdir = 0;
      }

      //set_mtime_after_put optional, default=1
      if (strlen(buf_set_mtime_after_put)) {
        gSftpConfig.ServerInfos[ID].set_mtime_after_put =
          (unsigned char)strtoul(buf_set_mtime_after_put, NULL, 10);
        if (gSftpConfig.ServerInfos[ID].set_mtime_after_put > 1)
          gSftpConfig.ServerInfos[ID].set_mtime_after_put = 1;
      } else {
        gSftpConfig.ServerInfos[ID].set_mtime_after_put = 1;
      }

      //chmod_value_put optional, default=700, valid only with set_chmod_after_put
      if (strlen(buf_chmod_value_put)) {
        gSftpConfig.ServerInfos[ID].chmod_value_put =
          strtoul(buf_chmod_value_put, NULL, 10);
      } else {
        gSftpConfig.ServerInfos[ID].chmod_value_put = 700;
      }

      //chmod_value_mkdir optional, default=700, valid only with set_chmod_after_mkdir
      if (strlen(buf_chmod_value_mkdir)) {
        gSftpConfig.ServerInfos[ID].chmod_value_mkdir =
          strtoul(buf_chmod_value_mkdir, NULL, 10);
      } else {
        gSftpConfig.ServerInfos[ID].chmod_value_mkdir = 700;
      }

      // port & home_dir are optional
      if (strlen(buf_port)) {
        gSftpConfig.ServerInfos[ID].port = strtol(buf_port, NULL, 10);
      } else {
        gSftpConfig.ServerInfos[ID].port = 22;
      }

      if (gSftpConfig.ServerInfos[ID].port < 1 || gSftpConfig.ServerInfos[ID].port > 65535) {
        gSftpConfig.ServerInfos[ID].port = 22;
      }

      if (strlen(buf_compression) == 0
          || strcmp(buf_compression, "1") == 0) {
        gSftpConfig.ServerInfos[ID].compression = 1;
      } else {
        gSftpConfig.ServerInfos[ID].compression = 0;
      }

      if (strlen(buf_home_dir)) {
        strcpy(gSftpConfig.ServerInfos[ID].home_dir, buf_home_dir);
      } else {
        strcpy(gSftpConfig.ServerInfos[ID].home_dir, "");
      }

      gSftpConfig.ServerInfos[ID].passphrase[0] = 0;
      gSftpConfig.ServerInfos[ID].password_cached[0] = 0;
      gSftpConfig.ServerInfos[ID].username_cached[0] = 0;
      gSftpConfig.ServerInfos[ID].is_imported_from_any_datasrc = 0;
      gSftpConfig.ServerInfos[ID].is_imported_from_putty_registry = 0;
      gSftpConfig.ServerInfos[ID].is_imported_from_sshcom_registry = 0;

      //experimental: Proxy
      if (strlen(buf_proxy_type)) {
        unsigned long tmp = strtoul(buf_proxy_type, NULL, 10);
        switch (tmp) {
        case 1:
          gSftpConfig.ServerInfos[ID].proxy_type = PROXY_SOCKS4;
          break;
        case 2:
          gSftpConfig.ServerInfos[ID].proxy_type = PROXY_SOCKS5;
          break;
        case 3:
          gSftpConfig.ServerInfos[ID].proxy_type = PROXY_HTTP;
          break;
        case 4:
		  gSftpConfig.ServerInfos[ID].proxy_type = PROXY_TELNET;
          break;
        case 5:
          gSftpConfig.ServerInfos[ID].proxy_type = PROXY_CMD;
          break;
        default:
          gSftpConfig.ServerInfos[ID].proxy_type = PROXY_NONE;
          break;
        }
      } else {
        gSftpConfig.ServerInfos[ID].proxy_type = PROXY_NONE;
      }

      if (strlen(buf_proxy_host)) {
        strcpy(gSftpConfig.ServerInfos[ID].proxy_host, buf_proxy_host);
      } else {
        strcpy(gSftpConfig.ServerInfos[ID].proxy_host, "");
      }

      if (strlen(buf_proxy_port)) {
        gSftpConfig.ServerInfos[ID].proxy_port = strtol(buf_proxy_port, NULL, 10);
      } else {
        gSftpConfig.ServerInfos[ID].proxy_port = 1080;  //standard port for socks-proxy
      }

      if ((gSftpConfig.ServerInfos[ID].proxy_port < 1) || 
          (gSftpConfig.ServerInfos[ID].proxy_port > 65535)) {
        gSftpConfig.ServerInfos[ID].proxy_port = 1080;  //standard port for socks-proxy
      }

      if (strlen(buf_proxy_username)) {
        strcpy(gSftpConfig.ServerInfos[ID].proxy_username, buf_proxy_username);
      } else {
        strcpy(gSftpConfig.ServerInfos[ID].proxy_username, "");
      }

      if (strlen(buf_proxy_password)) {
        strcpy(gSftpConfig.ServerInfos[ID].proxy_password, buf_proxy_password);
      } else {
        strcpy(gSftpConfig.ServerInfos[ID].proxy_password, "");
      }

      if (strlen(buf_proxy_telnet_command)) {
        strcpy(gSftpConfig.ServerInfos[ID].proxy_telnet_command,
               buf_proxy_telnet_command);
      } else {
        strcpy(gSftpConfig.ServerInfos[ID].proxy_telnet_command, "");
      }

      gSftpConfig.ServerInfos[ID].id = ID;
      ID++;
    } else if (i)
      break;                    // hey dude - this is a badness, too :-(
  }

  // okay, we are outta here  

  // Cache directories?
  strcpy(buf_divers, "");
  GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_CACHE_FS, "", buf_divers,
                          MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
  if (strlen(buf_divers) && (buf_divers[0]=='1'))
  {
    gSftpConfig.CacheFS = 1;
  } else {
    gSftpConfig.CacheFS = 0;
  }

  // DO Putty Import ?
  strcpy(buf_divers, "");
  GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_IMPORT_PUTTY_SSH_SESS, "", buf_divers,
                          MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
  if (strlen(buf_divers) && (buf_divers[0] == '1' || buf_divers[0] == '2')) {
    //import the putty saved session if...
    imported_num = 0;
    imported_num += import_putty_sessions(ID, buf_divers);
    ID += imported_num;
  }
  if (strlen(buf_divers)>0)
    gSftpConfig.DoImportPuttySessions = buf_divers[0];
  else
    gSftpConfig.DoImportPuttySessions = '0';

  gSftpConfig.ImportedSessions = imported_num;
  // DO ssh.com Import ?
  strcpy(buf_divers, "");
  GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_IMPORT_SSHCOM_SSH_SESS, "",
                          buf_divers, MAX_SERVER_INFO, gSftpConfig.ConfigIniFile);
  if (strlen(buf_divers)) {
    char dir_location[MAX_CMD_BUFFER];

    strcpy(dir_location, buf_divers);
    //import the ssh.com saved session if...
    if (strlen(dir_location) > 0 && strcmp(dir_location, "0") != 0) {
      imported_num = 0;
      imported_num = import_ssh_com_sessions(ID, dir_location);
      gSftpConfig.ImportedSessions += imported_num;
      ID += imported_num;
    }
  }
  strncpy(gSftpConfig.DoImportSSHcomSessions, buf_divers, MAX_SERVER_INFO);

  strcpy(gSftpConfig.ServerInfos[ID].title, QUICK_CONNECTION);
  //Set defaults 4 quick connection
  gSftpConfig.ServerInfos[ID].compression = 0;
  gSftpConfig.ServerInfos[ID].dont_ask4_username = 0;
  gSftpConfig.ServerInfos[ID].dont_ask4_passphrase = 0;
  gSftpConfig.ServerInfos[ID].dont_ask4_password = 0;
  gSftpConfig.ServerInfos[ID].home_dir[0] = '\0';
  gSftpConfig.ServerInfos[ID].host[0] = '\0';
  gSftpConfig.ServerInfos[ID].host_cached[0] = '\0';
  gSftpConfig.ServerInfos[ID].id = ID;
  //is imported, as we don't want to save it again
  gSftpConfig.ServerInfos[ID].is_imported_from_any_datasrc = 1;
  gSftpConfig.ServerInfos[ID].is_imported_from_putty_registry = 0;
  gSftpConfig.ServerInfos[ID].is_imported_from_sshcom_registry = 0;
  gSftpConfig.ServerInfos[ID].keyfilename[0] = '\0';
  gSftpConfig.ServerInfos[ID].passphrase[0] = '\0';
  gSftpConfig.ServerInfos[ID].proxy_host[0] = '\0';
  gSftpConfig.ServerInfos[ID].proxy_password[0] = '\0';
  gSftpConfig.ServerInfos[ID].proxy_port = 0;
  gSftpConfig.ServerInfos[ID].proxy_telnet_command[0] = '\0';
  gSftpConfig.ServerInfos[ID].proxy_type = PROXY_NONE;
  gSftpConfig.ServerInfos[ID].proxy_username[0] = '\0';
  gSftpConfig.ServerInfos[ID].password[0] = '\0';
  gSftpConfig.ServerInfos[ID].password_cached[0] = '\0';
  gSftpConfig.ServerInfos[ID].port = 22;
  gSftpConfig.ServerInfos[ID].use_key_auth = 0;
  gSftpConfig.ServerInfos[ID].username[0] = '\0';
  gSftpConfig.ServerInfos[ID].username_cached[0] = '\0';
  gSftpConfig.ServerInfos[ID].chmod_value_put = 0;
  gSftpConfig.ServerInfos[ID].chmod_value_mkdir = 0;
  gSftpConfig.ServerInfos[ID].set_chmod_after_put = 0;
  gSftpConfig.ServerInfos[ID].set_chmod_after_mkdir = 0;
  gSftpConfig.ServerInfos[ID].set_mtime_after_put = 0;

  ID++;                         // !!!

  make_server_titles_unique(ID);

  gSftpConfig.ServerCount = ID;

  if (new_passwd) SaveProperties(&gSftpConfig);
}

//---------------------------------------------------------------------

void LogProc_(int MsgType, char *LogString)
{
  if (&gLogProc && DoLogging())
    gLogProc(gPluginNumber, MsgType, LogString);
}

//---------------------------------------------------------------------

BOOL __stdcall FsDisconnect(char *DisconnectRoot)
{
  UNUSED_ARG(DisconnectRoot)
  //DisconnectRoot is unusable as only 1 connection is active, because of psftp's architecture
  //LogProc_(MSGTYPE_DISCONNECT,"DISCONNECTED");
  sftp_disconnect(gCurrentServerId, false);
  ResetAlreadyConnected();
  return true;
}

//---------------------------------------------------------------------

//this checks for a connection. we could use this multi dll concept to 
//run multiple connections simultanously
void check_concurrent_connection(char *path)
{
  //gCurrentServerId is global
  if (gCurrentServerId == NO_SERVER_ID)   //Is there already a connection to a Server ?
  {
    gCurrentServerId = get_sftp_server_id_by_path(path);
  } else {
    int server_id = get_sftp_server_id_by_path(path);
    if (server_id != gCurrentServerId) {
      //ooops, conflict!
      //we have to disconnect the connection to avoid the conflict
      if (IsAlreadyConnected())
        sftp_disconnect(gCurrentServerId, false);
      gCurrentServerId = server_id; // OK, conflict fixed :-)
    }
  }
}

//---------------------------------------------------------------------

int get_current_server_id()
{
  return gCurrentServerId;
}

//---------------------------------------------------------------------

SftpServerAccountInfoType *GetServerInfos(void)
{
  return gSftpConfig.ServerInfos;
}

//---------------------------------------------------------------------

BOOL Request_Password(char *buf)
{
  return gRequestProc(gPluginNumber, RT_Password, "Secure FTP", "Password", buf, MAX_PATH);
}

//---------------------------------------------------------------------

int octal_permissions_2_tc_integral(unsigned long octal_val)
{
  //quick and dirty

  char buf[100];                //should be enough [:-( not nice, dude!]
  char cbuf[100];
  size_t clen;
  int int_val;
  sprintf(buf, "%3o", octal_val);

  clen = strlen(buf);
  cbuf[0] = '0';
  cbuf[1] = buf[clen - 3];
  cbuf[2] = buf[clen - 2];
  cbuf[3] = buf[clen - 1];
  cbuf[4] = '\0';

  int_val = strtol(cbuf, '\0', 0);
  return int_val;
}

//---------------------------------------------------------------------

ProgressProcType GetProgressProc()
{
  return gProgressProc;
}
//---------------------------------------------------------------------

int GetPluginNumber()
{
  return gPluginNumber;
}
//---------------------------------------------------------------------

int file_exists_on_remote_server(char *remoteFile)
{
  if (get_sftp_server_id_by_path(remoteFile) == NO_SERVER_ID)
    return FS_FILE_NOTSUPPORTED;

  char path[MAX_CMD_BUFFER];
  char file_name[MAX_CMD_BUFFER];

  strcpy(path, remoteFile);
  if (get_basename_from_path(path, remoteFile) != 1) {
    dbg("file_exists_on_remote_server: fix me, path!");
    return FS_FILE_NOTSUPPORTED;
  }

  char *remote_file_name = remoteFile + (strlen(path) + 1);
  strcpy(file_name, remote_file_name);

  check_concurrent_connection(remoteFile);
  WIN32_FIND_DATA find_data;
  HANDLE h_find;

  h_find = FsFindFirst(path, &find_data);

  if (h_find == INVALID_HANDLE_VALUE) {
    FsFindClose(h_find);
    return FS_FILE_NOTFOUND;
  }

  LastFindStuctType* find_struct;
  find_struct = (LastFindStuctType*) h_find;
  int found = 0;
  if (find_struct->mCurrentDirStruct != NULL) {
    for (int i = 0; i < find_struct->mCurrentDirStruct[0].nnames; i++) {
      if (strcmp(find_struct->mCurrentDirStruct->names[i].filename, file_name) == 0) {
        found = 1;
        break;
      }
    }
  }

  FsFindClose(h_find);
  if (found == 1)
    return FS_FILE_EXISTS;

  return FS_FILE_NOTFOUND;
}

//---------------------------------------------------------------------

int get_basename_from_path(char *buf, char *path)
{
  size_t i;
  size_t dir_end = 0;

  i = strlen(path) - 1;
  for (; i > 0; i--) {
    if (path[i] == '\\') {
      dir_end = i;
      break;
    }
  }

  if (dir_end == 0)
    return 0;                   // FIX ME MORE!!!

  for (i = 0; i < (dir_end); i++) {
    buf[i] = path[i];
  }
  buf[i] = '\0';

  return 1;
}

//---------------------------------------------------------------------

void __stdcall FsGetDefRootName(char *DefRootName, int maxlen)
{
  strlcpy(DefRootName, FSPLUGIN_CAPTION, maxlen - 1);
}

//---------------------------------------------------------------------

void __stdcall FsStatusInfo(char *RemoteDir, int InfoStartEnd,
                            int InfoOperation)
{
  gAcceptRefresh = false;
  switch (InfoOperation) {
    case FS_STATUS_OP_DELETE:
      {
        if (strcmp(RemoteDir, "\\") == 0) // Deleting connection!
        {
          gDeleteOnlyConnection = (InfoStartEnd == FS_STATUS_START);
        }
      }
    case FS_STATUS_OP_PUT_SINGLE:
    case FS_STATUS_OP_PUT_MULTI:
    case FS_STATUS_OP_RENMOV_SINGLE:
    case FS_STATUS_OP_RENMOV_MULTI:
    case FS_STATUS_OP_ATTRIB:
    case FS_STATUS_OP_MKDIR:
    case FS_STATUS_OP_LIST:
      {
        gAcceptRefresh = true;
      }
  }
}

//---------------------------------------------------------------------

bool IsRemoteNameConnection(char *RemoteName)
{
  int i = 0;
  while (RemoteName[i] != 0 && RemoteName[i]!='\\') i++;
  i++;
  while (RemoteName[i] != 0 && RemoteName[i]!='\\') i++;
  return RemoteName[i] == 0;
}

//---------------------------------------------------------------------

int __stdcall FsExtractCustomIcon(char* RemoteName,int ExtractFlags,HICON* TheIcon)
{
  char *remote_name = RemoteName+1;
  if (strcmp(remote_name, QUICK_CONNECTION) == 0 || strcmp(remote_name, DefineAddConnection_define) == 0
    || strcmp(remote_name, DefineEditConnection_define) == 0 )
    return FS_ICON_USEDEFAULT;
  if (IsRemoteNameConnection(RemoteName))
  {
    *TheIcon = gConnectionIcon;
    return FS_ICON_EXTRACTED;
  }
  return FS_ICON_USEDEFAULT;
}

//---------------------------------------------------------------------

bool set_local_file_time(char *fullFilePath, FILETIME * lastWriteTime)
{
  bool f;
  HANDLE my_file;
  if (lastWriteTime == NULL)
    return false;
  my_file = CreateFile(fullFilePath, // file name
                      GENERIC_WRITE,  // access mode
                      FILE_SHARE_READ,  // share mode
                      NULL,     // security descriptor
                      OPEN_EXISTING,  // how to create
                      FILE_ATTRIBUTE_NORMAL | // file attributes
                      FILE_FLAG_SEQUENTIAL_SCAN, NULL); // handle to template file

  if (my_file == INVALID_HANDLE_VALUE) {
    return false;
  }

  f = SetFileTime(my_file,       // sets last-write time for file
                  lastWriteTime, (LPFILETIME) NULL, lastWriteTime) == TRUE;

  CloseHandle(my_file);
  return f;
}


//---------------------------------------------------------------------

//replace / and \ with _ (in title)
void force_valid_server_title(char *title)
{
  if ((title == NULL) || (strlen(title) < 1))
    return;                     

  for (size_t i = 0; i < strlen(title); i++) {
    if ((title[i] == '/') || (title[i] == '\\')) {
      title[i] = '_';
    }
  }

}
//---------------------------------------------------------------------

//what the hell does this one?
static void mungestr(char *in, char *out)
{
  int candot = 0;

  while (*in) {
    if (*in == ' ' || *in == '\\' || *in == '*' || *in == '?' ||
        *in == '%' || *in < ' ' || *in > '~' || (*in == '.' && !candot)) {
      *out++ = '%';
      *out++ = hex[((unsigned char) *in) >> 4];
      *out++ = hex[((unsigned char) *in) & 15];
    } else
      *out++ = *in;
    in++;
    candot = 1;
  }
  *out = '\0';
  return;
}

//---------------------------------------------------------------------

void *open_settings_r(char *sessionname)
{
  HKEY subkey1, sesskey;
  char *p;

  p = (char *) malloc(3 * strlen(sessionname) + 1);
  mungestr(sessionname, p);

  if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &subkey1) != ERROR_SUCCESS) {
    sesskey = NULL;
  } else {
    if (RegOpenKey(subkey1, p, &sesskey) != ERROR_SUCCESS) {
      sesskey = NULL;
    }
  }
  RegCloseKey(subkey1);
  free(p);

  return (void *) sesskey;
}

//---------------------------------------------------------------------

char *read_setting_s(void *handle, char *key, char *buffer, int buflen)
{
  DWORD type, size;
  size = buflen;

  if (!handle || RegQueryValueEx((HKEY) handle, key, 0, &type,
                                 (unsigned char *) buffer,
                                 &size) != ERROR_SUCCESS || type != REG_SZ)
    return NULL;
  else
    return buffer;
}
//---------------------------------------------------------------------

int read_setting_i(void *handle, char *key, int defvalue)
{
  DWORD type, val, size;
  size = sizeof(val);

  if (!handle || RegQueryValueEx((HKEY) handle, key, 0, &type,
                                 (BYTE *) & val, &size) != ERROR_SUCCESS
      || size != sizeof(val) || type != REG_DWORD)
    return defvalue;
  else
    return val;
}
//---------------------------------------------------------------------

static void unmungestr(char *in, char *out, int outlen)
{
  while (*in) {
    if (*in == '%' && in[1] && in[2]) {
      int i, j;

      i = in[1] - '0';
      i -= (i > 9 ? 7 : 0);
      j = in[2] - '0';
      j -= (j > 9 ? 7 : 0);

      *out++ = (i << 4) + j;
      if (!--outlen)
        return;
      in += 3;
    } else {
      *out++ = *in++;
      if (!--outlen)
        return;
    }
  }
  *out = '\0';
  return;
}
//---------------------------------------------------------------------

static void gpps(void *handle, char *name, char *def, char *val, int len)
{
  if (!read_setting_s(handle, name, val, len)) {
    strncpy(val, def, len);
    val[len - 1] = '\0';
  }
}
//---------------------------------------------------------------------

static void gppi(void *handle, char *name, int def, int *i)
{
  *i = read_setting_i(handle, name, def);
}
//---------------------------------------------------------------------

void *enum_settings_start(void)
{
  struct EnumSettingsType *ret;
  HKEY key;

  if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &key) != ERROR_SUCCESS) {
    return NULL;
  }

  ret = (struct EnumSettingsType *) malloc(sizeof(*ret));

  if (ret) {
    ret->mKey = key;
    ret->i = 0;
  }
  return ret;
}
//---------------------------------------------------------------------

char *enum_settings_next(void *handle, char *buffer, int buflen)
{
  struct EnumSettingsType *e = (struct EnumSettingsType *) handle;
  char *otherbuf;

  otherbuf = (char *) malloc(3 * buflen);
  if (otherbuf
      && RegEnumKey(e->mKey, e->i++, otherbuf,
                    3 * buflen) == ERROR_SUCCESS) {
    unmungestr(otherbuf, buffer, buflen);
    free(otherbuf);
    return buffer;
  } else {
    free(otherbuf);
    return NULL;
  }
}
//---------------------------------------------------------------------

// Session importing
int import_putty_sessions(int lastInsertId, char *importMode)
{
  static char otherbuf[2048];
  int count = 0;
  char *ret;
  void *handle;
  struct EnumSettingsType *handleFree;

  if ((handle = enum_settings_start())) {
    handleFree = (struct EnumSettingsType *) handle;
    do {
      if ((lastInsertId + count) >= MAX_SERVER_COUNT - 2) {
        RegCloseKey(handleFree->mKey);
        return count;
      }
      ret = enum_settings_next(handle, otherbuf, sizeof(otherbuf));

      if (ret) {
        SftpServerAccountInfoType ServerAccountInfo;
        if (import_one_putty_session(&ServerAccountInfo, otherbuf) == 1) {
          // OK success
          force_valid_server_title(otherbuf);
          strcpy(ServerAccountInfo.title, otherbuf);
          ServerAccountInfo.id = (lastInsertId + count);
          gSftpConfig.ServerInfos[lastInsertId + count] = ServerAccountInfo;
          if (importMode[0] == '2') {
            //User want no password promt, all the connection are using key auth trought pageant.exe
            gSftpConfig.ServerInfos[lastInsertId + count].use_key_auth = 1;
          }
          count++;
        }
      }
    } while (ret);
    RegCloseKey(handleFree->mKey);
    free(handle);
  }
  return count;
}

//---------------------------------------------------------------------

void ServerAccountInfoDefaults(SftpServerAccountInfoType *ServerAccountInfo)
{
  ServerAccountInfo->id = 0;
  ServerAccountInfo->title[0] = 0;
  ServerAccountInfo->username[0] = 0;
  ServerAccountInfo->username_cached[0] = 0;
  ServerAccountInfo->password[0] = 0;
  ServerAccountInfo->passphrase[0] = 0;
  ServerAccountInfo->password_cached[0] = 0;
  ServerAccountInfo->host[0] = 0;
  ServerAccountInfo->host_cached[0] = 0;
  ServerAccountInfo->home_dir[0] = 0;
  ServerAccountInfo->keyfilename[0] = 0;
  ServerAccountInfo->port = 22;
  ServerAccountInfo->compression = 0;
  ServerAccountInfo->use_key_auth = 0;
  ServerAccountInfo->dont_ask4_username = 0;
  ServerAccountInfo->dont_ask4_password = 0;
  ServerAccountInfo->dont_ask4_passphrase = 0;
  ServerAccountInfo->is_imported_from_any_datasrc = 0;  //private
  ServerAccountInfo->is_imported_from_putty_registry = 0; //private
  ServerAccountInfo->is_imported_from_sshcom_registry = 0;  //private
  //Experimental - just a copy from Putty.h:Config struct
  ServerAccountInfo->proxy_type = PROXY_NONE;
  ServerAccountInfo->proxy_host[512] = 0;
  ServerAccountInfo->proxy_port = 0;
  ServerAccountInfo->proxy_username[0] = 0;
  ServerAccountInfo->proxy_password[0] = 0;
  ServerAccountInfo->proxy_telnet_command[0] = 0;
  //Experimental - chmod
  ServerAccountInfo->chmod_value_put = 700;
  ServerAccountInfo->chmod_value_mkdir = 700;
  ServerAccountInfo->set_chmod_after_put = 0;
  ServerAccountInfo->set_chmod_after_mkdir = 0;
  ServerAccountInfo->set_mtime_after_put = 1;
}
//---------------------------------------------------------------------

int import_one_putty_session(SftpServerAccountInfoType *ServerAccountInfo, char *PuttySectionName)
{
  char HostName[1000];
  char Protocol[1000];
  char UserName[1000];
  char PublicKeyFile[4096];
  int PortNumber;
  int Compression;
  HKEY sesskey;

  HostName[0] = '\0';
  Protocol[0] = '\0';
  UserName[0] = '\0';
  PortNumber = 22;

  sesskey = (HKEY) open_settings_r(PuttySectionName);
  if (sesskey != NULL) {
    gpps(sesskey, "HostName", "", HostName, sizeof(HostName));
    gpps(sesskey, "Protocol", "", Protocol, sizeof(Protocol));
    gpps(sesskey, "UserName", "", UserName, sizeof(UserName));
    gppi(sesskey, "PortNumber", 22, &PortNumber);
    gppi(sesskey, "Compression", 0, &Compression);
    gpps(sesskey, "PublicKeyFile", "", PublicKeyFile,
         sizeof(PublicKeyFile));

    if (strlen(HostName) > 0 && strcmp(Protocol, "ssh") == 0) {
      ServerAccountInfoDefaults(ServerAccountInfo);
      if (!PortNumber || PortNumber < 1) {
        PortNumber = 22;
      }

      if (!Compression || Compression == 0) {
        Compression = 0;
      } else {
        Compression = 1;
      }

      //inherited char*
      strcpy(ServerAccountInfo->host, HostName);
      strcpy(ServerAccountInfo->host_cached, HostName);
      strcpy(ServerAccountInfo->username, UserName);
      strcpy(ServerAccountInfo->username_cached, ""); // MUSS strlen=0 sein  wegen anyconnectionactive()

      // inherited int
      ServerAccountInfo->compression = Compression;
      ServerAccountInfo->port = PortNumber;

      // Static
      ServerAccountInfo->use_key_auth = strlen(PublicKeyFile) > 0;
      ServerAccountInfo->dont_ask4_passphrase = 0;
      strncpy(ServerAccountInfo->keyfilename, PublicKeyFile,
              MAX_SERVER_INFO);
      if (strlen(ServerAccountInfo->username)) {
        ServerAccountInfo->dont_ask4_username = 1;
      } else {
        ServerAccountInfo->dont_ask4_username = 0;
      }

      ServerAccountInfo->is_imported_from_any_datasrc = 1;
      ServerAccountInfo->is_imported_from_putty_registry = 1;

      RegCloseKey(sesskey);
      return 1;
    }
  }
  RegCloseKey(sesskey);
  return 0;
}

//---------------------------------------------------------------------

int import_ssh_com_sessions(int lastInsertId, char *dirLocation)
{
  //int dwIndex=1;
  int count = 0;
  unsigned int file_num = 0;
  char buf[MAX_SERVER_INFO];
  char file_name[MAX_SERVER_INFO];
  char ini_location[MAX_SERVER_INFO];
  char buf_divers[MAX_SERVER_INFO];
  char dir_location_path[MAX_SERVER_INFO];

  bool next;                    // Done searching for files?
  HANDLE h_find = NULL;         // Handle to find data.
  WIN32_FIND_DATA find_data;    // Info on file found.

  if (strlen(dirLocation) > 0
      && dirLocation[strlen(dirLocation) - 1] == '\\') {
    dirLocation[strlen(dirLocation) - 1] = '\0';
  }


  strcpy(dir_location_path, dirLocation);
  strcat(dirLocation, "\\*.ssh2");

  h_find = FindFirstFile(dirLocation, &find_data);

  while (h_find != INVALID_HANDLE_VALUE) {
    next = FindNextFile(h_find, &find_data) == TRUE;

    if (next) {
      char hostname[MAX_SERVER_INFO];
      char title[MAX_SERVER_INFO];
      char username[MAX_SERVER_INFO];
      char home_dir[MAX_SERVER_INFO];
      int compression = 0;
      int port = 22;
      hostname[0] = '\0';
      title[0] = '\0';
      username[0] = '\0';
      home_dir[0] = '\0';

      file_num++;
      sprintf(buf, "File%d", file_num);
      file_name[0] = '\0';

      int new_insert_id = lastInsertId + count;

      if ((new_insert_id) >= MAX_SERVER_COUNT - 2) {
        if (h_find != INVALID_HANDLE_VALUE) // if there was anything found, then
          FindClose(h_find);                // close the find handle
        return count;
        break;
      }

      strcpy(file_name, find_data.cFileName);
      strcpy(ini_location, dir_location_path);
      strcat(ini_location, "\\");
      strcat(ini_location, file_name);

      if (strlen(file_name)) {
        GetPrivateProfileString("Connection", "Host Name", "", buf_divers,
                                MAX_SERVER_INFO, ini_location);
        if (strlen(buf_divers) > 2) {
          //title
          get_basename_from_path(title, file_name);
          strcpy(title, (file_name + (strlen(title))));
          force_valid_server_title(title);

          //hostname
          strcpy(hostname, 2 + buf_divers);

          //username
          GetPrivateProfileString("Connection", "User Name", "",
                                  buf_divers, MAX_SERVER_INFO, ini_location);
          if (strlen(buf_divers) > 2)
            strcpy(username, 2 + buf_divers);

          //Compression
          GetPrivateProfileString("Connection", "Compression", "0",
                                  buf_divers, MAX_SERVER_INFO, ini_location);
          if (strlen(buf_divers) > 2
              && strcmp(buf_divers + 2, "zlib") == 0) {
            compression = 1;
          } else {
            compression = 0;
          }

          //Port
          GetPrivateProfileString("Connection", "Port", "22", buf_divers,
                                  MAX_SERVER_INFO, ini_location);
          if (strlen(buf_divers) > 2) {
            port = atoi(2 + buf_divers);
          } else {
            port = 22;
          }
          if (!port || port <= 0)
            port = 22;

          //home dir :-)
          GetPrivateProfileString("Connection", "Default SFTP Directory",
                                  "", buf_divers, MAX_SERVER_INFO, ini_location);
          if (strlen(buf_divers) > 2) {
            strcpy(home_dir, 2 + buf_divers);
          }
          SftpServerAccountInfo &r_new_server_info = gSftpConfig.ServerInfos[new_insert_id];
          //inherited char*
          strcpy(r_new_server_info.title, title);
          strcpy(r_new_server_info.host, hostname);
          strcpy(r_new_server_info.host_cached, hostname);
          strcpy(r_new_server_info.username, username);
          strcpy(r_new_server_info.username_cached, ""); // must be "strlen=0" because anyconnectionactive()
          strcpy(r_new_server_info.home_dir, home_dir);

          // inherited int
          r_new_server_info.compression = compression;
          r_new_server_info.port = port;

          // Static
          r_new_server_info.use_key_auth = 0;

          if (strlen(r_new_server_info.username)) {
            r_new_server_info.dont_ask4_username = 1;
          } else {
            r_new_server_info.dont_ask4_username = 0;
          }

          strcpy(r_new_server_info.password, "");
          strcpy(r_new_server_info.password_cached, "");

          r_new_server_info.is_imported_from_any_datasrc = 1;
          r_new_server_info.is_imported_from_putty_registry = 0;
          r_new_server_info.is_imported_from_sshcom_registry = 1;

          r_new_server_info.id = new_insert_id;

          count++;              //last comand!!!
        }
      }
    } else {
      break;
    }
  }

  if (h_find != INVALID_HANDLE_VALUE) // if there was anything found, then
    FindClose(h_find);                // close the find handle

  return count;
}
