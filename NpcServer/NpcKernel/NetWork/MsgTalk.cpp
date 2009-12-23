#include "AllMsg.h"
#include "NpcWorld.h"
#include "basefunc.h"
#include "Agent.h"

void CMsgTalk::Process(OBJID idNpc)
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

	CHECK(strlen(szWords) <= 255);

	switch(m_unTxtAttribute)
	{
	case _TXTATR_WEBPAGE:
		break;
	case	_TXTATR_GLOBAL:
		{
		}
		break;
	case	_TXTATR_PRIVATE:
		{
			CAgent* pAgent = NpcManager()->QueryAgent(idNpc);
			if(pAgent)
				pAgent->PrivateTalkToMe(szSender, szWords, szEmotion);
		}
		break;
	case	_TXTATR_ENTRANCE:
		{
			if(strcmp(szWords, REPLAY_OK_STR) == 0)
			{
				NpcWorld()->PrintText("Login game server ok.");
				NpcWorld()->LoginOK();
			}
			else
			{
				NpcWorld()->PrintText("Login game server failed.");
				NpcWorld()->PrintText(szWords);
				NpcWorld()->CloseShell();
				return ;
			}
		}
		break;
	default:
		{
			CAgent* pAgent = NpcManager()->QueryAgent(szReceiver);
			if(pAgent)
				pAgent->TalkToMe(szSender, szWords, szEmotion);
		}
		break;
	}
}