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
	virtual	bool	CreateThread(bool bRun = true);		// false: 暂不运行，用 ResumeThread() 运行

/////////////////////////////////////////////////////////////////////
protected:	// 派生用
	//overrideable
	virtual	void	OnInit();
	virtual bool	OnProcess();		// 不需要返回DWORD
	virtual void	OnDestroy();

protected: // 核心对象及接口
	clock_t m_tNextClock;
//	CMapGroupKernel	m_cMapGroupKernel;
	IMapGroup*		m_pMapGroup;
	IMessagePort*	m_pMsgPort;

	HANDLE	m_hMutexThread;
};


