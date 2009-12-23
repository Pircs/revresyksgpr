#include "allmsg.h"
#include "team.h"
#include "npcWorld.h"
#include "Agent.h"

void CMsgTeamMember::Process(OBJID idNpc)
{
	CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
	if(!pAgent)
		return ;

	Cast<CTeam>(pAgent)->AddMember(m_ucAmount, m_setMember);
}