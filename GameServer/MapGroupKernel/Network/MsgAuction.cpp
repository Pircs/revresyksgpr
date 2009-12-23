#include "AllMsg.h"
#include "Auction.h"
#include "MapGroup.h"
#include "npc.h"

bool CMsgAuction::Create(char* szName,OBJID idItem,  DWORD dwValue,int nAuction)
{
	m_idItem = idItem;
	m_dwValue = dwValue;
	m_nAuction = nAuction;
	m_StrPacker.AddString(szName);
	UPDATE_MSG_SIZE;
	return true;
}
bool CMsgAuction::Create(OBJID dwData, int dwValue,int nAuction)
{
//	if(nAuction != AUCTION_SYS)
	{
		m_dwData = dwData;
		m_dwValue = dwValue;
	}
	m_nAuction = nAuction;
	UPDATE_MSG_SIZE;
	return true;
}

void CMsgAuction::Process(CMapGroup& mapGroup, CUser* pUser)
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgAuction, id:%u", m_idItem);
#endif
	//CUser* pUser = mapGroup.GetUserManager()->GetUser(idSocket,idNpc);
    CNpc* pNpc; 
	if (pUser)
	{
		switch(m_nAuction)
		{
		case AUCTION_ADD://加入拍买物品
			{
			if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, m_idNpc, IPP_OF(pNpc)))
				return ;
			if(!pNpc->IsAuctionNpc())
				return;
   			int count = pUser->GetAuctionPackageAmount(m_idNpc);
			if(count < PACKAGE_AUCTION_LIMIT)
			{
				CItem *pTemp = NULL;
				for(int i = 0;i < pNpc->QueryAuction()->QueryPackage()->GetAmount();i++)
				{ 
					pTemp = pNpc->QueryAuction()->QueryPackage()->GetItemByIndex(i);
					if( pTemp && pTemp->GetInt(ITEMDATA_PLAYERID) == pUser->GetID())
						if(++count >= PACKAGE_AUCTION_LIMIT)
							break;
				}
			}
			if(count >= PACKAGE_AUCTION_LIMIT)
			{
				pUser->SendSysMsg(STR_AUCTION_PACKAGE_FULL);
				return ;
			}
			pNpc->QueryAuction()->JoinAuction(pUser->GetID(),m_idItem ,m_dwValue,mapGroup.GetDatabase());
			}
			break;
		case AUCTION_BID://竞标出价
			{
			if(pUser->GetMoney() < m_dwValue)
			{
				pUser->SendSysMsg(STR_AUCTION_LESS_MONEY);
				return ;
			}
			if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, m_idNpc, IPP_OF(pNpc)))
				return ;
			if(!pNpc->IsAuctionNpc())
				return;
			int count = pUser->GetAuctionPackageAmount(m_idNpc);
			if(count < PACKAGE_AUCTION_LIMIT)
			{
				CItem *pTemp = NULL;
				for(int i = 0;i < pNpc->QueryAuction()->QueryPackage()->GetAmount();i++)
				{ 
					pTemp = pNpc->QueryAuction()->QueryPackage()->GetItemByIndex(i);
					if( pTemp && pTemp->GetInt(ITEMDATA_PLAYERID) == pUser->GetID())
						if(++count >= PACKAGE_AUCTION_LIMIT)
							break;
				}
			}
			if(count >= PACKAGE_AUCTION_LIMIT)
			{
				pUser->SendSysMsg(STR_AUCTION_PACKAGE_FULL1);
				return ;
			}
			pNpc->QueryAuction()->ShoutPrice(m_idItem,pUser->GetID(),m_dwValue);
			}
			break;
		case AUCTION_ITEM_INFO://查看物品的信息
			if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, m_idNpc, IPP_OF(pNpc)))
				return ;
			if(!pNpc->IsAuctionNpc())
				return;
			DEBUG_TRY
			pNpc->QueryAuction()->SendAuctionItemInfo(pUser->GetID(),m_idItem);
			DEBUG_CATCH("AUCTION Get ItemInfo ERROR!");
			break;
		default:
			ASSERT(!"CMsgAuction");
		}
	}
}