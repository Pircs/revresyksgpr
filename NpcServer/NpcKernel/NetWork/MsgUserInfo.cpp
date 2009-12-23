#include "AllMsg.h"
#include "agent.h"
#include "NPCWORLD.h"
#pragma	warning(disable:4786)

void CMsgUserInfo::Process()
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgUserInfo, id:%u", m_id);
#endif

	UserInfoStruct	info;
	memset(&info, 0, sizeof(UserInfoStruct));

	m_StrPacker.GetString(0, info.szName, _MAX_NAMESIZE);
	m_StrPacker.GetString(1, info.szMate, _MAX_NAMESIZE);
	IF_NOT(strlen(info.szName) > 0)
		return ;

	info.id					= m_id;
	info.dwLookface			= m_dwLookFace;
	info.dwHair				= m_usHair;
	info.dwMoney			= m_dwMoney;
	info.nExp				= m_nExp;

//	info.nTutorExp			= m_nTutorExp;
//	info.dwMercenaryExp		= m_dwMercenaryExp;

	info.nPotential			= m_nPotential;
	info.usForce			= m_usForce;
	info.usConstitution		= m_usConstitution;
	info.usDexterity		= m_usDexterity;
	info.usSpeed			= m_usSpeed;
	info.usHealth			= m_usHealth;
	info.usSoul				= m_usSoul;

	info.usAdditional_point	= m_usAdditionalPoint;

	info.usLife				= m_usLife;
	info.usMaxLife			= m_usMaxLife;
	info.usMana				= m_usMana;
	info.sPk				= m_sPk;

	info.ucLevel			= m_ucLevel;
	info.ucProfession		= m_ucProfession;
	info.ucNobility			= m_ucNobility;
	info.ucMetempsychosis	= m_ucMetempsychosis;
	info.ucAuto_allot		= m_ucAutoAllot;

//	info.ucTutorLevel		= m_ucTutorLevel;
//	info.ucMercenaryRank	= m_ucMercenaryRank;

//	info.ucNobilityRank		= m_ucNobilityRank;
//	info.dwExploit			= m_dwExploit;

	INpc* pNpc = NpcManager()->QueryNpc(info.id);
	CAgent*	pAgent = Cast<CAgent>(pNpc);
	IF_OK(pAgent)
		pAgent->AppendInfo(info);
}
