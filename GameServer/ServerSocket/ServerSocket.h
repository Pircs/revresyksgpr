#pragma once

#pragma warning(disable:4786)
#include <time.h>
#include "define.h"
#include <vector>
#include <map>
using namespace std;

class CSocketKernel : ISocketKernel
{
public:
	CSocketKernel() {}		// ��������
	virtual ~CSocketKernel() {}		// ��������

public:
	ISocketKernel*	GetInterface() { return (ISocketKernel*)this; }

public:
	virtual bool	Create(IMessagePort* pPort);
	virtual bool	ProcessMsg(OBJID idPacket, void* buf, int nType, int nFrom);
	virtual bool	ProcessSocket();
	virtual bool	OnTimer(time_t tCurr);
	virtual bool	Release();

public:
	bool	SendCloseNotify(SOCKET_ID idSocket);
	//? ��ʱ���ڿ���ֻ֪ͨ���Ĺر�һ��
	bool	IsNetBreakFlag(SOCKET_ID idSocket)			{ return (idSocket>=0 && idSocket<m_setSocket.size() && m_setSocket[idSocket] && m_setNetBreakFlag[idSocket]); }
	void	SetNetBreakFlag(SOCKET_ID idSocket)			{ if(idSocket>=0 && idSocket<m_setSocket.size() && m_setSocket[idSocket]) m_setNetBreakFlag[idSocket] = true; }

protected:
	void	AcceptNewSocket();
	bool	SendPacket(SOCKET_ID idSocket, const Msg* pMsg);

protected:
	IListenSocket*	m_pListen;
	CListenSocket	m_cListen;
	vector<IServerSocket*>	m_setSocket;				// SOCKET_ID��Ӧ����,�����ĸ�SETͬ�����ɾ��!
	vector<CServerSocket*>	m_setServerSocket;
	vector<PROCESS_ID>		m_setProcessID;				// PROCESS_ID��ӦMSGPORT��
	vector<bool>			m_setNetBreakFlag;
	IMessagePort*	m_pMsgPort;
	SOCKET_ID		m_idNpcSocket;
	typedef map<OBJID, PROCESS_ID>	NPCPROCESSID_MAP;
	NPCPROCESSID_MAP	m_setNpcProcessID;			// ÿ��NPC��PROCESS_ID����NPC����Ϣ����WORLD
};