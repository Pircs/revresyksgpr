#define _WINSOCKAPI_
#include "AllMsg.h"
#include "WorldKernel.h"
#include "MessageBoard.h"
#include "userlist.h"

bool CMsgMessageBoard::Create(int nAction, int nChannel, int nIndex, CMessageBoard* pObj)
{
	m_ucAction		=nAction;
	m_usChannel		=nChannel;
	m_usIndex		=nIndex;

	bool bSucMake	=true;
	if(pObj)
	{
		for(int i = m_usIndex; i < m_usIndex + MSGLIST_SIZE; i++)
		{
			if(!pObj->IsValid(i))
				break;

			bSucMake	&=m_StrPacker.AddString(pObj->GetMsgName(i));
			char	buf[MSGTITLE_SIZE];
			SafeCopy(buf, pObj->GetMsgWords(i), MSGTITLE_SIZE);
			bSucMake	&=m_StrPacker.AddString(buf);
			char	bufTime[_MAX_NAMESIZE];
			pObj->GetMsgTime(i, bufTime);
			bSucMake	&=m_StrPacker.AddString(bufTime);
		}
	}
	m_unMsgSize	=sizeof(*this)-256+m_StrPacker.GetSize();
	return bSucMake;
}

void CMsgMessageBoard::Process(SOCKET_ID idSocket)
{

#ifdef _MSGDEBUG
	::LogMsg("Process CMsgMessageBoard, Sender:%s, Receiver:%s, Words:%s", 
						szSender, 
						szReceiver, 
						szWords);
#endif

	char szSender[_MAX_NAMESIZE] = "";
	m_StrPacker.GetString(0, szSender, sizeof(szSender));

	CMessageBoard* pObj=NULL;
	switch(m_usChannel)
	{
	case _TXTATR_MSG_TRADE:
		{
			pObj = GameWorld()->QueryTradeMsgBd();
		}
		break;
	case _TXTATR_MSG_FRIEND:
		{
			pObj = GameWorld()->QueryFriendMsgBd();
		}
		break;
	case _TXTATR_MSG_TEAM:
		{
			pObj = GameWorld()->QueryTeamMsgBd();
		}
		break;
	case _TXTATR_MSG_SYN:
		{
			OBJID idSyn = GetTransData();
			CSyndicateWorld* pSyn = SynWorldManager()->QuerySyndicate(idSyn);
			IF_OK(pSyn)
				pObj = pSyn->QueryMessageBoard();
		}
		break;
	case _TXTATR_MSG_OTHER:
		{
			pObj = GameWorld()->QueryOtherMsgBd();
		}
		break;
	case _TXTATR_MSG_SYSTEM:
		{
			pObj = GameWorld()->QuerySystemMsgBd();
		}
		break;
	}

	if(!pObj)
		return ;

	CPlayer* pUser = UserList()->GetPlayerBySocket(idSocket);
	if(!pUser)
		return ;

	switch(m_ucAction)
	{
	case	MESSAGEBOARD_DEL:
		{
			pObj->DelMsg(pUser->GetID());
		}
		break;
	case	MESSAGEBOARD_GETLIST:
		{
			if(pObj->IsValid(m_usIndex))
			{
				CMsgMessageBoard	msg;
				IF_OK(msg.Create(MESSAGEBOARD_LIST, m_usChannel, m_usIndex, pObj))
					GameWorld()->SendClientMsg(idSocket, &msg);//SendMsg(&msg);
			}
		}
		break;
	case	MESSAGEBOARD_GETWORDS:
		{
			CHECK(strlen(szSender) > 0);

			CMessageBoard::MessageInfo* pInfo;
			pInfo = pObj->GetMsgInfo(szSender);

			if(pInfo)
			{
				NAMESTR		buf;
				::DateTimeStamp(buf, pInfo->time);

				CMsgTalk	msg;
				IF_OK(msg.Create((char*)pInfo->name, ALLUSERS_NAME, (char*)pInfo->words, buf, 0xFFFFFF, m_usChannel))
					GameWorld()->SendClientMsg(idSocket, &msg);//SendMsg(&msg);
			}
		}
		break;
	}
}
