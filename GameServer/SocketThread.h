

#pragma once

#include "ThreadBase.h"
#include "TimeOut.h"

class IMessagePort;
class CSocketThread : public CThreadBase  
{
public:
	CSocketThread(IMessagePort* pPort);
	virtual ~CSocketThread();

public: // overload
	virtual	bool	CreateThread(bool bRun = true);		// false: �ݲ����У��� ResumeThread() ����

/////////////////////////////////////////////////////////////////////
protected:	// ������
	//overrideable
	virtual	void	OnInit();
	virtual bool	OnProcess();		// ����Ҫ����DWORD
	virtual void	OnDestroy();

protected: // ���Ķ��󼰽ӿ�
	CTimeOutMS		m_tProcessSocket;
	clock_t m_tNextClock;
	ISocketKernel*	m_pServerSocket;
	IMessagePort*	m_pMsgPort;

protected: // ctrl
	HANDLE	m_hMutexThread;
};


