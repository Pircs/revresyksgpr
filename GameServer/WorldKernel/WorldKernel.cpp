
#define	_WINSOCKAPI_		// 阻止加载winsock.h
#include "inifile.h"
#include "protocol.h"
#include "windows.h"
#include "Winuser.h"
#include "SharedBaseFunc.h"
#include "I_MessagePort.h"
#include "AllMsg.h"
#include "NetMsg.h"
#include "MessageBoard.h"
#include "WorldKernel.h"
#include "ClientSocket.h"
#include "SynWorldManager.h"
#include "SupermanList.h"
#include "AutoPtr.h"
#include "ChatRoomManager.h"

//using namespace world_kernel;

CWorldKernel*	CWorldKernel::m_pWorld = NULL;

//extern CChatRoomManager g_objChatRoomManager;
// interface
bool CWorldKernel::Create(IMessagePort* pPort)
{
	m_pMsgPort		= pPort;
	m_idProcess		= m_pMsgPort->GetID();		// process id == msg_port_id
	m_pMsgPort->Open();
	ASSERT(m_idProcess == MSGPORT_WORLD);
	m_idNpcSocket	= SOCKET_NONE;

	// 初始化数据库
	char	DB_IP	[DBSTR_SIZE] = "";
	char	DB_USER	[DBSTR_SIZE] = "";
	char	DB_PW	[DBSTR_SIZE] = "";
	char	DB_DB	[DBSTR_SIZE] = "";
	CIniFile	ini(CONFIG_FILENAME, "Database");
	ini.GetString(DB_IP,	"DB_IP",	DBSTR_SIZE);
	ini.GetString(DB_USER,	"DB_USER",	DBSTR_SIZE);
	ini.GetString(DB_PW,	"DB_PW",	DBSTR_SIZE);
	ini.GetString(DB_DB,	"DB_DB",	DBSTR_SIZE);
	m_pDb	= ::CreateDatabase(DB_IP, DB_USER, DB_PW, DB_DB);
	if(!m_pDb)
	{
		LOGERROR("数据库连接失败!");
		return false;
	}

	// TODO: 请在此添加初始化代码
	m_pUserList	= CUserList::CreateNew();
	if(!m_pUserList)
		return false;
	m_pUserList->Create(GetSocketInterface(), m_pDb);

	m_pMapList	= CMapList::CreateNew();
	if(!m_pMapList)
		return false;
	m_pMapList->Create(m_pDb);

	m_pWorld	= this;

	m_pSynManager	= new CSynWorldManager;
	CHECKF(m_pSynManager);
	m_pSynManager->Create(m_pDb);

	m_setConnectClient		= CConnectSet::CreateNew(true);
	m_setConnectAccount		= CConnectSet::CreateNew(true);

	// account server
//	CIniFile	ini(CONFIG_FILENAME, "AccountServer");
	ini.SetSection("AccountServer");
	NAMESTR		ACCOUNT_IP;
	int			ACCOUNT_PORT;

	ini.GetString(ACCOUNT_IP, "ACCOUNT_IP", _MAX_NAMESIZE);
	ACCOUNT_PORT	= ini.GetInt("ACCOUNT_PORT");
	ini.GetString(m_szServer, "SERVERNAME", _MAX_NAMESIZE);
	ini.GetString(m_szAccount, "LOGINNAME", _MAX_NAMESIZE);
	ini.GetString(m_szPassword, "PASSWORD", _MAX_NAMESIZE);
	if(ACCOUNT_PORT == 0)
	{
		LOGERROR("INI配置错误!");
		return false;
	}
	if(m_sock.Open(ACCOUNT_IP, ACCOUNT_PORT, 0, SOCKET_ACCOUNTBUFSIZE, SOCKET_ACCOUNTBUFSIZE))
		m_nState = STATE_CONNECTOK;

	m_ptrTradeMsgBd		= CMessageBoard::CreateNew(_TXTATR_MSG_TRADE);
	m_ptrFriendMsgBd	= CMessageBoard::CreateNew(_TXTATR_MSG_FRIEND);
	m_ptrTeamMsgBd		= CMessageBoard::CreateNew(_TXTATR_MSG_TEAM);
	m_ptrOtherMsgBd		= CMessageBoard::CreateNew(_TXTATR_MSG_OTHER);
	m_ptrSystemMsgBd	= CMessageBoard::CreateNew(_TXTATR_MSG_SYSTEM);

	LoadConfig(m_pDb);

//	g_objChatRoomManager.Init();

	return true;		// return false : 创建失败，程序关闭。
}

