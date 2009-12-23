#include <time.h>
#include <winsock2.h>
#include "I_mydb.h"
#include "define.h"	// Added by ClassView
#include "usermanager.h"
#include "network\\NetMsg.h"
#include "I_Shell.h"
#pragma once


class CMapGroupKernel : IMapGroup, ISocket
{
public:
	CMapGroupKernel() {}		// 建议留空
	virtual ~CMapGroupKernel() {}		// 建议留空

public:
	IMapGroup*	GetInterface() { return (IMapGroup*)this; }
	ISocket*	GetSocketInterface() { return (ISocket*)this; }

protected: // IMapGroup
	virtual bool	Create(IMessagePort* pPort);
	virtual void	SynchroData();
	virtual bool	ProcessMsg(OBJID idPacket, void* buf, int nType, int nFrom);
	virtual bool	OnTimer(time_t tCurr);
	virtual bool	Release();

protected: // ISocket
	//bool SendMsg			(CNetMsg* pNetMsg);
	bool SendClientMsg		(SOCKET_ID idSocket, Msg* pMsg);
	bool SendNpcMsg			(OBJID idNpc, Msg* pMsg);
	bool CloseSocket		(SOCKET_ID idSocket);		// 直接关闭socket

protected:
	bool ProcessMsg(SOCKET_ID idSocket, OBJID idNpc, const Msg* pMsg, int nTrans=0);
	bool ProcessClientMsg(SOCKET_ID idSocket, const Msg* pMsg, int nTrans=0);
	bool ProcessNpcMsg(OBJID idNpc, const Msg* pMsg, int nTrans=0);
	bool BroadcastMapGroupMsg(Msg *pMsg);
	PROCESS_ID	GetProcessID() { return m_idProcess; }

protected:
	PROCESS_ID		m_idProcess;
	IDatabase*		m_pDb;
	IMessagePort*	m_pMsgPort;

public:
	MYHEAP_DECLARATION(s_heap)
};


