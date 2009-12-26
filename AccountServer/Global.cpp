// ȫ�ֱ���
// �ɽ��ޣ�2001.11.19

#include "AllHeads.h"

HANDLE	g_xDatabase;
///////
// ����ͳ��

long	s_nSocketCount = 0;
long	s_nLoginCount = 0;
long	s_nMaxServerTime = 0;
long	s_nAvgServerTime = 0;
long	s_nPointCount = 0;
long	s_nLoginAccept = 0;
long	s_nPointFee = 0;
long	s_nPointSum = 0;
long	s_nAllTimeSum = 0;
long	s_nDatabaseTimeSum = 0;

//CCriticalSection	s_xCtrl;
char	s_bufText[4096] = "";
char	s_bufState[MAXGAMESERVERS] = "";
char	s_bufStateBackup[MAXGAMESERVERS] = "";
#include <iostream>
void	PrintText(const char * szFormat, ...)
{
	//CSingleLock xLock(&s_xCtrl, true);
	//ASSERT(xLock.IsLocked());

	va_list argptr;
	va_start( argptr, szFormat);     /* Initialize variable arguments. */

	char	buf[4096] = "";					/* ?????�ռ���ܲ��� */
	int		ret = vsprintf(buf, szFormat, argptr);
	ASSERT(ret < 4096-2);						/* ?????�ռ���ܲ��� */

	va_end( argptr );              /* Reset variable arguments.      */

	s_bufText[0]=0;
	strcat(s_bufText, "��");
	char szCurrTime[20];
	DateTime(szCurrTime, time(NULL));
	if(strlen(szCurrTime) >= 8)
		strcat(s_bufText, szCurrTime);		//  + 8���˵�����·�
	strcat(s_bufText, ": ");
	strcat(s_bufText, buf);
	strcat(s_bufText, "��\r\n");
	std::cout<<s_bufText;
}

const char* getCurrTimeString()
{
	static char szCurrTime[60];
	FormatDateTime(szCurrTime, "[%04d-%02d-%02d %02d:%02d:%02d]: ", time(NULL));		// szFormat: 
	return szCurrTime;
}


bool	LockedGetText(char * buf)
{
//	CSingleLock xLock(&s_xCtrl, true);
//	ASSERT(xLock.IsLocked());

	if(strlen(s_bufText))
	{
		strcpy(buf, s_bufText);
//		s_bufText[0] = 0;
		return true;
	}
	else
	{
		return false;
	}
}

void	SetServerState(int nIndex, int nState)
{
//	CSingleLock xLock(&s_xCtrl, true);
//	ASSERT(xLock.IsLocked());

	if(nIndex >= 0 && nIndex < MAXGAMESERVERS)
		s_bufState[nIndex] = nState;

	if(nIndex >= 0 && nIndex < MAXGAMESERVERS && nState <= c_flagBaseMax)
		s_bufStateBackup[nIndex] = nState;
}

void	GetServerState(char * buf)
{
//	CSingleLock xLock(&s_xCtrl, true);
//	ASSERT(xLock.IsLocked());

	//�״γ�ʼ��
	if(s_bufState[0] == 0)
	{
		memset(s_bufState, c_flagNone, MAXGAMESERVERS);
		memset(s_bufStateBackup, c_flagNone, MAXGAMESERVERS);
	}

	memcpy(buf, s_bufState, MAXGAMESERVERS);
	memcpy(s_bufState, s_bufStateBackup, MAXGAMESERVERS);
}
///////

time_t		g_tStartServerTime = 0;
///////
// ȫ�ֶ���

CPointThread *	g_pPointThread = NULL;		//?? �����ȶ���(POINTLISTENPORT);
CLoginThread *	g_pLoginThread = NULL;		//?? Ҫ������һ����(LOGINLISTENPORT);
CTimerThread *	g_pTimerThread = NULL;		//  

COnlineTable *	g_pOnlineTable = NULL;		// ������ұ�


CAccount		g_cDatabase;
bool			g_bEnableLogin = false;		//?? �ɸ�ΪSHARE�������

CServerAccount	g_aServerAccount[MAXGAMESERVERS];	// ��Ϸ�������б�
int				g_nServerAccount;					// ��Ϸ����������
char			g_szCountHeadLine[4096];			// ��Ϸ�������б�ı�����
ISP_SET			g_setISPList;						// IPS��IP��ַ�б�


//////////////////////////////////////////////////////////////////
// ȫ�ֺ���

DWORD	NewAuthenID(uint32 nClientIP)
{
	// ������֤ID��
	srand(clock() * nClientIP);
	return (OBJID)((rand()+1) * (rand()+1));
}