bool CWorldKernel::ProcessMsg(OBJID idPacket, void* buf, int nType, int nFrom)
{
	switch(idPacket)
	{
	case	KERNEL_CLIENTMSG:
		{
			CLIENTMSG_PACKET0*	pMsg = (CLIENTMSG_PACKET0*)buf;
			ProcessMyMsg(pMsg->idSocket, ID_NONE, pMsg->msgBuf);
		}
		break;
	case	WORLD_TRANSMITMSG:
		{
			TRANSMSG_PACKET0*	pMsg = (TRANSMSG_PACKET0*)buf;
			ProcessMyMsg(pMsg->idSocket, pMsg->idNpc, pMsg->msgBuf, pMsg->nTrans);
		}
		break;
	case	KERNEL_NPCMSG:
		{
			NPCMSG_PACKET0*	pMsg = (NPCMSG_PACKET0*)buf;
			ProcessMyMsg(SOCKET_NONE, pMsg->idNpc, pMsg->msgBuf);
		}
		break;
	case	WORLD_SHELLTALK:
		{
			char* pText = (char*)buf;
			// TODO: 请在此添加外壳输入文本的处理代码
			m_pMsgPort->Send(MSGPORT_SHELL, SHELL_PRINTTEXT, STRING_TYPE(pText), pText);

			char szCmd[256] = "";
			if (sscanf(pText, "%s", szCmd) == 1)
			{
				if (0 == stricmp(szCmd, "setmaxplayer"))
				{
					int nMaxPlayers = 0;
					if (2 == sscanf(pText, "%s %d", szCmd, &nMaxPlayers))
					{
						extern DWORD g_dwMaxPlayer;
						InterlockedExchange((long*)&g_dwMaxPlayer, nMaxPlayers);
						{
							char szMsg[256] = "";
							sprintf(szMsg, "Set max player limit to %d", g_dwMaxPlayer);
							m_pMsgPort->Send(MSGPORT_SHELL, SHELL_PRINTTEXT, STRING_TYPE(szMsg), szMsg);
						}
					}
				}
				else if (0 == stricmp(szCmd, "kickoutcheat"))
				{
					int	nData = 0;
					if (2 == sscanf(pText, "%s %d", szCmd, &nData))
					{
						extern long	g_sKickoutCheat;
						long nOld = InterlockedExchange(&g_sKickoutCheat, nData);
					}
				}
				else if (0 == stricmp(szCmd, "kickoutall"))
				{
					OBJID idGmSocket = -1;		// -1: for console
					m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_BREAKALLCONNECT, VARTYPE_INT, &idGmSocket);
				}
				else if (0 == stricmp(szCmd, "broadcast"))
				{
					char* pCmd = pText + strlen(szCmd);
					if(*pCmd == ' ')
						pCmd++;

					GameWorld()->QueryUserList()->BroadcastMsg(pCmd, NULL, NULL, 0x00FF0000, _TXTATR_GM);
				}
				else if (0 == stricmp(szCmd, "shutdown"))
				{
					GameWorld()->QueryUserList()->BroadcastMsg(STR_SHUTDOWN_NOTIFY, NULL, NULL, 0x00FF0000, _TXTATR_GM);

					char* pMsg = "Send shut down broadcast message.";
					m_pMsgPort->Send(MSGPORT_SHELL, SHELL_PRINTTEXT, STRING_TYPE(pMsg), pMsg);
				}
				else if (0 == stricmp(szCmd, "restartserver"))
				{
					extern long g_nServerClosed;
					if(g_nServerClosed)
					{
						extern long g_nRestart;
						g_nRestart = true;
					}
					else
					{
						LPCTSTR pMsg = "Please <kickoutall> before <restartserver>!";
						m_pMsgPort->Send(MSGPORT_SHELL, SHELL_PRINTTEXT, STRING_TYPE(pMsg), pMsg);
					}
				}
				else if (0 == stricmp(szCmd, "restartwindows"))
				{
					extern long g_nServerClosed;
					if(g_nServerClosed)
					{
						HANDLE hToken;
						TOKEN_PRIVILEGES tkp;

						// Get a token for this process.
						if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
						{
							LOGERROR("OpenProcessToken");
							break;
						}

						// Get the LUID for the shutdown privilege.
						LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
						tkp.PrivilegeCount = 1; // one privilege to set
						tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

						// Get the shutdown privilege for this process.
						AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

						// Cannot test the return value of AdjustTokenPrivileges.
						if (GetLastError() != ERROR_SUCCESS)
						{
							LOGERROR("AdjustTokenPrivileges");
							break;
						}

						// Shut down the system and force all applications to close. 
#define EWX_FORCEIFHUNG      0x00000010
						if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 0))
						{
							LOGERROR("ExitWindowsEx");
							break;
						}

//						::ExitWindowsEx(EWX_REBOOT, 0);		// | EWX_FORCEIFHUNG
					}
					else
					{
						LPCTSTR pMsg = "Please <kickoutall> before <restartserver>!";
						m_pMsgPort->Send(MSGPORT_SHELL, SHELL_PRINTTEXT, STRING_TYPE(pMsg), pMsg);
					}
				}
			}
		}
		break;
	// TODO: 请在此添加其它内部消息处理代码
	case	KERNEL_CLOSEKERNEL:
		{
			SOCKET_ID	idSocket = *(SOCKET_ID*)buf;
			LOGDEBUG("DEBUG：世界核心收到关闭核心消息，SOCKET_ID[%u]", idSocket);
#ifdef	NEW_LOGOUT
			g_UserList.LogoutUser(idSocket);
			m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_CLOSESOCKET, VARTYPE_INT, &idSocket);		// readme.txt(2-1)
