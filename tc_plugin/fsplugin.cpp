/*
VERSION 1.1.55.2
[21.9. 2004]
+FIXED copying DLL without admin rights

VERSION 1.1.55.1 
[2.9. 2004]
/UPDATED TO PuTTY 0.55 (first beta)

VERSION 1.09	[24. Jan 2004]
+FIXED "quote cd" (Leszek Skorczynski <skorczyl@wp.pl>)
*ADDED Version number (MK)

VERSION 1.08b   [10.Mar.2003]
+FIXED sftp cd & sftp ls & "quote root"
*ADDED some New Connection Dlgs 

VERSION 1.08a	[28.Feb.2003]
+FIXED Connect/Disconnect design is a little bit wrong, so now happen no troubles anymore, but it's not the best solution
+FIXED permission denied/remote disconnect(idle,...) problem solved

VERSION 1.08	[21.Feb.2003]
?TODO ask Christian about MSGTYPEs and DISCONNECTION - something wrong there...
?TODO ask him about ability to force other path after connection
?TODO transfermodes are notified through FsExecute, but how to use it?
?TODO maybe it doesn't work under w98, maybe it just misses some libraries
*ADDED quote root <directory>
*ADDED reconnection ability

VERSION 1.07d  [19.Feb.2003]
/CHANGED UnixTimeToLocalTime to M$ compatible mode

VERSION 1.07b  [07.Feb.2003]
?TODO check FsExecuteFile
?TODO check Import-routines for Key-Auth import
?TODO check the Plugin-start (only performance fixed)
+FIXED FsInit performance
*ADDED Support for OPENSSH (and possibly for sshcom) private keys.
*ADDED support for SOCKS-PROXY v5
/CHANGED cleaned Code

VERSION 1.06   [17.Jan.2003]
?TODO  workaround for proxy support
?TODO  workaround for text-mode transfer
?TODO  setting up a cvs
?TODO  account for martin
?TODO  creating of licence - must be compatible with PUTTY (i would like something like apache/bsd)
*ADDED ini parameter "keyfilename" to override defaults for the host  (putty session framework), tested just with a key without passphrase and in putty format (.PPK)
+FIXED plugin shows the whole directory list now
+FIXED it should work with SSH 2.0(OpenSSH 3.4p1)   -   checked on FreeBSD 4.7
/CHANGED shared.h and shared.c moved to a separate shared folder "shared" - now ther's just one copy of that file (and the Server struct)!!!
/CHANGED Ported to latest PuttySource (0.53b)

VERSION 1.05
+FIXED Y2K Problem in UnixTimeToLocalTime() changed 		if (st.wYear<30  || (st.wYear<200 && st.wYear>=100)) to if ((st.wYear>=30 && st.wYear<=99) || (st.wYear<200 && st.wYear>=100)) 
+FIXED root commando works good now
*ADDED import session from putty && ssh.com (optional)
*ADDED Host Key Warnigs
*ADDED custom ini path
+Small cosmetics

VERSION 1.04   [12.Nov.2002]
*ADDED - optional: use_key_auth and dont_ask4_username as config flags in wcx_sftp.ini, possible values are 0 or 1,  default = 0

VERSION 1.03   [11.Nov.2002]
/CHANGED - MAX_Server=500 (preview was 100 only)

VERSION 1.02
+FIXED Server Title can not contain / and \     will replaced with _
?TODO wcplg_sftp_disconnect wenn connect nicht success "verbindung trennen" noch da

VERSION 1.01
+FIXED *.dll 2 *.wfx

VERSION 1.00   [09.10.2002 Berlin - Hans]
?TODO RequestProc mit RT_MsgYesNo Funzt nicht, gibt immer FALSE zurück
*ADDED Add Connection (Port syntax Host[:Port])


*************************************************************************************************
OLD LOG:
*************************************************************************************************

31.8.2002 Christian Ghisler
FIXED - make work with plugin interface changes
FIXED - cleanup code
ADDED - FsGetDefRootName
ADDED - pseudo-file to open ini for editing
ADDED - handle 'quote' command for command line, currently only for 'cd' command to set new root, e.g.  cd /

26.03.2002 Berlin, Hans <hans.petrich@tronic-media.com>
TODO's
FIXED - time atri wrong
FIXED - dir size wrong
 - escaping in cmd string for " \ * and like this 
FIXED - handle symbolic links on sftp server ?
FIXED - Correct Error handling
FIXED - disconnect while an open sftp session handling
FIXED  - double window in wc makes problem i.e left side is plugin and right side too
FIXED - progresbar ?!?!?!
FIXED  - fileoverwrite file del etc. ask or not ?? what to do ???
FIXED - rmdir error "!"
 - ask username and password, HOW, how 2 store password ???
 - form dialog for edit wcx_sftp.ini
FIXED  - where i am *.dll ?!?!
FIXED  - change file atri 
FIXED  - what todo when 0 servers in *.ini
 - title in allServers: no duplicates 
 - psftp are slow ?? it's not cpu and not network traffic
FIXED  - psftp have memory leak, have to fix this perhaps in psftp.c or sftp.c 
 - creating README & INSTALL 
FIXED  - How 2 disconnect a sftp session
FIXED  - other place for wcx_sftp.ini & psftp.dll ???
 - parse the cmd 4 unsecure chars !!!!
 - crypt password in wcx_sftp.ini.ini ?!?!? or disable password storing
FIXED  - add quick connection like ftp quick connection
NONE - scp support ?!?!
 - background download support via psftp.exe or scp (via createProcess or system() or like this) batchfile
FIXED - Private Key Authentikaction support ??? psftp doesn't support it realy, FIX ME anytime
BOGUS - define pluginroot in wcx_sftp.ini ???
 - socks support, anytime ?!??!?
 - connection timeout ?!? what 2do ?
FIXED - does psftp support compressing ???
FIXED  - tmp file in wcplg_sftp_do_commando, FIX ME P-L-E-A-S-E 
 - cleaning the code !!!
FIXED - hostkey abfrage, TESTEN !!!
 - cmd box API function ???
FIXED  - Folder permission check bevor chdir FIX ME PLZ
FIXED  - in psftp.dll stdout umleiten zu char *logBuf und dann an LogProc() weiterleiten
 - if psftp.dll connect 2 an open NON ssh [22] port, psftp hangs forever
 - get_sftpServer_ID_by_Path(): title nach sonderzeichen parsen und rausschneiden, doppelte title einträge vermeiden!
FIXED - "Bookmark" funst ned, müsste aba an WC selbst geändert werden, have 2 talk with Ghisler
 - WC merkt sich nicht die "aktuelle Position" wenn man z.B.: von plugion nach C: und dann wieda zu Plugin geht, have 2 talk with Ghisler
 - ftp <-> plugin filetransfer funst ned, API!, have 2 talk with Ghisler
FIXED - !!!alle delete aktionen  deaktivieren wenn der user nicht in seinem ensprechendem sftp dir drinn ist!!! 
 - wie komm ich an das windowscommander tmp $wc ran ???
FIXED - progressbar noch abstimmen
 - fsrenmv(): fehler abfangen wenn user im serverRoot folder moven oder renamen will
FIXED  - Verbinfungsleiste "oben ", geht manchmal ned wech obwohl disconnect gedrückt) API error ?  have 2 talk with Ghisler
FIXED  - Manchmal Segfault error wenn in dir wechseln will wo keine berechtigung 
FIXED  - SetLastError(ERROR_ACCESS_DENIED) in fsFindFirst !!! wechselt nach C zurück ?!?!?
FIXED  - Nach chmod: dir refresh zeigt aktuelle werte nicht an obwohl auf server gesetzt, cached by WC ?!
FIXED - DWORD    dwFileAttributes=FILE_ATTRIBUTE_REPARSE_POINT; wc sollte link als symbol auch anzeigen
FIXED - API fehler, wenn man die Filelisten sortiert und dann fasfifirst called
FIXED - Drag & Drop funst irgendwie ned
FIXED - API Bug FsPutFile() gibt als LocalFile noch ''' hinzu ?!?!?
- Testing 
*/

#include "stdafx.h"
#include "fsplugin.h"
#include "sftpmap.h"
#include <direct.h>
#include <stdio.h>
#include "putty_proxy.h"

extern "C"
{
	#include "share.h"
}

#define GHISLER_TC_REGP	"Software\\Ghisler\\Total Commander"
#define GHISLER_TC_REGP_SFTP_K	"SFtpIniName"

#define PETRICH_TC_REGP	"Software\\petrich\\tc_sftp_plugin"
#define PETRICH_TC_REGP_SFTP_K	"SFtpIniName"

// Session importing
#define PUTTY_REG_POS "Software\\SimonTatham\\PuTTY"
#define PUTTY_REG_PARENT "Software\\SimonTatham"
#define PUTTY_REG_PARENT_CHILD "PuTTY"
#define PUTTY_REG_GPARENT "Software"
#define PUTTY_REG_GPARENT_CHILD "SimonTatham"

static const char *const puttystr = PUTTY_REG_POS "\\Sessions"; 
static char hex[17] = "0123456789ABCDEF";  //Intel Compiler said, it's 17 Bytes long. :-)

struct enumsettings {
    HKEY key;
    int i;
};

void *open_settings_r(char *sessionname);
static void mungestr(char *in, char *out);
static void gpps(void *handle, char *name, char *def, char *val, int len);
static void gppi(void *handle, char *name, int def, int *i);
char *read_setting_s(void *handle, char *key, char *buffer, int buflen);
int read_setting_i(void *handle, char *key, int defvalue);
char *enum_settings_next(void *handle, char *buffer, int buflen);
static void unmungestr(char *in, char *out, int outlen);
void *enum_settings_start(void);
int loadPuttySectionToSftpPluginServerInfoStruct(struct SftpServerAccountInfo *ServerAccountInfo,char *PuttySectionName);
int do_import_putty_saved_session_to_plugin_session(int current_ID,char *import_mode);
int do_import_sshcom_saved_session_to_plugin_session(int lastInsert_ID, char *dir_location);

struct SftpServerAccountInfo allServer[MAX_Server];

#define S_IFLNK 0x0A000
#define DefIniName "wcx_sftp.ini"

HMODULE hDllModule;

// Custom
#define DefineNewConnection "Edit connections.lnk"
#define DefineNewConnection2 "\\Edit connections.lnk"

#define DefineAddConnection "Add connections.lnk"
#define DefineAddConnection2 "\\Add connections.lnk"

int octal_perm_2_wc_int(unsigned long octal_val);
int wcplg_sftp_connect_byID(unsigned int id);
void __stdcall dbg(char *msg);
unsigned int get_IDbyPath(char *path);

int get_sftpServer_ID_by_Title(char *Title);
int wcplg_sftp_do_commando_byID(char *sftp_cmd, char *serverOutput, int ID);

bool any_connection_active();
int init_servers_from_iniFile();
int get_sftpServer_ID_by_Path(char *Path);
void check_Concurrent_Connection(char *Path);
int file_exists_on_remote_server(char *RemoteFile);
int get_basename_from_Path(char *buf,char *Path);
void free_CurrentDirStruct(fxp_names *P_DirStruct, int ID);

BOOL SetFileTime__(char *fullFilePath,FILETIME *LastWriteTime);
void XconvertServerTitle(char *title);

char iniFname[MAX_PATH];
BOOL delete_only_connection=FALSE;

int Imported_ids_num=0;
#define INI_CONFIG__SECTION_NAME "config"
#define INI_CONFIG_IMPORT_PUTTY_SSH_SESS "import_putty_ssh_sessions"
#define INI_CONFIG_IMPORT_SSHCOM_SSH_SESS "import_sshcom_ssh_sessions"
int is_title_in_all_server_double(int numServer, int currentServerId, char *title);

int get_custom_users_sftp_inifile_from_reg(char *iniFname);
int PluginNumber;
tProgressProc ProgressProc;
tLogProc LogProc;
tRequestProc RequestProc;

