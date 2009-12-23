// define.h 全局类型和常量定义
// 仙剑修，2002.10.29
#pragma once

#pragma warning(disable:4786)

#include "common.h"
#include "myDDstring.h"
#include "Myheap.h"

#include "Msg.h"

// 帐号服务器相关常量
const int	LOGIN_FREE_LEVEL	= 16;			// 点卡收费的等级，小于该值不收费。
#define	FLAG_ISP_TO_NORMAL	"!ISP计费失败!"		// 注意：该字符串必须与游戏服务器相匹配。每个IP限一个帐号。否则自动转普通帐号并提示
#define	FLAG_NO_POINT		"!NO_POINT!"		// 注意：该字符串必须与游戏服务器相匹配。没有点数也可登录。由游戏服务器判断玩家等级是否可免费玩

const int	POINTFEE_SECS			= 3*60;			// 计点频率
const int	POINTFEE_DELAY_SECS		= 10;			// 首次计点延时
const int	SOCKET_SENDBUFSIZE		= 64*1024;			// SOCKET_BUF
const int	SOCKET_NPCBUFSIZE		= 128*1024;			// SOCKET_BUF
const int	SOCKET_ACCOUNTBUFSIZE	= 32*1024;			// SOCKET_BUF
const int	MAX_MAPGROUPSIZE		= 10;				// 最多10个地图组

// 类型
#define	IPP_OF(x)		((void**)&(x))

typedef	char			PARAMBUF[MAX_PARAMSIZE];
typedef	char			TENETSTR[MAX_TENETSIZE];

// 核心常量
//const int	ID_NONE			= 0;
const int	SOCKET_NONE		= -1;				// 无效的SOCKET索引
const int	PROCESS_NONE	= -1;				// 无效的PROCESS索引
const int	INVALID_ROOMID	= -1;				// 无效的ROOM_ID
const int	BCAST_NPCID		= 0;				// 发给NPC服务器的广播，由NPC服务器转发给相关NPC。
const int	INDEX_NONE		= -1;

const bool	UPDATE_TRUE				= true;
const bool	UPDATE_FALSE			= false;
const bool	DELRECORD_TRUE			= true;
const bool	DELRECORD_FALSE			= false;
const bool  INCLUDE_SELF			= true;
const bool  EXCLUDE_SELF			= false;
enum {SYNCHRO_FALSE=false, SYNCHRO_TRUE=true, SYNCHRO_BROADCAST};

// 征服常量
const int	DEFAULT_LOGIN_MAPGROUP		= 0;		// 登录点
const int	DEFAULT_LOGIN_MAPID			= 1000;		// 登录点
const int	DEFAULT_LOGIN_POSX			= 362;		// 登录点
const int	DEFAULT_LOGIN_POSY			= 594;		// 登录点

const int	CELLS_PER_BLOCK				= 18;		// 每ROOM的格子数
const int	CELLS_PER_VIEW				= 18;		// 可视区域的格子数
const int	ITEMTYPEMONEY_MIN			= 1090000;	// 最小堆钱
const int	ITEMTYPEMONEY_MAX			= 1091020;	// 最大堆钱
const int	SMALLHEAPMONEY_LIMIT		= 10;		// 小堆钱
const int	SMALLHEAPMONEY_TYPE			= 1090000;		// 小堆钱ITEMTYPE
const int	MIDDLEHEAPMONEY_LIMIT		= 100;		// 中堆钱
const int	MIDDLEHEAPMONEY_TYPE		= 1090010;		// 中堆钱ITEMTYPE
const int	BIGHEAPMONEY_LIMIT			= 1000;		// 大堆钱
const int	BIGHEAPMONEY_TYPE			= 1090020;		// 大堆钱ITEMTYPE

const int	SMALLHEAPGOLD_LIMIT			= 2000;		// 小堆金子
const int	SMALLHEAPGOLD_TYPE			= 1091000;		// 小堆金子ITEMTYPE
const int	MIDDLEHEAPGOLD_LIMIT		= 5000;		// 中堆金子
const int	MIDDLEHEAPGOLD_TYPE			= 1091010;		// 中堆金子ITEMTYPE
const int	BIGHEAPGOLD_LIMIT			= 10000;		// 大堆金子
const int	BIGHEAPGOLD_TYPE			= 1091020;		// 大堆金子ITEMTYPE

enum {
	OBJ_NONE	= 0x1000,
	OBJ_USER	= 0x1001,
	OBJ_MONSTER	= 0x1002,
	OBJ_ITEM	= 0x1004,
	OBJ_MAP		= 0x1008,
	OBJ_FRIEND	= 0x1010,
//	OBJ_NPCTYPE	= 0x1020, 
	OBJ_NPC		= 0x1040,
	OBJ_MAPITEM	= 0x1080,
	OBJ_SYN		= 0x1100,
	OBJ_BOOTH	= 0x1200,
	OBJ_TRAP	= 0x1400,
	OBJ_TUTOR	= 0x1800,
};
inline bool IsObjType(OBJID idObjType, OBJID idUnion) { return (idObjType & idUnion & 0x0FFF) != 0; }

// 通用接口

//#ifdef	WORLD_KERNEL
//#else
class Msg;
class ISocket
{
public:
	//virtual bool SendMsg			(Msg* pMsg)				PURE_VIRTUAL_FUNCTION_0
	virtual bool SendClientMsg		(SOCKET_ID idSocket, Msg* pMsg)	PURE_VIRTUAL_FUNCTION_0
	virtual bool SendNpcMsg			(OBJID idNpc, Msg* pMsg)			PURE_VIRTUAL_FUNCTION_0
	// 核心通知SOCKET关闭
	virtual bool CloseSocket		(SOCKET_ID idSocket)			PURE_VIRTUAL_FUNCTION_0
};

typedef	void			CGameSocket;		//? 兼容于幻灵的消息代码
//#endif

#ifdef	PALED_DEBUG_X
#define	DEBUG_CREATEMSG(x,y,a,s,n1,n2)	//{MSGBUF D_CM;sprintf(D_CM,"◎ msg:%s,id:%d,action:%d,%s,%d,%d",x,y,a,s,n1,n2);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#define	DEBUG_SENDMSG(x,y)				{MSGBUF D_CM;sprintf(D_CM,"↓ msgtype:%d,socketid:%d",x,y);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#define	DEBUG_PROCESSMSG(x,y,a,s,n1,n2)	{MSGBUF D_CM;sprintf(D_CM,"↑ msgtype:%d,socketid:%d,msg:%s,id:%d,action:%d,%s,%d,%d",GetType(),GetSocketID(),x,y,a,s,n1,n2);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#else
#define	DEBUG_CREATEMSG(x,y,a,s,n1,n2)	//{MSGBUF D_CM;sprintf(D_CM,"◎ msg:%s,id:%d,action:%d,%s,%d,%d",x,y,a,s,n1,n2);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#define	DEBUG_SENDMSG(x,y)				{}//{MSGBUF D_CM;sprintf(D_CM,"↓ msgid:%d,socketid:%d",x,y);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#define	DEBUG_PROCESSMSG(x,y,a,s,n1,n2)	{}//{MSGBUF D_CM;sprintf(D_CM,"↑ msg:%s,id:%d,action:%d,%s,%d,%d",x,y,a,s,n1,n2);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#endif

inline void EmptyFunction(...) {}
#ifdef MULTI_KERNEL_LOG
	#define LOGDEBUG	LOGMSG
#else
	#define LOGDEBUG	EmptyFunction
#endif