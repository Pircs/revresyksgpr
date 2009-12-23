

#pragma once

#include "mycom.h"
#include "VarType.h"
#include "I_AiCenter.h"
#include "TimeOut.h"

class CAgent;
class CSense  
{
	COM_STYLE(CSense)
protected:
	CSense();
	virtual ~CSense();

public: // construction
	bool	Create(CAgent* pOwner);

public: // interface
	bool	CheckCondition(int idxFactFunction, VarTypeSetRef setParam, ARGUMENT_SET* psetArgument);

public: // const

public: // application
	void OnTimer();
	void TeamApply(OBJID idUser);
	void Reborn();
	void Recruit(int nLife);
	void BeKill(IRole* pRole);
	void BeAttack(IRole* pRole);
	void Die();

protected:
	CAgent*		m_pOwner;
	CTimeOut	m_tLook;
};


