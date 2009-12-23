
#pragma once

#include "GameObjSet.h"
#include "ItemType.h"
#include "Myheap.h"

typedef	IGameObjSet<CItemTypeData>	IGoodsSet;
typedef	CGameObjSet<CItemTypeData>	CGoodsSet;
class CNpc;
class CUser;
class CNpcShop  
{
protected:
	CNpcShop();
	virtual ~CNpcShop();
public:
	static CNpcShop* CreateNew()						{ return new CNpcShop; }
	ULONG	Release()									{ delete this; return 0; }

public:
	bool	Create(PROCESS_ID idProcess, CNpc* pOwner, OBJID idNpc);
	CItemTypeData*	QueryItemType(OBJID idType)			{ return m_setGoods->GetObj(idType); }
	bool	IsOpen();
	int		Rebate(int nPrice, OBJID idSyn, int nRankShow);

protected: // data
	IGoodsSet*		m_setGoods;

private: // ctrl
	CNpc*			m_pOwner;
	PROCESS_ID		m_idProcess;

	MYHEAP_DECLARATION(s_heap)
};


