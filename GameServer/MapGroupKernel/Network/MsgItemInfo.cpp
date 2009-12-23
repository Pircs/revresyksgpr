// MsgItemInfo.cpp: interface for the CMsgItemInfo class.
//

#include "AllMsg.h"
#include "MapGroup.h"
#include "ItemType.h"
#include "AuctionSystemItemData.h"
#pragma	warning(disable:4786)

bool CMsgItemInfo::Create(CGameAuctionSystemData* pData, int nAction)
{	
	// init
	CHECKF(pData);
	CItemTypeData* pItemInfo = NULL;
	pItemInfo = ItemType()->QueryItemType(pData->GetInt(AUCTION_SYSTEMITEM_TYPE));				
	CHECKF(pItemInfo);
	
	// fill info now
	m_ucAction			= nAction;	
	m_id					= pData->GetID();	
	m_dwType				= pData->GetInt(AUCTION_SYSTEMITEM_TYPE);
	m_ucIdent			= _ITEM_STATUS_NOT_IDENT;
	m_usAmount			= pData->GetInt(AUCTION_SYSTEMITEM_AMOUNT);
	m_usAmountLimit		= pData->GetInt(AUCTION_SYSTEMITEM_AMOUNTLIMIT);
	m_ucIdent			= pData->GetInt(AUCTION_SYSTEMITEM_IDENT);
	m_ucGem1				= pData->GetInt(AUCTION_SYSTEMITEM_GEM1);
	m_ucGem2				= pData->GetInt(AUCTION_SYSTEMITEM_GEM2);
	m_ucMagic1			= pData->GetInt(AUCTION_SYSTEMITEM_MAGIC1);
	m_ucMagic2			= pData->GetInt(AUCTION_SYSTEMITEM_MAGIC2);
	m_ucMagic3			= pData->GetInt(AUCTION_SYSTEMITEM_MAGIC3);
	m_nData				= pData->GetInt(AUCTION_SYSTEMITEM_DATA);
	m_dwWarGhostExp      = pData->GetInt(AUCTION_SYSTEMITEM_WARGHOSTEXP);
	m_dwGemAtkType       = pData->GetInt(AUCTION_SYSTEMITEM_GEMTYPE);
	m_dwAvailabeTime     = pData->GetInt(AUCTION_SYSTEMITEM_AVAILABLETIME);		
	m_StrPacker.AddString(pItemInfo->GetStr(ITEMTYPEDATA_NAME));
	UPDATE_MSG_SIZE;
	return true;
}

bool CMsgItemInfo::Create(CItem* pItem, int nAction /*= ITEMINFO_ADDITEM*/, OBJID idUser/*=ID_NONE*/)
{
	CHECKF(pItem);
	ItemInfoStruct info;
	CHECKF(pItem->GetInfo(&info));

	// fill info now
	m_ucAction			= nAction;

	m_id					= info.id;
	if(idUser != ID_NONE)
		m_id					= idUser;
	m_dwType				= info.idType;
	m_ucPosition			= info.nPosition;
	m_ucIdent			= _ITEM_STATUS_NOT_IDENT;
	if(!pItem->IsNeedIdent())
	{
		m_usAmount			= info.nAmount;
		m_usAmountLimit		= info.nAmountLimit;
		m_ucIdent			= info.nIdent;
		m_ucGem1				= info.nGem1;
		m_ucGem2				= info.nGem2;
		m_ucMagic1			= info.nMagic1;
		m_ucMagic2			= info.nMagic2;
		m_ucMagic3			= info.nMagic3;
		m_nData				= info.nData;

		//---jinggy---2004-11-19---begin
		m_dwWarGhostExp = info.dwWarGhostExp;
		m_dwGemAtkType = info.dwGemAtkType;
		m_dwAvailabeTime = info.dwAvailabeTime;		
		//---jinggy---2004-11-19---end
	}
	else
	{
		m_dwType					= CItem::HideTypeUnident(m_dwType);
	}

	// eudemon
	if (pItem->IsEudemon())
		m_StrPacker.AddString(pItem->GetStr(ITEMDATA_NAME));

	UPDATE_MSG_SIZE;
	return true;
}
