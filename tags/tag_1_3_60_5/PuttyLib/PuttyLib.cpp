// PuttyLib.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "PuttyLib.h"
#include "windows.h"
#include "fsplugin.h"


#pragma warning( disable : 4786 )
#include <string>
#include <vector>

extern "C" {
#include <putty.h>
#include "sftp4tc_share.h"
#include "pl_misc.h"
#include "pl_psftp.h"
#include "resource.h"
#include "puttyver.h"

extern HINSTANCE hinst;

	int fxp_disconnected();

	BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
	{
		switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
	  hinst = (HINSTANCE) hModule;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
	  break;
		}
		return TRUE;
	}


	config_tag * __stdcall __map__wcplg_open_sftp_session(char *user, char *password,
		char *host, int port)
	{
		ISinitT = 0;
		return wcplg_open_sftp_session(host, user, password, port);
	}

	int __stdcall __map__wcplg_close_sftp_session()
	{
		return wcplg_close_sftp_session();
	}

	int __stdcall __map__wcplg_do_sftp(char *_cmd, char *_server_output)
	{
		return wcplg_do_sftp(_cmd, _server_output);
	}

	struct my_fxp_names * __stdcall __map__wcplg_get_current_dir_struct()
	{
		return wcplg_get_current_dir_struct();
	}

	void __stdcall __map__wcplg_free_current_dir_struct()
	{
		wcplg_free_current_dir_struct();
	}

	char * __stdcall __map__wcplg_get_last_error_msg()
	{
		return wcplg_get_last_error_msg();
	}

	int __stdcall __map__disconnected()
	{
		return fxp_disconnected();
	}

	int __stdcall __map__init_Procs(tRequestProcType AP_RequestProc, tProgressProc AP_ProgressProc,
		int Awc_PluginNr, HWND hwnd)
	{
		return init_Procs(AP_RequestProc, AP_ProgressProc, Awc_PluginNr, hwnd);
	}

	struct config_tag * __stdcall __map__do_config(HWND hwnd, int midsession, int protocol) {
		return do_config(hwnd, midsession, protocol);
	}

	extern struct fxp_attrs attrs;
	struct fxp_attrs * __stdcall __map__get_last_attr()
	{
		return &attrs;
	}

	extern int transferAscii;
	void __stdcall __map__set_transfer_ascii(int ca) {
		transferAscii = ca;
	}

	extern Config cfg;
	void __stdcall __map__set_config(Config * oldCfg) {
		if (oldCfg)
			cfg = *oldCfg;
	}

	void __stdcall __map__load_config(char * name, Config * cfg) {
		load_settings(name, cfg);
	}

	extern void *enum_settings_start(void);
	void * __stdcall __map__enum_settings_start(void) {
		return enum_settings_start();
	}

	extern char *enum_settings_next(void *handle, char *buffer, int buflen);
	bchar * __stdcall __map__enum_settings_next(void * handle, bchar *buffer, int buflen) {
		return enum_settings_next(handle, buffer, buflen);
	}

	extern void enum_settings_finish(void *handle);
	void __stdcall __map__enum_settings_close(void * handle) {
		enum_settings_finish(handle);
	}

	char * __stdcall __map__get_version() {
		return PUTTY_VERSION_STRING;
	}
}

