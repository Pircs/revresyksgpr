// define.h ȫ�����ͺͳ�������
// �ɽ��ޣ�2002.10.29
#pragma once

#pragma warning(disable:4786)

#include "common.h"
#include "myDDstring.h"
#include "Myheap.h"

#include "Msg.h"

// �ʺŷ�������س���
const int	LOGIN_FREE_LEVEL	= 16;			// �㿨�շѵĵȼ���С�ڸ�ֵ���շѡ�
#define	FLAG_ISP_TO_NORMAL	"!ISP�Ʒ�ʧ��!"		// ע�⣺���ַ�����������Ϸ��������ƥ�䡣ÿ��IP��һ���ʺš������Զ�ת��ͨ�ʺŲ���ʾ
#define	FLAG_NO_POINT		"!NO_POINT!"		// ע�⣺���ַ�����������Ϸ��������ƥ�䡣û�е���Ҳ�ɵ�¼������Ϸ�������ж���ҵȼ��Ƿ�������

const int	POINTFEE_SECS			= 3*60;			// �Ƶ�Ƶ��
const int	POINTFEE_DELAY_SECS		= 10;			// �״μƵ���ʱ
const int	SOCKET_SENDBUFSIZE		= 64*1024;			// SOCKET_BUF
const int	SOCKET_NPCBUFSIZE		= 128*1024;			// SOCKET_BUF
const int	SOCKET_ACCOUNTBUFSIZE	= 32*1024;			// SOCKET_BUF
const int	MAX_MAPGROUPSIZE		= 10;				// ���10����ͼ��

// ����
#define	IPP_OF(x)		((void**)&(x))

typedef	char			PARAMBUF[MAX_PARAMSIZE];
typedef	char			TENETSTR[MAX_TENETSIZE];

// ���ĳ���
//const int	ID_NONE			= 0;
const int	SOCKET_NONE		= -1;				// ��Ч��SOCKET����
const int	PROCESS_NONE	= -1;				// ��Ч��PROCESS����
const int	INVALID_ROOMID	= -1;				// ��Ч��ROOM_ID
const int	BCAST_NPCID		= 0;				// ����NPC�������Ĺ㲥����NPC������ת�������NPC��
const int	INDEX_NONE		= -1;

const bool	UPDATE_TRUE				= true;
const bool	UPDATE_FALSE			= false;
const bool	DELRECORD_TRUE			= true;
const bool	DELRECORD_FALSE			= false;
const bool  INCLUDE_SELF			= true;
const bool  EXCLUDE_SELF			= false;
enum {SYNCHRO_FALSE=false, SYNCHRO_TRUE=true, SYNCHRO_BROADCAST};

// ��������
const int	DEFAULT_LOGIN_MAPGROUP		= 0;		// ��¼��
const int	DEFAULT_LOGIN_MAPID			= 1000;		// ��¼��
const int	DEFAULT_LOGIN_POSX			= 362;		// ��¼��
const int	DEFAULT_LOGIN_POSY			= 594;		// ��¼��

const int	CELLS_PER_BLOCK				= 18;		// ÿROOM�ĸ�����
const int	CELLS_PER_VIEW				= 18;		// ��������ĸ�����
const int	ITEMTYPEMONEY_MIN			= 1090000;	// ��С��Ǯ
const int	ITEMTYPEMONEY_MAX			= 1091020;	// ����Ǯ
const int	SMALLHEAPMONEY_LIMIT		= 10;		// С��Ǯ
const int	SMALLHEAPMONEY_TYPE			= 1090000;		// С��ǮITEMTYPE
const int	MIDDLEHEAPMONEY_LIMIT		= 100;		// �ж�Ǯ
const int	MIDDLEHEAPMONEY_TYPE		= 1090010;		// �ж�ǮITEMTYPE
const int	BIGHEAPMONEY_LIMIT			= 1000;		// ���Ǯ
const int	BIGHEAPMONEY_TYPE			= 1090020;		// ���ǮITEMTYPE

const int	SMALLHEAPGOLD_LIMIT			= 2000;		// С�ѽ���
const int	SMALLHEAPGOLD_TYPE			= 1091000;		// С�ѽ���ITEMTYPE
const int	MIDDLEHEAPGOLD_LIMIT		= 5000;		// �жѽ���
const int	MIDDLEHEAPGOLD_TYPE			= 1091010;		// �жѽ���ITEMTYPE
const int	BIGHEAPGOLD_LIMIT			= 10000;		// ��ѽ���
const int	BIGHEAPGOLD_TYPE			= 1091020;		// ��ѽ���ITEMTYPE

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

// ͨ�ýӿ�

//#ifdef	WORLD_KERNEL
//#else
class Msg;
class ISocket
{
public:
	//virtual bool SendMsg			(Msg* pMsg)				PURE_VIRTUAL_FUNCTION_0
	virtual bool SendClientMsg		(SOCKET_ID idSocket, Msg* pMsg)	PURE_VIRTUAL_FUNCTION_0
	virtual bool SendNpcMsg			(OBJID idNpc, Msg* pMsg)			PURE_VIRTUAL_FUNCTION_0
	// ����֪ͨSOCKET�ر�
	virtual bool CloseSocket		(SOCKET_ID idSocket)			PURE_VIRTUAL_FUNCTION_0
};

typedef	void			CGameSocket;		//? �����ڻ������Ϣ����
//#endif

#ifdef	PALED_DEBUG_X
#define	DEBUG_CREATEMSG(x,y,a,s,n1,n2)	//{MSGBUF D_CM;sprintf(D_CM,"�� msg:%s,id:%d,action:%d,%s,%d,%d",x,y,a,s,n1,n2);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#define	DEBUG_SENDMSG(x,y)				{MSGBUF D_CM;sprintf(D_CM,"�� msgtype:%d,socketid:%d",x,y);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#define	DEBUG_PROCESSMSG(x,y,a,s,n1,n2)	{MSGBUF D_CM;sprintf(D_CM,"�� msgtype:%d,socketid:%d,msg:%s,id:%d,action:%d,%s,%d,%d",GetType(),GetSocketID(),x,y,a,s,n1,n2);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#else
#define	DEBUG_CREATEMSG(x,y,a,s,n1,n2)	//{MSGBUF D_CM;sprintf(D_CM,"�� msg:%s,id:%d,action:%d,%s,%d,%d",x,y,a,s,n1,n2);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#define	DEBUG_SENDMSG(x,y)				{}//{MSGBUF D_CM;sprintf(D_CM,"�� msgid:%d,socketid:%d",x,y);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#define	DEBUG_PROCESSMSG(x,y,a,s,n1,n2)	{}//{MSGBUF D_CM;sprintf(D_CM,"�� msg:%s,id:%d,action:%d,%s,%d,%d",x,y,a,s,n1,n2);CMsgTalk msg;if(msg.Create("DEBUG","DEBUG",D_CM))SendMsg(&msg);}
#endif

inline void EmptyFunction(...) {}
#ifdef MULTI_KERNEL_LOG
	#define LOGDEBUG	LOGMSG
#else
	#define LOGDEBUG	EmptyFunction
#endif