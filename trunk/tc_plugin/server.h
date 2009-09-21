#ifndef __SERVER_H__
#define __SERVER_H__

#include <map>
#include <vector>
#include <string>

#ifndef __SFTP_H__
#include "sftp.h"
#endif

#include "sftpmap.h"

struct ServerInfo {
	std::string name;
};

class Server;
typedef std::map<std::string, Server *> ServerMap;
typedef std::map<std::string, my_fxp_names *> DirCache;

class Server
{
	static ServerMap serverMap;
	static std::vector<ServerInfo> serverInfos;

	// initialized during CT
	std::string name;
	PsftpMapper * mapper;
	bool disableMtime;

	// init with default
	DirCache dirCache;

	// config settings
	std::string homeDir;
	bool cacheFolders;
	bool hideDotNames;
	std::string defChMod;
	std::string exeChMod;
	std::map<std::string, std::string> exeExtensions;

public:
	// ct / dt
	Server(std::string const & name);
	~Server();

	// open global config panel
	Sftp4tc const * const doConfig();
	// open config panel for running session
	Sftp4tc const * const doSelfConfig();

	// performs a server lookup by evaluating the remote path
	// on success the remote path is also set
	static Server * findServer(std::string & remotePath, char const * const fullPath);
	static bool disconnectServer(char const * serverName);

	// set the transfer mode
	void setTransferAscii(bool ta);

	// reload the config - return count of servers
	static size_t loadServers();
	// get the server name for the index 
	static char const * const getServerName(size_t index);

	void configure(Sftp4tc const * const sessionCfg);

	// used as ls - includes caching
	my_fxp_names * getDirContent(std::string const & path);

	void invalidateDirContent(std::string const & path);
	bool remoteFileExists(std::string const & remotePath);

	// the server commands
	bool cmdChmod(std::string const & remotePath, int flags);
	bool cmdMtime(std::string const & remotePath, FILETIME * ft);
	bool cmdMkDir(std::string const & remotePath);
	bool cmdGet(std::string const & remotePath, std::string const & localName, bool exists);
	bool cmdPut(std::string const & localName, std::string const & remotePath, bool exists);
	bool cmdRm(std::string const & remotePath);
	bool cmdRmDir(std::string const & remotePath);
	bool cmdLs(std::string const & remotePath);
	bool cmdMove(std::string const & oldRemotePath, std::string const & newRemotePath);

	// central handler
	bool doCommand(std::string const & command, std::string & response = std::string());

	inline std::string const & getName() const { return name; }
	inline bool isHideDotNames() const { return hideDotNames; }

	void updateFileAttr(std::string const & path, std::string const & file, std::string const & attrs);
private:


	// prohibit copies
	Server(Server const &);
	Server & operator =(Server const &);

	// maintain the dir cache
	void updateDirCache(std::string const & path, my_fxp_names * dir);
	fxp_name * removeFile(std::string const & remotePath);
	void insertFile(std::string const & remotePath, FILETIME * ft, unsigned long szLo, unsigned long szHi, char kind, std::string const & chmod = "---");

	// cleanup
	void clearDirCache();
};

#endif //__SERVER_H__