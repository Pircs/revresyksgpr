#include "MessagePort.h"
#include "protocol.h"
#include "define.h"
#include "AllMsg.h"
#include "WorldKernel.h"
//using namespace world_kernel;
#include "UserList.h"

void CMsgLogin::Process(SOCKET_ID idSocket, OBJID idNpc)
{

#ifdef _MSGDEBUG
	::LogMsg("Process CMsgLogin, Account:%d, Name:%s", 
					m_idAccount, 
					m_szUserName);
#endif
	if(m_usVersion == NPCSERVER_VERSION || m_usVersion == NPCSERVER_VERSION_RELOGIN)
	{
		ProcessNpcServerLogin(idSocket, m_usVersion == NPCSERVER_VERSION);
		return ;
	}
/* 无帐号服务器时，玩家登录代码
#ifdef	ACCOUNT_ENABLE
	return ;				// 从帐号服务器登录，该消息不处理!
#endif
	if(m_usVersion < GAME_VERSION)
	{
		CMsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, "错误：版本号错误!", NULL, _COLOR_WHITE, _TXTATR_ENTRANCE))
			SendMsg(&msg);
		return;
	}

	CPlayer*	pPlayer;
	if(pPlayer = g_UserList.GetPlayerByAccount(m_szAccount))
	{
		GameWorld()->GetSocketInterface()->CloseSocket(pPlayer->idSocket);

		CMsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, "错误：卡号，请重新登录!", NULL, _COLOR_WHITE, _TXTATR_ENTRANCE))
			SendMsg(&msg);
		GameWorld()->GetSocketInterface()->CloseSocket(GetSocketID());		// 关闭自己
		return;
	}

	if(!g_UserList.LoginUser(m_szAccount, m_szPassword, m_idSocket))
	{
		CMsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, "错误：帐号或密码错误!", NULL, _COLOR_WHITE, _TXTATR_ENTRANCE))
			SendMsg(&msg);
		return;
	}

	pPlayer	= g_UserList.GetPlayerByAccount(m_szAccount);
	if(!pPlayer)
		return;

	UserList()->LoginToMapGroup(pPlayer);
*/
	if(idSocket == SOCKET_NONE)//IsNpcMsg())
	{
		OBJID idAgent	= idNpc;
		if(!UserList()->LoginAgent(idAgent))
			return ;
		CPlayer* pPlayer	= g_UserList.GetPlayer(idAgent);
		if(!pPlayer)
			return;
		UserList()->LoginToMapGroup(pPlayer);
		return ;
	}
}

bool CMsgLogin::ProcessNpcServerLogin(SOCKET_ID idSocket, bool bDelAllMonster)
{
	IMessagePort* pPort = GameWorld()->GetMsgPort();

	SOCKET_ID idNpcSocket = GameWorld()->GetNpcSocketID();
	if(idNpcSocket != SOCKET_NONE)
	{
		LOGMSG("NPC server repeat login, break the last socket!");
//		pPort->Send(MSGPORT_SOCKET, SOCKET_CLOSESOCKET, VARTYPE_INT, &idNpcSocket);		//@ 可能有问题
	}

	if(strcmp(m_szAccount, NPCSERVER_ACCOUNT) != 0 
			|| strcmp(m_szPassword, NPCSERVER_PASSWORD) != 0)
	{
		MsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, "ERROR: login refuse!", NULL, _COLOR_WHITE, _TXTATR_ENTRANCE))
			GameWorld()->SendNpcMsg(ID_NONE, &msg);
		return false;
	}

	// del all monster
	IMessagePort* pMsgPort = GameWorld()->GetMsgPort();
	if(bDelAllMonster)
	{
		for(int i = MSGPORT_MAPGROUP_FIRST; i < GameWorld()->GetMsgPort()->GetSize(); i++)
			pMsgPort->Send(i, MAPGROUP_DELALLMONSTER, VARTYPE_INT, &m_usVersion);	// m_usVersion: no use
		pMsgPort->Send(MSGPORT_SOCKET, MAPGROUP_DELALLMONSTER, VARTYPE_INT, &m_usVersion);
		UserList()->DelAllAgent();
	}
	for(int i = MSGPORT_MAPGROUP_FIRST; i < GameWorld()->GetMsgPort()->GetSize(); i++)
		pMsgPort->Send(i, MAPGROUP_LOADALLPET, VARTYPE_INT, &m_usVersion);	// m_usVersion: no use

	idNpcSocket	= idSocket;
	GameWorld()->SetNpcSocketID(idNpcSocket);
	pPort->Send(MSGPORT_SOCKET, SOCKET_SETNPCSOCKETID, VARTYPE_INT, &idNpcSocket);

	{
		MsgTalk	msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, REPLAY_OK_STR, NULL, _COLOR_WHITE, _TXTATR_ENTRANCE))
			GameWorld()->SendNpcMsg(ID_NONE, &msg);
	}

	char*	pText = "NPC server login OK.";
	pPort->Send(MSGPORT_SHELL, SHELL_PRINTTEXT, STRING_TYPE(pText), pText);

	return true;
}