#else
			if(g_UserList.IsLoginMapGroup(idSocket))
			{
				for(int i = MSGPORT_MAPGROUP_FIRST; i < m_pMsgPort->GetSize(); i++)
					m_pMsgPort->Send(i, KERNEL_CLOSEKERNEL, VARTYPE_INT, &idSocket);		// readme.txt(2-2)
			}
			else
				m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_CLOSESOCKET, VARTYPE_INT, &idSocket);		// readme.txt(2-1)
#endif
		}
		break;
	case	WORLD_CLOSESOCKET:		// 由MAPGROUP发过来
		{
#ifdef	NEW_LOGOUT
			ASSERT(!"WORLD_CLOSESOCKET");
#else
			SOCKET_ID	idSocket = *(SOCKET_ID*)buf;
			LOGDEBUG("DEBUG：世界核心收到关闭网络消息，SOCKET_ID[%u]", idSocket);
			g_UserList.LogoutUser(idSocket);
			m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_CLOSESOCKET, VARTYPE_INT, &idSocket);		// readme.txt(2-6)
#endif
		}
		break;
	case	WORLD_SETPROCESSID:
		{
			CHANGE_USERDATA* pPakcet = (CHANGE_USERDATA*)buf;
			g_UserList.ChangeProcessID(pPakcet->idSocket, pPakcet->nData);
		}
		break;
	case	WORLD_USERNAMEMSG:
		{
			USERNAMEMSG_PACKET0*	pMsg = (USERNAMEMSG_PACKET0*)buf;
			//SOCKET_ID	idSocket= pMsg->idSocket;

//			if (pMsg->msgBuf->GetType() == _MSG_TALK)
//			{
//				// 服务频道判断
//				typedef struct {
//					DWORD	dwWordsColor;
//					USHORT	unTxtAttribute;
//					// ... 其它定义没必要写了
//				}MSG_Info;
//				MSG_Info* pInfo = (MSG_Info*)pMsg->buf;
//				if (pInfo->unTxtAttribute == _TXTATR_SERVE)
//				{
//					CPlayer* pTarget = UserList()->GetGM();
//					if (pTarget)
//					{
//						SendClientMsg(pTarget->idSocket, pMsg->buf);
//						break;
//					}
//				}
//			}

			CPlayer* pPlayer = UserList()->GetPlayer(pMsg->szName);
			if(pPlayer)
			{
				//***********************************************************************************************************************************
			//	if(!pPlayer->IsAgent())
			//		SendClientMsg(pPlayer->idSocket, pMsg->buf);
			//	else
			//		SendNpcMsg(pPlayer->GetID(), pMsg->buf);
			}
			else if(pMsg->msgBuf->GetType() == _MSG_TALK && pMsg->idSocket != SOCKET_NONE)		// SOCKET_NONE : agent talk to user (BUG :)
			{
				// 私聊
				CPlayer* pSender = UserList()->GetPlayerBySocket(pMsg->idSocket);
				if(pSender)
				{
					CMsgTalk	msg;
					IF_OK(msg.Create(SYSTEM_NAME, pSender->szName, STR_TARGET_OFFLINE))
						pSender->SendMsg(&msg);
				}
			}
		}
		break;
	case	WORLD_USERIDMSG:
		{
			USERIDMSG_PACKET0*	pMsg = (USERIDMSG_PACKET0*)buf;
			//SOCKET_ID	idSocket= pMsg->idSocket;
			CPlayer* pPlayer = UserList()->GetPlayer(pMsg->idUser);
			if(pPlayer)
			{
				//***********************************************************************************************
			//	if(!pPlayer->IsAgent())
		//			SendClientMsg(pPlayer->idSocket, pMsg->buf);
		//		else
		//			SendNpcMsg(pPlayer->GetID(), pMsg->buf);
			}
		}
		break;
	// syndicate -------------------------------------------------------------------------------
	case	KERNEL_CREATESYNDICATE:
		{
			CreateSyndicateInfo* pPacket = (CreateSyndicateInfo*)buf;
			GameWorld()->QuerySynManager()->QueryModify()->CreateSyndicate(pPacket);
		}
		break;
	case	KERNEL_DESTROYSYNDICATE:
		{
			OBJID* pId	= (OBJID*)buf;
			OBJID idSyn		= pId[0];
			OBJID idTarget	= pId[1];
			GameWorld()->QuerySynManager()->QueryModify()->DestroySyndicate(idSyn, idTarget);
		}
		break;
		
	case	KERNEL_COMBINESYNDICATE:
		{
			OBJID* pId	= (OBJID*)buf;
			OBJID idSyn		= pId[0];
			OBJID idTarget	= pId[1];
			GameWorld()->QuerySynManager()->QueryModify()->CombineSyndicate(idSyn, idTarget);
		}
		break;
	case	KERNEL_CHANGESYNDICATE:
		{
			SynFuncInfo0* pPacket = (SynFuncInfo0*)buf;
			CSyndicateWorld* pSyn = GameWorld()->QuerySynManager()->QuerySyndicate(pPacket->idSyn);
			IF_OK(pSyn)
			{
				pSyn->ChangeSyndicate(pPacket);
			}
		}
		break;
	case	WORLD_LEVELUP:
		{
			OBJID idUser = *(OBJID*)buf;
			CPlayer* pPlayer = GameWorld()->QueryUserList()->GetPlayer(idUser);
			IF_OK(pPlayer)
				pPlayer->m_nLevel	= LOGIN_FREE_LEVEL;
		}
		break;
	case	WORLD_QUERYFEE:
		{
			OBJID idAccount = *(OBJID*)buf;
			if(IsAccountLoginOK())
				SendQueryFee(idAccount, 0, 0, 0);
		}
		break;
	case	WORLD_SENDTIME:
		{
			SOCKET_ID idSocket = *(SOCKET_ID*)buf;
			IF_OK(idSocket != SOCKET_NONE)
				UserList()->SendTimeToSocket(idSocket);
		}
		break;
	case	KERNEL_SUPERMANLIST:
		{
			VARIANT_SET0* pInfo  = (VARIANT_SET0*)buf;
			OBJID	idUser	= pInfo->IntParam(0);
			int		nNumber	= pInfo->IntParam(1);
			if(nNumber)
				UserList()->QuerySupermanList()->AddMember(idUser, nNumber);
			else
				UserList()->QuerySupermanList()->QueryMember(idUser);
		}
		break;
	case	KERNEL_QUERYSUPERMANLIST:
		{
			VARIANT_SET0* pInfo  = (VARIANT_SET0*)buf;
			OBJID	idUser	= pInfo->IntParam(0);
			int		nIndex	= pInfo->IntParam(1);
			OBJID	idAction	= pInfo->IntParam(2);
			int		nNumber	= pInfo->IntParam(3);
			UserList()->QuerySupermanList()->SendList(idUser, nIndex, nNumber, idAction);
		}
		break;
	default:
		ASSERT(!"CWorldKernel::ProcessMsg()");
	}

	return true;		// return false : 消息处理异常，程序关闭。
}

