#include "AllMsg.h"
#include "SynAttr.h"

bool CMsgSynMemberInfo::Create(CUser* pUser)
{
	CHECKF(pUser);
	
	// fill info now
	SafeCopy(m_szName, pUser->GetName(), _MAX_NAMESIZE);
	SafeCopy(m_szMate, pUser->GetMate(), _MAX_NAMESIZE);

	if (pUser->QuerySynAttr())
	{
		m_ucRank			= pUser->QuerySynAttr()->GetSynRankShow();
		m_nProffer			= pUser->QuerySynAttr()->GetProffer();
		m_sPk				= pUser->GetPk();
		m_ucProfession		= pUser->GetProfession();
		m_ucNobilityRank	= pUser->GetNobilityRank();
		m_ucLevel			= pUser->GetLev();
		m_dwLookFace		= pUser->GetLookFace();
	}
	return true;
}

void CMsgSynMemberInfo::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{
	CUser* pUser = mapGroup.GetUserManager()->GetUser(idSocket, idNpc);
	if(!pUser)
	{
		// TransmitMsg
		CUser* pTargetUser = mapGroup.GetUserManager()->GetUser(m_szName);
		if(pTargetUser)
		{
			CHECK(pTargetUser->GetSynID() != ID_NONE);

			CMsgSynMemberInfo msg;
			IF_OK(msg.Create(pTargetUser))
			{
				if(idSocket!=SOCKET_NONE)
					mapGroup.QueryIntraMsg()->SendClientMsg(idSocket, &msg);
				else
					mapGroup.QueryIntraMsg()->SendNpcMsg(idNpc, &msg);
			}//	SendMsg(&msg);
		}
		return ;
	}

	if(pUser->GetSynID() == ID_NONE)
		return ;

	CUser* pTargetUser = mapGroup.GetUserManager()->GetUser(m_szName);
	if(pTargetUser)
	{
		CHECK(pTargetUser->GetSynID() != ID_NONE);

		CMsgSynMemberInfo msg;
		IF_OK(msg.Create(pTargetUser))
		{
			if(idSocket!=SOCKET_NONE)
				mapGroup.QueryIntraMsg()->SendClientMsg(idSocket, &msg);
			else
				mapGroup.QueryIntraMsg()->SendNpcMsg(idNpc, &msg);
		}
			//SendMsg(&msg);
	}
	else
	{
		mapGroup.QueryIntraMsg()->TransmitMsg(this, idSocket, idNpc);
	}
}