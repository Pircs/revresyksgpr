#pragma once

#include "ThreadBase.h"
#include "Msg.h"

class CTimerThread : public CThreadBase  
{
public:
	CTimerThread();
	virtual ~CTimerThread();
public:	// ����
	// �޹��������ã��ʹ����Ա�����޹����ͻ
protected:
	virtual	void	OnInit();
	virtual bool	OnProcess();
	virtual void	OnDestroy();
protected:	// ����
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