#include "windows.h"
#include "fsplugin.h"
#include "sftp4tc_share.h"
#include "sftp.h"

ProgressProcType gProgressProc = NULL;
int gTotalCommanderPluginNr = -1;
char gTotalCommanderLastErrorMessage[1000];

char *wcplg_get_last_error_msg()
{
  if (strlen(gTotalCommanderLastErrorMessage) < 1) 
  {
    if (fxp_error())
    {
      strcpy(gTotalCommanderLastErrorMessage, (char *) fxp_error());
    }
    else
    {
      gTotalCommanderLastErrorMessage[0] = 0;
    }
  }

  return gTotalCommanderLastErrorMessage;
}

void wcplg_set_last_error_msg(char *str_)
{
  if (str_) 
  {
    strcpy(gTotalCommanderLastErrorMessage, str_);
  } 
  else 
  {
    gTotalCommanderLastErrorMessage[0] = 0;
  }
}


int ProgressProc(char *SourceName, char *TargetName, int PercentDone)
{
  int progress;

  if (gProgressProc != NULL && gTotalCommanderPluginNr != -1) {
    progress =
      gProgressProc(
        gTotalCommanderPluginNr, 
        SourceName, 
        TargetName, 
        PercentDone);
    return progress;
  }
  return RESULT_ERR;
}

int init_ProgressProc(ProgressProcType pprogressProc, int totalCommaderPluginNr)
{
  gProgressProc = pprogressProc;
  gTotalCommanderPluginNr = totalCommaderPluginNr;
  return 1;
}
