#include "AllMsg.h"
#include "mapgroup.h"

bool CMsgAnnounceList::Create(USHORT usType, USHORT usAmount, ST_ANNOUNCE_TITLE* setAnnounce)
{
	if(usAmount > 0)
	    CHECKF (setAnnounce);

	m_unMsgType	= _MSG_ANNOUNCE_LIST;
	m_usType			= usType;
    for (int i=0; i<usAmount; i++)
	{
		m_setAnnounce.Append(setAnnounce[i]);
	}
	m_unMsgSize	= sizeof(*this)-m_setAnnounce.GetFreeBufferSize();	
	return true;
}

bool CMsgAnnounceList::Append(OBJID idAnnounce, const char* pszTitle,int type)
{
	CHECKF (pszTitle);
	ST_ANNOUNCE_TITLE Announce;
	Announce.idAnnounce	= idAnnounce;
	::SafeCopy(Announce.szName, pszTitle, 32);
	m_setAnnounce.Append(Announce);
	m_unMsgSize	= sizeof(*this)-m_setAnnounce.GetFreeBufferSize();	
	return true;
}

void CMsgAnnounceList::Process(CMapGroup& mapGroup, CUser* pUser)
{
#ifdef _MYDEBUG
	::LogSave("Process CMsgAnnounceList: type=%u, amount=%u, index=%u",
				m_usType, m_usAmount, m_dwIndexFirst);

#endif
	OBJID idUser = pUser->GetID();
	switch(m_usType)
	{
	case CHECK_BY_INDEX:
	    mapGroup.GetAnnounce()->SendAnnounceList(mapGroup.GetRoleManager()->QueryRole(idUser),CHECK_BY_INDEX,m_nIndex);
        break;
	case CHECK_BY_ID:
	    mapGroup.GetAnnounce()->SendAnnounceList(mapGroup.GetRoleManager()->QueryRole(idUser),CHECK_BY_ID,m_idAnnounce);
        break;
	case CHECK_BY_OWNER:
	    mapGroup.GetAnnounce()->SendAnnounceList(mapGroup.GetRoleManager()->QueryRole(idUser),CHECK_BY_OWNER,m_usAmount);
		break;
	}
//	ASSERT(!"CMsgAnnounceList::Process");
}