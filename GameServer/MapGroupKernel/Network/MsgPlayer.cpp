#include "AllMsg.h"
#include "mapgroup.h"
//#include "UserData.h"
#include "User.h"
#include "AiNpc.h"
//#include "Booth.h"
#include "Npc.h"

const int	STATUARY_LIFE_SCALE					= 100;			//? 补丁：雕像下传LIFE的比例


bool CMsgPlayer::Create(IRole* pRole)
{
	DEBUG_CREATEMSG("CMsgPlayer",pRole->GetID(),pRole->IsUser(),"",pRole->GetPosX(),pRole->GetPosY())
	// param check
	if (!pRole)
		return false;

	// fill info now
	m_id			= pRole->GetID();
	//m_i64Status	= pRole->GetEffect();
	m_dwStatus[0]	= pRole->GetEffect() & 0xFFFFFFFF;
	m_dwStatus[1]	= (pRole->GetEffect() >> 32) & 0xFFFFFFFF;
	m_dwLookFace	= pRole->GetLookFace();
	m_usHair		= (USHORT)pRole->GetHair();

	m_cLength	= pRole->GetLength();
	m_cFat		= pRole->GetFat();
	
	m_dwArmorType	= pRole->GetArmorTypeID();
	m_dwWeaponRType	= pRole->GetWeaponRTypeID();
	m_dwMountType	= pRole->GetMountTypeID();
	
	CMonster* pMonster = NULL;
	if (pRole->QueryObj(OBJ_MONSTER, IPP_OF(pMonster)) && pMonster->IsEudemon() && pMonster->QueryOwnerUser())
		m_idOwner	= pMonster->QueryOwnerUser()->GetID();
	else
		m_dwSynID_Rank	= (pRole->GetSynRankShow()<<MASK_RANK_SHIFT) | (pRole->GetSynID()&MASK_SYNID);

	CUser* pUser = NULL;
	if (pRole->QueryObj(OBJ_USER, IPP_OF(pUser)))
	{
		m_dwMantleType		= pUser->GetMantleTypeID();
		
		m_ucActionSpeed		= pUser->AdjustSpeed(pUser->GetSpeed());
		m_ucTutorLevel		= pUser->GetTutorLevel();
		m_ucMercenaryRank	= pUser->GetMercenaryRank();
		m_ucNobilityRank		= pUser->GetNobilityRank();
	}
	else
	{
		m_usMaxLife		= pRole->GetMaxLife();
		m_usMonsterLife	= pRole->GetLife();

		if (pMonster)
			m_idMonsterType	= pMonster->GetType();
	}

	m_usPosX		= pRole->GetPosX();
	m_usPosY		= pRole->GetPosY();
	m_ucDir		= pRole->GetDir();
	m_ucPose		= pRole->GetPose();

	if (IsSceneID(pRole->GetID()))
	{
		CNpc* pNpc;
		IF_OK(pRole->QueryObj(OBJ_NPC, IPP_OF(pNpc)))
		{
			if(pNpc->GetType()==_STATUARY_NPC)
			{
				m_usStatuaryLife	 = pRole->GetLife() / STATUARY_LIFE_SCALE;
				m_usStatuaryFrame = pNpc->GetInt(STATUARYDATA_FRAME);
			}
			else
			{
				m_usLife	 = pRole->GetLife();
				m_usLevel = pNpc->GetInt(NPCDATA_MAXLIFE);
			}
		}
	}
	else if (IsNpcID(pRole->GetID()))
	{
		m_usLife	 = pRole->GetLife();
		m_usLevel = pRole->GetLev();
	}
/*	else
	{
		CUser*	pUser;
		if (pRole->QueryObj(OBJ_USER, IPP_OF(pUser)))
		{
			m_ucSprite	= pUser->GetSpriteFace();
		}
	}*/

	bool bSucMake	=true;
	bSucMake	&=m_StrPacker.AddString((char*)pRole->GetName());
	UPDATE_MSG_SIZE;
	return true;
}

/*
bool CMsgPlayer::Create((CBooth* pBooth))
{
	// param check
	if (idPlayer == ID_NONE)
		return false;

	// init
	this->Init();

	// fill info now
	m_id					= pBooth->GetID();
	m_dwLookFace			= pBooty->GetLook();
	m_dwEffect			= 0;
	m_dwSynID_Rank		= 0;

	m_dwHelmetType		= pInfo->ucFace;
	m_dwArmorType		= pInfo->dwExp;
	m_dwWeaponRType		= pInfo->nRepute;
	m_dwWeaponLType		= pInfo->nRepute;
	m_dwMountType		= pInfo->ucRank;

	m_dwMedalSelect		= 0;

	m_usPosX				= pBooth->GetPosX();
	m_usPosY				= pBooth->GetPosY();
	m_usHair				= 0;
	m_ucDir				= pBooth->GetDir();
	m_ucPose				= 0;

	if(!m_StrPacker.AddString(pBooth->GetOwnerName())
		return false;
	UPDATE_MSG_SIZE;
	return true;
}*/

void CMsgPlayer::Process(CUser* pUser)
{
	return;
//?????????????????????????
	pUser->BroadcastRoomMsg(this, EXCLUDE_SELF);

#ifdef _MSGDEBUG
	::LogMsg("Process CMsgPlayer, id:%u", m_id);
#endif

	char szName[_MAX_NAMESIZE]		="";
	char szMate[_MAX_NAMESIZE]		="";
	char szSyndicate[_MAX_NAMESIZE]	="";
	char szTitle[_MAX_NAMESIZE]		="";

	m_StrPacker.GetString(0, szName, _MAX_NAMESIZE);
	m_StrPacker.GetString(1, szMate, _MAX_NAMESIZE);
	m_StrPacker.GetString(2, szSyndicate, _MAX_NAMESIZE);
	m_StrPacker.GetString(3, szTitle, _MAX_NAMESIZE);
}
