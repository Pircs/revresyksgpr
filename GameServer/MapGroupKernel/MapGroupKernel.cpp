#include "MessagePort.h"
#include "inifile.h"
#include "protocol.h"
#include "MapGroupKernel.h"
#include "NetMsg.h"
#include "SharedBaseFunc.h"
#include "MapGroup.h"
#include "Npc.h"
#include "Agent.h"
//#include "MsgSyndicate.h"
#include "DeadLoop.h"
#include "I_mydb.h"
#include "AllMSG.h"

MYHEAP_IMPLEMENTATION(CMapGroupKernel,s_heap)

// interface

bool CMapGroupKernel::Create(IMessagePort* pPort)
{
	m_pMsgPort		= pPort;
	m_idProcess		= m_pMsgPort->GetID();		// process id == msg_port_id
	m_pMsgPort->Open();
	ASSERT(m_idProcess >= MSGPORT_MAPGROUP_FIRST);

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
	m_pDb = ::CreateDatabase(DB_IP, DB_USER, DB_PW, DB_DB);
	if(!m_pDb)
	{
		LOGERROR("数据库连接失败!");
		return false;
	}

	// TODO: 请在此添加初始化代码
	CMapGroup::AddMapGroup(m_pMsgPort->GetID());
	if(!MapGroup(m_pMsgPort->GetID())->Create(pPort->GetID(), this, m_pDb, m_pMsgPort))
	{
		MapGroup(m_pMsgPort->GetID())->Destroy();
		return false;
	}

	return true;		// return false : 创建失败，程序关闭。
}

bool CMapGroupKernel::Release()
{
	// TODO: 请在此添加代码
	MapGroup(m_pMsgPort->GetID())->Destroy();
	S_REL(m_pDb);

	delete this;
	return true;		// return false : 无意义。
}

