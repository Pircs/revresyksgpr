#pragma once

#include "MultiObjSet.h"
//#include "RoleManager.h"
#include "Myheap.h"
#include "I_Role.h"

typedef	IMultiObjSet<IMapThing>		IThingSet;
typedef	CMultiObjSet<IMapThing>		CThingSet;

class CGameBlock  
{
public:
	CGameBlock();
	virtual ~CGameBlock();
public:
	static CGameBlock*	CreateNew() { return new CGameBlock; }
	bool	Create();
	virtual ULONG	Release() { delete this; return 0; }

public: // interface
	IThingSet*	QuerySet()		{ return m_pSet; }

public: // dormancy
	bool		IsDormancy()		{ return m_bDormancy; }
	void		SetDormancy(bool b)	{ m_bDormancy = b; }

protected:
	IThingSet*		m_pSet;

protected:
	bool			m_bDormancy;

public:
//	MYHEAP_DECLARATION(s_heap)
};