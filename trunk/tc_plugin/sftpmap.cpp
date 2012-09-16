#include "stdafx.h"
#include "sftpmap.h"
#include "puttyver.h"

extern "C" {
#include "../shared/sftp4tc_share.h"
}

#define MAX_CMD_BUFFER 1024

//---------------------------------------------------------------------
extern int gPluginNumber;
extern ProgressProcType gProgressProc;
extern LogProcType gLogProc;
extern RequestProcType gRequestProc;

extern HMODULE ghThisDllModule;
extern HWND gMainWin;

//---------------------------------------------------------------------
// cheat the progressProc with remote -> remote copies
bool firstHalf;
bool secondHalf;

// keep the last progress proc result
int lastPPR;

//---------------------------------------------------------------------
// show some progress
static int __stdcall myProgressProc(int PluginNr, bchar *SourceName, bchar *TargetName, int PercentDone) {
  if (firstHalf || secondHalf) {
    PercentDone >>= 1;
    if (secondHalf)
      PercentDone += 50;
  }

  lastPPR = gProgressProc(PluginNr, SourceName, TargetName, PercentDone);

  // DBGPRINT(("gProgressProc: %d\r\n", r));

  if (lastPPR)
    return lastPPR;

  MSG msg;
  for (int i = 0; i < 42; ++i) {
    if (!PeekMessage(&msg, 0, 0, 0, 0))
      break;

    if (GetMessage(&msg, 0, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return gProgressProc(PluginNr, SourceName, TargetName, PercentDone);
}

//---------------------------------------------------------------------
// get the psftp.dll path
static void GetPsftpDllPath(bchar* buf) {
  bchar plugin_dir[MAX_CMD_BUFFER];
  GetModuleFileName(ghThisDllModule, plugin_dir, MAX_CMD_BUFFER - 10);

  bstrcpy(buf, plugin_dir);

  bchar *p = bstrrchr(buf, TEXT('\\'));
  if (!p)
    p = buf;

  bstrcpy(p, TEXT("\\psftp.dll"));
}

//---------------------------------------------------------------------
// check file existance
static int FileExists(bchar const * const fname) {
  HANDLE hf = CreateFile(fname, 0, 0, 0, OPEN_EXISTING, 0, 0);
  int exists = hf != INVALID_HANDLE_VALUE;
  if (exists)
    CloseHandle(hf);

  return exists;
}

//---------------------------------------------------------------------
// DT - cleanup
PsftpMapper::~PsftpMapper() {
  cleanup();
}

//---------------------------------------------------------------------
// the cleanup method
void PsftpMapper::cleanup() {
  if (hDll) {
    if (disconnect)
      disconnect();
    FreeLibrary( hDll);
    hDll = 0;
  }

  if (dllName.size() > 0) {
    DeleteFile(dllName.c_str());
    dllName = TEXT("");
  }
}

//---------------------------------------------------------------------
// CT - create a new mapper
PsftpMapper::PsftpMapper(bstring const & serverName, config_tag * cfg) :
  hDll(0) {
  bchar tempPathBuffer[MAX_CMD_BUFFER];
  bchar dll_2_copy[MAX_CMD_BUFFER];
  GetTempPath(MAX_CMD_BUFFER, tempPathBuffer);

  GetPsftpDllPath(dll_2_copy);
  if (!FileExists(dll_2_copy)) {
    MessageBox(gMainWin, TEXT("psftp.dll not found"), dll_2_copy, MB_OK);
    return;
  }

  bstring usedName = serverName;
  bstring dll_2_load;
  bchar no[32] = { 0 };
  for (int n = 0; n < 999; ++n) {
    dll_2_load = tempPathBuffer;
    dll_2_load += TEXT("psftp_") + usedName + no + TEXT(".dll");
    if (CopyFile(dll_2_copy, dll_2_load.c_str(), 0))
      break;
    bsprintf(no, TEXT("%d"), ++n);
    usedName = TEXT("_~_");
  }

#ifdef _DEBUGX
  hDll = LoadLibrary(dll_2_copy);
#else
  hDll = LoadLibrary(dll_2_load.c_str());
#endif

  if (!hDll) {
    MessageBox(gMainWin, TEXT("instanced psftp.dll not found"), dll_2_load.c_str(), MB_OK);
    return;
  }

  dllName = dll_2_load;

  this->connect = (PsftpConnectProcType) GetProcAddress(hDll, "__map__wcplg_open_sftp_session");
  this->disconnect = GetProcAddress(hDll, "__map__wcplg_close_sftp_session");
  this->doSftp = (PsftpDoSftpProcType) GetProcAddress(hDll, "__map__wcplg_do_sftp");
  this->getCurrentDirStruct = (PsftpGetCurrentDirStructProcType) GetProcAddress(hDll,
      "__map__wcplg_get_current_dir_struct");
  this->freeCurrentDirStruct = (PsftpFreeCurrentDirStructProcType) GetProcAddress(hDll,
      "__map__wcplg_free_current_dir_struct");
  this->getLastErrorMessage
      = (PsftpGetLastErrorMessageProcType) GetProcAddress(hDll, "__map__wcplg_get_last_error_msg");
  this->initProcs = (PsftpInitProcsProcType) GetProcAddress(hDll, "__map__init_Procs");
  this->disconnected = (PsftpDisconnectedProcType) GetProcAddress(hDll, "__map__disconnected");
  // new
  this->doConfig = (PsftpDoConfigType) GetProcAddress(hDll, "__map__do_config");
  this->getLastAttr = (PsftpGetLastAttrType) GetProcAddress(hDll, "__map__get_last_attr");
  this->setTransferAscii = (PsftpSetTransferAscii) GetProcAddress(hDll, "__map__set_transfer_ascii");
  this->setConfig = (PsftpSetConfig) GetProcAddress(hDll, "__map__set_config");
  this->loadConfig = (PsftpLoadConfig) GetProcAddress(hDll, "__map__load_config");

  // enum settings
  this->enumSettingsClose = (PsftpEnumSettingsClose) GetProcAddress(hDll, "__map__enum_settings_close");
  this->enumSettingsNext = (PsftpEnumSettingsNext) GetProcAddress(hDll, "__map__enum_settings_next");
  this->enumSettingsStart = (PsftpEnumSettingsStart) GetProcAddress(hDll, "__map__enum_settings_start");

  // version
  this->getVersion = (PsftpGetVersion) GetProcAddress(hDll, "__map__get_version");

  if (this->connect == NULL || this->disconnect == NULL || this->doSftp == NULL || this->getCurrentDirStruct == NULL
      || this->getLastErrorMessage == NULL || this->initProcs == NULL || this->disconnected == NULL || this->doConfig
      == NULL || this->getLastAttr == NULL || this->setTransferAscii == NULL || this->setConfig == NULL
      || this->loadConfig == NULL || this->enumSettingsClose == NULL || this->enumSettingsNext == NULL
      || this->enumSettingsStart == NULL || this->getVersion == NULL
      || strcmp(this->getVersion(), PUTTY_VERSION_STRING) != 0) {
    char buff[256];
    if (this->getVersion)
      sprintf(buff, "expected version %s got version %s", PUTTY_VERSION_STRING, this->getVersion());
    else
      sprintf(buff, "wrong psftp.dll");
    ::MessageBoxA(gMainWin, buff, "SFTP4TC", MB_OK);
    cleanup();
    return;
  }

  /*
   DLL-loading and functionsimport successed,
   now try to connect to server
   */
#ifdef UNICODE0
  this->initProcs((RequestProcType)reqProc, (ProgressProcType)progProc, gPluginNumber, gMainWin);
#else
  this->initProcs(gRequestProc, myProgressProc, gPluginNumber, gMainWin);
#endif
  if (cfg)
    this->setConfig(cfg);
}

//replace / and \ with _ (in title)
static void force_valid_server_title(char *title) {
  if (!title)
    return;

  while (*title) {
    if ((*title == '/') || (*title == '\\')) {
      *title = '_';
    }
    ++title;
  }
}

// Session importing
void PsftpMapper::import_putty_sessions(std::vector<ServerInfo> & serverInfos) {
  static char otherbuf[2048];
  char *ret;
  EnumSettingsType * handle;

  if ((handle = enumSettingsStart())) {

    EnumSettingsType * handleFree = handle;
    do {
      ret = enumSettingsNext(handle, otherbuf, sizeof(otherbuf));

      if (ret) {
        {
          // OK success
          force_valid_server_title(otherbuf);
#ifdef UNICODE
          wchar_t * srvName;
          BCONVERT(wchar_t, 1024, srvName, ret);
          serverInfos.push_back(srvName);
#else
          serverInfos.push_back(ret);
#endif
        }
      }
    } while (ret);
    enumSettingsClose(handleFree);
  }
}
