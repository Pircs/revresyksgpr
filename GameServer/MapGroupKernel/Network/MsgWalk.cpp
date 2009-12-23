#include "AllMsg.h"
#include "mapgroup.h"
#include "transformation.h"
#include "I_Role.h"

void CMsgWalk::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{
	DEBUG_TRY
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgWalk, idUser:%u, data:%u",
					m_idUser,
					m_dwData);
#endif

	// get obj
	IRole* pRole	= mapGroup.GetRoleManager()->QueryRole(idSocket, idNpc, m_idUser);
	if(!pRole)
	{
		if(idSocket != SOCKET_NONE)//!this->IsNpcMsg()
			LOGERROR("CMsgWalk消息没有找到玩家，SOCKET_ID[%u],NPC_ID[%u]", idSocket, idNpc);
		return ;
	}
	//xxxCHECK(pRole->GetID() == m_idUser);

	int nDir = m_ucDir%8;
	int nNewX = pRole->GetPosX()+_DELTA_X[nDir];
	int nNewY = pRole->GetPosY()+_DELTA_Y[nDir];

	CUser* pUser = NULL;
	if(pRole->QueryObj(OBJ_USER, IPP_OF(pUser)))
	{
		if(!pRole->IsAlive() && !pUser->IsGhost())
		{
			pRole->SendSysMsg(STR_DIE);
			return ;
		}
		if (pUser->QueryTransformation() && !pUser->QueryTransformation()->IsMoveEnable())		// BUG: 刚变时，客户端还没禁跳
		{
			pUser->SendSysMsg(STR_YOUR_CANNOT_WALK);
			pUser->KickBack();
			//UserManager()->KickOutSocket(m_idSocket, "不能走！");
			return;
		}
		if (pUser->QueryStatus(STATUS_LOCK) || pUser->QueryStatus(STATUS_FREEZE)
			|| pRole->QueryStatus(STATUS_FAINT))
		{
			pUser->KickBack();
#ifdef ZLONG_DEBUG
			pUser->SendSysMsg("Debug: 不可移动。");
#endif
			return;
		}
		if (pUser->QueryMagic() && pUser->QueryMagic()->IsInLaunch())
		{
			pUser->KickBack();
#ifdef	ZLONG_DEBUG
			pUser->SendSysMsg("Debug: 魔法施展中，踢回。");
#endif
			return;
		}
		if (pUser->QueryMagic() && pUser->QueryMagic()->IsIntone())
		{
			pUser->QueryMagic()->AbortMagic();
		}
		if(pUser->IsAlive() && !pUser->GetMap()->IsMoveEnable(nNewX, nNewY))
		{
			pUser->KickBack();
#ifdef	LOCAL_DEBUG
			pUser->SendSysMsg("阻挡：(%d,%d)", nNewX, nNewY);
#endif
			return;
		}
		// 跑步模式的阻挡判断
		if (m_ucMode >= MOVEMODE_RUN_DIR0 && m_ucMode <= MOVEMODE_RUN_DIR7)
		{
			nNewX += _DELTA_X[m_ucMode - MOVEMODE_RUN_DIR0];
			nNewY += _DELTA_Y[m_ucMode - MOVEMODE_RUN_DIR0];

			if(pUser->IsAlive() && !pUser->GetMap()->IsMoveEnable(nNewX, nNewY))
			{
				pUser->KickBack();
#ifdef	LOCAL_DEBUG
				pUser->SendSysMsg("阻挡：(%d,%d)", nNewX, nNewY);
#endif
				return;
			}
		}
	}
	else
	{
		if(!pRole->IsAlive())
			return ;
		CMonster* pMonster = NULL;
		if (pRole->QueryObj(OBJ_MONSTER, IPP_OF(pMonster)))
		{
			if (!pRole->GetMap())
				return ;
			if (!pRole->GetMap()->IsMoveEnable(nNewX, nNewY))
			{
				pMonster->KickBack();
#ifdef LOCAL_DEBUG
				::LogSave("Kick back monster from (%d, %d) [ID=%u].", nNewX, nNewY, pMonster->GetID());
#endif
				return;
			}
			if (m_ucMode >= MOVEMODE_RUN_DIR0 && m_ucMode <= MOVEMODE_RUN_DIR7)
			{
				nNewX += _DELTA_X[m_ucMode - MOVEMODE_RUN_DIR0];
				nNewY += _DELTA_Y[m_ucMode - MOVEMODE_RUN_DIR0];

				if (!pRole->GetMap()->IsMoveEnable(nNewX, nNewY))
				{
					pMonster->KickBack();
#ifdef LOCAL_DEBUG
					::LogSave("Kick back monster from (%d, %d) [ID=%u].", nNewX, nNewY, pMonster->GetID());
#endif
					return;
				}
			}
		}
	}

	DEBUG_TRY
	// stop fight
	if(m_ucMode != MOVEMODE_SHIFT)
		pRole->ProcessOnMove(m_ucMode);
	DEBUG_CATCH("ProcessOnMove")

	// fill id
	DEBUG_TRY
//*
//	pRole->BroadcastRoomMsg(this, EXCLUDE_SELF);

	{
		MsgWalkEx msg;
		IF_OK(msg.Create(m_idUser, m_ucDir, m_ucMode, nNewX, nNewY))
		{
//			pRole->SendMsg(&msg);
			pRole->BroadcastRoomMsg(&msg, INCLUDE_SELF);
		}
	}
//*/
/*else
	pRole->SendMsg(this);
//*/
	bool bRunMode = (m_ucMode >= MOVEMODE_RUN_DIR0 && m_ucMode <= MOVEMODE_RUN_DIR7);
	pRole->MoveToward(nDir, !bRunMode);		// return true: 是n步模数
	// 如果是，跑步模式，移动第二步
	if (bRunMode)
		pRole->MoveToward(m_ucMode - MOVEMODE_RUN_DIR0);

	DEBUG_CATCH("MoveToward")

#ifdef	PALED_DEBUG_X
	MSGBUF	szMsg;
	if(IsNpcID(pRole->GetID()))
		sprintf(szMsg, "%s%06dWALK: (%d,%d)", pRole->GetName(), pRole->GetID(), pRole->GetPosX(), pRole->GetPosY());
	else
		sprintf(szMsg, "%sWALK: (%d,%d)", pRole->GetName(), pRole->GetPosX(), pRole->GetPosY());
	//LOGWARNING(szMsg);
	DebugSave(szMsg);
#endif
	DEBUG_CATCH("CMsgWalk::Process")
}
