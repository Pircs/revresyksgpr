

#pragma once

#include "mycom.h"
#include "AutoLink.h"
#include "I_Role.h"
#include "TimeOut.h"

const int	ATTACK_DELAY_MS					= 1500;			// 开始攻击到收到攻击命令的最大延时。(BUG，未考虑震晕)@@@

class CAgent;
class CRoleFight  
{
	COM_STYLE(CRoleFight)
protected:
	CRoleFight();
	virtual ~CRoleFight();

public:
	bool		Create(CAgent* pOwner);
	void		OnTimer();

public: // const
	IRole*		QueryTarget()							{ return m_pTarget; }

public: // application
	bool Attack(IRole* pRole);
	void		AttackOnce()							{ m_tAttackDelay.Update(); }
protected:
	CTimeOutMS*	QueryTime()								{ return &m_tFight; }

protected:
	CAgent*			m_pOwner;

protected: // attr
	OBJID			m_idAttackMe;
	CAutoLink<IRole>		m_pTarget;			// may be null
	CTimeOutMS		m_tAttackDelay;
	CTimeOutMS		m_tFight;
};


