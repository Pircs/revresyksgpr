

#pragma once

#include "define.h"
#include "User.h"
#include "GameObjSet.h"
#include <map>
using namespace std;

typedef	CGameObjSet<CUser>	IUserSet;
typedef CGameObjSet<CUser>	CUserSet;

class CUserManager
{
public:
	CUserManager();
	virtual ~CUserManager();
	bool	Create();
	virtual ULONG Release()				{ delete this; return 0; }
	virtual IUserSet* QuerySet()		{ CHECKF(m_pUserSet); return m_pUserSet; }
	CUser*	QueryUser(OBJID idUser)		{ return m_pUserSet->GetObj(idUser); }
	CUser*	QueryUser(LPCTSTR szUser);
	IRole*	QueryRole(OBJID idRole);

public:
	void OnTimer(DWORD nCurr);

public:
	static int	GetSizeAdd(int nLook);

protected:
	IUserSet*		m_pUserSet;

	typedef	map<int, int>	SIZEADD_SET;
	static SIZEADD_SET		m_setSizeAdd;
};


