#pragma warning(disable:4786)

#include "stdafx.h"
#include "fsplugin.h"
#include "sftpmap.h"
#include <direct.h>
#include <stdio.h>
#include "putty_proxy.h"
#include "properties_dlg.h"
#include "ServerInfo.h"
#include "share.h"
#include "passwd_crypter.h"
#include "ConfigProperties.h"

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
static char hex[17] = "0123456789ABCDEF";  //17 because of terminating 0-character

#define S_IFLNK 0x0A000
#define DefaultIniFileName "wcx_sftp.ini"
#define NO_SERVER_ID -1

//Links in the first level of plugin's fs
//
#define DefineEditConnection_define "Edit connections.lnk"
#define DefineEditConnection_selected "\\Edit connections.lnk"

#define DefineAddConnection_define "Add connections.lnk"
#define DefineAddConnection_selected "\\Add connections.lnk"

typedef struct {
  char Path[MAX_PATH];
  char LastFoundName[MAX_PATH];
  HANDLE searchhandle;
  int sumIndex;
  int currentIndex;
  int SearchMode;
  fxp_names *CurrentDirStruct;
} tLastFindStuct, *pLastFindStuct;

struct enumsettings {
  HKEY key;
  int i;
};

typedef std::map<std::string, fxp_names*> dir_cache;
typedef dir_cache::iterator dir_cache_iterator;

dir_cache DirCache;
char LastPath[MAX_PATH];

//struct SftpServerAccountInfo SftpConfig.ServerInfos[MAX_Server];
config_properties SftpConfig;

HMODULE hDllModule;
bool delete_only_connection = FALSE;
bool accept_refresh = false;

//Plugin's initialization values
int PluginNumber;
tProgressProc ProgressProc;
tLogProc LogProc;
tRequestProc RequestProc;

int CurrentServer_ID= -1;
static int DllInitialised = 0;

//config DLL
HMODULE hDialogDLL = 0;
tProperties hProperties = 0;
tFreeCfgDLL hFreeCfgDLL = 0;

//password crypter DLL
HMODULE hPasswdCrypter = 0;
tSetupPasswordCrypter hSetupPasswordCrypter = 0;

int ImportPuttySession(struct SftpServerAccountInfo *ServerAccountInfo, 
                       char *PuttySectionName);
int ImportPuttySessions(int current_ID, char *import_mode);
int ImportSSHcomSessions(int lastInsert_ID, char *dir_location);

int octal_permissions_2_tc_integral(unsigned long octal_val);
int wcplg_sftp_connect_byID(int id);
unsigned int get_IDbyPath(char *path);
int get_sftpServer_ID_by_Title(char *Title);
int wcplg_sftp_do_commando_byID(char *sftp_cmd, char *serverOutput, int ID);
bool any_connection_active();
void LoadServers();
int get_sftpServer_ID_by_Path(char *Path);
void check_Concurrent_Connection(char *Path);
int file_exists_on_remote_server(char *RemoteFile);
int get_basename_from_Path(char *buf, char *Path);
void free_CurrentDirStruct(fxp_names * P_DirStruct, int ID);
bool SetFileTime__(char *fullFilePath, FILETIME * LastWriteTime);
void XconvertServerTitle(char *title);

char *SimpleCaption = "PasswordCrypter's master-password";
char *CaptionAskFirst = "PasswordCrypter's master-password (enter new one)";
char *CaptionAskSecond = "PasswordCrypter's master-password (verification)";

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

int get_custom_users_sftp_inifile_from_reg(char *ConfigIniFile)
{
  int ret = -1;
  HKEY subkey1;
  DWORD type, buflen = MAX_PATH;

  if (RegOpenKey(HKEY_CURRENT_USER, PETRICH_TC_REGP, &subkey1) ==
      ERROR_SUCCESS) {
    if (RegQueryValueEx
        (subkey1, PETRICH_TC_REGP_SFTP_K, 0, &type,
         (unsigned char *) ConfigIniFile, &buflen) == ERROR_SUCCESS) {
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
  RequestProc(PluginNumber, RT_MsgOK, "DBG", msg, NULL, 0);
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

  RequestProc(PluginNumber, RT_MsgOK, "Error", buf, NULL, 0);
}

//---------------------------------------------------------------------

void free_cache()
{
  for (dir_cache_iterator dci=DirCache.begin(); dci!=DirCache.end(); dci++) {
    fxp_names* dir_struct = dci->second;
    free_CurrentDirStruct(dir_struct, CurrentServer_ID);  //if we had multiple connections, the ID should be computed, otherwise ID shouldn't be needed anymore
  }
  DirCache.clear();
}

//---------------------------------------------------------------------

int sftp_disconnect(int ServerId, bool log_message)
{
  if (SftpConfig.CacheFS) {
    free_cache();
  }

  return wcplg_sftp_disconnect(ServerId, log_message);
}

//---------------------------------------------------------------------

extern "C" BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  UNUSED_ARG(lpReserved);
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    {
      DllInitialised++;
      //Get the ini's filename
      if (get_custom_users_sftp_inifile_from_reg(SftpConfig.ConfigIniFile) == -1) {
        hDllModule = (HMODULE) hModule;
        GetModuleFileName(hDllModule, SftpConfig.ConfigIniFile, 
          sizeof(SftpConfig.ConfigIniFile) - 1);
        char *p = strrchr(SftpConfig.ConfigIniFile, '\\');
        if (p)
          p++;
        else
          p = SftpConfig.ConfigIniFile;
        strcpy(p, DefaultIniFileName);
      }
      break;
    }
  case DLL_PROCESS_DETACH:
    {
      DllInitialised--;
      if (DllInitialised == 0) try {
        if (hDialogDLL) {
          hFreeCfgDLL();
          FreeLibrary(hDialogDLL);
          hDialogDLL = NULL;
          hProperties = NULL;
          hFreeCfgDLL = NULL;
        }
        if (hPasswdCrypter) {
          FreeLibrary(hPasswdCrypter);
          hPasswdCrypter = 0;
          SftpConfig.PasswordCrypterPath[0] = 0;
        }
      } catch (...) {
      }
    }
  }
  return TRUE;
}
//---------------------------------------------------------------------

int ServerTitleExists(int numServer, int currentServerId, char *title)
{
  int i;
  for (i = 0; i < numServer; i++) {
    if (i != currentServerId && strcmp(SftpConfig.ServerInfos[i].title, title) == 0) {
      return 1;
    }
  }
  return 0;
}

//---------------------------------------------------------------------

void MakeServerTitlesUnique(int numServer)
{
  int i;
  char buf[MAX_Server_INFO];
  char buf2[MAX_Server_INFO];

  for (i = 0; i < numServer; i++) {
    int dbl = 0;
    strcpy(buf, SftpConfig.ServerInfos[i].title);
    while (1)                   // :-)
    {
      if (ServerTitleExists(numServer, i, buf) == 1) {
        dbl++;
        sprintf(buf2, "%d", (dbl + 1));
        strcpy(buf, SftpConfig.ServerInfos[i].title);
        strcat(buf, " (");
        strcat(buf, buf2);
        strcat(buf, ")");
        continue;
      }
      if (dbl > 0) {
        strcpy(SftpConfig.ServerInfos[i].title, buf);
      }
      break;
    }
  }
}

//---------------------------------------------------------------------

