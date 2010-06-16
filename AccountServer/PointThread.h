// PointThread.h: �Ƶ��߳���
// �ɽ��ޣ�2001.11.20

#pragma once

#include "T_Index.h"
#include "ThreadBase.h"

struct	CServerAccount
{
	char	m_szServerName[SERVERNAMESIZE];
	char	m_szLoginName[SERVERNAMESIZE];
	char	m_szPassword[SERVERNAMESIZE];
	int		m_bFree;
	CServerAccount() { 
		m_szServerName[0] = m_szLoginName[0] = m_szPassword[0] = m_bFree = 0;
	}
};

class CPointThread : public CThreadBase  
{
public:		// ���졢����
	CPointThread(u_short nPort, int nSndSize = 0);
	virtual ~CPointThread();

public:		// �����������к�����ʼǰ���뻥�⣬���ܵ����ڲ��������¼�������
	enum { ERR_NONE=0, ERR_NO_SERVER, ERR_BUSY, ERR_FULL };
	int		NewLogin(OBJID idAccount, DWORD nAuthenID, const char * szClientIP, const char * pGameServer);	// �ⲿ���ã�Ҫ����
	bool	GetServerIP(char * bufIP, const char * pServerName);
//	int 	GetServerIndex(const char * pServerName) { CSingleLock(&m_xCtrl, true); return GetServerIndex_0(pServerName); }	// ����0������
	int		GetServerCount();
	int		GetServerState(LPCTSTR szServerName);
//	bool	GetServerName(OBJID idServer, char bufName[SERVERNAMESIZE]);
	bool	CheckHeartbeatAll();
	void	LogCount();
	void	LogSerialCount();
	void	LogServer();
	bool	Kickout(OBJID idAccount);

protected:	// ����
	virtual void	OnInit();
	virtual bool	OnProcess();
	virtual void	OnDestroy();

public:
	enum { STATE_OFFLINE=0, STATE_NORMAL, STATE_BUSY, STATE_FULL, STATE_NORMAL91U, };
protected:	// ����
	int 	m_aState[MAXGAMESERVERS];
	CServerSocket<POINT_KEY1, POINT_KEY2>	m_aServerSocket[MAXGAMESERVERS];		//? ע����m_aServerNameͬ��
	char	m_aServerName[MAXGAMESERVERS][SERVERNAMESIZE];	//? ע����m_aServerSocketͬ����������ʾ���ߡ�
	CListenSocket	m_cListenSocket;
	time_t	m_aHeartbeat[MAXGAMESERVERS];
	time_t	m_aHeartbeatLast[MAXGAMESERVERS];
	time_t	m_aServerDelay[MAXGAMESERVERS];
	CBanIP *		m_pBanIPs;
private:	// �����ڲ�ʹ�ã������л���
	bool	Kickout_0(OBJID idAccount, LPCTSTR szServerName);
	bool	ProcessMsg(int nServerIndex,  char * pBuf, int nLen);
	OBJID	GetServerIndex_0(const char * pServerName);	// ����0������
	bool	CheckHeartbeat(int nIndex);
	void	Clear(int nIndex, bool flag = true);
	void	SetServerBusy(int nServerIndex);
	void	SetServerFull(int nServerIndex);
};