#include "stdafx.h"
#include "fsplugin.h"
#include "server.h"

extern bool UnixTimeToFileTime(unsigned long mtime, LPFILETIME ft);
extern unsigned long FileTimeToUnixTime(LPFILETIME ft);
extern bstring splitPath(bstring & path, bstring const & filePath);

extern HWND gMainWin;
extern DWORD gMainThread;

/**
 * This class ensures that the used mapper exists and is still connected.
 * If necessary a new mapper is initialized.
 * For temp mappers (configuration issues) or disconnected mappers
 * the mapper is cleaned up at end
 */
class Guard {
	PsftpMapper * intern;
	Server * server;
public:
	PsftpMapper * & mapper;
	Guard();
	Guard(Server * server, config_tag * cfg = 0);
	~Guard();
	bool isConnected();
	bool isLoaded();
};

Guard::Guard() :
		mapper(intern), server(0) {
	mapper = new PsftpMapper(TEXT("~"), TEXT("~"));
}

Guard::Guard(Server * server, config_tag * cfg) :
		intern(0), server(server), mapper(server->currentMapper) {
}

Guard::~Guard() {
	if (intern)
		delete intern;
}

bool Guard::isLoaded() {
	if (!mapper && server)
		mapper = new PsftpMapper(server->name, server->sessionName, server->myCfg);
	return mapper && mapper->hDll;
}

// checks existing connection and creates a new one if none exists.
bool Guard::isConnected() {
	if (!server)
		return false;

	if (!mapper || !mapper->hDll || mapper->disconnected()) {
		if (mapper) {
			//      gLogProc(gPluginNumber, MSGTYPE_DISCONNECT, (TEXT("connection to ") + server->name + TEXT(" IS BROKEN!")).c_str());
			delete mapper;
		}
		bstring ss = server->sessionName.length() ? server->sessionName : server->name;
		if (server->myCfg)
			qudConvert(server->myCfg->host, ss.c_str(), 511);
		mapper = new PsftpMapper(server->name, server->sessionName, server->myCfg);
		if (!mapper->hDll)
			return false;

		bstring msg = TEXT("CONNECT \\") + server->name;
		gLogProc(gPluginNumber, MSGTYPE_CONNECT, msg.c_str());
#ifdef UNICODE
		char srvName[256];
		qudConvert(srvName, server->name.c_str(), 255);
		int port = 0;

		if (server && server->myCfg) {
			strncpy(srvName, server->myCfg->host, 255);
			port = server->myCfg->port;
		}

		Sftp4tc * cfg = mapper->connect(srvName, 0, srvName, port);
#else
		char const * srvName = server->name.c_str();
		Sftp4tc * cfg = mapper->connect(0, 0, srvName, 0);
#endif
		if (!cfg || mapper->disconnected()) {
			gLogProc(gPluginNumber, MSGTYPE_CONNECTCOMPLETE,
					(TEXT("NOT connected to ") + server->name).c_str());
			delete mapper;
			mapper = 0;
			return false;
		}
		gLogProc(gPluginNumber, MSGTYPE_CONNECTCOMPLETE,
				(TEXT("connected to ") + server->name).c_str());
		if (cfg) {
			server->configure(cfg);
		}
	}
	return true;
}

//---------------------------------------------------------------------
ServerMap Server::serverMap;
std::vector<ServerInfo> Server::serverInfos;

