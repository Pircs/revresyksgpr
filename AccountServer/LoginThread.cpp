// 登录线程类
// 仙剑修，2001.11.20

#include "AllHeads.h"
#include "LoginThread.h"


#undef	LOCKTHREAD		// 登录线程不需要互斥★
#define	LOCKTHREAD

CLoginThread::CLoginThread(u_short nPort) 
		: CThreadBase(), m_cListenSocket(nPort)
{
	LOCKTHREAD;

	m_pBanIPs = new CBanIP[MAXBANIPS];
}

CLoginThread::~CLoginThread()
{
	LOCKTHREAD;

	delete m_pBanIPs;
}

//#define	CRITSECT	//	CSingleLock(&m_xCtrl, true);

//#define	LOCK	//?	{CSingleLock(&m_xCtrl, true);
//#define	UNLOCK	//?	}

// 该线程类没有外部消息进入，没有共享冲突
void	CLoginThread::OnInit()
{
	LOCKTHREAD;

	try{
		m_cRc5.Rc5InitKey(RC5PASSWORD_KEY);
		m_cRc5NetBar.Rc5InitKey(RC5BARPASSWORD_KEY);

		m_cListenSocket.Open();
		LOGMSG("登录线程正常启动");

	}catch(...) { LOGCATCH("登录线程初始化异常退出"); }
}

bool	CLoginThread::OnProcess()
{
	LOCKTHREAD;

	try{
		time_t	tStart = clock();
		long	nUsed = 0;

		if(g_bEnableLogin)		// 允许登录了：)
		{
			// select
			fd_set	readmask;
			FD_ZERO(&readmask);
			if(!m_cListenSocket.IsOpen())
				m_cListenSocket.Rebuild();
			if(m_cListenSocket.IsOpen())
				FD_SET(m_cListenSocket.Socket(), &readmask);
			for(int i = 0; i < MAXCONNECTS; i++)
			{
				if(m_aServerSocket[i].IsOpen())
					FD_SET(m_aServerSocket[i].Socket(), &readmask);
			}
			struct timeval	timeout = {0,0};
			int ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);

			// 检查是否接收到新连接
			if(ret == -1)	// error
			{
	//			m_cListenSocket.Close();
				LOGERROR("登录线程 select 错误[%d]", WSAGetLastError());
	//			PrintText("登录线程出错，SOCKET被关闭。%d 秒后将会自动重建", REBUILDLISTENDELAYSEC);
			}
			else if(ret > 0 && FD_ISSET(m_cListenSocket.Socket(), &readmask))
			{
				FD_CLR(m_cListenSocket.Socket(), &readmask);
				ret--;
				u_long	nClientIP;
				SOCKET sockNew = m_cListenSocket.Accept(nClientIP);
				if(sockNew != INVALID_SOCKET)
				{
					InterlockedIncrement(&s_nLoginAccept);

					// 取对方IP。查BAN表
					bool	bBan = false;
	//				sockaddr_in	inAddr;
	//				memset(&inAddr, 0, sizeof(inAddr));
	//				int		nLen = sizeof(inAddr);
	//				if(getpeername(sockNew, (sockaddr *)&inAddr, &nLen) == 0)		// 可以直接由ACCEPT返回
					{
	//					uint32	nClientIP = inAddr.sin_addr.S_un.S_addr;
						for(int i = 0; i < MAXBANIPS; i++)
						{
							if(m_pBanIPs[i].ClientIP() == nClientIP && m_pBanIPs[i].IsBan())
							{
								bBan = true;		// 不让该IP登录
								break;
							}
						}
					}

					// 添加在线表
					if(bBan)
					{
						// 关闭SOCKET
						//	修改SOCKET属性成立即关闭型
						struct linger ling;
						ling.l_onoff = 1;
						ling.l_linger = 0;
						setsockopt(sockNew, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
						closesocket(sockNew);		// ★主动关闭SOCKET，必须设置成立即关闭
						InterlockedDecrement(&s_nSocketCount);
					}
					else
					{
						// 找空位置
						bool	bSuccess = false;
						int i;
						for(i = 0; i < MAXCONNECTS; i++)
						{
							if(!m_aServerSocket[i].IsOpen())
							{
								bSuccess = true;
								break;	// 成功
							}
						}

						// 添加连接
						if(bSuccess)
						{
							m_aServerSocket[i].Open(sockNew, nClientIP);
							m_aServerSocket[i].GetPeerIP();		// 立即取IP
						}
						else
						{
							/*
							LOGWARNING("已接收到一个新连接，但连接表已满。请修改MAXCONNECTS，并重新编译程序。");

							//	修改SOCKET属性成立即关闭型
							struct linger ling;
							ling.l_onoff = 1;
							ling.l_linger = 0;
							setsockopt(sockNew, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
							closesocket(sockNew);
							InterlockedDecrement(&s_nSocketCount);
							//*/
							//*else
							// 抢占旧有的连接，以对抗“大量连接攻击”。可优化为找出攻击者，加入BAN表。??
							srand((unsigned int) clock());
							int		nPos = rand() % MAXCONNECTS;
							char	bufTemp[IPSTRSIZE];
							ASSERT(m_aServerSocket[nPos].IsOpen());
							SafeCopy(bufTemp, m_aServerSocket[nPos].GetPeerIP(), IPSTRSIZE);
							if(FD_ISSET(m_aServerSocket[nPos].Socket(), &readmask))
							{
								FD_CLR(m_aServerSocket[nPos].Socket(), &readmask);
								ret--;
							}
							m_aServerSocket[nPos].Close(true);
							// 由新SOCKET使用
							m_aServerSocket[nPos].Open(sockNew, nClientIP);
							LOGWARNING("连接表已满，MAXCONNECTS参数太小。[%s]抢占了[%s]的连接表。", 
										m_aServerSocket[nPos].GetPeerIP(), bufTemp);
							//*/
						}
					}
				}
			}	// 新连接

			// 接收到登录消息包。需要优化??
			for(int i = 0; i < MAXCONNECTS; i++)
			{
				if(ret <= 0)
					break;	// 没连接有数据了

				if(m_aServerSocket[i].IsOpen() && FD_ISSET(m_aServerSocket[i].Socket(), &readmask))
				{
					FD_CLR(m_aServerSocket[i].Socket(), &readmask);
					ret--;

					char	bufMsg[_MAX_MSGSIZE];
					int		nMsgLen = 0;
					if(m_aServerSocket[i].Recv(bufMsg, nMsgLen))
						if(nMsgLen)
							ProcessMsg(i, bufMsg, nMsgLen);
				}
			}
			nUsed = clock() - tStart;
			InterlockedExchangeAdd(&s_nAllTimeSum, nUsed);
			InterlockedIncrement(&s_nLoginCount);
			InterlockedExchangeAdd(&s_nAvgServerTime, nUsed);
			if(nUsed > InterlockedExchangeAdd(&s_nMaxServerTime, 0))
				InterlockedExchange(&s_nMaxServerTime, nUsed);
		}	// if(g_bEnableLogin)

		long	nRemain = LOGINLOOPDELAY - nUsed;
		if(nRemain < 0 || nRemain > LOGINLOOPDELAY)
			nRemain = 0;		//? 切换到计点线程。计费线程优先
		Sleep(nRemain);

		return true;

	}catch(...) { 
		LOGCATCH("登录线程主循环异常，仍坚持运行中。"); 
		PrintText("登录线程主循环出错，仍坚持运行中...");
		return true; 
	}
}

