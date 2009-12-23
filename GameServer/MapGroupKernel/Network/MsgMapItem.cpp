#include "AllMsg.h"
#include "MapGroup.h"
#include "MapItem.h"
#include "MapTrap.h"

bool CMsgMapItem::Create(int nAction, CMapItem* pMapItem)
{
	CHECKF(pMapItem && nAction);

	// fill info now
	m_nAction	= nAction;
	m_id			= pMapItem->GetID();
	m_idType		= pMapItem->GetType();
	m_nPosX		= pMapItem->GetPosX();
	m_nPosY		= pMapItem->GetPosY();
	ItemInfoStruct* pInfo	= pMapItem->GetInfo();
	if (strlen(pInfo->szName)>0)
		m_StrPacker.AddString(pInfo->szName);

	// hide id
	m_idType		= CItem::HideTypeQuality(m_idType);
	UPDATE_MSG_SIZE;
	return true;
}

bool CMsgMapItem::Create(int nAction, CMapTrap* pTrap)
{
	CHECKF(pTrap && nAction);
	// fill info now
	m_nAction	= nAction;
	m_id			= pTrap->GetID();
	m_usLook		= pTrap->GetLookFace();
	m_nPosX		= pTrap->GetPosX();
	m_nPosY		= pTrap->GetPosY();
	UPDATE_MSG_SIZE;
	return true;
}

/*bool CMsgMapItem::Create(int nAction, OBJID idUser)
{
	m_nAction	= nAction;
	m_id			= idUser;
	UPDATE_MSG_SIZE;
	return true;
}*/

bool CMsgMapItem::Create(OBJID id, int nPosX, int nPosY, int nAction)
{
	if (id == ID_NONE)
		return false;

	m_nAction	= nAction;
	m_id			= id;
	m_nPosX		= nPosX;
	m_nPosY		= nPosY;

	UPDATE_MSG_SIZE;
	return true;
}

void CMsgMapItem::Process(CMapGroup& mapGroup, CUser* pUser, SOCKET_ID idSocket)
{
	if(!pUser)
		return;

	if(!pUser->IsAlive())
	{
		pUser->SendSysMsg(STR_DIE);
		return ;
	}

	switch(m_nAction)
	{
	case MSGMAPITEM_PICK:
		{
			if (pUser->SynPosition(m_nPosX, m_nPosY, 0))
			{
				pUser->PickMapItem(m_id);
			}
			else
			{
				mapGroup.GetUserManager()->KickOutSocket(idSocket, "ITEM_PICK SynPosition");
			}
		}
		break;
	default:
		{
			ASSERT(!"switch");
		}
	}
	return;

#ifdef _MSGDEBUG
	::LogMsg("Process CMsgMapItem, id:%u", m_id);
#endif
}