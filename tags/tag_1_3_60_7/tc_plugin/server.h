#ifndef __SERVER_H__
#define __SERVER_H__

#include <map>
#include <vector>
#include <string>

#ifndef PUTTY_PUTTY_H
#include <putty.h>
#endif

#ifndef __SFTP_H__
#include <sftp.h>
#endif

#include <bstring.h>
#include <string>

#ifdef UNICODE
typedef std::wstring bstring;
#else
typedef std::string bstring;
#endif

#include <sftpmap.h>

class Server;
typedef std::map<bstring, Server *> ServerMap;
typedef std::map<bstring, my_fxp_names *> DirCache;

class Server {
    friend class Guard;

    static ServerMap serverMap;
    static std::vector<ServerInfo> serverInfos;

    // initialized during CT
    bstring name;
    PsftpMapper * currentMapper;
    bool disableMtime;

    // init with default
    DirCache dirCache;

    // config settings
    bstring homeDir;
    bool cacheFolders;
    bool hideDotNames;
    bstring defChMod;
    bstring exeChMod;
    std::map<bstring, bstring> exeExtensions;

    config_tag * myCfg;

  public:
    // ct / dt
    Server(bstring const & name);
    ~Server();

    // open global config panel
    config_tag * const doConfig();
    // open config panel for running session
    config_tag * const doSelfConfig();

    // performs a server lookup by evaluating the remote path
    // on success the remote path is also set
    static Server * findServer(bstring & remotePath, bchar const * const fullPath);
    static bool removeServer(bchar const * serverName);
    // inject a quick connection
    static void insertServer(bstring const & serverName, Server * server);

    // set the transfer mode
    void setTransferAscii(bool ta);

    // reload the config - return count of servers
    static size_t loadServers();
    // get the server name for the index
    static bchar const * const getServerName(size_t index);

    bool connect();
    void configure(config_tag * const sessionCfg);

    // used as ls - includes caching
    my_fxp_names * getDirContent(bstring const & path);

    void invalidateDirContent(bstring const & path);
    bool remoteFileExists(bstring const & remotePath);

    // the server commands
    bool cmdChmod(bstring const & remotePath, int flags);
    bool cmdMtime(bstring const & remotePath, FILETIME * ft);
    bool cmdMkDir(bstring const & remotePath);
    bool cmdGet(bstring const & remotePath, bstring const & localName, bool exists);
    bool cmdPut(bstring const & localName, bstring const & remotePath, bool exists);
    bool cmdRm(bstring const & remotePath);
    bool cmdRmDir(bstring const & remotePath);
    bool cmdLs(bstring const & remotePath);
    bool cmdMove(bstring const & oldRemotePath, bstring const & newRemotePath);

    // central handler
    bool doCommand(bstring const & command, bstring & response = bstring());

    // lookup the home folder
    bool getHomeDir(bstring & response);

    inline bstring const & getName() const {
      return name;
    }
    inline bool isHideDotNames() const {
      return hideDotNames;
    }

    void updateFileAttr(bstring const & path, bstring const & file, bstring const & attrs);
  private:

    // prohibit copies
    Server(Server const &);
    Server & operator =(Server const &);

    // maintain the dir cache
    void updateDirCache(bstring const & path, my_fxp_names * dir);
    fxp_name * removeFile(bstring const & remotePath);
    void insertFile(bstring const & remotePath, FILETIME * ft, unsigned long szLo, unsigned long szHi, char kind,
        bstring const & chmod = TEXT("---"));

    // cleanup
    void clearDirCache();
};

#endif //__SERVER_H__
