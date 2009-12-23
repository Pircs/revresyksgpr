// �����̲߳���
// �ɽ���, 2001.10.10
#pragma once

#ifdef	ENCRYPT
	#include "..\GameServer\ServerSocket\EncryptServer.h"
#endif

#include "Msg.h"
	//	2��CServerSocket���������ڷ����߳��д������������
template <uint32 key1, uint32 key2>
class	CServerSocket
{
public:
	CServerSocket();
	~CServerSocket();
public:
//	��Ҫ������
	bool	Open(SOCKET sockServer, u_long nClientIP);

	//*** ����һ����Ϣ��д��pBuf��ָ�Ļ������С�����false: �����رա�����trueʱnLen��0��û������
	bool	Recv(char * pBuf, int &nLen);

	bool	Send(const char * pBuf, int nLen);	// ����false: �������
	bool	Send(const Msg* pMsg);	// ����false: �������

	//	�ر�д�������Ա��öԷ������ر�SOCKET��δʹ��
	bool	ShutDown();
	//	���ظ����á�ͨ�����ã����������ر�SOCKET��
	void	Close(bool bLinger0 = false);
	//	����������ֱ�ӷ���m_sockServer��Ա������m_bStateΪfalseʱ����INVALID_SOCKET��
	SOCKET	Socket() { return m_sockServer; }
	bool	IsOpen() { return m_bState != 0; }
	bool	IsShutDown() { return m_bState == c_stateShutDown; }
	char *	GetPeerIP();		// ����NULLΪ���մ���������
protected:
	time_t m_tLastRecv;
//	��Ҫ��Ա������
	//	���߳������������ҵ����̵߳�ȫ���й��������
	SOCKET	m_sockServer;
	//	�Ƿ����������ݡ�����ShutDown()֮�󽫲���д��
//	bool	m_bSendEnable;
	//	����״̬�Ƿ������ı�־��Ϊfalseʱ��ʾSOCKET�Ѳ����á�
	enum	{ c_stateNone = 0, c_stateNormal, c_stateShutDown }	m_bState;
//	��Ҫ��Ա������
	char 	m_bufPeerIP[IPSTRSIZE];		// ���󲻱��и����ԣ���Ϊ CConnect ������Ҳ�д����ԡ�
#ifdef	ENCRYPT
	typedef		CEncryptServer<key1, key2>	POINTENCRYPT;
	POINTENCRYPT	m_cEncryptRecv;
	POINTENCRYPT	m_cEncryptSend;
#endif
};

#include "ServerSocket.hpp"