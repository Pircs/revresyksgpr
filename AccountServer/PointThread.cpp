// PointThread.cpp: �Ƶ��߳���
// �ɽ��ޣ�2001.11.20

#include "AllHeads.h"
#include "PointThread.h"
#include "Msg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CPointThread::CPointThread(u_short nPort, int nSndBuf /*= 0*/)
			: CThreadBase(), m_cListenSocket(nPort, nSndBuf)
{
	for(int i = 0; i < MAXGAMESERVERS; i++)
	{
		Clear(i, false);
	}

	// �����ʺű�
	GetServerAccount();		// Ԥ��
	m_pBanIPs = new CBanIP[MAXBANIPS];
}

CPointThread::~CPointThread()
{
	delete m_pBanIPs;
}
///////
// ��������ע�⻥��

//#define	LOCK	{LOCKTHREAD;
//#define	UNLOCK	}

// ��ʹ�ù����Ա�����������������������ⲿ�����������Ƚ�����
void	CPointThread::OnInit()
{
	LOCKTHREAD;
	try{

		m_cRc5.Rc5InitKey(RC5PASSWORD_KEY);

		m_cListenSocket.Open();
		LOGMSG("�Ƶ��߳���������");
	}catch(...) { LOGCATCH("�Ƶ��̳߳�ʼ��ʱ�쳣�˳�");}
}

bool	CPointThread::OnProcess()
{
	LOCKTHREAD;

	try{
		time_t	tStart = clock();
		// select
		fd_set	readmask;
		FD_ZERO(&readmask);
		if(!m_cListenSocket.IsOpen())
			m_cListenSocket.Rebuild();
		if(m_cListenSocket.IsOpen())
			FD_SET(m_cListenSocket.Socket(), &readmask);
		for(int i = 1; i < MAXGAMESERVERS; i++)	// 1: ��1��ʼ��
		{
			if(m_aServerSocket[i].IsOpen())
				FD_SET(m_aServerSocket[i].Socket(), &readmask);
		}

		struct timeval	timeout = {0,0};
		int ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);

		// ����Ƿ���յ�������
		if(ret == -1)	// error
		{
			m_cListenSocket.Close();
			LOGERROR("�Ƶ��߳� select ����[%d]", WSAGetLastError());
			PrintText("�Ƶ��̳߳���SOCKET���رա�%d ��󽫻��Զ��ؽ�", REBUILDLISTENDELAYSEC);
		}
		else if(ret > 0 && FD_ISSET(m_cListenSocket.Socket(), &readmask))
		{
			FD_CLR(m_cListenSocket.Socket(), &readmask);
			ret--;
			u_long	nClientIP;
			SOCKET	sockNew = m_cListenSocket.Accept(nClientIP);
			if(sockNew != INVALID_SOCKET)
			{
				// �ҿ�λ��
				bool	bSuccess = false;
				int i;
				for(i =1; i < MAXGAMESERVERS; i++)	// 1: ��1��ʼ��
				{
					if(!m_aServerSocket[i].IsOpen())
					{
						bSuccess = true;
						break;	// �ɹ�
					}
				}

				// �������
				struct in_addr	in;
				in.S_un.S_addr = nClientIP;
				char * pAddr = inet_ntoa(in);
				if(bSuccess)
				{
					m_aServerSocket[i].Open(sockNew, nClientIP);
					m_aHeartbeat[i] = clock();
					LOGMSG("һ������Ϸ���������ӽ�����������ID�ŷ���Ϊ[%d], IP��ַΪ[%s]", i, pAddr);
					::SetServerState(i, c_flagSocket);
				}
				else
				{
					LOGWARNING("�ѽ��յ�һ��������[%s]�������ӱ����������޸�MAXGAMESERVERS�������±������"
								, pAddr);
					//	�޸�SOCKET���Գ������ر���
					struct linger ling;
					ling.l_onoff = 1;
					ling.l_linger = 0;
					setsockopt(sockNew, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
					closesocket(sockNew);
					InterlockedDecrement(&s_nSocketCount);
				}
			}
		}

		// ���յ���¼��Ϣ������Ҫ�Ż�??
		for(int i = 1; i < MAXGAMESERVERS; i++)	// 1: ��1��ʼ��
		{
			if(ret <= 0)
				break;	// û������������

			if(m_aServerSocket[i].IsOpen())
			{
				if(FD_ISSET(m_aServerSocket[i].Socket(), &readmask))
				{
					FD_CLR(m_aServerSocket[i].Socket(), &readmask);
					ret--;
					char	bufMsg[_MAX_MSGSIZE];
					int		nMsgLen = 0;
					if(m_aServerSocket[i].Recv(bufMsg, nMsgLen))
					{
						if(nMsgLen)
						{
							m_aHeartbeat[i] = clock();
							m_aHeartbeatLast[i] = 0;
							ProcessMsg(i, bufMsg, nMsgLen);
						}
					}
					else	// �����ر�
					{
						LOGWARNING("Recv��������[%s]�رա�", m_aServerName[i]);
						LOGSERVER("���ߣ�������[%s]�رա�", m_aServerName[i]);
						PrintText("��������%s���رա�", m_aServerName[i]);
						Clear(i);
					}
				}
			}
		}
		long	nUsed = clock() - tStart;
		InterlockedExchangeAdd(&s_nAllTimeSum, nUsed);
		InterlockedIncrement(&s_nPointCount);

		long	nRemain = POINTLOOPDELAY - nUsed;
		if(nRemain < 0 || nRemain > POINTLOOPDELAY)
			nRemain = 0;
		if(nRemain)		//? ���л�����¼�̡߳��Ʒ��߳�����
			Sleep(nRemain);

		return true;

	}catch(...) { 
		LOGCATCH("�Ƶ��߳���ѭ���쳣��������������"); 
		PrintText("�Ƶ��߳���ѭ������������������...");
		return true; 
	}
}

