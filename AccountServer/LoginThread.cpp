// ��¼�߳���
// �ɽ��ޣ�2001.11.20

#include "AllHeads.h"
#include "LoginThread.h"


#undef	LOCKTHREAD		// ��¼�̲߳���Ҫ�����
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

// ���߳���û���ⲿ��Ϣ���룬û�й����ͻ
void	CLoginThread::OnInit()
{
	LOCKTHREAD;

	try{
		m_cRc5.Rc5InitKey(RC5PASSWORD_KEY);
		m_cRc5NetBar.Rc5InitKey(RC5BARPASSWORD_KEY);

		m_cListenSocket.Open();
		LOGMSG("��¼�߳���������");

	}catch(...) { LOGCATCH("��¼�̳߳�ʼ���쳣�˳�"); }
}

bool	CLoginThread::OnProcess()
{
	LOCKTHREAD;

	try{
		time_t	tStart = clock();
		long	nUsed = 0;

		if(g_bEnableLogin)		// �����¼�ˣ�)
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

			// ����Ƿ���յ�������
			if(ret == -1)	// error
			{
	//			m_cListenSocket.Close();
				LOGERROR("��¼�߳� select ����[%d]", WSAGetLastError());
	//			PrintText("��¼�̳߳���SOCKET���رա�%d ��󽫻��Զ��ؽ�", REBUILDLISTENDELAYSEC);
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

					// ȡ�Է�IP����BAN��
					bool	bBan = false;
	//				sockaddr_in	inAddr;
	//				memset(&inAddr, 0, sizeof(inAddr));
	//				int		nLen = sizeof(inAddr);
	//				if(getpeername(sockNew, (sockaddr *)&inAddr, &nLen) == 0)		// ����ֱ����ACCEPT����
					{
	//					uint32	nClientIP = inAddr.sin_addr.S_un.S_addr;
						for(int i = 0; i < MAXBANIPS; i++)
						{
							if(m_pBanIPs[i].ClientIP() == nClientIP && m_pBanIPs[i].IsBan())
							{
								bBan = true;		// ���ø�IP��¼
								break;
							}
						}
					}

					// ������߱�
					if(bBan)
					{
						// �ر�SOCKET
						//	�޸�SOCKET���Գ������ر���
						struct linger ling;
						ling.l_onoff = 1;
						ling.l_linger = 0;
						setsockopt(sockNew, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
						closesocket(sockNew);		// �������ر�SOCKET���������ó������ر�
						InterlockedDecrement(&s_nSocketCount);
					}
					else
					{
						// �ҿ�λ��
						bool	bSuccess = false;
						int i;
						for(i = 0; i < MAXCONNECTS; i++)
						{
							if(!m_aServerSocket[i].IsOpen())
							{
								bSuccess = true;
								break;	// �ɹ�
							}
						}

						// �������
						if(bSuccess)
						{
							m_aServerSocket[i].Open(sockNew, nClientIP);
							m_aServerSocket[i].GetPeerIP();		// ����ȡIP
						}
						else
						{
							/*
							LOGWARNING("�ѽ��յ�һ�������ӣ������ӱ����������޸�MAXCONNECTS�������±������");

							//	�޸�SOCKET���Գ������ر���
							struct linger ling;
							ling.l_onoff = 1;
							ling.l_linger = 0;
							setsockopt(sockNew, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
							closesocket(sockNew);
							InterlockedDecrement(&s_nSocketCount);
							//*/
							//*else
							// ��ռ���е����ӣ��ԶԿ����������ӹ����������Ż�Ϊ�ҳ������ߣ�����BAN��??
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
							// ����SOCKETʹ��
							m_aServerSocket[nPos].Open(sockNew, nClientIP);
							LOGWARNING("���ӱ�������MAXCONNECTS����̫С��[%s]��ռ��[%s]�����ӱ�", 
										m_aServerSocket[nPos].GetPeerIP(), bufTemp);
							//*/
						}
					}
				}
			}	// ������

			// ���յ���¼��Ϣ������Ҫ�Ż�??
			for(int i = 0; i < MAXCONNECTS; i++)
			{
				if(ret <= 0)
					break;	// û������������

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
			nRemain = 0;		//? �л����Ƶ��̡߳��Ʒ��߳�����
		Sleep(nRemain);

		return true;

	}catch(...) { 
		LOGCATCH("��¼�߳���ѭ���쳣���Լ�������С�"); 
		PrintText("��¼�߳���ѭ�������Լ��������...");
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

		LOGMSG("��¼�߳������ر�");

	}catch(...) { LOGCATCH("��¼�̹߳ر�ʱ�쳣�˳�"); }
}
// �ڲ�����/////////
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
		AddBan(nClientIP, pClientIP, "δ֪");
		if(nLen >= 20)
		{
			pBuf[19] = 0;
			LOGWARNING("һ���ͻ���[%s]�ϴ��˷Ƿ���¼��Ϣ��Msg[%d]������[%d]��������[%s]", 
								pClientIP, unMsgType, nLen, (pBuf+4));
		}
		else
			LOGWARNING("һ���ͻ���[%s]�ϴ��˷Ƿ���¼��Ϣ��Msg[%d]������[%d]", 
							pClientIP, unMsgType, nLen);
		m_aServerSocket[nIndex].Close(true);			// �����ر�
		return false;
	}
}
// ˽�к���///////////////////////
bool	AppendPassword(OBJID idAccount, LPCTSTR szAccount, LPCTSTR szPassword)
{
	bool	bUpdate = false;
	::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
	if(g_cDatabase.Create(szAccount, szPassword))		// ���ʺ�
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
	if(g_cDatabase.Create(szAccount, szPassword))		// ���ʺ�
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
					OBJID& idFeeAccount, int& nPoint, DWORD& nPointTime, 						// ����ֵ
					int& nLicenceType, int& nLicence, char* szNetBarIP, char* szIPMask)			// ����ֵ
{
	idFeeAccount	= ID_NONE;
	::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
	if(g_cDatabase.Create(szBarAccount, szBarPassword))		// ���ʺ�
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
	FormatStr(Msg.m_szAccount, MAX_NAMESIZE);		// ��β0��ɾβ��
	FormatStr(Msg.m_szPassword, MAX_NAMESIZE);		// ��β0��ɾβ��
	FormatStr(Msg.m_szServer, MAX_NAMESIZE);		// ��β0��ɾβ��

	if(Msg.m_szAccount[0] == 0 || Msg.m_szPassword[0] == 0 || Msg.m_szServer[0] == 0)
		return false;

	char *	pClientIP						= m_aServerSocket[nIndex].GetPeerIP();
	uint32 nClientIP					= inet_addr(pClientIP);
	DWORD	nAuthenID						= NewAuthenID(nClientIP);
	char *	pErrMsg							= "��¼ʧ��";
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
	char 	szPassword[MAX_NAMESIZE]	= "";		// ���ñ���
	OBJID	idOldFeeAccount				= ID_NONE;	// ���ñ���
	char	szOldIP[MAX_NAMESIZE]		= "";		// ���ñ���

	if(g_pOnlineTable->GetAttr(Msg.m_szAccount, idAccount, szPassword, nAuthenID,			// ��nAuthenID��szOldServerName��szOldFeeAccount ������
						szOldServerName, szOldFeeAccount, idOldFeeAccount, szOldIP))		//??? ���ߡ��ú�����������ADDNEW�ķ�ԭ���Բ���
	{
		bRejoin	= true;
	}

	if(!GetAccount(Msg.m_szAccount, Msg.m_szPassword, 
						idAccount, nFeeType, nPoint, nPointTime, nBlock))			// ����ֵ
	{
		AddBan(nClientIP, pClientIP, Msg.m_szAccount);
		RefuseLogin(nIndex, Msg.m_szAccount, c_errPassword, "�ʺ���������");
		LOGACCOUNT("���[%s]����[%s]��¼���ʺ���������, IP��ַ[%s]", 
					Msg.m_szAccount, Msg.m_szPassword, pClientIP);
		return false;
	}

	// ����
	if(nBlock)
	{
		RefuseLogin(nIndex, Msg.m_szAccount, c_errBan, "���ʺű����");
		return false;
	}

	// ���Ƿ���¼
	if(nFeeType == c_typeNetBarPoint || nFeeType == c_typeNetBarTime || nFeeType == c_typeISP)
	{
		RefuseLogin(nIndex, Msg.m_szAccount, c_errBarPassword, "���ʺŲ��ܵ�¼");
		return false;
	}

	if(IsFreeServer(Msg.m_szServer))
	{
		// ��ѷ�����
		nFeeType		= c_typeFree;
//		SafeCopy(szNotifyMsg, "��ӭ�μӲ���", MAX_NAMESIZE);
	}
	else if(CheckISP(idAccount, nClientIP, pClientIP, 
						idFeeAccount, szFeeAccount, nPoint, nPointTime, szNotifyMsg))		// ����ֵ
	{
		// ISP����
		nFeeType		= c_typeISP;
	}
	else
	{
		idFeeAccount	= idAccount;
		SafeCopy(szFeeAccount, Msg.m_szAccount, MAX_NAMESIZE);
	}

	if(bRejoin)
	{
		// ������ͬʱ��¼����һ̨������������������ͼƷ��ʺ����Ƿ���ͬ
		if(strcmp(Msg.m_szServer, szOldServerName) != 0 
										|| strcmp(szFeeAccount, szOldFeeAccount) != 0)	// �ϴ����������ʺŻ�ISP�ϵ�
		{
			if(!g_pOnlineTable->IsTimeWait(idAccount))
				g_pPointThread->Kickout(idAccount);		// ֪ͨ��Ϸ����������

			RefuseLogin(nIndex, Msg.m_szAccount, c_errOnline, "���Ժ����µ�¼");
			return false;
		}
	}

	// ���������
	switch(nFeeType)
	{
	case c_typePoint:	// û�д���Ƶ㿨��ʱ��(�㿨ҲӦ����ʱ��)
	case c_typeISP:
		if(nPoint <= 0)
		{
			if(ENABLE_LOGIN_NO_POINT)
			{
				SafeCopy(szNotifyMsg, FLAG_NO_POINT, MAX_NAMESIZE);
			}
			else
			{
				RefuseLogin(nIndex, Msg.m_szAccount, c_errNotPoint, "Сʱ�����þ�");
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
					RefuseLogin(nIndex, Msg.m_szAccount, c_errNotTime, "�ʺ��ѹ���");
					return false;
				}
			}
			else
			{
				// ����Ƿ�ʣ7��
				if(szNotifyMsg[0] == 0)		// û��֪ͨ��Ϣʱ
				{
					Check7DaysNodify(nPointTime, szNotifyMsg);		// return szNotifyMsg
				}
			}
		}
		break;
	case c_typeFree:
		break;
	default:
		RefuseLogin(nIndex, Msg.m_szAccount, c_errUnknown, "���ݿ����");
		LOGERROR("���[%s]�Ʒ����ʹ���[%d]������������", Msg.m_szAccount, nFeeType);
		return false;
	}

	// ��¼
	ASSERT(idAccount);

	// ֪ͨ��Ϸ������
	int nRet = g_pPointThread->NewLogin(idAccount, nAuthenID, szNotifyMsg, Msg.m_szServer);
	switch(nRet)
	{
	case CPointThread::ERR_NO_SERVER:
		RefuseLogin(nIndex, Msg.m_szAccount, c_errUnknowServer, "������δ����");
		return false;
	case CPointThread::ERR_BUSY:
		RefuseLogin(nIndex, Msg.m_szAccount, c_errServerBusy, "������æ���Ժ�");
		return false;
	case CPointThread::ERR_FULL:
		RefuseLogin(nIndex, Msg.m_szAccount, c_errServerFull, "��������������");
		return false;
	case CPointThread::ERR_NONE:
		{
			if(bRejoin)
			{
				// ���½�����Ϸ���޸�һЩ����
				g_pOnlineTable->Rejoin(idAccount, nAuthenID, pClientIP, 
						Msg.m_szAccount, Msg.m_szPassword, 
						nFeeType, Msg.m_szServer, idFeeAccount, szFeeAccount);
				AllowLogin(nIndex, idAccount, nAuthenID, Msg.m_szServer);
				LOGACCOUNT("ͬ�����[%s][%d]��[%d]�������µ�¼[%s]����֤ID[%08X]��IP[%s]���Ʒ��ʺ�[%s]��֪ͨ[%s]", 
								Msg.m_szAccount, idAccount, nFeeType, Msg.m_szServer, 
								nAuthenID, pClientIP, szFeeAccount, szNotifyMsg);
			}
			else
			{
				// ������߱�
				g_pOnlineTable->AddNew(idAccount, nAuthenID, pClientIP, 
						Msg.m_szAccount, Msg.m_szPassword, 
						nFeeType, Msg.m_szServer, idFeeAccount, szFeeAccount);			// return 0: error
				AllowLogin(nIndex, idAccount, nAuthenID, Msg.m_szServer);
				LOGACCOUNT("ͬ�����[%s][%d]��[%d]���͵�¼[%s]����֤ID[%08X]��IP[%s]���Ʒ��ʺ�[%s]��֪ͨ[%s]", 
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
	cRetMsg.Create(idAccount, nAuthenID, szServerIP);		//? �´���Ϸ��������IP��ַ
	m_aServerSocket[nIndex].Send(&cRetMsg);
	if(!STAT_SERIAL)
		m_aServerSocket[nIndex].ShutDown();		//?? �رգ��öԷ��ȹر�(ע�⣺���û�յ�CMsgConnect��Ϣ�򲻻�ر�)
}

void CLoginThread::RefuseLogin(int nIndex, LPCTSTR szLoginName, int nType, LPCTSTR szText)
{
	MsgConnect cRetMsg;
	cRetMsg.Create(ID_NONE, nType, (char*)szText);
	m_aServerSocket[nIndex].Send(&cRetMsg);
	m_aServerSocket[nIndex].ShutDown();		//?? �رգ��öԷ��ȹر�
	LOGACCOUNT("��¼���ɹ�[%s]��ԭ����[%s][%d]", szLoginName, szText, nType);
}

void CLoginThread::AddBan(DWORD nClientIP, LPCTSTR szClientIP, LPCTSTR szAccount)
{
	// ����BAN��
	bool	bFoundBan = false;
	int		nFreeSlot = -1;
	int		nBanCount = 0;
	// �Ҹ���λ��
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

		if(m_pBanIPs[i].ClientIP() == nClientIP)	// �Ѽ�¼
		{
			bFoundBan = true;
			m_pBanIPs[i].IncError();				//? �������ߣ���������
			if(m_pBanIPs[i].IsBan())
			{
				LOGERROR("ĳ�ͻ����ѷǷ���¼[%d]�Σ��������˹����ʺŷ�������IP��ַ[%s]", BANERRORS, szClientIP);
				LOGACCOUNT("ĳ���[%s]��¼���ʺŷ�������������[%d]�Ρ�IP��ַ[%s]����ֹ[%d]����", 
							szAccount, BANERRORS, szClientIP, BANSECS);
				PrintText("IP��ַ��%s����������%d����, ��%d�����ڽ��޷���¼", 
							szClientIP, BANERRORS, BANSECS);
			}
		}
	}
	if(!bFoundBan)
	{
		// �����BAN
		if(nFreeSlot != -1)
		{
			m_pBanIPs[nFreeSlot].Create(nClientIP);
			if(nBanCount*100/MAXBANIPS > 78)		// ������
				LOGWARNING("���BAN��[%d/%d]��IPΪ[%s]", nBanCount+1, MAXBANIPS, szClientIP);
		}
		else
		{
			LOGERROR("BAN ��̫С���и�IP[%s]û���ȥ�����ʵ����� MAXBANIPS ����", szClientIP);
		}
	}
}
///////////////////////
bool	CLoginThread::ProcessMsgConnect(int nIndex, MsgConnect& Msg)
{
	Msg.m_szInfo[MAX_NAMESIZE-1] = 0;

	g_pOnlineTable->SetSerial(Msg.m_idAccount, Msg.m_uData);
	m_aServerSocket[nIndex].ShutDown();		//?? �رգ��öԷ��ȹر�

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















