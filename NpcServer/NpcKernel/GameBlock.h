#pragma once

#include "UserManager.h"
#include "NpcManager.h"

class CGameBlock  
{
public:
	CGameBlock();
	virtual ~CGameBlock();
	static CGameBlock*	CreateNew() { return new CGameBlock; }
	bool	Create();
	virtual ULONG	Release() { delete this; return 0; }

public: // interface
	IUserSet*	QueryUserSet()		{ return m_pUserSet; }
	INpcSet*	QueryNpcSet()		{ return m_pNpcSet; }

public: // dormancy
	bool		IsDormancy()		{ return m_bDormancy; }
	void		SetDormancy(bool b)	{ m_bDormancy = b; }

protected:
	IUserSet*		m_pUserSet;
	INpcSet*		m_pNpcSet;

protected:
	bool			m_bDormancy;
};