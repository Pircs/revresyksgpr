#include "AllMsg.h"
#include "NpcWorld.h"
#include "ItemType.h"
#include "Agent.h"

void CMsgItem::Process(OBJID idNpc)
{
	CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
	if(!pAgent)
		return;
	CItemPack* pPack = Cast<CItemPack>(pAgent);
	CHECK(pPack);

	try {
	switch(m_usAction)
	{
	case ITEMACT_EQUIP:
		{
			pPack->EquipItem(m_id, m_dwData);
		}
		break;
	case ITEMACT_UNEQUIP:
		{
			pPack->UnequipItem(m_id, m_dwData);
		}
		break;
	case ITEMACT_DROP:
		{
			pPack->DropItem(m_id);
		}
		break;
	case ITEMACT_DROPEQUIPMENT:
		{
			pPack->DropEquipItem(m_id, m_dwData);
		}
		break;
	case ITEMACT_SYNCHRO_AMOUNT:
		{
			pPack->SetItemAmount(m_id, m_dwData);
		}
		break;
	case ITEMACT_QUERYMONEYSAVED:
		{
			//@ ²Ö¿â
		}
		break;
	case ITEMACT_REPAIRALL:
		{
			//@
		}
		break;
	case ITEMACT_DURABILITY:
		{
			//@
		}
		break;
	case ITEMACT_BOOTH_ADD:
		{
			//@
		}
		break;
	case ITEMACT_BOOTH_DEL:
		{
			//@
		}
		break;
	}
	}catch(...)
	{
		::LogSave("switch(MSGITEM) Action [%d], id [%u], data [%u] ", m_usAction, m_id, m_dwData);
	}


#ifdef _MSGDEBUG
	::LogMsg("Process CMsgItem, id:%u", m_id);
#endif
}