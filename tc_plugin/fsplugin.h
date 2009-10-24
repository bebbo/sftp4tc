#ifndef fsplugin_h
#define fsplugin_h

#ifndef __BSTRING_H__
#include <bstring.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
// contents of fsplugin.h

// ids for FsGetFile
#define FS_FILE_OK 0
#define FS_FILE_EXISTS 1
#define FS_FILE_NOTFOUND 2
#define FS_FILE_READERROR 3
#define FS_FILE_WRITEERROR 4
#define FS_FILE_USERABORT 5
#define FS_FILE_NOTSUPPORTED 6
#define FS_FILE_EXISTSRESUMEALLOWED 7

#define FS_EXEC_OK 0
#define FS_EXEC_ERROR 1
#define FS_EXEC_YOURSELF -1
#define FS_EXEC_SYMLINK -2

#define FS_COPYFLAGS_OVERWRITE 1
#define FS_COPYFLAGS_RESUME 2
#define FS_COPYFLAGS_MOVE 4
#define FS_COPYFLAGS_EXISTS_SAMECASE 8
#define FS_COPYFLAGS_EXISTS_DIFFERENTCASE 16

// flags for tRequestProc
#define RT_Other 0
#define RT_UserName 1
#define RT_Password 2
#define RT_Account 3
#define RT_UserNameFirewall 4
#define RT_PasswordFirewall 5
#define RT_TargetDir 6
#define RT_URL 7
#define RT_MsgOK 8
#define RT_MsgYesNo 9
#define RT_MsgOKCancel 10

// flags for tLogProc
#define MSGTYPE_CONNECT 1
#define MSGTYPE_DISCONNECT 2
#define MSGTYPE_DETAILS 3
#define MSGTYPE_TRANSFERCOMPLETE 4
#define MSGTYPE_CONNECTCOMPLETE 5
#define MSGTYPE_IMPORTANTERROR 6
#define MSGTYPE_OPERATIONCOMPLETE 7

// flags for FsStatusInfo
#define FS_STATUS_START 0
#define FS_STATUS_END 1

#define FS_STATUS_OP_LIST 1
#define FS_STATUS_OP_GET_SINGLE 2
#define FS_STATUS_OP_GET_MULTI 3
#define FS_STATUS_OP_PUT_SINGLE 4
#define FS_STATUS_OP_PUT_MULTI 5
#define FS_STATUS_OP_RENMOV_SINGLE 6
#define FS_STATUS_OP_RENMOV_MULTI 7
#define FS_STATUS_OP_DELETE 8
#define FS_STATUS_OP_ATTRIB 9
#define FS_STATUS_OP_MKDIR 10
#define FS_STATUS_OP_EXEC 11
#define FS_STATUS_OP_CALCSIZE 12

#define FS_ICONFLAG_SMALL 1
#define FS_ICONFLAG_BACKGROUND 2

#define FS_ICON_USEDEFAULT 0
#define FS_ICON_EXTRACTED 1
#define FS_ICON_EXTRACTED_DESTROY 2
#define FS_ICON_DELAYED 3

typedef struct {
  DWORD SizeLow, SizeHigh;
  FILETIME LastWriteTime;
  int Attr;
} RemoteInfoStruct;

// callback functions
typedef int (__stdcall * ProgressProcType) (int PluginNr, bchar *SourceName,
                                         bchar *TargetName,
                                         int PercentDone);
typedef void (__stdcall * LogProcType) (int PluginNr, int MsgType,
                                     bchar const *LogString);
typedef BOOL(__stdcall * RequestProcType) (int PluginNr, int RequestType,
                                        bchar *CustomTitle,
                                        bchar *CustomText,
                                        bchar *ReturnedText, int maxlen);

// Function prototypes
int __stdcall FsInit(int PluginNr, ProgressProcType pProgressProc,
                     LogProcType pLogProc, RequestProcType pRequestProc);
HANDLE __stdcall FsFindFirst(bchar *Path, WIN32_FIND_DATA * FindData);
BOOL __stdcall FsFindNext(HANDLE Hdl, WIN32_FIND_DATA * FindData);
int __stdcall FsFindClose(HANDLE Hdl);
BOOL __stdcall FsMkDir(bchar *Path);
int __stdcall FsExecuteFile(HWND MainWin, bchar *RemoteName, bchar *Verb);
int __stdcall FsRenMovFile(bchar *OldName, bchar *NewName, BOOL Move,
                           BOOL OverWrite, RemoteInfoStruct * ri);
int __stdcall FsGetFile(bchar *RemoteName, bchar *LocalName, int CopyFlags,
                        RemoteInfoStruct * ri);
int __stdcall FsPutFile(bchar *LocalName, bchar *RemoteName, int CopyFlags);
BOOL __stdcall FsDeleteFile(bchar *RemoteName);
BOOL __stdcall FsRemoveDir(bchar *RemoteName);
BOOL __stdcall FsDisconnect(bchar *DisconnectRoot);
BOOL __stdcall FsSetAttr(bchar *RemoteName, int NewAttr);
BOOL __stdcall FsSetTime(bchar *RemoteName, FILETIME * CreationTime,
                         FILETIME * LastAccessTime,
                         FILETIME * LastWriteTime);
void __stdcall FsStatusInfo(bchar *RemoteDir, int InfoStartEnd,
                            int InfoOperation);
void __stdcall FsGetDefRootName(bchar *DefRootName, int maxlen);
int __stdcall FsExtractCustomIcon(bchar* RemoteName,int ExtractFlags,HICON* TheIcon);

extern int gPluginNumber;
extern ProgressProcType gProgressProc;
extern LogProcType gLogProc;
extern RequestProcType gRequestProc;

#ifdef __cplusplus
}
#endif

#endif //fsplugin_h
