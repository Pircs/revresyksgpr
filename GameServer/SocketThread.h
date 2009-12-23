

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
	virtual	bool	CreateThread(bool bRun = true);		// false: 暂不运行，用 ResumeThread() 运行

/////////////////////////////////////////////////////////////////////
protected:	// 派生用
	//overrideable
	virtual	void	OnInit();
	virtual bool	OnProcess();		// 不需要返回DWORD
	virtual void	OnDestroy();

protected: // 核心对象及接口
	CTimeOutMS		m_tProcessSocket;
	clock_t m_tNextClock;
	ISocketKernel*	m_pServerSocket;
	IMessagePort*	m_pMsgPort;

protected: // ctrl
	HANDLE	m_hMutexThread;
};