void	CPointThread::OnDestroy()
{
	LOCKTHREAD;

//	LOCK	// VVVVVVVVV
	try{
		m_cListenSocket.Close();
		for(int i = 1; i < MAXGAMESERVERS; i++)	// 1: ��1��ʼ��
		{
			if(m_aServerSocket[i].IsOpen())
				m_aServerSocket[i].Close();
		}
		LOGMSG("�Ƶ��߳������˳�");

	}catch(...) { LOGCATCH("�Ƶ��߳̽���ʱ�쳣�˳�"); }

//	UNLOCK	// AAAAAAAAA
}

// �ﹲ��������Ҫ���⡣��LOGIN�̵߳��ã���ø�Ϊ��Ϣ����ģʽ??
// return ERR_NONE: OK
int	CPointThread::NewLogin(OBJID idAccount, DWORD nAuthenID, const char * szClientIP, const char * pGameServer)
{
	LOCKTHREAD;

	MsgConnect	cMsg;
	ASSERT(IPSTRSIZE == MAX_NAMESIZE);
	cMsg.Create(idAccount, nAuthenID, (char *)szClientIP);
	OBJID		nIndex = GetServerIndex_0(pGameServer);		// ���󷵻�NULL
	if(nIndex && m_aServerSocket[nIndex].IsOpen())
	{
		if(m_aState[nIndex] == STATE_NORMAL || idAccount <= MAX_GM_ID)
		{
			m_aServerSocket[nIndex].Send(&cMsg);	// ����ʧ��ʱ���Դ����´λ�ɾ����Ϸ��������
//			LOGACCOUNT("���͸�[%s]���[%d]�ĵ�¼��Ϣ����֤ID[%08X]���ͻ���IP[%s]", 
//						m_aServerName[nIndex], idAccount, nAuthenID, szClientIP);
			return ERR_NONE;
		}
		else if(m_aState[nIndex] == STATE_BUSY)
			return ERR_BUSY;
		else if(m_aState[nIndex] == STATE_FULL)
			return ERR_FULL;
	}

	return ERR_NO_SERVER;	// ����ķ��������������δ������
}

bool	CPointThread::GetServerIP(char * bufIP, const char * pServerName)
{
	LOCKTHREAD;

	int i;
	for(i = 1; i < MAXGAMESERVERS; i++)	// ����0��0Ϊ����
	{
		if(strcmp(m_aServerName[i], pServerName) == 0)
			break;
	}

	if(bufIP && i < MAXGAMESERVERS)
	{
		if(m_aServerSocket[i].IsOpen())
		{
			strcpy(bufIP, m_aServerSocket[i].GetPeerIP());
			return true;
		}
		else
		{
			if(m_aServerName[i][0])
			{
				LOGWARNING("ȡIPʱ����Ϸ������[%s]�Ѿ��ر�", m_aServerName[i]);
				LOGSERVER("������[%s]�Ѿ��ر���", m_aServerName[i]);
				PrintText("��������%s���Ѿ��ر���", m_aServerName[i]);
			}
			Clear(i);
		}
	}

	bufIP[0] = 0;
	return false;
}

int	CPointThread::GetServerState(LPCTSTR szServerName)
{
	LOCKTHREAD;

	int	nIndex = GetServerIndex_0(szServerName);
	if(nIndex)
		return m_aState[nIndex];
	else
		return STATE_OFFLINE;
}

int	CPointThread::GetServerCount()
{
	LOCKTHREAD;

	bool	nCount = 0;
	for(int i = 1; i < MAXGAMESERVERS; i++)	// 1: ��1��ʼ��
	{
		if(m_aState[i] != STATE_OFFLINE)
			nCount++;
	}

	return nCount;
}

bool CPointThread::CheckHeartbeatAll()
{
	LOCKTHREAD;

	bool	ret = false;
	for(int i = 1; i < MAXGAMESERVERS; i++)	// 1: ��1��ʼ��
	{
		if(m_aServerSocket[i].IsOpen())
		{
			CheckHeartbeat(i);
			ret = true;
		}
	}
	return ret;
}

