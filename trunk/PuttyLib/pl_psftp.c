#include "putty.h"
#include "pl_misc.h"
#include "misc.h"
#include "sftp4tc_share.h"
#include "psftp.h"
#include "pl_psftp.h"
#include "pl_consts.h"
#include "fsplugin.h"
//#include "pl_console.h"

extern char *console_password;
extern char *server_output;
struct Sftp4tc cfg;
extern struct sftp_command *line2command(char * line, int mode, int modeflags);

extern int disconnected;
extern int loaded_session;

HWND gGlobalHwnd;
tRequestProcW gRequestProc;
tProgressProcW gProgressProc;
tCryptProcW gCryptProc;

int gPluginNumber = -1;
int gCryptoNumber = -1;
wchar_t * gSessionName;

char gTotalCommanderLastErrorMessage[1000];

char *wcplg_get_last_error_msg() {
	if (strlen(gTotalCommanderLastErrorMessage) < 1) {
		if (fxp_error()) {
			strcpy(gTotalCommanderLastErrorMessage, (char *) fxp_error());
		} else {
			gTotalCommanderLastErrorMessage[0] = 0;
		}
	}

	return gTotalCommanderLastErrorMessage;
}

void wcplg_set_last_error_msg(char const *str_) {
	if (str_) {
		strcpy(gTotalCommanderLastErrorMessage, str_);
	} else {
		gTotalCommanderLastErrorMessage[0] = 0;
	}
}
//------------------------------------------------------------------------
// initialize hooks and WND handle
int init_Procs(tRequestProcW prequestProc, tProgressProcW pprogressProc, tCryptProcW cryptProc,
		int totalCommaderPluginNr, int cryptoNr, HWND hwnd, wchar_t  * sessionName) {
	gRequestProc = prequestProc;
	gProgressProc = pprogressProc;
	gCryptProc = cryptProc;
	gPluginNumber = totalCommaderPluginNr;
	gCryptoNumber = cryptoNr;
	gGlobalHwnd = hwnd;
	gSessionName = sessionName;

	return 1;
}

HANDLE myCreateFile(char * name, DWORD a, DWORD b, LPSECURITY_ATTRIBUTES p,
	DWORD d, DWORD e, HANDLE f) {
	wchar_t buff[1024] = { 0 };
	int cp = cfg.codePage;
	if (!MultiByteToWideChar(cp, 0, name, -1, buff, 1024)) {
		cp = CP_ACP;
		MultiByteToWideChar(cp, 0, name, -1, buff, 1024);
	}

	return CreateFileW(buff, a, b, p, d, e, f);
}

/*extern*/struct sftp_command {
	char **words;
	int nwords, wordssize;
	int (*obey)(struct sftp_command *); /* returns <0 to quit */
};

static void init_winsock(void) {
	WORD winsock_ver;
	WSADATA wsadata;

	winsock_ver = MAKEWORD(1, 1);
	if (WSAStartup(winsock_ver, &wsadata)) {
		fprintf(stderr, "Unable to initialise WinSock");
		cleanup_exit(1);
	}
	if (LOBYTE(wsadata.wVersion) != 1 || HIBYTE(wsadata.wVersion) != 1) {
		fprintf(stderr, "WinSock version is incompatible with 1.1");
		cleanup_exit(1);
	}
}

char connectMsg[256];
int connectPercent;

void configure(char const * sessionName) {
	strcpy(cfg.session, sessionName);
	if (!cfg.config) {
		cfg.config = conf_new();
		do_defaults((char *)sessionName, &cfg);
	} else if (cfg.copyMe) {
		// quick connection provides a config object, if open was pressed
		// take a copy, since it belongs to a different DLL
		cfg.config = conf_copy(cfg.config);
		cfg.copyMe = 0;
	}
	updateSftpCfg(&cfg);
}

struct Sftp4tc * wcplg_open_sftp_session(char const * displayName, char const *sessionName) {
	char * cfgHost, * user;
	int portnumber;

	wcplg_set_last_error_msg(NULL);
	strncpy(connectMsg, displayName, 255);

	if (ProgressProc("connecting", connectMsg, 0) == 1) {
		wcplg_set_last_error_msg("cancel by user");
		return RESULT_ERR;
	}

	configure(sessionName);

	cfgHost = conf_get_str(cfg.config, CONF_host);

	user = conf_get_str(cfg.config, CONF_username);
	// no username?
	if (!user || !*user) {
		char * at = strchr(cfgHost, '@');
		if (at) {
			static char huser[256];
			strncpy(huser, cfgHost, 255);
			huser[255] = 0;
			at = strchr(huser, '@');
			*at = 0;
			user = huser;
		}
	}
	portnumber = conf_get_int(cfg.config, CONF_port);

	//  init_winsock();
	sk_init();

	back = NULL;

	if (cfgHost) {
		loaded_session = 1;
		if (psftp_connect(cfgHost, user, portnumber)) {
			return RESULT_ERR;
		}

		if (1 == ProgressProc("connecting", connectMsg, 99))
			return RESULT_ERR;

		if (do_sftp_init()) {
			return RESULT_ERR;
		}
	} else {
		printf(
				"psftp: no hostname specified; use \"open host.name\" to connect\n");
		return RESULT_ERR;
	}

