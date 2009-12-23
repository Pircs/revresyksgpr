#include "AllMsg.h"
#include "npcworld.h"

void CMsgWalk::Process()
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgWalk, idUser:%u, data:%u",
					m_idUser,
					m_dwData);
#endif

	if(IsSceneID(m_idUser))
	{
		ASSERT(!"CMsgWalk");
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
			pUser->MoveForward(m_ucDir);

			// run mode step 2
			if (m_ucMode >= WALKMODE_RUN_DIR0 && m_ucMode <= WALKMODE_RUN_DIR7)
				pUser->MoveForward(m_ucMode - WALKMODE_RUN_DIR0);

#ifdef	PALED_DEBUG_X
			MSGBUF	szMsg;
			sprintf(szMsg, "MSGWALK£ºÍæ¼Ò£¬%s(%d,%d)", pUser->GetName(), pUser->GetPosX(), pUser->GetPosY());
			LOGWARNING(szMsg);
#endif
		}
		else
		{
#ifdef	PALED_DEBUG_X
			MSGBUF	szMsg;
			sprintf(szMsg, "MSGWALK£ºAGENT£¬%d(%d)", m_idUser, m_ucDir);
			LOGWARNING(szMsg);
#endif
		}
	}
}
