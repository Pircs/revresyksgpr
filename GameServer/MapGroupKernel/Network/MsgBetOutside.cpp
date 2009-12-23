// ÏÉ½£ÐÞ£¬2002.1.8

#include "AllMsg.h"
#include "UserManager.h"
#include "common.h"
#include "BetOutside.h"
#include "GameMap.h"
#include "MapGroup.h"

void CMsgBODice::Process(CUser* pUser)
{
	if(!pUser)
		return ;

	switch(m_ucAction)
	{
	case	_ACTION_CHIPIN:
		{
			IRole* pRole = pUser->FindAroundRole(m_idDiceNpc);
			CNpc* pNpc;
			if(pRole && pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)) && pNpc->IsBetNpc() && m_Data[0].dwData)
			{
				pNpc->QueryBet()->ChipIn(pUser, m_Data[0].ucType, m_Data[0].dwData);
			}
		}
		break;
	case	_ACTION_CANCELCHIP:
		{
			IRole* pRole = pUser->FindAroundRole(m_idDiceNpc);
			CNpc* pNpc;
			if(pRole && pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)) && pNpc->IsBetNpc())
			{
				pNpc->QueryBet()->ChipIn(pUser, m_Data[0].ucType, 0);
			}
		}
		break;
	default:
		ASSERT(!"switch(m_ucAction)");
	}
}