#include "putty.h"
#include "pl_misc.h"
#include "misc.h"
#include "share.h"
#include "psftp.h"
#include "pl_psftp.h"
#include "pl_consts.h"
//#include "pl_console.h"

extern char *console_password;
extern Config cfg;
extern struct sftp_command *sftp_getcmd(FILE * fp, int mode,
                                        int modeflags);

/*extern*/ struct sftp_command {
  char **words;
  int nwords, wordssize;
  int (*obey) (struct sftp_command *);  /* returns <0 to quit */
};

static void init_winsock(void)
{
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

int wcplg_open_sftp_session(char *userhost, char *user, char *pass,
                            int portnumber)
{
  wcplg_set_last_error_msg(NULL);

  if (ProgressProc("", "", 0) == 1) {
    wcplg_set_last_error_msg("cancel by user");
    return RESULT_ERR;
  }

  console_password = dupstr(pass);
  flags = 0;                    //FLAG_STDERR | FLAG_INTERACTIVE;
  cmdline_tooltype = TOOLTYPE_FILETRANSFER;
  ssh_get_line = &console_get_line;

  do_defaults(NULL, &cfg);

  init_winsock();
  sk_init();

  back = NULL;

  if (userhost) {
    if (psftp_connect(userhost, user, portnumber)) {
      return RESULT_ERR;
    }

    if (do_sftp_init()) {
      return RESULT_ERR;
    }
  } else {
    printf
      ("psftp: no hostname specified; use \"open host.name\" to connect\n");
    return RESULT_ERR;
  }

  ISinitT = 1;

  return RESULT_OK;
}

int wcplg_close_sftp_session()
{

  wcplg_set_last_error_msg("");

  if ((back != NULL) && (back->socket(backhandle) != NULL)) {
    char ch;
    back->special(backhandle, TS_EOF);
    sftp_recvdata(&ch, 1);
  }

  psftp_memory_hole__stopfen();
  random_save_seed();
  return 1;
}

int wcplg_do_sftp(char *_cmd, char *_server_output)
{

  FILE *fp;
  int ret = 0, i2 = 0, i3 = 0;
  char batchfile[MAX_PATH];
  char *_cmd_ln = NULL;
  struct sftp_command *cmd;


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


struct my_fxp_names *wcplg_get_current_dir_struct()
{
  return &CurrentDirStruct;
}

int psftp_memory_hole__stopfen(void)
{
  int i = 0;
  for (; i < names_to_freeing_num && i < MAX_DIR_FREE_NNAME_CACHE; i++) {
    //fxp_free_name(names_to_freeing[i]);
  }

  names_to_freeing_num = 0;
  return i;
}
