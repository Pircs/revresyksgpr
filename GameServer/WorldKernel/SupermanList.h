#pragma once
#include "define.h"
#include "windows.h"
#include <vector>
using namespace std;


struct	SUPERMAN_INFO
{
	OBJID	id;
	DWORD	dwNumber;
	NAMESTR	szName;
};

class IDatabase;
class CSupermanList  
{
protected:
	CSupermanList();
	virtual ~CSupermanList();

public:
	static CSupermanList* CreateNew()		{ return new CSupermanList; }
	void	Release()						{ delete this; }
	bool	Create(IDatabase* pDb);

public: // application
	bool	AddMember(OBJID idUser, int nNumber);
	void	QueryMember(OBJID idUser);
	void	SynchroInfo(OBJID idUser, int nRank);
	void	SendList(OBJID idUser, int idx, int nNumber, OBJID idAction);
protected:
	void	InsertInfo(OBJID idUser, int nNumber, LPCTSTR szName);
	void	UpdateInfo(OBJID idUser, int nNumber);
	void	DeleteInfo(OBJID idUser);

protected:
	typedef	vector<SUPERMAN_INFO>	SUPERMAN_SET;
	SUPERMAN_SET	m_setUser;

protected: // ctrl
	MYHEAP_DECLARATION(s_heap)
};


