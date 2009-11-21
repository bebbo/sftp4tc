#ifndef PL_PUTTY_MISC_H
#define PL_PUTTY_MISC_H

#include "sftp.h"

char *wcplg_get_last_error_msg();
void wcplg_set_last_error_msg(const char *str_);

typedef int (__stdcall * tProgressProc)(int PluginNr, char *SourceName, char *TargetName, int PercentDone);
typedef int (__stdcall * tRequestProcType)(int PluginNr, int RequestType, char *CustomTitle, char *CustomText,
    char *ReturnedText, int maxlen);

int ProgressProc(char *SourceName, char *TargetName, int PercentDone);
int init_Procs(tRequestProcType AP_RequestProc, tProgressProc AP_ProgressProc, int Awc_PluginNr, HWND hwnd);

#endif