bool CWorldKernel::OnTimer(time_t tCurr)
{
	// TODO: 请在此添加时钟处理代码
	g_UserList.OnTimer(tCurr);
#ifdef	ACCOUNT_ENABLE
	// account server
	if(!m_sock.IsOpen())
	{
		m_nState = STATE_NONE;
		GameWorld()->PrintText("Connect to account server again...");
		if(m_sock.Open(0))
			m_nState = STATE_CONNECTOK;
	}
#endif
	if(m_sock.IsOpen())
	{
		if(m_nState == STATE_CONNECTOK)
		{
			MsgC2SAccount msg;
			msg.Create(m_szAccount, m_szPassword, m_szServer);
			m_sock.SendPacket((LPCTSTR)&msg, msg.GetSize(), true);
			m_nState = STATE_ACCOUNTOK;
		}

		ProcessAccountNetMsg();
	}

//	g_objChatRoomManager.OnTimer(tCurr);

	return true;		// return false : 消息处理异常，程序关闭。
}

bool CWorldKernel::Release()
{
	// TODO: 请在此添加代码
	g_UserList.LogoutAllUser();

	S_REL (m_pUserList);
	S_REL (m_pMapList);
	S_REL (m_pSynManager);

	m_pMsgPort->Close();

	GameWorld()->SendFee(ID_NONE, MsgFee::SERVER_CLOSE);

	m_sock.Close();

	S_REL (m_setConnectClient);
	S_REL (m_setConnectAccount);

	S_REL (m_pDb);
	
	delete this;
	return true;		// return false : 无意义。
}

