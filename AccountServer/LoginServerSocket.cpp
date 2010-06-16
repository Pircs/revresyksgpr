// huihui 2010.6.16
#include "AllHeads.h"
#include "LoginServerSocket.h"
#include "LoginThread.h"
#include "RC5_321216.h"

#undef	LOCKTHREAD		// 登录线程不需要互斥★
#define	LOCKTHREAD
CRc5_321216		m_cRc5;
//m_cRc5.Rc5InitKey(RC5PASSWORD_KEY);

void CLoginServerSocket::refuseLogin(LPCTSTR szLoginName, int nType, LPCTSTR szText)
{
	MsgConnect cRetMsg;
	cRetMsg.Create(ID_NONE, nType, (char*)szText);
	Send(&cRetMsg);
	ShutDown();		//?? 关闭，让对方先关闭
	LOGACCOUNT("登录不成功[%s]。原因是[%s][%d]", szLoginName, szText, nType);
}

void CLoginServerSocket::allowLogin(OBJID idAccount, DWORD nAuthenID, LPCTSTR szServer)
{
	MsgConnect cRetMsg;
	char	szServerIP[IPSTRSIZE] = "";
	g_pPointThread->GetServerIP(szServerIP, szServer);
	cRetMsg.Create(idAccount, nAuthenID, szServerIP);		//? 下传游戏服务器的IP地址
	Send(&cRetMsg);
	if(!STAT_SERIAL)
		ShutDown();		//?? 关闭，让对方先关闭(注意：如果没收到CMsgConnect消息则不会关闭)
}

