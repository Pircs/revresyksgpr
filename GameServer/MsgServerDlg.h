// MsgServerDlg.h : header file
//

#pragma once


#pragma warning(disable:4786)
#include "typedef.h"
#include "InternetPort.h"
#include "TimeOut.h"
#include "I_Shell.h"
#include <vector>
using namespace std;
///////
// CMsgServerDlg dialog
enum {
		SHELLSTATE_NONE,
		SHELLSTATE_INIT, 
		SHELLSTATE_RUNNING, 
		SHELLSTATE_CLOSING, 
		SHELLSTATE_END,
};
///////
class	IMessagePort;
class	CMapGroupThread;
class	CSocketThread;
class	CWorldThread;
class CMsgServerDlg : public CDialog
{
// Construction
public:
	bool LoadConfigIni();
	CString	m_sText;

// Implementation
protected:
	NAMESTR		m_szServer;
	char m_szStartServer[20];
	long m_nLogoutPlayers;
	long m_nLoginPlayers;
	CTimeOut m_tStat5Min;
	CTimeOut m_tStat;
	typedef	vector<CMapGroupThread*>	MAPGROUP_SET;
	MAPGROUP_SET m_setMapGroupThread;

	IMessagePort* m_pMsgPort;
	IMessagePort* m_pInterPort;
	int m_nTextLines;
	int m_nState;

	STAT_STRUCT m_stat;
	HANDLE	m_hMutexThread;
	HANDLE	m_hMutexServer;
	virtual BOOL OnInitDialog();

	virtual void OnCancel();
	afx_msg void OnSend();
public:

};
