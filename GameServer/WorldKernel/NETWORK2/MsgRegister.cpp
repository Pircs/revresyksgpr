#include "AllMsg.h"
#include "basefunc.h"
#include "worldkernel.h"

void CMsgRegister::Process(SOCKET_ID idSocket)
{
	//SOCKET_ID idSocket = *pInfo;
	if(!StringCheck(m_szAccount))
	{
		MsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, STR_ERROR_ILLEGAL_ACCOUNT, NULL, _COLOR_WHITE, _TXTATR_REGISTER))
			GameWorld()->SendClientMsg(idSocket, &msg);
		return;
	}

	if(!StringCheck(m_szPassword))
	{
		MsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, STR_ERROR_ILLEGAL_PASSWORD, NULL, _COLOR_WHITE, _TXTATR_REGISTER))
			GameWorld()->SendClientMsg(idSocket, &msg);
		return;
	}

	if(!NameStrCheck(m_szName))
	{
		MsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, STR_ERROR_ILLEGAL_NAME, NULL, _COLOR_WHITE, _TXTATR_REGISTER))
			GameWorld()->SendClientMsg(idSocket, &msg);
		return;
	}

	if(g_UserList.CreateNewPlayer(m_szAccount, m_szName, m_szPassword, 
					m_unLook, m_unData, m_idAccount, m_cLength, m_cFat))
	{
		MsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, REPLAY_OK_STR, NULL, _COLOR_WHITE, _TXTATR_REGISTER))
			GameWorld()->SendClientMsg(idSocket, &msg);
	}
	else
	{
		MsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, STR_ERROR_DUPLICATE_NAME, NULL, _COLOR_WHITE, _TXTATR_REGISTER))
			GameWorld()->SendClientMsg(idSocket, &msg);
	}
}
