#pragma once
#define	_WINSOCKAPI_		// 阻止加载winsock.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "Msg.h"
#include "NetMsg.h"

//using namespace world_kernel;		// all msg use namespace world_kernel!!!
//#include "MsgTalk.h"
//#include "MsgMessageBoard.h"
//#include "MsgRegister.h"
//#include "MsgLogin.h"
//#include "MsgAiNpcInfo.h"
//#include "MsgConnect.h"
//#include "MsgName.h"
//#include "MsgFriend.h"
//#include "MsgChatRoom.h"

#include "protocol.h"
enum {
	MSGAINPCINFO_CREATENEW		= 1,		// NPCSERVER上传：要求创建并返回。NPCSERVER下传：添加
};
class CMsgAiNpcInfo  : public MsgAiNpcInfo
{
public:
	bool Create(ST_CREATENEWNPC* pInfo);
	void Process();
};
//
class CMsgConnect : public MsgConnect  
{
public:
	void Process(SOCKET_ID idSocket);
};
//
class CMsgFriend : public MsgFriend
{
public:
	void Process(int nTrans);
};
//
enum {
	MSG_LOGIN_REQUEST=0,
	MSG_LOGIN_OK,
	MSG_LOGIN_ERR_VERSION,
	MSG_LOGIN_ERR_DOUBLELOGIN,
};
class CMsgLogin : public MsgLogin 
{
public:
	void Process(SOCKET_ID idSocket, OBJID idNpc);
protected:
	bool ProcessNpcServerLogin(SOCKET_ID idSocket, bool bDelAllMonster);
};
//
#include "MessageBoard.h"
class CMsgMessageBoard : public MsgMessageBoard  
{
public:
	void Process(SOCKET_ID idSocket);
	bool Create	(int nAction, int nChannel, int nIndex, CMessageBoard* pObj=NULL);
};
//
class CMsgName : public MsgName   
{
public:	
	void Process(SOCKET_ID idSocket, int nTrans);
};
//
class CMsgRegister : public MsgC2SRegister  
{
public:	
	void Process(SOCKET_ID idSocket);
};
//
class CMsgTalk : public MsgTalk  
{
public:
	void Process(SOCKET_ID idSocket);
};
//
//WORLDKERNEL_BEGIN
////define constant ..
//
//WORLDKERNEL_END