//---------------------------------------------------------------------
// remove a file of the dir content - if present
fxp_name * Server::removeFile(bstring const & remotePath) {
	bstring path, file;
	file = splitPath(path, remotePath);
	DirCache::iterator i = dirCache.find(path);
	if (i == dirCache.end())
		return 0;

	my_fxp_names * dir = i->second;

	int count = dir->nnames;
	for (int i = 0; i < count; ++i) {
		fxp_name * fn = dir->names[i];
#ifdef UNICODE
		if (file == fn->ucFilename) {
#else
		if (file == fn->filename) {
#endif
			// not last -> replace current with last
			if (i + 1 < count) {
				dir->names[i] = dir->names[count - 1];
			}
			// and one less now
			--dir->nnames;

			return fn; // done
		}
	}

	return 0;
}

//---------------------------------------------------------------------
// add a file to the dir content - if not already there
void Server::insertFile(bstring const & remotePath, FILETIME * ft,
		unsigned long szLo, unsigned long szHi, char kind,
		bstring const & chmod) {
	bstring path, file;
	file = splitPath(path, remotePath);
	DirCache::iterator i = dirCache.find(path);
	if (i == dirCache.end())
		return;

#ifdef UNICODE
	dirCache.erase(i);
#else

	my_fxp_names * dir = i->second;

	int count = dir->nnames;
	for (int i = 0; i < count; ++i) {
		fxp_name * fn = dir->names[i];
		if (file == fn->filename) {
			// already there
			return;
		}
	}
	// create a new structure and fill it
	my_fxp_names * ndir = (my_fxp_names *) malloc(sizeof(my_fxp_names));
	ndir->names = (fxp_name **) malloc(sizeof(fxp_name*) * (count + 1));

	fxp_name * n = ndir->names[0] = (fxp_name *) malloc(sizeof(fxp_name));
	RtlZeroMemory(n, sizeof(fxp_name));

	FILETIME ft2;
	SYSTEMTIME st;
	if (ft) {
		FileTimeToSystemTime(ft, &st);
	} else {
		GetSystemTime(&st);
		SystemTimeToFileTime(&st, &ft2);
		ft = &ft2;
	}

	char buffer[32];
	sprintf(buffer, "%d ", szLo);
	std::string ln = kind + chmod + std::string(" 1 ? ? ") + buffer
			+ " Jan  1 1990 " + file;
	n->longname = strdup(ln.c_str());
	n->filename = strdup(file.c_str());

	n->attrs.size.hi = szHi;
	n->attrs.size.lo = szLo;
	n->attrs.mtime = FileTimeToUnixTime(ft);

	for (int i = 0; i < count; ++i) {
		ndir->names[i + 1] = dir->names[i];
	}
	ndir->nnames = count + 1;

	i->second = ndir;
#endif
}

//---------------------------------------------------------------------
// update the file attributes
void Server::updateFileAttr(bstring const & path, bstring const & file,
		bstring const & attrs) {
	DirCache::iterator i = dirCache.find(path);
	if (i == dirCache.end())
		return;

	my_fxp_names * dir = i->second;
	int count = dir->nnames;
	for (int i = 0; i < count; ++i) {
		fxp_name * fn = dir->names[i];
#ifdef UNICODE
		if (file == fn->ucFilename) {
#else
		if (file == fn->filename) {
#endif
			// got it
			fn->attrs.permissions = bstrtol(attrs.c_str(), '\0', 8);
			return;
		}
	}
}

//---------------------------------------------------------------------
// free one fxp_name
static void freeFn(fxp_name * fn) {
	if (!fn)
		return;

	free(fn->filename);
	free(fn->longname);
	free(fn);
}

//---------------------------------------------------------------------
// free all fxp_names + my_fxp_names
static void freeMfn(my_fxp_names * dir) {
	if (!dir)
		return;

	for (int i = 0; i < dir->nnames; i++) {
		freeFn(dir->names[i]);
	}
	free(dir);
}

//---------------------------------------------------------------------
// CT
// init with zeros
Server::Server(bstring const & serverName_, bstring const & orgSessionName_, DWORD _tid) :
		sessionName(orgSessionName_), name(serverName_), currentMapper(0), disableMtime(false), cacheFolders(
				false), hideDotNames(false), myCfg(0), tid(_tid) {
					mutex = ::CreateMutex(0, 0, 0);
}

//---------------------------------------------------------------------
// DT
// free all cached structures
Server::~Server() {
	::CloseHandle(mutex);

	delete currentMapper;

	clearDirCache();

	delete myCfg;
}

bool Server::connect() {
	Guard guard(this);
	return guard.isConnected();
	//return true;
}

//---------------------------------------------------------------------
// clear the folder chache
void Server::clearDirCache() {
	for (DirCache::iterator i = dirCache.begin(); i != dirCache.end(); ++i) {
		my_fxp_names * dir = i->second;
		freeMfn(dir);
	}
	dirCache.clear();
}

//---------------------------------------------------------------------
// performs a server lookup by evaluating the remote path
// on success the remote path is also set
Server * Server::findServer(bstring & remotePath,
		bchar const * const fullPath) {
	WLock wlock(global);

	bstring serverName = *fullPath == TEXT('\\') ? fullPath + 1 : fullPath;
	int slash = (int) serverName.find_first_of(TEXT('\\'));
	if (slash > 0) {
		remotePath = serverName.substr(slash);
		for (bstring::iterator i = remotePath.begin(); i != remotePath.end();
				++i) {
			if (*i == TEXT('\\'))
				*i = '/';
		}
		serverName = serverName.substr(0, slash);
	} else {
		remotePath = TEXT("/");
	}

	bstring sessionName = serverName;
	DWORD tid = ::GetCurrentThreadId();
	// if this is not the main thread, appand the ThreadId to the server name
	if (tid != gMainThread) {
		bchar buf[32];
		bsprintf(buf, TEXT("-background-%d"), tid);
		serverName += buf;
	}

	// remove servers whithout active thread
	for (ServerMap::const_iterator i = serverMap.begin(); i != serverMap.end(); ++i) {
		Server * server = i->second;
		HANDLE h = ::OpenThread(THREAD_QUERY_INFORMATION, 1, server->tid);
		if (h) {
			DWORD x = 0;
			if (!::GetExitCodeThread(h, &x))
				x = STILL_ACTIVE;
			::CloseHandle(h);
			if (x != STILL_ACTIVE) {
				delete server;
				serverMap.erase(i);
				break;
			}
		}
	}

	ServerMap::const_iterator i = serverMap.find(serverName);
	Server * server;
	if (i != serverMap.end()) {
		server = i->second;
	} else {
		server = new Server(serverName, sessionName, tid);
		serverMap.insert(std::make_pair(serverName, server));

		if (tid != gMainThread) {
			// copy settings from main thread session
			ServerMap::const_iterator j = serverMap.find(sessionName);
			if (j != serverMap.end()) {
				Server * mainServer = j->second;
				if (mainServer->myCfg)
					server->configure(mainServer->myCfg);
			}
		}

	}

	return server;
}

void Server::insertServer(bstring const & serverName, Server * server) {
	ServerMap::iterator i = serverMap.find(serverName);
	server->name = serverName;
	if (i != serverMap.end()) {
		delete i->second;
		i->second = server;
	} else {
		serverMap.insert(std::make_pair(serverName, server));
	}
}

//---------------------------------------------------------------------
// disconnects and performs a cleanup
bool Server::removeServer(bchar const * serverName) {
	WLock wlock(global);

	ServerMap::const_iterator i = serverMap.find(serverName);
	if (i == serverMap.end())
		return false;
	Server * server = i->second;
	gLogProc(gPluginNumber, MSGTYPE_DISCONNECT,
			(TEXT("connection to ") + server->name
					+ TEXT(" closed - bye bye!")).c_str());

	serverMap.erase(i);
	delete server;
	return true;
}

//---------------------------------------------------------------------
// reload the config
size_t Server::loadServers() {
	Guard guard;
	if (guard.isLoaded()) {
		serverInfos.clear();
		guard.mapper->import_putty_sessions(serverInfos);
	}
	return serverInfos.size();
}

//---------------------------------------------------------------------
// get the server name for the index - use from FindFirst/Next
bchar const * const Server::getServerName(size_t index) {
	if (index < serverInfos.size())
		return serverInfos[index].c_str();
	return 0;
}

//---------------------------------------------------------------------
// get the home folder
bool Server::getHomeDir(bstring & response) {
	Guard guard(this);
	if (!guard.isLoaded())
		return false;

	Sftp4tc cfg;
	memset(&cfg, 0, sizeof(cfg));
#ifdef UNICODE
	char srvName[256];
	qudConvert(srvName, name.c_str(), 256);
#else
	char const * srvName = name.c_str();
#endif
	guard.mapper->loadConfig(srvName, &cfg);
#ifdef UNICODE
	BCONVERT(wchar_t, 256, response, cfg.homeDir);
#else
	response = cfg.homeDir;
#endif
	return true;
}

//---------------------------------------------------------------------
// execute a SFTP command
bool Server::doCommand(bstring const & command, bstring & response) {
	Guard guard(this);
	if (!guard.isConnected())
		return false;

	gLogProc(gPluginNumber, MSGTYPE_DETAILS, command.c_str());

	bchar buffer[8192];
	*buffer = 0;
	int r = guard.mapper->doSftp(command.c_str(), buffer);

	if (*buffer) {
		response = buffer;
		gLogProc(gPluginNumber, MSGTYPE_OPERATIONCOMPLETE, buffer);
	}
	return 0 != r;
}

//---------------------------------------------------------------------
// get the directory for the given path
my_fxp_names * Server::getDirContent(bstring const & _path) {
	bstring path = _path;
	if (path.length() == 0 || path[path.length() - 1] != TEXT('/'))
		path = path + TEXT('/');
	DirCache::iterator i = dirCache.find(path);
	if (i != dirCache.end())
		return i->second;

	if (!this->cmdLs(path))
		return 0;

	i = dirCache.find(path);
	if (i != dirCache.end())
		return i->second;

	return 0;
}

//---------------------------------------------------------------------
// remove a dir entry from the cache
void Server::invalidateDirContent(bstring const & _path) {
	bstring path = _path;
	if (path.size() == 0 || path[path.size() - 1] != TEXT('/'))
		path = path + TEXT('/');
	dirCache.erase(path);
}

//---------------------------------------------------------------------
// checks existance of a remote file
bool Server::remoteFileExists(bstring const & remotePath) {
	bstring path;
	bstring file = splitPath(path, remotePath);
	my_fxp_names * dir = getDirContent(path);
	if (!dir)
		return true;

	for (int i = 0; i < dir->nnames; ++i) {
#ifdef UNICODE
		bchar const * const name = dir->names[i]->ucFilename;
#else
		bchar const * const name = dir->names[i]->filename;
#endif
		if (file == name) {
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------
// get remote content and store it into the dir cache
bool Server::cmdLs(bstring const & remotePath) {
	Guard guard(this);
	if (!guard.isConnected())
		return false;

	bstring cmd = bstring(TEXT("ls \"")) + remotePath + TEXT('"');

	guard.mapper->freeCurrentDirStruct();
	my_fxp_names * cds = guard.mapper->getCurrentDirStruct();

	if (!doCommand(cmd))
		return false;

	if (!cds->names)
		return false;

	my_fxp_names * ndir = (my_fxp_names *) malloc(sizeof(my_fxp_names));
	ndir->names = (fxp_name **) malloc(sizeof(fxp_name*) * cds->nnames);
	ndir->nnames = cds->nnames;

	for (int j = 0; j < ndir->nnames; ++j) {
		fxp_name * fn = ndir->names[j] = (fxp_name *) malloc(sizeof(fxp_name));
		fxp_name * cn = cds->names[j];
		fn->attrs = cn->attrs;
#ifdef UNICODE
		fn->filename = 0;
		fn->ucFilename = wcsdup(cn->ucFilename);
#else
		fn->filename = strdup(cn->filename);
		fn->ucFilename = 0;
#endif
		fn->longname = strdup(cn->longname);
	}

	guard.mapper->freeCurrentDirStruct();

	updateDirCache(remotePath, ndir);

	return true;
}

//---------------------------------------------------------------------
// insert or replace an existing dir cache entry
void Server::updateDirCache(bstring const & _path, my_fxp_names * dir) {
	bstring path = _path;
	if (path.length() == 0 || path[path.length() - 1] != TEXT('/'))
		path = path + TEXT('/');

	if (cacheFolders) {
		// remove old dir listing
		DirCache::iterator i = dirCache.find(path);
		if (i != dirCache.end()) {
			my_fxp_names * del = i->second;
			freeMfn(del);
			dirCache.erase(i);
		}
	} else {
		clearDirCache();
	}

	// add new dir listing
	dirCache.insert(std::make_pair(path, dir));
}

//---------------------------------------------------------------------
// get a remote file -> local
bool Server::cmdGet(bstring const & remotePath, bstring const & localName,
		bool exists) {
	DBGPRINT(("get '%s' '%s' %b\r\n", remotePath.c_str(), localName.c_str(), exists));
	bstring cmd =
			bstring(
					exists ?
							TEXT("reget \"") :
							TEXT(
									"get \"")) + remotePath + TEXT("\" \"") + localName + TEXT(
											"\"");

	if (!doCommand(cmd))
		return false;

	FILETIME ft;
	fxp_attrs * attr = currentMapper->getLastAttr();
	if (UnixTimeToFileTime(attr->mtime, &ft)) {
		HANDLE hf = CreateFile(localName.c_str(), GENERIC_WRITE, 0, 0,
				OPEN_EXISTING, 0, 0);
		SetFileTime(hf, &ft, &ft, &ft);
		CloseHandle(hf);
	}
	return true;
}

//---------------------------------------------------------------------
// put a local file --> remote
bool Server::cmdPut(bstring const & localName, bstring const & remotePath,
		bool exists) {
	DBGPRINT(("put '%s' '%s' %b\r\n", localName.c_str(), remotePath.c_str(), exists));
	bstring cmd =
			bstring(
					exists ?
							TEXT("reput \"") :
							TEXT(
									"put \"")) + localName + TEXT("\" \"") + remotePath + TEXT(
											"\"");

	if (!doCommand(cmd))
		return false;

	FILETIME ft;
	HANDLE hFile = CreateFile(localName.c_str(), GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == (HANDLE) HFILE_ERROR)
		return true; // put already succeeded

	GetFileTime(hFile, 0, 0, &ft);
	long fsHigh = 0;
	long fsLow = SetFilePointer(hFile, 0, &fsHigh, FILE_END);

	CloseHandle(hFile);

	bstring chmod = TEXT("----");
	if (defChMod.length() > 0 || exeChMod.length() > 0) {
		chmod = defChMod;
		size_t ldot = remotePath.find_last_of('.');
		if (ldot != (size_t) -1) {
			bstring x = remotePath.substr(ldot);
			std::map<bstring, bstring>::iterator i = exeExtensions.find(x);
			if (i != exeExtensions.end())
				chmod = exeChMod;
		}
		if (chmod.length() > 0) {
			cmd =
					bstring(
							TEXT(
									"chmod ")) + chmod + TEXT(" \"") + remotePath + TEXT("\"");
			doCommand(cmd);
		}
	}

	// fake update the directory
	insertFile(remotePath, &ft, fsLow, fsHigh, '-', chmod);

	// put succeeded.
	return true;
}

//---------------------------------------------------------------------
// create a remote folder
bool Server::cmdMkDir(bstring const & remotePath) {
	bstring cmd = bstring(TEXT("mkdir \"")) + remotePath + TEXT("\"");

	if (!doCommand(cmd))
		return false;

	// fake update the directory
	insertFile(remotePath, 0, 0, 0, 'd');

	return true;
}
//---------------------------------------------------------------------
// move a remote file to a remote file
bool Server::cmdMove(bstring const & oldRemotePath,
		bstring const & newRemotePath) {
	bstring cmd =
			bstring(
					TEXT(
							"mv \"")) + oldRemotePath + TEXT("\" \"") + newRemotePath + TEXT("\"");

	if (!doCommand(cmd))
		return false;

	fxp_name * fn = removeFile(oldRemotePath);
	if (fn) {
		FILETIME ft;
		UnixTimeToFileTime(fn->attrs.mtime, &ft);
		insertFile(newRemotePath, &ft, fn->attrs.size.lo, fn->attrs.size.hi,
				fn->longname[0]);
		freeFn(fn);
	} else {
		bstring path;
		splitPath(path, newRemotePath);
		invalidateDirContent(path);
	}

	return true;
}

//---------------------------------------------------------------------
// remove a remote file
bool Server::cmdRm(bstring const & remotePath) {
	bstring path;
	splitPath(path, remotePath);
	invalidateDirContent(path);
	bstring cmd = bstring(TEXT("rm \"")) + remotePath + TEXT("\"");
	if (!doCommand(cmd))
		return false;

	freeFn(removeFile(remotePath));
	return true;
}

//---------------------------------------------------------------------
// remove a remote folder
bool Server::cmdRmDir(bstring const & remotePath) {
	bstring path;
	splitPath(path, remotePath);
	invalidateDirContent(path);
	bstring cmd = bstring(TEXT("rmdir \"")) + remotePath + TEXT("\"");
	if (!doCommand(cmd))
		return false;

	freeFn(removeFile(remotePath));
	return true;
}

//---------------------------------------------------------------------

bool Server::cmdChmod(bstring const & remotePath, int flags) {
	//TODO - not needed yet
	return 0;
}

//---------------------------------------------------------------------
// try to set the remote mtime
bool Server::cmdMtime(bstring const & remotePath, FILETIME * ft) {
	return 0;
	//	SYSTEMTIME st;
	//	char buffer[32];
	//
	//	FileTimeToSystemTime(ft, &st);
	//	_snprintf(buffer, 32, "%d%02d%02d%02d%02d.%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	//	bstring cmd = bstring("!touch -m -t ") + buffer + " " + remotePath;
	//
	//	return doCommand(cmd);
}

//---------------------------------------------------------------------
void Server::setTransferAscii(bool ta) {
	Guard guard(this);
	if (!guard.isConnected())
		return;

	guard.mapper->setTransferAscii(ta);
}

//---------------------------------------------------------------------
// show the global config dialog
Sftp4tc * const Server::doConfig() {
	static int inside;
	{
		WLock wlock(global);
		if (inside)
			return 0;
		++inside;
	}

	Guard guard(this);
	if (!guard.isLoaded())
		return 0;

	Sftp4tc * r = guard.mapper->doConfig(gMainWin, 0, 0);
	WLock wlock(global);
	inside = 0;
	return r;
}
//---------------------------------------------------------------------
// reconfigure current server
Sftp4tc * const Server::doSelfConfig() {
	Guard guard(this);
	if (!guard.isConnected())
		return 0;

	return guard.mapper->doConfig(gMainWin, 1, 0);
}
//---------------------------------------------------------------------
// apply the configuration
void Server::configure(Sftp4tc * cfg_) {
	if (!myCfg)
		myCfg = new Sftp4tc;
	*myCfg = *cfg_;

	const Sftp4tc * cfg = cfg_;
	bchar const * homeDir2;
	bchar const * defChMod2;
	bchar const * exeChMod2;
	bchar const * exeExtensions2;

#ifdef UNICODE
	BCONVERT(wchar_t, 512, homeDir2, cfg->homeDir);
	BCONVERT(wchar_t, 512, defChMod2, cfg->defChMod);
	BCONVERT(wchar_t, 512, exeChMod2, cfg->exeChMod);
	BCONVERT(wchar_t, 512, exeExtensions2, cfg->exeExtensions);
#else
	homeDir2 = cfg->homeDir;
	defChMod2 = cfg->defChMod;
	exeChMod2 = cfg->exeChMod;
	exeExtensions2 = cfg->exeExtensions;
#endif
	homeDir = homeDir2;
	cacheFolders = cfg->cacheFolders != 0;
	if (!cacheFolders)
		clearDirCache();
	hideDotNames = cfg->hideDotNames != 0;
	defChMod = defChMod2;
	exeChMod = exeChMod2;

	bstring ext = exeExtensions2;
	exeExtensions.clear();
	size_t start = 0;
	for (size_t i = 0; i < ext.size(); ++i) {
		if (ext[i] == TEXT('.') || ext[i] <= 32) {
			if (i > start) {
				size_t len = i - start;
				bstring x = ext.substr(start, len);
				exeExtensions.insert(std::make_pair(x, x));
			}
			start = i + 1;
		}
	}
	if (start < ext.length()) {
		bstring x = ext.substr(start);
		exeExtensions.insert(std::make_pair(x, x));
	}
}
