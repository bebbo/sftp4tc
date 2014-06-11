/**
 * major code change:
 * - more object orientated
 */

//disabling that ugly warnings from vc++6.0 about cutting identifiers from
//template types, like std::map<bstring, fxp_names*>
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
int gCryptoNumber;
tProgressProcW gProgressProc;
tLogProcW gLogProc;
tRequestProcW gRequestProc;

// non unicode
tProgressProc aProgressProc;
tLogProc aLogProc;
tRequestProc aRequestProc;

HWND gMainWin;

HINSTANCE ghThisDllModule;

DWORD gMainThread;

tCryptProcW gCryptProc;

extern int lastPPR;

static int quickConnectionCount;

HANDLE global = CreateMutex(0, 0, 0);

//---------------------------------------------------------------------
// local defines
//---------------------------------------------------------------------

//Plugin's caption, shown in TC's list
#define FSPLUGIN_CAPTION "Secure FTP Connections"
#define WFSPLUGIN_CAPTION L"Secure FTP Connections"

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
static bstring lastDir;

// transfer modes
static std::map<bstring, bstring> modeExtensions;
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
static void toDos(bchar * p) {
	while (*p) {
		if (*p == TEXT('/'))
			*p = TEXT('\\');
		++p;
	}
}

//---------------------------------------------------------------------
#ifdef UNICODE
bchar * qudConvert(wchar_t * dst, char const * src, unsigned int len) {
	wchar_t * s = dst;
	while (--len > 0) {
		wchar_t c = 0xff & *src++;
		if (!c)
		break;
		*dst++ = c;
	}
	*dst = 0;
	return s;
}
char * qudConvert(char * dst, wchar_t const * src, unsigned int len) {
	char * s = dst;
	while (--len > 0) {
		char c = (char)*src++;
		if (!c)
		break;
		*dst++ = c;
	}
	*dst = 0;
	return s;
}
#endif
//---------------------------------------------------------------------
/***********************************************************************************************/

void copyfinddatawa(WIN32_FIND_DATAA *lpFindFileDataA,
		WIN32_FIND_DATAW *lpFindFileDataW) {
	qudConvert(lpFindFileDataA->cAlternateFileName,
			lpFindFileDataW->cAlternateFileName,
			sizeof(lpFindFileDataA->cAlternateFileName) - 1);
	qudConvert(lpFindFileDataA->cFileName, lpFindFileDataW->cFileName,
			sizeof(lpFindFileDataA->cFileName) - 1);
	lpFindFileDataA->dwFileAttributes = lpFindFileDataW->dwFileAttributes;
	lpFindFileDataA->dwReserved0 = lpFindFileDataW->dwReserved0;
	lpFindFileDataA->dwReserved1 = lpFindFileDataW->dwReserved1;
	lpFindFileDataA->ftCreationTime = lpFindFileDataW->ftCreationTime;
	lpFindFileDataA->ftLastAccessTime = lpFindFileDataW->ftLastAccessTime;
	lpFindFileDataA->ftLastWriteTime = lpFindFileDataW->ftLastWriteTime;
	lpFindFileDataA->nFileSizeHigh = lpFindFileDataW->nFileSizeHigh;
	lpFindFileDataA->nFileSizeLow = lpFindFileDataW->nFileSizeLow;
}

void copyfinddataaw(WIN32_FIND_DATAW *lpFindFileDataW,
		WIN32_FIND_DATAA *lpFindFileDataA) {
	qudConvert(lpFindFileDataW->cAlternateFileName,
			lpFindFileDataA->cAlternateFileName,
			sizeof(lpFindFileDataA->cAlternateFileName) - 1);
	qudConvert(lpFindFileDataW->cFileName, lpFindFileDataA->cFileName,
			sizeof(lpFindFileDataA->cFileName) - 1);
	lpFindFileDataW->dwFileAttributes = lpFindFileDataA->dwFileAttributes;
	lpFindFileDataW->dwReserved0 = lpFindFileDataA->dwReserved0;
	lpFindFileDataW->dwReserved1 = lpFindFileDataA->dwReserved1;
	lpFindFileDataW->ftCreationTime = lpFindFileDataA->ftCreationTime;
	lpFindFileDataW->ftLastAccessTime = lpFindFileDataA->ftLastAccessTime;
	lpFindFileDataW->ftLastWriteTime = lpFindFileDataA->ftLastWriteTime;
	lpFindFileDataW->nFileSizeHigh = lpFindFileDataA->nFileSizeHigh;
	lpFindFileDataW->nFileSizeLow = lpFindFileDataA->nFileSizeLow;
}

