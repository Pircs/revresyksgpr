
#pragma once

#include "MapGroup.h"
#include "Npc.h"
#include "Myheap.h"

class CNpcTask  
{
protected:
	CNpcTask();
	virtual ~CNpcTask();
public:
	static CNpcTask* CreateNew()			{ return new CNpcTask; }
	ULONG	Release()						{ delete this; return 0; }

public:
	void	OnTimer(DWORD tCurr);
	bool	Create(PROCESS_ID idProcess, CNpc* pOwner, INpcData* pData);
	bool	ActivateTask		(CUser* pUser);

protected:
	bool		m_bActiveTime;
	ITaskSet*	m_setTask;
	CNpc*		m_pOwner;

protected: // ctrl
	PROCESS_ID	m_idProcess;

	MYHEAP_DECLARATION(s_heap)
};



