
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PUTTYLIB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PUTTYLIB_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef PUTTYLIB_EXPORTS
#define PUTTYLIB_API __declspec(dllexport)
#else
#define PUTTYLIB_API __declspec(dllimport)
#endif

/*

// This class is exported from the PuttyLib.dll
class PUTTYLIB_API CPuttyLib {
public:
	CPuttyLib(void);
	// TODO: add your methods here.
};

extern PUTTYLIB_API int nPuttyLib;

PUTTYLIB_API int fnPuttyLib(void);

*/