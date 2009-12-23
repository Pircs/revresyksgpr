#include "AllMsg.h"
#include "MapGroup.h"

bool CMsgInteract::Create(USHORT unType, OBJID idSender, OBJID idTarget, USHORT unPosX, USHORT unPosY, USHORT usMagicType, USHORT usMagicLev)
{
	DEBUG_CREATEMSG("Interact",idSender,unType,"",idTarget,dwData)
	CHECKF(unType && idSender);		// && idTarget

	m_unType			= unType;
	m_idSender		= idSender;
	m_idTarget		= idTarget;
	m_unPosX			= unPosX;
	m_unPosY			= unPosY;

	m_usMagicType	= usMagicType;
	m_usMagicLevel	= usMagicLev;
	return true;
}

void CMsgInteract::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{
	DEBUG_PROCESSMSG("Interact",m_idSender,m_unType,"",m_idTarget,m_dwData)

	IRole* pRole = mapGroup.GetRoleManager()->QueryRole(idSocket, idNpc, m_idSender);
	if(!pRole)
		return;
	
//	if (m_unType == INTERACT_ATTACK || m_unType == INTERACT_SHOOT
//		|| m_unType == INTERACT_RUSHATK || m_unType == INTERACT_MAGICATTACK)
	{
		if (pRole->QueryStatus(STATUS_LOCK) || pRole->QueryStatus(STATUS_FREEZE)
			|| pRole->QueryStatus(STATUS_FAINT))
		{
#ifdef ZLONG_DEBUG
			pRole->SendSysMsg("Debug: 不可交互");
#endif
			return;
		}
	}
	
	//ASSERT(m_idSender == pRole->GetID());
//		m_idSender = pRole->GetID();

	IRole* pTarget = mapGroup.GetRoleManager()->QueryRole(m_idTarget);
	if(!pTarget && INTERACT_MAGICATTACK != m_unType)
		return;

	CUser* pUser = NULL;
	pRole->QueryObj(OBJ_USER, IPP_OF(pUser));

	if(pUser && !pUser->IsAlive())
	{
		pUser->SendSysMsg(STR_DIE);
		return ;
	}

	if (pUser)
		pUser->SetPose(_ACTION_STANDBY);

	// fill id
	m_idSender	= pRole->GetID();

	// stop fight
	pRole->ClrAttackTarget();
	// stop mine
	if (pUser)
		pUser->StopMine();

	DEBUG_TRY	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
	switch(m_unType)
	{
	case	INTERACT_ATTACK:
	case	INTERACT_SHOOT:
		{

			if (pUser)
			{
				if (!pUser->SynPosition(m_unPosX, m_unPosY))
				{
					mapGroup.GetUserManager()->KickOutSocket(idSocket, "CMsgInteract SynPosition");
					return;
				}

				if (pUser->GetMap()->IsNewbieMap())
				{
					pUser->SendSysMsg(STR_NO_PK_VILLAGE);
					return;
				}

				if (pUser->QueryMagic()->IsInLaunch())
				{
#ifdef _DEBUGx
					pUser->SendSysMsg("DEBUG: magic in launch!");
#endif
					return;
				}
				if (pUser->QueryMagic()->IsIntone())
				{
					pUser->QueryMagic()->AbortMagic();
				}
			}

			pRole->QueryStatusSet()->DelObj(STATUS_LURKER);
			if(pUser)
				pUser->ProcessOnAttack();
			pRole->SetAttackTarget(pTarget);
			if(pUser)
				pUser->ProcessAutoAttack();
		}
		break;

	case	INTERACT_RUSHATK:
		{
			if (pUser)
			{
				LOGCHEAT("rush attack: user[%s][%u]", pUser->GetName(), pUser->GetAccountID());

				if (!pUser->SynPosition(m_unPosX, m_unPosY)
						|| pUser->GetDistance(m_usData0, m_usData1) >= 2*CELLS_PER_BLOCK)
				{
					pUser->SendSysMsg(STR_INVALID_MSG);
					mapGroup.GetUserManager()->KickOutSocket(idSocket, "RUSHATK 超远");
					return;
				}

				if (pUser->QueryMagic()->IsInLaunch())
				{
#ifdef _DEBUGx
					pUser->SendSysMsg("DEBUG: magic in launch!");
#endif
					return;
				}
				if (pUser->QueryMagic()->IsIntone())
				{
					pUser->QueryMagic()->AbortMagic();
				}

				pUser->BroadcastRoomMsg(this, INCLUDE_SELF);

				IRole* pTarget = pUser->FindAroundRole(m_idTarget);
				if (!pTarget)
					return;
				
				pUser->QueryStatusSet()->DelObj(STATUS_LURKER);

				pUser->ProcessOnAttack();
				pUser->SetAttackTarget(pTarget);
				pUser->ProcessAutoAttack();
			}
		}
		break;

	case	INTERACT_MAGICATTACK:
		{
			if (pUser)
			{
				if (pUser->GetMap()->IsNewbieMap())
					pUser->SendSysMsg(STR_NO_PK_VILLAGE);

				if (pUser->QueryMagic()->IsInLaunch())
				{
#ifdef _DEBUGx
					pUser->SendSysMsg("DEBUG: magic in launch!");
#endif
					return;
				}
				if (pUser->QueryMagic()->IsIntone())
				{
					pUser->QueryMagic()->AbortMagic();
				}

				pUser->ResetEnergyInc();
			}
			pRole->QueryStatusSet()->DelObj(STATUS_LURKER);

			// decode
			if (idSocket != SOCKET_NONE)//!this->IsNpcMsg())
			{
				if(!pUser->CheckTimeStamp(m_dwTimeStamp))
					return ;

				bool bTestMagic = false;
				switch(m_usData0)
				{
				case 1000:
				case 1001:
				case 1002:
				case 1030:
				case 1125:
				case 1150:
				case 1165:
				case 1160:
					m_usData1 ^= 0x6279;
					bTestMagic	= true;
					break;

				default:
					break;
				}

				USHORT usLev  = (m_usData1^0x3721)&0xff;
				USHORT usData = (m_usData1^0x3721)>>8;
				if (usData != (m_dwTimeStamp%0x100))
				{
					mapGroup.GetUserManager()->KickOutSocket(idSocket, "外挂！");
					return;
				}

				m_usData1 = usLev;

//* DECODE
////
#define	ENCODE_MAGICATTACK(idUser,usType,idTarget,usPosX,usPosY) {	\
				usType		= (::ExchangeShortBits((usType - 0x14BE),3) ^ (idUser) ^ 0x915D);	\
				idTarget	= ::ExchangeLongBits(((idTarget - 0x8B90B51A) ^ (idUser) ^ 0x5F2D2463),32-13); \
				usPosX		= (::ExchangeShortBits((usPosX - 0xDD12),1) ^ (idUser) ^ 0x2ED6);	\
				usPosY		= (::ExchangeShortBits((usPosY - 0x76DE),5) ^ (idUser) ^ 0xB99B);	}
#define	DECODE_MAGICATTACK(idUser,usType,idTarget,usPosX,usPosY) {	\
				usType		= 0xFFFF&(::ExchangeShortBits(((usType) ^ (idUser) ^ 0x915D),16-3) + 0x14BE);	\
				idTarget	= (::ExchangeLongBits((idTarget),13) ^ (idUser) ^ 0x5F2D2463) + 0x8B90B51A; \
				usPosX		= 0xFFFF&(::ExchangeShortBits(((usPosX) ^ (idUser) ^ 0x2ED6),16-1) + 0xDD12);	\
				usPosY		= 0xFFFF&(::ExchangeShortBits(((usPosY) ^ (idUser) ^ 0xB99B),16-5) + 0x76DE);	}
////
				if(idSocket != SOCKET_NONE)//!IsNpcMsg())
					DECODE_MAGICATTACK(pUser->GetID(), m_usMagicType, m_idTarget, m_unPosX, m_unPosY)
//*/// DECODE

				if (pUser)
				{
					if (bTestMagic && pUser->IsMagicAtkCheat(m_idTarget, m_unPosX, m_unPosY, m_dwTimeStamp))
					{
						::MyLogSave("gmlog/cheater", "Cheater found: %s[%d]", 
									pUser->GetName(), 
									pUser->GetID());

						if (1 == ::RandGet(8))
						{
							pUser->DoCheaterPunish();
							//UserManager()->KickOutSocket(m_idSocket, "cheat found!");
							return;
						}
					}
					if (pUser->QueryMagic())
					{
						// 
						IMagicData* pMagicData = pUser->QueryMagic()->FindMagic(m_usMagicType);
						if (pMagicData)
						{
							if ((pMagicData->GetInt(MAGICDATA_AUTO_ACTIVE) & AUTOACTIVE_RANDOM) != 0)
							{
								// 暂时不做惩罚
								//pUser->DoCheaterPunish();
#ifdef _DEBUG
								pUser->SendSysMsg("被动技能不能主动触发！");
#endif
								return;
							}
						}
					}
				}
			}

			if(pUser)
				pUser->ProcessOnAttack();
			pRole->MagicAttack(m_usMagicType, m_idTarget, m_unPosX, m_unPosY);	// 在内部广播
		}
		break;

	case	INTERACT_ABORTMAGIC:
		{
			if(pUser)
			{
				if(pUser->QueryMagic()->AbortMagic())
					pUser->BroadcastRoomMsg(this, INCLUDE_SELF);
			}
		}
		break;

	case	INTERACT_COURT:
		{
			if (!pUser)
				break;

			pUser->Court(m_idTarget);
		}
		break;

	case	INTERACT_MARRY:
		{			
			if (!pUser)
				break;

			pUser->Marry(m_idTarget);
		}
		break;

	case	INTERACT_DIVORCE:
		{
			if (!pUser)
				break;
		}
		break;
		
	case	INTERACT_MINE:
		{
			if(pUser)
			{
				if (!pUser->SynPosition(m_unPosX, m_unPosY))
				{
					mapGroup.GetUserManager()->KickOutSocket(idSocket, "CMsgInteract SynPosition");
					return;
				}

				pUser->Mine(pTarget);
			}
		}
		break;

	default:
		{
			LOGERROR("INVALID MSG INTERACT TYPE %d", m_unType);
		}
	} // switch
	DEBUG_CATCH2("switch(MSGINTERACT) [%d]", m_unType)		// AAAAAAAAAAAAAAAA

	return;

#ifdef _MSGDEBUG
	::LogMsg("Process CMsgInteract, id:%u", m_id);
#endif
}