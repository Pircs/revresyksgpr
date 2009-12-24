#pragma once

#include "ThreadBase.h"

//#include "typedef.h"
#include "InternetPort.h"
#include "TimeOut.h"
#include "I_Shell.h"
#include <vector>

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
class CTimerThread : public CThreadBase  
{
public:
	CTimerThread();
	virtual ~CTimerThread();
public:	// 共享
	// 无共享函数调用，故共享成员变量无共享冲突
protected:
	virtual	void	OnInit();
	virtual bool	OnProcess();
	virtual void	OnDestroy();
protected:	// 共享
	bool ProcessMsg(OBJID idPacket, void* pMsg, int nType, int nSource);
	bool ProcessInterMsg();
	bool Process();
private:
	std::string	m_sKernelState;
	std::string	m_sShellState;
	std::string m_sText;

	CSocketThread* m_pSocketThread;
	CWorldThread* m_pWorldThread;

	typedef	vector<CMapGroupThread*>	MAPGROUP_SET;
	MAPGROUP_SET m_setMapGroupThread;

	CTimeOut m_tStat5Min;
	CTimeOut m_tStat;

	int m_nAllPlayers;
	int m_nMaxPlayers;

	long m_nLogoutPlayers;
	long m_nLoginPlayers;

protected:
	NAMESTR		m_szServer;
	char m_szStartServer[20];

	IMessagePort* m_pMsgPort;
	IMessagePort* m_pInterPort;
	int m_nTextLines;
	int m_nState;

	STAT_STRUCT m_stat;
	HANDLE	m_hMutexThread;
	HANDLE	m_hMutexServer;

	bool LoadConfigIni();
	BOOL OnInitDialog();
	void Send(const std::string& strCmd);
	void PrintText(const std::string& szMsg);
};