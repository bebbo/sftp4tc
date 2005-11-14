#ifndef sftpmap_h
#define sftpmap_h

#define SFTP_SUCCESS 1
#define SFTP_FAILED 0
#define SFTP_DISCONNECTED -1
#define MAX_CMD_BUFFER 1024
#define MAX_MSG_BUFFER 2048

#define HANDLE__SHOW_SFTP_SERVER 1
#define HANDLE__SHOW_SFTP_DIR    2

//Links in the first level of plugin's fs
//
#define DefineEditConnection_define "<Edit connections>"
#define DefineEditConnection_selected "\\<Edit connections>"

#define DefineAddConnection_define "<Add connections>"
#define DefineAddConnection_selected "\\<Add connections>"

#define QUICK_CONNECTION "<Quick connection>"

void LogProc_(int MsgType, char *LogString);
ProgressProcType get_ProgressProc();
int get_PluginNumber();

struct SftpServerAccountInfo *GetServerInfos(void);
int Connect(char *user, char *password, char *host, int port, 
            SftpServerAccountInfo * allServers, int CurrentServerId);
int wcplg_sftp_connect_byID(int id);
void SetTransferMode(int id, char *mode);
int Disconnect(int id, bool nolog);
int get_current_server_id(void);
int ExecuteCommand(char *command, char *serverOutput, int serverId);
struct fxp_names* GetCurrentDirectoryStruct(int serverId);
void GetLastPsftpError(int id, char *buf);

void __stdcall dbg(char *msg);

typedef struct {
  unsigned long hi, lo;
} uint64, int64;

struct fxp_attrs {
  unsigned long flags;
  uint64 size;
  unsigned long uid;
  unsigned long gid;
  unsigned long permissions;
  unsigned long atime;
  unsigned long mtime;
};

struct fxp_name {
  char *filename, *longname;
  struct fxp_attrs attrs;
};

struct fxp_names {
  int nnames;
  struct fxp_name *names;
};

struct my_fxp_names {
  int nnames;
  struct fxp_name **names;
};

typedef int (__stdcall CALLBACK * PsftpConnectProcType) (char *, char *, char *, int);
typedef int (__stdcall CALLBACK * PsftpDoSftpProcType) (char *, char *);
typedef my_fxp_names *(__stdcall CALLBACK * PsftpGetCurrentDirStructProcType) (void);
typedef char *(__stdcall CALLBACK * PsftpGetLastErrorMessageProcType) (void);
typedef int (__stdcall CALLBACK * PsftpInitProgressProcProcType) 
    (ProgressProcType AP_ProgressProc, int Awc_PluginNr);
typedef void (__stdcall CALLBACK * PsftpSetSftpServerAccountInfoProcType)
    (SftpServerAccountInfo ServerAccountInfo);
typedef void (CALLBACK * PsftpSetTransferModeProcType)(char *mode);
typedef int (CALLBACK * PsftpDisconnectedProcType)(void);

void Unload_PSFTP_DLL_HANDLER(int ServerId);
void *__wcplg_sftp_get_current_dir_struct(void);
void convert_slash_windows_to_unix(char* string);
void convert_slash_unix_to_windows(char* string);
int InitProgressProc(ProgressProcType AP_ProgressProc, int Awc_PluginNr,
                      int CurrentServerId);
int psftp_memory_hole__stopfen(int ID);
void InitPsftpWrappers(void);
void GetPsftpDllPath(char *buf);
int FileCopy(char *src, char *dest);
void UnlinkTemporaryDllFile(int id);
void UnlinkAllTemporaryDllFiles(void);
int FileExists(char* fname);
int Risdir(char *Path);

bool IsAlreadyConnected();
void ResetAlreadyConnected();
void trim_host_from_hoststring(char *hoststring);
void trim_port_from_hoststring(char *hoststring);

void DisableLoggingOnce();
int DoLogging();

#endif //sftpmap_h