bool CWorldKernel::ProcessMyMsg(SOCKET_ID idSocket, OBJID idNpc, Msg* pMsg, int nTrans)
{
	// TODO: 请在此添加客户端上传消息的处理代码
	if(!pMsg || pMsg->GetSize() <= 0 || pMsg->GetSize() >_MAX_MSGSIZE)
        return false;
	try {
		switch(pMsg->GetType())
		{
		case _MSG_TALK:
			((CMsgTalk*)pMsg)->Process(idSocket);
			break;
		case _MSG_MESSAGEBOARD:
			((CMsgMessageBoard*)pMsg)->Process(idSocket);
			break;
		case _MSG_REGISTER:
			((CMsgRegister*)pMsg)->Process(idSocket);
			break;
		case _MSG_LOGIN:
			((CMsgLogin*)pMsg)->Process(idSocket, idNpc);
			break;
		case _MSG_AINPCINFO:
			((CMsgAiNpcInfo*)pMsg)->Process();
			break;
		case _MSG_CONNECT:
			((CMsgConnect*)pMsg)->Process(idSocket);
			break;
		case _MSG_NAME:
			((CMsgName*)pMsg)->Process(idSocket, nTrans);
			break;
		case _MSG_FRIEND:
			((CMsgFriend*)pMsg)->Process(nTrans);
			break;
			//	case _MSG_CHATROOM:
			//		pMsg	=new CMsgChatRoom;
			//		break;
		default:
			break;
		}
	}
	catch(...)
	{
		//			CNetMsg::DumpMsg(pMsg);
		ASSERT(!"pMsg->Process(this);");
		::LogSave("exception catch at CGameSocket::ProcessMsg()! MsgType:%d, SocketID:%u", pMsg->GetType(), idSocket);
	}
	return true;
}
//
//bool CWorldKernel::SendMsg(CNetMsg* pNetMsg)
//{
//	ASSERT(pNetMsg);
//	if(!pNetMsg)
//		return false;
//
//	if(!pNetMsg->IsNpcMsg())
//		return SendMsg(pNetMsg->GetSocketID(), pNetMsg->GetType(), pNetMsg->GetBuf(), pNetMsg->GetSize());
//	else
//		return SendNpcMsg(pNetMsg->GetNpcID(), pNetMsg->GetType(), pNetMsg->GetBuf(), pNetMsg->GetSize());
//}

bool CWorldKernel::SendClientMsg(SOCKET_ID idSocket, Msg* pMsg)
{
	// TODO: 请在此添加NPC服务器下传消息的代码
	if (!pMsg)
	{
		::LogSave("Error: null msg point found in CGameSocket::SendMsg.");
		return false;
	}
	try {
		MESSAGESTR	bufPacket;
		SENDCLIENTMSG_PACKET0* pPacket = (SENDCLIENTMSG_PACKET0*)bufPacket;
		pPacket->Create(idSocket, pMsg);
		m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_SENDCLIENTMSG, STRUCT_TYPE(bufPacket), &bufPacket);
		return true;
	}
	catch(...)
	{
		ASSERT(!"catch");
		::LogSave("Error: exception catched in CGameSocket::SendMsg().");
		return false;
	}			

	return true;
}

bool CWorldKernel::SendNpcMsg(OBJID idNpc, Msg* pMsg)
{
	// TODO: 请在此添加NPC服务器下传消息的代码
	if (!pMsg)
	{
		::LogSave("Error: null msg point found in CGameSocket::SendMsg.");
		return false;
	}
	try {
		MESSAGESTR	bufPacket;
		SENDNPCMSG_PACKET0* pPacket = (SENDNPCMSG_PACKET0*)bufPacket;
		pPacket->Create(idNpc, pMsg);
		m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_SENDNPCMSG, STRUCT_TYPE(bufPacket), &bufPacket);
		return true;
	}
	catch(...)
	{
		ASSERT(!"catch");
		::LogSave("Error: exception catched in CGameSocket::SendMsg().");
		return false;
	}			
	return true;
}

bool CWorldKernel::CloseSocket(SOCKET_ID idSocket)			// 直接关闭socket
{
	// TODO: 请在此添加关闭客户端SOCKET的代码
	m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_BREAKCONNECT, VARTYPE_INT, &idSocket);

	if(idSocket == m_idNpcSocket)
	{
		LOGMSG("CWorldKernel::CloseSocket break npc connect!", idSocket);
		m_idNpcSocket	= SOCKET_NONE;		//@???
	}

	return true;
}

bool CWorldKernel::PrintText			(LPCTSTR szText)
{
	return m_pMsgPort->Send(MSGPORT_SHELL, SHELL_PRINTTEXT, STRING_TYPE(szText), szText);
}

bool CWorldKernel::ChangeCode(SOCKET_ID idSocket, DWORD dwData)
{
	CHANGE_USERDATA	st;
	st.idSocket		= idSocket;
	st.nData		= dwData;
	return m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_CHANGECODE, STRUCT_TYPE(CHANGE_USERDATA), &st);
}
/////////////////
// account server/////////////////
bool CWorldKernel::SendFee(OBJID idAccount, int nType)
{
	MsgFee msg;
	msg.Create(idAccount,nType);
	return m_sock.SendPacket((LPCTSTR)&msg, msg.GetSize(), true);
}

