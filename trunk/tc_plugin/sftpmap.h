#ifndef sftpmap_h
#define sftpmap_h

#include <sftp.h>
#include <fsplugin.h>

typedef Sftp4tc * (__stdcall CALLBACK * PsftpConnectProcType) (char const * const, char const * const, char const *const, int);
typedef int (__stdcall CALLBACK * PsftpDoSftpProcType) (char const * const, char *);
typedef my_fxp_names *(__stdcall CALLBACK * PsftpGetCurrentDirStructProcType) (void);
typedef void (__stdcall CALLBACK * PsftpFreeCurrentDirStructProcType) (void);
typedef char *(__stdcall CALLBACK * PsftpGetLastErrorMessageProcType) (void);
typedef int (__stdcall CALLBACK * PsftpInitProcsProcType) 
    (RequestProcType AP_RequestProc, ProgressProcType AP_ProgressProc, int Awc_PluginNr);
typedef int (CALLBACK * PsftpDisconnectedProcType)(void);
typedef Sftp4tc * (CALLBACK * PsftpDoConfigType)(HWND, int, int);
typedef fxp_attrs * (CALLBACK * PsftpGetLastAttrType)(void);
typedef void (CALLBACK * PsftpSetTransferAscii)(int);

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


	std::string dllName;

	PsftpMapper(std::string const &);
	~PsftpMapper();

private:
	void cleanup();
};

#endif //sftpmap_h