int __stdcall FsInit(int PluginNr, tProgressProc pProgressProc,
                     tLogProc pLogProc, tRequestProc pRequestProc)
{
  //initialise global variables
  LastPath[0] = '\0';

  //remember all those values
  ProgressProc = pProgressProc;
  LogProc = pLogProc;
  RequestProc = pRequestProc;
  PluginNumber = PluginNr;

  //initialize all servers
  LoadServers();
  init_server_dll_handlers();
  unlink_ALL_dll_tmp_files();
  ResetAlreadyConnected();

  //dialog dll present?
  char cDir[MAX_CMD_BUFFER], cDLL[MAX_CMD_BUFFER];
  GetModuleFileName(hDllModule, cDir, MAX_CMD_BUFFER);
  char *p = strrchr(cDir, '\\');
  p[1] = 0;

  _snprintf(cDLL, MAX_CMD_BUFFER, "%s%s", cDir, DIALOG_DLL);

  hDialogDLL = LoadLibrary(cDLL);
  if (hDialogDLL) {
    hProperties = (tProperties)GetProcAddress(hDialogDLL, PROPERTIES_FUNCTION);
    tInitialize hInitialize = (tInitialize)GetProcAddress(hDialogDLL, INITIALIZE_FUNCTION);
    hFreeCfgDLL = (tFreeCfgDLL)GetProcAddress(hDialogDLL, FREE_FUNCTION);
    if ((hProperties==NULL) ||(hInitialize==NULL) || (hFreeCfgDLL==NULL))
    {
      FreeLibrary(hDialogDLL);
      hDialogDLL=0;
    } else {
      hInitialize(hDialogDLL);
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
  if (delete_only_connection) {
    SetLastError(ERROR_NO_MORE_FILES);
    return INVALID_HANDLE_VALUE;
  }

  pLastFindStuct lf;
  lf = (pLastFindStuct) malloc(sizeof(tLastFindStuct));
  lf->CurrentDirStruct = NULL;

  memset(FindData, 0, sizeof(WIN32_FIND_DATA));

  if (strcmp(Path, "\\") == 0) {
    // Connection selected
    if (!any_connection_active()) //Load Servers if there is no active connection
    {
      LoadServers();
    }
   
    lf->currentIndex = 0;
    lf->sumIndex = SftpConfig.ServerCount + 1;
    strcpy(FindData->cFileName, SftpConfig.ServerInfos[lf->currentIndex].title);

    FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
    FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
    lf->searchhandle = INVALID_HANDLE_VALUE;
    lf->SearchMode = HANDLE__SHOW_SFTP_SERVER;
    return (HANDLE) lf;
  } else {
    // So we have a connection - list selected directory
    lf->SearchMode = HANDLE__SHOW_SFTP_DIR;
    // check, which server is requested
    check_Concurrent_Connection(Path);

    //it's fatal if it doesn't even exists!
    if (CurrentServer_ID == NO_SERVER_ID) {
      dbg("fsfindfirst: CurrentServer_ID == -1, FIX ME!");
      SetLastError(ERROR_INVALID_ACCESS);
      return INVALID_HANDLE_VALUE;
    }

    char sftp_cmd[MAX_CMD_BUFFER];
    char *lPath = Path;
    fxp_names* CurrentDirStruct=NULL;
    std::string lFullPath;
    if (lFullPath[lFullPath.length()-1]=='\\')
      lFullPath = lFullPath.substr(0, lFullPath.length()-1);

    strlcpy(lf->Path, Path, MAX_PATH);
    lPath += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title)); //skip backslash and title ('\CONNECTION')

    if (SftpConfig.CacheFS) 
    {
      lFullPath = Path;
      if (lFullPath[lFullPath.length()-1]=='\\')
        lFullPath = lFullPath.substr(0, lFullPath.length()-1);
      if ((!accept_refresh) || (strcmp(lFullPath.c_str(), LastPath)!=0)) {
        CurrentDirStruct = DirCache[lFullPath];
      }
      strncpy(LastPath, lFullPath.c_str(), MAX_PATH);
    }    

    if (!CurrentDirStruct) {
      //prepare command line
      if (strcmp(lPath, "") == 0)
        sprintf(sftp_cmd, "ls");
      else
        _snprintf(sftp_cmd, MAX_CMD_BUFFER, "ls \".%s\"", lPath);

      //change that command line to unix style
      winSlash2unix(sftp_cmd);

      //execute commando
      if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) !=
          SFTP_SUCCESS) {
        LogProc_(MSGTYPE_CONNECTCOMPLETE, "Access denied!");
        SetLastError(ERROR_ACCESS_DENIED);
        return INVALID_HANDLE_VALUE;
      }

      //get the directory listing
      CurrentDirStruct = wcplg_sftp_get_current_dir_struct(CurrentServer_ID);

      if ((SftpConfig.CacheFS) && (CurrentDirStruct)) {
        fxp_names* old_dir_struct = DirCache[lFullPath];
        if ((old_dir_struct) && (CurrentDirStruct!=old_dir_struct)) {
          DirCache.erase(lFullPath);
          free_CurrentDirStruct(old_dir_struct, CurrentServer_ID);
        }
        DirCache[lFullPath] = CurrentDirStruct;
      }
    }

    if (CurrentDirStruct == NULL) {
      //this should come if you were disconnected. just a badness for 
      //reconnection :-) maybe we should read the ini for the right password :-) 
      //or better don't erase it if it was in .ini 
      char buf_log[MAX_CMD_BUFFER];
      wcplg_sftp_getLastError(CurrentServer_ID, buf_log);
      if (strcmp(buf_log, UNEXPECTED_OK_MSG) == 0) {
        sprintf(buf_log, "permission denied");
        LogProc_(MSGTYPE_DETAILS, buf_log);
        SetLastError(ERROR_INVALID_ACCESS);
        return (HANDLE) INVALID_HANDLE_VALUE;
      } else {
        sftp_disconnect(CurrentServer_ID, false);
        return FsFindFirst(Path, FindData);
      }

    }

    lf->CurrentDirStruct = CurrentDirStruct;
    lf->sumIndex = CurrentDirStruct->nnames;
    lf->currentIndex = 0;

    //skip some directories
    while ((lf->currentIndex < lf->sumIndex) &&
           ((strcmp
             (lf->CurrentDirStruct->names[lf->currentIndex].filename,
              ".") == 0)
            ||
            (strcmp
             (lf->CurrentDirStruct->names[lf->currentIndex].filename,
              "..") == 0)))
      (lf->currentIndex)++;

    //is there anything?
    if (lf->currentIndex >= lf->CurrentDirStruct->nnames) {
      if (lf->CurrentDirStruct->nnames<0) {
        LogProc_(MSGTYPE_DETAILS, "Access denied(*)!");
        SetLastError(ERROR_ACCESS_DENIED);
      } else
        SetLastError(ERROR_NO_MORE_FILES);

      if (!SftpConfig.CacheFS) {
        free_CurrentDirStruct(CurrentDirStruct, CurrentServer_ID);
      }
      return INVALID_HANDLE_VALUE;
    }

    //Copy that info to TotalCommander's structure
    char FileTyp;
    FileTyp = lf->CurrentDirStruct->names[lf->currentIndex].longname[0];
    FindData->dwReserved0 =
      octal_permissions_2_tc_integral(lf->CurrentDirStruct->names[lf->currentIndex].
                          attrs.permissions);

    if (FileTyp == 'd') {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | 0x80000000;
    } else if (FileTyp == 'l') {
      FindData->dwFileAttributes =
        FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000;
      FindData->dwReserved0 += S_IFLNK; // Wincmd uses only this one!
    } else {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL | 0x80000000;
    }

    if (!UnixTimeToLocalTime
        ((long *) &lf->CurrentDirStruct->names[lf->currentIndex].attrs.
         mtime, &FindData->ftLastWriteTime)) {
      FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
      FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
    }

    FindData->nFileSizeHigh =
      (DWORD) lf->CurrentDirStruct->names[lf->currentIndex].attrs.size.hi;
    FindData->nFileSizeLow =
      (DWORD) lf->CurrentDirStruct->names[lf->currentIndex].attrs.size.lo;
    strcpy(FindData->cFileName,
           CurrentDirStruct->names[lf->currentIndex].filename);

    return (HANDLE) lf;
  }
  return INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------

BOOL __stdcall FsFindNext(HANDLE Hdl, WIN32_FIND_DATA * FindData)
{
  pLastFindStuct lf;
  lf = (pLastFindStuct) Hdl;

  //is it a connection listing?
  if (lf->SearchMode == HANDLE__SHOW_SFTP_SERVER) {
    if (lf->currentIndex >= lf->sumIndex)
      return false;             //this is the last entry 

    //copy entry
    if (lf->currentIndex == lf->sumIndex - 1) {
      lf->currentIndex++;
      strcpy(FindData->cFileName, DefineEditConnection_define);
      FindData->dwFileAttributes = 0;
    } else {
      if (lf->currentIndex == lf->sumIndex - 2) {
        lf->currentIndex++;
        strcpy(FindData->cFileName, DefineAddConnection_define);
        FindData->dwFileAttributes = 0;
      } else {
        lf->currentIndex++;
        strcpy(FindData->cFileName, SftpConfig.ServerInfos[lf->currentIndex].title);
        FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
      }
    }
    FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
    FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
    return true;
  }

  //it's not a connection listing, it's real directory list
  if (lf->SearchMode == HANDLE__SHOW_SFTP_DIR) {
    lf->currentIndex++;
    while ((lf->currentIndex < lf->sumIndex) &&
           ((strcmp
             (lf->CurrentDirStruct->names[lf->currentIndex].filename,
              ".") == 0)
            ||
            (strcmp
             (lf->CurrentDirStruct->names[lf->currentIndex].filename,
              "..") == 0)))
      (lf->currentIndex)++;

    if (lf->currentIndex >= lf->sumIndex)
      return false;             //it's the last entry
    strcpy(FindData->cFileName,
           lf->CurrentDirStruct->names[lf->currentIndex].filename);

    FindData->dwReserved0 =
      octal_permissions_2_tc_integral(lf->CurrentDirStruct->names[lf->currentIndex].
                          attrs.permissions);

    char FileTyp;
    FileTyp = lf->CurrentDirStruct->names[lf->currentIndex].longname[0];
    if (FileTyp == 'd')         //directory
    {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | 0x80000000;
    } else if (FileTyp == 'l')  //link
    {
      FindData->dwFileAttributes =
        FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000;
      FindData->dwReserved0 += S_IFLNK; //TotalCommander uses only this one!
    } else                      //anything else :-)
    {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL | 0x80000000;
    }

    if (!UnixTimeToLocalTime
        ((long *) &lf->CurrentDirStruct->names[lf->currentIndex].attrs.
         mtime, &FindData->ftLastWriteTime)) {
      FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
      FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
    }

    FindData->nFileSizeHigh =
      (DWORD) lf->CurrentDirStruct->names[lf->currentIndex].attrs.size.hi;
    FindData->nFileSizeLow =
      (DWORD) lf->CurrentDirStruct->names[lf->currentIndex].attrs.size.lo;

    return true;
  }

  dbg("FIX ME: this is not a regular listing mode!");
  return false;
}
//---------------------------------------------------------------------

int __stdcall FsFindClose(HANDLE Hdl)
{
  pLastFindStuct lf;
  lf = (pLastFindStuct) Hdl;

  if (lf != INVALID_HANDLE_VALUE) {
    if (!SftpConfig.CacheFS)
      free_CurrentDirStruct(lf->CurrentDirStruct, CurrentServer_ID);
    else
      lf->CurrentDirStruct = NULL;
    free(lf);                   //now we can free this
  }

  return 0;
}
//---------------------------------------------------------------------

//free all buffers
void free_CurrentDirStruct(fxp_names * P_DirStruct, int ServerID)
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

bool addnewserver(char *servertitle)
{
  char host[MAX_PATH], user[MAX_PATH], pwd[MAX_PATH], port[MAX_PATH];
  char home_dir[MAX_PATH];
  char section_name[MAX_Server_INFO];
  char servertitle_[MAX_Server_INFO];
  bool wantcompression;

  servertitle_[0] = '\0';

  host[0] = 0;
  user[0] = 0;
  pwd[0] = 0;
  home_dir[0] = '/';
  home_dir[1] = 0;

  if (servertitle == NULL) {
    while (strlen(servertitle_) <= 0) {
      if (!RequestProc
          (PluginNumber, RT_Other, "Server Title", "Server Title:",
           servertitle_, sizeof(servertitle_) - 1))
        return false;
    }
  } else {
    strcpy(servertitle_, servertitle);
  }

  XconvertServerTitle(servertitle);
  XconvertServerTitle(servertitle_);  //we have to convert both buffers, but where's the difference?

  if (!RequestProc
      (PluginNumber, RT_Other, "New Connection", "Host[:port]", host,
       sizeof(host) - 1))
    return false;

  strcpy(port, host);
  trim_host_from_hoststring(host);
  trim_port_from_hoststring(port);
  if (!strlen(port))
    strcpy(port, "22");

  if (!RequestProc
      (PluginNumber, RT_UserName, "New Connection", NULL, user,
       sizeof(user) - 1))
    return false;

  if (!RequestProc
      (PluginNumber, RT_Password, "New Connection", NULL, pwd,
       sizeof(user) - 1))
    return false;

  if (!RequestProc
      (PluginNumber, RT_TargetDir, "New Connection", NULL, home_dir,
       sizeof(user) - 1))
    return false;

  // RequestProc mit RT_MsgYesNo Funzt nicht, gibt immer FALSE zurück

  wantcompression =
    RequestProc(PluginNumber, RT_MsgOKCancel,
                "Add connection - compression",
                "Do you want to compress the data connection (only recommended for slow connections)?",
                NULL, 0) == TRUE;

  sprintf(section_name, "%i", SftpConfig.ServerCount - SftpConfig.ImportedSessions);
  WritePrivateProfileString(section_name, "title", servertitle_, SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "host", host, SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "username", user, SftpConfig.ConfigIniFile);
  if ((SftpConfig.EncryptPassword) && (strlen(SftpConfig.PasswordCrypterPassword)>0)) {
    char buf[4000];
    memset(buf, 0, sizeof(buf));
    SftpConfig.EncryptPassword(pwd, buf);
    WritePrivateProfileString(section_name, "password", buf, SftpConfig.ConfigIniFile);
  } else {
    WritePrivateProfileString(section_name, "password", pwd, SftpConfig.ConfigIniFile);
  }
  WritePrivateProfileString(section_name, "port", port, SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "home_dir", home_dir, SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "compression",
                            wantcompression ? "1" : "0", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "use_key_auth", "0", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "keyfilename", "", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "dont_ask4_passphrase", "0",
                            SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "dont_ask4_username",
                            strcmp(user, "") == 0 ? "0" : "1", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "dont_ask4_password",
                            strcmp(pwd, "") == 0 ? "0" : "1", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_type", "0", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_host", "", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_port", "0", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_username", "", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_password", "", SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "proxy_telnet_command", "",
                            SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "chmod_value_put", "",
                            SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "chmod_value_mkdir", "",
                            SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "set_chmod_after_put", "0",
                            SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "set_chmod_after_mkdir", "0",
                            SftpConfig.ConfigIniFile);
  WritePrivateProfileString(section_name, "set_mtime_after_put", "1",
                            SftpConfig.ConfigIniFile);

  LoadServers();
  return true;
}
//---------------------------------------------------------------------

