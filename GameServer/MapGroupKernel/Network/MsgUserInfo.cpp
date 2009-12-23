#include "AllMsg.h"
#include "UserData.h"
#pragma	warning(disable:4786)

bool CMsgUserInfo::Create(CUser* pUser)
{
	CHECKF(pUser);
	const UserInfoStruct* pInfo = pUser->GetInfo();

	m_id				= pInfo->id;
	m_dwLookFace		= pInfo->dwLookface;
	m_usHair			= pInfo->dwHair;
	m_cLength			= pInfo->cLength;
	m_cFat				= pInfo->cFat;
	m_dwMoney			= pInfo->dwMoney;
	m_nExp				= pInfo->nExp;

	m_nTutorExp			= pInfo->nTutorExp;
	m_dwMercenaryExp	= pInfo->dwMercenaryExp;

	m_nPotential		= pInfo->nPotential;
	m_usForce			= pInfo->usForce;
	m_usConstitution	= pInfo->usConstitution;	// zlong 2004-1-30
	m_usDexterity		= pInfo->usDexterity;
	m_usSpeed			= pInfo->usSpeed;
	m_usHealth			= pInfo->usHealth;
	m_usSoul			= pInfo->usSoul;

	m_usAdditionalPoint	= pInfo->usAdditional_point;

	m_usLife			= pInfo->usLife;
	m_usMaxLife			= pUser->GetMaxLife();

	m_usMana			= pInfo->usMana;
	m_sPk				= pInfo->sPk;
	m_ucLevel			= pInfo->ucLevel;
	m_ucProfession		= pInfo->ucProfession;

	m_ucNobility		= pInfo->ucNobility;
	m_ucMetempsychosis	= pInfo->ucMetempsychosis;
	m_ucAutoAllot		= pInfo->ucAuto_allot;
	m_ucTutorLevel		= pInfo->ucTutorLevel;

	m_ucMercenaryRank	= pInfo->ucMercenaryRank;
	m_ucNobilityRank	= pInfo->ucNobilityRank;
	m_usMaxEudemon		= pUser->GetMedalSelect();
	
	m_dwExploit			= pInfo->dwExploit;

	bool bSucMake	=true;
	bSucMake	&=m_StrPacker.AddString((char*)pInfo->szName);
	bSucMake	&=m_StrPacker.AddString((char*)pInfo->szMate);
	UPDATE_MSG_SIZE;
	return true;
}