bool CWorldKernel::SendQueryFee(OBJID idAccount, DWORD dwData, int nTime, int ucType)
{
	MsgQueryFee msg;
	msg.Create(idAccount,ucType,dwData,nTime);
	return m_sock.SendPacket((LPCTSTR)&msg, msg.GetSize(), true);
}
/////////////////
void CWorldKernel::ProcessAccountNetMsg()
{
	bool	bGetData = true;
	while(true)
	{
		int	nNetSize = 0;
		Msg* pMsg = (Msg*)m_sock.GetPacket(&nNetSize, bGetData);
		if(!pMsg || nNetSize == 0)
			break;
		int nMsgSize = *((unsigned short*)pMsg);
		if(nNetSize < nMsgSize)
			break;

		switch(pMsg->GetType())
		{
		case _MSG_LOGIN:// 服务器登录 ////////////////////////////////////////
			{
				MsgAccountLogin* pMsgLogin = (MsgAccountLogin*)pMsg;
				CHECK(pMsgLogin->GetSize() == sizeof(MsgAccountLogin));
				ProcessLogin(*pMsgLogin);
				m_sock.ClearPacket(nMsgSize);
			}
			break;
			// 玩家部分 ////////////////////////////////////////
		case _MSG_CONNECT:
			{
				MsgConnect*	pMsgConnect = (MsgConnect*)pMsg;
				CHECK(pMsgConnect->GetSize() == sizeof(MsgConnect));
				if(ProcessConnect(*pMsgConnect))
				{
					CPlayer* pPlayer	= g_UserList.GetPlayerByAccountID(pMsgConnect->m_idAccount);
					ASSERT(pPlayer);
					if(pPlayer)
						UserList()->LoginToMapGroup(pPlayer);
				}
				m_sock.ClearPacket(nMsgSize);
			}
			break;
		case _MSG_FEE:
			{
				MsgFee*	pMsgFee = (MsgFee*)pMsg;
				CHECK(pMsgFee->GetSize() == sizeof(MsgFee));
				ProcessFee(*pMsgFee);
				m_sock.ClearPacket(nMsgSize);
			}
			break;
		case _MSG_QUERYFEE:
			{
				MsgQueryFee* pMsgQFee = (MsgQueryFee*)pMsg;
				CHECK(pMsgQFee->GetSize() == sizeof(MsgQueryFee));
				ProcessQueryFee(*pMsgQFee);
				m_sock.ClearPacket(nMsgSize);
			}
			break;
		default:
			bGetData = false;
			break;
		}
	} // for
}
/////////////////
void CWorldKernel::ProcessLogin(const MsgAccountLogin& msgLogin)
{
	if(msgLogin.m_idAccount == ID_NONE)
	{
		PrintText("Account server login failed!");
		return;
	}

	//??? 未检查版本号
	m_idServer	= msgLogin.m_idAccount;
	PrintText("Account server login OK。");
	m_sock.SendPacket((LPCTSTR)&msgLogin, msgLogin.GetSize(), true);
	m_nState = STATE_NORMAL;
}
/////////////////
bool CWorldKernel::ProcessConnect(const MsgConnect& msgConnect)			// return true: pass barrier
{
	extern DWORD g_dwMaxPlayer;
	if(GameWorld()->QueryUserList()->GetUserAmount() > g_dwMaxPlayer && msgConnect.m_idAccount > MAX_GM_ID)
	{
		GameWorld()->SendFee(msgConnect.m_idAccount, MsgFee::SERVER_FULL);		// 未上传SERVER_BUSY
		return false;
	}

	CConnectUser* pConnect = m_setConnectClient->GetObj(msgConnect.m_idAccount);
	if(pConnect)
	{
		if(pConnect->IsValid() && pConnect->CheckData(msgConnect.m_uData))	// same id, same chkdata
		{
			SOCKET_ID	idSocket = pConnect->GetSocketID();

			m_setConnectClient->DelObj(msgConnect.m_idAccount);

			// check repeat user_id
			if(UserList()->GetPlayerByAccountID(msgConnect.m_idAccount))
			{
				LOGMSG("DEBUG：帐号[%u]重复登录，踢前一人下线 !", msgConnect.m_idAccount);
				UserList()->KickOutAccount(msgConnect.m_idAccount, STR_LOGIN_AGAIN);
				return false;
			}

			// OK
			int ret = UserList()->LoginUser(msgConnect.m_idAccount, idSocket, msgConnect.m_szInfo);		// enum { LOGIN_OK, LOGIN_NEW, LOGIN_BAN };
			if (ret == LOGIN_OK)
				return true;
			else if(ret == LOGIN_BAN)
			{
				MsgTalk msg;
				if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, "错误：余额不足，请及时充值!", NULL, _COLOR_WHITE, _TXTATR_ENTRANCE))
					SendClientMsg(idSocket, &msg);
				return false;
			}
			else	// LOGIN_NEW new account, no player created
			{
				MsgTalk msg;
				if (msg.Create(SYSTEM_NAME, ALLUSERS_NAME, NEW_ROLE_STR, NULL, _COLOR_WHITE, _TXTATR_ENTRANCE))
				{
					SendClientMsg(idSocket, &msg);
				}
			}
		}
		else	// same id, but wrong chkdata
		{
			m_setConnectClient->DelObj(msgConnect.m_idAccount);
		}
	}

	pConnect = CConnectUser::CreateNew();
	CHECKF(pConnect->Create(msgConnect.m_idAccount, msgConnect.m_uData, msgConnect.m_szInfo));
	m_setConnectAccount->DelObj(msgConnect.m_idAccount);					// 冲掉，防止WARNING输出
	m_setConnectAccount->AddObj(pConnect);
	
	return false;
}
/////////////////
bool CWorldKernel::ClientConnect(OBJID idAccount, DWORD dwData, LPCTSTR szInfo, SOCKET_ID idSocket)			// return true: pass barrier
{
	CHECKF(idSocket != SOCKET_NONE);

	CConnectUser* pConnect = m_setConnectAccount->GetObj(idAccount);
	if(pConnect)
	{
		NAMESTR	info;
		SafeCopy(info, pConnect->GetInfo(), _MAX_NAMESIZE);

		if(pConnect->IsValid() && pConnect->CheckData(dwData))	// same id, same chkdata
		{
			m_setConnectAccount->DelObj(idAccount);

			// check repeat user_id
			if(UserList()->GetPlayerByAccountID(idAccount))
			{
				LOGMSG("DEBUG：帐号[%u]重复登录，踢前一人下线!", idAccount);
				UserList()->KickOutAccount(idAccount, STR_LOGIN_AGAIN);
				return false;
			}

			// OK
			int ret = UserList()->LoginUser(idAccount, idSocket, info);		// enum { LOGIN_OK, LOGIN_NEW, LOGIN_BAN };
			if (ret == LOGIN_OK)
				return true;
			else if(ret == LOGIN_BAN)
			{
				MsgTalk msg;
				if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, "错误：余额不足，请及时充值!", NULL, _COLOR_WHITE, _TXTATR_ENTRANCE))
					SendClientMsg(idSocket, &msg);
				return false;
			}
			else	// LOGIN_NEW new account, no player created
			{
				MsgTalk msg;
				if (msg.Create(SYSTEM_NAME, ALLUSERS_NAME, NEW_ROLE_STR, "", _COLOR_WHITE, _TXTATR_ENTRANCE))
				{
					SendClientMsg(idSocket, &msg);
				}
			}
		}
		else
		{
			m_setConnectAccount->DelObj(idAccount);
		}
	}

	pConnect = CConnectUser::CreateNew();
	CHECKF(pConnect->Create(idAccount, dwData, szInfo, idSocket));
	m_setConnectClient->DelObj(idAccount);			// 防止WARNING
	m_setConnectClient->AddObj(pConnect);

	return false;
}
/////////////////
void CWorldKernel::ProcessFee(const MsgFee& msgFee)
{
	switch(msgFee.m_ucType)
	{
	case	MsgFee::HEARTBEAT:
		SendFee(msgFee.m_idAccount, MsgFee::HEARTBEAT);
		break;
	case	MsgFee::FEE_KICKOUT:
		LOGMSG("DEBUG：帐号服务器踢出帐号[%u]", msgFee.m_idAccount);
		if(!UserList()->KickOutAccount(msgFee.m_idAccount, STR_LOGIN_ANOTHER))		// 不在线
			SendFee(msgFee.m_idAccount, MsgFee::FEE_OFFLINE);
		break;
	default:
		break;
	}

}
/////////////////
enum { c_typeNone = 0, c_typePoint, c_typeTime, c_typeNetBarPoint, c_typeNetBarTime, c_typeISP, c_typeFree, c_typeAll };
void CWorldKernel::ProcessQueryFee(const MsgQueryFee& msgQueryFee)
{
	CPlayer*  pUser	=UserList()->GetPlayerByAccountID(msgQueryFee.m_idAccount);
	if(pUser)
	{
		char szMsg[1024] = STR_FEETYPE_KNOWN;
		switch(msgQueryFee.m_ucType)
		{
		case	c_typePoint:
			sprintf(szMsg, STR_FEETYPE_HOUR, msgQueryFee.m_uData/20, (msgQueryFee.m_uData/2)%10);
			break;
		case	c_typeTime:
			sprintf(szMsg, STR_FEETYPE_MONTH, msgQueryFee.m_nTime/10000, (msgQueryFee.m_nTime/100) % 100, msgQueryFee.m_nTime % 100);
			break;
		case	c_typeNetBarPoint:
			sprintf(szMsg, STR_FEETYPE_BARHOUR, msgQueryFee.m_uData);
			break;
		case	c_typeNetBarTime:
			sprintf(szMsg, STR_FEETYPE_BARMONTH, msgQueryFee.m_uData);
			break;
		case	c_typeISP:
			sprintf(szMsg, STR_FEETYPE_ISP, msgQueryFee.m_uData);
			break;
		case	c_typeFree:
			sprintf(szMsg, STR_FEETYPE_FREE, msgQueryFee.m_uData);
			break;
		}

		CMsgTalk	msg;
		IF_OK(msg.Create(SYSTEM_NAME, pUser->szName, szMsg))
			pUser->SendMsg(&msg);

		szMsg[0]	= 0;
		if(strcmp(pUser->szNotify, FLAG_ISP_TO_NORMAL) == 0)			// ★注意：该字符串必须与帐号服务器同步
			SafeCopy(szMsg, STR_LOGIN_ISP_FAIL);
		else if(strcmp(pUser->szNotify, FLAG_NO_POINT) == 0)
			SafeCopy(szMsg, STR_ACCOUNT_NOFEE);
		else if(pUser->szNotify[0] >= '0' && pUser->szNotify[0] <= '9')
			sprintf(szMsg, STR_FEW_FEE_NOTIFY, pUser->szNotify);
		else
			SafeCopy(szMsg, pUser->szNotify, _MAX_NAMESIZE);
		if(strlen(szMsg) > 0)
		{
			CMsgTalk	msg;
			IF_OK(msg.Create(SYSTEM_NAME, pUser->szName, szMsg))
				pUser->SendMsg(&msg);
		}
	}
	else
		LOGERROR("帐号服务器返回了未登录玩家的查询计费消息");
}
/////////////////
// global entry/////////////////
IWorld* IWorld::CreateNew()
{
	return (IWorld*)(new CWorldKernel);
}

