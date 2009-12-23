#include "AllMsg.h"
#pragma warning(disable:4786)
#include "MapGroup.h"
#include "usermanager.h"

const int	MSGTITLE_SIZE		= 44;
const int	MSGLIST_SIZE		= 5;

void CMsgMessageBoard::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{

#ifdef _MSGDEBUG
	::LogMsg("Process CMsgMessageBoard, Sender:%s, Receiver:%s, Words:%s", 
						szSender, 
						szReceiver, 
						szWords);
#endif

	if(m_usChannel == _TXTATR_MSG_SYN)
	{
		CUser* pUser = mapGroup.GetUserManager()->GetUser(idSocket, idNpc);
		if(!pUser)
			return ;
		OBJID idSyn = pUser->GetSynID();
		if(idSyn == ID_NONE)
			return ;

		SetTransData(idSyn);
	}

	mapGroup.QueryIntraMsg()->TransmitWorldMsg(this);
}
