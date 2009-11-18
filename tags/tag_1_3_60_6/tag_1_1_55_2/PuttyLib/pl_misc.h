#ifndef PL_PUTTY_MISC_H
#define PL_PUTTY_MISC_H

#include "sftp.h"

char *wcplg_get_last_error_msg();
void wcplg_set_last_error_msg(const char *str_);

typedef int (__stdcall *tProgressProc)(int PluginNr,char* SourceName,
             char* TargetName,int PercentDone);


int ProgressProc(char* SourceName,char* TargetName,int PercentDone);
int init_ProgressProc(tProgressProc AP_ProgressProc, int Awc_PluginNr);
int psftp_memory_hole__stopfen(void);

#endif