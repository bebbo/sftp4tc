#include "share.h"

struct SftpServerAccountInfo MyServerAccountInfo;
unsigned long tmp_count = 0;

int get_tmp_file_name(char *buf)
{

  char buf2[MAX_PATH];
  struct timeb timebuffer;
  char *timeline;

  tmp_count++;

  ftime(&timebuffer);
  timeline = ctime(&(timebuffer.time));
  GetTempPath(MAX_PATH, buf);
  sprintf(buf2, "%i%i%i", timebuffer.time, timebuffer.millitm, tmp_count);

  strcat(buf, "wcsftp_");
  strcat(buf, buf2);
  strcat(buf, ".tmp");

  if (tmp_count > 60000)
    tmp_count = 0;
  puts(buf);
  return 1;
}

//This is ONLY 4 psftp.dll
struct SftpServerAccountInfo *get_Server_config_Struct(void)
{
  //if (strcmp(MyServerAccountInfo.username,"www105") == 0) exit(0);
  return &MyServerAccountInfo;
}

//This is ONLY 4 psftp.dll
void set_Server_config_Struct(struct SftpServerAccountInfo
                              ServerAccountInfo)
{
  MyServerAccountInfo = ServerAccountInfo;
  //if (strcmp(MyServerAccountInfo.username,"www105") == 0) exit(0);
}
