#include "putty.h"
#include "pl_misc.h"
#include "misc.h"
#include "sftp4tc_share.h"
#include "psftp.h"
#include "pl_psftp.h"
#include "pl_consts.h"
#include "fsplugin.h"
//#include "pl_console.h"

extern RequestProcType gRequestProc;
extern ProgressProcType gProgressProc;
extern int gTotalCommanderPluginNr;

extern char *console_password;
extern char *server_output;
extern Config cfg;
extern Config savedCfg;
extern struct sftp_command *sftp_getcmd(FILE * fp, int mode, int modeflags);

extern int disconnected;
extern int loaded_session;

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

extern char * selectedSession;
char connectMsg[256];
int connectPercent;

struct config_tag * wcplg_open_sftp_session(char *userhost, char *user, char *pass, int portnumber) {
  wcplg_set_last_error_msg(NULL);
  strncpy(connectMsg, userhost, 255);

  if (ProgressProc("connecting", connectMsg, 0) == 1) {
    wcplg_set_last_error_msg("cancel by user");
    return RESULT_ERR;
  }

  if (!user && !pass && !portnumber) {
    if (!cfg.host[0])
      do_defaults(userhost, &cfg);
    userhost = cfg.host;
    user = cfg.username;
    portnumber = cfg.port;
    loaded_session = 1;
  } else {
    console_password = dupstr(pass);
    flags = 0; //FLAG_STDERR | FLAG_INTERACTIVE;
    cmdline_tooltype = TOOLTYPE_FILETRANSFER;
    //ssh_get_line = &console_get_line;
    do_defaults(NULL, &cfg);
  }

  //  init_winsock();
  sk_init();

  back = NULL;

  if (userhost) {
    if (psftp_connect(userhost, user, portnumber)) {
      return RESULT_ERR;
    }

    if (1 == ProgressProc("connecting", connectMsg, 99))
      return RESULT_ERR;

    if (do_sftp_init()) {
      return RESULT_ERR;
    }
  } else {
    printf("psftp: no hostname specified; use \"open host.name\" to connect\n");
    return RESULT_ERR;
  }

  ISinitT = 1;

  disconnected = 0;

  cfg.sftp4tc.selectedSession = selectedSession;

  return &cfg;
}

int wcplg_close_sftp_session() {

  wcplg_set_last_error_msg("");

  if ((back != NULL) && (back->connected(backhandle))) {
    char ch;
    back->special(backhandle, TS_EOF);
    sftp_recvdata(&ch, 1);
  }

  // psftp_memory_hole__stopfen();
  random_save_seed();
  return 1;
}

int wcplg_do_sftp(char *_cmd, char *_server_output) {

  FILE *fp;
  int ret = 0, i2 = 0, i3 = 0;
  char batchfile[MAX_PATH];
  char *_cmd_ln = NULL;
  struct sftp_command *cmd;

  server_output = _server_output;

  _cmd_ln = (char *) malloc(strlen(_cmd) + 2);
  wcplg_set_last_error_msg("");
  strcpy(_cmd_ln, _cmd);
  strcat(_cmd_ln, "\n");

  get_tmp_file_name(batchfile);

  fp = fopen(batchfile, "w");
  if (!fp) {
    printf("Fatal: unable to open %s\n", batchfile);
    return 2;
  }

  fwrite(_cmd_ln, sizeof(char), strlen(_cmd_ln), fp);
  fclose(fp);
  free(_cmd_ln);

  fp = fopen(batchfile, "r");
  if (!fp) {
    printf("Fatal: unable to open %s\n", batchfile);
    return 2;
  }
  //sftp_getcmd() Speicherloch!!! FIXME
  cmd = sftp_getcmd(fp, 1, 1);

  // löschen
  fclose(fp);

  remove(batchfile);

  //cmd->words=  eventl. noch zusammenbasteln

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
  }
  CurrentDirStruct.nnames = 0;
}

wchar_t * toWideChar(char * str) {
  wchar_t * p;
  int len;
  int cp;

  if (!str)
    return 0;

  cp = cfg.sftp4tc.codePage;
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
  wchar_t buf1[1024];
  wchar_t buf2[1024];
  int progress;
  if (gProgressProc != NULL && gTotalCommanderPluginNr != -1) {
    if (cfg.sftp4tc.isUnicode) {
      int cp = cfg.sftp4tc.codePage;
      int len = MultiByteToWideChar(cp, 0, SourceName, -1, buf1, 1024);
      if (!len) {
        cp = CP_ACP;
        len = MultiByteToWideChar(cp, 0, SourceName, -1, buf1, 1024);
      }
      len = MultiByteToWideChar(cp, 0, TargetName, -1, buf2, 1024);
      // hack o - tcmd expects wchar_t!
      SourceName = (char *) &buf1[0];
      TargetName = (char *) &buf2[0];
    }
    progress = gProgressProc(gTotalCommanderPluginNr, SourceName, TargetName, PercentDone);
    return progress;
  }
  return 0;
}

//------------------------------------------------------------------------
// call the progress bar - also perform multibyte conversion, if necessary
int getPasswordDialog(char * caption, int isPw, char * dest, int len) {
  wchar_t buf1[1024];
  wchar_t buf2[1024];
  char * pwd = dest;
  int cp = cfg.sftp4tc.codePage;
  int r;

  if (cfg.sftp4tc.isUnicode) {
    int xlen = MultiByteToWideChar(cp, 0, caption, -1, buf1, 1024);
    if (!xlen) {
      cp = CP_ACP;
      xlen = MultiByteToWideChar(cp, 0, caption, -1, buf1, 1024);
    }
    caption = (char *) &buf1[0];
    dest = (char *) &buf2[0];
	buf2[0] = 0;
  } else {
	  *dest = 0;
  }
  r = gRequestProc(gTotalCommanderPluginNr, isPw ? RT_UserName : RT_Password, caption, NULL, dest, 1023);
  if (cfg.sftp4tc.isUnicode) {
    WideCharToMultiByte(cp, 0, buf2, -1, pwd, len, 0, 0);
  }
  return r;
}

