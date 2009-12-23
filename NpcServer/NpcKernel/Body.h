

#pragma once

#include "mycom.h"
#include "VarType.h"

class CAgent;
class CBody  
{
	COM_STYLE(CBody)
protected:
	CBody();
	virtual ~CBody();

public:
	bool ProcessAction(int idxAction, VarTypeSetRef setParam);
	bool Create(CAgent* pOwner);
	void OnTimer();

protected:
	CAgent*		m_pOwner;
};


