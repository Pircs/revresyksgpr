#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "Msg.h"
#include "define.h"

enum {
	WALKMODE_MOVE=0,					// PathMove()的模式
	WALKMODE_RUN,
	WALKMODE_SHIFT,
	WALKMODE_JUMP,

	WALKMODE_RUN_DIR0 = 20,
	WALKMODE_RUN_DIR7 = 27,
};
class CMsgWalk : public MsgWalk  
{
public:	
	void Process();
};

class CMsgPlayer : public MsgPlayer  
{
public:
	void Process(OBJID idNpc);
};

class CMsgWalkEx : public MsgWalkEx  
{
public:
	void Process();
};

class CMsgUserAttrib : public MsgUserAttrib
{
public:
	void Process();
};

class CMsgTalk : public MsgTalk  
{
public:
	void Process(OBJID idNpc);
};

class CMsgInteract : public MsgInteract  
{
public:
	void Process();
};

class CMsgAction : public MsgAction  
{
public:
	void Process(OBJID idNpc);
};

enum {
	MSGAINPCINFO_CREATENEW		= 1,		// NPCSERVER上传：要求创建并返回。NPCSERVER下传：添加
};
class CMsgAiNpcInfo  : public MsgAiNpcInfo
{
public:
	bool Create(int nAction, OBJID idNpc, OBJID idGen, int nType, OBJID idMap, int nPosX, int nPosY);
	void Process();
};

class CMsgItem : public MsgItem  
{
public:
	void Process(OBJID idNpc);
};

class CMsgItemInfo : public MsgItemInfo  
{
public:
	void Process(OBJID idNpc);
};

enum {
	MAGICSORT_ATTACK			= 1,
	MAGICSORT_RECRUIT			= 2,			// auto active too.
	MAGICSORT_CROSS				= 3,
	MAGICSORT_FAN				= 4,
	MAGICSORT_BOMB				= 5,
	MAGICSORT_ATTACHSTATUS		= 6,
	MAGICSORT_DETACHSTATUS		= 7,
	MAGICSORT_SQUARE			= 8,
	MAGICSORT_JUMPATTACK		= 9,			// move, a-lock
	MAGICSORT_RANDOMTRANS		= 10,			// move, a-lock
	MAGICSORT_DISPATCHXP		= 11,
	MAGICSORT_COLLIDE			= 12,			// move, a-lock & b-synchro
	MAGICSORT_SERIALCUT			= 13,			// auto active only.
	MAGICSORT_LINE				= 14,			// support auto active(random).
	MAGICSORT_ATKRANGE			= 15,			// auto active only, forever active.
	MAGICSORT_ATKSTATUS			= 16,			// support auto active, random active.
	MAGICSORT_CALLTEAMMEMBER	= 17,
	MAGICSORT_RECORDTRANSSPELL	= 18,
	MAGICSORT_TRANSFORM			= 19,
	MAGICSORT_ADDMANA			= 20,			// support self target only.
	MAGICSORT_LAYTRAP			= 21,
	MAGICSORT_DANCE				= 22,			// 跳舞(only use for client)
};
class CMsgMagicEffect : public MsgMagicEffect
{
public:
	void Process();
};

class CMsgTeam : public MsgTeam  
{
public:
	void Process(OBJID idNpc);
};

class CMsgTeamMember : public MsgTeamMember  
{
public:
	void Process(OBJID idNpc);
};

class CMsgUserInfo : public MsgUserInfo
{
public:	
	void Process();
};

class CMsgSyndicate : public MsgSyndicate
{
public:	
	void Process(OBJID idNpc);
};