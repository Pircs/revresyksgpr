#pragma once

#include "ThreadBase.h"
#include "Msg.h"

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

	CSocketThread* m_pSocketThread;
	CWorldThread* m_pWorldThread;


	int m_nAllPlayers;
	int m_nMaxPlayers;
};