/*bool	CPointThread::GetServerName(OBJID idServer, char bufName[SERVERNAMESIZE])
{
	LOCKTHREAD;

	ASSERT(bufName);

	bufName[0] = 0;
	if(!idServer)
		return false;

	if(m_aServerSocket[idServer].IsOpen())
	{
		SafeCopy(bufName, m_aServerName[idServer], MAX_NAMESIZE);
		return true;
	}
	else
	{
		if(m_aServerName[idServer][0])
		{
			LOGWARNING("ȡ��ʱ����Ϸ������[%s]�Ѿ��ر�", m_aServerName[idServer]);
			LOGSERVER("������[%s]�Ѿ��رա�", m_aServerName[idServer]);
			PrintText("��������%s���Ѿ��رա�", m_aServerName[idServer]);
		}
		Clear(idServer);		// SOCKET�رպ�ɾ����Ϸ��������������ͬ��
		return false;
	}
}*/

void	CPointThread::LogCount()
{
	LOCKTHREAD;

	char	bufTime[20];
	DateTime(bufTime, time(NULL));
	char	bufLine[4096]="";			//?
	int		nAmount = g_pOnlineTable->GetPlayerCount();
	for(int i = 1; i < g_nServerAccount; i++)	// 1: ��1��ʼ��
	{
		int	nCount = g_pOnlineTable->GetPlayerCount(g_aServerAccount[i].m_szServerName);
		int	nState = g_pPointThread->GetServerState(g_aServerAccount[i].m_szServerName);
		if(nState == CPointThread::STATE_OFFLINE)
		{
			nAmount	-= nCount;
			nCount	= 0;
		}
		char	buf[256];
		sprintf(buf, " | %12d", nCount);
		strcat(bufLine, buf);
	}
	strcat(bufLine, "\n");

	char	buf[4096];
	sprintf(buf, "%s | %12d", bufTime, nAmount);
	strcat(buf, bufLine);
	//log_SaveFile("count", buf, g_szCountHeadLine);		//? LOGFILE�����Լ��Ļ�������˫�ػ���Ч�ʽϵ�
	log_Save("count", buf);		//? LOGFILE�����Լ��Ļ�������˫�ػ���Ч�ʽϵ�
}

void	CPointThread::LogSerialCount()
{
	LOCKTHREAD;

	char	bufTime[20];
	DateTime(bufTime, time(NULL));
	char	bufLine[4096]="";			//?
	int		nAmount = 0;
	int		nCount = 0;
	for(COnlineTable::SERIAL_INDEX::iterator iter = g_pOnlineTable->GetSerialSet()->Begin();
			iter != g_pOnlineTable->GetSerialSet()->End();
			iter++, nCount++)
	{
		char	buf[256];
		sprintf(buf, " | %4d:%4d", iter->first, iter->second);
		strcat(bufLine, buf);
		nAmount	+= iter->second;
		if(strlen(bufLine) > 4000)
			break;
		if(nCount > 50000)
		{
			LOGERROR("��LogSerialCount()������");
			break;
		}
	}
	strcat(bufLine, "\n");

	char	buf[4196];
	sprintf(buf, "%s | %8d", bufTime, nAmount);
	strcat(buf, bufLine);
	//log_SaveFile("serial", buf, "ʱ��                |   �ϼ�   | ���к�:����\n");		//? LOGFILE�����Լ��Ļ�������˫�ػ���Ч�ʽϵ�
	log_Save("serial", buf);		//? LOGFILE�����Լ��Ļ�������˫�ػ���Ч�ʽϵ�
}

void	CPointThread::LogServer()
{
	LOCKTHREAD;

	char	bufTime[20];
	DateTime(bufTime, time(NULL));
	char	bufLine[4096];			//?
	sprintf(bufLine, "%s | %12d", bufTime, GetServerCount());
	for(int i = 1; i < g_nServerAccount; i++)	// 1: ��1��ʼ��
	{
		BOOL	bState = GetServerState(g_aServerAccount[i].m_szServerName);
		char	buf[256];
		if(bState != STATE_OFFLINE)
			sprintf(buf, " | %12s", "��");
		else
			sprintf(buf, " | %12s", "�w");
		strcat(bufLine, buf);
	}
	strcat(bufLine, "\n");

	//log_SaveFile("server", bufLine, g_szCountHeadLine);		//? LOGFILE�����Լ��Ļ�������˫�ػ���Ч�ʽϵ�
	log_Save("server", bufLine);
}

bool	CPointThread::Kickout(OBJID idAccount)
{
	LOCKTHREAD;

	char	bufServer[256];
	if(g_pOnlineTable->GetServerName(idAccount, bufServer))
	{
		return Kickout_0(idAccount, bufServer);
	}
	return false;
}
//////
// ��ǹ������������̶߳�ռ���á����ܵ����ⲿ������

bool	CPointThread::Kickout_0(OBJID idAccount, LPCTSTR szServerName)
{
	int nIndex = GetServerIndex_0(szServerName);

	if(nIndex != 0 )
	{
		MsgFee	cMsg;
		cMsg.Create(idAccount, MsgFee::FEE_KICKOUT);
		m_aServerSocket[nIndex].Send(&cMsg);
		return true;
	}


	return false;
}

