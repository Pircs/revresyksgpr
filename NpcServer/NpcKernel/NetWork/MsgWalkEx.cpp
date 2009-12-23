#include "AllMsg.h"
#include "npcworld.h"

void CMsgWalkEx::Process()
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgWalkEx, idUser:%u, dir:%u, mode:%u",
		m_idUser, m_ucDir, m_ucMode);
#endif
	
	if(IsSceneID(m_idUser))
	{
		ASSERT(!"CMsgWalkEx");
	}
	else if(IsNpcID(m_idUser))
	{
		//		INpc* pNpc = NpcManager()->QueryNpc(m_idUser);
		//		if(pNpc)
		//			pNpc->MoveForward(m_ucDir);
	}
	else
	{
		CUser* pUser = UserManager()->QueryUser(m_idUser);
		if(pUser)
		{
			pUser->SetPos(m_usPosX, m_usPosY);
		}
	}
}