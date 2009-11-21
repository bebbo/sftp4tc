#include "windows.h"
#include "fsplugin.h"
#include "sftp4tc_share.h"
#include "sftp.h"

extern struct config_tag cfg;

HWND gGlobalHwnd;
RequestProcType gRequestProc;
ProgressProcType gProgressProc;

int gTotalCommanderPluginNr = -1;
char gTotalCommanderLastErrorMessage[1000];

int ProgressProc(char *SourceName, char *TargetName, int PercentDone);
int getPasswordDialog(char * caption, int isPw, char * dest, int len);

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

void wcplg_set_last_error_msg(char *str_) {
  if (str_) {
    strcpy(gTotalCommanderLastErrorMessage, str_);
  } else {
    gTotalCommanderLastErrorMessage[0] = 0;
  }
}
//------------------------------------------------------------------------
// initialize hooks and WND handle
int init_Procs(RequestProcType prequestProc, ProgressProcType pprogressProc, int totalCommaderPluginNr, HWND hwnd) {
  gRequestProc = prequestProc;
  gProgressProc = pprogressProc;
  gTotalCommanderPluginNr = totalCommaderPluginNr;
  gGlobalHwnd = hwnd;
  return 1;
}