// syndicate
bool CWorldKernel::CreateSyndicate		(const CreateSyndicateInfo* pInfo)
{
	for(int i = MSGPORT_MAPGROUP_FIRST; i < GameWorld()->GetMsgPort()->GetSize(); i++)
	{
		m_pMsgPort->Send(i, KERNEL_CREATESYNDICATE, STRUCT_TYPE(CreateSyndicateInfo), pInfo);
	}
	return true;
}

bool CWorldKernel::DestroySyndicate	(OBJID idSyn, OBJID idTarget)
{
	OBJID	setID[2];
	setID[0]	= idSyn;
	setID[1]	= idTarget;
	for(int i = MSGPORT_MAPGROUP_FIRST; i < GameWorld()->GetMsgPort()->GetSize(); i++)
	{
		m_pMsgPort->Send(i, KERNEL_DESTROYSYNDICATE, STRUCT_TYPE(setID), &setID);
	}
	return true;
}

bool CWorldKernel::CombineSyndicate(OBJID idSyn, OBJID idTarget)
{
	OBJID	setID[2];
	setID[0]	= idSyn;
	setID[1]	= idTarget;
	for(int i = MSGPORT_MAPGROUP_FIRST; i < GameWorld()->GetMsgPort()->GetSize(); i++)
	{
		m_pMsgPort->Send(i, KERNEL_COMBINESYNDICATE, STRUCT_TYPE(setID), &setID);
	}
	return true;
}

