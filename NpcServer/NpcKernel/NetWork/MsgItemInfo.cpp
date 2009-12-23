#include "AllMsg.h"
#include "Item.h"
#include "NpcWorld.h"
#include "Agent.h"
#pragma	warning(disable:4786)

void CMsgItemInfo::Process(OBJID idNpc)
{
	CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
	if(!pAgent)
		return ;

	switch(m_ucAction)
	{
	case	ITEMINFO_ADDITEM:
		{
			ItemInfoStruct info;
			memset(&info, 0, sizeof(ItemInfoStruct));
			info[ITEMDATA_ID_]			= m_id;
			info[ITEMDATA_TYPE]			= m_dwType;
			info[ITEMDATA_AMOUNT]		= m_usAmount;
			info[ITEMDATA_AMOUNTLIMIT]	= m_usAmountLimit;
			info[ITEMDATA_IDENT]		= m_ucIdent;
			info[ITEMDATA_POSITION]		= m_ucPosition;
			info[ITEMDATA_GEM1]			= m_ucGem1;
			info[ITEMDATA_GEM2]			= m_ucGem2;
			info[ITEMDATA_MAGIC1]		= m_ucMagic1;
			info[ITEMDATA_MAGIC2]		= m_ucMagic2;
			info[ITEMDATA_MAGIC3]		= m_ucMagic3;
			info[ITEMDATA_WARGHOSTEXP]	= m_dwWarGhostExp;
			info[ITEMDATA_GEMTYPE]		= m_dwGemAtkType;
			info[ITEMDATA_AVAILABLETIME]= m_dwAvailabeTime;
			CItem* pItem = CItem::CreateNew();
			IF_OK(pItem)
			{
				IF_OK(pItem->Create(&info))
				{
					if(m_ucPosition == ITEMPOSITION_BACKPACK)
					{
						ASSERT(Cast<CItemPack>(pAgent)->AddItem(pItem));
					}
					else
					{
						ASSERT(Cast<CItemPack>(pAgent)->AddEquip(pItem));
					}
				}
				else
					pItem->ReleaseByOwner();
			}
		}
		break;
	case	ITEMINFO_UPDATE:
		{
			ItemInfoStruct info;
			memset(&info, 0, sizeof(ItemInfoStruct));
			info[ITEMDATA_ID_]			= m_id;
			info[ITEMDATA_TYPE]			= m_dwType;
			info[ITEMDATA_AMOUNT]		= m_usAmount;
			info[ITEMDATA_AMOUNTLIMIT]	= m_usAmountLimit;
			info[ITEMDATA_IDENT]		= m_ucIdent;
			info[ITEMDATA_POSITION]		= m_ucPosition;
			info[ITEMDATA_GEM1]			= m_ucGem1;
			info[ITEMDATA_GEM2]			= m_ucGem2;
			info[ITEMDATA_MAGIC1]		= m_ucMagic1;
			info[ITEMDATA_MAGIC2]		= m_ucMagic2;
			info[ITEMDATA_MAGIC3]		= m_ucMagic3;
			info[ITEMDATA_WARGHOSTEXP]	= m_dwWarGhostExp;
			info[ITEMDATA_GEMTYPE]		= m_dwGemAtkType;
			info[ITEMDATA_AVAILABLETIME]= m_dwAvailabeTime;

			OBJID idItem = m_id;
			CItem* pItem = Cast<CItemPack>(pAgent)->FindItem(idItem, idItem);
			if(pItem)
			{
				ASSERT(pItem->UpdateInfo(&info));
			}
		}
		break;
	}

}
