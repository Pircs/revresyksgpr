#pragma once

#include "ThreadBase.h"
#include "RC5_321216.h"
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
private:
	long m_nLoginAcceptP5;
	long m_nPointFeeP5;
	std::string	m_sText;
	std::string	m_sState2;
	std::string	m_sState;
};