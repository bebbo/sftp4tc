#define SFTP_SUCCESS 1
#define SFTP_FAILED 0
#define MAX_CMD_BUFFER 1024
#define MAX_Server 500

#define HANDLE__SHOW_SFTP_SERVER 1
#define HANDLE__SHOW_SFTP_DIR    2 

void LogProc_(int MsgType,char* LogString);
tProgressProc get_ProgressProc();
int get_PluginNumber();

#define QUICK_CONNECTION "Quick connection"

struct SftpServerAccountInfo *getP_allServer(void);
int wcplg_sftp_connect(char *user, char *password, char *host, int port, SftpServerAccountInfo *allServers, int CurrentServerId);
int wcplg_sftp_connect_byID(unsigned int id); 
int wcplg_sftp_disconnect(int id, bool nolog);
int get_current_server_id(void);
int wcplg_sftp_do_commando(char *commando, char *server_output, int CurrentServerId);
struct fxp_names *wcplg_sftp_get_current_dir_struct(int ID);
void wcplg_sftp_getLastError(int id, char *buf);

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

void Unload_PSFTP_DLL_HANDLER(int ServerId);
void *__wcplg_sftp_get_current_dir_struct(void);
void winSlash2unix(char *s);
int init_ProgressProc(tProgressProc AP_ProgressProc, int Awc_PluginNr, int CurrentServerId);
int psftp_memory_hole__stopfen(int ID);
void init_server_dll_handlers(void);
void get_psftpDll_path(char *buf);
int fileCopy(char *src, char *dest);
void unlink_dll_tmp_file(int id);
void unlink_ALL_dll_tmp_files(void);
int file_exists(char *fname);
int Risdir(char *Path);