char* strlcpy(char* p,char*p2,int maxlen)
{
	if ((int)strlen(p2)>=maxlen)
	{
		strncpy(p,p2,maxlen);
		p[maxlen]=0;
	} else
		strcpy(p,p2);
	return p;
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			if (get_custom_users_sftp_inifile_from_reg(iniFname) == -1)
			{
				hDllModule=(HMODULE)hModule;
				GetModuleFileName(hDllModule,iniFname,sizeof(iniFname)-1);
				char* p=strrchr(iniFname,'\\');
				if (p)
					p++;
				else
					p=iniFname;
				strcpy(p,DefIniName);
			}
			break;
    }
	return TRUE;
}

typedef struct {
	char Path[MAX_PATH];
	char LastFoundName[MAX_PATH];
	HANDLE searchhandle;
	int sumIndex;
	int currentIndex;
	int SearchMode;
	fxp_names *CurrentDirStruct;
} tLastFindStuct,*pLastFindStuct;

int Num_allServer=0;
int CurrentServer_ID=-1;
void make_unique_title_in_allServers(int numServer);
extern bool already_connected;

int __stdcall FsInit(int PluginNr,tProgressProc pProgressProc,tLogProc pLogProc,tRequestProc pRequestProc)
{
	ProgressProc=pProgressProc;
    LogProc=pLogProc;
    RequestProc=pRequestProc;
	PluginNumber=PluginNr;

	// init alle Server
	Num_allServer	=	init_servers_from_iniFile();
	init_server_dll_handlers();
	unlink_ALL_dll_tmp_files();
	already_connected = false;
	return 0;
}

BOOL UnixTimeToLocalTime(long* mtime,LPFILETIME ft)
{
	struct tm* fttm=localtime(mtime);
	SYSTEMTIME st;
	FILETIME ft2;

	st.wYear=fttm->tm_year+1900;

	/*
	// M$ Help says something different :-(
	if ((st.wYear>=30 && st.wYear<=99) || (st.wYear<200 && st.wYear>=100))
		st.wYear+=1900;
	else if (st.wYear<100)
		st.wYear+=2000;
	*/
	st.wMonth=fttm->tm_mon+1;  // 0-11 in struct tm*
	st.wDay=fttm->tm_mday;
	st.wHour=fttm->tm_hour;
	st.wMinute=fttm->tm_min;
	st.wSecond=fttm->tm_sec;
	st.wDayOfWeek=0;
	st.wMilliseconds=0;
	if (SystemTimeToFileTime(&st,&ft2)) {
		return LocalFileTimeToFileTime(&ft2,ft);  // Wincmd expects system time!
	} else
		return false;
}

HANDLE __stdcall FsFindFirst(char* Path,WIN32_FIND_DATA *FindData)
{
	// user is trying to delete a connection!
	if (delete_only_connection) {
		SetLastError(ERROR_NO_MORE_FILES);
		return INVALID_HANDLE_VALUE;
	}
	
	pLastFindStuct lf;
	lf=(pLastFindStuct)malloc(sizeof(tLastFindStuct));
	lf->CurrentDirStruct=NULL;

	memset(FindData,0,sizeof(WIN32_FIND_DATA));
	
	if (strcmp(Path,"\\")==0) 
	{		
		// Connection selected
		if (!any_connection_active())  //Load Servers if there is no active connection
		{
			Num_allServer = init_servers_from_iniFile();
		}

		lf->currentIndex=0;
		lf->sumIndex=Num_allServer+1;
		strcpy(FindData->cFileName,allServer[lf->currentIndex].title);

		FindData->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
		FindData->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF;
		FindData->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE;
		lf->searchhandle=INVALID_HANDLE_VALUE;
		lf->SearchMode=HANDLE__SHOW_SFTP_SERVER;
		return (HANDLE)lf;
	} else {
		// So we have a connection - list selected directory
		lf->SearchMode=HANDLE__SHOW_SFTP_DIR;
		// Um welchen Server handelt es sich
		check_Concurrent_Connection(Path);

		CurrentServer_ID = get_sftpServer_ID_by_Path(Path); 
		if (CurrentServer_ID == -1)
		{
			dbg("fsfindfirst: CurrentServer_ID == -1, FIX ME!");
			SetLastError(ERROR_INVALID_ACCESS);
			return INVALID_HANDLE_VALUE;
		}
		
		char sftp_cmd[MAX_CMD_BUFFER];
		char *lPath = Path;

		strcpy(lf->Path,Path);
		lPath+=(1+strlen(allServer[CurrentServer_ID].title));
		
		if (strcmp(lPath,"")==0) sprintf(sftp_cmd,"ls");
		else sprintf(sftp_cmd,"ls \".%s\"",lPath);
		winSlash2unix(sftp_cmd);

		if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) != SFTP_SUCCESS)
		{
			LogProc_(MSGTYPE_CONNECTCOMPLETE,"Access denied!");
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
		
		fxp_names *CurrentDirStruct;
		CurrentDirStruct=wcplg_sftp_get_current_dir_struct(CurrentServer_ID);

		if (CurrentDirStruct == NULL)
		{
			//this should come if you were disconnected. just a badness for 
			//reconnection :-) maybe we should read the ini for the right password :-) 
			//or better don't erase it if it was in .ini 
			char buf_log[MAX_CMD_BUFFER];
			wcplg_sftp_getLastError(CurrentServer_ID,buf_log);
			if (strcmp(buf_log,UNEXPECTED_OK_MSG)==0) {
			  sprintf(buf_log,"permission denied");
			  LogProc_(MSGTYPE_DETAILS,buf_log);
  			  SetLastError(ERROR_INVALID_ACCESS);
			  return (HANDLE)INVALID_HANDLE_VALUE;
			} else {
  			   wcplg_sftp_disconnect(CurrentServer_ID, false);
			   return FsFindFirst(Path, FindData);
			}

		}

		lf->CurrentDirStruct = CurrentDirStruct;
		lf->sumIndex = CurrentDirStruct->nnames;
		lf->currentIndex = 0; 

		while ((lf->currentIndex<lf->sumIndex) && 
			((strcmp(lf->CurrentDirStruct->names[lf->currentIndex].filename,".")==0) || 
			 (strcmp(lf->CurrentDirStruct->names[lf->currentIndex].filename,"..")==0))) (lf->currentIndex)++;

		if (lf->currentIndex>=lf->CurrentDirStruct[0].nnames)
		{
			LogProc_(MSGTYPE_DETAILS,"Access denied(*)!");
			SetLastError(ERROR_NO_MORE_FILES);
			free_CurrentDirStruct(CurrentDirStruct,CurrentServer_ID);
			return INVALID_HANDLE_VALUE;
		}

		char FileTyp;
		FileTyp	= lf->CurrentDirStruct->names[lf->currentIndex].longname[0];
		FindData->dwReserved0=octal_perm_2_wc_int(lf->CurrentDirStruct->names[lf->currentIndex].attrs.permissions);

		if (FileTyp == 'd')
		{
			FindData->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY | 0x80000000;
		}
		else if (FileTyp == 'l')
		{
			FindData->dwFileAttributes=FILE_ATTRIBUTE_REPARSE_POINT| 0x80000000;
			FindData->dwReserved0+=S_IFLNK;    // Wincmd uses only this one!
		}
		else
		{
			FindData->dwFileAttributes=FILE_ATTRIBUTE_NORMAL | 0x80000000;
		}

		if (!UnixTimeToLocalTime((long*)&lf->CurrentDirStruct->names[lf->currentIndex].attrs.mtime,
					&FindData->ftLastWriteTime)) {
			FindData->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF;
			FindData->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE;
		}

		FindData->nFileSizeHigh=(DWORD)lf->CurrentDirStruct->names[lf->currentIndex].attrs.size.hi;
		FindData->nFileSizeLow=(DWORD)lf->CurrentDirStruct->names[lf->currentIndex].attrs.size.lo;
		strcpy(FindData->cFileName,CurrentDirStruct->names[lf->currentIndex].filename);
		
		return (HANDLE)lf;
	}
	return INVALID_HANDLE_VALUE;
}

BOOL __stdcall FsFindNext(HANDLE Hdl,WIN32_FIND_DATA *FindData)
{
	pLastFindStuct lf;
	lf=(pLastFindStuct)Hdl;

	if (lf->SearchMode==HANDLE__SHOW_SFTP_SERVER)
	{
		if (lf->currentIndex >= lf->sumIndex) return false; // Der letzte Eintrag wurde erreicht
	
		if (lf->currentIndex == lf->sumIndex-1) 
		{
			lf->currentIndex++;
			strcpy(FindData->cFileName,DefineNewConnection);
			FindData->dwFileAttributes=0;
			FindData->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF;
			FindData->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE;
		} else {
			if (lf->currentIndex == lf->sumIndex-2)
			{
				lf->currentIndex++;
				strcpy(FindData->cFileName,DefineAddConnection);
				FindData->dwFileAttributes=0;
				FindData->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF;
				FindData->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE;
			} else {
				lf->currentIndex++;
				strcpy(FindData->cFileName,allServer[lf->currentIndex].title);
		
				FindData->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
				FindData->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF;
				FindData->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE;
			}
		}
		return true;
	}

	if (lf->SearchMode==HANDLE__SHOW_SFTP_DIR)
	{
		lf->currentIndex++;
		while ((lf->currentIndex<lf->sumIndex) && 
			   ((strcmp(lf->CurrentDirStruct->names[lf->currentIndex].filename,".")==0) || 
			    (strcmp(lf->CurrentDirStruct->names[lf->currentIndex].filename,"..")==0))) (lf->currentIndex)++;

		if (lf->currentIndex >= lf->sumIndex) return false; // Der letzte Eintrag wurde erreicht
		strcpy(FindData->cFileName,lf->CurrentDirStruct->names[lf->currentIndex].filename);

		FindData->dwReserved0=octal_perm_2_wc_int(lf->CurrentDirStruct->names[lf->currentIndex].attrs.permissions);

		char FileTyp;
		FileTyp = lf->CurrentDirStruct->names[lf->currentIndex].longname[0];
		if (FileTyp == 'd')				//directory
		{
			FindData->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY | 0x80000000;
		}
		else if (FileTyp == 'l')		//link
		{
			FindData->dwFileAttributes=FILE_ATTRIBUTE_REPARSE_POINT| 0x80000000;
			FindData->dwReserved0+=S_IFLNK;    // Wincmd uses only this one!
		}
		else							//anything else :-)
		{
			FindData->dwFileAttributes=FILE_ATTRIBUTE_NORMAL  | 0x80000000;
		}

		if (!UnixTimeToLocalTime((long*)&lf->CurrentDirStruct->names[lf->currentIndex].attrs.mtime,
				&FindData->ftLastWriteTime)) 
		{
			FindData->ftLastWriteTime.dwHighDateTime=0xFFFFFFFF;
			FindData->ftLastWriteTime.dwLowDateTime=0xFFFFFFFE;
		}

		FindData->nFileSizeHigh=(DWORD)lf->CurrentDirStruct->names[lf->currentIndex].attrs.size.hi;
		FindData->nFileSizeLow=(DWORD)lf->CurrentDirStruct->names[lf->currentIndex].attrs.size.lo;
	
		return true;
	}

	dbg("FIX ME 667");
	return false;
}

int __stdcall FsFindClose(HANDLE Hdl)
{
	pLastFindStuct lf;
	lf=(pLastFindStuct)Hdl;

	if ( lf != INVALID_HANDLE_VALUE)
	{
		free_CurrentDirStruct(lf->CurrentDirStruct,CurrentServer_ID);
		free(lf);//jetzt dürft alles clean sein 
	}
	
	return 0;
}

void free_CurrentDirStruct(fxp_names *P_DirStruct, int ServerID)
{
	if (P_DirStruct != NULL)
	{
		for(int i=0;i<P_DirStruct->nnames;i++)
		{
			free(P_DirStruct->names[i].filename);
			free(P_DirStruct->names[i].longname);
		}
		free(P_DirStruct);
	}

	int n;
	n = psftp_memory_hole__stopfen(ServerID);//zwengs memory bug in psftp.c's sftp_cmd_ls()	
}

