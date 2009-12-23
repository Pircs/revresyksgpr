#include "allmsg.h"
#include "team.h"
#include "mapgroup.h"
#include <memory.h>

bool CMsgTeamMember::Create(CUser* pMember)
{
	if (!pMember)
		return false;

	CTeam* pTeam	= pMember->GetTeam();
	if (!pTeam)
		return false;

	TeamMemberInfo info;
	memset(&info, 0L, sizeof(TeamMemberInfo));
	info.id			= pMember->GetID();
	info.dwLookFace	= pMember->GetLookFace();
	info.usHp		= pMember->GetLife();
	info.usMaxHp	= pMember->GetMaxLife();
	strcpy(info.szName, pMember->GetName());


	// fill info now
	m_unMsgSize	= sizeof(*this) - sizeof(TeamMemberInfo)*(_MAX_TEAMMEMBER-1);

	m_ucAction	= _MSG_TEAMMEMBER_ADDMEMBER;
	m_ucAmount	= 1;
	memcpy(m_setMember, &info, sizeof(TeamMemberInfo));
	return true;
}