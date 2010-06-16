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


// ���߳���û���ⲿ��Ϣ���룬û�й����ͻ
void	CLoginThread::OnInit()
{
	LOCKTHREAD;
	try{
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
	return m_aServerSocket[nIndex].processMsg(pMsg);
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

void CLoginThread::addBan(DWORD nClientIP, LPCTSTR szClientIP, LPCTSTR szAccount)
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