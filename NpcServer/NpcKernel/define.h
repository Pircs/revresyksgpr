// define.h ȫ�����ͺͳ�������
// �ɽ��ޣ�2002.10.29

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


// ���п���
const int	RESEND_MSGLOGIN_SECS		= 5;			// ��¼����Ϸ���������ٶ�



typedef	char			NETMSGBUF[1024];				// �������BUF
typedef	char			FIELDBUF[MAX_FIELDSIZE];



// ���ĳ���
//const int	ID_NONE			= 0;
const int	INVALID_ID		= -1;				// ��Ч��SOCKET����
const int	INVALID_ROOMID	= -1;				// ��Ч��ROOM_ID
const int	BCAST_NPCID		= 0;				// ����NPC�������Ĺ㲥����NPC������ת�������NPC��

const bool	UPDATE_TRUE				= true;
const bool	UPDATE_FALSE			= false;
const bool	DELRECORD_TRUE			= true;
const bool	DELRECORD_FALSE			= false;
const bool	SYNCHRO_TRUE			= true;
const bool	SYNCHRO_FALSE			= false;


// �ʽ�����
const int	DEFAULT_LOGIN_MAPGROUP		= 0;		// ��¼��
const int	DEFAULT_LOGIN_MAPID			= 1000;		// ��¼��
const int	DEFAULT_LOGIN_POSX			= 44;		// ��¼��
const int	DEFAULT_LOGIN_POSY			= 44;		// ��¼��
const int	DEFAULT_LOGIN_DIR			= 0;
const int	DEFAULT_LOGIN_STATUS		= 0;		// STATUS_NORMAL
const int	DEFAULT_LOGIN_POSE			= 0;
const int	DEFAULT_LOGIN_EMOTION		= 0;

const int	CELLS_PER_BLOCK				= 18;		// ÿROOM�ĸ�����
const int	CELLS_PER_VIEW				= 18;		// ��������ĸ�����
const int	BLOCKWIDTH_PER_MAP			= 100;		// ��ͼX����ROOM���������˹��󣬿պ��ڴ�

const int	DEFAULT_MAP_WIDTH			= 50;		// ÿ�ŵ�ͼ��ROOM��
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

#define NOMATE_NAME		"��"						// ����ż����ż��

#define	SYNNAME_NONE	"��"						// �ް�����ҵİ�����
#define	SYNLEADER_TITLE	"����"						// ������TITLE
#define	SYNMEMBER_TITLE	"��ͨ����"						// �������ҵ�TITLE
#define	NOSYN_TITLE		"��"						// �ް�����ҵ�TITLE

// ��������
const int	STATESTR_SIZE		= 2048;


/*class CNetMsg;
class ISocket
{
public:
	virtual bool SendMsg			(CNetMsg* pNetMsg)				PURE_VIRTUAL_FUNCTION_0
	virtual bool SendMsg			(SOCKET_ID idSocket, OBJID idMsg, const char* pBuf, int nMsgLen)	PURE_VIRTUAL_FUNCTION_0
	virtual bool SendNpcMsg			(OBJID idNpc, OBJID idMsg, const char* pBuf, int nMsgLen)			PURE_VIRTUAL_FUNCTION_0
	// ����֪ͨSOCKET�ر�
	virtual bool CloseSocket		(SOCKET_ID idSocket)			PURE_VIRTUAL_FUNCTION_0
};
*/
typedef	void			CGameSocket;		// �����ڻ������Ϣ����

// ���߷���
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