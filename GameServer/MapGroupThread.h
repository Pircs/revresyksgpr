#pragma once

#include "ThreadBase.h"
//#include "MapGroupKernel.h"

class IMessagePort;
class CMapGroupThread : public CThreadBase  
{
public:
	CMapGroupThread(IMessagePort* pPort);
	virtual ~CMapGroupThread();

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
//	CMapGroupKernel	m_cMapGroupKernel;
	IMapGroup*		m_pMapGroup;
	IMessagePort*	m_pMsgPort;

	HANDLE	m_hMutexThread;
};


