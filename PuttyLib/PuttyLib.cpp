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
extern Sftp4tc cfg;

int fxp_disconnected();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
		LPVOID lpReserved) {
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

Sftp4tc * __stdcall __map__wcplg_open_sftp_session(char *user, char *password,
		char *host, int port) {
	Sftp4tc * c;
	ISinitT = 0;
	c = wcplg_open_sftp_session(host, 0, 0, port);

	return c;
}

int __stdcall __map__wcplg_close_sftp_session() {
	return wcplg_close_sftp_session();
}

int __stdcall __map__wcplg_do_sftp(wchar_t *_cmd, wchar_t *_server_output) {

	// handle unicode conversion
	{
		char buff[1024] = { 0 };
		char out[1024] = { 0 };
		wchar_t * wcmd = (wchar_t*) _cmd;
		wchar_t * wout = (wchar_t*) _server_output;
		int cp = cfg.codePage;
		if (!WideCharToMultiByte(cp, 0, wcmd, -1, buff, 1024, 0, 0))
			cp = CP_ACP;
		if (WideCharToMultiByte(cp, 0, wcmd, -1, buff, 1024, 0, 0)) {
			int ret = wcplg_do_sftp(buff, out);
			MultiByteToWideChar(cp, 0, out, -1, wout, 1024);
			return ret;
		}
	}
	return 0;
}

struct my_fxp_names * __stdcall __map__wcplg_get_current_dir_struct() {
	return wcplg_get_current_dir_struct();
}

void __stdcall __map__wcplg_free_current_dir_struct() {
	wcplg_free_current_dir_struct();
}

char * __stdcall __map__wcplg_get_last_error_msg() {
	return wcplg_get_last_error_msg();
}

int __stdcall __map__disconnected() {
	return fxp_disconnected();
}

int __stdcall __map__init_Procs(tRequestProcW AP_RequestProc,
								tProgressProcW AP_ProgressProc, tCryptProcW cryptProc,
								int PluginNr, int CryptoNr, HWND hwnd, wchar_t * sessionName) {
	return init_Procs(AP_RequestProc, AP_ProgressProc, cryptProc, PluginNr, CryptoNr, hwnd, sessionName);
}

struct Sftp4tc * __stdcall __map__do_config(HWND hwnd, int midsession,
		int protocol) {
	Sftp4tc * c = do_config(hwnd, midsession, protocol);
	return c;
}

extern struct fxp_attrs attrs;
struct fxp_attrs * __stdcall __map__get_last_attr() {
	return &attrs;
}

extern int transferAscii;
void __stdcall __map__set_transfer_ascii(int ca) {
	transferAscii = ca;
}

void __stdcall __map__set_config(Sftp4tc * oldCfg) {
	if (oldCfg)
		cfg = *oldCfg;
}

void __stdcall __map__load_config(char * name, Sftp4tc * cfg) {
	load_settings(name, cfg);
}

extern void *enum_settings_start(void);
void * __stdcall __map__enum_settings_start(void) {
	return enum_settings_start();
}

extern char *enum_settings_next(void *handle, char *buffer, int buflen);
bchar * __stdcall __map__enum_settings_next(void * handle, bchar *buffer,
		int buflen) {
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

