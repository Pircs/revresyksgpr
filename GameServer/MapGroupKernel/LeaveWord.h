// �ɽ���, 2002.10.11
#pragma once

#include <windows.h>
#include "define.h"
#include "TimeOut.h"
#include "Myheap.h"

//////////////////////
enum LEAVEWORDDATA {
	LEAVEWORDDATA_ID=0,		// 
	LEAVEWORDDATA_USER=1,
	LEAVEWORDDATA_SENDER,
	LEAVEWORDDATA_TIME,
	LEAVEWORDDATA_WORDS,
};

//////////////////////
const int	PLAYER_WORDS_PER_TIME	= 1;				// ���ÿ�ζ�ȡ5������
const int	NEW_WORDS_PER_MINUTE	= 5;				// ÿ�����Զ���ȡ����������
const int	DELETE_LEAVEWORD_DAYS	= 31;				// 31���������Ի��Զ�ɾ��
const int	SECS_PER_AUTOFLUSH		= 60;				// ÿ�����Զ�ˢ��һ��

//////////////////////
class	CUser;
class CLeaveWord  
{
public:
	CLeaveWord();
	virtual ~CLeaveWord();

public:
	bool Create		(PROCESS_ID idProcess);
	ULONG	Release			()							{ delete this; return 0; }
	bool	InsertWords		(LPCTSTR szSender, LPCTSTR szRecvUser, LPCTSTR szWords);
//	int		CountWords		(LPCTSTR szRecvUser);
	bool	ShowWords		(CUser* pUser, int nMaxNum = PLAYER_WORDS_PER_TIME);
	void	OnTimer			(DWORD tCurr);

protected:
	DWORD			m_idLastMsg;
	CTimeOut		m_tAutoFlush;

protected: // ctrl
	PROCESS_ID		m_idProcess;

public: // ctrl
	MYHEAP_DECLARATION(s_heap)
};

