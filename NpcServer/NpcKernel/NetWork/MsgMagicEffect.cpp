#include "AllMsg.h"
#include "NpcWorld.h"

void CMsgMagicEffect::Process()
{
	INpc* pNpc = NpcManager()->QueryNpc(m_idUser);
	if(pNpc)
		pNpc->AttackOnce();

	// check npc be attack
	IRole* pRole = UserManager()->QueryRole(m_idUser);
	if(pRole && m_usType != MAGICSORT_RECRUIT)
	{
		OBJID idTarget = m_idTarget;
		if(idTarget && idTarget != pRole->GetID())
		{
			INpc* pNpc = NpcManager()->QueryNpc(idTarget);
			if(pNpc)
				pNpc->BeAttack(pRole);
		}

		for(int i = 0; i < m_setEffectRole.GetSize(); i++)
		{
			OBJID idRole = m_setEffectRole[i].idRole;

			if(idRole != idTarget)		// call BeAttack() one times.
			{
				INpc* pNpc = NpcManager()->QueryNpc(idRole);
				if(pNpc)
					pNpc->BeAttack(pRole);
			}
		}
	}

	CUser* pUser = UserManager()->QueryUser(m_idUser);
	if(!pUser)
		return;

	switch(m_usType)		// 只处理会MOVE的消息
	{
	case	MAGICSORT_JUMPATTACK:
		{
			pUser->JumpPos(m_usPosX, m_usPosY, 0);
		}
		break;
	case	MAGICSORT_RANDOMTRANS:
		{
			pUser->JumpPos(m_usPosX, m_usPosY, 0);
		}
		break;
	case	MAGICSORT_COLLIDE:
		{/* move code to interact
			int nDir = m_ucCollideDir;
			if(nDir >= 0 && nDir < MAX_DIRSIZE)
			{
				pUser->MoveForward(nDir);
				OBJID idTarget = m_setEffectRole[0].idRole;
				if(!IsNpcID(idTarget))IsSceneID
				{
					CUser* pTarget = UserManager()->QueryUser(idTarget);
					if(pTarget && pTarget->IsAlive())
						pTarget->MoveForward(m_ucCollideDir);
				}
				else
				{
					INpc* pNpc = NpcManager()->QueryNpc(idTarget);
					if(pNpc && pNpc->IsAlive())
					{
						POINT pos;
						pos.x = pUser->GetPosX()+_DELTA_X[nDir];
						pos.y = pUser->GetPosY()+_DELTA_Y[nDir];
						pNpc->SynchroPos(pos.x, pos.y, pos.x+_DELTA_X[nDir], pos.y+_DELTA_Y[nDir]);
					}
				}
			}*/
		}
		break;
	}
}