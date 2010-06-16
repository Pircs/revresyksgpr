#pragma once
#include "windows.h"
#include "SharedBaseFunc.h"

//----------------------------------------------------
class CMapObj
{
public:
	CMapObj();
	virtual ~CMapObj();
protected:
	POINT	m_posCell;
	int		m_nObjType;

//------------------------------------------------------------
// for mapobj
public:
	virtual POINT GetPos()				{ return m_posCell; }
	virtual void SetPos(POINT posCell)	{ m_posCell = posCell; }

	enum { TERRAIN_NONE=0, TERRAIN_TREE, TERRAIN_LEAF, };
	virtual int GetObjType   ()	{ return m_nObjType; }
protected:
	void SetObjType	(int nType) { m_nObjType = nType; }
};
#include <vector>
using namespace std;
typedef vector<CMapObj* >  DEQUE_MAPOBJ;