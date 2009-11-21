#ifndef sftpmap_h
#define sftpmap_h

#include <putty.h>
#include <bstring.h>
#include <fsplugin.h>

#include <string>
#include <vector>

#ifdef UNICODE
typedef std::wstring bstring;
#else
typedef std::string bstring;
#endif

typedef bstring ServerInfo;

typedef config_tag * (__stdcall CALLBACK * PsftpConnectProcType) (char const * const, char const * const, char const *const, int);
// this one is ok to use bchar - psftp.dll side handles conversion.
typedef int (__stdcall CALLBACK * PsftpDoSftpProcType) (bchar const * const, bchar *);
typedef my_fxp_names *(__stdcall CALLBACK * PsftpGetCurrentDirStructProcType) (void);
typedef void (__stdcall CALLBACK * PsftpFreeCurrentDirStructProcType) (void);
typedef char *(__stdcall CALLBACK * PsftpGetLastErrorMessageProcType) (void);
typedef int (__stdcall CALLBACK * PsftpInitProcsProcType)
(RequestProcType AP_RequestProc, ProgressProcType AP_ProgressProc, int Awc_PluginNr, HWND hwnd);
typedef int (CALLBACK * PsftpDisconnectedProcType)(void);
typedef config_tag * (CALLBACK * PsftpDoConfigType)(HWND, int, int);
typedef fxp_attrs * (CALLBACK * PsftpGetLastAttrType)(void);
typedef void (CALLBACK * PsftpSetTransferAscii)(int);
typedef void (CALLBACK * PsftpSetConfig)(config_tag *);
typedef void (CALLBACK * PsftpLoadConfig)(char const * name, config_tag *);

// enumerate settings
typedef void * EnumSettingsType;
typedef char * (CALLBACK * PsftpEnumSettingsNext)(EnumSettingsType * handle, char *buffer, int buflen);
typedef EnumSettingsType * (CALLBACK * PsftpEnumSettingsStart)(void);
typedef void (CALLBACK * PsftpEnumSettingsClose)(EnumSettingsType *);

typedef char * (CALLBACK * PsftpGetVersion)(void);

struct PsftpMapper {
    HMODULE hDll;
    PsftpConnectProcType connect;
    PsftpDoSftpProcType doSftp;
    PsftpGetCurrentDirStructProcType getCurrentDirStruct;
    PsftpFreeCurrentDirStructProcType freeCurrentDirStruct;
    FARPROC disconnect;
    PsftpGetLastErrorMessageProcType getLastErrorMessage;
    PsftpInitProcsProcType initProcs;
    PsftpDisconnectedProcType disconnected;
    PsftpDoConfigType doConfig;
    PsftpGetLastAttrType getLastAttr;
    PsftpSetTransferAscii setTransferAscii;
    PsftpSetConfig setConfig;
    PsftpLoadConfig loadConfig;

    PsftpEnumSettingsStart enumSettingsStart;
    PsftpEnumSettingsNext enumSettingsNext;
    PsftpEnumSettingsClose enumSettingsClose;

    PsftpGetVersion getVersion;

    bstring dllName;

    PsftpMapper(bstring const &, config_tag * = 0);
    ~PsftpMapper();

    void import_putty_sessions(std::vector<ServerInfo> & serverInfos);

  private:
    void cleanup();
};

#endif //sftpmap_h
