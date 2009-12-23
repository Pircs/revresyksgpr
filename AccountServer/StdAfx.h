// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once


#define _WINSOCKAPI_		// 阻止加载winsock2.h

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define	FD_SETSIZE			600		//??? 当客户端LOGIN连接数很大时，应增加该值
#include <winsock2.h>		// MFC socket extensions	2.0版

#define MFC_VER_4
#define INDEX           register int
#define WINDEX          register WORD
#define LINDEX          register long
#define DWINDEX         register DWORD
typedef signed  char    INT8  ;
typedef unsigned char   UINT8 ;
typedef signed  short   INT16 ;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


