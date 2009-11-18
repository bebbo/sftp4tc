#include "windows.h"
#include "fsplugin.h"
#include "share.h"
#include "sftp.h"

tProgressProc P_ProgressProc = NULL;
int wc_PluginNr = -1;
char WC_PLG_LAST_ERROR_MSG[1000];

char *wcplg_get_last_error_msg()
{
  if (strlen(WC_PLG_LAST_ERROR_MSG) < 1) {
    if (fxp_error())
      strcpy(WC_PLG_LAST_ERROR_MSG, (char *) fxp_error());
    else
      WC_PLG_LAST_ERROR_MSG[0] = 0;
  }

  return WC_PLG_LAST_ERROR_MSG;
}

void wcplg_set_last_error_msg(char *str_)
{
  if (str_) {
    strcpy(WC_PLG_LAST_ERROR_MSG, str_);
  } else {
    WC_PLG_LAST_ERROR_MSG[0] = 0;
  }
}


int ProgressProc(char *SourceName, char *TargetName, int PercentDone)
{
  int progress;

  if (P_ProgressProc != NULL && wc_PluginNr != -1) {
    progress =
      P_ProgressProc(wc_PluginNr, SourceName, TargetName, PercentDone);
    return progress;
  }
  return RESULT_ERR;
}

int init_ProgressProc(tProgressProc AP_ProgressProc, int Awc_PluginNr)
{
  P_ProgressProc = AP_ProgressProc;
  wc_PluginNr = Awc_PluginNr;
  return 1;
}