#define MOVE_INFO(KEY) GetPrivateProfileString(section_name_old, KEY, "", copybuf, \
                                MAX_Server_INFO, SftpConfig.ConfigIniFile); \
        WritePrivateProfileString(section_name_new, KEY, copybuf, \
                                  SftpConfig.ConfigIniFile);

#define MOVE_INFO_PARAM(KEY, PARAM) GetPrivateProfileString(section_name_old, KEY, PARAM, copybuf, \
                                MAX_Server_INFO, SftpConfig.ConfigIniFile); \
        WritePrivateProfileString(section_name_new, KEY, copybuf, \
                                  SftpConfig.ConfigIniFile); 

#define MOVE_INFO_WRITEPARAM(KEY, PARAM) GetPrivateProfileString(section_name_old, KEY, "", copybuf, \
                                MAX_Server_INFO, SftpConfig.ConfigIniFile); \
        WritePrivateProfileString(section_name_new, KEY, PARAM, \
                                  SftpConfig.ConfigIniFile); 
//---------------------------------------------------------------------

bool deletethisconnection(char *servertitle)
{
  int i, j, Sid;
  char section_name_old[MAX_Server_INFO];
  char section_name_new[MAX_Server_INFO];

  Sid = get_sftpServer_ID_by_Title(servertitle);
  if (Sid == -1) {
    RequestProc(PluginNumber, RT_MsgOK, "Delete connection",
                "Delete Error: deletethisconnection(): Sid=-1 FIX me plz - FAST",
                NULL, 0);
    return false;
  }
  if (SftpConfig.ServerInfos[Sid].is_imported_from_any_datasrc == 1) {
    if (SftpConfig.ServerInfos[Sid].is_imported_from_putty_registry == 1) {
      RequestProc(PluginNumber, RT_MsgOK, "Delete connection",
                  "This connection is imported from the PuTTY Session Database - can not delete",
                  NULL, 0);
      return false;
    }
    if (SftpConfig.ServerInfos[Sid].is_imported_from_sshcom_registry == 1) {
      RequestProc(PluginNumber, RT_MsgOK, "Delete connection",
                  "This connection is imported from the ssh.com Session Database - can not delete",
                  NULL, 0);
      return false;
    }
    RequestProc(PluginNumber, RT_MsgOK, "Delete connection",
                "This connection is imported  - can not delete", NULL, 0);
    return false;
  }

  for (i = 0; i < SftpConfig.ServerCount - 1 - SftpConfig.ImportedSessions; i++) {
    if (strcmp(SftpConfig.ServerInfos[i].title, servertitle) == 0) {
      if (SftpConfig.ServerInfos[i].is_imported_from_any_datasrc == 1) {
        RequestProc(PluginNumber, RT_MsgOK, "Delete connection",
                    "This connection is imported from the PuTTY Session Database - can not delete",
                    NULL, 0);
        return false;
      }
      // Delete server i by moving all sections 1 up
      // What about Section rename?
      i++;                      // It's 1-based in the ini file!
      for (j = i; j < SftpConfig.ServerCount - SftpConfig.ImportedSessions; j++) {
        sprintf(section_name_old, "%i", j + 1);
        sprintf(section_name_new, "%i", j);
        char copybuf[MAX_Server_INFO];

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
      sprintf(section_name_old, "%i", SftpConfig.ServerCount - 1 - SftpConfig.ImportedSessions);
      WritePrivateProfileString(section_name_old, NULL, NULL, SftpConfig.ConfigIniFile);
      LoadServers();
      return TRUE;
    }
  }
  return FALSE;
}
//---------------------------------------------------------------------

int remote_chmod(char *RemoteName, unsigned int value)
{
  check_Concurrent_Connection(RemoteName);
  char *lRemoteName = RemoteName + (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  char sftp_cmd[MAX_CMD_BUFFER];

  _snprintf(sftp_cmd, MAX_CMD_BUFFER, "chmod %d \"%s\"", value, lRemoteName);

  winSlash2unix(sftp_cmd);

  if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) == SFTP_SUCCESS) {
    return FS_EXEC_OK;
  }
    
  return FS_EXEC_ERROR;
}
//---------------------------------------------------------------------

int remote_mtime(char *RemoteName, unsigned long value)
{
  check_Concurrent_Connection(RemoteName);
  char *lRemoteName = RemoteName + (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  char sftp_cmd[MAX_CMD_BUFFER];

  _snprintf(sftp_cmd, MAX_CMD_BUFFER, "mtime %d \"%s\"", value, lRemoteName);

  winSlash2unix(sftp_cmd);

  if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) == SFTP_SUCCESS) {
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

  check_Concurrent_Connection(Path);

  lPath += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);

  sprintf(cmd_buf, "mkdir \"%s\"", lPath);

  winSlash2unix(cmd_buf);

  if (wcplg_sftp_do_commando_byID(cmd_buf, NULL, CurrentServer_ID) == SFTP_SUCCESS)
  {
    if (SftpConfig.ServerInfos[CurrentServer_ID].set_chmod_after_mkdir)
      remote_chmod(Path, SftpConfig.ServerInfos[CurrentServer_ID].chmod_value_mkdir); 
    return true;
  }
  return false;
}
//---------------------------------------------------------------------

