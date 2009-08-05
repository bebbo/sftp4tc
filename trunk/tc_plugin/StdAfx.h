// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__ABEAC199_F5FA_40D0_B9C8_3EF9875720F1__INCLUDED_)
#define AFX_STDAFX_H__ABEAC199_F5FA_40D0_B9C8_3EF9875720F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif                          // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdlib.h>
#include <shellapi.h>
#include <stdio.h>
#include <string>

// TODO: reference additional headers your program requires here


#ifdef _DEBUG
#ifndef _WIN32_WCE
  char buff[255];
  va_list args;
  va_start(args, fmt);

  _vsnprintf( buff, sizeof(buff), fmt, args);
  OutputDebugString(buff);
}
#define DBGPRINT(a) __d a
#endif
#else
#define DBGPRINT(a)
#endif


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif                          // !defined(AFX_STDAFX_H__ABEAC199_F5FA_40D0_B9C8_3EF9875720F1__INCLUDED_)
