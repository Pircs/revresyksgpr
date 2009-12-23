#pragma once
///////////////////////////
// 用于隐藏内部名空间
//#define WORLDKERNEL_BEGIN	namespace world_kernel {
//#define WORLDKERNEL_END		};
///////////////////////////
//#pragma	warning(disable:4786)
//#include "basefunc.h"
//#include "define.h"
//class Msg;
//WORLDKERNEL_BEGIN
//
//class ISocket
//{
//public:
//	virtual bool SendMsg			(Msg* pMsg)				PURE_VIRTUAL_FUNCTION_0
//	virtual bool SendMsg			(SOCKET_ID idSocket, OBJID idMsg, const char* pBuf, int nMsgLen)	PURE_VIRTUAL_FUNCTION_0
//	virtual bool SendNpcMsg			(OBJID idNpc, OBJID idMsg, const char* pBuf, int nMsgLen)			PURE_VIRTUAL_FUNCTION_0
//	// 核心通知SOCKET关闭
//	virtual bool CloseSocket		(SOCKET_ID idSocket)			PURE_VIRTUAL_FUNCTION_0
//};
//
//class CNetMsg
//{
//public:
//	CNetMsg();
//	virtual ~CNetMsg();
//public:	
//	virtual bool			SendMsg		(CNetMsg* pMsg);
//protected:
//	PROCESS_ID	m_idProcess;
//	SOCKET_ID	m_idSocket;
//	OBJID		m_idNpc;
//	//int			GetTransData()						{ return m_nTransData; }
//
//};
//WORLDKERNEL_END