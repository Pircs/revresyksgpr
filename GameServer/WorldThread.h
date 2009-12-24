#pragma once

#include "ThreadBase.h"
//#include "WorldKernel.h"

class IMessagePort;
class CWorldThread : public CThreadBase  
{
public:
	CWorldThread(IMessagePort* pPort);
	virtual ~CWorldThread();

public: // overload
	virtual	bool	CreateThread(bool bRun = true);		// false: �ݲ����У��� ResumeThread() ����

/////////////////////////////////////////////////////////////////////
protected:	// ������
	//overrideable
	virtual	void	OnInit();
	virtual bool	OnProcess();		// ����Ҫ����DWORD
	virtual void	OnDestroy();

protected: // ���Ķ��󼰽ӿ�
	clock_t m_tNextClock;
	IWorld*			m_pWorld;
	IMessagePort*	m_pMsgPort;

	HANDLE	m_hMutexThread;
};


