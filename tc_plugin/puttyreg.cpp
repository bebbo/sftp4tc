#include "stdafx.h"
#include "server.h"

//Registry keys for PuTTY - Session importing
#define PUTTY_REG_POS "Software\\SimonTatham\\PuTTY"
#define PUTTY_REG_PARENT "Software\\SimonTatham"
#define PUTTY_REG_PARENT_CHILD "PuTTY"
#define PUTTY_REG_GPARENT "Software"
#define PUTTY_REG_GPARENT_CHILD "SimonTatham"
static const char *const puttystr = PUTTY_REG_POS "\\Sessions";

struct EnumSettingsType {
	HKEY mKey;
	int i;
};

static void unmungestr(char *in, char *out, int outlen)
{
	while (*in) {
		if (*in == '%' && in[1] && in[2]) {
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

static EnumSettingsType * enum_settings_start(void)
{
	EnumSettingsType *ret;
	HKEY key;

	if (RegOpenKey(HKEY_CURRENT_USER, puttystr, &key) != ERROR_SUCCESS) {
		return NULL;
	}

	ret = new EnumSettingsType();

	if (ret) {
		ret->mKey = key;
		ret->i = 0;
	}
	return ret;
}
//---------------------------------------------------------------------

static char *enum_settings_next(void *handle, char *buffer, int buflen)
{
	struct EnumSettingsType *e = (struct EnumSettingsType *) handle;
	char *otherbuf;

	otherbuf = (char *) malloc(3 * buflen);
	if (otherbuf
		&& RegEnumKey(e->mKey, e->i++, otherbuf,
		3 * buflen) == ERROR_SUCCESS) {
			unmungestr(otherbuf, buffer, buflen);
	} else {
		buffer = 0;
	}
	free(otherbuf);
	return buffer;
}
//---------------------------------------------------------------------
#if 0
static int import_one_putty_session(ServerInfo * serverInfo, char *PuttySectionName)
{
	HKEY sesskey;

	sesskey = (HKEY) open_settings_r(PuttySectionName);
	if (sesskey != NULL) {
		gpps(sesskey, "HostName", "", HostName, sizeof(HostName));
		gpps(sesskey, "Protocol", "", Protocol, sizeof(Protocol));
		gpps(sesskey, "UserName", "", UserName, sizeof(UserName));
		gppi(sesskey, "PortNumber", 22, &PortNumber);
		gppi(sesskey, "Compression", 0, &Compression);
		gpps(sesskey, "PublicKeyFile", "", PublicKeyFile,
			sizeof(PublicKeyFile));

		if (strlen(HostName) > 0 && strcmp(Protocol, "ssh") == 0) {
			if (strstr(HostName,"@")!=NULL) { // get username from hostname if it's there
				int count=strstr(HostName,"@")-HostName;
				strncpy(UserName,HostName,count);
			}
			ServerAccountInfoDefaults(ServerAccountInfo);
			if (!PortNumber || PortNumber < 1) {
				PortNumber = 22;
			}

			if (!Compression || Compression == 0) {
				Compression = 0;
			} else {
				Compression = 1;
			}

			//inherited char*
			strcpy(ServerAccountInfo->host, HostName);
			strcpy(ServerAccountInfo->host_cached, HostName);
			strcpy(ServerAccountInfo->username, UserName);
			strcpy(ServerAccountInfo->username_cached, ""); // MUSS strlen=0 sein  wegen anyconnectionactive()

			// inherited int
			ServerAccountInfo->compression = Compression;
			ServerAccountInfo->port = PortNumber;

			// Static
			ServerAccountInfo->use_key_auth = strlen(PublicKeyFile) > 0;
			ServerAccountInfo->dont_ask4_passphrase = 0;
			strncpy(ServerAccountInfo->keyfilename, PublicKeyFile,
				MAX_SERVER_INFO);
			if (strlen(ServerAccountInfo->username)) {
				ServerAccountInfo->dont_ask4_username = 1;
			} else {
				ServerAccountInfo->dont_ask4_username = 0;
			}

			ServerAccountInfo->is_imported_from_any_datasrc = 1;
			ServerAccountInfo->is_imported_from_putty_registry = 1;

			RegCloseKey(sesskey);
			return 1;
		}
	}
	RegCloseKey(sesskey);
	return 0;
}
#endif

//replace / and \ with _ (in title)
static void force_valid_server_title(char *title)
{
	if (!title)
		return;                     

	while (*title) {
		if ((*title == '/') || (*title == '\\')) {
			*title = '_';
		}
		++title;
	}
}



// Session importing
void import_putty_sessions(std::vector<ServerInfo> & serverInfos)
{
	static char otherbuf[2048];
	char *ret;
	void *handle;

	if ((handle = enum_settings_start())) {

		EnumSettingsType * handleFree = (struct EnumSettingsType *) handle;
		do {
			ret = enum_settings_next(handle, otherbuf, sizeof(otherbuf));

			if (ret) {
				// if (import_one_putty_session(&ServerAccountInfo, otherbuf) == 1)
				{
					// OK success
					force_valid_server_title(otherbuf);
					ServerInfo si;
					si.name = otherbuf;
					serverInfos.push_back(si);
				}
			}
		} while (ret);
		RegCloseKey(handleFree->mKey);
		free(handle);
	}
}

