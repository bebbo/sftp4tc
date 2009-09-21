/**
* major code change:
* - more object orientated
*/

//disabling that ugly warnings from vc++6.0 about cutting identifiers from 
//template types, like std::map<std::string, fxp_names*>
#pragma warning(disable:4786)

#include <time.h>
#include "stdafx.h"
#include "fsplugin.h"
#include "resource.h"
#include "server.h"

//---------------------------------------------------------------------
// global variables
//---------------------------------------------------------------------
// Plugin's initialization values (TC FS Plugin Parameters)
int gPluginNumber;
ProgressProcType gProgressProc;
LogProcType gLogProc;
RequestProcType gRequestProc;
HWND gMainWin;

HINSTANCE ghThisDllModule;

//---------------------------------------------------------------------
// local defines
//---------------------------------------------------------------------

//Plugin's caption, shown in TC's list
#define FSPLUGIN_CAPTION "Secure FTP Connections"

// modes for find first/next
#define HANDLE__SHOW_SFTP_SERVER 1
#define HANDLE__SHOW_SFTP_DIR    2

//---------------------------------------------------------------------
// static variables, structs, functions
//---------------------------------------------------------------------
// DllMain
static HICON gConnectionIcon;
static HICON gConfigIcon;
static HICON gServerIcon;

static const DWORD S_IFLNK = 0x0A000;
static TCHAR const * const EDIT_CONNECTIONS = TEXT("<edit connections>");

// used to force reloads
static Server * lastServer;
static std::string lastDir;

// transfer modes
static std::map<std::string, std::string> modeExtensions;
static bool transferAscii;


// used for FindFirst / FindNext
typedef struct {
	char mPath[MAX_PATH];
	HANDLE mhSearch;
	size_t mSumIndex;
	size_t mCurrentIndex;
	int mSearchMode;
	my_fxp_names * mCurrentDir;
	bool hideDotNames;
} LastFindStructType;

//---------------------------------------------------------------------
static void toDos(char * p) {
	while (*p) {
		if (*p == '/') *p = '\\';
		++p;
	}
}

//---------------------------------------------------------------------

bool doTransferAscii(std::string const & filename) {
	if (transferAscii)
		return true;
	if (modeExtensions.size() == 0)
		return false;
	// check the extension
	size_t dot = filename.find_last_of('.');
	if (dot == (size_t)-1)
		return false;

	std::string ext = filename.substr(dot);
	std::map<std::string, std::string>::iterator i = modeExtensions.find(ext);
	return i != modeExtensions.end();
}

//---------------------------------------------------------------------

std::string splitPath(std::string & path, std::string const & filePath) {
	size_t i = filePath.find_last_of('/');
	if (i == (size_t)-1) {
		path = "/";
		return filePath;
	}
	++i;
	path = filePath.substr(0, i);
	return filePath.substr(i);
}

//---------------------------------------------------------------------
#define octal_permissions_2_tc_integral(octal_val) (octal_val & 0xfff)

//---------------------------------------------------------------------
unsigned long FileTimeToUnixTime(LPFILETIME ft)
{
	LONGLONG ll = ft->dwHighDateTime;
	ll = (ll << 32) | ft->dwLowDateTime;
	ll = (ll - 116444736000000000) / 10000000;
	return (unsigned long)ll;
}

//---------------------------------------------------------------------

bool UnixTimeToFileTime(unsigned long mtime, LPFILETIME ft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll = Int32x32To64(mtime, 10000000) + 116444736000000000;
	ft->dwLowDateTime = (DWORD)ll;
	ft->dwHighDateTime = ll >> 32;
	return true;
}

//---------------------------------------------------------------------

extern "C" BOOL APIENTRY DllMain(HANDLE _hModule, DWORD ul_reason_for_call, LPVOID )
{
	static int gDllInitialised = 0;
	HINSTANCE hModule = (HINSTANCE)_hModule;
	ghThisDllModule = hModule;
	switch (ul_reason_for_call) {
case DLL_PROCESS_ATTACH:
	{
		if (!gDllInitialised) {
			gConnectionIcon = LoadIcon(hModule, MAKEINTRESOURCE(IDI_ICON1));
			gServerIcon = LoadIcon(hModule, MAKEINTRESOURCE(IDI_ICON_CONNECTION));
			gConfigIcon = LoadIcon(hModule, MAKEINTRESOURCE(IDI_ICON_CONFIG));
		}
		gDllInitialised++;
		break;
	}
case DLL_PROCESS_DETACH:
	{
		gDllInitialised--;
	}
	}
	return TRUE;
}
//---------------------------------------------------------------------