/***********************************************************************************************/

int __stdcall ProgressProcT(int PluginNr, WCHAR* SourceName, WCHAR* TargetName,
		int PercentDone) {
	char buf1[MAX_PATH], buf2[MAX_PATH];
	return aProgressProc(PluginNr, qudConvert(buf1, SourceName, MAX_PATH - 1),
			qudConvert(buf2, TargetName, MAX_PATH - 1), PercentDone);
}

void __stdcall LogProcT(int PluginNr, int MsgType, WCHAR const* LogString) {
	char buf[1024];
	aLogProc(PluginNr, MsgType, qudConvert(buf, LogString, sizeof(buf) - 1));

}

BOOL __stdcall RequestProcT(int PluginNr, int RequestType, WCHAR* CustomTitle,
WCHAR* CustomText, WCHAR* ReturnedText, int maxlen) {
	char buf1[MAX_PATH], buf2[MAX_PATH], buf3[MAX_PATH];
	char* preturn = qudConvert(buf3, ReturnedText, MAX_PATH - 1);
	BOOL retval = aRequestProc(PluginNr, RequestType,
			qudConvert(buf1, CustomTitle, MAX_PATH - 1),
			qudConvert(buf2, CustomText, MAX_PATH - 1), preturn, maxlen);
	if (retval && preturn)
		qudConvert(ReturnedText, preturn, maxlen);
	return retval;
}

bool doTransferAscii(bstring const & filename) {
	WLock wlock(global);
	if (transferAscii)
		return true;
	if (modeExtensions.size() == 0)
		return false;
	// check the extension
	size_t dot = filename.find_last_of('.');
	if (dot == (size_t) -1)
		return false;

	bstring ext = filename.substr(dot);
	std::map<bstring, bstring>::iterator i = modeExtensions.find(ext);
	return i != modeExtensions.end();
}

//---------------------------------------------------------------------

bstring splitPath(bstring & path, bstring const & filePath) {
	size_t i = filePath.find_last_of('/');
	if (i == (size_t) -1) {
		path = TEXT("/");
		return filePath;
	}
	++i;
	path = filePath.substr(0, i);
	return filePath.substr(i);
}

//---------------------------------------------------------------------
#define octal_permissions_2_tc_integral(octal_val) (octal_val & 0xfff)

//---------------------------------------------------------------------
unsigned long FileTimeToUnixTime(LPFILETIME ft) {
	LONGLONG ll = ft->dwHighDateTime;
	ll = (ll << 32) | ft->dwLowDateTime;
	ll = (ll - 116444736000000000) / 10000000;
	return (unsigned long) ll;
}

//---------------------------------------------------------------------

bool UnixTimeToFileTime(unsigned long mtime, LPFILETIME ft) {
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll = Int32x32To64(mtime, 10000000) + 116444736000000000;
	ft->dwLowDateTime = (DWORD) ll;
	ft->dwHighDateTime = ll >> 32;
	return true;
}

//---------------------------------------------------------------------

extern "C" BOOL APIENTRY DllMain(HANDLE _hModule, DWORD ul_reason_for_call,
		LPVOID) {
	static int gDllInitialised = 0;
	HINSTANCE hModule = (HINSTANCE) _hModule;
	ghThisDllModule = hModule;
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH: {
		if (!gDllInitialised) {
			gConnectionIcon = LoadIcon(hModule, MAKEINTRESOURCE(IDI_ICON1));
			gServerIcon = LoadIcon(hModule,
					MAKEINTRESOURCE(IDI_ICON_CONNECTION));
			gConfigIcon = LoadIcon(hModule, MAKEINTRESOURCE(IDI_ICON_CONFIG));
		}
		gDllInitialised++;
		break;
	}
	case DLL_PROCESS_DETACH: {
		gDllInitialised--;
	}
	}
	return TRUE;
}
//---------------------------------------------------------------------

