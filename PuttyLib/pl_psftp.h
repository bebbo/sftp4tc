#ifndef _PL_PSFTP_H
#define _PL_PSFTP_H

#include "winstuff.h"
#include "pl_consts.h"

extern int psftp_connect(char *userhost, char *user, int portnumber);
extern int do_sftp_init(void);
struct config_tag;
struct config_tag * wcplg_open_sftp_session(char *userhost, char *user, char *pass, int portnumber);
int wcplg_close_sftp_session(void);
int wcplg_do_sftp(char *_cmd, char *_server_output);
char *wcplg_get_last_error_msg();
struct my_fxp_names *wcplg_get_current_dir_struct();
extern void wcplg_free_current_dir_struct();
extern int getTransferMode(char *filename);

extern Backend *back;
extern void *backhandle;

/*
 moved from old custom psftp.h
 */
int ISinitT;
char *pwd, *homedir;

struct my_fxp_names CurrentDirStruct;

// int names_to_freeing_num;

#endif
