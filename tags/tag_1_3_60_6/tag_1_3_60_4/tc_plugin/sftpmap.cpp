#include "stdafx.h"
#include "sftpmap.h"

extern "C" {
#include "../shared/sftp4tc_share.h"
}

#define MAX_CMD_BUFFER 1024

//---------------------------------------------------------------------
extern int gPluginNumber;
extern ProgressProcType gProgressProc;
extern LogProcType gLogProc;
extern RequestProcType gRequestProc;

extern HMODULE ghThisDllModule;

//---------------------------------------------------------------------
// cheat the progressProc with remote -> remote copies
bool firstHalf;
bool secondHalf;

//---------------------------------------------------------------------
// show some progress
static int __stdcall myProgressProc(int PluginNr, char *SourceName,
                                         char *TargetName,
                                         int PercentDone)
{
	if (firstHalf || secondHalf) {
		PercentDone >>= 1;
		if (secondHalf)
			PercentDone += 50;
	}

	int r = gProgressProc(PluginNr, SourceName, TargetName, PercentDone);

	// DBGPRINT(("gProgressProc: %d\r\n", r));

	if (r) return r;

	MSG msg;
	for (int i = 0; i < 42; ++i) {
		if (!PeekMessage(&msg, 0, 0, 0, 0))
			break;

		if (GetMessage(&msg, 0, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);		
		}
	}

	return gProgressProc(PluginNr, SourceName, TargetName, PercentDone);
}
//---------------------------------------------------------------------
// get the psftp.dll path
static void GetPsftpDllPath(char* buf)
{
	char plugin_dir[MAX_CMD_BUFFER];
	GetModuleFileName(ghThisDllModule, plugin_dir, MAX_CMD_BUFFER - 10);

	strcpy(buf, plugin_dir);

	char *p = strrchr(buf, '\\');
	if (!p) p = buf;

	strcpy(p, "\\psftp.dll");
}

//---------------------------------------------------------------------
// check file existance
static int FileExists(char const * const fname)
{
	OFSTRUCT ofs;
	HFILE hf = OpenFile(fname, &ofs, OF_EXIST);
	int exists = hf != HFILE_ERROR;
	if (exists) _lclose(hf);

	return exists;
}


//---------------------------------------------------------------------
// DT - cleanup
PsftpMapper::~PsftpMapper()
{
	cleanup();
}

//---------------------------------------------------------------------
// the cleanup method
void PsftpMapper::cleanup() {
	if (hDll) {
		FreeLibrary(hDll);
		hDll = 0;
	}
	
	if (dllName.size() > 0) {
		OFSTRUCT ofs;
		HFILE hf = OpenFile(dllName.c_str(), &ofs, OF_DELETE);
		if (hf != HFILE_ERROR) _lclose(hf);
		dllName = "";
	}
}

//---------------------------------------------------------------------
// CT - create a new mapper
PsftpMapper::PsftpMapper(std::string const & serverName, config_tag * cfg)
: hDll(0)
{
	char tempPathBuffer[MAX_CMD_BUFFER];
	char dll_2_copy[MAX_CMD_BUFFER];
	GetTempPath(MAX_CMD_BUFFER, tempPathBuffer);

	std::string dll_2_load = tempPathBuffer;
	dll_2_load += "psftp_" + serverName + ".dll";

	GetPsftpDllPath(dll_2_copy);
	if (!FileExists(dll_2_copy))
		return;


	CopyFile(dll_2_copy, dll_2_load.c_str(), 0);

#ifdef _DEBUG
	hDll = LoadLibrary(dll_2_copy);
#else
	hDll = LoadLibrary(dll_2_load.c_str());
#endif

	if (!hDll)
		return;

	dllName = dll_2_load;

	this->connect =
		(PsftpConnectProcType) GetProcAddress(hDll, "__map__wcplg_open_sftp_session");
	this->disconnect =
		GetProcAddress(hDll, "__map__wcplg_close_sftp_session");
	this->doSftp =
		(PsftpDoSftpProcType) GetProcAddress(hDll, "__map__wcplg_do_sftp");
	this->getCurrentDirStruct =
		(PsftpGetCurrentDirStructProcType) GetProcAddress(hDll, "__map__wcplg_get_current_dir_struct");
	this->freeCurrentDirStruct =
		(PsftpFreeCurrentDirStructProcType) GetProcAddress(hDll, "__map__wcplg_free_current_dir_struct");
	this->getLastErrorMessage =
		(PsftpGetLastErrorMessageProcType) GetProcAddress(hDll, "__map__wcplg_get_last_error_msg");
	this->initProcs =
		(PsftpInitProcsProcType) GetProcAddress(hDll, "__map__init_Procs");
	this->disconnected =
		(PsftpDisconnectedProcType) GetProcAddress(hDll, "__map__disconnected");
	// new
	this->doConfig = (PsftpDoConfigType)GetProcAddress(hDll, "__map__do_config");
	this->getLastAttr = (PsftpGetLastAttrType)GetProcAddress(hDll, "__map__get_last_attr");
	this->setTransferAscii = (PsftpSetTransferAscii)GetProcAddress(hDll, "__map__set_transfer_ascii");
	this->setConfig = (PsftpSetConfig)GetProcAddress(hDll, "__map__set_config");
	this->loadConfig = (PsftpLoadConfig)GetProcAddress(hDll, "__map__load_config");

	if (this->connect == NULL
		|| this->disconnect == NULL
		|| this->doSftp == NULL
		|| this->getCurrentDirStruct == NULL
		|| this->getLastErrorMessage == NULL
		|| this->initProcs == NULL
		|| this->disconnected == NULL
		|| this->doConfig == NULL
		|| this->getLastAttr == NULL
		|| this->setTransferAscii == NULL
		|| this->setConfig == NULL
		) {
			cleanup();
	}

	/* 
	DLL-loading and functionsimport successed,
	now try to connect to server
	*/

	this->initProcs(gRequestProc, myProgressProc, gPluginNumber);
	if (cfg) this->setConfig(cfg);
}

