#include "AllMsg.h"
#ifdef	WORLD_KERNEL
#include "userlist.h"
#else
#pragma warning(disable:4786)
#include "mapgroup.h"
#endif


void CMsgDialog::Process(CUser* pUser)
{
#ifdef _MSGDEBUG
	::LogMsg("Process MsgDialog, Sender:%s, Receiver:%s, Words:%s", 
						szSender, 
						szReceiver, 
						szWords);
#endif

	char* pAccept = "";
	char szText[MAX_PARAMSIZE];
	if(m_StrPacker.GetString(0, szText, sizeof(szText)))
		pAccept	= szText;
	CHECK(strlen(szText) < MAX_PARAMSIZE);

	CHECK(pUser);

	if(!pUser->IsAlive())
	{
		pUser->SendSysMsg(STR_DIE);
		return ;
	}

	switch(m_ucAction)
	{
	case	MSGDIALOG_ANSWER:
		{
			pUser->ProcessTask(m_idxTask, pAccept);
		}
		break;
	case	MSGDIALOG_TASKID:
		{
			pUser->ProcessClientTask(m_idTask, pAccept);
		}
		break;
	}
}