BOOL addnewserver(char* servertitle)
{
	char host[MAX_PATH], user[MAX_PATH], pwd[MAX_PATH], port[MAX_PATH];
	char home_dir[MAX_PATH];
	char section_name[MAX_Server_INFO];
	char servertitle_[MAX_Server_INFO];
	BOOL wantcompression;

	servertitle_[0]='\0';

	/*
	if (any_connection_active()) {
		RequestProc(PluginNumber,RT_MsgOK,"New Connection","Please close all open connections first!",NULL,0);
		return false;
	}
	*/

	host[0] = 0;
	user[0] = 0;
	pwd[0]  = 0;
	home_dir[0] = '/';
	home_dir[1] = 0;
	
	if (servertitle == NULL)
	{
		while(strlen(servertitle_)<=0)
		{
			if (!RequestProc(PluginNumber,RT_Other,"Server Title","Server Title:",servertitle_,sizeof(servertitle_)-1))
				return false;
		}
	} else {
		strcpy(servertitle_,servertitle);
	}	

	XconvertServerTitle(servertitle);
	XconvertServerTitle(servertitle_);//Beide um nachträgliche fehler zu vermeiden
	
	if (!RequestProc(PluginNumber,RT_Other,"New Connection","Host[:port]",host,sizeof(host)-1))
		return false;
	
	strcpy(port,host);
	trim_host_from_hoststring(host);
	trim_port_from_hoststring(port);
	if (!strlen(port)) strcpy(port,"22");

	if (!RequestProc(PluginNumber,RT_UserName, "New Connection",NULL,user,sizeof(user)-1))
		return false;

	if (!RequestProc(PluginNumber,RT_Password, "New Connection",NULL,pwd,sizeof(user)-1))
		return false;

	if (!RequestProc(PluginNumber,RT_TargetDir, "New Connection",NULL,home_dir,sizeof(user)-1))
		return false;

	// RequestProc mit RT_MsgYesNo Funzt nicht, gibt immer FALSE zurück

	wantcompression=RequestProc(PluginNumber,RT_MsgOKCancel, "Add connection - compression",
		"Do you want to compress the data connection (only recommended for slow connections)?",NULL,0);

	sprintf(section_name,"%i",Num_allServer-Imported_ids_num);
	WritePrivateProfileString (section_name , "title",					servertitle_,	iniFname); 
	WritePrivateProfileString (section_name , "host",					host,			iniFname); 
	WritePrivateProfileString (section_name , "username",				user,			iniFname); 
	WritePrivateProfileString (section_name , "password",				pwd,			iniFname); 
	WritePrivateProfileString (section_name , "port",					port,			iniFname); 
	WritePrivateProfileString (section_name , "home_dir",				home_dir,		iniFname); 
	WritePrivateProfileString (section_name , "compression",
										wantcompression ?				"1" : "0",		iniFname);
	WritePrivateProfileString (section_name , "use_key_auth",			"0",			iniFname);
	WritePrivateProfileString (section_name , "keyfilename",			"",				iniFname);
	WritePrivateProfileString (section_name , "dont_ask4_passphrase",	"0",			iniFname);
	WritePrivateProfileString (section_name , "dont_ask4_username",		
										strcmp(user,"")==0 ?			"0" : "1", 		iniFname);
	WritePrivateProfileString (section_name , "dont_ask4_password",		
										strcmp(pwd,"")==0 ?				"0" : "1", 		iniFname);
	WritePrivateProfileString (section_name , "proxy_type", 			"0", 			iniFname);
	WritePrivateProfileString (section_name , "proxy_host", 			"", 			iniFname);
	WritePrivateProfileString (section_name , "proxy_port", 			"0", 			iniFname);
	WritePrivateProfileString (section_name , "proxy_username",  		"",  			iniFname);
	WritePrivateProfileString (section_name , "proxy_password",  		"",  			iniFname);
	WritePrivateProfileString (section_name , "proxy_telnet_command",  	"",  			iniFname);

	Num_allServer	=	init_servers_from_iniFile();
	return true;
}

