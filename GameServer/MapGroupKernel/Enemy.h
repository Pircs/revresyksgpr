
#pragma once

#include "mycom.h"
#include "array.h"
#include "vartype.h"

const int	MAX_ENEMYSIZE			= 10;		//??? info size is 10*(16+4)=200, close to 256 limit

struct CEnemyInfoStruct
{
	OBJID	setEnemy[MAX_ENEMYSIZE];
	char	setName[MAX_ENEMYSIZE][_MAX_NAMESIZE];
};
class CUser;
class IDatabase;
class CEnemy  
{
	COM_STYLE(CEnemy)
protected:
	CEnemy();
	virtual ~CEnemy();

public:
	bool	Create(PROCESS_ID idProcess, CUser* pOwner, IDatabase* pDb);
	bool	Create(PROCESS_ID idProcess, CUser *pOwner);	// for chgmap

public:
	void	Add(OBJID idEnemy, LPCTSTR szName, bool bSynchro, bool bUpdate);
	void	Del(OBJID idEnemy, bool bUpdate);
	LPCTSTR	GetName(OBJID idEnemy);
	void	SendToClient();
	void	BeforeLogout();
public: // persistent
	bool	GetInfo(CEnemyInfoStruct* pInfo);
	bool	AppendInfo(const CEnemyInfoStruct* pInfo);

public:
	typedef	Array<pair<OBJID, std::string> >	ENEMY_SET;
protected:
	ENEMY_SET	m_set;
	CUser*		m_pOwner;
	PROCESS_ID	m_idProcess;

protected: // ctrl
	MYHEAP_DECLARATION(s_heap)
};