int __stdcall FsInit(int PluginNr, ProgressProcType pProgressProc,
					 LogProcType pLogProc, RequestProcType pRequestProc)
{
	//remember all those values
	gProgressProc = pProgressProc;
	gLogProc = pLogProc;
	gRequestProc = pRequestProc;
	gPluginNumber = PluginNr;

	return 0;
}
//---------------------------------------------------------------------

static HANDLE fillLfWithServer(WIN32_FIND_DATA * FindData, LastFindStructType* lf, size_t n)
{
	lf->mCurrentIndex = n;
	strcpy(FindData->cFileName, n ? Server::getServerName(n - 1) : EDIT_CONNECTIONS);

	FindData->dwFileAttributes = n ? FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000 : 0;
	FindData->dwReserved0 |= S_IFLNK; // Wincmd uses only this one!
	FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
	FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
	lf->mhSearch = INVALID_HANDLE_VALUE;
	return (HANDLE) lf;
}

//---------------------------------------------------------------------

static HANDLE fillLfWithFile(WIN32_FIND_DATA * FindData, LastFindStructType * lf, size_t n)
{	
	//Copy that info to TotalCommander's structure
	my_fxp_names * currentDir = lf->mCurrentDir;
	fxp_name * name	= currentDir->names[n];

	if (lf->hideDotNames) {
		// skip dot names
		for(;;) {
			if (name->filename[0] != '.' || name->filename[1] == 0)
				break;
			++n;
			if (n >= lf->mSumIndex)
				return INVALID_HANDLE_VALUE;
			name = currentDir->names[n];
		}
	}

	lf->mCurrentIndex = n;
	char FileTyp = name->longname[0];
	FindData->dwReserved0 = octal_permissions_2_tc_integral(name->attrs.permissions);

	if (FileTyp == 'd') {
		FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | 0x80000000;
	} else if (FileTyp == 'l') {
		FindData->dwFileAttributes =
			FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY |0x80000000;
		FindData->dwReserved0 |= S_IFLNK; // Wincmd uses only this one!
	} else {
		FindData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL | 0x80000000;
	}

	if (!UnixTimeToFileTime
		(name->attrs.mtime, &FindData->ftLastWriteTime)) {
			FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
			FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
	}

	FindData->nFileSizeHigh =
		(DWORD) name->attrs.size.hi;
	FindData->nFileSizeLow = (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0 :
		(DWORD) name->attrs.size.lo;
	strcpy(FindData->cFileName,
		name->filename);

	return (HANDLE) lf;
}

//---------------------------------------------------------------------

HANDLE __stdcall FsFindFirst(char * fullPath, WIN32_FIND_DATA * FindData)
{
	LastFindStructType * lf = (LastFindStructType*) malloc(sizeof(LastFindStructType));
	lf->mCurrentDir = 0;

	memset(FindData, 0, sizeof(WIN32_FIND_DATA));

	if (strcmp(fullPath, "\\") == 0) {
		// list the connections - always scan PuTTy registry		
		lf->mSearchMode = HANDLE__SHOW_SFTP_SERVER;
		lf->mSumIndex = Server::loadServers() + 1;
		return fillLfWithServer(FindData, lf, 0);
	} 

	// So we have a connection - list selected directory
	lf->mSearchMode = HANDLE__SHOW_SFTP_DIR;

	// get the server instance
	std::string remotePath;
	Server * server = Server::findServer(remotePath, fullPath);
	if (server) {
		if (server == lastServer && remotePath == lastDir) {
			server->invalidateDirContent(remotePath);
		} else {
			lastServer = server;
			lastDir = remotePath;
		}

		// get the dir content
		my_fxp_names * currentDir = server->getDirContent(remotePath);
		if (currentDir) {
			lf->mCurrentDir = currentDir;
			lf->mSumIndex = currentDir->nnames;
			lf->hideDotNames = server->isHideDotNames();

			return fillLfWithFile(FindData, lf, 0);
		}
	}

	free(lf);
	return INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------

BOOL __stdcall FsFindNext(HANDLE Hdl, WIN32_FIND_DATA * FindData)
{
	LastFindStructType* lf = (LastFindStructType*) Hdl;

	if (++lf->mCurrentIndex >= lf->mSumIndex)
		return false;             //this is the last entry 

	//is it a connection listing?
	if (lf->mSearchMode == HANDLE__SHOW_SFTP_SERVER) {
		return INVALID_HANDLE_VALUE != fillLfWithServer(FindData, lf, lf->mCurrentIndex);
	}

	return INVALID_HANDLE_VALUE != fillLfWithFile(FindData, lf, lf->mCurrentIndex);
}
//---------------------------------------------------------------------

int __stdcall FsFindClose(HANDLE Hdl)
{
	LastFindStructType* lf = (LastFindStructType*) Hdl;

	if (lf != INVALID_HANDLE_VALUE) {
		free(lf); //now we can free this
	}

	return 0;
}
//---------------------------------------------------------------------

BOOL __stdcall FsMkDir(char * fullPath)
{
	// disable automatic reloads
	lastServer = 0;

	std::string remotePath;
	Server * server = Server::findServer(remotePath, fullPath);
	if (!server)
		return FS_EXEC_ERROR;
	
	return server->cmdMkDir(remotePath);
}
//---------------------------------------------------------------------

/**
* Invoked from Total Commander's command line - or by pressing enter.
*/ 
int __stdcall FsExecuteFile(HWND MainWin, char * fullRemoteName, char * verb)
{
	gMainWin = MainWin;
	if (!verb || !*verb)
		return FS_EXEC_ERROR;

	// disable automatic reloads
	lastServer = 0;

	std::string cmd = verb;
	std::string fullRemotePath;
	if (fullRemoteName && *fullRemoteName) fullRemotePath = fullRemoteName + 1;

	// set the global mode!? -- ignore, since SFTP does not support it.
	if (cmd.length() > 5 && cmd.substr(0, 4) == "MODE") {
		if (cmd[5] == 'A')
			transferAscii = true;
		else {
			modeExtensions.clear();
			transferAscii = false;
			if (cmd[5] == 'X') {
				size_t start = (size_t)-1;
				for (size_t i = 6; i < cmd.size(); ++i)
				{
					if (cmd[i] == '.') start = i;
					else if (cmd[i] <= 32) {
						size_t len = i - start;
						std::string x = cmd.substr(start, len);
						modeExtensions.insert(std::make_pair(x, x));
						start = (size_t)-1;
					}
				}
				if (start != (size_t)-1) {
					std::string x = cmd.substr(start);
					modeExtensions.insert(std::make_pair(x, x));
				}
			}
		}
		return FS_EXEC_OK;
	}

	std::string remotePath;
	if (cmd == "open") {
		size_t slash = fullRemotePath.find_first_of('\\');
		if (slash != (size_t)-1)
			return FS_EXEC_YOURSELF;

		// it's a server name
		if (fullRemotePath != EDIT_CONNECTIONS) {
			// get or create the server
			Server * server = Server::findServer(remotePath, fullRemotePath.c_str());
			if (!server)
				return FS_EXEC_ERROR;
			
			// connect and get the home folder
			std::string response;
			if (!server->doCommand("~", response))
				return FS_EXEC_ERROR;

			// return the full remote path and force reload of dir
			fullRemoteName[0] = '/';
			strcpy(fullRemoteName + 1, server->getName().c_str());
			strcat(fullRemoteName, response.c_str());
			toDos(fullRemoteName);

			return FS_EXEC_SYMLINK;
		}

		Server server("");
		Sftp4tc const * const session = server.doConfig();
		// force reload
		if (session && session->selectedSession) {
			strcpy(fullRemoteName + 1, session->selectedSession);
			strcat(fullRemoteName, session->homeDir);
			toDos(fullRemoteName);
		} else {
			fullRemoteName[1] = '\0';
		}
        return FS_EXEC_SYMLINK;
	}

	
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server) {
		return FS_EXEC_ERROR;
	}

	if (cmd == "properties") {
		size_t slash = fullRemotePath.find_first_of('\\');
		if (slash != (size_t)-1) {
			// invoke da menu
			::PostMessage(gMainWin, WM_COMMAND, 2, 0);
			return FS_EXEC_OK;
		}

		Sftp4tc const * const session = server->doSelfConfig();
		if (session) {
			server->configure(session);
		}
		return FS_EXEC_OK;
	}

	if (cmd.length() >= 5 && cmd.substr(0,5) == "chmod") {
		size_t last = remotePath.find_last_of('/') + 1;
		std::string fileName = remotePath.substr(last);
		std::string remoteDir = remotePath.substr(0, last);

		std::string cd = std::string("cd \"") + remoteDir + "\"";
		cmd = cmd + " " + fileName;
		if (server->doCommand(cd) &&
			server->doCommand(cmd)) {
			server->updateFileAttr(remoteDir, fileName, cmd.substr(5));
			return FS_EXEC_OK;
		}
		return FS_EXEC_ERROR;
	}

	if (cmd.length() >= 6 && cmd.substr(0, 6) == "quote ") {
		cmd = cmd.substr(6);

		// set transfer mode for commands
		if ((cmd.length() > 4 && (cmd.substr(4) == "get "
			|| cmd.substr(4) == "put "))
			|| (cmd.length() > 6 && (cmd.substr(6) == "reget "
			|| cmd.substr(6) == "reput "))) {
				server->setTransferAscii(doTransferAscii(cmd));
		}
		
		// change current dir and execute command
		std::string cd = std::string("cd \"") + remotePath + "\"";
		if (server->doCommand(cd) &&
			server->doCommand(cmd)) {
				if (cmd.length() > 2 && cmd.substr(0, 2) == "cd") {
				std::string nd = remotePath;
				if (server->doCommand("pwd", nd)) {
					if (nd.size() == 0 || nd[nd.size() - 1] != '/')
						nd += '/';
					if (remotePath != nd) {
						std::string toDir = "/" + server->getName() + nd;
						strcpy(fullRemoteName, toDir.c_str());
						toDos(fullRemoteName);
						return FS_EXEC_SYMLINK;
					}
				}
			}
			return FS_EXEC_OK;
		}
		return FS_EXEC_ERROR;
	}

	DBGPRINT(("not yet implemented: %s | %d\r\n", verb, fullRemoteName));

	return FS_EXEC_ERROR;
}
//---------------------------------------------------------------------
extern bool firstHalf;
extern bool secondHalf;

int __stdcall FsRenMovFile(char *fullOldName, char *fullNewName, BOOL move,
						   BOOL overWrite, RemoteInfoStruct * ri)
{
	// disable automatic reloads
	lastServer = 0;

	std::string oldRemotePath;
	Server * oldServer = Server::findServer(oldRemotePath, fullOldName);
	std::string newRemotePath;
	Server * newServer = Server::findServer(newRemotePath, fullNewName);

	DBGPRINT(("renmove %s -> %s\r\n", fullOldName, fullNewName));

	bool exists = newServer->remoteFileExists(newRemotePath);
	// no action but signal resume is allowed
	if (exists && !overWrite)
		return FS_FILE_EXISTS;

	// short way on same server
	if (oldServer == newServer && move) {
		if (exists)
			newServer->cmdRm(newRemotePath);
		if (newServer->cmdMove(oldRemotePath, newRemotePath))
			return FS_FILE_OK;
		return FS_EXEC_ERROR;
	}

	// slow way
	char td[512];
	GetTempPath(512, td);
	char tf[512];
	GetTempFileName(td, "sftp4tc", 0, tf);

	firstHalf = true;
	int flags = (move ? FS_COPYFLAGS_MOVE : 0) | (overWrite ? FS_COPYFLAGS_OVERWRITE : 0);
	int ret = FsGetFile(fullOldName, tf, flags | FS_COPYFLAGS_OVERWRITE, ri);
	if (FS_FILE_OK == ret) {
		secondHalf = true;
		ret = FsPutFile(tf, fullNewName, flags);
	}
	firstHalf = secondHalf = false;

	OFSTRUCT ofs;
	OpenFile(tf, &ofs, OF_DELETE);

	return ret;
}

//---------------------------------------------------------------------

int __stdcall FsGetFile(char *fullRemoteName, char *LocalName, int CopyFlags,
						RemoteInfoStruct * ri)
{
	// disable automatic reloads
	lastServer = 0;

	// check flags and decide what to do
	bool overwrite = 0 != (CopyFlags & FS_COPYFLAGS_OVERWRITE);
	bool resume = 0 != (CopyFlags & FS_COPYFLAGS_RESUME);
	bool move = 0 != (CopyFlags & FS_COPYFLAGS_MOVE);

	// check file existance
	OFSTRUCT ofs;
	HFILE hf = OpenFile(LocalName, &ofs, OF_EXIST);
	bool exists = hf != HFILE_ERROR;
	if (exists) _lclose(hf);

	// no action but signal resume is allowed
	if (exists && !overwrite && !resume)
		return FS_FILE_EXISTSRESUMEALLOWED;

	std::string remotePath;
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server) return FS_FILE_NOTFOUND;

	// remove the file if overwrite is set
	if (exists && overwrite) {
		OpenFile(LocalName, &ofs, OF_DELETE);
		exists = false;
	}
	
	// set transfer mode
	server->setTransferAscii(doTransferAscii(remotePath));

	// get it
	if (!server->cmdGet(remotePath, LocalName, resume && exists))
		return FS_FILE_READERROR;
		
	if (!move)
		return FS_FILE_OK;

	if (!server->cmdRm(remotePath))
		return FS_FILE_NOTFOUND;
	return FS_FILE_OK;
}
//---------------------------------------------------------------------

