#include "AllMsg.h"
#include "npcworld.h"

void CMsgInteract::Process()
{
	IRole* pRole = UserManager()->QueryRole(m_idSender);
	if(!pRole)
		return;

	switch(m_unType)
	{
	case	INTERACT_STEAL:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			pNpc->BeAttack(pRole);
		}
		break;

	case	INTERACT_ATTACK:
	case	INTERACT_SHOOT:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idSender);
			if(pNpc)
				pNpc->AttackOnce();

			{
				INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
				if(pNpc)
					pNpc->BeAttack(pRole);
			}
		}
		break;
	case	INTERACT_RUSHATK:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(pNpc)
				pNpc->BeAttack(pRole);

			CUser* pUser = UserManager()->QueryUser(m_idSender);
			if(pUser)
				pUser->JumpPos(m_unPosX, m_unPosY, m_dwData);
		}
		break;
	case	INTERACT_MAGICATTACK:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;
		}
		break;
	case	INTERACT_ABORTMAGIC:
		{
			ASSERT(!"INTERACT_ABORTMAGIC");
		}
		break;

	case	INTERACT_HEAL:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("thanks!");
		}
		break;

	case	INTERACT_POISON:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			pNpc->BeAttack(pRole);
		}
		break;

	case	INTERACT_ASSASSINATE:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			pNpc->BeAttack(pRole);
		}
		break;

	case	INTERACT_FREEZE:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			pNpc->BeAttack(pRole);
		}
		break;

	case	INTERACT_UNFREEZE:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("thanks!");
		}
		break;

	case	INTERACT_COURT:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("sorry!");
		}
		break;

	case	INTERACT_MARRY:
		{			
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("sorry!");
		}
		break;

	case	INTERACT_DIVORCE:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("sorry!");
		}
		break;

	case	INTERACT_PRESENTMONEY:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("thanks!");
		}
		break;

	case	INTERACT_PRESENTITEM:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("thanks!");
		}
		break;

	case	INTERACT_SENDFLOWERS:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("thanks!");
		}
		break;

	case	INTERACT_JOINSYN:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("sorry!");
		}
		break;
	case	INTERACT_ACCEPTSYNMEMBER:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("sorry!");
		}
		break;

	case	INTERACT_KICKOUTSYNMEMBER:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("sorry!");
		}
		break;

	case	INTERACT_PRESENTPOWER:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
			if(!pNpc)
				return;

			//pNpc->SendMsg("thanks!");
		}
		break;

	case	INTERACT_QUERYINFO:
		{
			ASSERT(!"INTERACT_QUERYINFO");
		}
		break;

	case	INTERACT_KILL:
		{
			if(IsSceneID(m_idTarget))
			{
				;
			}
			else
			{
				INpc* pNpc = NpcManager()->QueryNpc(m_idTarget);
				if(pNpc)
				{
					//pNpc->SetEffect(KEEPEFFECT_DIE|pNpc->GetEffect());
					pNpc->BeKill(pRole);
				}
			}
			//else
			{
				/*CUser* pUser = UserManager()->QueryUser(m_idTarget);
				if(pUser)
					pUser->SetEffect(KEEPEFFECT_DIE|pUser->GetEffect());*/
			}
		}
		break;
	case	INTERACT_BUMP:
		{
			bool	nBack	= (m_dwData & 0x10000000) == 0;
			CUser* pUser = UserManager()->QueryUser(m_idSender);
			if(pUser && nBack)
				pUser->SetPos(m_unPosX, m_unPosY);

			int		nDir	= (m_dwData & 0x0FFFFFFF) / 0x01000000;
			if(nDir >= 0 && nDir < MAX_DIRSIZE)
			{
				OBJID idTarget = m_idTarget;
				if(IsSceneID(idTarget))
				{
					ASSERT(!"INTERACT_BUMP");
				}
				else
				{
					if(!IsNpcID(idTarget))
					{
						CUser* pTarget = UserManager()->QueryUser(idTarget);
						if(pTarget)
						{
							if(pTarget->IsAlive())
								pTarget->MoveForward(nDir);
							break;
						}
					}

					INpc* pNpc = NpcManager()->QueryNpc(idTarget);
					if(pNpc && pNpc->IsAlive())
					{
						POINT pos;
						pos.x = m_unPosX;
						pos.y = m_unPosY;
						pNpc->SynchroPos(pos.x, pos.y, pos.x+_DELTA_X[nDir], pos.y+_DELTA_Y[nDir]);

						pNpc->BeAttack(pRole);
					}
				}
			}
		}
		break;

	} // switch
	return;

#ifdef _MSGDEBUG
	::LogMsg("Process CMsgInteract, id:%u", m_id);
#endif
}







