#define _WINSOCKAPI_
#include "AllMsg.h"
#include "WorldKernel.h"
#include "userlist.h"
#include "MessageBoard.h"

void CMsgTalk::Process(SOCKET_ID idSocket)
{
#ifdef _MSGDEBUG
	::LogMsg("Process MsgTalk, Sender:%s, Receiver:%s, Words:%s", 
						szSender, 
						szReceiver, 
						szWords);
#endif

	char szSender[_MAX_NAMESIZE];
	char szReceiver[_MAX_NAMESIZE];
	char szEmotion[_MAX_NAMESIZE];
	char szWords[_MAX_WORDSSIZE];

	m_StrPacker.GetString(0, szSender, sizeof(szSender));
	m_StrPacker.GetString(1, szReceiver, sizeof(szReceiver));
	m_StrPacker.GetString(2, szEmotion, sizeof(szEmotion));
	m_StrPacker.GetString(3, szWords, sizeof(szWords));

	CPlayer* pUser = UserList()->GetPlayerBySocket(idSocket);
	CHECK(pUser || m_unTxtAttribute==_TXTATR_MSG_SYSTEM);

	switch(m_unTxtAttribute)
	{
	case _TXTATR_WEBPAGE:
		break;
	case _TXTATR_PRIVATE:
		{
			/*/ 转发耳语消息
			CPlayer* pPlayer = g_UserList.GetPlayer(szReceiver);
			if(pPlayer)
			{
				pPlayer->SendMsg(this);
			}
			else
			{
				CPlayer* pSender = g_UserList.GetPlayerBySocket(idSocket);
				if(pSender)
				{
					if(this->Create(SYSTEM_NAME, pSender->szName, "对方不在线。"))
						pSender->SendMsg(this);
				}
			}//*/
		}
		break;
	case _TXTATR_MSG_TRADE:
		{
			GameWorld()->QueryTradeMsgBd()->AddMsg(pUser->GetID(), szSender, szWords);
		}
		break;
	case _TXTATR_MSG_FRIEND:
		{
			GameWorld()->QueryFriendMsgBd()->AddMsg(pUser->GetID(), szSender, szWords);
		}
		break;
	case _TXTATR_MSG_TEAM:
		{
			GameWorld()->QueryTeamMsgBd()->AddMsg(pUser->GetID(), szSender, szWords);
		}
		break;
	case _TXTATR_MSG_SYN:
		{
			OBJID idSyn = GetTransData();
			CSyndicateWorld* pSyn = SynWorldManager()->QuerySyndicate(idSyn);
			IF_OK(pSyn)
				pSyn->QueryMessageBoard()->AddMsg(pUser->GetID(), szSender, szWords);
		}
		break;
	case _TXTATR_MSG_OTHER:
		{
			GameWorld()->QueryOtherMsgBd()->AddMsg(pUser->GetID(), szSender, szWords);
		}
		break;
	case _TXTATR_MSG_SYSTEM:
		{
			OBJID idUser = GetTransData();
			GameWorld()->QuerySystemMsgBd()->AddMsg(idUser, szSender, szWords);
		}
		break;
	case _TXTATR_SERVE:
		{
			CPlayer* pPlayer = UserList()->GetGM();
			if (pPlayer)
			{
				pPlayer->SendMsg(this);
			}
			else
			{
				CPlayer* pSpeaker = UserList()->GetPlayerBySocket(idSocket);
				if (pSpeaker)
				{
					MsgTalk	msg;
					if (msg.Create(SYSTEM_NAME, pSpeaker->szName, STR_TARGET_OFFLINE))
						pSpeaker->SendMsg(&msg);
				}
			}
		}
		break;

	default:
		break;
	}
}