void DefineAndAddConnection()
{
  HANDLE myFile;

  myFile = CreateFile(SftpConfig.ConfigIniFile, // file name
    GENERIC_READ,     // access mode
    FILE_SHARE_READ,  // share mode
    NULL,             // security descriptor
    OPEN_ALWAYS,      // how to create
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
    NULL); // handle to template file

  if (myFile == INVALID_HANDLE_VALUE)
  {
    err_v("Error opening '%s'", SftpConfig.ConfigIniFile);
  } else {
    CloseHandle(myFile);
  }

  ShellExecute(0, "open", SftpConfig.ConfigIniFile, NULL, "c:\\", SW_SHOW);
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

//This function needs a real revision
int __stdcall FsExecuteFile(HWND MainWin, char *RemoteName, char *Verb)
{
  char buf[MAX_CMD_BUFFER];
  char sftp_cmd[MAX_CMD_BUFFER];
  char log_sftp_cmd[MAX_CMD_BUFFER];
  char *lVerb = Verb;
  char *lRemoteName = RemoteName;

  SftpConfig.MainWindow = MainWin;

  if (stricmp(Verb, "open") == 0) {
    if (strcmp(DefineEditConnection_selected, RemoteName) == 0) {
      if (hDialogDLL) {
        if (hProperties(0, &SftpConfig)) {
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
      if (hDialogDLL) {
        ret = hProperties(1, &SftpConfig);
      } else {
        ret = addnewserver(NULL);
      }
      if (ret) {
        RemoteName[1] = '\0';
        return FS_EXEC_SYMLINK;
      }
      return FS_EXEC_OK;
    }

    check_Concurrent_Connection(RemoteName);

    if (Risdir(RemoteName))     //if symlink
    {
      return FS_EXEC_SYMLINK;
    }
  }

  if (_strnicmp(Verb, "quote root ", 11) == 0) {

    if (CurrentServer_ID == -1)
      return FS_EXEC_ERROR;

    if (SftpConfig.CacheFS)
      free_cache();

    strcpy(SftpConfig.ServerInfos[CurrentServer_ID].home_dir, &Verb[11]);
    /*size_t i = strlen(SftpConfig.ServerInfos[CurrentServer_ID].home_dir);
    char last_one = SftpConfig.ServerInfos[CurrentServer_ID].home_dir[i - 1];*/

    sprintf(sftp_cmd, "cd %s", SftpConfig.ServerInfos[CurrentServer_ID].home_dir);
    winSlash2unix(sftp_cmd);
    if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) !=
        SFTP_SUCCESS) {
      sprintf(log_sftp_cmd, "change root failed!");
      LogProc_(MSGTYPE_DETAILS, log_sftp_cmd);
    }

    sprintf(RemoteName, "\\%s\\", SftpConfig.ServerInfos[CurrentServer_ID].title);
    return FS_EXEC_SYMLINK;
  }

  //explicit REPUT SUPPORT
  if (stricmp(Verb, "quote reput") == 0 || stricmp(Verb, "quote reput ") == 0
      || (strlen(Verb) >= strlen("quote reput  ")
          && strncmp(Verb, "quote reput  ", 13) == 0)) {
    RequestProc(PluginNumber, RT_MsgOK, "Usage for reput", "Usage: reput <file>", NULL, 0);
    return FS_EXEC_ERROR;
  }

  if (strlen(Verb) > strlen("quote reput") + 1) {
    //check if connected and inside of a connection
    if ((RemoteName != NULL) && (strlen(RemoteName) == 1) && (RemoteName[0] == '\\')) {
      RequestProc(PluginNumber, RT_MsgOK, "Reput", "Can not use reput here", NULL, 0);
      return FS_EXEC_ERROR;
    }

    strcpy(buf, Verb);
    if (_strnicmp(buf, "quote reput ", 12) == 0) {
      char remote_users_current_dir[MAX_CMD_BUFFER];
      int sftp_ret;

      if ((RemoteName == NULL) || (strlen(RemoteName) <= 0)
          || (strchr(RemoteName, '?'))) {
        dbg_v("Invalid RemoteName ('%s') FIX ME PLZ", RemoteName);
        return FS_EXEC_ERROR;
      }

      check_Concurrent_Connection(RemoteName);

      //we must use tricks
      // segFault check!!!

      strncpy(remote_users_current_dir, buf + strlen("quote reput") + 1,
              MAX_CMD_BUFFER);
      strncat(remote_users_current_dir, "?", MAX_CMD_BUFFER);
      strncat(remote_users_current_dir,
        RemoteName + 2 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title), MAX_CMD_BUFFER);

      winSlash2unix(remote_users_current_dir);

      //Log msg
      _snprintf(log_sftp_cmd, MAX_CMD_BUFFER, "reput %s",
                remote_users_current_dir);

      strncpy(sftp_cmd, "reput ", MAX_CMD_BUFFER);
      strncat(sftp_cmd, "\"", MAX_CMD_BUFFER);
      strncat(sftp_cmd, remote_users_current_dir, MAX_CMD_BUFFER);
      strncat(sftp_cmd, "\"", MAX_CMD_BUFFER);

      char *buf_shifted = buf + strlen("quote reput") + 1;

      ProgressProc(PluginNumber, buf_shifted, buf_shifted, 0);

      LogProc_(MSGTYPE_DETAILS, log_sftp_cmd);
      DISABLE_LOGGING_ONCE();

      sftp_ret = wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID);
      //Put done
      ProgressProc(PluginNumber, buf_shifted, buf_shifted, 100);

      //any error?
      if (sftp_ret == SFTP_SUCCESS) {
        return FS_EXEC_OK;
      }

      return FS_EXEC_ERROR;
    }
  }

  //explicit REGET SUPPORT
  if (strcmp(Verb, "quote reget") == 0 || strcmp(Verb, "quote reget ") == 0
      || (strlen(Verb) >= strlen("quote reget  ")
          && strncmp(Verb, "quote reget  ", 13) == 0)) {
    RequestProc(PluginNumber, RT_MsgOK, "Usage for reget",
                "Usage: reget <file>", NULL, 0);
    return FS_EXEC_ERROR;
  }

  if (strlen(Verb) > strlen("quote reget") + 1) {
    //check if connected 
    if (RemoteName != NULL && strlen(RemoteName) == 1
        && RemoteName[0] == '\\') {
      RequestProc(PluginNumber, RT_MsgOK, "reget", "Can not use reget here", NULL, 0);
      return FS_EXEC_ERROR;
    }

    strcpy(buf, Verb);
    if (_strnicmp(buf, "quote reget ", 12) == 0) {
      char remote_users_current_dir[MAX_CMD_BUFFER];
      char log_sftp_cmd[MAX_CMD_BUFFER];
      int sftp_ret;

      if (RemoteName == NULL || strlen(RemoteName) <= 0
          || strchr(RemoteName, '?')) {
        dbg_v("Invalid RemoteName('%s') FIX ME PLZ", RemoteName);
        return FS_EXEC_ERROR;
      }

      check_Concurrent_Connection(RemoteName);

      char *buf_shifted = buf + strlen("quote reget") + 1;

      //we must use tricks
      // segFault check!!!
      strcpy(remote_users_current_dir, buf_shifted);

      //Log msg
      _snprintf(log_sftp_cmd, MAX_CMD_BUFFER, "reget %s", remote_users_current_dir);

      strcat(remote_users_current_dir, "?");
      strcat(remote_users_current_dir,
        RemoteName + 2 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title));
      winSlash2unix(remote_users_current_dir);

      _snprintf(sftp_cmd, MAX_CMD_BUFFER, "reget \"%s\"", remote_users_current_dir);

      ProgressProc(PluginNumber, buf_shifted, buf_shifted, 0);

      LogProc_(MSGTYPE_DETAILS, log_sftp_cmd);
      DISABLE_LOGGING_ONCE();

      sftp_ret = wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID);
      //Done
      ProgressProc(PluginNumber, buf_shifted, buf_shifted, 100);

      //Any errors?
      if (sftp_ret == SFTP_SUCCESS) {
        return FS_EXEC_OK;
      }
      return FS_EXEC_ERROR;

    }
  }

  if (_strnicmp(Verb, "quote cd ", 9) == 0) {
    strcpy(buf, Verb);
    check_Concurrent_Connection(RemoteName);

    //cd command: Change dir to what user wanted!
    // two cases: absolute path with / at start, relative with backslash

    if (buf[9] == '/' || buf[9] == '\\') {
      _snprintf(RemoteName, MAX_PATH - 1, "\\%s%s", SftpConfig.ServerInfos[CurrentServer_ID].title, buf + 9);
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
    if (_strnicmp(RemoteName+1, SftpConfig.ServerInfos[CurrentServer_ID].title, 
      strlen(SftpConfig.ServerInfos[CurrentServer_ID].title))==0) {
      if (hDialogDLL) {
        if (hProperties(2, &SftpConfig)) {
          RemoteName[1] = '\0';
          return FS_EXEC_SYMLINK;
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
    check_Concurrent_Connection(lRemoteName);
    lRemoteName += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);

    strcpy(sftp_cmd, "chmod ");
    strcat(sftp_cmd, lVerb);
    if (Verb[0]!='q')
    {
      strcat(sftp_cmd, " \"./");
      strcat(sftp_cmd, lRemoteName);
      strcat(sftp_cmd, "\"");
    }

    winSlash2unix(sftp_cmd);

    if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) ==
        SFTP_SUCCESS) {
      return FS_EXEC_OK;
    } else
      return FS_EXEC_ERROR;
  }

  //CHOWN
  if (_strnicmp(Verb, "quote chown ", 12)==0) {
    strcpy(sftp_cmd, &Verb[6]);

    winSlash2unix(sftp_cmd);

    if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) ==
        SFTP_SUCCESS) {
      return FS_EXEC_OK;
    } else
      return FS_EXEC_ERROR;
  }

  //MODE
  if (_strnicmp(Verb, "MODE ", 5)==0) {
    if (strlen(Verb)>=6) {
      switch (Verb[5])
      {
        case 'A':
        case 'X':
          wcplg_sftp_transfermode(CurrentServer_ID, Verb+5);
          break;
        default:
          wcplg_sftp_transfermode(CurrentServer_ID, "I\0");
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

  serverid_from_oldname = get_sftpServer_ID_by_Path(OldName);
  serverid_from_newname = get_sftpServer_ID_by_Path(NewName);
  if (serverid_from_oldname == -1 || serverid_from_newname == -1) {
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
    int err = ProgressProc(PluginNumber, OldName, NewName, 0);
    check_Concurrent_Connection(OldName);

    if (!OverWrite) {
      int R_fexists = file_exists_on_remote_server(NewName);
      if (R_fexists == FS_FILE_EXISTS) {
        return FS_FILE_EXISTS;
      }
    }
    // Rename!
    OldName += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
    NewName += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);

    strcpy(cmd_buf, "rename \"./");
    strcat(cmd_buf, OldName);
    strcat(cmd_buf, "\" \"./");
    strcat(cmd_buf, NewName);
    strcat(cmd_buf, "\"");

    OldName -= (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
    NewName -= (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);

    winSlash2unix(cmd_buf);

    err = ProgressProc(PluginNumber, OldName, NewName, 50);
    if (wcplg_sftp_do_commando_byID(cmd_buf, NULL, CurrentServer_ID) ==
        SFTP_SUCCESS) {
      err = ProgressProc(PluginNumber, OldName, NewName, 100);
      return FS_FILE_OK;
    }

    err = ProgressProc(PluginNumber, OldName, NewName, 100);

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
    char path_[MAX_CMD_BUFFER];
    char fname_[MAX_CMD_BUFFER];
    char wd_saved[MAX_CMD_BUFFER];
    char wd_curr[MAX_CMD_BUFFER];

    get_basename_from_Path(path_, RemoteName);
    strcpy(fname_, RemoteName + (strlen(path_) + 1));

    strcpy(cmd_buf, "quote reget ");
    strcat(cmd_buf, fname_);
    strcat(path_, "\\");

    if (getcwd(wd_saved, MAX_PATH) == NULL) {
      return FS_FILE_WRITEERROR;
    }
    get_basename_from_Path(wd_curr, LocalName);
    if (chdir(wd_curr) != 0) {
      return FS_FILE_WRITEERROR;
    }

    ret = FsExecuteFile((HWND) NULL, path_, cmd_buf);
    chdir(wd_saved);

    if (ret == FS_EXEC_OK) {
      return FS_FILE_OK;
    }
    return FS_FILE_WRITEERROR;
  }

  check_Concurrent_Connection(RemoteName);

  strcpy(cmd_buf, "get \"./");

  RemoteName += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  strcat(cmd_buf, RemoteName);
  RemoteName -= (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  winSlash2unix(cmd_buf);
  strcat(cmd_buf, "\"");

  strcat(cmd_buf, " \"");
  strcat(cmd_buf, LocalName);
  strcat(cmd_buf, "\"");

  err = ProgressProc(PluginNumber, RemoteName, LocalName, 0);

  if (err)
    return FS_FILE_USERABORT;

  ok =
    (wcplg_sftp_do_commando_byID(cmd_buf, NULL, CurrentServer_ID) ==
     SFTP_SUCCESS);

  if (ok) {
    if (ri != NULL) {
      //is RemoteInfo  set?
      SetFileTime__(LocalName, &ri->LastWriteTime);
    }

    if (CopyFlags & FS_COPYFLAGS_MOVE) {
      cmd_buf[1] = 'r';
      cmd_buf[2] = 'm';         // send rm command to move!
      ok =
        (wcplg_sftp_do_commando_byID(cmd_buf + 1, NULL, CurrentServer_ID)
         == SFTP_SUCCESS);
    }

    err = ProgressProc(PluginNumber, RemoteName, LocalName, 100);
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
    if (get_sftpServer_ID_by_Path(RemoteName) == -1) {
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
    char path_[MAX_CMD_BUFFER];
    char fname_[MAX_CMD_BUFFER];
    char wd_saved[MAX_CMD_BUFFER];
    char wd_curr[MAX_CMD_BUFFER];

    get_basename_from_Path(path_, RemoteName);
    strcpy(fname_, RemoteName + (strlen(path_) + 1));

    strcpy(cmd_buf, "quote reput ");
    strcat(cmd_buf, fname_);
    strcat(path_, "\\");

    if (getcwd(wd_saved, MAX_PATH) == NULL) {
      return FS_FILE_WRITEERROR;
    }

    get_basename_from_Path(wd_curr, LocalName);

    if (chdir(wd_curr) != 0) {
      return FS_FILE_WRITEERROR;
    }

    ret = FsExecuteFile((HWND) NULL, path_, cmd_buf);
    chdir(wd_saved);

    if (ret == FS_EXEC_OK) {
      return FS_FILE_OK;
    }

    return FS_FILE_WRITEERROR;
  }

  check_Concurrent_Connection(RemoteName);

  strcpy(cmd_buf, "put \"");
  strcat(cmd_buf, LocalName);
  strcat(cmd_buf, "\" \"./");

  char *lRemoteName = RemoteName + (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  strcpy(buf2, lRemoteName);
  winSlash2unix(buf2);
  strcat(cmd_buf, buf2);
  strcat(cmd_buf, "\"");

  err = ProgressProc(PluginNumber, LocalName, RemoteName, 0);
  if (err)
    return FS_FILE_USERABORT;

  ok =
    (wcplg_sftp_do_commando_byID(cmd_buf, NULL, CurrentServer_ID) ==
     SFTP_SUCCESS);

  if (ok) {
    if (CopyFlags & FS_COPYFLAGS_MOVE) {
      ok = (0 == _unlink(LocalName)); // delete source after successful upload
    }
    err = ProgressProc(PluginNumber, RemoteName, LocalName, 100);
    if (err)
      return FS_FILE_USERABORT;
    if (ok)
    {
      if (SftpConfig.ServerInfos[CurrentServer_ID].set_chmod_after_put)
      {
        remote_chmod(RemoteName, SftpConfig.ServerInfos[CurrentServer_ID].chmod_value_put);
      }
      if (SftpConfig.ServerInfos[CurrentServer_ID].set_mtime_after_put)
      {
        bool f;
        HANDLE myFile;
        FILETIME LastWriteTime;

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
             NULL, NULL, &LastWriteTime)==TRUE);
          CloseHandle(myFile);
        }

        if (f) {
          SYSTEMTIME st;
          tm atm;
          FileTimeToSystemTime(&LastWriteTime, &st);

          atm.tm_year = st.wYear - 1900;
          atm.tm_mon = st.wMonth - 1;
          atm.tm_mday = st.wDay;
          atm.tm_hour = st.wHour;
          atm.tm_min = st.wMinute;
          atm.tm_sec = st.wSecond;
          atm.tm_isdst = 0;
          unsigned long new_mtime = mktime(&atm);
          
          remote_mtime(RemoteName, new_mtime);
        }
      }
      return FS_FILE_OK;
    }
  }

  return FS_FILE_WRITEERROR;
}

//---------------------------------------------------------------------

BOOL __stdcall FsDeleteFile(char *RemoteName)
{
  char cmd_buf[MAX_CMD_BUFFER];

  if (RemoteName[0] != '\\')
    return false;

  strcpy(cmd_buf, "rm \"./");

  ProgressProc(PluginNumber, RemoteName, RemoteName, 0);
  check_Concurrent_Connection(RemoteName);

  RemoteName += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  strcat(cmd_buf, RemoteName);
  RemoteName -= (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  strcat(cmd_buf, "\"");

  winSlash2unix(cmd_buf);

  ProgressProc(PluginNumber, RemoteName, RemoteName, 50);
  if (wcplg_sftp_do_commando_byID(cmd_buf, NULL, CurrentServer_ID) ==
      SFTP_SUCCESS) {
    ProgressProc(PluginNumber, RemoteName, RemoteName, 100);
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
    RequestProc(PluginNumber, RT_MsgOK, "Delete Connection",
                "Can not delete " QUICK_CONNECTION, NULL, 0);
    return false;
  }

  if (delete_only_connection) {
    deletethisconnection(RemoteName + 1);
    return true;
  }

  char cmd_buf[MAX_CMD_BUFFER];

  check_Concurrent_Connection(RemoteName);

  if (SftpConfig.CacheFS) {
    std::string lFullPath = RemoteName;
    fxp_names* tmp_dir = DirCache[lFullPath];
    if (tmp_dir) {
      DirCache.erase(lFullPath);
      free_CurrentDirStruct(tmp_dir, CurrentServer_ID);
    }
    std::string::size_type pos = lFullPath.find_last_of('\\');
    if (pos!=std::string::npos) {
      lFullPath = lFullPath.substr(0, pos);
      fxp_names* tmp_dir = DirCache[lFullPath];
      if (tmp_dir) {
        DirCache.erase(lFullPath);
        free_CurrentDirStruct(tmp_dir, CurrentServer_ID);
      }
    }
  }

  strcpy(cmd_buf, "rmdir \"./");

  RemoteName += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  strcat(cmd_buf, RemoteName);
  RemoteName -= (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);

  strcat(cmd_buf, "\"");

  winSlash2unix(cmd_buf);

  if (wcplg_sftp_do_commando_byID(cmd_buf, NULL, CurrentServer_ID) ==
      SFTP_SUCCESS)
    return true;
  return false;
}

//---------------------------------------------------------------------

int wcplg_sftp_do_commando_byID(char *sftp_cmd, char *serverOutput, int ID)
{
  if (wcplg_sftp_connect_byID(ID) == SFTP_SUCCESS) {
    return wcplg_sftp_do_commando(sftp_cmd, serverOutput, ID);
  }
  return SFTP_FAILED;
}

//---------------------------------------------------------------------

int wcplg_sftp_connect_byID(int ID)
{
  bool server_entered = false;
  bool user_entered = false;
  char buf[MAX_CMD_BUFFER];

  /*
     sprintf(buf,"severid:%d",ID);
     LogProc_(MSGTYPE_DETAILS,buf);
   */

  if (ID == -1)
    return SFTP_FAILED;         //who knows, what happend before :-)

  if (strlen(SftpConfig.ServerInfos[ID].host_cached) < 1) {
    strcpy(SftpConfig.ServerInfos[ID].host_cached, SftpConfig.ServerInfos[ID].host);
    if (!RequestProc(PluginNumber, RT_Other, "Secure FTP", "Host[:port]",
      SftpConfig.ServerInfos[ID].host_cached, MAX_CMD_BUFFER)) {
        SftpConfig.ServerInfos[ID].host_cached[0] = '\0';
      return SFTP_FAILED;
    }

    server_entered = true;
    strcpy(SftpConfig.ServerInfos[ID].host, SftpConfig.ServerInfos[ID].host_cached);
  }

  if (strlen(SftpConfig.ServerInfos[ID].username_cached) < 1) {
    strcpy(buf, "Username for ");
    strcat(buf, SftpConfig.ServerInfos[ID].host_cached);

    strcpy(SftpConfig.ServerInfos[ID].username_cached, SftpConfig.ServerInfos[ID].username);

    // Do not ask 4 username if dont_ask4_username==1
    if (SftpConfig.ServerInfos[ID].dont_ask4_username != 1) {
      if (!RequestProc
          (PluginNumber, RT_UserName, "Secure FTP", buf,
           SftpConfig.ServerInfos[ID].username_cached, MAX_CMD_BUFFER)) {
        // For quick connection, clear host if failed
        SftpConfig.ServerInfos[ID].username_cached[0] = 0;
        if (server_entered)
          SftpConfig.ServerInfos[ID].host_cached[0] = 0;
        return SFTP_FAILED;
      }
    }
    user_entered = true;
    strcpy(SftpConfig.ServerInfos[ID].username, SftpConfig.ServerInfos[ID].username_cached);
  }

  if (strlen(SftpConfig.ServerInfos[ID].password_cached) < 1) {
    strcpy(buf, "Password for ");
    strcat(buf, SftpConfig.ServerInfos[ID].username_cached);
    strcat(buf, "@");
    strcat(buf, SftpConfig.ServerInfos[ID].host_cached);

    strcpy(SftpConfig.ServerInfos[ID].password_cached, SftpConfig.ServerInfos[ID].password);

    if (SftpConfig.ServerInfos[ID].use_key_auth != 1) {
      if (!SftpConfig.ServerInfos[ID].dont_ask4_password) {
        if (!RequestProc
            (PluginNumber, RT_Password, "Secure FTP", buf,
             SftpConfig.ServerInfos[ID].password_cached, MAX_CMD_BUFFER)) {
          // For quick connection, clear host+user if failed
          if (server_entered)
            SftpConfig.ServerInfos[ID].host_cached[0] = 0;
          if (user_entered)
            SftpConfig.ServerInfos[ID].username_cached[0] = 0;
          return SFTP_FAILED;
        }
      }
    }

    if (SftpConfig.ServerInfos[ID].dont_ask4_password)
      strcpy(SftpConfig.ServerInfos[ID].password_cached, SftpConfig.ServerInfos[ID].password);
    else
      strcpy(SftpConfig.ServerInfos[ID].password, SftpConfig.ServerInfos[ID].password_cached);
  }

  if (wcplg_sftp_connect
      (SftpConfig.ServerInfos[ID].username, SftpConfig.ServerInfos[ID].password, 
       SftpConfig.ServerInfos[ID].host, SftpConfig.ServerInfos[ID].port, 
       SftpConfig.ServerInfos, ID) != SFTP_SUCCESS) {
    SftpConfig.ServerInfos[ID].password_cached[0] = '\0';
    SftpConfig.ServerInfos[ID].host_cached[0] = 0;
    SftpConfig.ServerInfos[ID].username_cached[0] = 0;
    return SFTP_FAILED;
  }
  //OK connect done
  return SFTP_SUCCESS;
}

//---------------------------------------------------------------------

int get_sftpServer_ID_by_Path(char *Path)
{
  char *Path_ORG;
  char ServerTitle[MAX_Server_INFO];
  Path_ORG = Path;

  Path += 1;

  if (strchr(Path, '\\') != NULL) {
    // OK, it's a piece like this:
    // server n\foo\bla\etc
    // have 2 cut the rest after server n
    int count_;
    count_ = (int) (strchr(Path, '\\') - Path);
    strncpy(ServerTitle, Path, count_);
    ServerTitle[count_] = '\0';
  } else {
    // OK, it's a piece like this:
    // server n
    // we have the Server Title
    strcpy(ServerTitle, Path);
    //ServerTitle = Path;
  }

  Path = Path_ORG;              // Flush
  return get_sftpServer_ID_by_Title(ServerTitle);
}

//---------------------------------------------------------------------

int get_sftpServer_ID_by_Title(char *Title)
{
  int i;
  for (i = 0; i < SftpConfig.ServerCount; i++) {
    if (strcmp(SftpConfig.ServerInfos[i].title, Title) == 0) {
      return i;
    }
  }
  return -1;
}

//---------------------------------------------------------------------

bool any_connection_active()
{
  for (int i = 0; i < SftpConfig.ServerCount; i++) {
    if (SftpConfig.ServerInfos[i].username_cached[0])
      return true;
  }
  return false;
}

//---------------------------------------------------------------------

void LoadServers()
{
  int max_sections = MAX_Server_Count;
  int i;
  int ID = 0;
  int imported_num = 0;
  char buf_divers[4000];
  char section_name[MAX_Server_INFO];
  char buf_title[MAX_Server_INFO];
  char buf_host[MAX_Server_INFO];
  char buf_username[MAX_Server_INFO];
  char buf_password[4000];
  char buf_port[MAX_Server_INFO];
  char buf_home_dir[MAX_Server_INFO];
  char buf_compression[MAX_Server_INFO];
  char buf_use_key_auth[MAX_Server_INFO];
  char buf_dont_ask4_username[MAX_Server_INFO];
  char buf_dont_ask4_password[MAX_Server_INFO];
  char buf_keyfilename[MAX_Server_INFO];
  char buf_dont_ask4_passphrase[MAX_Server_INFO];
  char buf_proxy_type[MAX_Server_INFO];
  char buf_proxy_host[MAX_Server_INFO];
  char buf_proxy_port[MAX_Server_INFO];
  char buf_proxy_username[MAX_Server_INFO];
  char buf_proxy_password[MAX_Server_INFO];
  char buf_proxy_telnet_command[MAX_Server_INFO];
  char buf_chmod_value_put[MAX_Server_INFO];
  char buf_chmod_value_mkdir[MAX_Server_INFO];
  char buf_set_chmod_after_put[MAX_Server_INFO];
  char buf_set_chmod_after_mkdir[MAX_Server_INFO];
  char buf_set_mtime_after_put[MAX_Server_INFO];
  BOOL retry, load, new_passwd=false;
    
  char caption[4000];

  do {
    char buf_test[4000];
    retry = false;
    load = false;
    //Check for password crypter library
    GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_USE_PASSWORD_CRYPTER, "", buf_divers,
                          MAX_Server_INFO, SftpConfig.ConfigIniFile);
    GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_TEST_PASSWORD, "", buf_test,
                          4000, SftpConfig.ConfigIniFile);

    if (strlen(buf_divers)>0) {
      if (stricmp(buf_divers, SftpConfig.PasswordCrypterPath)!=0) {
        strncpy(SftpConfig.PasswordCrypterPath, buf_divers, MAX_Server_INFO);
        hPasswdCrypter = LoadLibrary(buf_divers);
        if (hPasswdCrypter) {
          new_passwd=false;
          hSetupPasswordCrypter = (tSetupPasswordCrypter)
            GetProcAddress(hPasswdCrypter, "SetupPasswordCrypter");
          SftpConfig.EncryptPassword = (tEncryptPassword)
            GetProcAddress(hPasswdCrypter, "EncryptPassword");
          SftpConfig.DecryptPassword = (tDecryptPassword)
            GetProcAddress(hPasswdCrypter, "DecryptPassword");
          if ((hSetupPasswordCrypter) && (SftpConfig.EncryptPassword) && 
            (SftpConfig.DecryptPassword)) {
            buf_divers[0]=0;
            //what to do, if user cancel this?
            if (strlen(buf_test)>0) 
              strcpy(caption, SimpleCaption);
            else {
              strcpy(caption, CaptionAskFirst);
              new_passwd = true;
            }
            load = RequestProc(PluginNumber, RT_Password, caption, 
              NULL, buf_divers, sizeof(buf_divers) - 1);
            if (load) {
              if (new_passwd) { 
                load = RequestProc(PluginNumber, RT_Password, strcpy(caption, CaptionAskSecond), 
                   NULL, buf_test, sizeof(buf_test) - 1);
                if (strcmp(buf_divers,buf_test)==0) {
                  strcpy(SftpConfig.PasswordCrypterPassword, buf_divers);
                  hSetupPasswordCrypter(buf_divers);
                } else {
                  retry = RequestProc(PluginNumber, RT_MsgYesNo, 
                    "Passwordcrypter's master-password verification failed!", "Retry?", NULL, 0);
                  load = false;
                  SftpConfig.PasswordCrypterPath[0] = 0;
                }
              }
            } else {
              SftpConfig.PasswordCrypterPath[0] = 0;
              SftpConfig.PasswordCrypterPassword[0] = 0;
            }
          } else {
            SftpConfig.PasswordCrypterPath[0] = 0;
            load = false;
          }

          if (load && (!new_passwd)) {
            char buf_test2[4000];

            memset(buf_test, 0, sizeof(buf_test));

            strncpy(SftpConfig.PasswordCrypterPassword, buf_divers, MAX_Server_INFO);
            hSetupPasswordCrypter(SftpConfig.PasswordCrypterPassword);

            if (strlen(buf_test)>0) {
              memset(buf_test2, 0, sizeof(buf_test2));

              SftpConfig.DecryptPassword(buf_test, buf_test2);
              load = strcmp(buf_test2, INI_CONFIG_TEST_PASSWORD_TEST)==0;
              if (!load) {
                retry = RequestProc(PluginNumber, RT_MsgYesNo, 
                  "Passwordcrypter's master-password verification failed!", "Retry?", NULL, 0);
                SftpConfig.PasswordCrypterPath[0] = 0;
                SftpConfig.PasswordCrypterPassword[0] = 0;
              }
            }
          }
          
          if (!load) {
            FreeLibrary(hPasswdCrypter);
            hPasswdCrypter=0;
            SftpConfig.EncryptPassword = 0;
            SftpConfig.DecryptPassword = 0;
          }
        }
      } else {
        if (!hPasswdCrypter)
          err_v("Passwordcrypter module failed to load!\n(%s)", SftpConfig.PasswordCrypterPath);
      }

      if (!hPasswdCrypter) {
        //SftpConfig.PasswordCrypterPath[0] = 0;
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
                            MAX_Server_INFO, SftpConfig.ConfigIniFile);
    GetPrivateProfileString(section_name, "host", "", buf_host,
                            MAX_Server_INFO, SftpConfig.ConfigIniFile);

    if (strlen(buf_host) && strlen(buf_title)) {
      GetPrivateProfileString(section_name, "username", "", buf_username,
                              MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "password", "", buf_password,
                              MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "port", "", buf_port,
                              MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "home_dir", "", buf_home_dir,
                              MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "compression", "",
                              buf_compression, MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "use_key_auth", "",
                              buf_use_key_auth, MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "keyfilename", "",
                              buf_keyfilename, MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "dont_ask4_passphrase", "",
                              buf_dont_ask4_passphrase, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "dont_ask4_password", "",
                              buf_dont_ask4_password, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "dont_ask4_username", "",
                              buf_dont_ask4_username, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_type", "",
                              buf_proxy_type, MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_host", "",
                              buf_proxy_host, MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_port", "",
                              buf_proxy_port, MAX_Server_INFO, SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_username", "",
                              buf_proxy_username, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_password", "",
                              buf_proxy_password, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "proxy_telnet_command", "",
                              buf_proxy_telnet_command, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "chmod_value_put", "",
                              buf_chmod_value_put, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "chmod_value_mkdir", "",
                              buf_chmod_value_mkdir, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "set_chmod_after_put", "",
                              buf_set_chmod_after_put, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "set_chmod_after_mkdir", "",
                              buf_set_chmod_after_mkdir, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      GetPrivateProfileString(section_name, "set_mtime_after_put", "",
                              buf_set_mtime_after_put, MAX_Server_INFO,
                              SftpConfig.ConfigIniFile);
      // Check title for / && 
      XconvertServerTitle(buf_title);

      // TODO check if title already exists, in such case we change it a little, e.g. append '[1]'

      strcpy(SftpConfig.ServerInfos[ID].title, buf_title);
      strcpy(SftpConfig.ServerInfos[ID].host, buf_host);
      strcpy(SftpConfig.ServerInfos[ID].host_cached, buf_host);
      strcpy(SftpConfig.ServerInfos[ID].username, buf_username);
      if ((hPasswdCrypter) && (!new_passwd) && (strlen(buf_password)>0)) {
        SftpConfig.DecryptPassword(buf_password, SftpConfig.ServerInfos[ID].password);
      } else {
        strncpy(SftpConfig.ServerInfos[ID].password, buf_password, 
          sizeof(SftpConfig.ServerInfos[ID].password));
      }
      strcpy(SftpConfig.ServerInfos[ID].keyfilename, buf_keyfilename);

      //use_key_auth optional, default=0
      if (strlen(buf_use_key_auth)) {
        SftpConfig.ServerInfos[ID].use_key_auth = 
          (unsigned char)strtoul(buf_use_key_auth, NULL, 10);
        if (SftpConfig.ServerInfos[ID].use_key_auth > 1)
          SftpConfig.ServerInfos[ID].use_key_auth = 1;
      } else {
        SftpConfig.ServerInfos[ID].use_key_auth = 0;
      }

      //dont_ask4_username optional, default=0
      if (strlen(buf_dont_ask4_username)) {
        SftpConfig.ServerInfos[ID].dont_ask4_username = 
          (unsigned char)strtoul(buf_dont_ask4_username, NULL, 10);
        if (SftpConfig.ServerInfos[ID].dont_ask4_username > 1)
          SftpConfig.ServerInfos[ID].dont_ask4_username = 1;
      } else {
        SftpConfig.ServerInfos[ID].dont_ask4_username = 0;
      }

      //dont_ask4_passphrase optional, default=0
      if (strlen(buf_dont_ask4_passphrase)) {
        SftpConfig.ServerInfos[ID].dont_ask4_passphrase =
          (unsigned char)strtoul(buf_dont_ask4_passphrase, NULL, 10);
        if (SftpConfig.ServerInfos[ID].dont_ask4_passphrase > 1)
          SftpConfig.ServerInfos[ID].dont_ask4_passphrase = 1;
      } else {
        SftpConfig.ServerInfos[ID].dont_ask4_passphrase = 0;
      }

      //dont_ask4_password optional, default=0
      if (strlen(buf_dont_ask4_password)) {
        SftpConfig.ServerInfos[ID].dont_ask4_password =
          (unsigned char)strtoul(buf_dont_ask4_password, NULL, 10);
        if (SftpConfig.ServerInfos[ID].dont_ask4_password > 1)
          SftpConfig.ServerInfos[ID].dont_ask4_password = 1;
      } else {
        SftpConfig.ServerInfos[ID].dont_ask4_password = 0;
      }

      //set_chmod_after_put optional, default=0
      if (strlen(buf_set_chmod_after_put)) {
        SftpConfig.ServerInfos[ID].set_chmod_after_put =
          (unsigned char)strtoul(buf_set_chmod_after_put, NULL, 10);
        if (SftpConfig.ServerInfos[ID].set_chmod_after_put > 1)
          SftpConfig.ServerInfos[ID].set_chmod_after_put = 1;
      } else {
        SftpConfig.ServerInfos[ID].set_chmod_after_put = 0;
      }

      //set_chmod_after_mkdir optional, default=0
      if (strlen(buf_set_chmod_after_mkdir)) {
        SftpConfig.ServerInfos[ID].set_chmod_after_mkdir =
          (unsigned char)strtoul(buf_set_chmod_after_mkdir, NULL, 10);
        if (SftpConfig.ServerInfos[ID].set_chmod_after_mkdir > 1)
          SftpConfig.ServerInfos[ID].set_chmod_after_mkdir = 1;
      } else {
        SftpConfig.ServerInfos[ID].set_chmod_after_mkdir = 0;
      }

      //set_mtime_after_put optional, default=1
      if (strlen(buf_set_mtime_after_put)) {
        SftpConfig.ServerInfos[ID].set_mtime_after_put =
          (unsigned char)strtoul(buf_set_mtime_after_put, NULL, 10);
        if (SftpConfig.ServerInfos[ID].set_mtime_after_put > 1)
          SftpConfig.ServerInfos[ID].set_mtime_after_put = 1;
      } else {
        SftpConfig.ServerInfos[ID].set_mtime_after_put = 1;
      }

      //chmod_value_put optional, default=700, valid only with set_chmod_after_put
      if (strlen(buf_chmod_value_put)) {
        SftpConfig.ServerInfos[ID].chmod_value_put =
          strtoul(buf_chmod_value_put, NULL, 10);
      } else {
        SftpConfig.ServerInfos[ID].chmod_value_put = 700;
      }

      //chmod_value_mkdir optional, default=700, valid only with set_chmod_after_mkdir
      if (strlen(buf_chmod_value_mkdir)) {
        SftpConfig.ServerInfos[ID].chmod_value_mkdir =
          strtoul(buf_chmod_value_mkdir, NULL, 10);
      } else {
        SftpConfig.ServerInfos[ID].chmod_value_mkdir = 700;
      }

      // port & home_dir are optional
      if (strlen(buf_port)) {
        SftpConfig.ServerInfos[ID].port = strtol(buf_port, NULL, 10);
      } else {
        SftpConfig.ServerInfos[ID].port = 22;
      }

      if (SftpConfig.ServerInfos[ID].port < 1 || SftpConfig.ServerInfos[ID].port > 65535) {
        SftpConfig.ServerInfos[ID].port = 22;
      }

      if (strlen(buf_compression) == 0
          || strcmp(buf_compression, "1") == 0) {
        SftpConfig.ServerInfos[ID].compression = 1;
      } else {
        SftpConfig.ServerInfos[ID].compression = 0;
      }

      if (strlen(buf_home_dir)) {
        strcpy(SftpConfig.ServerInfos[ID].home_dir, buf_home_dir);
      } else {
        strcpy(SftpConfig.ServerInfos[ID].home_dir, "");
      }

      SftpConfig.ServerInfos[ID].passphrase[0] = 0;
      SftpConfig.ServerInfos[ID].password_cached[0] = 0;
      SftpConfig.ServerInfos[ID].username_cached[0] = 0;
      SftpConfig.ServerInfos[ID].is_imported_from_any_datasrc = 0;
      SftpConfig.ServerInfos[ID].is_imported_from_putty_registry = 0;
      SftpConfig.ServerInfos[ID].is_imported_from_sshcom_registry = 0;

      //experimental: Proxy
      if (strlen(buf_proxy_type)) {
        unsigned long tmp = strtoul(buf_proxy_type, NULL, 10);
        switch (tmp) {
        case 1:
          SftpConfig.ServerInfos[ID].proxy_type = PROXY_SOCKS4;
          break;
        case 2:
          SftpConfig.ServerInfos[ID].proxy_type = PROXY_SOCKS5;
          break;
        default:
          SftpConfig.ServerInfos[ID].proxy_type = PROXY_NONE;
          break;
        }
      } else {
        SftpConfig.ServerInfos[ID].proxy_type = PROXY_NONE;
      }

      if (strlen(buf_proxy_host)) {
        strcpy(SftpConfig.ServerInfos[ID].proxy_host, buf_proxy_host);
      } else {
        strcpy(SftpConfig.ServerInfos[ID].proxy_host, "");
      }

      if (strlen(buf_proxy_port)) {
        SftpConfig.ServerInfos[ID].proxy_port = strtol(buf_proxy_port, NULL, 10);
      } else {
        SftpConfig.ServerInfos[ID].proxy_port = 1080;  //standard port for socks-proxy
      }

      if ((SftpConfig.ServerInfos[ID].proxy_port < 1) || 
          (SftpConfig.ServerInfos[ID].proxy_port > 65535)) {
        SftpConfig.ServerInfos[ID].proxy_port = 1080;  //standard port for socks-proxy
      }

      if (strlen(buf_proxy_username)) {
        strcpy(SftpConfig.ServerInfos[ID].proxy_username, buf_proxy_username);
      } else {
        strcpy(SftpConfig.ServerInfos[ID].proxy_username, "");
      }

      if (strlen(buf_proxy_password)) {
        strcpy(SftpConfig.ServerInfos[ID].proxy_password, buf_proxy_password);
      } else {
        strcpy(SftpConfig.ServerInfos[ID].proxy_password, "");
      }

      if (strlen(buf_proxy_telnet_command)) {
        strcpy(SftpConfig.ServerInfos[ID].proxy_telnet_command,
               buf_proxy_telnet_command);
      } else {
        strcpy(SftpConfig.ServerInfos[ID].proxy_telnet_command, "");
      }

      //this line checks if we get a really supported proxy version - the other stuff
      //is not really supported right now...
      if ((SftpConfig.ServerInfos[ID].proxy_type != PROXY_SOCKS4)
          && (SftpConfig.ServerInfos[ID].proxy_type != PROXY_SOCKS5))
        SftpConfig.ServerInfos[ID].proxy_type = PROXY_NONE;

      SftpConfig.ServerInfos[ID].id = ID;
      ID++;
    } else if (i)
      break;                    // hey dude - this is a badness, too :-(
  }

  // okay, we are outta here  

  // Cache directories?
  strcpy(buf_divers, "");
  GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_CACHE_FS, "", buf_divers,
                          MAX_Server_INFO, SftpConfig.ConfigIniFile);
  if (strlen(buf_divers) && (buf_divers[0]=='1'))
  {
    SftpConfig.CacheFS = 1;
  } else {
    SftpConfig.CacheFS = 0;
  }

  // DO Putty Import ?
  strcpy(buf_divers, "");
  GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_IMPORT_PUTTY_SSH_SESS, "", buf_divers,
                          MAX_Server_INFO, SftpConfig.ConfigIniFile);
  if (strlen(buf_divers) && (buf_divers[0] == '1' || buf_divers[0] == '2')) {
    //import the putty saved session if...
    imported_num = 0;
    imported_num += ImportPuttySessions(ID, buf_divers);
    SftpConfig.ImportedSessions = imported_num;
    ID += imported_num;
  }
  if (strlen(buf_divers)>0)
    SftpConfig.DoImportPuttySessions = buf_divers[0];
  else
    SftpConfig.DoImportPuttySessions = '0';

  // DO ssh.com Import ?
  strcpy(buf_divers, "");
  GetPrivateProfileString(INI_CONFIG_SECTION_NAME,
                          INI_CONFIG_IMPORT_SSHCOM_SSH_SESS, "",
                          buf_divers, MAX_Server_INFO, SftpConfig.ConfigIniFile);
  if (strlen(buf_divers)) {
    char dir_location[MAX_CMD_BUFFER];

    strcpy(dir_location, buf_divers);
    //import the ssh.com saved session if...
    if (strlen(dir_location) > 0 && strcmp(dir_location, "0") != 0) {
      imported_num = 0;
      imported_num = ImportSSHcomSessions(ID, dir_location);
      SftpConfig.ImportedSessions += imported_num;
      ID += imported_num;
    }
  }
  strncpy(SftpConfig.DoImportSSHcomSessions, buf_divers, MAX_Server_INFO);

  strcpy(SftpConfig.ServerInfos[ID].title, QUICK_CONNECTION);
  //Set defaults 4 quick connection
  SftpConfig.ServerInfos[ID].compression = 0;
  SftpConfig.ServerInfos[ID].dont_ask4_username = 0;
  SftpConfig.ServerInfos[ID].dont_ask4_passphrase = 0;
  SftpConfig.ServerInfos[ID].dont_ask4_password = 0;
  SftpConfig.ServerInfos[ID].home_dir[0] = '\0';
  SftpConfig.ServerInfos[ID].host[0] = '\0';
  SftpConfig.ServerInfos[ID].host_cached[0] = '\0';
  SftpConfig.ServerInfos[ID].id = ID;
  //is imported, as we don't want to save it again
  SftpConfig.ServerInfos[ID].is_imported_from_any_datasrc = 1;
  SftpConfig.ServerInfos[ID].is_imported_from_putty_registry = 0;
  SftpConfig.ServerInfos[ID].is_imported_from_sshcom_registry = 0;
  SftpConfig.ServerInfos[ID].keyfilename[0] = '\0';
  SftpConfig.ServerInfos[ID].passphrase[0] = '\0';
  SftpConfig.ServerInfos[ID].proxy_host[0] = '\0';
  SftpConfig.ServerInfos[ID].proxy_password[0] = '\0';
  SftpConfig.ServerInfos[ID].proxy_port = 0;
  SftpConfig.ServerInfos[ID].proxy_telnet_command[0] = '\0';
  SftpConfig.ServerInfos[ID].proxy_type = PROXY_NONE;
  SftpConfig.ServerInfos[ID].proxy_username[0] = '\0';
  SftpConfig.ServerInfos[ID].password[0] = '\0';
  SftpConfig.ServerInfos[ID].password_cached[0] = '\0';
  SftpConfig.ServerInfos[ID].port = 22;
  SftpConfig.ServerInfos[ID].use_key_auth = 0;
  SftpConfig.ServerInfos[ID].username[0] = '\0';
  SftpConfig.ServerInfos[ID].username_cached[0] = '\0';
  SftpConfig.ServerInfos[ID].chmod_value_put = 0;
  SftpConfig.ServerInfos[ID].chmod_value_mkdir = 0;
  SftpConfig.ServerInfos[ID].set_chmod_after_put = 0;
  SftpConfig.ServerInfos[ID].set_chmod_after_mkdir = 0;
  SftpConfig.ServerInfos[ID].set_mtime_after_put = 0;

  ID++;                         // !!!

  MakeServerTitlesUnique(ID);

  SftpConfig.ServerCount = ID;

  if (new_passwd) SaveProperties(&SftpConfig);
}

//---------------------------------------------------------------------

void LogProc_(int MsgType, char *LogString)
{
  if (&LogProc && do_logging())
    LogProc(PluginNumber, MsgType, LogString);
}

//---------------------------------------------------------------------

BOOL __stdcall FsDisconnect(char *DisconnectRoot)
{
  UNUSED_ARG(DisconnectRoot)
  //DisconnectRoot is unusable as only 1 connection is active, because of psftp's architecture
  //LogProc_(MSGTYPE_DISCONNECT,"DISCONNECTED");
  sftp_disconnect(CurrentServer_ID, false);
  ResetAlreadyConnected();
  return true;
}

//---------------------------------------------------------------------

//this checks for a connection. we could use this multi dll concept to 
//run multiple connections simultanously
void check_Concurrent_Connection(char *Path)
{
  //CurrentServer_ID is global
  if (CurrentServer_ID == NO_SERVER_ID)   //Is there already a connection to a Server ?
  {
    CurrentServer_ID = get_sftpServer_ID_by_Path(Path);
  } else {
    int server_ID = get_sftpServer_ID_by_Path(Path);
    if (server_ID != CurrentServer_ID) {
      //ooops, conflict!
      //we have to disconnect the connection to avoid the conflict
      if (IsAlreadyConnected())
        sftp_disconnect(CurrentServer_ID, false);
      CurrentServer_ID = server_ID; // OK, conflict fixed :-)
    }
  }
}

//---------------------------------------------------------------------

int get_current_server_id()
{
  return CurrentServer_ID;
}

//---------------------------------------------------------------------

struct SftpServerAccountInfo *GetServerInfos(void)
{
  return SftpConfig.ServerInfos;
}

//---------------------------------------------------------------------

BOOL Request_Password(char *buf)
{
  return RequestProc(PluginNumber, RT_Password, "Secure FTP", "Password", buf, MAX_PATH);
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

tProgressProc get_ProgressProc()
{
  return ProgressProc;
}
//---------------------------------------------------------------------

int get_PluginNumber()
{
  return PluginNumber;
}
//---------------------------------------------------------------------

int file_exists_on_remote_server(char *RemoteFile)
{
  if (get_sftpServer_ID_by_Path(RemoteFile) == -1)
    return FS_FILE_NOTSUPPORTED;

  char Path[MAX_CMD_BUFFER];
  char File_[MAX_CMD_BUFFER];

  strcpy(Path, RemoteFile);
  if (get_basename_from_Path(Path, RemoteFile) != 1) {
    dbg("file_exists_on_remote_server: fix me, path!");
    return FS_FILE_NOTSUPPORTED;
  }

  RemoteFile += (strlen(Path) + 1);
  strcpy(File_, RemoteFile);
  RemoteFile -= (strlen(Path) + 1);

  check_Concurrent_Connection(RemoteFile);
  WIN32_FIND_DATA FindData;
  HANDLE lf;

  lf = FsFindFirst(Path, &FindData);

  if (lf == INVALID_HANDLE_VALUE) {
    FsFindClose(lf);
    return FS_FILE_NOTFOUND;
  }

  pLastFindStuct lf_res;
  lf_res = (pLastFindStuct) lf;
  int found = 0;
  if (lf_res->CurrentDirStruct != NULL) {
    for (int i = 0; i < lf_res->CurrentDirStruct[0].nnames; i++) {
      if (strcmp(lf_res->CurrentDirStruct->names[i].filename, File_) == 0) {
        found = 1;
        break;
      }
    }
  }

  FsFindClose(lf);
  if (found == 1)
    return FS_FILE_EXISTS;

  return FS_FILE_NOTFOUND;
}

//---------------------------------------------------------------------

int get_basename_from_Path(char *buf, char *Path)
{
  size_t i;
  size_t dir_end = 0;

  i = strlen(Path) - 1;
  for (; i > 0; i--) {
    if (Path[i] == '\\') {
      dir_end = i;
      break;
    }
  }

  if (dir_end == 0)
    return 0;                   // FIX ME MORE!!!

  for (i = 0; i < (dir_end); i++) {
    buf[i] = Path[i];
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
  accept_refresh = false;
  switch (InfoOperation) {
    case FS_STATUS_OP_DELETE:
      {
        if (strcmp(RemoteDir, "\\") == 0) // Deleting connection!
        {
          delete_only_connection = (InfoStartEnd == FS_STATUS_START);
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
        accept_refresh = true;
      }
  }
}

//---------------------------------------------------------------------

int Risdir(char *Path)
{
  char sftp_cmd[MAX_CMD_BUFFER];
  int Server_ID;

  Server_ID = get_sftpServer_ID_by_Path(Path);
  if (Server_ID == -1)
    return 0;

  strcpy(sftp_cmd, "ls \"./");

  Path += (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);
  strcat(sftp_cmd, Path);
  Path -= (1 + strlen(SftpConfig.ServerInfos[CurrentServer_ID].title) + 1);

  strcat(sftp_cmd, "\"");

  winSlash2unix(sftp_cmd);

  //DISABLE_LOGGING();  
  if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, Server_ID) !=
      SFTP_SUCCESS) {
    //ENABLE_LOGGING();
    return 0;
  }
  //ENABLE_LOGGING();

  return 1;
}

//---------------------------------------------------------------------

bool SetFileTime__(char *fullFilePath, FILETIME * LastWriteTime)
{
  bool f;
  HANDLE myFile;
  if (LastWriteTime == NULL)
    return false;
  myFile = CreateFile(fullFilePath, // file name
                      GENERIC_WRITE,  // access mode
                      FILE_SHARE_READ,  // share mode
                      NULL,     // security descriptor
                      OPEN_EXISTING,  // how to create
                      FILE_ATTRIBUTE_NORMAL | // file attributes
                      FILE_FLAG_SEQUENTIAL_SCAN, NULL); // handle to template file

  if (myFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  f = SetFileTime(myFile,       // sets last-write time for file
                  LastWriteTime, (LPFILETIME) NULL, LastWriteTime) == TRUE;

  CloseHandle(myFile);
  return f;
}


//---------------------------------------------------------------------

//replace / and \ with _ (in title)
void XconvertServerTitle(char *title)
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
  struct enumsettings *ret;
  HKEY key;

  if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &key) != ERROR_SUCCESS) {
    return NULL;
  }

  ret = (struct enumsettings *) malloc(sizeof(*ret));

  if (ret) {
    ret->key = key;
    ret->i = 0;
  }
  return ret;
}
//---------------------------------------------------------------------

