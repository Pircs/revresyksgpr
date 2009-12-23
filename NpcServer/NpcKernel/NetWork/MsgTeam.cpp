#include "allmsg.h"
#include "npcWorld.h"
#include "Agent.h"

void CMsgTeam::Process(OBJID idNpc)
{
	CAgent* pAgent	= NpcManager()->QueryAgent(idNpc);
	if (!pAgent)
		return;
	CTeam* pTeam = Cast<CTeam>(pAgent);

	switch(m_unAction)
	{
	case _MSGTEAM_APPLYJOIN:
	case _MSGTEAM_INVITE:
		{
			pTeam->Apply(m_idPlayer);
		}
		break;
	case _MSGTEAM_LEAVE:
	case _MSGTEAM_KICKOUT:
		{
			if(m_idPlayer == pAgent->GetID())
				pTeam->Dismiss(m_idPlayer);
			else
				pTeam->DelMember(m_idPlayer);
		}
		break;
	case _MSGTEAM_DISMISS:
		{
			pTeam->Dismiss(m_idPlayer);
		}
		break;
	}	
}