void	CLoginThread::OnDestroy()
{
	LOCKTHREAD;

	try{
		m_cListenSocket.Close();
		for(int i = 0; i < MAXCONNECTS; i++)
		{
//			if(m_aServerSocket[i].IsOpen())
				m_aServerSocket[i].Close();
		}

		LOGMSG("登录线程正常关闭");

	}catch(...) { LOGCATCH("登录线程关闭时异常退出"); }
}
// 内部函数/////////
bool	CLoginThread::ProcessMsg(int nIndex, char * pBuf, int nLen)
{
	Msg* pMsg = (Msg*)pBuf;
	if(nLen <= 2*sizeof(uint16)|| nLen > _MAX_MSGSIZE)
		return false;
	if(pMsg->GetSize()!=nLen)
		return false;
	uint16 unMsgType = pMsg->GetType();
	if(unMsgType == _MSG_ACCOUNT&&sizeof(MsgC2SAccount)==nLen)
	{
		return ProcessMsgClientAccount(nIndex, *(MsgC2SAccount*)pMsg);
	}
	else if(unMsgType == _MSG_CONNECT&&sizeof(MsgConnect)==nLen)
	{
		if(STAT_SERIAL)
		{
			return ProcessMsgConnect(nIndex, *(MsgConnect*)pMsg);
		}
		return true;
	}
	else
	{
		char *	pClientIP	= m_aServerSocket[nIndex].GetPeerIP();
		uint32 nClientIP	= inet_addr(pClientIP);
		AddBan(nClientIP, pClientIP, "未知");
		if(nLen >= 20)
		{
			pBuf[19] = 0;
			LOGWARNING("一个客户端[%s]上传了非法登录消息，Msg[%d]，长度[%d]，可能是[%s]", 
								pClientIP, unMsgType, nLen, (pBuf+4));
		}
		else
			LOGWARNING("一个客户端[%s]上传了非法登录消息，Msg[%d]，长度[%d]", 
							pClientIP, unMsgType, nLen);
		m_aServerSocket[nIndex].Close(true);			// 立即关闭
		return false;
	}
}
// 私有函数///////////////////////
bool	AppendPassword(OBJID idAccount, LPCTSTR szAccount, LPCTSTR szPassword)
{
	bool	bUpdate = false;
	::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
	if(g_cDatabase.Create(szAccount, szPassword))		// 有帐号
	{	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
		g_cDatabase.Destroy();
		// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
		g_pOnlineTable->SetPassword(idAccount, szPassword);
		bUpdate = true;
	}
	::ReleaseMutex(g_xDatabase);	//------------------------------------------
	return bUpdate;
}
///////////////////////
bool	GetFeeType(LPCTSTR szAccount, LPCTSTR szPassword, int& nFeeType, int& nPoint)		// return nType
{
	nFeeType	= c_typeNone;
	::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
	if(g_cDatabase.Create(szAccount, szPassword))		// 有帐号
	{	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
		nFeeType	= g_cDatabase.GetType();
		nPoint		= g_cDatabase.GetPoint();
		g_cDatabase.Destroy();
		// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	}
	::ReleaseMutex(g_xDatabase);	//------------------------------------------
	return (nFeeType != c_typeNone);
}
///////////////////////
bool	GetLicence(LPCTSTR szBarAccount, LPCTSTR szBarPassword, 
					OBJID& idFeeAccount, int& nPoint, DWORD& nPointTime, 						// 返回值
					int& nLicenceType, int& nLicence, char* szNetBarIP, char* szIPMask)			// 返回值
{
	idFeeAccount	= ID_NONE;
	::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
	if(g_cDatabase.Create(szBarAccount, szBarPassword))		// 有帐号
	{	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
		idFeeAccount	= g_cDatabase.GetID();
		nPoint			= g_cDatabase.GetPoint();
		nPointTime		= g_cDatabase.GetPointTime();
		nLicenceType	= g_cDatabase.GetType();
		nLicence		= g_cDatabase.GetLicence();
		SafeCopy(szNetBarIP, g_cDatabase.GetNetBarIP(), MAX_NAMESIZE);
		SafeCopy(szIPMask, g_cDatabase.GetIPMask(), MAX_NAMESIZE);
		g_cDatabase.Destroy();
		// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	}
	::ReleaseMutex(g_xDatabase);	//------------------------------------------
	return (idFeeAccount != ID_NONE);
}
///////////////////////
bool	CLoginThread::ProcessMsgClientAccount(int nIndex, MsgC2SAccount& Msg)
{
#ifdef	RC5ENCRYPT
	m_cRc5.Rc5Decrypt(Msg.m_szPassword, MAX_NAMESIZE);
#endif
	FormatStr(Msg.m_szAccount, MAX_NAMESIZE);		// 添尾0，删尾空
	FormatStr(Msg.m_szPassword, MAX_NAMESIZE);		// 添尾0，删尾空
	FormatStr(Msg.m_szServer, MAX_NAMESIZE);		// 添尾0，删尾空

	if(Msg.m_szAccount[0] == 0 || Msg.m_szPassword[0] == 0 || Msg.m_szServer[0] == 0)
		return false;

	char *	pClientIP						= m_aServerSocket[nIndex].GetPeerIP();
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
		AddBan(nClientIP, pClientIP, Msg.m_szAccount);
		RefuseLogin(nIndex, Msg.m_szAccount, c_errPassword, "帐号名或口令错");
		LOGACCOUNT("玩家[%s]口令[%s]登录。帐号名或口令错, IP地址[%s]", 
					Msg.m_szAccount, Msg.m_szPassword, pClientIP);
		return false;
	}

	// 查封号
	if(nBlock)
	{
		RefuseLogin(nIndex, Msg.m_szAccount, c_errBan, "该帐号被封号");
		return false;
	}

	// 检查非法登录
	if(nFeeType == c_typeNetBarPoint || nFeeType == c_typeNetBarTime || nFeeType == c_typeISP)
	{
		RefuseLogin(nIndex, Msg.m_szAccount, c_errBarPassword, "该帐号不能登录");
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

			RefuseLogin(nIndex, Msg.m_szAccount, c_errOnline, "请稍后重新登录");
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
				RefuseLogin(nIndex, Msg.m_szAccount, c_errNotPoint, "小时数已用尽");
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
					RefuseLogin(nIndex, Msg.m_szAccount, c_errNotTime, "帐号已过期");
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
		RefuseLogin(nIndex, Msg.m_szAccount, c_errUnknown, "数据库错误");
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
		RefuseLogin(nIndex, Msg.m_szAccount, c_errUnknowServer, "服务器未启动");
		return false;
	case CPointThread::ERR_BUSY:
		RefuseLogin(nIndex, Msg.m_szAccount, c_errServerBusy, "服务器忙请稍候");
		return false;
	case CPointThread::ERR_FULL:
		RefuseLogin(nIndex, Msg.m_szAccount, c_errServerFull, "服务器人数已满");
		return false;
	case CPointThread::ERR_NONE:
		{
			if(bRejoin)
			{
				// 重新进入游戏，修改一些属性
				g_pOnlineTable->Rejoin(idAccount, nAuthenID, pClientIP, 
						Msg.m_szAccount, Msg.m_szPassword, 
						nFeeType, Msg.m_szServer, idFeeAccount, szFeeAccount);
				AllowLogin(nIndex, idAccount, nAuthenID, Msg.m_szServer);
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
				AllowLogin(nIndex, idAccount, nAuthenID, Msg.m_szServer);
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

void CLoginThread::AllowLogin(int nIndex, OBJID idAccount, DWORD nAuthenID, LPCTSTR szServer)
{
	MsgConnect cRetMsg;
	char	szServerIP[IPSTRSIZE] = "";
	g_pPointThread->GetServerIP(szServerIP, szServer);
	cRetMsg.Create(idAccount, nAuthenID, szServerIP);		//? 下传游戏服务器的IP地址
	m_aServerSocket[nIndex].Send(&cRetMsg);
	if(!STAT_SERIAL)
		m_aServerSocket[nIndex].ShutDown();		//?? 关闭，让对方先关闭(注意：如果没收到CMsgConnect消息则不会关闭)
}

void CLoginThread::RefuseLogin(int nIndex, LPCTSTR szLoginName, int nType, LPCTSTR szText)
{
	MsgConnect cRetMsg;
	cRetMsg.Create(ID_NONE, nType, (char*)szText);
	m_aServerSocket[nIndex].Send(&cRetMsg);
	m_aServerSocket[nIndex].ShutDown();		//?? 关闭，让对方先关闭
	LOGACCOUNT("登录不成功[%s]。原因是[%s][%d]", szLoginName, szText, nType);
}

void CLoginThread::AddBan(DWORD nClientIP, LPCTSTR szClientIP, LPCTSTR szAccount)
{
	// 增加BAN表
	bool	bFoundBan = false;
	int		nFreeSlot = -1;
	int		nBanCount = 0;
	// 找个空位置
	for(int i = 0; i < MAXBANIPS; i++)
	{
		if(m_pBanIPs[i].ClientIP() == 0)
		{
			if(nFreeSlot == -1)
				nFreeSlot = i;
		}
		else
		{
			nBanCount++;
		}

		if(m_pBanIPs[i].ClientIP() == nClientIP)	// 已记录
		{
			bFoundBan = true;
			m_pBanIPs[i].IncError();				//? 不立即踢，超过再踢
			if(m_pBanIPs[i].IsBan())
			{
				LOGERROR("某客户端已非法登录[%d]次，可能有人攻击帐号服务器。IP地址[%s]", BANERRORS, szClientIP);
				LOGACCOUNT("某玩家[%s]登录到帐号服务器连续出错[%d]次。IP地址[%s]被禁止[%d]秒钟", 
							szAccount, BANERRORS, szClientIP, BANSECS);
				PrintText("IP地址“%s”连续出错“%d”次, “%d”秒内将无法登录", 
							szClientIP, BANERRORS, BANSECS);
			}
		}
	}
	if(!bFoundBan)
	{
		// 添加新BAN
		if(nFreeSlot != -1)
		{
			m_pBanIPs[nFreeSlot].Create(nClientIP);
			if(nBanCount*100/MAXBANIPS > 78)		// 快满了
				LOGWARNING("添加BAN表[%d/%d]，IP为[%s]", nBanCount+1, MAXBANIPS, szClientIP);
		}
		else
		{
			LOGERROR("BAN 表太小，有个IP[%s]没填进去。请适当增大 MAXBANIPS 参数", szClientIP);
		}
	}
}
///////////////////////
bool	CLoginThread::ProcessMsgConnect(int nIndex, MsgConnect& Msg)
{
	Msg.m_szInfo[MAX_NAMESIZE-1] = 0;

	g_pOnlineTable->SetSerial(Msg.m_idAccount, Msg.m_uData);
	m_aServerSocket[nIndex].ShutDown();		//?? 关闭，让对方先关闭

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