char *enum_settings_next(void *handle, char *buffer, int buflen)
{
  struct enumsettings *e = (struct enumsettings *) handle;
  char *otherbuf;

  otherbuf = (char *) malloc(3 * buflen);
  if (otherbuf
      && RegEnumKey(e->key, e->i++, otherbuf,
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
int ImportPuttySessions(int lastInsert_ID, char *import_mode)
{
  static char otherbuf[2048];
  int count = 0;
  char *ret;
  void *handle;
  struct enumsettings *handleFree;

  if ((handle = enum_settings_start())) {
    handleFree = (struct enumsettings *) handle;
    do {
      if ((lastInsert_ID + count) >= MAX_Server_Count - 2) {
        RegCloseKey(handleFree->key);
        return count;
      }
      ret = enum_settings_next(handle, otherbuf, sizeof(otherbuf));

      if (ret) {
        struct SftpServerAccountInfo ServerAccountInfo;
        if (ImportPuttySession(&ServerAccountInfo, otherbuf) == 1) {
          // OK success
          XconvertServerTitle(otherbuf);
          strcpy(ServerAccountInfo.title, otherbuf);
          ServerAccountInfo.id = (lastInsert_ID + count);
          SftpConfig.ServerInfos[lastInsert_ID + count] = ServerAccountInfo;
          if (import_mode[0] == '2') {
            //User want no password promt, all the connection are using key auth trought pageant.exe
            SftpConfig.ServerInfos[lastInsert_ID + count].use_key_auth = 1;
          }
          count++;
        }
      }
    } while (ret);
    RegCloseKey(handleFree->key);
    free(handle);
  }
  return count;
}

//---------------------------------------------------------------------

void ServerAccountInfoDefaults(struct SftpServerAccountInfo
                               *ServerAccountInfo)
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

int ImportPuttySession(struct SftpServerAccountInfo *ServerAccountInfo, char *PuttySectionName)
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
              MAX_Server_INFO);
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

int ImportSSHcomSessions(int lastInsert_ID, char *dir_location)
{
  //int dwIndex=1;
  int count = 0;
  unsigned int Fnum = 0;
  char buf[MAX_Server_INFO];    //MAX_Server_INFO soll MAXcmdbuffer sein      
  char buf2[MAX_Server_INFO];   //MAX_Server_INFO soll MAXcmdbuffer sein     
  char buf3[MAX_Server_INFO];   //MAX_Server_INFO soll MAXcmdbuffer sein     
  char buf4[MAX_Server_INFO];   //MAX_Server_INFO soll MAXcmdbuffer sein     
  char buf_divers[MAX_Server_INFO]; //MAX_Server_INFO soll MAXcmdbuffer sein     
  char dir_location_PATH[MAX_Server_INFO];  //MAX_Server_INFO soll MAXcmdbuffer sein      

  bool Next;                    // Done searching for files?
  HANDLE FndHnd = NULL;         // Handle to find data.
  WIN32_FIND_DATA FindDat;      // Info on file found.

  if (strlen(dir_location) > 0
      && dir_location[strlen(dir_location) - 1] == '\\') {
    dir_location[strlen(dir_location) - 1] = '\0';
  }
  strcpy(dir_location_PATH, dir_location);
  strcat(dir_location, "\\*.ssh2");
  FndHnd = FindFirstFile(dir_location, &FindDat);

  while (FndHnd != INVALID_HANDLE_VALUE) {
    Next = FindNextFile(FndHnd, &FindDat) == TRUE;

    if (!Next) {
      break;
    } else {
      char hostname[MAX_Server_INFO];
      char title[MAX_Server_INFO];
      char username[MAX_Server_INFO];
      char home_dir[MAX_Server_INFO];
      int compression = 0;
      int port = 22;
      hostname[0] = '\0';
      title[0] = '\0';
      username[0] = '\0';
      home_dir[0] = '\0';

      Fnum++;
      sprintf(buf, "%d", Fnum);
      strcpy(buf2, "File");
      strcat(buf2, buf);
      buf3[0] = '\0';

      if ((lastInsert_ID + count) >= MAX_Server_Count - 2) {
        if (FndHnd != INVALID_HANDLE_VALUE) // If there was anything found, then
          FindClose(FndHnd);    // Close the find handle
        return count;
        break;
      }

      strcpy(buf3, FindDat.cFileName);
      strcpy(buf4, dir_location_PATH);
      strcat(buf4, "\\");
      strcat(buf4, buf3);

      if (strlen(buf3)) {
        GetPrivateProfileString("Connection", "Host Name", "", buf_divers,
                                MAX_Server_INFO, buf4);
        if (strlen(buf_divers) > 2) {
          //title
          get_basename_from_Path(title, buf3);
          strcpy(title, (buf3 + (strlen(title))));
          XconvertServerTitle(title);

          //hostname
          strcpy(hostname, 2 + buf_divers);

          //username
          GetPrivateProfileString("Connection", "User Name", "",
                                  buf_divers, MAX_Server_INFO, buf3);
          if (strlen(buf_divers) > 2)
            strcpy(username, 2 + buf_divers);

          //Compression
          GetPrivateProfileString("Connection", "Compression", "0",
                                  buf_divers, MAX_Server_INFO, buf3);
          if (strlen(buf_divers) > 2
              && strcmp(buf_divers + 2, "zlib") == 0) {
            compression = 1;
          } else {
            compression = 0;
          }

          //Port
          GetPrivateProfileString("Connection", "Port", "22", buf_divers,
                                  MAX_Server_INFO, buf3);
          if (strlen(buf_divers) > 2) {
            port = atoi(2 + buf_divers);
          } else {
            port = 22;
          }
          if (!port || port <= 0)
            port = 22;

          //home dir :-)
          GetPrivateProfileString("Connection", "Default SFTP Directory",
                                  "", buf_divers, MAX_Server_INFO, buf3);
          if (strlen(buf_divers) > 2) {
            strcpy(home_dir, 2 + buf_divers);
          }
          //inherited char*
          strcpy(SftpConfig.ServerInfos[lastInsert_ID + count].title, title);
          strcpy(SftpConfig.ServerInfos[lastInsert_ID + count].host, hostname);
          strcpy(SftpConfig.ServerInfos[lastInsert_ID + count].host_cached, hostname);
          strcpy(SftpConfig.ServerInfos[lastInsert_ID + count].username, username);
          strcpy(SftpConfig.ServerInfos[lastInsert_ID + count].username_cached, ""); // MUSS strlen=0 sein  wegen anyconnectionactive()
          strcpy(SftpConfig.ServerInfos[lastInsert_ID + count].home_dir, home_dir);

          // inherited int
          SftpConfig.ServerInfos[lastInsert_ID + count].compression = compression;
          SftpConfig.ServerInfos[lastInsert_ID + count].port = port;

          // Static
          SftpConfig.ServerInfos[lastInsert_ID + count].use_key_auth = 0;

          if (strlen(SftpConfig.ServerInfos[lastInsert_ID + count].username)) {
            SftpConfig.ServerInfos[lastInsert_ID + count].dont_ask4_username = 1;
          } else {
            SftpConfig.ServerInfos[lastInsert_ID + count].dont_ask4_username = 0;
          }

          strcpy(SftpConfig.ServerInfos[lastInsert_ID + count].password, "");
          strcpy(SftpConfig.ServerInfos[lastInsert_ID + count].password_cached, "");

          SftpConfig.ServerInfos[lastInsert_ID + count].is_imported_from_any_datasrc =
            1;
          SftpConfig.ServerInfos[lastInsert_ID +
                    count].is_imported_from_putty_registry = 0;
          SftpConfig.ServerInfos[lastInsert_ID +
                    count].is_imported_from_sshcom_registry = 1;

          SftpConfig.ServerInfos[lastInsert_ID + count].id = lastInsert_ID + count;

          count++;              //last comand!!!
        }
      }
    }
  }
  if (FndHnd != INVALID_HANDLE_VALUE) // If there was anything found, then
    FindClose(FndHnd);          // Close the find handle

  return count;
}