int __stdcall FsPutFile(char * localName, char *fullRemoteName, int CopyFlags)
{
	// disable automatic reloads
	lastServer = 0;

	bool overwrite = 0 != (CopyFlags & FS_COPYFLAGS_OVERWRITE);
	bool resume = 0 != (CopyFlags & FS_COPYFLAGS_RESUME);
	bool move = 0 != (CopyFlags & FS_COPYFLAGS_MOVE);

	std::string remotePath;
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server) return FS_FILE_NOTFOUND;

	bool exists = server->remoteFileExists(remotePath);

	if (!overwrite && !resume) {
		if (exists) return FS_FILE_EXISTSRESUMEALLOWED;
	}

	// set transfer mode
	server->setTransferAscii(doTransferAscii(remotePath));

	if (!server->cmdPut(localName, remotePath, resume && exists))
		return FS_FILE_WRITEERROR;

	if (!move)
		return FS_FILE_OK;

	OFSTRUCT ofs;
	HFILE hf = OpenFile(localName, &ofs, OF_DELETE);
	if (hf != HFILE_ERROR) _lclose(hf);

	return FS_FILE_OK;

}

//---------------------------------------------------------------------

BOOL __stdcall FsDeleteFile(char *fullRemoteName)
{
	// disable automatic reloads
	lastServer = 0;

	std::string remotePath;
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server)
		return FS_FILE_NOTFOUND;

	if (remotePath == "/")
		return FS_FILE_NOTSUPPORTED;

	return server->cmdRm(remotePath);
}

