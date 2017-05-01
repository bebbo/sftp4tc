#ifndef PL_PUTTY_MISC_H
#define PL_PUTTY_MISC_H

#include "fsplugin.h"
#include "sftp.h"

char *wcplg_get_last_error_msg();
void wcplg_set_last_error_msg(const char *str_);

int init_Procs(tRequestProcW AP_RequestProc, tProgressProcW AP_ProgressProc,
			   tCryptProcW cryptProc, int PluginNr, int CryptoNr, HWND hwnd, wchar_t * sessionName);

extern int ProgressProc(char *SourceName, char *TargetName, int PercentDone);
extern int getPasswordDialog(char * caption, int isPw, char * dest, int len);
extern int CryptProc(int cryptoNr, int mode, char *ConnectionName, char *Password, int maxlen);

#endif
