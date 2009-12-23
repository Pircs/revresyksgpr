
#pragma once


class CNpcLink  
{
protected:
	CNpcLink();
	virtual ~CNpcLink();
public:
	static CNpcLink* CreateNew(PROCESS_ID idProcess, CNpc* pOwner);
	void	Release()						{ delete this; }

public: // application
	void	SetLinkMap(OBJID idMap);
	CGameMap* GetLinkMap();				// return null: data error
	bool	EraseMap();

protected:
	PROCESS_ID	m_idProcess;
	CNpc*		m_pOwner;
	CGameMap*	m_pLinkMap;

	MYHEAP_DECLARATION(s_heap)
};


