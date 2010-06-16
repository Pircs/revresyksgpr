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


// 该线程类没有外部消息进入，没有共享冲突
void	CLoginThread::OnInit()
{
	LOCKTHREAD;
	try{
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
					{
						if(nMsgLen)
						{
							ProcessMsg(i, bufMsg, nMsgLen);
						}
					}
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
	return m_aServerSocket[nIndex].processMsg(pMsg);
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

void CLoginThread::addBan(DWORD nClientIP, LPCTSTR szClientIP, LPCTSTR szAccount)
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