bool	CPointThread::ProcessMsg(int nServerIndex,  char * pBuf, int nLen)
{
	Msg* pMsg = (Msg*)pBuf;
	if(nLen <= 2*sizeof(uint16)|| nLen > _MAX_MSGSIZE)
		return false;
	if(pMsg->GetSize()!=nLen)
		return false;
	uint16 unMsgType = pMsg->GetType();
	switch(unMsgType)
	{
	case _MSG_ACCOUNT:// ��Ϸ������ע�����˺ŷ�����
		{
			MsgC2SAccount& cMsg=*(MsgC2SAccount*)pBuf;
			FormatStr(cMsg.m_szAccount, MAX_NAMESIZE);		// ��β0��ɾβ��
			FormatStr(cMsg.m_szPassword, MAX_NAMESIZE);		// ��β0��ɾβ��
			FormatStr(cMsg.m_szServer, MAX_NAMESIZE);		// ��β0��ɾβ��
			LOGMSG("���յ�ACCOUNT��¼��Ϣ:�ʺ�[%s]��Ϸ������[%s]��", cMsg.m_szAccount, cMsg.m_szServer);

			// ��Ϣ���
			if(strlen(cMsg.m_szAccount) == 0 
						|| strlen(cMsg.m_szPassword) == 0 
						|| strlen(cMsg.m_szServer) == 0)
			{
				LOGERROR("����ĵ�¼��Ϣ[%s][%s][%s]", 
						cMsg.m_szAccount, cMsg.m_szPassword, cMsg.m_szServer);
				return false;
			}

			GetServerAccount();		// ��ʱ����

			// ����ʺ�
			int		nAccount = 0;		// ���ʺű��е���ţ�����Ϸ������ID�š�
			for(int i = 1; i < g_nServerAccount; i++)	// 1: ��1��ʼ��
			{
				if(!g_aServerAccount[i].m_b91U
						&& strcmp(g_aServerAccount[i].m_szServerName, cMsg.m_szServer) == 0)		// �Է�������ƥ��Ϊ׼
				{
					nAccount = i;
					break;
				}
			}

			MsgAccountLogin	cRetMsg;
			if(nAccount					// 0���޴��ʺ�
						&& strcmp(g_aServerAccount[nAccount].m_szLoginName, cMsg.m_szAccount) == 0
						&& strcmp(g_aServerAccount[nAccount].m_szPassword, cMsg.m_szPassword) == 0)	// �ҵ�
			{
				for(int i = 1; i < MAXGAMESERVERS; i++)
				{
					if(strcmp(g_aServerAccount[nAccount].m_szServerName, m_aServerName[i]) == 0)
					{
						// �ظ���¼
						if(m_aServerSocket[i].IsOpen())
						{
							LOGMSG("��Ϸ������[%s]�ظ�ACCOUNT��¼��ǰһ������[%d]�ѶϿ���", m_aServerName[i], i);
							m_aServerSocket[i].Close();		// �ر�ǰһ����¼������
							Clear(i);
						}
						m_aServerName[i][0] = 0;		// ɾ��SERVER����
						//��С����ȫ��һ��	break;
					}
				}

				// ������Ϸ����������/IP
				strcpy(m_aServerName[nServerIndex], g_aServerAccount[nAccount].m_szServerName);	// cMsg.m_szServer);
							//?? BUG����ǰ�����¼�ˣ�Ӧ�ý��յ�_MSG_LOGIN��ʼ�����¼
				PrintText("��%d����Ϸ��������%s���ѵ�¼", nServerIndex, m_aServerName[nServerIndex]);
				LOGSERVER("��%d����Ϸ��������%s���ѵ�¼", nServerIndex, m_aServerName[nServerIndex]);
							//?? BUG����ǰ�����¼�ˣ�Ӧ�ý��յ�_MSG_LOGIN��ʼ�����¼
				cRetMsg.Create(nServerIndex, 0, "", ACCOUNTVERSION);
				m_aServerSocket[nServerIndex].Send(&cRetMsg);
				LOGMSG("�µ���Ϸ������[%s]ACCOUNT�ʺ��Ͽɡ����ط�����ID[%d],��Ϸ�汾[%04X]", 
											m_aServerName[nServerIndex], nServerIndex, ACCOUNTVERSION);
				LOGMSG("�µ���Ϸ������[%s]ACCOUNT��¼��......", m_aServerName[nServerIndex]);
				::SetServerState(nServerIndex, c_flagAccount);

				// �������
//				m_idxID.Add(nServerIndex, nAccount);
//				g_aServerAccount[nAccount].m_nIndex = nServerIndex;
			}
			else
			{
				cRetMsg.Create(0, 0, "", ACCOUNTVERSION);
				m_aServerSocket[nServerIndex].Send(&cRetMsg);
				LOGMSG("�µ���Ϸ������[%s]��¼ʧ��", cMsg.m_szServer);
			}
		}
		break;
	case _MSG_LOGIN:
		{
			if(g_aServerAccount[nServerIndex].m_b91U)
			{
				LOGERROR("�������յ�_MSG_LOGIN��Ϣ��");
				return false;
			}
			// ��Ϣ���(����Ҫ)

//			CMsgConnect	cMsg;
//			cMsg.Create(nServerIndex, 0, "��¼�ɹ�");

			// ���򵥻ش�
//			m_aServerSocket[nServerIndex].Send(&cMsg);
			m_aState[nServerIndex] = STATE_NORMAL;		// ��¼���

			LOGMSG("......[%s]��Ϸ������LOGIN��¼�ɹ�", m_aServerName[nServerIndex]);
			LOGMSG("���յ���¼��Ϣ���µ���Ϸ������[%s]LOGIN��¼�ɹ�", m_aServerName[nServerIndex]);
			::SetServerState(nServerIndex, c_flagNormal);
		}
		break;
	case _MSG_FEE:
		{
			MsgFee&	cMsg=*(MsgFee*)pBuf;
			OBJID	idAccount = cMsg.m_idAccount;

			if(g_aServerAccount[nServerIndex].m_b91U && cMsg.m_ucType != MsgFee::HEARTBEAT)
			{
				LOGERROR("�������յ�_MSG_FEE��Ϣ��");
				return false;
			}

			switch(cMsg.m_ucType)
			{
			case MsgFee::FEE_BEGIN:
				{
					// ��Ϣ���
					if(idAccount == ID_NONE)
					{
						LOGERROR("�����FEE_BEGIN��Ϣ[%d]��", idAccount);
						return false;
					}

					// ������߱�����������ͬ��
					if(!g_pOnlineTable->IsOnline(idAccount))		//? ��������APPENDNEW()�ķ�ԭ���Բ���
					{
						DWORD	nAuthenID = 0xCCCCCCCC;		// ������֤��
						::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
						if(g_cDatabase.Create(idAccount))	// VVVVVVVVVVVVVVV
						{
							char* pName = g_cDatabase.GetName();
							if(!g_pOnlineTable->AppendNew(idAccount, nAuthenID, "StartFee", pName, "", 
//											g_cDatabase.GetType(), m_aServerName[nServerIndex], idAccount, pName);
											c_typeNone, m_aServerName[nServerIndex], idAccount, pName))
								Kickout_0(idAccount, m_aServerName[nServerIndex]);
							else
							//?if(g_bEnableLogin)
								LOGERROR("����FEE_BEGIN, ������[%s]�ʺ�[%s][%d]����ΪISPģʽ��Ƶ�ģʽ�����ٿ۵��", 
																m_aServerName[nServerIndex], pName, idAccount);
							g_cDatabase.Destroy();			// AAAAAAAAAAAAAAA
						}
						else
						{
							LOGERROR("��Ϸ������[%s]�ϴ���һ��δ֪�ʺ�[%d]��FEE_BEGIN��Ϣ��δ����", 
										m_aServerName[nServerIndex], idAccount);
						}
						::ReleaseMutex(g_xDatabase);	//------------------------------------------
					}

//#ifdef	SERVER_X
//					LOGPOINT("[%s]�ϴ�[%d]��START_FEE", m_aServerName[nServerIndex], idAccount);
//#endif	// SERVER_X
					int nRet = g_pOnlineTable->StartFee(idAccount, m_aServerName[nServerIndex]);		// return -n: ��������ƥ��
					// ˫�ص�¼������
					if(nRet < 0)		// <0 : error
					{
						LOGERROR("...˫�ص�¼�������START_FEE���ʺ�[%d]�������ߡ�", idAccount);
						Kickout_0(idAccount, m_aServerName[nServerIndex]);
					}

					if(m_aState[nServerIndex] == STATE_NORMAL)
						::SetServerState(nServerIndex, c_flagBegin);
					else
						::SetServerState(nServerIndex, c_flagBegin2);
				}
				break;
			case MsgFee::FEE_END:
			case MsgFee::FEE_OFFLINE:
				{
					// ��Ϣ���
					if(idAccount == ID_NONE)
					{
						LOGERROR("�����FEE_END��Ϣ[%d]��", idAccount);
						return false;
					}

					g_pOnlineTable->EndFee(idAccount, m_aServerName[nServerIndex], 
											cMsg.m_ucType == MsgFee::FEE_OFFLINE);

					// ɾ�����߱�
//��ENDFEEɾ��					g_pOnlineTable->Remove(idAccount);				// ����ɾ�����ʺ�
					if(m_aState[nServerIndex] == STATE_NORMAL)
						::SetServerState(nServerIndex, c_flagEnd);
					else
						::SetServerState(nServerIndex, c_flagEnd2);
				}
				break;
			case MsgFee::FEE_POINT:
			case MsgFee::FEE_TICK:
				{
					// ��Ϣ���
					if(idAccount == ID_NONE)
					{
						LOGERROR("�����FEE_POINT��Ϣ[%d]��", idAccount);
						return false;
					}

					// ������߱�����������ͬ��
					if(!g_pOnlineTable->IsOnline(idAccount))		//? ��������APPENDNEW()�ķ�ԭ���Բ���
					{
						DWORD	nAuthenID = 0xCCCCCCCC;		// ������֤��
						::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
						if(g_cDatabase.Create(idAccount))	// VVVVVVVVVVVVVVV
						{
							char* pName = g_cDatabase.GetName();
							if(!g_pOnlineTable->AppendNew(idAccount, nAuthenID, "PointFee", pName, "", 
//											g_cDatabase.GetType(), m_aServerName[nServerIndex], idAccount, pName);
											c_typeNone, m_aServerName[nServerIndex], idAccount, pName))
								Kickout_0(idAccount, m_aServerName[nServerIndex]);
							else
							//?if(g_bEnableLogin)
								LOGERROR("����FEE_POINT��������[%s]�ʺ�[%s][%d]����ΪISPģʽ��Ƶ�ģʽ�����ٿ۵��", 
												m_aServerName[nServerIndex], pName, idAccount);
							g_cDatabase.Destroy();			// AAAAAAAAAAAAAAA
						}
						else
						{
							LOGERROR("��Ϸ������[%s]�ϴ���һ��δ֪�ʺ�[%d]��FEE_POINT��Ϣ��δ����", 
										m_aServerName[nServerIndex], idAccount);
						}
						::ReleaseMutex(g_xDatabase);	//------------------------------------------
					}

					int		nFeeType		= c_typeNone;
					char	szServerName[MAX_NAMESIZE] = "(δ֪)";
					OBJID	idFeeAccount	= ID_NONE;
					char	szFeeAccount[MAX_NAMESIZE] = "(δ֪)";
					int		nPoint			= 0;
					if(g_pOnlineTable->GetAttr2(idAccount, nFeeType, szServerName, idFeeAccount, szFeeAccount))	//? ��������APPENDNEW()�ķ�ԭ���Բ���
					{
						if((nFeeType == c_typePoint || nFeeType == c_typeNetBarPoint 
										|| nFeeType == c_typeISP) && cMsg.m_ucType != MsgFee::FEE_TICK)		//? �ǼƵ����Ͳ�����
						{
							if(strcmp(szServerName, m_aServerName[nServerIndex]) == 0)
							{
								bool	bOk		= false;
								::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
								if(g_cDatabase.Create(idFeeAccount))	// VVVVVVVVVVVVVVV
								{
									bOk		= true;
									nPoint	= g_cDatabase.GetPoint();
									g_cDatabase.Destroy();			// AAAAAAAAAAAAAAA
								}
								::ReleaseMutex(g_xDatabase);	//------------------------------------------
								
								if(bOk)
								{
									if(nPoint > 0)
									{
										if(g_cDatabase.DecPoint(idFeeAccount))		//?? ����ҪOPEN��ֱ��ʹ��CDATABASE�Ĳ�ѯ��䡣
										{
											InterlockedIncrement(&s_nPointFee);
											InterlockedIncrement(&s_nPointSum);
										}
										else
										{
											LOGERROR("���ݿ�����ʺ�ID[%d]�޷��۵㡣��", idFeeAccount);
											LOGPOINT("���ݿ�����ʺ�ID[%d]�޷��۵㡣", idFeeAccount);
										}
									}
									else if(KICKOUT_USER_WHEN_NO_POINT)
									{
										LOGPOINT("������[%s]�����[%d]���ʺ�[%s][%d]�����ľ����ߡ�", 
											m_aServerName[nServerIndex], idAccount, szFeeAccount, idFeeAccount);
										Kickout_0(idAccount, m_aServerName[nServerIndex]);
									}
								}
								else
								{
									LOGERROR("��Ϸ������[%s]�ϴ���һ��δ֪�ʺ�[%d]�ļƵ���Ϣ������δ�Ǽǡ�", 
										m_aServerName[nServerIndex], idFeeAccount);
									LOGPOINT("��Ϸ������[%s]�ϴ���һ��δ֪�ʺ�[%d]�ļƵ���Ϣ������δ�Ǽ�", 
										m_aServerName[nServerIndex], idFeeAccount);
								}
							}
							else
							{
								LOGERROR("˫�ص�¼�����ID[%d]ʹ���ʺ�[%s][%d]��¼��[%s]�����յ�[%s]�ϴ��˵ĵļƵ���Ϣ����ұ������ߡ�", 
									idAccount, szFeeAccount, idFeeAccount, szServerName, m_aServerName[nServerIndex]);
								Kickout_0(idAccount, m_aServerName[nServerIndex]);
							}
						}

						int	nPointCount = g_pOnlineTable->PointFee(idAccount, m_aServerName[nServerIndex]);		// return -n: ��������ƥ��
//							LOGPOINT("[%s]�ϴ�[%d]��POINT_FEE������[%d]��ʣ��[%d]", 
//								m_aServerName[nServerIndex], idAccount, nPointCount, nPoint-1);
						// ˫�ص�¼������
//						if(nPointCount < 0)		// <0 : error
						{
//							LOGERROR("...˫�ص�¼�������POINT_FEE���ʺ�[%d]�������ߡ�", idAccount);
//							Kickout_0(idAccount, m_aServerName[nServerIndex]);
						}
						
						if(m_aState[nServerIndex] == STATE_NORMAL)
							::SetServerState(nServerIndex, c_flagPoint);
						else
							::SetServerState(nServerIndex, c_flagPoint2);
					} // GetAttr()
					else
					{
						LOGERROR("�����ͻ��FEE_POINT����¼�ʺŲ��ɹ�");
					}
				}
				break;
			case MsgFee::HEARTBEAT:
				{
					/*// ��Ϣ���
					if(idAccount == ID_NONE)
					{
						LOGERROR("�����HEARTBEAT��Ϣ[%d]��", idAccount);
						return false;
					}*/

					LOGMSG("���յ�[%s]��Ϸ�������ϴ���������Ϣ.", m_aServerName[nServerIndex]);
					// �޲�����ONPROCESS�л��Զ�ˢ������ʱ�䡣
					::SetServerState(nServerIndex, c_flagHeartbeat);
				}
				break;
			case MsgFee::SERVER_BUSY:
				{
					SetServerBusy(nServerIndex);
				}
				break;
			case MsgFee::SERVER_FULL:
				{
					SetServerFull(nServerIndex);
				}
				break;
			case MsgFee::SERVER_CLOSE:
				{
					PrintText("���յ�������[%s]�Ĺر���Ϣ��", m_aServerName[nServerIndex]);
					//??? SetServerClose(nServerIndex);
					//??? ::SetServerState(nIndex, c_flagOffline);
				}
				break;
			default:
				LOGERROR("��Ϸ������[%s]�Ʒ���Ϣ������[%d]����", m_aServerName[nServerIndex], cMsg.m_ucType);
			}
		}
		break;
	case _MSG_QUERYFEE:
		{
			if(g_aServerAccount[nServerIndex].m_b91U)
			{
				LOGERROR("�������յ�_MSG_QUERYFEE��Ϣ��");
				return false;
			}

			MsgQueryFee& cMsg=*(MsgQueryFee*)pBuf;

			// ��Ϣ���
			if(cMsg.m_idAccount == ID_NONE)
			{
				LOGERROR("�����_MSG_QUERYFEE��Ϣ[%d]��", cMsg.m_idAccount);
				return false;
			}

			int		nFeeType	= c_typeNone;
			DWORD	dwData		= 0;
			int		nTime		= 0;
			char	szServerName[MAX_NAMESIZE];
			OBJID	idFeeAccount;
			char	szFeeAccount[MAX_NAMESIZE];
			if(g_pOnlineTable->GetAttr2(cMsg.m_idAccount, nFeeType, szServerName, idFeeAccount, szFeeAccount))
			{
				switch(nFeeType)
				{
				case	c_typePoint:
					::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
					if(g_cDatabase.Create(cMsg.m_idAccount))	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVV
					{
						dwData	= g_cDatabase.GetPoint();
						g_cDatabase.Destroy();			// AAAAAAAAAAAAAAAAAAAAAAAAAAAAA
					}
					else
					{
						LOGERROR("��Ϸ������[%s]�����ѯһ��δ֪�ʺ�[%d]�Ʒ�����", m_aServerName[nServerIndex], cMsg.m_idAccount);
					}
					::ReleaseMutex(g_xDatabase);	//------------------------------------------
					break;
				case	c_typeTime:
					::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
					if(g_cDatabase.Create(cMsg.m_idAccount))	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVV
					{
						nTime	= g_cDatabase.GetPointTime();
						g_cDatabase.Destroy();			// AAAAAAAAAAAAAAAAAAAAAAAAAAAAA
					}
					else
					{
						LOGERROR("��Ϸ������[%s]�����ѯһ��δ֪�ʺ�[%d]�Ʒ�����", m_aServerName[nServerIndex], cMsg.m_idAccount);
					}
					::ReleaseMutex(g_xDatabase);	//------------------------------------------
					break;
				case	c_typeNetBarTime:
					/* �ݲ�����
					::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
					if(g_cDatabase.Create(cMsg.m_idAccount))	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVV
					{
						dwData	= g_cDatabase.GetLicence();
						g_cDatabase.Destroy();			// AAAAAAAAAAAAAAAAAAAAAAAAAAAAA
					}
					else
					{
						LOGERROR("��Ϸ������[%s]�����ѯһ��δ֪�ʺ�[%d]�Ʒ�����", m_aServerName[nServerIndex], cMsg.m_idAccount);
					}
					::ReleaseMutex(g_xDatabase);	//------------------------------------------
					break;
					//*/
				case	c_typeNetBarPoint:
				case	c_typeISP:
				case	c_typeFree:
					break;
				}
			}
/*
			::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
			if(g_cDatabase.Create(cMsg.m_idAccount))	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVV
			{
				nType	= g_cDatabase.GetType();
				dwData	= g_cDatabase.GetPoint();
				nTime	= g_cDatabase.GetPointTime();
				g_cDatabase.Destroy();			// AAAAAAAAAAAAAAAAAAAAAAAAAAAAA
			}
			else
			{
				LOGERROR("��Ϸ������[%s]�����ѯһ��δ֪���[%d]�Ʒ�����", m_aServerName[nServerIndex], cMsg.m_idAccount);
			}
			::ReleaseMutex(g_xDatabase);	//------------------------------------------
//*/
			// ������Ϣ
			cMsg.m_ucType	= nFeeType;
			cMsg.m_uData	= dwData;
			cMsg.m_nTime		= nTime;
			m_aServerSocket[nServerIndex].Send(&cMsg);
//			LOGPOINT("���͸�[%s]��Ϸ������[%d]�ʺŵļƷѲ�ѯ��Ϣ������[%d]������[%d]��ʱ��[%04d-%02d-%02d]", 
//						m_aServerName[nServerIndex], cMsg.m_idAccount, 
//						nFeeType, dwData, nTime/10000, (nTime/100)%100, nTime%100);
		}
		break;
	default:
		LOGERROR("��Ϸ�������ϴ���δ֪����Ϣ����[%d]", unMsgType);
		LOGPOINT("���յ�[%s]��Ϸ�������ϴ���δ֪��Ϣ.", m_aServerName[nServerIndex], unMsgType);
	}

	return true;
}

