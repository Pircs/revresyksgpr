#pragma once

//-------------------------------------
#include "MapObj.h"
//-------------------------------------
class CTerrainObj : public CMapObj
{
public: // construct
	CTerrainObj() { SetObjType(TERRAIN_TREE); }
	virtual ~CTerrainObj();
	static CTerrainObj*		CreateNew(LPCTSTR szFileName, OBJID idOwner = ID_NONE);
    void SetPos(POINT posCell);

private:
	OBJID			m_idOwner;
	char            m_szFileName[_MAX_PATH];

//--------------------------------------------
// additional 
public:
	OBJID	GetOwnerID() { return m_idOwner; }
protected:
	void Destory();
};

#include <vector>
using namespace std;
typedef vector<CTerrainObj* >  DEQUE_TERRAINOBJ;