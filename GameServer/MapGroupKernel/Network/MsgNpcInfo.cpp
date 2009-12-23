#include "allmsg.h"
#include "Npc.h"
#include "NpcManager.h"
#include "GameMap.h"
#include "MapGroup.h"


bool CMsgNpcInfo::Create(CNpc* pNpc)
{
	m_id			= pNpc->GetID();
	m_usType		= pNpc->GetType();
	m_usSort		= pNpc->GetSort();
	m_usLook		= pNpc->GetLookFace();
	m_usCellX	= pNpc->GetPosX();
	m_usCellY	= pNpc->GetPosY();
	m_cLength	= pNpc->GetLength();
	m_cFat		= pNpc->GetFat();

	// with name
	if(0)
	{
		m_StrPacker.AddString(pNpc->GetName());
	}
	UPDATE_MSG_SIZE
	return true;
}

bool CMsgNpcInfo::Create(OBJID id, int nType, int nSort, int nLookFace, int nCellX, int nCellY, int nLength, int nFat, const char* pszName/*=NULL*/)
{
	m_id			= id;
	m_usType		= (unsigned short)nType;
	m_usSort		= (unsigned short)nSort;
	m_usLook		= (unsigned short)nLookFace;
	m_usCellX	= (unsigned short)nCellX;
	m_usCellY	= (unsigned short)nCellY;
	m_cLength	= (char)nLength;
	m_cFat		= (char)nFat;

	// with name
	if(pszName)
	{
		m_StrPacker.AddString(pszName);
	}
	UPDATE_MSG_SIZE
	return true;
}

void CMsgNpcInfo::Process(CMapGroup& mapGroup, CUser* pUser)
{
#ifdef _MYDEBUG
	::LogSave("Process CMsgNpcInfo: ID:0x:%x, Type:%d, LookType:%d, CellX:%d, CellY:%d, Name:%s",
				m_id	, m_ucType,
				m_usLook, m_usCellX, 
				m_usCellY, m_szName);

#endif
	// create new npc
	//CUser* pUser = UserManager()->GetUser(this);
	if(!pUser)
		return;

	MSGBUF	szMsg;
	sprintf(szMsg, "%d %d %d %d %d", 
			m_usCellX, m_usCellY, 
			m_usLook, m_usType, m_usSort);		// 转换成串，提供给任务系统
			// dir,			 frame,			, pose

	OBJID	idTaskItem = pUser->GetTaskItemID();
	if(idTaskItem == ID_NONE)
		return ;
	CItem* pItem = pUser->GetItem(idTaskItem);
	if(!pItem)
		return ;

	OBJID idAction = pUser->GetTaskItemActionID();
	if(idAction == ID_NONE)
		return ;

	mapGroup.GetGameAction()->ProcessAction(idAction, pUser, NULL, pItem, szMsg);
}