bool CWorldKernel::ChangeSyndicate		(const SynFuncInfo0* pFuncInfo)
{
	for(int i = MSGPORT_MAPGROUP_FIRST; i < GameWorld()->GetMsgPort()->GetSize(); i++)
	{
		m_pMsgPort->Send(i, KERNEL_CHANGESYNDICATE, BUFFER_TYPE(pFuncInfo->nSize), pFuncInfo);
	}
	return true;
}

enum { CONFIGDATA_NAME=1, CONFIGDATA_DATA, CONFIGDATA_STRING };
bool CWorldKernel::LoadConfig(IDatabase *pDb)
{
	extern long	g_sKickoutCheat;

	char	szSQL[1024];
	sprintf(szSQL, "SELECT * FROM %s", _TBL_CONFIG);
	CAutoPtr<IRecordset> pRes = pDb->CreateNewRecordset(szSQL, false);
	CHECKF(pRes);
	for(int i = 0; i < pRes->RecordCount(); i++, pRes->MoveNext())
	{
		if(stricmp(pRes->GetStr(CONFIGDATA_NAME), "kickoutcheat") == 0)
			InterlockedExchange(&g_sKickoutCheat, pRes->GetInt(CONFIGDATA_DATA));
	}

	return true;
}

bool CWorldKernel::SendMapGroupMsgForChatRoom(int nMapGroup,SOCKET_ID idSocket,Msg *pMsg)
{
	ASSERT(idSocket != SOCKET_NONE);
	MESSAGESTR	buf;
	TRANSMSG_PACKET0* pPacket = (TRANSMSG_PACKET0*)buf;
	pPacket->Create(idSocket, ID_NONE, pMsg->GetTransData(), pMsg);
	CHECKF( nMapGroup >= MSGPORT_MAPGROUP_FIRST && nMapGroup < m_pMsgPort->GetSize());
	m_pMsgPort->Send(nMapGroup, MAPGROUP_TRANSMITMSG_FORCHATROOM, STRUCT_TYPE(buf), pPacket);

	return true;
}