bool CMapGroupKernel::ProcessMsg(OBJID idPacket, void* buf, int nType, int nFrom)
{
	switch(idPacket)
	{
	case	KERNEL_CLIENTMSG:
		{
			CLIENTMSG_PACKET0*	pMsg = (CLIENTMSG_PACKET0*)buf;
			DEADLOOP_CHECK(PID, "KERNEL_CLIENTMSG: ")
			ProcessClientMsg(pMsg->idSocket, pMsg->msgBuf);
		}
		break;
	case MAPGROUP_TRANSMITMSG_FORCHATROOM://聊天室消息
		{
			TRANSMSG_PACKET0*	pMsg = (TRANSMSG_PACKET0*)buf;


			DEADLOOP_CHECK(PID, "MAPGROUP_TRANSMITMSG: ")
			if(pMsg->idSocket != SOCKET_NONE)
			{
				ProcessClientMsg(pMsg->idSocket, pMsg->msgBuf, pMsg->nTrans);
			}
		}
		break;
	case	MAPGROUP_TRANSMITMSG:
		{
			TRANSMSG_PACKET0*	pMsg = (TRANSMSG_PACKET0*)buf;
			DEADLOOP_CHECK(PID, "MAPGROUP_TRANSMITMSG: ")
			if(pMsg->idSocket == SOCKET_NONE)
			{
				if(UserManager()->GetUser(pMsg->idNpc) == NULL)
					ProcessNpcMsg(pMsg->idNpc, pMsg->msgBuf, pMsg->nTrans);
			}
			else
			{
				if(UserManager()->GetUserBySocketID(pMsg->idSocket) == NULL)			//??? 玩家不在此地图组，防止自激!!!
					ProcessClientMsg(pMsg->idSocket, pMsg->msgBuf, pMsg->nTrans);
			}
		}
		break;
	case	MAPGROUP_REMOTECALL:
		{
			REMOTE_CALL0*	pInfo = (REMOTE_CALL0*)buf;
			CUser*	pUser = UserManager()->GetUser(pInfo->idUser);
			if(pUser)
			{
				DEADLOOP_CHECK(PID, "MAPGROUP_REMOTECALL: ")
				pUser->RemoteCall(pInfo);
			}
		}
		break;
	case	MAPGROUP_RPC:
		{
			DEBUG_TRY
			int	nMsgSize = nType;
			CEventPack	pack((char*)buf, nMsgSize);
			switch(pack.GetEventType())
			{
			case	OBJ_MAP:
				{
					CGameMap* pMap = MapManager()->QueryMap(pack.GetObjID());
					if(pMap)
						pMap->ProcessRpc(pack);
				}
				break;
			default:
				ASSERT(!"switch(MAPGROUP_RPC)");
			}
			DEBUG_CATCH("MAPGROUP_RPC")
		}
		break;
	case	KERNEL_NPCMSG:
		{
			NPCMSG_PACKET0*	pMsg = (NPCMSG_PACKET0*)buf;
			DEADLOOP_CHECK(PID, "KERNEL_NPCMSG: ")
			ProcessNpcMsg(pMsg->idNpc, pMsg->msgBuf);
		}
		break;
	// TODO: 请在此添加内部消息处理代码
	case	KERNEL_CLOSEKERNEL:
		{
			SOCKET_ID	idSocket = *(SOCKET_ID*)buf;
			LOGDEBUG("DEBUG：地图组核心[%d]收到关闭核心消息，SOCKET_ID[%u]", PID, idSocket);

			DEBUG_TRY
			DEADLOOP_CHECK(PID, "KERNEL_CLOSEKERNEL: ")
#ifdef	NEW_LOGOUT
			if(UserManager()->IsLoginMapGroup(idSocket))	  // readme.txt(3-1), 否则不操作
			{
				g_UserManager.LogoutUser(idSocket);
				m_pMsgPort->Send(MSGPORT_WORLD, KERNEL_CLOSEKERNEL, VARTYPE_INT, &idSocket);	  // readme.txt(3-2)
			}
			else
			{
				m_pMsgPort->Send(m_idProcess-1, KERNEL_CLOSEKERNEL, VARTYPE_INT, &idSocket);	  // readme.txt(3-1)
			}
#else
			if(UserManager()->IsLoginMapGroup(idSocket))	  // readme.txt(3-1), 否则不操作
			{
				g_UserManager.LogoutUser(idSocket);
				m_pMsgPort->Send(MSGPORT_WORLD, WORLD_CLOSESOCKET, VARTYPE_INT, &idSocket);	  // readme.txt(3-2)
			}
#endif
			DEBUG_CATCH("KERNEL_CLOSEKERNEL")
		}
		break;
	case	MAPGROUP_LOGIN:
		{
			ST_LOGIN* pPacket = (ST_LOGIN*)buf;

			if(pPacket->idSocket == SOCKET_NONE)	// agent
			{
				DEBUG_TRY	// VVVVVVVVVVVVVV
				DEADLOOP_CHECK(PID, "MAPGROUP_LOGIN agent: ")
				CAgent* pAgent = UserManager()->CreateAgent(pPacket->idUser);
				IF_NOT(pAgent)
					break;

				CHANGE_NPCPROCESSID	buf;
				buf.idNpc		= pPacket->idUser;
				buf.idProcess	= m_idProcess;
				m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_SETNPCPROCESSID, STRUCT_TYPE(CHANGE_NPCPROCESSID), &buf);

				UserManager()->LoginAgent(pAgent);
				DEBUG_CATCH("LoginAgent")	//AAAAAAAAAAAAAAAAA@ temporary
			}
			else
			{
				DEBUG_TRY	// VVVVVVVVVVVVVV
				DEADLOOP_CHECK(PID, "MAPGROUP_LOGIN pUser: ")
				CUser* pUser = UserManager()->CreateUser(pPacket->idSocket, pPacket->idUser);
				IF_NOT(pUser)
					break;

				//? 必须先转PROCESS_ID
				CHANGE_USERDATA	obj;
				obj.idSocket	= pPacket->idSocket;
				obj.nData		= m_idProcess;
				m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_SETPROCESSID, STRUCT_TYPE(CHANGE_USERDATA), &obj);		// readme.txt(3-6)
				//@ 不需要再通知WORLDKERNEL(已经修改过了)

				g_UserManager.LoginUser(pPacket->idSocket, pUser);

				MapGroup(PID)->QueryMsgPort()->Send(MSGPORT_WORLD, WORLD_SENDTIME, VARTYPE_INT, &pPacket->idSocket);
				DEBUG_CATCH("LoginUser")	//AAAAAAAAAAAAAAAAA@ temporary
			}
		}
		break;
	case	MAPGROUP_BROADCASTMSG:
		{
			CLIENTMSG_PACKET0* pMsg = (CLIENTMSG_PACKET0*)buf;
			DEADLOOP_CHECK(PID, "MAPGROUP_BROADCASTMSG: ")
			BroadcastMapGroupMsg(pMsg->msgBuf);
		}
		break;

	case	MAPGROUP_SOCKETUSERINFO:			// 1: create user
		{
			ST_USERCHANGEMAPGORUP* pMsg = (ST_USERCHANGEMAPGORUP*)buf;
			DEADLOOP_CHECK(PID, "MAPGROUP_SOCKETUSERINFO: ")

			UserManager()->GetForMapGroup()->JoinMapGroup(pMsg->idSocket, &pMsg->info);

			//? 必须先通知, 再回送FLY消息
			CHANGE_USERDATA	st;
			st.idSocket		= pMsg->idSocket;
			st.nData		= m_idProcess;
			m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_SETPROCESSID, STRUCT_TYPE(CHANGE_USERDATA), &st);   // readme.txt (3-6)
			m_pMsgPort->Send(MSGPORT_WORLD, WORLD_SETPROCESSID, STRUCT_TYPE(CHANGE_USERDATA), &st);
		}
		break;

	case	MAPGROUP_SENDOBJINFO:				// 2: append info
		{
			CHANGEMAPGORUP_INFO0*	pInfo = (CHANGEMAPGORUP_INFO0*)buf;
			DEADLOOP_CHECK(PID, "MAPGROUP_SENDOBJINFO: ")

			CUser* pUser = UserManager()->GetUser(pInfo->idUser);
			if(pUser)
				pUser->AppendObjInfo((INFO_ID)pInfo->nInfoType, pInfo->info);
		}
		break;
	case	MAPGROUP_CHANGEMAPGROUP:			// 3: join map
		{
			ST_CHANGEMAPGROUP* pPacket = (ST_CHANGEMAPGROUP*)buf;
			DEADLOOP_CHECK(PID, "MAPGROUP_CHANGEMAPGROUP: ")

			CUser* pUser = UserManager()->GetUser(pPacket->idUser);
			if(pUser)
				pUser->JoinMap(pPacket->idMap, pPacket->nPosX, pPacket->nPosY);
		}
		break;
	case	MAPGROUP_DELALLMONSTER:
		{
			DEADLOOP_CHECK(PID, "DelAllMonster")
			NpcManager()->DelAllMonster();
		}
		break;
	case	MAPGROUP_LOADALLPET:
		{
			DEADLOOP_CHECK(PID, "LoadAllPet")
			RoleManager()->LoadAllPet();
		}
		break;
	case	MAPGROUP_CREATENEWNPC:
		{
			DEADLOOP_CHECK(PID, "MAPGROUP_CREATENEWNPC")
			ST_CREATENEWNPC* pPacket = (ST_CREATENEWNPC*)buf;
			NpcManager()->CreateMonster(pPacket);
		}
		break;
	case	MAPGROUP_CHANGETEAM:
		{
			ST_CHANGETEAM* pPacket = (ST_CHANGETEAM*)buf;
			switch(pPacket->nAction)
			{
			case	CHANGETEAM_ADD:
				{
					DEADLOOP_CHECK(PID, "MAPGROUP_CHANGETEAM: CHANGETEAM_ADD")
					CUserManager::Iterator pUser = UserManager()->NewEnum();
					while(pUser.Next())
					{
						if(pUser && pUser->GetTeam() && pUser->GetTeam()->GetID() == pPacket->idTeam)
							pUser->QueryChangeTeam()->AddTeamMember(pPacket->idUser);
					}
				}
				break;
			case	CHANGETEAM_DEL:
				{
					DEADLOOP_CHECK(PID, "MAPGROUP_CHANGETEAM: CHANGETEAM_DEL")
					CUserManager::Iterator pUser = UserManager()->NewEnum();
					while(pUser.Next())
					{
						if(pUser && pUser->GetTeam() && pUser->GetTeam()->GetID() == pPacket->idTeam)
							pUser->QueryChangeTeam()->DelTeamMember(pPacket->idUser);
					}
				}
				break;
			case	CHANGETEAM_DISMISS:
				{
					DEADLOOP_CHECK(PID, "MAPGROUP_CHANGETEAM: CHANGETEAM_DISMISS")
					CUserManager::Iterator pUser = UserManager()->NewEnum();
					while(pUser.Next())
					{
						if(pUser && pUser->GetTeam() && pUser->GetTeam()->GetID() == pPacket->idTeam)
							pUser->QueryChangeTeam()->DelTeam();
					}
				}
				break;
			case	CHANGETEAM_INFO:
				{
					DEADLOOP_CHECK(PID, "MAPGROUP_CHANGETEAM: CHANGETEAM_INFO")
					SOCKET_ID	idSocket	= pPacket->nData;
					OBJID		idAgent		= pPacket->idUser;
					CUserManager::Iterator pUser = UserManager()->NewEnum();
					while(pUser.Next())
					{
						if(pUser && pUser->GetTeam() && pUser->GetTeam()->GetID() == pPacket->idTeam)
							pUser->QueryChangeTeam()->SendInfo(idSocket, idAgent);
					}
				}
				break;
			case	CHANGETEAM_FLY:
				{
					DEADLOOP_CHECK(PID, "MAPGROUP_CHANGETEAM: CHANGETEAM_FLY")
					OBJID	idMap = pPacket->idTeam;
					CUser* pUser = UserManager()->GetUser(pPacket->idUser);
					if(pUser && pUser->IsAlive())
						pUser->FlyMap(idMap, LOWORD(pPacket->nData), HIWORD(pPacket->nData));
				}
				break;
			}
		}
		break;
	// syndicate -------------------------------------------------------------------------------
	case	KERNEL_CREATESYNDICATE:
		{
			DEADLOOP_CHECK(PID, "KERNEL_CREATESYNDICATE")
			CreateSyndicateInfo* pPacket = (CreateSyndicateInfo*)buf;
			SynManager()->QuerySynchro()->CreateSyndicate(pPacket);
		}
		break;
	case	KERNEL_DESTROYSYNDICATE:
		{
			DEADLOOP_CHECK(PID, "KERNEL_DESTROYSYNDICATE")
			OBJID*	pId		= (OBJID*)buf;
			OBJID	idSyn	= pId[0];
			OBJID	idTarget= pId[1];
			SynManager()->QuerySynchro()->DestroySyndicate(idSyn, idTarget);
		}
		break;
	case	KERNEL_COMBINESYNDICATE:
		{
			DEADLOOP_CHECK(PID, "KERNEL_COMBINESYNDICATE")
			OBJID*	pId		= (OBJID*)buf;
			OBJID idSyn		= pId[0];
			OBJID idTarget	= pId[1];
			SynManager()->QuerySynchro()->CombineSyndicate(idSyn, idTarget);
		}
		break;
	case	KERNEL_CHANGESYNDICATE:
		{
			DEADLOOP_CHECK(PID, "KERNEL_CHANGESYNDICATE")
			SynFuncInfo0* pPacket = (SynFuncInfo0*)buf;
			CSyndicate* pSyn = SynManager()->QuerySyndicate(pPacket->idSyn);
			if(pSyn)
				pSyn->ChangeSyndicate(pPacket);
		}
		break;
	case	MAPGROUP_SETMAPSYNID:
		{
			DEADLOOP_CHECK(PID, "MAPGROUP_SETMAPSYNID")
			OBJID*	pId		= (OBJID*)buf;
			OBJID idMap		= pId[0];
			OBJID idSyn		= pId[1];
			SynManager()->QuerySynchro()->SetMapSynID(idMap, idSyn);
		}
		break;
	case	MAPGROUP_CHANGENPC:
		{
			DEADLOOP_CHECK(PID, "MAPGROUP_CHANGENPC")
			char* ptr = (char*)buf;
			OBJID idNpc = *(int*)ptr;
			char* pszField = ptr + sizeof(int);
			char* pszData = pszField + strlen(pszField) + 1;
			int	  nData = atol(pszData);
			bool	bUpdate = UPDATE_FALSE;

			IRole*	pRole = RoleManager()->QueryRole(idNpc);
			if(!pRole)
				return false;

			CNpc*	pTarget;
			IF_NOT(pRole->QueryObj(OBJ_NPC, IPP_OF(pTarget)))
				return false;

			if(stricmp(pszField, "lookface") == 0)
			{
				//if(strcmp(szOpt, "=") == 0)
				{
					pTarget->SetInt(NPCDATA_LOOKFACE, nData, bUpdate);
					return true;
				}
			}
			else if(stricmp(pszField, "datastr") == 0)
			{
				//if(strcmp(szOpt, "=") == 0)
				{
					pTarget->SetStr(NPCDATA_DATASTR, pszData, _MAX_NAMESIZE, bUpdate);
					return true;
				}
			}
			else if(strnicmp(pszField, "data", 4) == 0)
			{
				int idx = (*(pszField+4) - '0');
				if(idx < 0 || idx >= MAX_NPCDATA)
					return false;
				//if(strcmp(szOpt, "=") == 0)
				{
					pTarget->SetData(idx, nData, bUpdate);
					return true;
				}
			}
		}
		break;
	case	MAPGROUP_DELTRANSNPC:
		{
			DEADLOOP_CHECK(PID, "MAPGROUP_DELTRANSNPC")
			OBJID idMastNpc = *(OBJID*)buf;
			MapGroup(PID)->QueryIntraMsg()->DelTransNpc(idMastNpc, false);		// false: no brocast to other mapgroup
		}
		break;
	case	KERNEL_SUPERMANLIST:
		{
			DEADLOOP_CHECK(PID, "KERNEL_SUPERMANLIST")
			VARIANT_SET0* pInfo = (VARIANT_SET0*)buf;
			CUser* pUser = UserManager()->GetUser(pInfo->IntParam(0));
			if(pUser)
				pUser->SynchroSupermanOrder(pInfo->IntParam(1));
		}
		break;

	default:
		ASSERT(!"CWorldKernel::ProcessMsg()");
	}

	return true;		// return false : 消息处理异常，程序关闭。
}

