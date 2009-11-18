#ifndef _PROPERTIES_DLG_H
#define _PROPERTIES_DLG_H

#include "windows.h"
#include "sftp4tc_share.h"

#define DIALOG_DLL "wfx_sftp_cfg.dll"
#define PROPERTIES_FUNCTION "Properties"
#define INITIALIZE_FUNCTION "InitializeCfgDLL"
#define FREE_FUNCTION "FreeCfgDLL"

typedef bool (__stdcall * tProperties) (int Mode, ConfigPropertiesType *aProperties);
typedef void (__stdcall * tInitialize) (HMODULE hModule);
typedef void (__stdcall * tFreeCfgDLL) ();

#endif //_PROPERTIES_DLG_H