bool CLoginServerSocket::processMsgClientAccount(MsgC2SAccount& Msg)
{
#ifdef	RC5ENCRYPT
	m_cRc5.Rc5InitKey(RC5PASSWORD_KEY);
	m_cRc5.Rc5Decrypt(Msg.m_szPassword, MAX_NAMESIZE);
#endif
	FormatStr(Msg.m_szAccount, MAX_NAMESIZE);		// 添尾0，删尾空
	FormatStr(Msg.m_szPassword, MAX_NAMESIZE);		// 添尾0，删尾空
	FormatStr(Msg.m_szServer, MAX_NAMESIZE);		// 添尾0，删尾空

	if(Msg.m_szAccount[0] == 0 || Msg.m_szPassword[0] == 0 || Msg.m_szServer[0] == 0)
		return false;

	char *	pClientIP						= GetPeerIP();
	uint32 nClientIP					= inet_addr(pClientIP);
	DWORD	nAuthenID						= NewAuthenID(nClientIP);
	char *	pErrMsg							= "登录失败";
	char	szNotifyMsg[MAX_NAMESIZE]		= "";
	OBJID	idAccount						= ID_NONE;
	OBJID	idFeeAccount					= ID_NONE;
	char	szFeeAccount[MAX_NAMESIZE]		= "";
	int		nFeeType						= c_typeNone;
	int		nPoint							= 0;
	DWORD	nPointTime						= 0;
	int		nBlock							= false;
	bool	bRejoin							= false;
	char	szOldServerName[SERVERNAMESIZE]	= "";
	char	szOldFeeAccount[MAX_NAMESIZE]	= "";
	char 	szPassword[MAX_NAMESIZE]	= "";		// 无用变量
	OBJID	idOldFeeAccount				= ID_NONE;	// 无用变量
	char	szOldIP[MAX_NAMESIZE]		= "";		// 无用变量

	if(g_pOnlineTable->GetAttr(Msg.m_szAccount, idAccount, szPassword, nAuthenID,			// 仅nAuthenID、szOldServerName、szOldFeeAccount 有意义
		szOldServerName, szOldFeeAccount, idOldFeeAccount, szOldIP))		//??? 在线。该函数可能引起ADDNEW的非原子性操作
	{
		bRejoin	= true;
	}

	if(!GetAccount(Msg.m_szAccount, Msg.m_szPassword, 
		idAccount, nFeeType, nPoint, nPointTime, nBlock))			// 返回值
	{
		//AddBan(nClientIP, pClientIP, Msg.m_szAccount);
		refuseLogin(Msg.m_szAccount, c_errPassword, "帐号名或口令错");
		LOGACCOUNT("玩家[%s]口令[%s]登录。帐号名或口令错, IP地址[%s]", 
			Msg.m_szAccount, Msg.m_szPassword, pClientIP);
		return false;
	}

	// 查封号
	if(nBlock)
	{
		refuseLogin(Msg.m_szAccount, c_errBan, "该帐号被封号");
		return false;
	}

	// 检查非法登录
	if(nFeeType == c_typeNetBarPoint || nFeeType == c_typeNetBarTime || nFeeType == c_typeISP)
	{
		refuseLogin(Msg.m_szAccount, c_errBarPassword, "该帐号不能登录");
		return false;
	}

	if(IsFreeServer(Msg.m_szServer))
	{
		// 免费服务器
		nFeeType		= c_typeFree;
		//		SafeCopy(szNotifyMsg, "欢迎参加测试", MAX_NAMESIZE);
	}
	else if(CheckISP(idAccount, nClientIP, pClientIP, 
		idFeeAccount, szFeeAccount, nPoint, nPointTime, szNotifyMsg))		// 返回值
	{
		// ISP类型
		nFeeType		= c_typeISP;
	}
	else
	{
		idFeeAccount	= idAccount;
		SafeCopy(szFeeAccount, Msg.m_szAccount, MAX_NAMESIZE);
	}

	if(bRejoin)
	{
		// 不允许同时登录到另一台服务器。查服务器名和计费帐号名是否相同
		if(strcmp(Msg.m_szServer, szOldServerName) != 0 
			|| strcmp(szFeeAccount, szOldFeeAccount) != 0)	// 上次是用网吧帐号或ISP上的
		{
			if(!g_pOnlineTable->IsTimeWait(idAccount))
				g_pPointThread->Kickout(idAccount);		// 通知游戏服务器踢人

			refuseLogin(Msg.m_szAccount, c_errOnline, "请稍后重新登录");
			return false;
		}
	}

	// 分类检查过期
	switch(nFeeType)
	{
	case c_typePoint:	// 没有处理计点卡的时限(点卡也应该有时限)
	case c_typeISP:
		if(nPoint <= 0)
		{
			if(ENABLE_LOGIN_NO_POINT)
			{
				SafeCopy(szNotifyMsg, FLAG_NO_POINT, MAX_NAMESIZE);
			}
			else
			{
				refuseLogin(Msg.m_szAccount, c_errNotPoint, "小时数已用尽");
				return false;
			}
		}
		break;
	case c_typeTime:
		{
			time_t	tCurr = time(NULL);
			tm *	pTm = localtime(&tCurr);
			DWORD	nCurrTime = (pTm->tm_year+1900)*10000 + (pTm->tm_mon+1)*100 + pTm->tm_mday;
			if(nCurrTime > nPointTime)
			{
				if(MONTH_CARD_TO_POINT_CARD && nPoint > 0)
				{
					nFeeType	= c_typePoint;
				}
				else if(MONTH_CARD_TO_POINT_CARD && ENABLE_LOGIN_NO_POINT)
				{
					nFeeType	= c_typePoint;
					SafeCopy(szNotifyMsg, FLAG_NO_POINT, MAX_NAMESIZE);
				}
				else
				{
					refuseLogin(Msg.m_szAccount, c_errNotTime, "帐号已过期");
					return false;
				}
			}
			else
			{
				// 检查是否还剩7天
				if(szNotifyMsg[0] == 0)		// 没有通知消息时
				{
					Check7DaysNodify(nPointTime, szNotifyMsg);		// return szNotifyMsg
				}
			}
		}
		break;
	case c_typeFree:
		break;
	default:
		refuseLogin(Msg.m_szAccount, c_errUnknown, "数据库错误");
		LOGERROR("玩家[%s]计费类型错误[%d]。★★★★★★★★★", Msg.m_szAccount, nFeeType);
		return false;
	}

	// 登录
	ASSERT(idAccount);

	// 通知游戏服务器
	int nRet = g_pPointThread->NewLogin(idAccount, nAuthenID, szNotifyMsg, Msg.m_szServer);
	switch(nRet)
	{
	case CPointThread::ERR_NO_SERVER:
		refuseLogin(Msg.m_szAccount, c_errUnknowServer, "服务器未启动");
		return false;
	case CPointThread::ERR_BUSY:
		refuseLogin(Msg.m_szAccount, c_errServerBusy, "服务器忙请稍候");
		return false;
	case CPointThread::ERR_FULL:
		refuseLogin(Msg.m_szAccount, c_errServerFull, "服务器人数已满");
		return false;
	case CPointThread::ERR_NONE:
		{
			if(bRejoin)
			{
				// 重新进入游戏，修改一些属性
				g_pOnlineTable->Rejoin(idAccount, nAuthenID, pClientIP, 
					Msg.m_szAccount, Msg.m_szPassword, 
					nFeeType, Msg.m_szServer, idFeeAccount, szFeeAccount);
				allowLogin(idAccount, nAuthenID, Msg.m_szServer);
				LOGACCOUNT("同意玩家[%s][%d]按[%d]类型重新登录[%s]。认证ID[%08X]，IP[%s]，计费帐号[%s]，通知[%s]", 
					Msg.m_szAccount, idAccount, nFeeType, Msg.m_szServer, 
					nAuthenID, pClientIP, szFeeAccount, szNotifyMsg);
			}
			else
			{
				// 添加在线表
				g_pOnlineTable->AddNew(idAccount, nAuthenID, pClientIP, 
					Msg.m_szAccount, Msg.m_szPassword, 
					nFeeType, Msg.m_szServer, idFeeAccount, szFeeAccount);			// return 0: error
				allowLogin(idAccount, nAuthenID, Msg.m_szServer);
				LOGACCOUNT("同意玩家[%s][%d]按[%d]类型登录[%s]。认证ID[%08X]，IP[%s]，计费帐号[%s]，通知[%s]", 
					Msg.m_szAccount, idAccount, nFeeType, Msg.m_szServer, 
					nAuthenID, pClientIP, szFeeAccount, szNotifyMsg);
			}

			// dump to database
			char	szStamp[255];
			time_t	tCurr = time(NULL);
			tm*	pTm = localtime(&tCurr);
			sprintf(szStamp, "%04d%02d%02d%02d%02d%02d", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
			char szSQL[1024];
			sprintf(szSQL, "INSERT DELAYED logon VALUES ('%s', '%s',%d,%d,'%s',   %u,'%s','%s','%s',%d);",
				szStamp, Msg.m_szAccount, idAccount, nFeeType, Msg.m_szServer, 
				nAuthenID, pClientIP, szFeeAccount, szNotifyMsg, bRejoin);
			g_db.ExecuteSQL(szSQL);
			return true;
		}
		break;
	default:
		ASSERT(!"NewLogin");
		return false;
	}
}

bool CLoginServerSocket::processMsgConnect(MsgConnect& Msg)
{
	Msg.m_szInfo[MAX_NAMESIZE-1] = 0;

	g_pOnlineTable->SetSerial(Msg.m_idAccount, Msg.m_uData);
	ShutDown();		//?? 关闭，让对方先关闭

	// dump to database
	char	szStamp[255];
	time_t	tCurr = time(NULL);
	tm*	pTm = localtime(&tCurr);
	sprintf(szStamp, "%04d%02d%02d%02d%02d%02d", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	char	szSerial[255];
	::InsertBackslash(szSerial, Msg.m_szInfo);
	char szSQL[1024];
	sprintf(szSQL, "INSERT DELAYED serial VALUES ('%s',%d,%d,'%s');",
						szStamp, Msg.m_idAccount, Msg.m_uData, szSerial);
	g_db.ExecuteSQL(szSQL);
	return true;
}

bool CLoginServerSocket::processMsg(Msg* pMsg)
{
	switch(pMsg->GetType())
	{
	case _MSG_ACCOUNT:
		if (sizeof(MsgC2SAccount)==pMsg->GetSize())
		{
			return processMsgClientAccount(*(MsgC2SAccount*)pMsg);
		}
		break;
	case _MSG_CONNECT:
		if (sizeof(MsgConnect)==pMsg->GetSize())
		{
			if(STAT_SERIAL)
			{
				return processMsgConnect(*(MsgConnect*)pMsg);
			}
			return true;
		}
		break;
	default:
		break;
	}
	addBan("未知");
	// 	if(pMsg->GetSize() >= 20)
	// 	{
	// 		pBuf[19] = 0;
	// 		LOGWARNING("一个客户端[%s]上传了非法登录消息，Msg[%d]，长度[%d]，可能是[%s]", 
	// 			pClientIP, pMsg->GetType(), pMsg->GetSize(), (pMsg+4));
	// 	}
	// 	else
	{
		LOGWARNING("一个客户端[%s]上传了非法登录消息，Msg[%d]，长度[%d]", 
			GetPeerIP(), pMsg->GetType(), pMsg->GetSize());
	}
	Close(true);			// 立即关闭
}

void CLoginServerSocket::addBan(LPCTSTR szAccount)
{
	char *	pClientIP	= GetPeerIP();
	uint32 nClientIP	= inet_addr(pClientIP);
	g_pLoginThread->addBan(nClientIP, pClientIP, szAccount);
}