//---------------------------------------------------------------------

BOOL __stdcall FsRemoveDir(char *fullRemoteName)
{
	// disable automatic reloads
	lastServer = 0;

	std::string remotePath;
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server)
		return FS_FILE_NOTFOUND;

	if (remotePath == "/")
		return FS_FILE_NOTSUPPORTED;

	return server->cmdRmDir(remotePath);
}

//---------------------------------------------------------------------

BOOL __stdcall FsDisconnect(char * disconnectRoot)
{
	if (*disconnectRoot == '\\') ++disconnectRoot;
	if (Server::disconnectServer(disconnectRoot)) {
		return true;
	}

	std::string err = std::string("no connection to disconnect: '") + disconnectRoot + "'";
	gLogProc(gPluginNumber, MSGTYPE_DISCONNECT, err.c_str());

	return false;
}

//---------------------------------------------------------------------

void __stdcall FsGetDefRootName(char * defRootName, int maxlen)
{
	if (--maxlen >= 0) {
		lstrcpyn(defRootName, FSPLUGIN_CAPTION, maxlen);
		defRootName[maxlen] = 0;
	}
}

//---------------------------------------------------------------------

void __stdcall FsStatusInfo(char *RemoteDir, int InfoStartEnd,
							int InfoOperation)
{
	switch (InfoOperation) {
		case FS_STATUS_OP_DELETE:
			{
				//			if (strcmp(RemoteDir, "\\") == 0) // Deleting connection!
				{
					//				gDeleteOnlyConnection = (InfoStartEnd == FS_STATUS_START);
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
				//			gAcceptRefresh = true;
			}
	}
}

//---------------------------------------------------------------------

int __stdcall FsExtractCustomIcon(char * fullRemoteName, int extractFlags, HICON* theIcon)
{
	std::string remoteName = fullRemoteName + 1;
	if (remoteName.length() == 0) {
		*theIcon = gConnectionIcon;
		return FS_ICON_EXTRACTED;
	}
	if (remoteName == EDIT_CONNECTIONS) {
		*theIcon = gConfigIcon;
		return FS_ICON_EXTRACTED;
	}
	if (remoteName == "..\\")
		return FS_ICON_USEDEFAULT;
	if (remoteName.find('\\') == remoteName.length() - 1) {
		*theIcon = gServerIcon;
		return FS_ICON_EXTRACTED;
	}
	return FS_ICON_USEDEFAULT;
}

