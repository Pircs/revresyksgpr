// define.h 全局类型和常量定义
// 仙剑修，2002.10.29

#pragma	warning(disable:4786)

#ifndef	ALL_GLOBAL_DEFINE_H
#define	ALL_GLOBAL_DEFINE_H

#include "common.h"
#include "mycom.h"

// #define ENG_RC
#ifdef ENG_RC
#include "eng_rc.h"
#else
#include "chn_rc.h"
#endif

#include "Msg.h"


// 运行控制
const int	RESEND_MSGLOGIN_SECS		= 5;			// 登录到游戏服务器的速度



typedef	char			NETMSGBUF[1024];				// 网络包的BUF
typedef	char			FIELDBUF[MAX_FIELDSIZE];



// 核心常量
//const int	ID_NONE			= 0;
const int	INVALID_ID		= -1;				// 无效的SOCKET索引
const int	INVALID_ROOMID	= -1;				// 无效的ROOM_ID
const int	BCAST_NPCID		= 0;				// 发给NPC服务器的广播，由NPC服务器转发给相关NPC。

const bool	UPDATE_TRUE				= true;
const bool	UPDATE_FALSE			= false;
const bool	DELRECORD_TRUE			= true;
const bool	DELRECORD_FALSE			= false;
const bool	SYNCHRO_TRUE			= true;
const bool	SYNCHRO_FALSE			= false;


// 彩江常量
const int	DEFAULT_LOGIN_MAPGROUP		= 0;		// 登录点
const int	DEFAULT_LOGIN_MAPID			= 1000;		// 登录点
const int	DEFAULT_LOGIN_POSX			= 44;		// 登录点
const int	DEFAULT_LOGIN_POSY			= 44;		// 登录点
const int	DEFAULT_LOGIN_DIR			= 0;
const int	DEFAULT_LOGIN_STATUS		= 0;		// STATUS_NORMAL
const int	DEFAULT_LOGIN_POSE			= 0;
const int	DEFAULT_LOGIN_EMOTION		= 0;

const int	CELLS_PER_BLOCK				= 18;		// 每ROOM的格子数
const int	CELLS_PER_VIEW				= 18;		// 可视区域的格子数
const int	BLOCKWIDTH_PER_MAP			= 100;		// 地图X方向ROOM数量，不宜过大，空耗内存

const int	DEFAULT_MAP_WIDTH			= 50;		// 每张地图的ROOM数
const int	DEFAULT_MAP_HEIGHT			= 50;

const int	MAPS_PER_MAPGROUP			= 1;

enum {
	OBJ_NONE	= 1234,
	OBJ_USER,
	OBJ_ITEM,
	OBJ_MAP,
	OBJ_FRIEND,
	OBJ_NPC,
	OBJ_NPCGEN,
	OBJ_NPCTYPE,
	OBJ_AGENT,
};

class CUser;
class CNpc;
class CAgent;
GUID_DECLARE(CUser,OBJ_USER)
GUID_DECLARE(CNpc,OBJ_NPC)
GUID_DECLARE(CAgent,OBJ_AGENT)

#define NOMATE_NAME		"无"						// 无配偶的配偶名

#define	SYNNAME_NONE	"无"						// 无帮派玩家的帮派名
#define	SYNLEADER_TITLE	"帮主"						// 帮主的TITLE
#define	SYNMEMBER_TITLE	"普通帮众"						// 新入帮玩家的TITLE
#define	NOSYN_TITLE		"无"						// 无帮派玩家的TITLE

// 征服常量
const int	STATESTR_SIZE		= 2048;


/*class CNetMsg;
class ISocket
{
public:
	virtual bool SendMsg			(CNetMsg* pNetMsg)				PURE_VIRTUAL_FUNCTION_0
	virtual bool SendMsg			(SOCKET_ID idSocket, OBJID idMsg, const char* pBuf, int nMsgLen)	PURE_VIRTUAL_FUNCTION_0
	virtual bool SendNpcMsg			(OBJID idNpc, OBJID idMsg, const char* pBuf, int nMsgLen)			PURE_VIRTUAL_FUNCTION_0
	// 核心通知SOCKET关闭
	virtual bool CloseSocket		(SOCKET_ID idSocket)			PURE_VIRTUAL_FUNCTION_0
};
*/
typedef	void			CGameSocket;		// 兼容于幻灵的消息代码

// 行走方向
const int _DELTA_X[9]={ 0,-1,-1,-1, 0, 1, 1, 1, 0 };
const int _DELTA_Y[9]={ 1, 1, 0,-1,-1,-1, 0, 1, 0 };
const int MAX_DIRSIZE = 8;
inline int GetDirByPos(int nFromX, int nFromY, int nToX, int nToY)		// return MAX_DIRSIZE: error
{
	if(nFromX < nToX)
	{
		if(nFromY < nToY)
			return 7;
		else if(nFromY > nToY)
			return 5;
		else
			return 6;
	}
	else if(nFromX > nToX)
	{
		if(nFromY < nToY)
			return 1;
		else if(nFromY > nToY)
			return 3;
		else
			return 2;
	}
	else // if(nFromX == nToX)
	{
		if(nFromY < nToY)
			return 0;
		else if(nFromY > nToY)
			return 4;
	}
	return MAX_DIRSIZE;
}


#endif // ALL_GLOBAL_DEFINE_H