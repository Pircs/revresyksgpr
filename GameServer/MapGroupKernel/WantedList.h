
#pragma once

#include "windows.h"

#include "define.h"
#include "GameObj.h"
#include "GameData.h"
#include "Myheap.h"

#include <vector>
using namespace std;

///////////////////////////////////////////////////////
enum WANTEDDATA{
	DATA_ID	= 0, 
	DATA_TARGET_NAME,
	DATA_TARGET_LEV,
	DATA_TARGET_PRO,
	DATA_TARGET_SYN,
	DATA_PAYER,
	DATA_BOUNTY,
	DATA_ORDER_TIME,
	DATA_HUNTER,
	DATA_FINISH_TIME,
};

char	szWantedTable[];
typedef	CGameData<WANTEDDATA,szWantedTable,szID>	CWantedData;
 
#define _MAX_BONUTY			9999999
#define _MIN_BONUTY			100000
#define _MIN_ADDBONUTY		10000
#define _WANTED_ORDERCOST	1000
#define _BONUTY_TAX			30

class CCriticalSection;
class IDatabase;
class IRecordset;
class CWantedList  
{
public:
	CWantedList();
	virtual ~CWantedList() {}

// interface
public:
	// operation
	static CWantedList* CreateNew(void) { return new CWantedList; }
	ULONG	Release	();

	bool Create	(IDatabase* pDb);
	bool	AddWanted	(const char* pszTarget, const char* pszPayer, DWORD dwBonuty);
	bool	DelWanted	(OBJID id);

	// info
	int		GetWantedAmount		(void)		{ return m_setWanted.size(); }
	CWantedData*	GetWanted	(OBJID id);
	CWantedData*	GetWantedByIndex	(int idx);
	CWantedData*	GetWantedByName		(const char* pszName);

	// static 
	static CWantedList	s_WantedList;
	static CCriticalSection	s_xCtrl;

// Implementation
private:
	void	InsertWanted	(CWantedData* pData);

	typedef	vector<CWantedData*>	WANTED_SET;
	WANTED_SET	m_setWanted;
	IRecordset*	m_pResDefault;

	MYHEAP_DECLARATION(s_heap)
};


