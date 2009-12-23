
#pragma once

#include "Item.h"
#include "Myheap.h"
#include "Package.h"

class CPackage;
class CNpcTrunk  
{
protected:
	CNpcTrunk();
	virtual ~CNpcTrunk();
public:
	static CNpcTrunk* CreateNew()			{ return new CNpcTrunk; }
	ULONG	Release()						{ delete this; return 0; }

public:
	bool	Create(PROCESS_ID, OBJID idRecordNpc, int nSize, int nPosition = ITEMPOSITION_TRUNK);
	CPackage*	QueryPackage(OBJID idPlayer=ID_NONE);	//				{ ASSERT(m_pPackage); return m_pPackage; }
	bool	IsPackageFull(OBJID idPlayer=ID_NONE)				{ return QueryPackage(idPlayer)->GetAmount() >= m_nSize; }
	bool	IsEmpty(OBJID idPlayer=ID_NONE)						{ return QueryPackage(idPlayer)->GetAmount() == 0; }

protected:
	CPackage*	m_pPackage;
	int			m_nSize;
	int			m_nPosition;
	OBJID		m_idRecordNpc;

private: // ctrl
	PROCESS_ID	m_idProcess;

	MYHEAP_DECLARATION(s_heap)
};