// ��β0��ɾβ��
bool	FormatStr(char* pszBuf, int nBufLen /*= 0*/)
{
	ASSERT(pszBuf);

	try{
		if(nBufLen)
			pszBuf[nBufLen-1] = 0;

		// ɾ��β�ո�
		char* ptr = pszBuf + strlen(pszBuf) - 1;
		while(ptr >= pszBuf && *ptr == ' ')
			*(ptr--) = 0;
	}catch(...){
		LOGCATCH("FormatStr(). invalid access.");
	}
	return false;
}
///////////////////////
// szPassword="" ��ʾ���������
bool	GetAccount(LPCTSTR szAccount, LPCTSTR szPassword, 
					OBJID& idAccount, int& nType, int& nPoint, DWORD& nPointTime, int& nBlock)			// ����ֵ
{
	idAccount	= ID_NONE;
	::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
	if(g_cDatabase.Create(szAccount, szPassword))
	{	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
		idAccount		= g_cDatabase.GetID();
		nBlock			= g_cDatabase.GetOnline();			// ��ONLINE
		nPoint			= g_cDatabase.GetPoint();
		nType			= g_cDatabase.GetType();
		nPointTime		= g_cDatabase.GetPointTime();
		g_cDatabase.Destroy();
		// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	}
	::ReleaseMutex(g_xDatabase);	//------------------------------------------
	return	(idAccount != ID_NONE);
}
///////////////////////
bool	IsFreeServer(LPCTSTR szServer)
{
	for(int i = 0; i < g_nServerAccount; i++)
	{
		if(g_aServerAccount[i].m_bFree && strcmp(g_aServerAccount[i].m_szServerName, szServer) == 0)
			return true;
	}
	return false;
}
///////////////////////
bool	CheckISP(OBJID idAccount, uint32 nClientIP, LPCTSTR pClientIP, 
				 OBJID& idFeeAccount, char* szFeeAccount, int& nPoint, DWORD& nPointTime, char* szNotifyMsg)	// ����ֵ
{
	bool	bIsISP = false;

	unsigned char ip1,ip2,ip3,ip4;
	ip1 = *( ((unsigned char*)&nClientIP) + 0);
	ip2 = *( ((unsigned char*)&nClientIP) + 1);
	ip3 = *( ((unsigned char*)&nClientIP) + 2);
	ip4 = *( ((unsigned char*)&nClientIP) + 3);

	for(int i = 0; i < g_setISPList.size(); i++)
	{
		int	fi1, fi2, fi3, fi4;
		if(sscanf(g_setISPList[i].szFirstIP, "%d.%d.%d.%d", &fi1, &fi2, &fi3, &fi4) != 4)
		{
			LOGERROR("invalud first ip format [%s]", g_setISPList[i].szFirstIP);
			continue;
		}

		int	la1, la2, la3, la4;
		if(sscanf(g_setISPList[i].szLastIP, "%d.%d.%d.%d", &la1, &la2, &la3, &la4) != 4)
		{
			LOGERROR("invalud last ip format [%s]", g_setISPList[i].szLastIP);
			continue;
		}

		if(ip1 >= fi1 && ip1 <= la1 
			&& ip2 >= fi2 && ip2 <= la2 
			&& ip3 >= fi3 && ip3 <= la3 
			&& ip4 >= fi4 && ip4 <= la4 )
		{
			if(pClientIP && g_pOnlineTable->CheckUniqueIP(pClientIP, idAccount))
			{
				::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
				if(g_cDatabase.Create(g_setISPList[i].szAccount, g_setISPList[i].szPassword))
				{	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
					if(g_cDatabase.GetPoint() > 0)
					{
						SafeCopy(szFeeAccount, g_cDatabase.GetName(), MAX_NAMESIZE);
						idFeeAccount	= g_cDatabase.GetID();
						nPoint			= g_cDatabase.GetPoint();
						bIsISP	= true;
						nPointTime		= g_cDatabase.GetPointTime();
					}
					else
					{
						// ISPת��ͨ�ʺ�
						sprintf(szNotifyMsg, FLAG_ISP_TO_NORMAL);
					}
					g_cDatabase.Destroy();
					// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
				}
				::ReleaseMutex(g_xDatabase);	//------------------------------------------
			}
			else
			{
				// ISPת��ͨ�ʺ�
				sprintf(szNotifyMsg, FLAG_ISP_TO_NORMAL);
			}

			return bIsISP;	// found it!
		}
	} // for

	if(pClientIP)
	{
		::WaitForSingleObject(g_xDatabase, INFINITE);	//+++++++++++++++++++++++++
		if(g_cDatabase.Create(c_typeISP, pClientIP))
		{	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
			SafeCopy(szFeeAccount, g_cDatabase.GetName(), MAX_NAMESIZE);
			idFeeAccount	= g_cDatabase.GetID();
			nPoint			= g_cDatabase.GetPoint();
			bIsISP	= true;
			nPointTime		= g_cDatabase.GetPointTime();
			g_cDatabase.Destroy();
			// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
		}
		::ReleaseMutex(g_xDatabase);	//------------------------------------------
	}

	return bIsISP;
}
///////////////////////
bool	Check7DaysNodify(DWORD nPointTime, char* szNotifyMsg)		// return szNotifyMsg
{
	time_t	tCurr = time(NULL);
	struct tm	tmCurr;
	tmCurr = *localtime(&tCurr);
	tmCurr.tm_mday	+= ACCOUNT_AHEAD_DAYS;
	time_t	tAhead	= mktime(&tmCurr);
	if(tAhead != (time_t)-1)
	{
		tm*		pTm	= localtime(&tAhead);
		int		nAhead	= (pTm->tm_year+1900)*10000 + (pTm->tm_mon+1)*100 + pTm->tm_mday;
		if(nAhead > nPointTime)
		{
			nPointTime %= 100000000;	// ��ֹszNotifyMsg���
			sprintf(szNotifyMsg, "%d��%d��%d��", 			// ��ע�⣺��һ�ֽڱ���Ϊ���֡��Ա���Ϸ������ͬ����
						(nPointTime/10000) % 10000, (nPointTime/100) % 100, nPointTime % 100);
			return true;
		}
	}
	return false;
}