bool deletethisconnection(char* servertitle)
{
	/*
	if (any_connection_active()) {
		RequestProc(PluginNumber,RT_MsgOK,"Delete connection","Please close all open connections first!",NULL,0);
		return false;
	}
	*/

	int i,j,Sid;
	char section_name_old[MAX_Server_INFO];
	char section_name_new[MAX_Server_INFO];

	Sid=get_sftpServer_ID_by_Title(servertitle);
	if (Sid == -1)
	{
		RequestProc(PluginNumber,RT_MsgOK,"Delete connection","Delete Error: deletethisconnection(): Sid=-1 FIX me plz - FAST",NULL,0);
		return false;
	}
	if (allServer[Sid].is_imported_from_any_datasrc == 1)
	{
		if (allServer[Sid].is_imported_from_putty_registry ==1)
		{
			RequestProc(PluginNumber,RT_MsgOK,"Delete connection","This connection is imported from the PuTTY Session Database - can not delete",NULL,0);
			return false;
		}
		if (allServer[Sid].is_imported_from_sshcom_registry==1)
		{
			RequestProc(PluginNumber,RT_MsgOK,"Delete connection","This connection is imported from the ssh.com Session Database - can not delete",NULL,0);
			return false;
		}
		RequestProc(PluginNumber,RT_MsgOK,"Delete connection","This connection is imported  - can not delete",NULL,0);
		return false;
	}

	for (i=0;i<Num_allServer-1-Imported_ids_num;i++) 
	{
		if (strcmp(allServer[i].title,servertitle)==0) 
		{
			if (allServer[i].is_imported_from_any_datasrc == 1)
			{
				RequestProc(PluginNumber,RT_MsgOK,"Delete connection","This connection is imported from the PuTTY Session Database - can not delete",NULL,0);
				return false;
			}
			// Delete server i by moving all sections 1 up
			// What about Section rename?
			i++;     // It's 1-based in the ini file!
			for (j=i;j<Num_allServer-Imported_ids_num;j++) 
			{
				sprintf(section_name_old,"%i",j+1);
				sprintf(section_name_new,"%i",j);
				char copybuf[MAX_Server_INFO];

				GetPrivateProfileString (section_name_old,"title","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new ,"title",copybuf,iniFname); 
				GetPrivateProfileString (section_name_old,"host","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "host",	copybuf,		iniFname); 
				GetPrivateProfileString (section_name_old,"username","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "username",	copybuf,	iniFname); 
				GetPrivateProfileString (section_name_old,"port","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "port",copybuf[0] ? copybuf : NULL,	iniFname); 
				GetPrivateProfileString (section_name_old,"home_dir","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "home_dir",copybuf[0] ? copybuf : NULL, iniFname);
				GetPrivateProfileString (section_name_old,"compression","0",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "compression",	copybuf, iniFname);
				GetPrivateProfileString (section_name_old,"keyfilename","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "keyfilename",	copybuf, iniFname);
				GetPrivateProfileString (section_name_old,"dont_ask4_passphrase","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "dont_ask4_passphrase",	copybuf, iniFname);
				GetPrivateProfileString (section_name_old,"dont_ask4_password","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "dont_ask4_password",	copybuf, iniFname);
				GetPrivateProfileString (section_name_old,"use_key_auth","0",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "use_key_auth",	copybuf, iniFname);
				GetPrivateProfileString (section_name_old,"dont_ask4_username","0",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "dont_ask4_username",	copybuf, iniFname);
				GetPrivateProfileString (section_name_old,"password","",copybuf,MAX_Server_INFO,iniFname);
				WritePrivateProfileString (section_name_new, "password",copybuf, iniFname); 
				GetPrivateProfileString (section_name_old , "proxy_type", "0", copybuf,MAX_Server_INFO, iniFname);
				WritePrivateProfileString (section_name_new , "proxy_type", copybuf, iniFname);
				GetPrivateProfileString (section_name_old , "proxy_host", "", copybuf,MAX_Server_INFO, iniFname);
				WritePrivateProfileString (section_name_new , "proxy_host", copybuf, iniFname);
				GetPrivateProfileString (section_name_old , "proxy_port", "0", copybuf,MAX_Server_INFO, iniFname);
				WritePrivateProfileString (section_name_new , "proxy_port", copybuf, iniFname);
				GetPrivateProfileString (section_name_old , "proxy_username", "", copybuf,MAX_Server_INFO, iniFname);
				WritePrivateProfileString (section_name_new , "proxy_username", copybuf, iniFname);
				GetPrivateProfileString (section_name_old , "proxy_password", "", copybuf,MAX_Server_INFO, iniFname);
				WritePrivateProfileString (section_name_new , "proxy_password", copybuf, iniFname);
				GetPrivateProfileString (section_name_old , "proxy_telnet_command", "", copybuf,MAX_Server_INFO, iniFname);
				WritePrivateProfileString (section_name_new , "proxy_telnet_command", copybuf, iniFname);
			}
			// delete last section
			sprintf(section_name_old,"%i",Num_allServer-1-Imported_ids_num);
			WritePrivateProfileString (section_name_old , NULL,	NULL,	iniFname); 
			Num_allServer	=	init_servers_from_iniFile();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL __stdcall FsMkDir(char* Path)
{
	char cmd_buf[MAX_CMD_BUFFER];
	char *lPath = Path;

	if (strchr(Path+1,'\\')==NULL) {    // Create new server!
		return addnewserver(Path+1);
	}

	check_Concurrent_Connection(Path);

	lPath+=(1+strlen(allServer[CurrentServer_ID].title)+1);
	
	sprintf(cmd_buf,"mkdir \"%s\"",lPath);

	winSlash2unix(cmd_buf);
	
	if (wcplg_sftp_do_commando_byID(cmd_buf, NULL,CurrentServer_ID) == SFTP_SUCCESS) return true;
	return false;
}

void DefineAndAddConnection()
{	
	/*
	if (any_connection_active())
	{
		RequestProc(PluginNumber,RT_MsgOK,"Edit Connection","Please close all open connections first!",NULL,0);
	} else {
	*/
	ShellExecute(0,"open",iniFname,NULL,"c:\\",SW_SHOW);
	/*
	}
	*/
}

char* strlcat(char* p,char*p2,int maxlen)
{
	int restlen=maxlen-strlen(p);
	if (restlen>0)
		strlcpy(p+strlen(p),p2,restlen);
	return p;
}

void formcorrectpath(char* completepath)
{
	int j,j0,i;
	char *p,*p1,*pEnd;
	char ch;
	char* searchpos=completepath;

	do {
		j=0;
		p=strstr(searchpos,"\\.");
		if (p)
			j=2;
		else 
		{              // Detect also \ ..\ or \ ..#0}
			p=strstr(searchpos,"\\ .");
			if (p) j=3;
		}
		if (p) 
		{
			j0=j+1;
			while (p[j]=='.') j++;     // cd ...
			ch=p[j];
			pEnd=&p[j+1];
			if (ch==0 || ch=='\\') 
			{  //cd ..
				p[1]=0;
				for (i=j0;i<=j;i++)             //For each additional '.'
				{
					//Delete previous path
					p[0]=0;
					p1=strrchr(completepath,'\\');
					if (p1) 
					{
						p1[1]=0;
						if (ch=='\\')       //Copy rest of path!
							memmove(p1+1,pEnd,strlen(pEnd)+1);
					} else
						p[0]='\\';
					p=p1;
				}
				searchpos=completepath;
			} else                   //.. belongs to long name!
				searchpos=&p[3];
		}
    } while (p);
}

//This function needs a real revision
int __stdcall FsExecuteFile(HWND MainWin,char* RemoteName,char* Verb)
{
	char buf[MAX_CMD_BUFFER];
	char sftp_cmd[MAX_CMD_BUFFER];
	char log_sftp_cmd[MAX_CMD_BUFFER];

	if (strcmp(Verb,"open")==0) 
	{
		if (strcmp(DefineNewConnection2,RemoteName)==0)
		{
			DefineAndAddConnection();
			return FS_EXEC_OK;
		}

		if (strcmp(DefineAddConnection2,RemoteName)==0)
		{
			if (addnewserver(NULL) == (BOOL)true)
			{
				/*
				RequestProc(PluginNumber,RT_MsgOK,
				"Info",
				"New Server added, please press ctrl-r to reload the Screen",NULL,0);
				*/
				RemoteName[1]='\0';
				return FS_EXEC_SYMLINK;
			}
			return FS_EXEC_OK;
		}

		check_Concurrent_Connection(RemoteName);

		if (Risdir(RemoteName))		//if symlink
		{
			return FS_EXEC_SYMLINK;
		}
	}

	if (strncmp(Verb,"quote root ",11)==0) 
	{
		
		if (CurrentServer_ID == -1) return FS_EXEC_ERROR;
		strcpy(allServer[CurrentServer_ID].home_dir, &Verb[11]);
		int i=strlen(allServer[CurrentServer_ID].home_dir);
		char last_one=allServer[CurrentServer_ID].home_dir[i-1];

		sprintf(sftp_cmd,"cd %s",allServer[CurrentServer_ID].home_dir);
		winSlash2unix(sftp_cmd);
		if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID)!=SFTP_SUCCESS) 
		{
			sprintf(log_sftp_cmd,"change root failed!");
			LogProc_(MSGTYPE_DETAILS,log_sftp_cmd);
		}

		sprintf(RemoteName,"\\%s\\",allServer[CurrentServer_ID].title);
		return FS_EXEC_SYMLINK;
		
	}

	//REPUT SUPPORT
	if (strcmp(Verb,"quote reput") == 0 || strcmp(Verb,"quote reput ") == 0 || (strlen(Verb)>=strlen("quote reput  ") && strncmp(Verb,"quote reput  ",13) == 0)  )
	{
		RequestProc(PluginNumber,RT_MsgOK,"Usage for reput","Usage: reput <file>",NULL,0);			
		return FS_EXEC_ERROR;
	}

	if (strlen(Verb) > strlen("quote reput")+1 )
	{
		//check if connected 
		if (RemoteName != NULL && strlen(RemoteName) == 1 && RemoteName[0]=='\\')
		{
			RequestProc(PluginNumber,RT_MsgOK,"Reput","Can not use reput here",NULL,0);			
			return FS_EXEC_ERROR;
		}
		strcpy(buf,Verb);
		if (strncmp(buf,"quote reput ",12) == 0)
		{
			char remote_users_current_dir[MAX_CMD_BUFFER];
			int sftp_ret;

			if (RemoteName == NULL || strlen(RemoteName)<=0 || strchr(RemoteName,'?'))
			{
				dbg("Invalid RemoteName FIX ME PLZ");
				dbg(RemoteName);
				return FS_EXEC_ERROR;
			}

			check_Concurrent_Connection(RemoteName);
					
			//we must use tricks
			// segFault check!!!

			strncpy(remote_users_current_dir, buf+strlen("quote reput")+1, MAX_CMD_BUFFER);
			strncat(remote_users_current_dir, "?", MAX_CMD_BUFFER);
			strncat(remote_users_current_dir, RemoteName+2+strlen(allServer[CurrentServer_ID].title), MAX_CMD_BUFFER);

			winSlash2unix(remote_users_current_dir);

			//Log msg
			_snprintf(log_sftp_cmd, MAX_CMD_BUFFER, "reput %s", remote_users_current_dir);
			
			strncpy(sftp_cmd, "reput ", MAX_CMD_BUFFER);
			strncat(sftp_cmd, "\"", MAX_CMD_BUFFER);
			strncat(sftp_cmd, remote_users_current_dir, MAX_CMD_BUFFER);
			strncat(sftp_cmd, "\"", MAX_CMD_BUFFER);

			ProgressProc(PluginNumber,buf+strlen("quote reput")+1,buf+strlen("quote reput")+1,0);

			LogProc_(MSGTYPE_DETAILS,log_sftp_cmd);
			DISABLE_LOGGING_ONCE();					
			
			sftp_ret = wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID);
			if (sftp_ret == SFTP_SUCCESS)
			{
				ProgressProc(PluginNumber,buf+strlen("quote reput")+1,buf+strlen("quote reput")+1,100);
				//Set file time ?
				return FS_EXEC_OK;
			}
					
			ProgressProc(PluginNumber,buf+strlen("quote reput")+1,buf+strlen("quote reput")+1,100);
			return FS_EXEC_ERROR;
		}
	}

	//reget SUPPORT
	if (strcmp(Verb,"quote reget") == 0 || strcmp(Verb,"quote reget ") == 0 || (strlen(Verb)>=strlen("quote reget  ") && strncmp(Verb,"quote reget  ",13) == 0)  )
	{
		RequestProc(PluginNumber,RT_MsgOK,"Usage for reget","Usage: reget <file>",NULL,0);			
		return FS_EXEC_ERROR;
	}

	if (strlen(Verb) > strlen("quote reget")+1 )
	{
		//check if connected 
		if (RemoteName != NULL && strlen(RemoteName) == 1 && RemoteName[0]=='\\')
		{
			RequestProc(PluginNumber,RT_MsgOK,"reget","Can not use reget here",NULL,0);			
			return FS_EXEC_ERROR;
		}
		strcpy(buf,Verb);
		if (strncmp(buf,"quote reget ",12) == 0)
		{
			char remote_users_current_dir[MAX_CMD_BUFFER];
			char log_sftp_cmd[MAX_CMD_BUFFER];
			int sftp_ret;

			if (RemoteName == NULL || strlen(RemoteName)<=0 || strchr(RemoteName,'?'))
			{
				dbg("Invalid RemoteName FIX ME PLZ");
				dbg(RemoteName);
				return FS_EXEC_ERROR;
			}

			check_Concurrent_Connection(RemoteName);
		
			//we must use tricks
			// segFault check!!!
			strcpy(remote_users_current_dir,buf+strlen("quote reget")+1);
					
			//Log msg
			strcpy(log_sftp_cmd,"reget ");
			strcat(log_sftp_cmd,remote_users_current_dir);
			//

			strcat(remote_users_current_dir,"?");
			strcat(remote_users_current_dir,RemoteName+2+strlen(allServer[CurrentServer_ID].title));
			winSlash2unix(remote_users_current_dir);
					
			strcpy(sftp_cmd,"reget ");
			strcat(sftp_cmd,"\"");
			strcat(sftp_cmd,remote_users_current_dir);
			strcat(sftp_cmd,"\"");
					
			ProgressProc(PluginNumber,buf+strlen("quote reget")+1,buf+strlen("quote reget")+1,0);

			LogProc_(MSGTYPE_DETAILS,log_sftp_cmd);
			DISABLE_LOGGING_ONCE();					
			sftp_ret	=	wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID);
			if (sftp_ret == SFTP_SUCCESS)
			{
				ProgressProc(PluginNumber,buf+strlen("quote reget")+1,buf+strlen("quote reget")+1,100);
				//Set file time ?
				return FS_EXEC_OK;
			}
			
			ProgressProc(PluginNumber,buf+strlen("quote reget")+1,buf+strlen("quote reget")+1,100);
			return FS_EXEC_ERROR;

		}
	}

	if (strlen(Verb) > strlen("quote root")+1 )
	{
		//doesnt work correctly

		strcpy(buf,Verb);
		if (strncmp(buf,"quote root ",11) == 0)
		{
			check_Concurrent_Connection(RemoteName);
			buf[8]='c';
			buf[9]='d';

			if (wcplg_sftp_do_commando_byID(buf+8, NULL, CurrentServer_ID) == SFTP_SUCCESS)
			{
				strcpy(RemoteName,"\\");
				strcat(RemoteName,allServer[CurrentServer_ID].title);
				strcat(RemoteName,"\\");
				return FS_EXEC_SYMLINK;
			} else
				return FS_EXEC_ERROR;
		}
	}

	if (strlen(Verb) > strlen("quote cd")+1 )
	{
		strcpy(buf,Verb);
		if (strncmp(buf,"quote cd ",9) == 0)
		{
			check_Concurrent_Connection(RemoteName);
			
			//cd command: Change dir to what user wanted!
			// two cases: absolute path with / at start, relative with backslash

			if (buf[9]=='/' || buf[9]=='\\')
			{
				_snprintf(RemoteName, MAX_PATH - 1, "\\%s%s",
					allServer[CurrentServer_ID].title, buf + 9);
			} else {

				if (RemoteName[strlen(RemoteName)-1] != '\\')
					_snprintf(buf, MAX_PATH - 1, "%s\\%s", RemoteName, Verb + 9);
				else
					_snprintf(buf, MAX_PATH - 1, "%s%s", RemoteName, Verb + 9);
				strlcpy(RemoteName, buf, MAX_PATH - 1);

			}

			for(unsigned int i=0;i<strlen(RemoteName);i++)
				if (RemoteName[i] == '/') RemoteName[i]='\\';

			formcorrectpath(RemoteName);

			//do not work, WC API ?!?
			return FS_EXEC_SYMLINK;
		}
	}

	if (strlen(Verb) > strlen("chmod")+1 ) //Later 
	{
		strcpy(buf,Verb);
		buf[5]='\0';

		if (strcmp(buf,"chmod") == 0)
		{
			check_Concurrent_Connection(RemoteName);
			Verb+=6;
			RemoteName+=(1+strlen(allServer[CurrentServer_ID].title)+1);
				
			strcpy(sftp_cmd,"chmod ");
			strcat(sftp_cmd,Verb);
			strcat(sftp_cmd," \"./");
			strcat(sftp_cmd,RemoteName);
			strcat(sftp_cmd,"\"");
					
			RemoteName-=(1+strlen(allServer[CurrentServer_ID].title)+1);
			Verb-=6;
			winSlash2unix(sftp_cmd);

			if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) == SFTP_SUCCESS)
			{
				return FS_EXEC_OK;
			} else 
				return FS_EXEC_ERROR;
		}
	}

	//return FS_EXEC_OK;
	return FS_EXEC_ERROR;

	check_Concurrent_Connection(RemoteName);
	strcpy(sftp_cmd,"ls \".");
	RemoteName+=(1+strlen(allServer[CurrentServer_ID].title));
	strcat(sftp_cmd,RemoteName);
	RemoteName-=(1+strlen(allServer[CurrentServer_ID].title));
	strcat(sftp_cmd,"\"");
	winSlash2unix(sftp_cmd);

	if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, CurrentServer_ID) != SFTP_SUCCESS)
	{
		return FS_EXEC_ERROR;
	}
	return FS_EXEC_SYMLINK ;
	return FS_EXEC_SYMLINK ;
	return FS_EXEC_ERROR;
}

int __stdcall FsRenMovFile(char* OldName,char* NewName,BOOL Move,BOOL OverWrite,RemoteInfoStruct* ri)
{
	int retv;
	char cmd_buf[MAX_CMD_BUFFER];

	int server_2_server_move = 0;
	int serverid_from_oldname;
	int serverid_from_newname;

	if (strchr(OldName+1,'\\')==NULL)
		return 0;
	
	serverid_from_oldname	=	get_sftpServer_ID_by_Path(OldName);
	serverid_from_newname	=	get_sftpServer_ID_by_Path(NewName);
	if (serverid_from_oldname == -1 || serverid_from_newname== -1 )
	{
		dbg("FsRenMovFile(): get_sftpServer_ID_by_Path == -1 FIX me plz!!!");
		return FS_FILE_READERROR;
	}

	if (Move && serverid_from_oldname != serverid_from_newname)
	{
		server_2_server_move = 1;
	}

	if (!Move || server_2_server_move == 1)
	{
		// mhhhh Server2Server copy... okay :->
		char tmp_dir_file[MAX_PATH];
		int cp_flags;
		
		RemoteInfoStruct *BlindStrukt=NULL;
		get_tmp_file_name(tmp_dir_file);
				
		FsGetFile(OldName,tmp_dir_file,FS_COPYFLAGS_OVERWRITE,BlindStrukt);
		// if file exsits ?

		if (OverWrite)
		{
			cp_flags	= FS_COPYFLAGS_OVERWRITE;
		} else {
			cp_flags	= 0;
		}

		retv = FsPutFile(tmp_dir_file, NewName,cp_flags);
		_unlink( tmp_dir_file );

		if (retv==FS_FILE_OK && server_2_server_move == 1)
		{
			FsDeleteFile(OldName);
		}

		return retv;
	} else {    // its a rename/move on the same server
		int err=ProgressProc(PluginNumber,OldName,NewName,0);
		check_Concurrent_Connection(OldName);

		if (!OverWrite)
		{
			int R_fexists	=	file_exists_on_remote_server(NewName);
			if (R_fexists == FS_FILE_EXISTS) 
			{
				return FS_FILE_EXISTS	;
			}
		}

		// Rename!
		OldName+=(1+strlen(allServer[CurrentServer_ID].title)+1);
		NewName+=(1+strlen(allServer[CurrentServer_ID].title)+1);

		strcpy(cmd_buf,"rename \"./");
		strcat(cmd_buf,OldName);
		strcat(cmd_buf,"\" \"./");
		strcat(cmd_buf,NewName);
		strcat(cmd_buf,"\"");
	
		OldName-=(1+strlen(allServer[CurrentServer_ID].title)+1);
		NewName-=(1+strlen(allServer[CurrentServer_ID].title)+1);

		winSlash2unix(cmd_buf);
	
		err=ProgressProc(PluginNumber,OldName,NewName,50);
		if (wcplg_sftp_do_commando_byID(cmd_buf, NULL, CurrentServer_ID) == SFTP_SUCCESS)
		{
			err=ProgressProc(PluginNumber,OldName,NewName,100);
			return FS_FILE_OK;
		}
		
		err=ProgressProc(PluginNumber,OldName,NewName,100);	

		return FS_FILE_WRITEERROR;

	}
	return FS_FILE_WRITEERROR;
}

int __stdcall FsGetFile(char* RemoteName,char* LocalName,int CopyFlags,RemoteInfoStruct* ri)
{
	char cmd_buf[MAX_CMD_BUFFER];
	int err;
	bool ok=true;

	if ((CopyFlags & FS_COPYFLAGS_OVERWRITE) == 0 && CopyFlags != FS_COPYFLAGS_RESUME) //möchte file holen 
	{
		//local
		FILE *fp;
		fp=fopen(LocalName,"r") ;
		if (fp != NULL)
		{
			//file ist vorhanden!
			fclose(fp);
			//reget ?
			return FS_FILE_EXISTSRESUMEALLOWED ;//FS_FILE_EXISTS;
		}
	}
	
	if (CopyFlags & FS_COPYFLAGS_RESUME)
	{
		//reget
		int ret=FS_EXEC_ERROR;
		char path_[MAX_CMD_BUFFER];
		char fname_[MAX_CMD_BUFFER];
		char wd_saved[MAX_CMD_BUFFER];
		char wd_curr[MAX_CMD_BUFFER];

		get_basename_from_Path(path_,RemoteName);
		strcpy(fname_,RemoteName+(strlen(path_)+1));

		strcpy(cmd_buf,"quote reget ");
		strcat(cmd_buf,fname_);
		strcat(path_,"\\");

		if (getcwd(wd_saved,MAX_PATH) == NULL)
		{
			return FS_FILE_WRITEERROR;
		}
		get_basename_from_Path(wd_curr,LocalName);
		if (chdir(wd_curr) != 0)
		{
			return FS_FILE_WRITEERROR;
		}

		ret	=	FsExecuteFile((HWND) NULL,path_,cmd_buf);
		chdir(wd_saved);

		if (ret == FS_EXEC_OK)
		{
			return FS_FILE_OK;
		}
		return FS_FILE_WRITEERROR;
	}
	
	check_Concurrent_Connection(RemoteName);
	
	strcpy(cmd_buf,"get \"./");

	RemoteName+=(1+strlen(allServer[CurrentServer_ID].title)+1);
	strcat(cmd_buf,RemoteName);
	RemoteName-=(1+strlen(allServer[CurrentServer_ID].title)+1);
	winSlash2unix(cmd_buf);
	strcat(cmd_buf,"\"");	

	strcat(cmd_buf," \"");
	strcat(cmd_buf,LocalName);
	strcat(cmd_buf,"\"");
	
	err=ProgressProc(PluginNumber,RemoteName,LocalName,0);

	if (err) return FS_FILE_USERABORT;	

	ok=(wcplg_sftp_do_commando_byID(cmd_buf, NULL,CurrentServer_ID) == SFTP_SUCCESS);

	if (ok) 
	{
		if (ri != NULL) 
		{
			//Nur wenn ri gesetzt ist
			SetFileTime__(LocalName,&ri->LastWriteTime);
		}

		if (CopyFlags & FS_COPYFLAGS_MOVE) 
		{
			cmd_buf[1]='r';
			cmd_buf[2]='m';   // send rm command to move!
			ok=(wcplg_sftp_do_commando_byID(cmd_buf+1, NULL,CurrentServer_ID) == SFTP_SUCCESS);
		}

		err=ProgressProc(PluginNumber,RemoteName,LocalName,100);
		if (err) return FS_FILE_USERABORT;	

		if (ok)
			return FS_FILE_OK;
	}
	return FS_FILE_READERROR;
}

int __stdcall FsPutFile(char* LocalName,char* RemoteName,int CopyFlags)
{
	char cmd_buf[MAX_CMD_BUFFER];
	char buf2[MAX_CMD_BUFFER];
	    
	int err=0;
	int ok=true;
	
	if ((CopyFlags & FS_COPYFLAGS_OVERWRITE) == 0) //möchte file holen 
	{
		//ok check Server existiert
		if (get_sftpServer_ID_by_Path(RemoteName) == -1)
		{
			//local
			// Dürfte gar nicht vorkommen
			dbg("Unknown Target [error 1]");
			return FS_FILE_WRITEERROR;
		} else {
			if (CopyFlags & FS_COPYFLAGS_EXISTS_SAMECASE)  //optimized: use hint from Wincmd
				return FS_FILE_EXISTSRESUMEALLOWED;
		}
	}

	if (CopyFlags & FS_COPYFLAGS_RESUME)
	{
		//reput
		int ret=FS_EXEC_ERROR;
		char path_[MAX_CMD_BUFFER];
		char fname_[MAX_CMD_BUFFER];
		char wd_saved[MAX_CMD_BUFFER];
		char wd_curr[MAX_CMD_BUFFER];

		get_basename_from_Path(path_,RemoteName);
		strcpy(fname_,RemoteName+(strlen(path_)+1));

		strcpy(cmd_buf,"quote reput ");
		strcat(cmd_buf,fname_);
		strcat(path_,"\\");

		if (getcwd(wd_saved,MAX_PATH) == NULL)
		{
			return FS_FILE_WRITEERROR;
		}

		get_basename_from_Path(wd_curr,LocalName);

		if (chdir(wd_curr) != 0)
		{
			return FS_FILE_WRITEERROR;
		}

		ret	=	FsExecuteFile((HWND) NULL,path_,cmd_buf);
		chdir(wd_saved);

		if (ret == FS_EXEC_OK)
		{
			return FS_FILE_OK;
		}

		return FS_FILE_WRITEERROR;
	}
	
	check_Concurrent_Connection(RemoteName);

	strcpy(cmd_buf,"put \"");
	strcat(cmd_buf,LocalName);
	strcat(cmd_buf,"\" \"./");

	RemoteName+=(1+strlen(allServer[CurrentServer_ID].title)+1);
	strcpy(buf2,RemoteName);
	RemoteName-=(1+strlen(allServer[CurrentServer_ID].title)+1);
	winSlash2unix(buf2);
	strcat(cmd_buf,buf2);	
	strcat(cmd_buf,"\"");	
	
	err=ProgressProc(PluginNumber,LocalName,RemoteName,0);
	if (err) return FS_FILE_USERABORT;

	ok=(wcplg_sftp_do_commando_byID(cmd_buf, NULL,CurrentServer_ID) == SFTP_SUCCESS);
	
	if (ok)
	{
		if (CopyFlags & FS_COPYFLAGS_MOVE) 
		{
			ok=(0==_unlink(LocalName));    // delete source after successful upload
		}
		err=ProgressProc(PluginNumber,RemoteName,LocalName,100);
		if (err) return FS_FILE_USERABORT;
		if (ok)
			return FS_FILE_OK;
	}

	return FS_FILE_WRITEERROR;
}

BOOL __stdcall FsDeleteFile(char* RemoteName)
{
	char cmd_buf[MAX_CMD_BUFFER];

	if (RemoteName[0]!='\\')
		return false;
	
	strcpy(cmd_buf,"rm \"./");

	ProgressProc(PluginNumber,RemoteName,RemoteName,0);
	check_Concurrent_Connection(RemoteName);	

	RemoteName+=(1+strlen(allServer[CurrentServer_ID].title)+1);
	strcat(cmd_buf,RemoteName);
	RemoteName-=(1+strlen(allServer[CurrentServer_ID].title)+1);
	strcat(cmd_buf,"\"");	

	winSlash2unix(cmd_buf);
	
	ProgressProc(PluginNumber,RemoteName,RemoteName,50);
	if (wcplg_sftp_do_commando_byID(cmd_buf, NULL,CurrentServer_ID) == SFTP_SUCCESS) 	
	{
		ProgressProc(PluginNumber,RemoteName,RemoteName,100);
		return true;
	}
	return false;
}

BOOL __stdcall FsRemoveDir(char* RemoteName)
{
	// user is trying to delete a connection!
	if (RemoteName != NULL && strlen(RemoteName)==strlen(QUICK_CONNECTION)+1 && strcmp(1+RemoteName,QUICK_CONNECTION) == 0)
	{
		RequestProc(PluginNumber,RT_MsgOK,"Delete Connection","Can not delete " QUICK_CONNECTION,NULL,0);
		return false;
	}

	if (delete_only_connection) 
	{
		deletethisconnection(RemoteName+1);
		return true;
	}

	char cmd_buf[MAX_CMD_BUFFER];

	check_Concurrent_Connection(RemoteName);

	strcpy(cmd_buf,"rmdir \"./");

	RemoteName+=(1+strlen(allServer[CurrentServer_ID].title)+1);
	strcat(cmd_buf,RemoteName);
	RemoteName-=(1+strlen(allServer[CurrentServer_ID].title)+1);
	
	strcat(cmd_buf,"\"");

	winSlash2unix(cmd_buf);
	
	if (wcplg_sftp_do_commando_byID(cmd_buf, NULL,CurrentServer_ID) == SFTP_SUCCESS) return true;
	return false;
}


void __stdcall dbg(char *msg)
{
	if (msg == NULL) return;
	RequestProc(PluginNumber,RT_MsgOK,"DBG",msg,NULL,2000);	
}

int wcplg_sftp_do_commando_byID(char *sftp_cmd, char *serverOutput, int ID)
{
	if (wcplg_sftp_connect_byID(ID) == SFTP_SUCCESS)
	{
		return wcplg_sftp_do_commando(sftp_cmd,serverOutput, ID);
	}
	return SFTP_FAILED;
}

int wcplg_sftp_connect_byID(unsigned int ID)
{
	bool server_entered=false;
	bool user_entered=false;
	char buf[MAX_CMD_BUFFER];

	/*
	sprintf(buf,"severid:%d",ID);
	LogProc_(MSGTYPE_DETAILS,buf);
	*/

	if (ID == -1) return SFTP_FAILED; //wer weis!

	if (strlen(allServer[ID].host_cached) < 1)
	{
		strcpy(allServer[ID].host_cached,allServer[ID].host);
		if (!RequestProc(PluginNumber,RT_Other,"Secure FTP","Host[:port]",allServer[ID].host_cached,MAX_CMD_BUFFER))
		{
			allServer[ID].host_cached[0]='\0';
			return SFTP_FAILED;
		}
				
		server_entered=true;
		strcpy(allServer[ID].host,allServer[ID].host_cached);
	}

	if (strlen(allServer[ID].username_cached) < 1)
	{
		strcpy(buf,"Username for ");
		strcat(buf,allServer[ID].host_cached);
		
		strcpy(allServer[ID].username_cached,allServer[ID].username);
			
		// Do not ask 4 username if dont_ask4_username==1
		if (allServer[ID].dont_ask4_username != 1)
		{
			if (!RequestProc(PluginNumber,RT_UserName,"Secure FTP",buf,allServer[ID].username_cached,MAX_CMD_BUFFER)) 
			{
				// For quick connection, clear host if failed
				allServer[ID].username_cached[0]=0;
				if (server_entered)
					allServer[ID].host_cached[0]=0;
				return SFTP_FAILED;
			}
		}
		user_entered=true;
		strcpy(allServer[ID].username,allServer[ID].username_cached);
	}

	if (strlen(allServer[ID].password_cached) < 1)
	{
		strcpy(buf,"Password for ");
		strcat(buf,allServer[ID].username_cached);
		strcat(buf,"@");
		strcat(buf,allServer[ID].host_cached);
			
		strcpy(allServer[ID].password_cached,allServer[ID].password);

		if (allServer[ID].use_key_auth != 1)
		{
			if (!allServer[ID].dont_ask4_password) {
				if (!RequestProc(PluginNumber,RT_Password ,"Secure FTP",buf,allServer[ID].password_cached,MAX_CMD_BUFFER)) 
				{
					// For quick connection, clear host+user if failed
					if (server_entered)
						allServer[ID].host_cached[0]=0;
					if (user_entered)
						allServer[ID].username_cached[0]=0;
					return SFTP_FAILED;
				}
			}
		}

		if (allServer[ID].dont_ask4_password) strcpy(allServer[ID].password_cached,allServer[ID].password);
		else strcpy(allServer[ID].password,allServer[ID].password_cached);
	}

	if (wcplg_sftp_connect(allServer[ID].username, allServer[ID].password, allServer[ID].host, allServer[ID].port, allServer,ID) != SFTP_SUCCESS)
	{
		allServer[ID].password_cached[0]='\0';
		allServer[ID].host_cached[0]=0;
		allServer[ID].username_cached[0]=0;
		return SFTP_FAILED;
	}

	//OK connect hat geklappt
	return SFTP_SUCCESS;
}

int get_sftpServer_ID_by_Path(char *Path)
{
	char *Path_ORG;
	char ServerTitle[MAX_Server_INFO];
	Path_ORG	=	Path;

	Path+=1;
	
	if (strchr(Path,'\\') != NULL) 
	{
		// OK, it's a peace like this:
		// server n\foo\bla\etc
		// have 2 cut the rest after server n
		int count_;
		count_	=	(int)((int)strchr(Path,'\\') - (int)Path);
		strncpy(ServerTitle,Path, count_);
		ServerTitle[count_]='\0';
	} else {
		// OK, it's a peace like this:
		// server n
		// we have the Server Title
		strcpy(ServerTitle,Path);
		//ServerTitle	=	Path;
	}

	Path	=	Path_ORG;// Flush
	return get_sftpServer_ID_by_Title(ServerTitle);
}

int get_sftpServer_ID_by_Title(char *Title)
{
	int i;
	for(i=0;i<Num_allServer;i++)
	{
		if (strcmp(allServer[i].title,Title)==0) 
		{
			return i;
		}
	}
	return -1;
}

bool any_connection_active()
{
	for (int i=0;i<Num_allServer;i++) 
	{
		if (allServer[i].username_cached[0])
			return true;
	}
	return false;
}

int init_servers_from_iniFile()
{
	int max_sections=MAX_Server;
	int i;
	int ID=0;
	int imported_num=0;
	char section_name[MAX_Server_INFO];
	char buf_title[MAX_Server_INFO];
	char buf_host[MAX_Server_INFO];
	char buf_username[MAX_Server_INFO];
	char buf_password[MAX_Server_INFO];
	char buf_port[MAX_Server_INFO];
	char buf_home_dir[MAX_Server_INFO];
	char buf_compression[MAX_Server_INFO];
	char buf_use_key_auth[MAX_Server_INFO];
	char buf_dont_ask4_username[MAX_Server_INFO];
	char buf_dont_ask4_password[MAX_Server_INFO];
	char buf_divers[MAX_Server_INFO];
	char buf_keyfilename[MAX_Server_INFO];
	char buf_dont_ask4_passphrase[MAX_Server_INFO];
	char buf_proxy_type[MAX_Server_INFO];
	char buf_proxy_host[MAX_Server_INFO];
	char buf_proxy_port[MAX_Server_INFO];
	char buf_proxy_username[MAX_Server_INFO];
	char buf_proxy_password[MAX_Server_INFO];
	char buf_proxy_telnet_command[MAX_Server_INFO];

	WritePrivateProfileString( NULL, NULL, NULL, iniFname ); 

	for(i=0;i<max_sections;i++)
	{
		buf_title[0]='\0';
		buf_host[0]='\0';
		buf_username[0]='\0';
		buf_password[0]='\0';
		buf_port[0]='\0';
		buf_home_dir[0]='\0';
		buf_compression[0]='\0';
		buf_keyfilename[0]='\0';
		buf_dont_ask4_passphrase[0]='\0';
		buf_dont_ask4_password[0]='\0';
		buf_use_key_auth[0]='\0';
		buf_dont_ask4_username[0]='\0';
		buf_proxy_type[0]='\0';
		buf_proxy_host[0]='\0';
		buf_proxy_port[0]='\0';
		buf_proxy_username[0]='\0';
		buf_proxy_password[0]='\0';
		buf_proxy_telnet_command[0]='\0';

		sprintf(section_name,"%i",i);

		GetPrivateProfileString (section_name , "title",				"", buf_title,				MAX_Server_INFO,	iniFname); 
		GetPrivateProfileString (section_name , "host",					"", buf_host,				MAX_Server_INFO,	iniFname); 

		if (strlen(buf_host) && strlen(buf_title))
		{
			GetPrivateProfileString (section_name , "username",				"",	buf_username,			 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "password",				"", buf_password,			 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "port",					"", buf_port,				 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "home_dir",				"", buf_home_dir,			 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "compression",			"", buf_compression,		 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "use_key_auth",			"", buf_use_key_auth,		 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "keyfilename",			"", buf_keyfilename,		 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "dont_ask4_passphrase",	"", buf_dont_ask4_passphrase,MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "dont_ask4_password",	"", buf_dont_ask4_password,  MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "dont_ask4_username",	"", buf_dont_ask4_username,	 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "proxy_type",			"", buf_proxy_type,			 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "proxy_host",			"", buf_proxy_host,			 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "proxy_port",			"", buf_proxy_port,			 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "proxy_username",		"", buf_proxy_username,		 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "proxy_password",		"", buf_proxy_password,		 MAX_Server_INFO,	iniFname); 
			GetPrivateProfileString (section_name , "proxy_telnet_command",	"", buf_proxy_telnet_command,MAX_Server_INFO,	iniFname); 
			// Check title for / && 
			XconvertServerTitle(buf_title);

			// TODO check ob title schon gibt wenn ja dann andren namen geben z.B [1]

			strcpy(allServer[ID].title,buf_title);
			strcpy(allServer[ID].host,buf_host);
			strcpy(allServer[ID].host_cached,buf_host);
			strcpy(allServer[ID].username,buf_username);
			strcpy(allServer[ID].password,buf_password);
			strcpy(allServer[ID].keyfilename,buf_keyfilename);
						
			//use_key_auth optional, default=0
			if (strlen(buf_use_key_auth))
			{
				allServer[ID].use_key_auth=strtol(buf_use_key_auth,NULL,NULL);
				if (allServer[ID].use_key_auth > 1) allServer[ID].use_key_auth=1;
			} else {
				allServer[ID].use_key_auth=0;
			}

			//dont_ask4_username optional, default=0
			if (strlen(buf_dont_ask4_username))
			{
				allServer[ID].dont_ask4_username=strtol(buf_dont_ask4_username,NULL,NULL);
				if (allServer[ID].dont_ask4_username > 1) allServer[ID].dont_ask4_username=1;
			} else {
				allServer[ID].dont_ask4_username=0;
			}

			//dont_ask4_passphrase optional, default=0
			if (strlen(buf_dont_ask4_passphrase))
			{
				allServer[ID].dont_ask4_passphrase=strtol(buf_dont_ask4_passphrase,NULL,NULL);
				if (allServer[ID].dont_ask4_passphrase > 1) allServer[ID].dont_ask4_passphrase=1;
			} else {
				allServer[ID].dont_ask4_passphrase=0;
			}

			//dont_ask4_password optional, default=0
			if (strlen(buf_dont_ask4_password))
			{
				allServer[ID].dont_ask4_password=strtol(buf_dont_ask4_password,NULL,NULL);
				if (allServer[ID].dont_ask4_password > 1) allServer[ID].dont_ask4_password=1;
			} else {
				allServer[ID].dont_ask4_password=0;
			}

			// port und home_dir sind optional
			if (strlen(buf_port))
			{
				allServer[ID].port	=	strtol(buf_port,NULL,NULL);
			} else {
				allServer[ID].port	=	22;
			}

			if (allServer[ID].port < 1 || allServer[ID].port > 65535)		
			{
				allServer[ID].port=22;
			}
				
			if (strlen(buf_compression) == 0 || strcmp(buf_compression,"1") == 0)
			{
				allServer[ID].compression=1;		
			} else {
				allServer[ID].compression=0;
			}

			if (strlen(buf_home_dir))
			{
				strcpy(allServer[ID].home_dir,buf_home_dir);
			} else {
				strcpy(allServer[ID].home_dir,"");
			}
				
			allServer[ID].passphrase[0]						=	0;
			allServer[ID].password_cached[0]				=	0;
			allServer[ID].username_cached[0]				=	0;
			allServer[ID].is_imported_from_any_datasrc		=	0;
			allServer[ID].is_imported_from_putty_registry	=	0;
			allServer[ID].is_imported_from_sshcom_registry	=	0;

			//experimental: Proxy
			if (strlen(buf_proxy_type))
			{
				unsigned long tmp = strtoul(buf_proxy_type,NULL,NULL);
				switch (tmp) 
				{
					case 1: 
						allServer[ID].proxy_type = PROXY_SOCKS4; 
						break;
					case 2:
						allServer[ID].proxy_type = PROXY_SOCKS5;
						break;
					default:
						allServer[ID].proxy_type = PROXY_NONE;
						break;
				}
			} else {
				allServer[ID].proxy_type = PROXY_NONE;
			}

			if (strlen(buf_proxy_host))
			{
				strcpy(allServer[ID].proxy_host,buf_proxy_host);
			} else {
				strcpy(allServer[ID].proxy_host,"");
			}

			if (strlen(buf_proxy_port))
			{
				allServer[ID].proxy_port	=	strtol(buf_proxy_port,NULL,NULL);
			} else {
				allServer[ID].proxy_port	=	1080;   //standard port for socks-proxy
			}
			
			if (allServer[ID].proxy_port < 1 || allServer[ID].proxy_port > 65535)		
			{
				allServer[ID].proxy_port	=   1080;	//standard port for socks-proxy
			}

			if (strlen(buf_proxy_username))
			{
				strcpy(allServer[ID].proxy_username,buf_proxy_username);
			} else {
				strcpy(allServer[ID].proxy_username,"");
			}
			
			if (strlen(buf_proxy_password))
			{
				strcpy(allServer[ID].proxy_password,buf_proxy_password);
			} else {
				strcpy(allServer[ID].proxy_password,"");
			}

			if (strlen(buf_proxy_telnet_command))
			{
				strcpy(allServer[ID].proxy_telnet_command,buf_proxy_telnet_command);
			} else {
				strcpy(allServer[ID].proxy_telnet_command,"");
			}

			//this line checks if we get a really supported proxy version - the other stuff is not really supported right now...
			if ((allServer[ID].proxy_type!=PROXY_SOCKS4) && (allServer[ID].proxy_type!=PROXY_SOCKS5))
				allServer[ID].proxy_type=PROXY_NONE;

			allServer[ID].id=ID;
			ID++;
		} else if (i) break; // hey dude - this is a badness, too :-(
	}
	
	// okay, we are outta here  

	// DO Putty Import ?
	strcpy(buf_divers,"");
	GetPrivateProfileString (INI_CONFIG__SECTION_NAME,INI_CONFIG_IMPORT_PUTTY_SSH_SESS,"",buf_divers,MAX_Server_INFO,iniFname);
	if (strlen(buf_divers) && (buf_divers[0] == '1' || buf_divers[0] == '2'))
	{
		//import the putty saved session if...
		imported_num		=	0;
		imported_num		+=	do_import_putty_saved_session_to_plugin_session(ID, buf_divers);
		Imported_ids_num	=	imported_num;
		ID+=imported_num;
	}

	// DO ssh.com Import ?
	strcpy(buf_divers,"");
	GetPrivateProfileString (INI_CONFIG__SECTION_NAME,INI_CONFIG_IMPORT_SSHCOM_SSH_SESS,"",buf_divers,MAX_Server_INFO,iniFname);
	if (strlen(buf_divers))
	{
		char dir_location[MAX_CMD_BUFFER];

		strcpy(dir_location,buf_divers);
		//import the ssh.com saved session if...
		if (strlen(dir_location)>0 && strcmp(dir_location,"0") != 0)
		{
			imported_num=0;
			imported_num		=	do_import_sshcom_saved_session_to_plugin_session(ID,dir_location);
			Imported_ids_num	+=	imported_num;
			ID+=imported_num;
		}
	}
  	
	strcpy(allServer[ID].title,QUICK_CONNECTION);	
	//Set defaults 4 quick connection
	allServer[ID].compression=0;
	allServer[ID].dont_ask4_username=0;
	allServer[ID].dont_ask4_passphrase=0;
	allServer[ID].dont_ask4_password=0;
	allServer[ID].home_dir[0]='\0';
	allServer[ID].host[0]='\0';
	allServer[ID].host_cached[0]='\0';
	allServer[ID].id=ID;
	allServer[ID].is_imported_from_any_datasrc=0;
	allServer[ID].is_imported_from_putty_registry=0;
	allServer[ID].is_imported_from_sshcom_registry=0;
	allServer[ID].keyfilename[0]='\0';
	allServer[ID].passphrase[0]='\0';
	allServer[ID].proxy_host[0]='\0';
	allServer[ID].proxy_password[0]='\0';
	allServer[ID].proxy_port=0;
	allServer[ID].proxy_telnet_command[0]='\0';
	allServer[ID].proxy_type=PROXY_NONE;
	allServer[ID].proxy_username[0]='\0';
	allServer[ID].password[0]='\0';
	allServer[ID].password_cached[0]='\0';
	allServer[ID].port=22;	
	allServer[ID].use_key_auth=0;
	allServer[ID].username[0]='\0';
	allServer[ID].username_cached[0]='\0';
	
	ID++;// !!!

	make_unique_title_in_allServers(ID);
	
	return ID;  
}

void LogProc_(int MsgType,char* LogString)
{
	if (&LogProc && do_logging())
		LogProc(PluginNumber,MsgType,LogString);
}

BOOL __stdcall FsDisconnect(char* DisconnectRoot)
{
	// DisconnectRoot nützt nix, kann eh nur 1ne connection gleichzeitig aufrecht erhalten psftp.c Architektur is schuld drann
	//LogProc_(MSGTYPE_DISCONNECT,"DISCONNECTED");
	wcplg_sftp_disconnect(CurrentServer_ID,false);//get_sftpServer_ID_by_Path(DisconnectRoot),false);
	already_connected = false;
	return true;
}

void check_Concurrent_Connection(char *Path)
{
	//CurrentServer_ID is global
	if (CurrentServer_ID != -1) //Do we have überhaupt a connection  to any Server ?
	{
		if (get_sftpServer_ID_by_Path(Path) != CurrentServer_ID)
		{
			// upppps, conflict!
			// Perhaps kann ich ja mehrere Instanzen der DLL speichern, muss mal gucken, but now we disconnect the connection to avoid the conflict
			//wcplg_sftp_disconnect();// reconnecting automatisch
			if (already_connected) wcplg_sftp_disconnect(CurrentServer_ID,false); 
			CurrentServer_ID = get_sftpServer_ID_by_Path(Path);// OK, conflict fixed :-)

		}
	}
	
}

int get_current_server_id()
{
	return CurrentServer_ID;
}

struct SftpServerAccountInfo *getP_allServer(void)
{
	return allServer;
}

BOOL Request_Password(char *buf)
{
	return RequestProc(PluginNumber,RT_Password ,"Secure FTP", "Password",buf,MAX_PATH);
}

int octal_perm_2_wc_int(unsigned long octal_val)
{
	//quick and dirty

	char buf[100];//dürft reichen
	char cbuf[100];
	int clen;
	int int_val;
	sprintf(buf,"%3o",octal_val);

	clen	=	strlen(buf);
	cbuf[0]	=	'0';
	cbuf[1]	=	buf[clen-3];
	cbuf[2]	=	buf[clen-2];
	cbuf[3]	=	buf[clen-1];
	cbuf[4]	=	'\0';
		
	int_val = strtol( cbuf, '\0', 0);
	return int_val ;
}

tProgressProc get_ProgressProc()
{
	return ProgressProc;
}

int get_PluginNumber()
{
	return PluginNumber;
}

int file_exists_on_remote_server(char *RemoteFile)
{
	if (get_sftpServer_ID_by_Path(RemoteFile) == -1) return FS_FILE_NOTSUPPORTED;
		
	char Path[MAX_CMD_BUFFER];
	char File_[MAX_CMD_BUFFER];
		
	strcpy(Path,RemoteFile);
	if (get_basename_from_Path(Path,RemoteFile) != 1)
	{
		dbg("file_exists_on_remote_server: fix me, path!");
		return FS_FILE_NOTSUPPORTED; 
	}

	RemoteFile+=(strlen(Path)+1);
	strcpy(File_,RemoteFile);
	RemoteFile-=(strlen(Path)+1);
	
	check_Concurrent_Connection(RemoteFile);
	WIN32_FIND_DATA FindData;
	HANDLE lf;

	lf=FsFindFirst(Path,&FindData);

	if (lf == INVALID_HANDLE_VALUE)
	{
	 	FsFindClose(lf);
		return FS_FILE_NOTFOUND;
	}

	//while(FsFindNext(lf,&FindData) != false) {;} //zur Sicherheit nochmal durchlaufen lassen das auch alle Daten da sind

	pLastFindStuct lf_res;
	lf_res=(pLastFindStuct)lf;
	int found=0;
	if (lf_res->CurrentDirStruct != NULL) 
	{
		for(int i=0;i<lf_res->CurrentDirStruct[0].nnames;i++)
		{
			if (strcmp(lf_res->CurrentDirStruct->names[i].filename,File_) == 0) 
			{
				found=1;
				break;
			}
		}
	}

	FsFindClose(lf);
	if (found==1) return FS_FILE_EXISTS;
	
	return FS_FILE_NOTFOUND;
}

int get_basename_from_Path(char *buf,char *Path)
{
	unsigned int i;
	unsigned int dir_end=0;

	i=strlen(Path)-1;
	for(;i>0;i--)
	{
		if (Path[i] == '\\')
		{
			dir_end=i;	
			break;
		}
	}
	
	if (dir_end== 0) return 0;// FIX ME MORE!!!
		
	for(i=0;i<(dir_end);i++)
	{
		buf[i]	=	Path[i];
	}
	buf[i]='\0';
	return 1;
}

void __stdcall FsGetDefRootName(char* DefRootName,int maxlen)
{
	strlcpy(DefRootName,FSPLUGIN_CAPTION,maxlen-1);
}

void __stdcall FsStatusInfo(char* RemoteDir,int InfoStartEnd,int InfoOperation)
{
	if (InfoOperation==FS_STATUS_OP_DELETE) //why is this here? :-)
	{
		if (strcmp(RemoteDir,"\\")==0)  // Deleting connection!
		{    
			delete_only_connection=(InfoStartEnd==FS_STATUS_START);
		}
	}
}

int Risdir(char *Path)
{
	char sftp_cmd[MAX_CMD_BUFFER];
	int Server_ID;
	
	Server_ID	=	get_sftpServer_ID_by_Path(Path);
	if (Server_ID == -1) return 0;

	strcpy(sftp_cmd,"ls \"./");

	Path+=(1+strlen(allServer[CurrentServer_ID].title)+1);
	strcat(sftp_cmd,Path);
	Path-=(1+strlen(allServer[CurrentServer_ID].title)+1);

	strcat(sftp_cmd,"\"");

	winSlash2unix(sftp_cmd);

	//DISABLE_LOGGING();	
	if (wcplg_sftp_do_commando_byID(sftp_cmd, NULL, Server_ID) != SFTP_SUCCESS)
	{
		//ENABLE_LOGGING();
		return 0;
	}
	//ENABLE_LOGGING();

	return 1;
}

BOOL SetFileTime__(char *fullFilePath,FILETIME *LastWriteTime)
{
    BOOL f;
	HANDLE myFile;
	if (LastWriteTime == NULL) return false;
    myFile = CreateFile(	fullFilePath,				// file name
							GENERIC_WRITE,				// access mode
							FILE_SHARE_READ,			// share mode
							NULL,						// security descriptor
							OPEN_EXISTING,				// how to create
							FILE_ATTRIBUTE_NORMAL |		// file attributes
                             FILE_FLAG_SEQUENTIAL_SCAN,
							NULL );						// handle to template file

	if (myFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	
    f = SetFileTime(
			myFile,              // sets last-write time for file
			LastWriteTime, (LPFILETIME) NULL, 
			LastWriteTime);

	CloseHandle(myFile);
    return f;
}


//Title nach /  suchen und ersetzt durch _
void XconvertServerTitle(char *title)
{
	unsigned int i;
	if (title == NULL)		return; //dürfte eigentlich gar nicht vorkommen - denkst deeee :-)
	if (strlen(title)<1)	return; //dürfte eigentlich gar nicht vorkommen

	for(i=0;i<strlen(title);i++)
	{
		if (title[i] == '/' || title[i] == '\\')
		{
			title[i]='_';
		}
	}

}

// Session importing
int do_import_putty_saved_session_to_plugin_session(int lastInsert_ID, char *import_mode)
{
    static char otherbuf[2048];
    static char *buffer;
    int buflen=0, bufsize=0, i=0, count=0;
	char *ret;
	void *handle;
    struct enumsettings *handleFree;

	if ((handle = enum_settings_start()))
	{
		handleFree=(struct enumsettings*)handle;
	    do {
			if ((lastInsert_ID+count) >= MAX_Server-2) 
			{
				RegCloseKey(handleFree->key);
				return count;
			}
			ret = enum_settings_next(handle, otherbuf, sizeof(otherbuf));
						
			if (ret) 
			{
				struct SftpServerAccountInfo ServerAccountInfo;
				if (loadPuttySectionToSftpPluginServerInfoStruct(&ServerAccountInfo,otherbuf) == 1)
				{
					// OK success
					XconvertServerTitle(otherbuf);
					strcpy(ServerAccountInfo.title,otherbuf);
					ServerAccountInfo.id=(lastInsert_ID+count);
					allServer[lastInsert_ID+count]=ServerAccountInfo;
					if (import_mode[0]=='2')
					{
						//User want no password promt, all the connection are using key auth trought pageant.exe
						allServer[lastInsert_ID+count].use_key_auth=1;
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

void *open_settings_r(char *sessionname)
{
    HKEY subkey1, sesskey;
    char *p;

    p = (char*)malloc(3 * strlen(sessionname) + 1);
    mungestr(sessionname, p);

    if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &subkey1) != ERROR_SUCCESS) 
	{
		sesskey = NULL;
	} else {
		if (RegOpenKey(subkey1, p, &sesskey) != ERROR_SUCCESS) 
		{
			sesskey = NULL;
		}
	}
	RegCloseKey(subkey1);
    free(p);

    return (void *) sesskey;
}

//what the hell does this one?
static void mungestr(char *in, char *out)
{
    int candot = 0;

    while (*in) {
		if (*in == ' ' || *in == '\\' || *in == '*' || *in == '?' ||
			*in == '%' || *in < ' ' || *in > '~' || (*in == '.' && !candot)) 
		{
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

static void gpps(void *handle, char *name, char *def, char *val, int len)
{
    if (!read_setting_s(handle, name, val, len)) 
	{
		strncpy(val, def, len);
		val[len - 1] = '\0';
    }
}

static void gppi(void *handle, char *name, int def, int *i)
{
    *i = read_setting_i(handle, name, def);
}

char *read_setting_s(void *handle, char *key, char *buffer, int buflen)
{
    DWORD type, size;
    size = buflen;

    if (!handle || RegQueryValueEx((HKEY) handle, key, 0, &type, 
		(unsigned char *)buffer, &size) != ERROR_SUCCESS || type != REG_SZ) return NULL;
    else
		return buffer;
}

int read_setting_i(void *handle, char *key, int defvalue)
{
    DWORD type, val, size;
    size = sizeof(val);

    if (!handle || RegQueryValueEx((HKEY) handle, key, 0, &type,
		(BYTE *) & val, &size) != ERROR_SUCCESS || size != sizeof(val) || type != REG_DWORD)
		return defvalue;
    else
		return val;
}

char *enum_settings_next(void *handle, char *buffer, int buflen)
{
    struct enumsettings *e = (struct enumsettings *) handle;
    char *otherbuf;

    otherbuf = (char*)malloc(3 * buflen);
    if (otherbuf && RegEnumKey(e->key, e->i++, otherbuf, 3 * buflen) == ERROR_SUCCESS) 
	{
		unmungestr(otherbuf, buffer, buflen);
		free(otherbuf);
		return buffer;
	}  else {
		free(otherbuf);
		return NULL;
	}
}

static void unmungestr(char *in, char *out, int outlen)
{
    while (*in) 
	{
		if (*in == '%' && in[1] && in[2]) 
		{
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

void *enum_settings_start(void)
{
    struct enumsettings *ret;
    HKEY key;

    if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &key) != ERROR_SUCCESS)
	{
		return NULL;
	}
	
    ret = (struct enumsettings*)malloc(sizeof(*ret));

    if (ret) 
	{
		ret->key = key;
		ret->i = 0;
    }
    return ret;
}

void ServerAccountInfoDefaults(struct SftpServerAccountInfo *ServerAccountInfo)
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
	ServerAccountInfo->port=22;
	ServerAccountInfo->compression=0;
	ServerAccountInfo->use_key_auth=0;
	ServerAccountInfo->dont_ask4_username=0;
	ServerAccountInfo->dont_ask4_password=0;
	ServerAccountInfo->dont_ask4_passphrase=0;
	ServerAccountInfo->is_imported_from_any_datasrc=0; //private
	ServerAccountInfo->is_imported_from_putty_registry=0; //private
	ServerAccountInfo->is_imported_from_sshcom_registry=0; //private
	//Experimental - just a copy from Putty.h:Config struct
	ServerAccountInfo->proxy_type=PROXY_NONE;
	ServerAccountInfo->proxy_host[512]=0;
	ServerAccountInfo->proxy_port=0;
	ServerAccountInfo->proxy_username[0]=0;
	ServerAccountInfo->proxy_password[0]=0;
	ServerAccountInfo->proxy_telnet_command[0]=0;
}

int loadPuttySectionToSftpPluginServerInfoStruct(struct SftpServerAccountInfo *ServerAccountInfo,char *PuttySectionName)
{
	char HostName[1000];
	char Protocol[1000];
	char UserName[1000];
	char PublicKeyFile[4096];
	int  PortNumber;
	int  Compression;
	HKEY sesskey;

	HostName[0]='\0';
	Protocol[0]='\0';
	UserName[0]='\0';
	PortNumber = 22;

	sesskey	= (HKEY)open_settings_r(PuttySectionName);
	if (sesskey != NULL)
	{
		gpps(sesskey, "HostName",   "", HostName, sizeof(HostName));
		gpps(sesskey, "Protocol",   "", Protocol, sizeof(Protocol));
		gpps(sesskey, "UserName",   "", UserName, sizeof(UserName));
		gppi(sesskey, "PortNumber", 22, &PortNumber);
		gppi(sesskey, "Compression", 0, &Compression);
		gpps(sesskey, "PublicKeyFile",   "", PublicKeyFile, sizeof(PublicKeyFile));

		if (strlen(HostName)>0 && strcmp(Protocol,"ssh") == 0)
		{
			ServerAccountInfoDefaults(ServerAccountInfo);
			if (!PortNumber || PortNumber<1 )
			{
				PortNumber=22;
			}

			if (!Compression || Compression==0 )
			{
				Compression=0;
			} else { 
				Compression=1;
			}

			//inherited char*
			strcpy(ServerAccountInfo->host,HostName);
			strcpy(ServerAccountInfo->host_cached,HostName);
			strcpy(ServerAccountInfo->username,UserName);
			strcpy(ServerAccountInfo->username_cached,"");// MUSS strlen=0 sein  wegen anyconnectionactive()

			// inherited int
			ServerAccountInfo->compression=Compression;
			ServerAccountInfo->port=PortNumber;

			// Static
			ServerAccountInfo->use_key_auth=strlen(PublicKeyFile)>0;	
			ServerAccountInfo->dont_ask4_passphrase=0;
			strncpy(ServerAccountInfo->keyfilename, PublicKeyFile, MAX_Server_INFO);
			if (strlen(ServerAccountInfo->username))
			{
				ServerAccountInfo->dont_ask4_username=1;
			} else {
				ServerAccountInfo->dont_ask4_username=0;
			}
					
			ServerAccountInfo->is_imported_from_any_datasrc=1;
			ServerAccountInfo->is_imported_from_putty_registry=1;

			RegCloseKey(sesskey);
			return 1;
		}
	}
	RegCloseKey(sesskey);
	return 0;
}

void make_unique_title_in_allServers(int numServer)
{
	int i;
	char buf[MAX_Server_INFO];
	char buf2[MAX_Server_INFO];

	for(i=0;i<numServer;i++)
	{
		int dbl=0;
		strcpy(buf,allServer[i].title);
		while (1) // :-)
		{
			if (is_title_in_all_server_double(numServer, i, buf)==1 )
			{
				dbl++;
				sprintf(buf2,"%d",(dbl+1));
				strcpy(buf,allServer[i].title);
				strcat(buf," (");
				strcat(buf,buf2);
				strcat(buf,")");
				continue;	
			}
			if (dbl>0)
			{
				strcpy(allServer[i].title,buf);
			}
			break;
		}
	}
}

int is_title_in_all_server_double(int numServer, int currentServerId, char *title)
{
	int i;
	for(i=0;i<numServer;i++)
	{
		if (i != currentServerId && strcmp(allServer[i].title,title) == 0)
		{
			return 1;
		}
	}
	return 0;
}

int do_import_sshcom_saved_session_to_plugin_session(int lastInsert_ID, char *dir_location)
{
	//int dwIndex=1;
	int count=0;
	unsigned int Fnum=0;
	char buf[MAX_Server_INFO];//MAX_Server_INFO soll MAXcmdbuffer sein			
	char buf2[MAX_Server_INFO];//MAX_Server_INFO soll MAXcmdbuffer sein			
	char buf3[MAX_Server_INFO];//MAX_Server_INFO soll MAXcmdbuffer sein			
	char buf4[MAX_Server_INFO];//MAX_Server_INFO soll MAXcmdbuffer sein			
	char buf_divers[MAX_Server_INFO];//MAX_Server_INFO soll MAXcmdbuffer sein			
	char dir_location_PATH[MAX_Server_INFO];//MAX_Server_INFO soll MAXcmdbuffer sein			

	BOOL Next;              // Done searching for files?
	HANDLE  FndHnd = NULL;   // Handle to find data.
	WIN32_FIND_DATA FindDat; // Info on file found.

	if (strlen(dir_location)>0 && dir_location[strlen(dir_location)-1] == '\\')
	{
		dir_location[strlen(dir_location)-1]='\0';
	}
	strcpy(dir_location_PATH,dir_location);
	strcat(dir_location,"\\*.ssh2");
	FndHnd = FindFirstFile(dir_location, &FindDat);
		
	while(FndHnd != INVALID_HANDLE_VALUE)
	{
		Next = FindNextFile(FndHnd, &FindDat); 

		if (!Next) 
		{
			break;
		} else {
			char hostname[MAX_Server_INFO];
			char title[MAX_Server_INFO];
			char username[MAX_Server_INFO];
			char home_dir[MAX_Server_INFO];
			int compression=0;
			int port=22;
			hostname[0]='\0';
			title[0]='\0';
			username[0]='\0';
			home_dir[0]='\0';

			Fnum++;
			sprintf(buf,"%d",Fnum);
			strcpy(buf2,"File");
			strcat(buf2,buf);
			buf3[0]='\0';
			
			if ((lastInsert_ID+count) >= MAX_Server-2) 
			{
				if (FndHnd != INVALID_HANDLE_VALUE)            // If there was anything found, then
					FindClose(FndHnd); // Close the find handle
				return count;
				break;
			}

			(buf3,FindDat.cFileName);
			strcpy(buf4,dir_location_PATH);
			strcat(buf4,"\\");
			strcat(buf4,buf3);

			if (strlen(buf3))
			{
				GetPrivateProfileString ("Connection","Host Name","",buf_divers,MAX_Server_INFO,buf4);
				if (strlen(buf_divers)>2)
				{
					//title
					get_basename_from_Path(title,buf3);
					strcpy(title,(buf3+(strlen(title))));
					XconvertServerTitle(title);
					
					//hostname
					strcpy(hostname,2+buf_divers);

					//username
					GetPrivateProfileString ("Connection","User Name","",buf_divers,MAX_Server_INFO,buf3);
					if (strlen(buf_divers)>2) strcpy(username,2+buf_divers);

					//Compression
					GetPrivateProfileString ("Connection","Compression","0",buf_divers,MAX_Server_INFO,buf3);
					if (strlen(buf_divers)>2 && strcmp(buf_divers+2,"zlib") == 0)
					{
						compression=1;
					} else {
						compression=0;
					}
										
					//Port
					GetPrivateProfileString ("Connection","Port","22",buf_divers,MAX_Server_INFO,buf3);
					if (strlen(buf_divers)>2)
					{
						port = atoi(2+buf_divers);
					} else {
						port=22;
					}
					if (!port ||port <=0) port=22;

					//home dir :-)
					GetPrivateProfileString("Connection","Default SFTP Directory","",buf_divers,MAX_Server_INFO,buf3);
					if (strlen(buf_divers)>2)
					{
						strcpy(home_dir,2+buf_divers);
					}

					//inherited char*
					strcpy(allServer[lastInsert_ID+count].title,title);
					strcpy(allServer[lastInsert_ID+count].host,hostname);
					strcpy(allServer[lastInsert_ID+count].host_cached,hostname);
					strcpy(allServer[lastInsert_ID+count].username,username);
					strcpy(allServer[lastInsert_ID+count].username_cached,"");// MUSS strlen=0 sein  wegen anyconnectionactive()
					strcpy(allServer[lastInsert_ID+count].home_dir,home_dir);

					// inherited int
					allServer[lastInsert_ID+count].compression=compression;
					allServer[lastInsert_ID+count].port=port;

					// Static
					allServer[lastInsert_ID+count].use_key_auth=0;

					if (strlen(allServer[lastInsert_ID+count].username))
					{
						allServer[lastInsert_ID+count].dont_ask4_username=1;
					} else { 
						allServer[lastInsert_ID+count].dont_ask4_username=0;
					}
					
					strcpy(allServer[lastInsert_ID+count].password,"");
					strcpy(allServer[lastInsert_ID+count].password_cached,"");

					allServer[lastInsert_ID+count].is_imported_from_any_datasrc=1;
					allServer[lastInsert_ID+count].is_imported_from_putty_registry=0;
					allServer[lastInsert_ID+count].is_imported_from_sshcom_registry=1;
										
					allServer[lastInsert_ID+count].id=lastInsert_ID+count;
										
					count++;//last comand!!!
				}
			}
		}
	}
	if (FndHnd != INVALID_HANDLE_VALUE)            // If there was anything found, then
		FindClose(FndHnd); // Close the find handle

	return count;
}

int get_custom_users_sftp_inifile_from_reg(char *iniFname)
{
	int ret = -1;
	HKEY subkey1;
    DWORD type, buflen=MAX_PATH;

	if (RegOpenKey(HKEY_CURRENT_USER, PETRICH_TC_REGP , &subkey1) == ERROR_SUCCESS) 
	{
		if (RegQueryValueEx(subkey1, PETRICH_TC_REGP_SFTP_K, 0,&type, (unsigned char *)iniFname, &buflen) == ERROR_SUCCESS)
		{
			ret=0;
		}

	} 
	
	RegCloseKey(subkey1);

	return ret;
}
