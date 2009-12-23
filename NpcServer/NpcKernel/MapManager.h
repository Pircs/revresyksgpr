

#pragma once

#include "define.h"
#include "GameMap.h"
#include "GameObjSet.h"


typedef	IGameObjSet<CGameMap>	IGameMapSet;
typedef	CGameObjSet<CGameMap>	CGameMapSet;

class CMapManager
{
public:
	CMapManager();
	virtual ~CMapManager();
	ULONG		Release() { delete this; return 0; }

	bool		Create();
	CGameMap*	QueryMap(OBJID idMap);
	IGameMapSet*	QuerySet() { CHECKF(m_pMapSet); return m_pMapSet; }

public:
	void OnTimer(DWORD nCurr);

protected:
	IGameMapSet*		m_pMapSet;
};


