
#pragma once
#include "ThreadSafety.h"
#include <deque>
#include <string>

typedef struct {
	OBJID		idUser;
	string		strName;
	string		strSynName;
	int			nPk;	
	int			nLev;
}PoliceWantedStruct;

#define  PKVALUE_BADGUY		1000

class CCriticalSection;
class CUser;
class CPoliceWanted  
{
public:
	CPoliceWanted();
	virtual ~CPoliceWanted();

	// interface
	bool	AddWanted	(CUser* pUser);
	bool	DelWanted	(OBJID idUser);

	PoliceWantedStruct*	GetWantedByIndex(int idx);
	PoliceWantedStruct*	GetWanted	(OBJID idUser);
	int		GetWantedAmount	(void)	{ return m_setWanted.size(); }

	static CPoliceWanted		s_objPoliceWanted;

protected:
	deque<PoliceWantedStruct> m_setWanted;
	// static 
	static LOCK_DECLARATION;
};

extern CPoliceWanted& PoliceWanted(void);


