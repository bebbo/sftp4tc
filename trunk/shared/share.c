#include "share.h"

struct SftpServerAccountInfo MyServerAccountInfo;
unsigned long tmp_count=0;
int disable_run_time_logging=0;
int disable_run_time_logging_ONCE=0;

int get_tmp_file_name(char *buf)
	{
	
	char buf2[MAX_PATH];
	struct _timeb timebuffer;
	char *timeline;

	tmp_count++;
	
	_ftime( &timebuffer );
	timeline = ctime( & ( timebuffer.time ) );
	GetTempPath(MAX_PATH,buf);
	sprintf(buf2,"%i%i%i",timebuffer.time,timebuffer.millitm,tmp_count);
	
	strcat(buf,"wcsftp_");
	strcat(buf,buf2);
	strcat(buf,".tmp");

	if (tmp_count > 60000) tmp_count=0;
	puts(buf);
	return 1;

	}

//This is ONLY 4 psftp.dll
struct SftpServerAccountInfo get_Server_config_Struct(void)
	{
		//if (strcmp(MyServerAccountInfo.username,"www105") == 0) exit(0);
		return MyServerAccountInfo;
	}

//This is ONLY 4 psftp.dll
void set_Server_config_Struct(struct SftpServerAccountInfo ServerAccountInfo)
	{
		MyServerAccountInfo	=	ServerAccountInfo;
		//if (strcmp(MyServerAccountInfo.username,"www105") == 0) exit(0);
	}


void DISABLE_LOGGING()	
	{
		disable_run_time_logging=1;
	}
void DISABLE_LOGGING_ONCE()	
	{
		disable_run_time_logging_ONCE=1;
	}
void ENABLE_LOGGING()
	{
		disable_run_time_logging=0;
	}

int do_logging()
	{
		if (disable_run_time_logging_ONCE == 1)
			{
				disable_run_time_logging_ONCE=0;
				return 0;
			}
		if (disable_run_time_logging==1) return 0;
		return 1;
	}

void trim_host_from_hoststring(char *hoststring)
	{
		char *buf;
		if (hoststring == NULL) return;
		buf		=			strstr(hoststring,":");
		if (buf == NULL)	return;//Kein Port angegeben
		hoststring[strlen(hoststring)-strlen(buf)]='\0';
	}
void trim_port_from_hoststring(char *hoststring)
	{
		char *buf;
		if (hoststring == NULL) return;
		buf		=			strstr(hoststring,":");
		if (buf == NULL)
			{
				strcpy(hoststring,"");
				return;//Kein Port angegeben
			}
		buf++;// : überspringen
		strcpy(hoststring,buf);
		//hoststring+=(strlen(hoststring)-strlen(buf)+1);
	}
