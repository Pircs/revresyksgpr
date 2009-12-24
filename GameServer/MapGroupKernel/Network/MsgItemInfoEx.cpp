#include "AllMsg.h"
#include "Item.h"
#pragma	warning(disable:4786)

bool CMsgItemInfoEx::Create(CItem* pItem, OBJID idOwner, int nCost, int nAction /*= ITEMINFOEX_BOOTH*/)
{
	CHECKF(pItem);
	ItemInfoStruct info;
	CHECKF(pItem->GetInfo(&info));

	m_ucAction			= nAction;

	m_id					= info.id;
	m_dwType				= info.idType;
	m_idOwner			= idOwner;
	m_dwPrice			= nCost;
	m_ucPosition			= info.nPosition;
	m_ucStatus			= _ITEM_STATUS_NOT_IDENT;
	if(!pItem->IsNeedIdent())
	{
		m_usAmount			= info.nAmount;
		m_usAmountLimit		= info.nAmountLimit;
		m_ucStatus			= info.nIdent;
		m_ucGem1				= info.nGem1;
		m_ucGem2				= info.nGem2;
		m_ucMagic1			= info.nMagic1;
		m_ucMagic2			= info.nMagic2;
		m_ucMagic3			= info.nMagic3;
		m_nData				= info.nData;
		
		m_dwWarGhostExp		= info.dwWarGhostExp;
		m_dwGemAtkType		= info.dwGemAtkType;
		m_dwAvailabeTime		= info.dwAvailabeTime;

		if (pItem->IsEudemon())
			m_StrPacker.AddString(pItem->GetStr(ITEMDATA_NAME));
	}
	UPDATE_MSG_SIZE;
	return true;
}