	ISinitT = 1;

	disconnected = 0;

// keep it
//	conf_free(cfg.config);
//	cfg.config = 0;

	return &cfg;
}

int wcplg_close_sftp_session() {

	wcplg_set_last_error_msg("");

	if ((back != NULL) && (back->connected(backhandle))) {
		char ch;
		back->special(backhandle, TS_EOF);
		sftp_recvdata(&ch, 1);
	}

	random_save_seed();
	return 1;
}

int wcplg_do_sftp(char *line, char *_server_output) {

	int ret = 0;
	struct sftp_command *cmd;

	server_output = _server_output;

	cmd = line2command(line, 1, 1);

	if (cmd) {
		ret = cmd->obey(cmd);
	}

	return ret;
}

struct my_fxp_names *wcplg_get_current_dir_struct() {
	return &CurrentDirStruct;
}

void wcplg_free_current_dir_struct() {
	int i;
	for (i = 0; i < CurrentDirStruct.nnames; ++i) {
		struct fxp_name * fn = CurrentDirStruct.names[i];
		free(fn->filename);
		free(fn->longname);
		free(fn->ucFilename);
		free(fn);
		CurrentDirStruct.names[i] = 0;
	}
	CurrentDirStruct.nnames = 0;
}

wchar_t * toWideChar(char * str) {
	wchar_t * p;
	int len;
	int cp;

	if (!str)
		return 0;

	cp = cfg.codePage;
	len = MultiByteToWideChar(cp, 0, str, -1, 0, 0);
	if (!len) {
		cp = CP_ACP;
		len = MultiByteToWideChar(cp, 0, str, -1, 0, 0);
	}
	p = (wchar_t *) malloc(len + len);
	MultiByteToWideChar(cp, 0, str, -1, p, len);
	return p;
}

//------------------------------------------------------------------------
// call the progress bar - also perform multibyte conversion, if necessary
int ProgressProc(char *SourceName, char *TargetName, int PercentDone) {
	static wchar_t buf1[1024];
	static wchar_t buf2[1024];
	int progress;
	if (gProgressProc != NULL && gPluginNumber != -1) {
			int cp = cfg.codePage;
			int len = MultiByteToWideChar(cp, 0, SourceName, -1, buf1, 1024);
			if (!len) {
				cp = CP_ACP;
				len = MultiByteToWideChar(cp, 0, SourceName, -1, buf1, 1024);
			}
			len = MultiByteToWideChar(cp, 0, TargetName, -1, buf2, 1024);
		progress = gProgressProc(gPluginNumber, buf1,
				buf2, PercentDone);
		return progress;
	}
	return 0;
}

//------------------------------------------------------------------------
// call the request proc - also perform multibyte conversion, if necessary

int firstPwdPrompt;

int getPasswordDialog(char * caption, int showClearText, char * dest, int len) {
	wchar_t buf2[1024];
	int cp = cfg.codePage;
	int r = 1;

	buf2[0] = 0;
	if (!cfg.storePassword || !firstPwdPrompt || showClearText || !gCryptProc || gCryptProc(gPluginNumber, gCryptoNumber, FS_CRYPT_LOAD_PASSWORD, gSessionName, buf2, 1023) != FS_FILE_OK) {
		wchar_t buf1[1024];

		int xlen = MultiByteToWideChar(cp, 0, caption, -1, buf1, 1024);
		if (!xlen) {
			cp = CP_ACP;
			xlen = MultiByteToWideChar(cp, 0, caption, -1, buf1, 1024);
		}
		r = gRequestProc(gPluginNumber, showClearText ? RT_UserName : RT_Password,
				buf1, NULL, buf2, 1023);
		if (r && cfg.storePassword && !showClearText && gCryptProc) {
			gCryptProc(gPluginNumber, gCryptoNumber, FS_CRYPT_SAVE_PASSWORD, gSessionName, buf2, 1023);
		}
	}
	firstPwdPrompt = 0;
	WideCharToMultiByte(cp, 0, buf2, -1, dest, len, 0, 0);
	return r;
}

//------------------------------------------------------------------------
// call the crypt proc - also perform multibyte conversion, if necessary
int CryptProc(int cryptoNr, int mode, char *ConnectionName, char *Password, int maxlen) {
	int r;
	static wchar_t buf1[1024];
	static wchar_t buf2[1024];
	if (gCryptProc != NULL && gPluginNumber != -1) {
		int cp = cfg.codePage;
		int len = MultiByteToWideChar(cp, 0, ConnectionName, -1, buf1, 1024);
		if (!len) {
			cp = CP_ACP;
			len = MultiByteToWideChar(cp, 0, ConnectionName, -1, buf1, 1024);
		}
		buf2[0] = 0;
		r = gCryptProc(gPluginNumber, cryptoNr, mode, buf1, buf2, 1023);

		WideCharToMultiByte(cp, 0, buf2, -1, Password, maxlen, 0, 0);
		return r;
	}
	return FS_FILE_NOTSUPPORTED;
}

