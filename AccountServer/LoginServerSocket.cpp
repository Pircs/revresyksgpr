// huihui 2010.6.16
#include "AllHeads.h"
#include "LoginServerSocket.h"
#include "LoginThread.h"
#include "RC5_321216.h"

#undef	LOCKTHREAD		// ��¼�̲߳���Ҫ�����
#define	LOCKTHREAD
CRc5_321216		m_cRc5;
//m_cRc5.Rc5InitKey(RC5PASSWORD_KEY);

void CLoginServerSocket::refuseLogin(LPCTSTR szLoginName, int nType, LPCTSTR szText)
{
	MsgConnect cRetMsg;
	cRetMsg.Create(ID_NONE, nType, (char*)szText);
	Send(&cRetMsg);
	ShutDown();		//?? �رգ��öԷ��ȹر�
	LOGACCOUNT("��¼���ɹ�[%s]��ԭ����[%s][%d]", szLoginName, szText, nType);
}

void CLoginServerSocket::allowLogin(OBJID idAccount, DWORD nAuthenID, LPCTSTR szServer)
{
	MsgConnect cRetMsg;
	char	szServerIP[IPSTRSIZE] = "";
	g_pPointThread->GetServerIP(szServerIP, szServer);
	cRetMsg.Create(idAccount, nAuthenID, szServerIP);		//? �´���Ϸ��������IP��ַ
	Send(&cRetMsg);
	if(!STAT_SERIAL)
		ShutDown();		//?? �رգ��öԷ��ȹر�(ע�⣺���û�յ�CMsgConnect��Ϣ�򲻻�ر�)
}

bool CLoginServerSocket::processMsgClientAccount(MsgC2SAccount& Msg)
{
#ifdef	RC5ENCRYPT
	m_cRc5.Rc5InitKey(RC5PASSWORD_KEY);
	m_cRc5.Rc5Decrypt(Msg.m_szPassword, MAX_NAMESIZE);
#endif
	FormatStr(Msg.m_szAccount, MAX_NAMESIZE);		// ��β0��ɾβ��
	FormatStr(Msg.m_szPassword, MAX_NAMESIZE);		// ��β0��ɾβ��
	FormatStr(Msg.m_szServer, MAX_NAMESIZE);		// ��β0��ɾβ��

	if(Msg.m_szAccount[0] == 0 || Msg.m_szPassword[0] == 0 || Msg.m_szServer[0] == 0)
		return false;

	char *	pClientIP						= GetPeerIP();
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
		//AddBan(nClientIP, pClientIP, Msg.m_szAccount);
		refuseLogin(Msg.m_szAccount, c_errPassword, "�ʺ���������");
		LOGACCOUNT("���[%s]����[%s]��¼���ʺ���������, IP��ַ[%s]", 
			Msg.m_szAccount, Msg.m_szPassword, pClientIP);
		return false;
	}

	// ����
	if(nBlock)
	{
		refuseLogin(Msg.m_szAccount, c_errBan, "���ʺű����");
		return false;
	}

	// ���Ƿ���¼
	if(nFeeType == c_typeNetBarPoint || nFeeType == c_typeNetBarTime || nFeeType == c_typeISP)
	{
		refuseLogin(Msg.m_szAccount, c_errBarPassword, "���ʺŲ��ܵ�¼");
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

			refuseLogin(Msg.m_szAccount, c_errOnline, "���Ժ����µ�¼");
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
				refuseLogin(Msg.m_szAccount, c_errNotPoint, "Сʱ�����þ�");
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
					refuseLogin(Msg.m_szAccount, c_errNotTime, "�ʺ��ѹ���");
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
		refuseLogin(Msg.m_szAccount, c_errUnknown, "���ݿ����");
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
		refuseLogin(Msg.m_szAccount, c_errUnknowServer, "������δ����");
		return false;
	case CPointThread::ERR_BUSY:
		refuseLogin(Msg.m_szAccount, c_errServerBusy, "������æ���Ժ�");
		return false;
	case CPointThread::ERR_FULL:
		refuseLogin(Msg.m_szAccount, c_errServerFull, "��������������");
		return false;
	case CPointThread::ERR_NONE:
		{
			if(bRejoin)
			{
				// ���½�����Ϸ���޸�һЩ����
				g_pOnlineTable->Rejoin(idAccount, nAuthenID, pClientIP, 
					Msg.m_szAccount, Msg.m_szPassword, 
					nFeeType, Msg.m_szServer, idFeeAccount, szFeeAccount);
				allowLogin(idAccount, nAuthenID, Msg.m_szServer);
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
				allowLogin(idAccount, nAuthenID, Msg.m_szServer);
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

bool CLoginServerSocket::processMsgConnect(MsgConnect& Msg)
{
	Msg.m_szInfo[MAX_NAMESIZE-1] = 0;

	g_pOnlineTable->SetSerial(Msg.m_idAccount, Msg.m_uData);
	ShutDown();		//?? �رգ��öԷ��ȹر�

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
	addBan("δ֪");
	// 	if(pMsg->GetSize() >= 20)
	// 	{
	// 		pBuf[19] = 0;
	// 		LOGWARNING("һ���ͻ���[%s]�ϴ��˷Ƿ���¼��Ϣ��Msg[%d]������[%d]��������[%s]", 
	// 			pClientIP, pMsg->GetType(), pMsg->GetSize(), (pMsg+4));
	// 	}
	// 	else
	{
		LOGWARNING("һ���ͻ���[%s]�ϴ��˷Ƿ���¼��Ϣ��Msg[%d]������[%d]", 
			GetPeerIP(), pMsg->GetType(), pMsg->GetSize());
	}
	Close(true);			// �����ر�
}

void CLoginServerSocket::addBan(LPCTSTR szAccount)
{
	char *	pClientIP	= GetPeerIP();
	uint32 nClientIP	= inet_addr(pClientIP);
	g_pLoginThread->addBan(nClientIP, pClientIP, szAccount);
}