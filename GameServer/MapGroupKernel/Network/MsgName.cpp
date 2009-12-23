#include "AllMsg.h"
#include "mapgroup.h"
#include "WantedList.h"
#include "PoliceWanted.h"
#include "BetOutside.h"

void CMsgName::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{
	CUser* pUser = mapGroup.GetUserManager()->GetUser(idSocket, idNpc);
	if(!pUser)
		return;

	char szName[_MAX_WORDSSIZE];
	m_StrPacker.GetString(0, szName, sizeof(szName));

	switch(m_ucType)
	{
	case	NAMEACT_QUERY_WANTED:
		{
			CWantedData* pWanted = CWantedList::s_WantedList.GetWanted(pUser->WantedInfo().idWanted);
			if (pWanted)
			{
				CUser* pTarget = mapGroup.GetUserManager()->GetUser(pWanted->GetStr(DATA_TARGET_NAME));
				if (pTarget && pUser->FindAroundUser(pTarget->GetID()))
					pUser->SendSysMsg(_TXTATR_NORMAL, STR_FOND_WANTED, pTarget->GetName());
			}
		}
		break;

	case	NAMEACT_QUERY_POLICEWANTED:
		{
			OBJID idWanted = pUser->PoliceWantedID();
			PoliceWantedStruct* pInfo = PoliceWanted().GetWanted(idWanted);

			if (pUser->FindAroundUser(idWanted) && pInfo)
				pUser->SendSysMsg(_TXTATR_NORMAL, STR_FOND_POLICEWANTED, pInfo->strName.c_str());
		}
		break;

	case	NAMEACT_FIREWORKS:
		{
			pUser->BroadcastRoomMsg(this, EXCLUDE_SELF);
		}
		break;
	case	NAMEACT_CREATE_SYN: 				// 改到TALK中
		{
			ASSERT(!"NAMEACT_CREATE_SYN");
		}
		break;
	case	NAMEACT_SYNDICATE: 				// 无法实现修改帮派名，仅下传
		{
			ASSERT(!"NAMEACT_SYNDICATE");
		}
		break;
	case	NAMEACT_CHANGE_TITLE: 
		{
			ASSERT(!"NAMEACT_CHANGE_TITLE");
		}
		break;
	case	NAMEACT_DELROLE:						// 无意义
		{
			ASSERT(!"NAMEACT_DELROLE");
		}
		break;
	case	NAMEACT_MATE:
		{
			ASSERT(!"NAMEACT_MATE");
		}
		break;
	case	NAMEACT_QUERY_NPC:
		{
			if(ID_NONE == m_idTarget)
				return ;
			CNpc* pNpc;
			IRole* pRole = mapGroup.GetRoleManager()->QuerySet()->GetObj(m_idTarget);
			if (pRole && pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)))
			{
				Create(NAMEACT_QUERY_NPC, pNpc->GetName(), m_idTarget);
				pUser->SendMsg(this);
			}
		}
		break;
	case NAMEACT_MEMBERLIST_SPECIFYSYN:
		{
 			CSyndicate * pSyn = mapGroup.GetSynManager()->QuerySynByName(szName);
			CHECK(pSyn);
			OBJID idSyn = pSyn->GetID();
			CHECK(idSyn != ID_NONE);
			SetTransData(idSyn);
			mapGroup.QueryIntraMsg()->TransmitWorldMsg(this);
		}
		break;
	case	NAMEACT_MEMBERLIST:
		{
			OBJID idSyn = pUser->GetSynID();
			CHECK(idSyn != ID_NONE);
			SetTransData(idSyn);
			mapGroup.QueryIntraMsg()->TransmitWorldMsg(this);
		}
		break;
	case	NAMEACT_QUERY_MATE:
		{
			CUser* pTarget = mapGroup.GetUserManager()->GetUser(m_idTarget);
			if(pTarget)
			{
				MsgName msg;
				IF_OK(msg.Create(NAMEACT_QUERY_MATE, pTarget->GetMate(), m_idTarget))
				{
					if(idSocket!=SOCKET_NONE)
						mapGroup.QueryIntraMsg()->SendClientMsg(idSocket, &msg);
					else
						mapGroup.QueryIntraMsg()->SendNpcMsg(idNpc, &msg);
				}//	SendMsg(this);
			}
		}
		break;
	case	NAMEACT_ADDDICE_PLAYER:
		{
			IRole* pRole = pUser->FindAroundRole(m_idTarget);
			CNpc* pNpc;
			if(pRole && pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)) && pNpc->IsBetNpc())
			{
				pNpc->QueryBet()->JoinBet(pUser);
			}
		}
		break;
	case	NAMEACT_DELDICE_PLAYER:
		{
			CMsgName	msg;
			IF_OK(msg.Create(NAMEACT_DELDICE_PLAYER, pUser->GetName(), m_idTarget))
				pUser->SendMsg(&msg);

			IRole* pRole = mapGroup.GetRoleManager()->QueryRole(m_idTarget);
			CNpc* pNpc;
			if(pRole && pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)) && pNpc->IsBetNpc())
			{
				pNpc->QueryBet()->LeaveBet(pUser);
			}
		}
		break;
	case	NAMEACT_CHANGE_EUDEMON_NAME:
		{
			if (m_StrPacker.GetCount()<1)
				return;

			// 只有幻兽才可以改名
			CItem* pItem = pUser->GetItem(m_idTarget);
			if (!pItem || !pItem->IsEudemon())
				return ;

			// 召回幻兽
			//pUser->CallBackEudemon(pItem->GetID());

			// 检查名字合法性
			char szName[_MAX_NAMESIZE];
			if (!m_StrPacker.GetString(0, szName, _MAX_NAMESIZE))
				return;
			if (!NameStrCheck(szName))
			{
				pUser->SendSysMsg(STR_ERROR_ILLEGAL_NAME2);
				return;
			}

			// 成功改名
			pItem->SetStr(ITEMDATA_NAME, szName, _MAX_NAMESIZE, true);
			CMsgItemInfo	msg;
			if (msg.Create(pItem, ITEMINFO_UPDATE))
				pUser->SendMsg(&msg);

			// 如果幻兽召唤出来了，则更新幻兽资料
			CMonster* pEudemon = pUser->QueryEudemon(pItem->GetID());
			if (pEudemon)
			{
				CMsgPlayer	msg;
				if (msg.Create(pEudemon->QueryRole()))
					pEudemon->BroadcastRoomMsg(&msg, true);
			}
		}
		break;
	}
}