int __stdcall FsInit(int PluginNr, tProgressProc pProgressProc,
		tLogProc pLogProc, tRequestProc pRequestProc) {
	//remember all those values
	aProgressProc = pProgressProc;
	aLogProc = pLogProc;
	aRequestProc = pRequestProc;

	gProgressProc = ProgressProcT;
	gLogProc = LogProcT;
	gRequestProc = RequestProcT;

	gPluginNumber = PluginNr;

	gMainThread = ::GetCurrentThreadId();
#ifdef UNICODE
	return 1;
#else
	return 0;
#endif
}

int __stdcall FsInitW(int PluginNr, tProgressProcW pProgressProc,
		tLogProcW pLogProc, tRequestProcW pRequestProc) {
	//remember all those values
	gProgressProc = pProgressProc;
	gLogProc = pLogProc;
	gRequestProc = pRequestProc;
	gPluginNumber = PluginNr;

	gMainThread = ::GetCurrentThreadId();
#ifndef UNICODE
	return 1;
#else
	return 0;
#endif
}
//---------------------------------------------------------------------

static HANDLE fillLfWithServer(WIN32_FIND_DATA * FindData,
		LastFindStructType* lf, size_t n) {
	lf->mCurrentIndex = n;
	bstrcpy(FindData->cFileName,
			n ? Server::getServerName(n - 1) : EDIT_CONNECTIONS);
	FindData->dwFileAttributes =
			n ? FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000 : 0;
	FindData->dwReserved0 |= S_IFLNK; // Wincmd uses only this one!
	FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
	FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
	lf->mhSearch = INVALID_HANDLE_VALUE;
	return (HANDLE) lf;
}

//---------------------------------------------------------------------

