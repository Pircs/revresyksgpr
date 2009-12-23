
#pragma once


#pragma warning(disable:4786)

#include "define.h"
/*
#include "ConstGameData.h"
#include "GameData.h"
#include "GameObj.h"
#include "T_SingleMap2.h"
*/
#include "DropRuleGroup.h"
#include "Myheap.h"
#include "I_mydb.h"
#include <map>
using namespace std;






class IDatabase;
class CDropRuleMap  
{
public:
	CDropRuleMap();
	virtual ~CDropRuleMap();

public:
	static CDropRuleMap*	CreateNew()	{ return new CDropRuleMap; }
	ULONG	Release()	{ delete this; return 0; }

	bool	Create(IDatabase*	pDb);
	void	Destroy();

	CDropRuleGroup*	GetObjByIndex(OBJID	idRuleGroup)	{ if (m_map[idRuleGroup]) return m_map[idRuleGroup]; return NULL; }

protected:
	typedef map<OBJID, CDropRuleGroup*>				MAP_RULE;
	typedef map<OBJID, CDropRuleGroup*>::iterator	Iter;

	MAP_RULE	m_map;

	MYHEAP_DECLARATION(s_heap)
};


