// �궨��
// �ɽ��ޣ�2001.11.19

#include "AllHeads.h"
#include "typedef.h"


#define	GETINTROUND(x,y,z)	x = ini.GetInt(#x);\
	if(x < (y))\
		x = (y);\
	else if(x > (z))\
		x = (z);\
	LOGMSG("config.ini->" #x " = %d", x);

#define	GETINTERROR(x,y,z)	x = ini.GetInt(#x);\
	if(x < (y) || x > (z))\
	{\
		LOGERROR("illegal " #x " in config.ini");\
		return false;\
	}\
	LOGMSG("config.ini->" #x " = %d", x);

#define	GETSTRING(x,y)	if(!ini.GetString(x, #x, INIWORDSIZE))\
	{\
		LOGERROR("illegal " #x " in config.ini");\
		return false;\
	}\
	if(y && x[0] == 0)\
	{\
		LOGERROR(#x "can't be NULL in config.ini");\
		return false;\
	}\
	LOGMSG("config.ini->" #x " = \"%s\"", x);

//#define	INIDEFINE(x)	int x;//////////
char	SERVERKEY[INIWORDSIZE];
int		LOGINLOOPDELAY;
int		POINTLOOPDELAY;
int		REBUILDLISTENDELAYSEC;
int		LOGINLISTENPORT;
int		POINTLISTENPORT;
int		POINTSOCKETSNDBUF;
int		MAXBANIPS;
int		BANERRORS;
int		BANERRORSECS;
int		BANSECS;
int		ONLINETABLESIZE;
int		HEARTBEATINTERVALSECS;
int		HEARTBEATKICKSECS;
int		ONLINEHEARTBEATSECS;
int		POINTFEEINTERVALSECS;
int		ENABLELOGINDELAY;
int		OUTPUTONLINESECS;
int		REJOINTIMEWAITSECS;
int		COUNTFILESECS;
int		ACCOUNT_AHEAD_DAYS;
int		SERVER_BUSY_DELAY_SECS;
int		SERVER_FULL_DELAY_SECS;
int		KICKOUT_USER_WHEN_NO_POINT;
int		ENABLE_LOGIN_NO_POINT;
int		MONTH_CARD_TO_POINT_CARD;
int		MAX_GM_ID;
int		STAT_SERIAL;
char	SERVER_TITLE[INIWORDSIZE];
int		ENABLE_91U_CREATE_ACCOUNT;

char	DBHOSTNAME[INIWORDSIZE];
char	DBUSER[INIWORDSIZE];
char	DBPASSWORD[INIWORDSIZE];
char	DATABASENAME[INIWORDSIZE];
char    POINTTABLE[INIWORDSIZE];   // ��config.iniָ����ҵ���������
//////////
bool GetConfig()
{//////////

		CIniFile	ini("config.ini", "System");
		GETSTRING(SERVERKEY, true);
		GETINTROUND(LOGINLOOPDELAY,			10, 5000);
		GETINTROUND(POINTLOOPDELAY,			0, 500);
		GETINTROUND(REBUILDLISTENDELAYSEC,	0, 60);
		GETINTERROR(LOGINLISTENPORT,		1024, 32767);
		GETINTERROR(POINTLISTENPORT,		1024, 32767);
		GETINTROUND(POINTSOCKETSNDBUF,		0, 1048576);
		GETINTROUND(MAXBANIPS,				1, 1000);
		GETINTROUND(BANERRORS,				1, 100);
		GETINTROUND(BANERRORSECS,			0, 86400);
		GETINTROUND(BANSECS,				0, 86400);
		GETINTROUND(ONLINETABLESIZE,		1000, 1000000);
		GETINTROUND(HEARTBEATINTERVALSECS,	10, 86400);
		GETINTROUND(HEARTBEATKICKSECS,		HEARTBEATINTERVALSECS, 86400);
		GETINTROUND(POINTFEEINTERVALSECS,	20, 86400);
		GETINTROUND(ONLINEHEARTBEATSECS,	POINTFEEINTERVALSECS*2, 86400);	//? ������ڼƵ�ʱ��Ƭ������
		GETINTROUND(ENABLELOGINDELAY,		0, 86400);
		if(ENABLELOGINDELAY <= POINTFEEINTERVALSECS)
			LOGWARNING("��¼�ӳ�ʱ��ENABLELOGINDELAYӦ���ڼƵ���POINTFEEINTERVALSECS���������߱���ͬ����");
		GETINTROUND(OUTPUTONLINESECS,		1, 86400);
		GETINTROUND(REJOINTIMEWAITSECS,		0, 3600);
		GETINTROUND(COUNTFILESECS,			1, 86400);
		GETINTROUND(ACCOUNT_AHEAD_DAYS,		0, 365);
		GETINTROUND(SERVER_BUSY_DELAY_SECS,	0, 86400);
		GETINTROUND(SERVER_FULL_DELAY_SECS,	0, 86400);
		GETINTROUND(KICKOUT_USER_WHEN_NO_POINT,	0, 1);
		GETINTROUND(ENABLE_LOGIN_NO_POINT,	0, 1);
		GETINTROUND(MONTH_CARD_TO_POINT_CARD,	0, 1);
		GETINTROUND(MAX_GM_ID,	0, 1000000);
		GETINTROUND(STAT_SERIAL,	0, 1);
		ini.GetString(SERVER_TITLE,"SERVER_TITLE",INIWORDSIZE);
		GETINTROUND(ENABLE_91U_CREATE_ACCOUNT,	0, 1);

		ini.SetSection("Database");
		GETSTRING(DBHOSTNAME, true);
		GETSTRING(DBUSER, true);
		GETSTRING(DBPASSWORD, false);	// ����Ϊ�մ�
		GETSTRING(DATABASENAME, true);
		
		ini.SetSection("Special");
		GETSTRING(POINTTABLE, true);
//////////

		CIniFile	ini2("isp.ini", "MAIN");
		int nMaxPart = ini2.GetInt("MaxPart");
		if(nMaxPart > 100)
			nMaxPart = 100;
		for(int i = 0; i <= nMaxPart; i++)
		{
			char buf[INIWORDSIZE];
			sprintf(buf, "PART%d", i);
			ini.SetSection(buf);

			ISP_ST		st;
			if(!ini2.GetString(st.szName,"Name",MAX_NAMESIZE))
				continue;
			ini2.GetString(st.szFirstIP, "FirstIP", MAX_NAMESIZE);
			ini2.GetString(st.szLastIP, "LastIP", MAX_NAMESIZE);
			ini2.GetString(st.szAccount, "Account", MAX_NAMESIZE);
			ini2.GetString(st.szPassword, "Password", MAX_NAMESIZE);
			if(strlen(st.szName) > 0)
				 g_setISPList.push_back(st);
		}
//////////

	return true;
}


bool	GetServerAccount()
{
	CIniFile	ini("Account.ini", "Account");
//	g_nServerAccount = ini.GetInt("ACCOUNTNUMS");

	// �����ʺ���
//	if(g_nServerAccount > MAXGAMESERVERS)
//		g_nServerAccount = MAXGAMESERVERS;
//	LOGACCOUNT("Account.ini -> ACCOUNTNUMS = %d", g_nServerAccount);

	int i;
	for(i = 1; i < MAXGAMESERVERS; i++)	//?? 1: ��1��ʼ������0��
	{
		char bufName[250];
		char bufString[SERVERNAMESIZE];
		sprintf(bufName, "Account%d", i);
		ini.SetSection(bufName);

		ini.GetString(bufString, "SERVERNAME", SERVERNAMESIZE);
#ifdef	DUMP
		LOGSERVER("Account.ini -> SERVERNAME = \"%s\"", bufString);
#endif
		strcpy(g_aServerAccount[i].m_szServerName, bufString);

		ini.GetString(bufString, "LOGINNAME",SERVERNAMESIZE);
#ifdef	DUMP
		LOGSERVER("Account.ini -> LOGINNAME = \"%s\"", bufString);
#endif
		strcpy(g_aServerAccount[i].m_szLoginName, bufString);

		ini.GetString(bufString, "PASSWORD", SERVERNAMESIZE);
#ifdef	DUMP
		LOGSERVER("Account.ini -> PASSWORD = \"%s\"", bufString);
#endif
		strcpy(g_aServerAccount[i].m_szPassword, bufString);

		g_aServerAccount[i].m_bFree		= ini.GetInt("FREE_DEBUG");
#ifdef	DUMP
		LOGSERVER("Account.ini -> FREE_DEBUG = \"%d\"", m_bFree);
#endif

		g_aServerAccount[i].m_b91U		= ini.GetInt("91U_ACCOUNT");
#ifdef	DUMP
		LOGSERVER("Account.ini -> 91U_ACCOUNT = \"%d\"", m_b91U);
#endif

		if(g_aServerAccount[i].m_szServerName[0] == 0 || g_aServerAccount[i].m_szLoginName[0] == 0)
			break;
		if(strlen(g_aServerAccount[i].m_szServerName) >= SERVERNAMESIZE
							|| strlen(g_aServerAccount[i].m_szLoginName) >= SERVERNAMESIZE
							|| strlen(g_aServerAccount[i].m_szPassword) >= SERVERNAMESIZE)
			break;
	}

	g_nServerAccount = i;
	LOGMSG("������[%d]����Ϸ�������ʺ�", g_nServerAccount - 1);
	PrintText("����%d����Ϸ�������ʺ�", g_nServerAccount - 1);

	// ����g_szCountHeadLine
	sprintf(g_szCountHeadLine, "ʱ��                | %12s", "      �ϼ�");
	for( i = 1; i < g_nServerAccount; i++)	// 1: ��1��ʼ��
	{
		char	buf[1024];
		sprintf(buf, " | %12s", g_aServerAccount[i].m_szServerName);
		strcat(g_szCountHeadLine, buf);
	}
	strcat(g_szCountHeadLine, "\n");

	return true;
}