static HANDLE fillLfWithFile(WIN32_FIND_DATA * FindData,
		LastFindStructType * lf, size_t n) {
			if (n >= lf->mSumIndex)
				return INVALID_HANDLE_VALUE ;
	//Copy that info to TotalCommander's structure
	my_fxp_names * currentDir = lf->mCurrentDir;
	fxp_name * name = currentDir->names[n];

	if (lf->hideDotNames) {
		// skip dot names
		for (;;) {
#ifdef UNICODE
			if (name->ucFilename[0] != '.' || name->ucFilename[1] == 0)
			break;
#else
			if (name->filename[0] != '.' || name->filename[1] == 0)
				break;
#endif
			++n;
			if (n >= lf->mSumIndex)
				return INVALID_HANDLE_VALUE ;
			name = currentDir->names[n];
		}
	}

	lf->mCurrentIndex = n;
	char FileTyp = name->longname[0];
	FindData->dwReserved0 = octal_permissions_2_tc_integral(
			name->attrs.permissions);

	if (FileTyp == 'd') {
		FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | 0x80000000;
	} else if (FileTyp == 'l') {
		FindData->dwFileAttributes = FILE_ATTRIBUTE_REPARSE_POINT | 0x80000000;
		FindData->dwReserved0 |= S_IFLNK; // Wincmd uses only this one!
	} else {
		FindData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL | 0x80000000;
	}

	if (!UnixTimeToFileTime(name->attrs.mtime, &FindData->ftLastWriteTime)) {
		FindData->ftLastWriteTime.dwHighDateTime = 0xFFFFFFFF;
		FindData->ftLastWriteTime.dwLowDateTime = 0xFFFFFFFE;
	}

	FindData->nFileSizeHigh = (DWORD) name->attrs.size.hi;
	FindData->nFileSizeLow =
			(FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ?
					0 : (DWORD) name->attrs.size.lo;
#ifdef UNICODE
	wcscpy(FindData->cFileName, name->ucFilename);
#else
	strcpy(FindData->cFileName, name->filename);
#endif

	return (HANDLE) lf;
}

//---------------------------------------------------------------------
HANDLE __stdcall FsFindFirstW(bchar * fullPath, WIN32_FIND_DATA * FindData) {
	LastFindStructType * lf = (LastFindStructType*) malloc(
			sizeof(LastFindStructType));
	lf->mCurrentDir = 0;

	memset(FindData, 0, sizeof(WIN32_FIND_DATA));

	if (bstrcmp(fullPath, TEXT("\\")) == 0) {
		WLock wlock(global);
		// list the connections - always scan PuTTy registry
		lf->mSearchMode = HANDLE__SHOW_SFTP_SERVER;
		lf->mSumIndex = Server::loadServers() + 1;
		return fillLfWithServer(FindData, lf, 0);
	}

	// So we have a connection - list selected directory
	lf->mSearchMode = HANDLE__SHOW_SFTP_DIR;

	// get the server instance
	bstring remotePath;
	Server * server = Server::findServer(remotePath, fullPath);
	if (server) {
			WLock wlock(server->mutex);
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
	return INVALID_HANDLE_VALUE ;
}
HANDLE __stdcall FsFindFirst(char * fullPath, WIN32_FIND_DATAA * FindData) {
	WIN32_FIND_DATAW findDataW;
	bchar f[1024];
	qudConvert(f, fullPath, 1023);
	HANDLE retval = FsFindFirstW(f, &findDataW);
	if (retval != INVALID_HANDLE_VALUE)
		copyfinddatawa(FindData, &findDataW);
	return retval;
}
//---------------------------------------------------------------------

BOOL __stdcall FsFindNextW(HANDLE Hdl, WIN32_FIND_DATA * FindData) {
	LastFindStructType* lf = (LastFindStructType*) Hdl;

	if (++lf->mCurrentIndex >= lf->mSumIndex)
		return false; //this is the last entry

	//is it a connection listing?
	if (lf->mSearchMode == HANDLE__SHOW_SFTP_SERVER) {
		return INVALID_HANDLE_VALUE
				!= fillLfWithServer(FindData, lf, lf->mCurrentIndex);
	}

	return INVALID_HANDLE_VALUE
			!= fillLfWithFile(FindData, lf, lf->mCurrentIndex);
}
BOOL __stdcall FsFindNext(HANDLE Hdl, WIN32_FIND_DATAA * FindData) {
	WIN32_FIND_DATAW findDataW;
	copyfinddataaw(&findDataW, FindData);
	BOOL retval = FsFindNextW(Hdl, &findDataW);
	if (retval)
		copyfinddatawa(FindData, &findDataW);
	return retval;
}
//---------------------------------------------------------------------

int __stdcall FsFindClose(HANDLE Hdl) {
	LastFindStructType* lf = (LastFindStructType*) Hdl;

	if (lf != INVALID_HANDLE_VALUE) {
		free(lf); //now we can free this
	}

	return 0;
}
//---------------------------------------------------------------------

BOOL __stdcall FsMkDirW(bchar * fullPath) {
	// disable automatic reloads
	lastServer = 0;

	bstring remotePath;
	Server * server = Server::findServer(remotePath, fullPath);
	if (!server)
		return FS_EXEC_ERROR;

	WLock(server->mutex);
	return server->cmdMkDir(remotePath);
}
BOOL __stdcall FsMkDir(char * fullPath) {
	bchar f[1024];
	qudConvert(f, fullPath, 1023);
	return FsMkDirW(f);
}
//---------------------------------------------------------------------

/**
 * Invoked from Total Commander's command line - or by pressing enter.
 */
int __stdcall FsExecuteFileW(HWND MainWin, bchar * fullRemoteName,
		bchar * verb) {
	gMainWin = MainWin;
	if (!verb || !*verb)
		return FS_EXEC_ERROR;

	// disable automatic reloads
	lastServer = 0;

	bstring cmd = verb;
	bstring fullRemotePath;
	if (fullRemoteName && *fullRemoteName)
		fullRemotePath = fullRemoteName + 1;

	// set the global mode!? -- ignore, since SFTP does not support it.
	if (cmd.length() > 5 && cmd.substr(0, 4) == TEXT("MODE")) {
		WLock wlock(global);
		if (cmd[5] == 'A')
			transferAscii = true;
		else {
			modeExtensions.clear();
			transferAscii = false;
			if (cmd[5] == 'X') {
				size_t start = (size_t) -1;
				for (size_t i = 6; i < cmd.size(); ++i) {
					if (cmd[i] == '.')
						start = i;
					else if (cmd[i] <= 32) {
						size_t len = i - start;
						bstring x = cmd.substr(start, len);
						modeExtensions.insert(std::make_pair(x, x));
						start = (size_t) -1;
					}
				}
				if (start != (size_t) -1) {
					bstring x = cmd.substr(start);
					modeExtensions.insert(std::make_pair(x, x));
				}
			}
		}
		return FS_EXEC_OK;
	}

	bstring remotePath;
	if (cmd == TEXT("open")) {
		size_t slash = fullRemotePath.find_first_of('\\');
		if (slash != (size_t) -1) {
			// get or create the server
			Server * server = Server::findServer(remotePath,
					fullRemotePath.c_str());
			if (!server)
				return FS_EXEC_ERROR;

			WLock wlock(server->mutex);

			// distinguish between link and something else?
			size_t last = remotePath.find_last_of('/') + 1;
			bstring fileName = remotePath.substr(last);
			bstring remoteDir = remotePath.substr(0, last);
			my_fxp_names * dir = server->getDirContent(remoteDir);
			if (!dir)
				return FS_EXEC_SYMLINK;
			for (int i = 0; i < dir->nnames; ++i) {
				fxp_name * file = dir->names[i];
#ifdef UNICODE
				if (fileName == file->ucFilename) {
#else
				if (fileName == file->filename) {
#endif
					if (file->longname[0] == 'l')
						return FS_EXEC_SYMLINK;
					return FS_EXEC_YOURSELF;
				}
			}
			return FS_EXEC_SYMLINK;
		}

		// it's a server name
		if (fullRemotePath != EDIT_CONNECTIONS) {
			// get or create the server
			Server * server = Server::findServer(remotePath,
					fullRemotePath.c_str());
			if (!server)
				return FS_EXEC_ERROR;

			WLock wlock(server->mutex);
			/*
			 if (!server->connect()) {
			 Server::removeServer(server->getName().c_str());
			 return FS_EXEC_ERROR;
			 }
			 */
			// simply get the home folder and append. Otherwise the progress bar hides
			// connect and get the home folder
			bstring response;
			if (!server->getHomeDir(response))
				return FS_EXEC_ERROR;

			// return the full remote path and force reload of dir
			fullRemoteName[0] = '/';
			bstrcpy(fullRemoteName + 1, server->getName().c_str());
			bstrcat(fullRemoteName, response.c_str());
			toDos(fullRemoteName);

			return FS_EXEC_SYMLINK;
		}

		// It's edit connections. Create a temp server - maybe we keep it.
		Server server(TEXT(""), TEXT("~"), 0);

		// popup config dialog
		Sftp4tc * const cfg = server.doConfig();
		if (!cfg) {
			fullRemoteName[1] = 0;
			return FS_EXEC_SYMLINK;
		}

		// is there a session?
		bstring sessionName;
#ifdef UNICODE
		BCONVERT(wchar_t, 256, sessionName, cfg->selectedSession)
#else
		sessionName = cfg->selectedSession;
#endif
		bchar buf[16];
		bstring orgSessionName = sessionName;

		if (sessionName.length() == 0) {
			// no create a name from host
			sessionName = TEXT("quick connection (");
			bsprintf(buf, TEXT("%d) "), ++quickConnectionCount);
			bchar const * host;
#ifdef UNICODE
			BCONVERT(wchar_t, 256, host, cfg->host)
#else
			host = cfg->host;
#endif
			sessionName = sessionName + buf + host;
		} else if (!cfg->saved) {
			bsprintf(buf, TEXT("%d) "), ++quickConnectionCount);
			sessionName = TEXT("(") + (buf + sessionName);
		}
		bstrcpy(fullRemoteName + 1, sessionName.c_str());
		bchar const * homeDir;
#ifdef UNICODE
		BCONVERT(wchar_t, 512, homeDir, cfg->homeDir)
#else
		homeDir = cfg->homeDir;
#endif
		bstrcat(fullRemoteName, homeDir);
		toDos(fullRemoteName);

		Server * newServer = new Server(sessionName, orgSessionName, ::GetCurrentThreadId());
		Server::insertServer(sessionName, newServer);
		newServer->configure(cfg);

		if (!newServer->connect()) {
			Server::removeServer(sessionName.c_str());
			return FS_EXEC_ERROR;
		}

		return FS_EXEC_SYMLINK;
	}

	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server) {
		return FS_EXEC_ERROR;
	}
		
	WLock wlock(server->mutex);
	if (cmd == TEXT("properties")) {
		size_t slash = fullRemotePath.find_first_of('\\');
		if (slash != (size_t) -1) {
			// invoke da menu
			::PostMessage(gMainWin, WM_COMMAND, 2, 0);
			return FS_EXEC_OK;
		}

		Sftp4tc * const session = server->doSelfConfig();
		if (session) {
			server->configure(session);
		}
		return FS_EXEC_OK;
	}

	if (cmd.length() >= 5 && cmd.substr(0, 5) == TEXT("chmod")) {
		size_t last = remotePath.find_last_of('/') + 1;
		bstring fileName = remotePath.substr(last);
		bstring remoteDir = remotePath.substr(0, last);

		bstring cd = bstring(TEXT("cd \"")) + remoteDir + TEXT("\"");
		cmd = cmd + TEXT(" ") + fileName;
		if (server->doCommand(cd) && server->doCommand(cmd)) {
			server->updateFileAttr(remoteDir, fileName, cmd.substr(5));
			return FS_EXEC_OK;
		}
		return FS_EXEC_ERROR;
	}

	if (cmd.length() >= 6 && cmd.substr(0, 6) == TEXT("quote ")) {
		cmd = cmd.substr(6);

		// set transfer mode for commands
		if ((cmd.length() > 4
				&& (cmd.substr(4) == TEXT("get ")
						|| cmd.substr(4) == TEXT("put ")))
				|| (cmd.length() > 6
						&& (cmd.substr(6) == TEXT("reget ")
								|| cmd.substr(6) == TEXT("reput ")))) {
			server->setTransferAscii(doTransferAscii(cmd));
		}

		// change current dir and execute command
		bstring cd = bstring(TEXT("cd \"")) + remotePath + TEXT("\"");
		if (server->doCommand(cd) && server->doCommand(cmd)) {
			if (cmd.length() > 2 && cmd.substr(0, 2) == TEXT("cd")) {
				bstring nd = remotePath;
				if (server->doCommand(TEXT("pwd"), nd)) {
					if (nd.size() == 0 || nd[nd.size() - 1] != TEXT('/'))
						nd += TEXT('/');
					if (remotePath != nd) {
						bstring toDir = TEXT("/") + server->getName() + nd;
						bstrcpy(fullRemoteName, toDir.c_str());
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
int __stdcall FsExecuteFile(HWND MainWin, char * fullRemoteName, char * verb) {
	bchar f[1024], g[1024];
	qudConvert(f, fullRemoteName, 1023);
	qudConvert(g, verb, 1023);
	return FsExecuteFileW(MainWin, f, g);
}
//---------------------------------------------------------------------

int __stdcall FsGetFileW(bchar *fullRemoteName, bchar *LocalName, int CopyFlags,
		RemoteInfoStruct * ri) {
	// disable automatic reloads
	lastServer = 0;

	// check flags and decide what to do
	bool overwrite = 0 != (CopyFlags & FS_COPYFLAGS_OVERWRITE);
	bool resume = 0 != (CopyFlags & FS_COPYFLAGS_RESUME);
	bool move = 0 != (CopyFlags & FS_COPYFLAGS_MOVE);

	// check file existance
	HANDLE hf = CreateFile(LocalName, 0, 0, 0, OPEN_EXISTING, 0, 0);
	bool exists = hf != INVALID_HANDLE_VALUE;
	if (exists)
		CloseHandle(hf);

	// no action but signal resume is allowed
	if (exists && !overwrite && !resume)
		return FS_FILE_EXISTSRESUMEALLOWED;

	bstring remotePath;
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server)
		return FS_FILE_NOTFOUND;

	WLock wlock(server->mutex);
	// remove the file if overwrite is set
	if (exists && overwrite) {
		DeleteFile(LocalName);
		exists = false;
	}

	// set transfer mode
	server->setTransferAscii(doTransferAscii(remotePath));

	// get it
	if (!server->cmdGet(remotePath, LocalName, resume && exists))
		return lastPPR ? FS_FILE_USERABORT : FS_FILE_WRITEERROR;

	if (!move)
		return FS_FILE_OK;

	if (!server->cmdRm(remotePath))
		return FS_FILE_NOTFOUND;
	return FS_FILE_OK;
}
int __stdcall FsGetFile(char *fullRemoteName, char *localName, int CopyFlags,
		RemoteInfoStruct * ri) {
	bchar f[1024], g[1024];
	qudConvert(f, localName, 1023);
	qudConvert(g, fullRemoteName, 1023);
	return FsGetFileW(g, f, CopyFlags, ri);
}

//---------------------------------------------------------------------

int __stdcall FsPutFileW(bchar * localName, bchar *fullRemoteName,
		int CopyFlags) {
	// disable automatic reloads
	lastServer = 0;

	bool overwrite = 0 != (CopyFlags & FS_COPYFLAGS_OVERWRITE);
	bool resume = 0 != (CopyFlags & FS_COPYFLAGS_RESUME);
	bool move = 0 != (CopyFlags & FS_COPYFLAGS_MOVE);

	bstring remotePath;
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server)
		return FS_FILE_NOTFOUND;

	WLock wlock(server->mutex);
	bool exists = server->remoteFileExists(remotePath);

	if (!overwrite && !resume) {
		if (exists)
			return FS_FILE_EXISTSRESUMEALLOWED;
	}

	// set transfer mode
	server->setTransferAscii(doTransferAscii(remotePath));

	if (!server->cmdPut(localName, remotePath, resume && exists))
		return lastPPR ? FS_FILE_USERABORT : FS_FILE_WRITEERROR;

	if (!move)
		return FS_FILE_OK;

	HFILE hf = DeleteFile(localName);
	if (hf != HFILE_ERROR)
		_lclose(hf);

	return FS_FILE_OK;

}
int __stdcall FsPutFile(char * localName, char *fullRemoteName, int CopyFlags) {
	bchar f[1024], g[1024];
	qudConvert(f, localName, 1023);
	qudConvert(g, fullRemoteName, 1023);
	return FsPutFileW(f, g, CopyFlags);
}
extern bool firstHalf;
extern bool secondHalf;

int __stdcall FsRenMovFileW(bchar *fullOldName, bchar *fullNewName, BOOL move,
		BOOL overWrite, RemoteInfoStruct * ri) {
	// disable automatic reloads
	lastServer = 0;

	bstring oldRemotePath;
	Server * oldServer = Server::findServer(oldRemotePath, fullOldName);
	bstring newRemotePath;
	Server * newServer = Server::findServer(newRemotePath, fullNewName);

	WLock wlock1(oldServer->mutex);
	WLock wlock2(newServer->mutex);

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
	bchar td[512];
	GetTempPath(512, td);
	bchar tf[512];
	GetTempFileName(td, TEXT("sftp4tc"), 0, tf);

	firstHalf = true;
	int flags = (move ? FS_COPYFLAGS_MOVE : 0)
			| (overWrite ? FS_COPYFLAGS_OVERWRITE : 0);
	int ret = FsGetFileW(fullOldName, tf, flags | FS_COPYFLAGS_OVERWRITE, ri);
	if (FS_FILE_OK == ret) {
		secondHalf = true;
		ret = FsPutFileW(tf, fullNewName, flags);
	}
	firstHalf = secondHalf = false;

	DeleteFile(tf);
	return ret;
}
int __stdcall FsRenMovFile(char *fullOldName, char *fullNewName, BOOL move,
		BOOL overWrite, RemoteInfoStruct * ri) {
	bchar f[1024], g[1024];
	qudConvert(f, fullOldName, 1023);
	qudConvert(g, fullNewName, 1023);
	return FsRenMovFileW(f, g, move, overWrite, ri);
}
//---------------------------------------------------------------------

//---------------------------------------------------------------------
BOOL __stdcall FsDeleteFileW(bchar *fullRemoteName) {
	// disable automatic reloads
	lastServer = 0;

	bstring remotePath;
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server)
		return FS_FILE_NOTFOUND;

	WLock wlock(server->mutex);
	if (remotePath == TEXT("/"))
		return FS_FILE_NOTSUPPORTED;

	return server->cmdRm(remotePath);
}
BOOL __stdcall FsDeleteFile(char *fullRemoteName) {
	bchar f[1024];
	qudConvert(f, fullRemoteName, 1023);
	return FsDeleteFileW(f);
}

//---------------------------------------------------------------------

BOOL __stdcall FsRemoveDirW(bchar *fullRemoteName) {
	// disable automatic reloads
	lastServer = 0;

	bstring remotePath;
	Server * server = Server::findServer(remotePath, fullRemoteName);
	if (!server)
		return FS_FILE_NOTFOUND;

	WLock wlock(server->mutex);
	if (remotePath == TEXT("/"))
		return FS_FILE_NOTSUPPORTED;

	return server->cmdRmDir(remotePath);
}
BOOL __stdcall FsRemoveDir(char *fullRemoteName) {
	bchar f[1024];
	qudConvert(f, fullRemoteName, 1023);
	return FsRemoveDirW(f);
}

//---------------------------------------------------------------------

BOOL __stdcall FsDisconnectW(bchar * disconnectRoot) {
	WLock wlock(global);

	if (*disconnectRoot == TEXT('\\'))
		++disconnectRoot;
	if (Server::removeServer(disconnectRoot)) {
		return true;
	}

	bstring err =
			bstring(
					TEXT(
							"no connection to disconnect: '")) + disconnectRoot + TEXT("'");
	gLogProc(gPluginNumber, MSGTYPE_DISCONNECT, err.c_str());

	return false;
}
BOOL __stdcall FsDisconnect(char *disconnectRoot) {
	bchar f[1024];
	qudConvert(f, disconnectRoot, 1023);
	return FsDisconnectW(f);
}
//---------------------------------------------------------------------

void __stdcall FsGetDefRootNameW(bchar * defRootName, int maxlen) {
	if (--maxlen >= 0) {
		bstrcpyn(defRootName, WFSPLUGIN_CAPTION, maxlen);
		defRootName[maxlen] = 0;
	}
}
void __stdcall FsGetDefRootName(char * defRootName, int maxlen) {
	if (--maxlen >= 0) {
		strncpy(defRootName, FSPLUGIN_CAPTION, maxlen);
		defRootName[maxlen] = 0;
	}
}

//---------------------------------------------------------------------

void __stdcall FsStatusInfo(bchar *RemoteDir, int InfoStartEnd,
		int InfoOperation) {
}

//---------------------------------------------------------------------

int __stdcall FsExtractCustomIconW(bchar * fullRemoteName, int extractFlags,
		HICON* theIcon) {
	bstring remoteName = fullRemoteName + 1;
	if (remoteName.length() == 0) {
		*theIcon = gConnectionIcon;
		return FS_ICON_EXTRACTED;
	}
	if (remoteName == EDIT_CONNECTIONS) {
		*theIcon = gConfigIcon;
		return FS_ICON_EXTRACTED;
	}
	if (remoteName == TEXT("..\\"))
		return FS_ICON_USEDEFAULT;
	if (remoteName.find(TEXT('\\')) == remoteName.length() - 1) {
		*theIcon = gServerIcon;
		return FS_ICON_EXTRACTED;
	}
	return FS_ICON_USEDEFAULT;
}
BOOL __stdcall FsExtractCustomIcon(char *fullRemoteName, int extractFlags,
		HICON* theIcon) {
	bchar f[1024];
	qudConvert(f, fullRemoteName, 1023);
	return FsExtractCustomIconW(f, extractFlags, theIcon);
}

//---------------------------------------------------------------------
void __stdcall FsSetCryptCallbackW(tCryptProcW pCryptProc,int CryptoNr,int Flags) {
	gCryptProc = pCryptProc;
	gCryptoNumber = CryptoNr;
}

//---------------------------------------------------------------------
int __stdcall FsGetBackgroundFlags(void) {
	return BG_UPLOAD | BG_DOWNLOAD | BG_ASK_USER;
}
