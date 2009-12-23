#include "AllMsg.h"
#include "mapgroup.h"

void CMsgTick::Process(CUser* pUser)
{
	if(pUser)
	{
		/*
		// cheat test!
		DWORD dwMsgCount = m_dwChkData^pUser->GetID();
		if (pUser->m_dwMsgCount > dwMsgCount 
				|| pUser->m_dwMsgCount+16 < dwMsgCount)	// cheater found!
		{	
			if (::RandGet(8) == 1)
			{
				pUser->DoCheaterPunish();

				::MyLogSave("gmlog/cheater", "Invalid msg Cheater found: %s[%d]", 
							pUser->GetName(), 
							pUser->GetID());
			}
		}
		*/

		// tick process
		DWORD dwTime = m_dwData ^ (m_idPlayer*m_idPlayer+9527);
		pUser->ProcessTick(dwTime);
	}
}

