
#pragma once


class CNpcScene  
{
protected:
	CNpcScene();
	virtual ~CNpcScene();

public:
	static CNpcScene* CreateNew()						{ return new CNpcScene; }
	ULONG	Release()									{ delete this; return 0; }

public:
	bool	Create(CNpc* pOwner, CGameMap* pMap);
	void	DelTerrainObj();

protected:
	CNpc*		m_pOwner;
	CGameMap*	m_pMap;
	auto_long	m_bTerrainObj;

	MYHEAP_DECLARATION(s_heap)
};


