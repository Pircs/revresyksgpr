#include "allmsg.h"
#include "Npc.h"
#include "NpcManager.h"
#include "GameMap.h"
#include "MapGroup.h"

void CMsgNpc::Process(CMapGroup& mapGroup, CUser* pUser)
{//返回本NPC要说的话。CMsgTaskBarInfo
#ifdef _MYDEBUG
	::LogSave("Process CMsgName, id:%u, usAction:%u", 
		m_id,
		m_usAction);

#endif

	//if (!pSocket)
	//	return;

	//CUser* pUser = mapGroup.GetUserManager()->GetUser(this);
	if (!pUser)
		return ;

	if(!pUser->IsAlive())
	{
		pUser->SendSysMsg(STR_DIE);
		return ;
	}

	switch(m_usAction)
	{
	case EVENT_BEACTIVED:
		{
			CNpc* pNpc;
			IRole* pRole = mapGroup.GetRoleManager()->QuerySet()->GetObj(m_id);
			if (pRole && pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)) && pUser->GetMapID() == pNpc->GetMapID())
			{
				pNpc->ActivateNpc(pUser->QueryRole(), 0);
			}
		}
		break;
	case EVENT_DELNPC:
		{
			CNpc* pNpc;
			IRole* pRole = mapGroup.GetRoleManager()->QuerySet()->GetObj(m_id);
			if (pRole && pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)) && pUser->GetMapID() == pNpc->GetMapID())
			{
				if(pNpc->IsDeleted())
					return ;

				if(pNpc->IsDelEnable() && pNpc->IsOwnerOf(pUser))		// 帮派柱子不能删除
				{
					if(pNpc->IsRecycleEnable())
					{
						// 只检查普通背包是否满
						if(pUser->IsItemFull(CItem::GetWeight(pNpc->GetInt(NPCDATA_ITEMTYPE)), ID_NONE, ITEMPOSITION_BACKPACK))
						{
							pUser->SendSysMsg(STR_YOUR_BAG_FULL);
							return ;
						}

						CItem* pItem = pNpc->Recycle(pUser->GetID());
						if(pItem)
						{
							pUser->AddItem(pItem, SYNCHRO_TRUE);
							pUser->SendSysMsg(STR_GOT_ITEM, pItem->GetStr(ITEMDATA_NAME));
						}
						else
							pUser->SendSysMsg(STR_ITEM_DAMAGED);
					}
					if(!pNpc->DelNpc())
						pUser->SendSysMsg(STR_DELETE_FAILED);
				}
			}
		}
		break;
	case EVENT_CHANGEPOS:
		{
			CNpc* pNpc;
			IRole* pRole = mapGroup.GetRoleManager()->QuerySet()->GetObj(m_id);
			if (pRole && pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)) && pUser->GetMapID() == pNpc->GetMapID())
			{
				int	nLook = ::MaskLook(m_usType);
				if(::MaskLook(pNpc->GetLookFace()) == nLook)
					pNpc->ChangeDir(nLook);
				pNpc->TransPos(m_usPosX, m_usPosY);		// synchro true
			}
		}
		break;

	default:
		::LogSave("ERROR: CMsgNpc::Process() 内部异常");
	}
}