// ����0������
OBJID	CPointThread::GetServerIndex_0(const char * pServerName)
{
	int i;
	for(i = 1; i < MAXGAMESERVERS; i++)	// ����0��0Ϊ����
	{
		if(strcmp(m_aServerName[i], pServerName) == 0)
			break;
	}

	if(i < MAXGAMESERVERS)
	{
		if(m_aServerSocket[i].IsOpen())
		{
			return i;
		}
		else
		{
			if(m_aServerName[i][0])
			{
				LOGWARNING("ȡ������IDʱ����Ϸ������[%s]�Ѿ��ر�", m_aServerName[i]);
				LOGSERVER("������[%s]�Ѿ��ر��ˡ�", m_aServerName[i]);
				PrintText("��������%s���Ѿ��ر��ˡ�", m_aServerName[i]);
			}
			Clear(i);		// SOCKET�رպ�ɾ����Ϸ��������������ͬ��
		}
	}

	return 0;
}

// ���������Ƿ����ߣ�������ʱת����ʾ״̬����������ҵ�¼��
bool	CPointThread::CheckHeartbeat(int nIndex)
{
	// �ָ�BUSY��FULL
	if((m_aState[nIndex] == STATE_BUSY || m_aState[nIndex] == STATE_FULL) && clock() > m_aServerDelay[nIndex])
	{
		m_aState[nIndex] = STATE_NORMAL;
		::SetServerState(nIndex, c_flagNormal);
	}

	bool	ret = false;
	// �������
	if(m_aHeartbeatLast[nIndex] && (clock() - m_aHeartbeat[nIndex]) / CLOCKS_PER_SEC > HEARTBEATKICKSECS)
	{
		LOGWARNING("��Ϸ������[%s]��Ϊ��Ӧ��ʱ[%d]�룬���ӱ�ǿ�жϿ�", 
						m_aServerName[nIndex], (clock() - m_aHeartbeat[nIndex])/CLOCKS_PER_SEC);
		LOGSERVER("��Ӧ��ʱ��������[%s]�ѶϿ�����!", m_aServerName[nIndex]);
		PrintText("��Ӧ��ʱ����������%s���ѶϿ�����!", m_aServerName[nIndex]);
		m_aServerSocket[nIndex].Close();
		Clear(nIndex);
		ret = true;
	}
	else if((clock() - m_aHeartbeat[nIndex]) / CLOCKS_PER_SEC > HEARTBEATINTERVALSECS
				&& (clock() - m_aHeartbeatLast[nIndex]) / CLOCKS_PER_SEC > HEARTBEATINTERVALSECS)
	{
		MsgFee	cMsg;
		cMsg.Create(666666, MsgFee::HEARTBEAT);		// 666666: ����ʺ�ID��������
		if(m_aServerSocket[nIndex].Send(&cMsg))
		{
			LOGERROR("��Ϸ������[%s]��Ӧ��ʱ[%d]�룬�ѷ���һheartbeat��Ϣ", 
						m_aServerName[nIndex], (clock() - m_aHeartbeat[nIndex])/CLOCKS_PER_SEC);
		}
		else
		{
			LOGWARNING("��Ϸ������[%s]�����ѹرգ��޷�SEND������Ϣ", m_aServerName[nIndex]);
			LOGSERVER("��������ʧ�ܣ�������[%s]�ѶϿ�����!", m_aServerName[nIndex]);
			PrintText("��������ʧ�ܣ���������%s���ѶϿ�����!", m_aServerName[nIndex]);
			Clear(nIndex);
		}
		m_aHeartbeatLast[nIndex] = clock();
	}

	return ret;
}

void CPointThread::Clear(int nIndex, bool flag /*= true*/) 
{ 
	m_aState[nIndex] = STATE_OFFLINE;
	m_aServerName[nIndex][0] = m_aHeartbeat[nIndex] = m_aHeartbeatLast[nIndex] = m_aServerDelay[nIndex] = 0; 
	if(flag)
		::SetServerState(nIndex, c_flagOffline);
}

void CPointThread::SetServerBusy(int nServerIndex) { 
	m_aServerDelay[nServerIndex] = clock() + SERVER_BUSY_DELAY_SECS*CLOCKS_PER_SEC;
	m_aState[nServerIndex] = STATE_BUSY;
	::SetServerState(nServerIndex, c_flagStop);
}

void CPointThread::SetServerFull(int nServerIndex) {
	m_aServerDelay[nServerIndex] = clock() + SERVER_FULL_DELAY_SECS*CLOCKS_PER_SEC;
	m_aState[nServerIndex] = STATE_FULL;
	::SetServerState(nServerIndex, c_flagStop);
}

