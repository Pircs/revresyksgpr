#include "AllMsg.h"
#include "npcworld.h"
#include "SharedBaseFunc.h"
#include "agent.h"

void CMsgAction::Process(OBJID idNpc)
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgAction, idUser:%u, data:%u",
					m_idUser,
					m_dwData);
#endif


	// actions...
	switch(m_usAction)
	{
	case actionChgDir:
		{
			CUser* pUser	= UserManager()->QueryUser(m_idUser);
			if (pUser)
				pUser->SetDir(m_unDir);
		}
		break;
	case actionPosition:
		{
			//@
		}
		break;
	case actionEmotion:
		{
			CUser* pUser	= UserManager()->QueryUser(m_idUser);
			if (pUser)
				pUser->SetPose(m_dwData);
		}
		break;
	case actionBroadcastPos:
		{
			//@
		}
		break;
	case actionDivorce:
		{
			ASSERT(!"actionDivorce");
		}
		break;
	case actionSelfUnfreeze:
		{
			ASSERT(!"actionSelfUnfreeze");
		}
		break;
	case actionChgMap:				// 注意：没有回应actionChgMap，而是回应actionFly
		{
			ASSERT(!"actionChgMap");
		}
		break;
	case actionFlyMap:
		{
			CUser* pUser	= UserManager()->QueryUser(m_idUser);
			if (pUser)
			{
				OBJID idMap = m_idTarget;
				pUser->FlyMap(idMap, m_unPosX, m_unPosY);		// 回应actionFlyMap
				return ;
			}

			CAgent* pAgent = NpcManager()->QueryAgent(m_idUser);
			if(pAgent)
			{
				OBJID idMap = m_idTarget;
				Cast<CRoleMove>(pAgent)->FlyMap(idMap, m_unPosX, m_unPosY);		// 回应actionFlyMap
				return ;
			}
		}
		break;
	case actionChgWeather:
		{
			//@
		}
		break;
	case actionFireworks:
		{
			//@
		}
		break;
	case actionDie:
		{
			// 自杀
			if(IsSceneID(m_idUser))
			{
				;
			}
			else
			{
				INpc* pNpc = NpcManager()->QueryNpc(m_idUser);
				IF_OK(pNpc)
					pNpc->SetDie();
			}
		}
		break;
	case actionQuitSyn:
		{
			ASSERT(!"actionQuitSyn");
		}
		break;
	case actionWalk:
	case actionRun:
		{
			//pUser->SetPos(m_unPosX, m_unPosY);
		}
		break;
	case actionEnterMap:			// 1
		{
			CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
			if(pAgent)
			{
				OBJID idMap = m_idUser;			//? idUser is idMap
				Cast<CRoleMove>(pAgent)->EnterMap(idMap, m_unPosX, m_unPosY);

				CMsgAction	msg;
				IF_OK(msg.Create(pAgent->GetID(), pAgent->GetPosX(), pAgent->GetPosY(), pAgent->GetDir(), actionGetItemSet))
					NpcWorld()->SendMsg(idNpc, &msg);//SendMsg(&msg);
			}
		}
		break;
	case actionGetItemSet:			// 2
		{
			CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
			if(pAgent)
			{
				CMsgAction	msg;
				IF_OK(msg.Create(pAgent->GetID(), pAgent->GetPosX(), pAgent->GetPosY(), pAgent->GetDir(), actionGetGoodFriend))
					NpcWorld()->SendMsg(idNpc, &msg);//SendMsg(&msg);
			}
		}
		break;
	case actionGetGoodFriend:			// 3
		{
			CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
			if(pAgent)
			{
				CMsgAction	msg;
				IF_OK(msg.Create(pAgent->GetID(), pAgent->GetPosX(), pAgent->GetPosY(), pAgent->GetDir(), actionGetWeaponSkillSet))
					NpcWorld()->SendMsg(idNpc, &msg);//SendMsg(&msg);
			}
		}
		break;
	case actionGetWeaponSkillSet:			// 4
		{
			CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
			if(pAgent)
			{
				CMsgAction	msg;
				IF_OK(msg.Create(pAgent->GetID(), pAgent->GetPosX(), pAgent->GetPosY(), pAgent->GetDir(), actionGetMagicSet))
					NpcWorld()->SendMsg(idNpc, &msg);//SendMsg(&msg);
			}
		}
		break;
	case actionGetMagicSet:			// 5
		{
			CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
			if(pAgent)
			{
				CMsgAction	msg;
				IF_OK(msg.Create(pAgent->GetID(), pAgent->GetPosX(), pAgent->GetPosY(), pAgent->GetDir(), actionGetSynAttr))
					NpcWorld()->SendMsg(idNpc, &msg);//SendMsg(&msg);
			}
		}
		break;
	case actionGetSynAttr:			// 6
		{
			CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
			if(pAgent)
				pAgent->LoginOK();
		}
		break;
	case actionForward:
		{
			/*/ 测试 PALED_DEBUG
			{
				char	szText[1024];
				sprintf(szText, "MsgAction: forword, dir[%d], x[%d], y[%d]", m_unDir, m_unPosX, m_unPosY);
				CMsgTalk msg;
				if(msg.Create("测试", "测试", szText))
					pUser->SendMsg(&msg);
			}
			//*/
#ifdef	FW_ENABLE
			CUser* pUser	= UserManager()->QueryUser(m_idUser);
			if (pUser)
				pUser->MoveForward(m_unDir);		// return true: 是n步模数
#endif
		}
		break;
	case actionJump:
		{
			CUser* pUser	= UserManager()->QueryUser(m_idUser);
			if (pUser)
				pUser->JumpPos(m_usTargetPosX, m_usTargetPosY, m_unDir);
		}
		break;
	case actionLeaveMap:
		{
			if(IsSceneID(m_idUser))
			{
				;
			}
			else if(IsNpcID(m_idUser))
			{
				NpcManager()->QuerySet()->DelObj(m_idUser);
			}
			else
			{
				UserManager()->QuerySet()->DelObj(m_idUser);
			}
		}
		break;
	case actionEquip:
	case actionUnequip:
	case actionUplev:
		{
		}
		break;
	case actionSynchro:
		{
			CUser* pUser	= UserManager()->QueryUser(m_idUser);
			if(pUser)
			{
				IF_NOT(pUser->GetMap()->IsValidPoint(m_usTargetPosX, m_usTargetPosY))
					return ;

				pUser->SetPos(m_usTargetPosX, m_usTargetPosY);
			}
			else if(idNpc == m_idUser)		// send to one self
			{
				INpc* pNpc = NpcManager()->QueryNpc(m_idUser);
				if(pNpc)
				{
					IF_NOT(pNpc->GetMap()->IsValidPoint(m_usTargetPosX, m_usTargetPosY))
						return ;

					pNpc->SynchroPos(m_usTargetPosX, m_usTargetPosY, m_usTargetPosX, m_usTargetPosY);
				}
			}
		}
		break;
	case	actionKickBack:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idUser);
			if(pNpc)
			{
				IF_NOT(pNpc->GetMap()->IsValidPoint(m_unPosX, m_unPosY))
					return ;

				pNpc->KickBack(m_unPosX, m_unPosY);
			}
		}
		break;
	case	actionMapStatus:
		{
			OBJID idMap = m_idUser;
			int nStatus = m_iData;
			CGameMap* pMap = MapManager()->QueryMap(idMap);
			IF_OK(pMap)
				pMap->SetStatus(nStatus);
		}
		break;
	case	actionMoveStop:
		{
			INpc* pNpc = NpcManager()->QueryNpc(idNpc);
			if(pNpc && Cast<CNpc>(pNpc))
			{
				Cast<CNpc>(pNpc)->LockMove(m_dwData);
			}
		}
		break;
	case	actionChangeMapDoc:
		{
			OBJID idMap = m_idUser;
			OBJID idDoc = m_dwData;
			CGameMap* pMap = MapManager()->QueryMap(idMap);
			IF_OK(pMap)
			{
				pMap->ChangeMapDoc(idDoc);
			}
		}
		break;
	case	actionAddTerrainObj:
		{
			OBJID idMap = m_idUser;
			OBJID idOwner = m_dwData;
			OBJID idTerrainObj = idNpc;
			CGameMap* pMap = MapManager()->QueryMap(idMap);
			IF_OK(pMap)
			{
				pMap->AddTerrainObj(idOwner, m_unPosX, m_unPosY, idTerrainObj);
			}
		}
		break;
	case	actionDelTerrainObj:
		{
			OBJID idMap = m_idUser;
			OBJID idOwner = m_dwData;
			CGameMap* pMap = MapManager()->QueryMap(idMap);
			IF_OK(pMap)
			{
				pMap->DelTerrainObj(idOwner);
			}
		}
		break;
	case	actionLockUser:
		{
			INpc* pNpc = NpcManager()->QueryNpc(idNpc);
			if(pNpc && Cast<CNpc>(pNpc))
			{
				pNpc->Lock(true);
			}
		}
		break;
	case	actionUnlockUser:
		{
			INpc* pNpc = NpcManager()->QueryNpc(idNpc);
			if(pNpc && Cast<CNpc>(pNpc))
			{
				pNpc->Lock(false);
			}
		}
		break;
	case	actionMagicTrack:
		{
			INpc* pNpc = NpcManager()->QueryNpc(m_idUser);
			if(pNpc && Cast<CNpc>(pNpc))
			{
				Cast<CNpc>(pNpc)->SetPos(m_unPosX, m_unPosY);
				Cast<CNpc>(pNpc)->SetDir(m_unDir);
			}
		}
		break;
	default:
//		ASSERT(!"switch");
		break;
	}
}