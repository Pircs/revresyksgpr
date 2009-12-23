#pragma once

//#pragma warning(disable:4786)
#include "define.h"
#include "windows.h"
#include <time.h>
//#include <vector>

struct CMapSt
{
	OBJID		idMap;
	OBJID		idxMapGroup;
	int			nPortalX;
	int			nPortalY;

	CMapSt() { idMap = ID_NONE; }
	OBJID	GetID() { return idMap; }
};

class IDatabase;
class CMapList  
{
public:
	CMapList();
	virtual ~CMapList();
	static CMapList* CreateNew()		{ return new CMapList; }
	bool	Create(IDatabase* pDb);
	ULONG	Release()			{ delete this; return 0; }

public:
	PROCESS_ID	GetMapProcessID(OBJID idMap);
	CMapSt*		GetMap(OBJID idMap);

protected:
	typedef std::vector<CMapSt*>	MAP_SET;
	MAP_SET	m_setMap;
};