#include "stdafx.h"
#include "fsplugin.h"
#include "server.h"

extern void import_putty_sessions(std::vector<ServerInfo> & serverInfos);
extern bool UnixTimeToFileTime(unsigned long mtime, LPFILETIME ft);
extern unsigned long FileTimeToUnixTime(LPFILETIME ft);
extern std::string splitPath(std::string & path, std::string const & filePath);

extern HWND gMainWin;

//---------------------------------------------------------------------
ServerMap Server::serverMap;
std::vector<ServerInfo> Server::serverInfos;

//---------------------------------------------------------------------
// remove a file of the dir content - if present
fxp_name * Server::removeFile(std::string const & remotePath) {
	std::string path, file;
	file = splitPath(path, remotePath);
	DirCache::iterator i = dirCache.find(path);
	if (i == dirCache.end())
		return 0;

	my_fxp_names * dir = i->second;	
	
	int count = dir->nnames;
	for (int i = 0; i < count; ++i) {
		fxp_name * fn = dir->names[i];
		if (file == fn->longname) {
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
void Server::insertFile(std::string const & remotePath, FILETIME * ft, unsigned long szLo, unsigned long szHi, char kind, std::string const & chmod) {
	std::string path, file;
	file = splitPath(path, remotePath);
	DirCache::iterator i = dirCache.find(path);
	if (i == dirCache.end())
		return;

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
	ndir->names = (fxp_name **)malloc(sizeof(fxp_name*) * (count + 1));
	
	fxp_name * n = ndir->names[0] = (fxp_name *)malloc(sizeof(fxp_name));
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
	_snprintf(buffer, 32, "%d ", szLo);
	std::string ln = kind + chmod + std::string(" 1 ? ? ") + buffer + " Jan  1 1990 " + file; 
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
}


//---------------------------------------------------------------------
// CT
// init with zeros
Server::Server(std::string const & serverName)
: name(serverName)
, mapper(new PsftpMapper(serverName)) 
, disableMtime(false)
{
}

//---------------------------------------------------------------------
// free one fxp_name
static void freeFn(fxp_name * fn) {
	if (!fn) return; 

	free(fn->filename);
	free(fn->longname);
	free(fn);
}

//---------------------------------------------------------------------
// free all fxp_names + my_fxp_names
static void freeMfn(my_fxp_names * dir) {
	if (!dir) return; 

	for (int i = 0; i < dir->nnames; i++) {
		freeFn(dir->names[i]);
	}
	free(dir);
}

//---------------------------------------------------------------------
// DT
// free all cached structures
Server::~Server()
{
	if (mapper->hDll && !mapper->disconnected()) {
		mapper->disconnect();
	}
	delete mapper;

	clearDirCache();
}

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
Server * Server::findServer(std::string & remotePath, char const * const fullPath)
{
	std::string serverName = fullPath + 1;
	int slash = (int)serverName.find_first_of('\\');
	if (slash > 0) {
		remotePath = serverName.substr(slash);
		for (std::string::iterator i = remotePath.begin(); i != remotePath.end(); ++i)
		{
			if (*i == '\\') *i = '/';
		}
		serverName = serverName.substr(0, slash);
	} else {
		remotePath = "/";
	}

	ServerMap::const_iterator i = serverMap.find(serverName);
	Server * server;
	if (i != serverMap.end()) {
		server = i->second;
	} else {
		server = new Server(serverName);
		serverMap.insert(std::make_pair(serverName, server));
	}
	
	return server;
}

//---------------------------------------------------------------------
// disconnects and performs a cleanup
bool Server::disconnectServer(char const * serverName) {
	ServerMap::const_iterator i = serverMap.find(serverName);
	if (i == serverMap.end())
		return false;
	Server * server = i->second;
	gLogProc(gPluginNumber, MSGTYPE_DISCONNECT, ("connection to " + server->name + " closed - bye bye!").c_str());

	serverMap.erase(i);
	delete server;
	return true;
}

//---------------------------------------------------------------------
// reload the config
size_t Server::loadServers() {
	serverInfos.clear();
	import_putty_sessions(serverInfos);
	return serverInfos.size();
}

//---------------------------------------------------------------------
// get the server name for the index - use from FindFirst/Next
char const * const Server::getServerName(size_t index) {
	if (index < serverInfos.size())
		return serverInfos[index].name.c_str();
	return 0;
}

//---------------------------------------------------------------------
// execute a SFTP command
bool Server::doCommand(std::string const & command, std::string & response) {
	if (!mapper->hDll)
		return false;

	if (mapper->disconnected()) {
		std::string msg = "CONNECT \\" + name;
		gLogProc(gPluginNumber, MSGTYPE_CONNECT, msg.c_str());
		Sftp4tc * cfg = mapper->connect(0, 0, name.c_str(), 0);
		if (!cfg || mapper->disconnected()) {
			gLogProc(gPluginNumber, MSGTYPE_DISCONNECT, "connection failed");
			delete mapper;
			mapper = new PsftpMapper(name);
			return false;
		}
		gLogProc(gPluginNumber, MSGTYPE_CONNECTCOMPLETE, ("connected to " + name).c_str());
		if (cfg) {
			configure(cfg);
		}
	}


	gLogProc(gPluginNumber, MSGTYPE_DETAILS, command.c_str());

	char buffer[8192];
	*buffer = 0;
	int r = mapper->doSftp(command.c_str(), buffer);

	if (*buffer) {
		response = buffer;
		gLogProc(gPluginNumber, MSGTYPE_OPERATIONCOMPLETE, buffer);
	}
	return 0 != r;
}

//---------------------------------------------------------------------
// get the directory for the given path
my_fxp_names * Server::getDirContent(std::string const & _path)
{
	std::string path = _path;
	if (path.length() == 0 || path[path.length() -1] != '/')
		path = path + '/';
	DirCache::iterator i = dirCache.find(path);
	if (i != dirCache.end())
		return i->second;

	if (!mapper->hDll)
		return 0;

	if (!this->cmdLs(path))
		return 0;

	i = dirCache.find(path);
	if (i != dirCache.end())
		return i->second;

	return 0;	
}

//---------------------------------------------------------------------
// remove a dir entry from the cache
void Server::invalidateDirContent(std::string const & _path) {
	std::string path = _path;
	if (path.size() == 0 || path[path.size() - 1] != '/')
		path = path + "/";
	dirCache.erase(path);
}

//---------------------------------------------------------------------
// checks existance of a remote file
bool Server::remoteFileExists(std::string const & remotePath) {
	std::string path;
	std::string file = splitPath(path, remotePath);
	my_fxp_names * dir = getDirContent(path);
	if (!dir)
		return true;

	for (int i = 0; i < dir->nnames; ++i) {
		char const * const name = dir->names[i]->filename;
		if (file == name) {
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------
// get remote content and store it into the dir cache
bool Server::cmdLs(std::string const & remotePath) {
	std::string cmd = std::string("ls \"") + remotePath + "\"";

	my_fxp_names * cds = mapper->getCurrentDirStruct();
	cds->nnames = 0;
	cds->names = 0;

	if (!doCommand(cmd))
		return false;

	if (!cds->names)
		return false;


	my_fxp_names * ndir = (my_fxp_names *) malloc(sizeof(my_fxp_names));
	ndir->names = (fxp_name **)malloc(sizeof(fxp_name*) * cds->nnames);
	ndir->nnames = cds->nnames;
	
	for (int j = 0; j < ndir->nnames; ++j) {
		fxp_name * fn = ndir->names[j] = (fxp_name *)malloc(sizeof(fxp_name));
		fxp_name * cn = cds->names[j];
		fn->attrs = cn->attrs;
		fn->filename = strdup(cn->filename);
		fn->longname = strdup(cn->longname);
	}
	
	mapper->freeCurrentDirStruct();

	updateDirCache(remotePath, ndir);

	return true;
}

//---------------------------------------------------------------------
// insert or replace an existing dir cache entry
void Server::updateDirCache(std::string const & _path, my_fxp_names * dir) {
	std::string path = _path;
	if (path.length() == 0 || path[path.length() -1] != '/')
		path = path + '/';

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
bool Server::cmdGet(std::string const & remotePath, std::string const & localName, bool exists)
{
	DBGPRINT(("get '%s' '%s' %b\r\n", remotePath.c_str(), localName.c_str(), exists));
	std::string cmd = std::string(exists ? "reget \"" : "get \"") + remotePath + "\" \"" + localName + "\"";

	if (!doCommand(cmd))
		return false;

	FILETIME ft;
	fxp_attrs * attr = mapper->getLastAttr();
	if (UnixTimeToFileTime(attr->mtime, &ft)) {
		HANDLE hf = CreateFile(localName.c_str(), GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		SetFileTime(hf, &ft, &ft, &ft);
		CloseHandle(hf);
	}
	return true;
}

//---------------------------------------------------------------------
// put a local file --> remote 
bool Server::cmdPut(std::string const & localName, std::string const & remotePath, bool exists)
{
	DBGPRINT(("put '%s' '%s' %b\r\n", localName.c_str(), remotePath.c_str(), exists));
	std::string cmd = std::string(exists ? "reput \"" : "put \"") + localName + "\" \"" + remotePath + "\"";

	if (!doCommand(cmd))
		return false;

	FILETIME ft;
	HANDLE hFile = CreateFile(localName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == (HANDLE)HFILE_ERROR)
		return true; // copy succeeded

	GetFileTime(hFile, 0, 0, &ft);
	long fsHigh = 0;
	long fsLow = SetFilePointer(hFile, 0, &fsHigh, FILE_END);

	CloseHandle(hFile);

	std::string chmod = "---";
	if (defChMod.length() > 0 || exeChMod.length() > 0) {
		chmod = defChMod;
		size_t ldot = remotePath.find_last_of('.');
		if (ldot != (size_t)-1) {
			std::string x = remotePath.substr(ldot);
			std::map<std::string, std::string>::iterator i = exeExtensions.find(x);
			if (i != exeExtensions.end())
				chmod = exeChMod;
		}
		if (chmod.length() > 0) {
			cmd = std::string("chmod ") + chmod + " \"" + remotePath + "\"";
			doCommand(cmd);
		}
	}

	// fake update the directory
	insertFile(remotePath, &ft, fsLow, fsHigh, '-', chmod);

	// we cannot set the file time
	if (disableMtime)
		return true;

	// if setting the mtime failes, do not try it again
	if (!this->cmdMtime(remotePath, &ft))
		disableMtime = true;

	// put already succeeded.
	return true;
}

//---------------------------------------------------------------------
// create a remote folder
bool Server::cmdMkDir(std::string const & remotePath) {
	std::string cmd = std::string("mkdir \"") + remotePath + "\"";

	if (!doCommand(cmd))
		return false;

	// fake update the directory
	insertFile(remotePath, 0, 0, 0, 'd');
	
	return true;
}
//---------------------------------------------------------------------
// move a remote file to a remote file
bool Server::cmdMove(std::string const & oldRemotePath, std::string const & newRemotePath) {
	std::string cmd = std::string("mv \"") + oldRemotePath + "\" \"" + newRemotePath + "\"";

	if (!doCommand(cmd))
		return false;

	fxp_name * fn = removeFile(oldRemotePath);
	if (fn) {
		FILETIME ft;
		UnixTimeToFileTime(fn->attrs.mtime, &ft);
		insertFile(newRemotePath, &ft, fn->attrs.size.lo, fn->attrs.size.hi, fn->longname[0]);
		freeFn(fn);
	} else {
		std::string path;
		splitPath(path, newRemotePath);
		invalidateDirContent(path);
	}

	return true;
}

//---------------------------------------------------------------------
// remove a remote file
bool Server::cmdRm(std::string const & remotePath)
{
	std::string path;
	splitPath(path, remotePath);
	invalidateDirContent(path);
	std::string cmd = std::string("rm \"") + remotePath + "\"";
	if (!doCommand(cmd))
		return false;

	freeFn(removeFile(remotePath));
	return true;
}

//---------------------------------------------------------------------
// remove a remote folder
bool Server::cmdRmDir(std::string const & remotePath)
{
	std::string path;
	splitPath(path, remotePath);
	invalidateDirContent(path);
	std::string cmd = std::string("rmdir \"") + remotePath + "\"";
	if (!doCommand(cmd))
		return false;

	freeFn(removeFile(remotePath));
	return true;
}

//---------------------------------------------------------------------

bool Server::cmdChmod(std::string const & remotePath, int flags) {
	//TODO - not needed yet
	return 0;
}

//---------------------------------------------------------------------
// try to set the remote mtime
bool Server::cmdMtime(std::string const & remotePath, FILETIME * ft) {
	SYSTEMTIME st;
	char buffer[32];

	FileTimeToSystemTime(ft, &st);
	_snprintf(buffer, 32, "%d%02d%02d%02d%02d.%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	std::string cmd = std::string("!touch -m -t ") + buffer + " " + remotePath;

	return doCommand(cmd);
}

//---------------------------------------------------------------------
void Server::setTransferAscii(bool ta) {
	if (!mapper->hDll)
		return;

	mapper->setTransferAscii(ta);
}

//---------------------------------------------------------------------
// show the global config dialog
Sftp4tc const * const Server::doConfig() {
	if (!mapper->hDll)
		return 0;

	return mapper->doConfig(gMainWin, 0, 0);
}
//---------------------------------------------------------------------
// reconfigure current server
Sftp4tc const * const  Server::doSelfConfig() {
	if (!mapper->hDll)
		return 0;

	return mapper->doConfig(gMainWin, 1, 0);
}

//---------------------------------------------------------------------
// apply the configuration
void Server::configure(Sftp4tc const * const cfg)
{
	cacheFolders = cfg->cacheFolders != 0;
	if (!cacheFolders)
		clearDirCache();
	hideDotNames = cfg->hideDotNames != 0;
	defChMod = cfg->defChMod;
	exeChMod = cfg->exeChMod;
	
	std::string ext = cfg->exeExtensions;
	exeExtensions.clear();
	size_t start = (size_t)-1;
	for (size_t i = 0; i < ext.size(); ++i)
	{
		if (ext[i] == '.') start = i;
		else if (ext[i] <= 32) {
			size_t len = i - start;
			std::string x = ext.substr(start, len);
			exeExtensions.insert(std::make_pair(x, x));
			start = (size_t)-1;
		}
	}
	if (start != (size_t)-1) {
		std::string x = ext.substr(start);
		exeExtensions.insert(std::make_pair(x, x));
	}
}
