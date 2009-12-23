#include "allmsg.h"
#include "Npc.h"
#include "NpcWorld.h"

bool CMsgAiNpcInfo::Create(int nAction, OBJID idNpc, OBJID idGen, int nType, OBJID idMap, int nPosX, int nPosY)
{
	CHECKF(nAction == MSGAINPCINFO_CREATENEW);

	m_id			= idNpc;
	m_usAction		= nAction;
	m_usType		= nType;
	m_idMap			= idMap;
	m_usPosX		= nPosX;
	m_usPosY		= nPosY;
	m_idGen			= idGen;
	m_idOwner		= ID_NONE;
	m_ucOwnerType	= OWNER_NONE;
	m_nData			= 0;

	return true;
}

void CMsgAiNpcInfo::Process()
{
#ifdef _MYDEBUG
	::LogSave("Process CMsgAiNpcInfo: ID:0x:%x, Type:%d, LookType:%d, CellX:%d, CellY:%d, Name:%s",
				m_id	, m_usType,
				m_usLook, m_usCellX, 
				m_usCellY, m_szName);
#endif

	//if (!pSocket)
	//	return;

	if(NpcManager()->QuerySet()->GetObj(m_id))
	{
		return;		//@ 已经有此NPC，如何处理？
	}

	CNpc*	pNpc = CNpc::CreateNew();
	if(pNpc)
	{
		if(pNpc->Create(m_id, m_usType, m_idMap, m_usPosX, m_usPosY, m_idGen, 
						m_idOwner, m_ucOwnerType, m_nData))
		{
			NpcManager()->QuerySet()->AddObj(Cast<INpc>(pNpc));
			return ;
		}
		pNpc->ReleaseByOwner();
	}
}








