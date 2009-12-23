#include "AllMsg.h"
#include "MapGroup.h"
#include "UserManager.h"

bool CMsgSchoolMember::Create(uint8 ucAction, ST_MEMBERINFO* pMember, uint8 ucAmount)
{
	m_ucAction	= ucAction;
	if (pMember)
	{
		for (int i=0; i<ucAmount; ++i)
		{
			m_setMember.Append(pMember[i]);
		}
	}
	m_unMsgSize	=sizeof(*this) - m_setMember.GetFreeBufferSize();
	return true;
}

bool CMsgSchoolMember::Append(const ST_MEMBERINFO& Member)
{
	m_setMember.Append(Member);
	m_unMsgSize	=sizeof(*this) - m_setMember.GetFreeBufferSize();
	return true;
}

bool CMsgSchoolMember::Append(uint8 ucRelation, uint8 ucStatus, OBJID idMember, const char* pszName)
{
	ST_MEMBERINFO Member;
	memset(&Member, 0L, sizeof(ST_MEMBERINFO));
	Member.ucRelation	= ucRelation;
	Member.ucStatus		= ucStatus;
	Member.idMember		= idMember;
	if (pszName)
		::SafeCopy(Member.szName, pszName, MAX_NAMESIZE);

	Append(Member);
	return true;
}

bool CMsgSchoolMember::Append(uint8 ucRelation, uint8 ucStatus, CUser* pUser)
{
	CHECKF (pUser);

	ST_MEMBERINFO Member;
	memset(&Member, 0L, sizeof(ST_MEMBERINFO));
	Member.ucRelation	= ucRelation;
	Member.ucStatus		= ucStatus;
	Member.idMember		= pUser->GetID();

	::SafeCopy(Member.szName, pUser->GetName(), MAX_NAMESIZE);
	::SafeCopy(Member.szMate, pUser->GetMate(), MAX_NAMESIZE);

	Member.ucLevel			= pUser->GetLev();
	Member.ucProfession		= pUser->GetProfession();
	Member.usPkValue		= pUser->GetPk();
	Member.ucNobilityRank	= pUser->GetNobilityRank();
	Member.dwSynID_Rank		= (pUser->GetSynRankShow()<<MASK_RANK_SHIFT) | (pUser->GetSynID()&MASK_SYNID);

	Append(Member);
	return true;
}

void CMsgSchoolMember::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgSchoolMember");
#endif

	CUser* pUser = mapGroup.GetUserManager()->GetUser(idSocket, idNpc);
	if (!pUser)
	{
		if(idSocket == SOCKET_NONE)//IsNpcMsg()
			return;

		// TransmitMsg
		switch(m_ucAction)
		{
		case MESSAGEBOARD_DELMEMBER:
			{
			}
			break;
		case MESSAGEBOARD_ADDMEMBER:
			{
				CMsgSchoolMember msg;
				msg.Create(MESSAGEBOARD_ADDMEMBER, NULL, 0);
				for (int i=0; i<m_setMember.GetSize(); i++)
				{
					CUser* pTarget = mapGroup.GetUserManager()->GetUser(m_setMember[i].idMember);
					if (pTarget)
						msg.Append(m_setMember[i].ucRelation, MEMBERSTATUS_ONLINE, pTarget);
				}
				mapGroup.QueryIntraMsg()->SendClientMsg(idSocket, &msg);
				//SendMsg(&msg);
			}
			break;
		case MESSAGEBOARD_UPDATEMEMBER:
			{
			}
			break;
		}
	}
}
