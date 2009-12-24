#pragma once

#include "ThreadBase.h"
#include "RC5_321216.h"
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
private:
	long m_nLoginAcceptP5;
	long m_nPointFeeP5;
	std::string	m_sText;
	std::string	m_sState2;
	std::string	m_sState;
};