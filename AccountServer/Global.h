// ȫ�ֱ���
// �ɽ��ޣ�2001.11.19
#pragma once
#include <vector>
extern HANDLE	g_xDatabase;
//////////////////////////////////////////////////////////////////
// ����ͳ��
extern long	s_nSocketCount;
extern long	s_nLoginCount;
extern long	s_nMaxServerTime;
extern long	s_nAvgServerTime;
extern long	s_nPointCount;
extern long s_nLoginAccept;
extern long s_nPointFee;
extern long s_nPointSum;
extern long s_nAllTimeSum;
extern long s_nDatabaseTimeSum;

void	PrintText(const char * szFormat, ...);
const char* getCurrTimeString();
bool	LockedGetText(char * buf);

enum	{ c_flagNone='.', c_flagOffline=',', c_flagSocket=':', c_flagAccount='-', c_flagStop='#', c_flagNormal='=', c_flagNormal91U='U', 
						c_flagBaseMax='@',					// ���ֻ������ź���ʱ���ŵķֽ� ==============================================
						c_flagBegin='S', c_flagPoint='Y', c_flagEnd='X', c_flagHeartbeat='?', 
						c_flagBegin2='s', c_flagPoint2='y', c_flagEnd2='x' };
void	SetServerState(int nServer, int nState);
void	GetServerState(char * buf);
//////////////////////////////////////////////////////////////////

extern	time_t	g_tStartServerTime;				// log need

//////////////////////////////////////////////////////////////////
// IPS�б���
typedef	char	NAMESTR[MAX_NAMESIZE];
struct	ISP_ST{
	NAMESTR		szName;
	NAMESTR		szFirstIP;
	NAMESTR		szLastIP;
	NAMESTR		szAccount;
	NAMESTR		szPassword;
};
typedef vector<ISP_ST>		ISP_SET;

//////////////////////////////////////////////////////////////////
// ȫ�ֶ���

extern COnlineTable *	g_pOnlineTable;		// ������ұ�
extern CPointThread *	g_pPointThread;		// (POINTLISTENPORT);
extern CLoginThread *	g_pLoginThread;		// (LOGINLISTENPORT);
extern CTimerThread *	g_pTimerThread;		// (LOGINLISTENPORT);

extern CAccount			g_cDatabase;
extern bool				g_bEnableLogin;

extern CServerAccount	g_aServerAccount[];		// ��Ϸ�������б�
extern int				g_nServerAccount;		// ��Ϸ����������
extern char				g_szCountHeadLine[];	// ��Ϸ�������б�ı�����
extern ISP_SET			g_setISPList;			// IPS��IP��ַ�б�


//////////////////////////////////////////////////////////////////
// ȫ�ֺ���

DWORD	NewAuthenID(uint32 nClientIP);
bool	FormatStr(char* pszBuf, int nBufLen = 0);

//////////////////////////////////////////////////////////////////
enum	{	c_errPassword		= 1,		// �ʺ������������
			c_errNotPoint		= 6,		// �Ƶ��ʺţ�û����
			c_errNotTime		= 7, 		// �����ʺţ�������
			c_errUnknowServer	= 10,		// GAME��������������
			c_errOnline			= 11,		// �Ѿ�����һ��ҵ�¼����Ϸ(���ٴε�¼)
			c_errBan			= 12, 		// �ѱ����
			c_errBarPassword	= 13, 		// ���ɵ�¼ģʽ���ʺ������������
			c_errBarOverflow	= 14, 		// ���ɵ�¼ģʽ����������ɵ�¼����
			c_errServerBusy		= 20,		// ������æ
			c_errServerFull		= 21,		// ����������
			c_errUnknown		= 999		// δ֪
};

bool	GetAccount(LPCTSTR szAccount, LPCTSTR szPassword, 
					OBJID& idAccount, int& nType, int& nPoint, DWORD& nPointTime, int& nBlock);			// ����ֵ
bool	IsFreeServer(LPCTSTR szServer);
bool	CheckISP(OBJID idAccount, uint32 nClientIP, LPCTSTR pClientIP, 
				 OBJID& idFeeAccount, char* szFeeAccount, int& nPoint, DWORD& nPointTime, char* szNotifyMsg);	// ����ֵ
bool	Check7DaysNodify(DWORD nPointTime, char* szNotifyMsg);		// return szNotifyMsg