bool CMapGroupKernel::OnTimer(time_t tCurr)
{
	// TODO: 请在此添加时钟处理代码
	MapGroup(PID)->OnTimer(tCurr);

	return true;		// return false : 消息处理异常，程序关闭。
}

bool CMapGroupKernel::ProcessMsg(SOCKET_ID idSocket, OBJID idNpc, const Msg* pMsg, int nTrans)
{
	try {
		CMapGroup* pMapGroup  = MapGroup(m_idProcess);
		if (pMapGroup)
		{
			CMapGroup& mapGroup = *pMapGroup;

			CUser* pUser = mapGroup.GetUserManager()->GetUser(idSocket, idNpc);
			// msg count
			//CUser* pUser = UserManager()->GetUser(pMsg);
			if (pUser)
				pUser->m_dwMsgCount++;

			// msg process
			//pMsg->Process(this);
			switch(pMsg->GetType())
			{
			case _MSG_TALK:
				((CMsgTalk*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_MESSAGEBOARD:
				((CMsgMessageBoard*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_ACTION:
				((CMsgAction*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_PLAYER:
				((CMsgPlayer*)pMsg)->Process(pUser);
				break;
			case _MSG_TICK:
				((CMsgTick*)pMsg)->Process(pUser);
				break;
			case _MSG_ITEM:
				((CMsgItem*)pMsg)->Process(pUser);
				break;
			case _MSG_FRIEND:
				((CMsgFriend*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_INTERACT:
				((CMsgInteract*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
				//case _MSG_USERATTRIB:
				///	((CMsgUserAttrib*)pMsg)->Process(mapGroup, idSocket, idNpc);
				//	break;
			case _MSG_NAME:
				((CMsgName*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_WALK:
				((CMsgWalk*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
				//case _MSG_WALKEX:
				//	((CMsgWalkEx*)pMsg)->Process(mapGroup, idSocket, idNpc);
				//	break;
			case _MSG_MAPITEM:
				((CMsgMapItem*)pMsg)->Process(mapGroup, pUser, idSocket);
				break;
			case _MSG_TRADE:
				((CMsgTrade*)pMsg)->Process(pUser);
				break;
			case _MSG_NPC:
				((CMsgNpc*)pMsg)->Process(mapGroup, pUser);
				break;
			case _MSG_NPCINFO:
				((CMsgNpcInfo*)pMsg)->Process(mapGroup, pUser);
				break;
			case _MSG_PACKAGE:
				((CMsgPackage*)pMsg)->Process(pUser);
				break;
			case _MSG_TASKDIALOG:
				((CMsgDialog*)pMsg)->Process(pUser);
				break;
			case _MSG_ALLOT:
				((CMsgAllot*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_TEAM:
				((CMsgTeam*)pMsg)->Process(pUser);
				break;
				//case _MSG_TEAMMEMBER:
				//	((CMsgTeamMember*)pMsg)->Process(idNpc);
				//	break;
				//case _MSG_MAGICEFFECT:
				//	((CMsgMagicEffect*)pMsg)->Process(mapGroup, idSocket, idNpc);
				//	break;
			case _MSG_SYNDICATE:
				((CMsgSyndicate*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_GEMEMBED:
				((CMsgGemEmbed*)pMsg)->Process(pUser);
				break;
			case _MSG_SYNMEMBERINFO:
				((CMsgSynMemberInfo*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_DICE:
				((CMsgBODice*)pMsg)->Process(pUser);
				break;
			case _MSG_DATAARRAY:
				((CMsgDataArray*)pMsg)->Process(pUser);
				break;
			case _MSG_SCHOOLMEMBER:
				((CMsgSchoolMember*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_PLAYERTASK:
				((CMsgPlayerTask*)pMsg)->Process(mapGroup, pUser);
				break;
			case _MSG_ANNOUNCE_INFO:
				((CMsgAnnounceInfo*)pMsg)->Process(mapGroup, idSocket, idNpc);
				break;
			case _MSG_ANNOUNCE_LIST:
				((CMsgAnnounceList*)pMsg)->Process(mapGroup, pUser);
				break;
			case _MSG_AUCTION:
				((CMsgAuction*)pMsg)->Process(mapGroup, pUser);
				break;
				//	case _MSG_CHATROOM:
				//		((CMsgChatRoom*)pMsg)->Process(mapGroup, idSocket, idNpc);
				//		break;
				//case _MSG_ITEMATTRIB:
				//	((CMsgItemAttrib*)pMsg)->Process(mapGroup, idSocket, idNpc);
				//	break;
			default:
				break;
			}
		}
	}
	catch(...)
	{
		//			CNetMsg::DumpMsg(pMsg);
		ASSERT(!"catch");
		::LogSave("exception catch at CGameSocket::ProcessMsg()! MsgType:%d, SocketID:%u, NpcID:%u", pMsg->GetType(), idSocket, idNpc);
	}
	return true;
}

bool CMapGroupKernel::ProcessClientMsg(SOCKET_ID idSocket, const Msg* pMsg, int nTrans)
{
	// TODO: 请在此添加客户端上传消息的处理代码
	if(!pMsg || pMsg->GetSize() <= 0 || pMsg->GetSize() >_MAX_MSGSIZE)
		return false;
	DEADLOOP_CHECK(PID, "ProcessClientMsg: ")

		clock_t tStart = clock();

	ProcessMsg(idSocket, ID_NONE, pMsg, nTrans);

	clock_t tUsed = clock()-tStart;
	extern struct STAT_STRUCT	g_stat;
	if(tUsed > g_stat.setDebug[0])
	{
		InterlockedExchange(&g_stat.setDebug[0], tUsed);		// debug
		//				InterlockedExchange(&g_stat.setDebug[3], idMsg);		// debug
	}

	return true;
}

bool CMapGroupKernel::BroadcastMapGroupMsg(Msg *pMsg)
{
	DEADLOOP_CHECK(PID, "BroadcastMapGroupMsg: ")
	int nTransmit = 0;
	//CNetMsg* pMsg	=CNetMsg::CreateClientMsg(m_idProcess, idSocket, idMsg, pbufMsg, nSize, nTransmit);
	//if(pMsg)
	{
		try {
			RoleManager()->BroadcastMapGroupMsg(pMsg);
		}
		catch(...)
		{
//			CNetMsg::DumpMsg(pMsg);
			ASSERT(!"catch");
			::LogSave("exception catch at CGameSocket::ProcessMsg()! MsgType:%d", pMsg->GetType());
		}
		//S_DEL(pMsg);
	}

	return true;
}

//bool CMapGroupKernel::SendMsg(Msg* pNetMsg)
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

bool CMapGroupKernel::SendClientMsg(SOCKET_ID idSocket, Msg* pMsg)
{
	// TODO: 请在此添加客户端下传消息的代码
	if (!pMsg)
	{
		::LogSave("Error: null msg point found in CGameSocket::SendMsg.");
		return false;
	}

	if (_MSG_NONE == pMsg->GetType())
	{
		::LogSave("Error: invalid msg type in CGameSocket::SendMsg().");
		return false;
	}

	try {
		char	bufPacket[MAX_MESSAGESIZE];
		SENDCLIENTMSG_PACKET0* pPacket = (SENDCLIENTMSG_PACKET0*)bufPacket;
		pPacket->Create(idSocket, pMsg);
		m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_SENDCLIENTMSG, STRUCT_TYPE(bufPacket), &bufPacket);
		return true;
	}
	catch(...)
	{
		ASSERT(!"catch");
		::LogSave("Error: exception catched in CGameSocket::SendMsg(%d).", pMsg->GetType());
		return false;
	}			

	return true;
}

bool CMapGroupKernel::ProcessNpcMsg(OBJID idNpc, const Msg* pMsg, int nTrans)
{
	// TODO: 请在此添加NPC服务器上传消息的处理代码
	if(!pMsg || pMsg->GetSize() <= 0 || pMsg->GetSize() >_MAX_MSGSIZE)
        return false;
	DEADLOOP_CHECK(PID, "ProcessNpcMsg: ")
	ProcessMsg(SOCKET_NONE, idNpc, pMsg, nTrans);
	return true;
}

bool CMapGroupKernel::SendNpcMsg(OBJID idNpc, Msg* pMsg)
{
	// TODO: 请在此添加NPC服务器下传消息的代码
	if (!pMsg)
	{
		::LogSave("Error: null msg point found in CGameSocket::SendNpcMsg.");
		return false;
	}

	if (_MSG_NONE == pMsg->GetType())
	{
		::LogSave("Error: invalid msg type in CGameSocket::SendNpcMsg().");
		return false;
	}

	try {
		char	bufPacket[MAX_MESSAGESIZE];
		SENDNPCMSG_PACKET0* pPacket = (SENDNPCMSG_PACKET0*)bufPacket;
		pPacket->Create(idNpc, pMsg);
		m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_SENDNPCMSG, STRUCT_TYPE(bufPacket), &bufPacket);
		return true;
	}
	catch(...)
	{
		ASSERT(!"catch");
		::LogSave("Error: exception catched in CGameSocket::SendNpcMsg().");
		return false;
	}			

	return true;
}

bool CMapGroupKernel::CloseSocket(SOCKET_ID idSocket)		// 直接关闭socket
{
	// TODO: 请在此添加关闭客户端SOCKET的代码
	m_pMsgPort->Send(MSGPORT_SOCKET, SOCKET_BREAKCONNECT, VARTYPE_INT, &idSocket);

	return true;
}

/////////////////
void CMapGroupKernel::SynchroData()
{
	CGameMap* pMap = MapManager()->QueryMap(SET_WHITE_SYN);
	if(pMap)
		_SynManager(PID)->QueryModify()->SetMapSynID(pMap->GetID(), pMap->GetOwnerID());
	 pMap = MapManager()->QueryMap(SET_BLACK_SYN);
	if(pMap)
		_SynManager(PID)->QueryModify()->SetMapSynID(pMap->GetID(), pMap->GetOwnerID());
}

/////////////////
// global entry
/////////////////
IMapGroup* IMapGroup::CreateNew()
{
	return (IMapGroup*)(new CMapGroupKernel);
}
