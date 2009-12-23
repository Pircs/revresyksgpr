#include "AllMsg.h"
#include "WorldKernel.h"
#include "userlist.h"

void CMsgName::Process(SOCKET_ID idSocket, int nTrans)
{
	CPlayer* pUser = UserList()->GetPlayerBySocket(idSocket);
	CHECK(pUser);

	char szName[_MAX_WORDSSIZE];
	m_StrPacker.GetString(0, szName, sizeof(szName));

	switch(m_ucType)
	{
//	case NAMEACT_MEMBERLIST_SPECIFYSYN:
//		{
//			CSyndicateWorld* pSyn = GameWorld()->QuerySynManager()->QuerySyndicate(this->GetTransData());
//			CHECK(pSyn);
//			pSyn->SendMemberListSpecifySyn(pUser, m_dwData);
//		}
//		break;
	case	NAMEACT_MEMBERLIST:
		{
			CSyndicateWorld* pSyn = GameWorld()->QuerySynManager()->QuerySyndicate(nTrans);
			CSyndicateWorld * pMasterSyn = pSyn->GetMasterSyn();			
			CHECK(pSyn);
			CHECK(pMasterSyn);
			pMasterSyn->SendMemberList(pUser, m_dwData);
		}
		break;
	}
}