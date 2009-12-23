
#pragma once

#include "User.h"
#include "mycom.h"

class CAgent : public CUser  
{
	COM_STYLE(CAgent)
protected:
	CAgent();
	virtual ~CAgent();

public:
	bool Create(PROCESS_ID idProcess, OBJID idAgent);

public: // overload
	bool FullItem(int nItemType, DWORD dwData);
	virtual bool SendMsg(Msg* pMsg);
	virtual bool IsAgent()									{ return true; }
	virtual CAgent*	QueryAgent		()						{ return this; }			// agent overload
	 bool LeaveMapGroup	();		//@@@ Í¬²½PROCESS_ID

protected: // ctrl
	MYHEAP_DECLARATION(s_heap)
};


