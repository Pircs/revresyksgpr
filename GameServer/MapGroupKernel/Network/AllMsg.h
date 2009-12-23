#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "Msg.h"
#include "define.h"
#include "myheap.h"

#include "MapGroup.h"

class CReceiveMsg
{
public:
	virtual void Process(PROCESS_ID idProcess, SOCKET_ID idSocket, OBJID idNpc) = 0;
};

////
#define CLASS_SERVER_MSG_BAGIN(_name) class C##_name:public _name{\
public:void Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc);
#define CLASS_SERVER_MSG_END	};
#define CLASS_SERVER_MSG(_name) class C##_name:public _name{\
public:void Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc);};

#define CLASS_SERVER_MSG_1(_name) class C##_name:public _name{\
public:void Process(CUser* pUser);};

CLASS_SERVER_MSG_1(MsgTick);
CLASS_SERVER_MSG_1(MsgTeam);
CLASS_SERVER_MSG_1(MsgDialog);
CLASS_SERVER_MSG_1(MsgBODice);
CLASS_SERVER_MSG_1(MsgGemEmbed);
CLASS_SERVER_MSG_1(MsgItem);//#include "ItemData.h"bool Create(OBJID id, int nAction, int nPosition = ITEMPOSITION_NONE);
CLASS_SERVER_MSG_1(MsgTrade);

CLASS_SERVER_MSG(MsgAllot);
//#include "User.h"
CLASS_SERVER_MSG(MsgWalk);
CLASS_SERVER_MSG(MsgMessageBoard);
CLASS_SERVER_MSG(MsgFriend);
CLASS_SERVER_MSG(MsgTalk);

CLASS_SERVER_MSG(MsgAction);

#include "protocol.h"
enum {
	MSGAINPCINFO_CREATENEW		= 1,		// NPCSERVER上传：要求创建并返回。NPCSERVER下传：添加
};

class CMsgAiNpcInfo : public MsgAiNpcInfo
{
public:
	bool Create(const ST_CREATENEWNPC* pInfo);
};

CLASS_SERVER_MSG(MsgName)

#include "User.h"
class CMsgTeamMember : public MsgTeamMember
{
public:
	bool Create(CUser* pMember);
};

CLASS_SERVER_MSG_BAGIN(MsgSchoolMember)
bool Create(uint8 ucAction, ST_MEMBERINFO* pMember, uint8 ucAmount);
bool Append(const ST_MEMBERINFO& Member);
bool Append(uint8 ucRelation, uint8 ucStatus, OBJID idMember, const char* pszName = NULL);
bool Append(uint8 ucRelation, uint8 ucStatus, CUser* pUser);
CLASS_SERVER_MSG_END


class CMsgNpc :public MsgNpc 
{
public:
	void Process(CMapGroup& mapGroup, CUser* pUser);
};

class CNpc;
class CMsgNpcInfo :public MsgNpcInfo 
{
public:
	bool Create(CNpc* pNpc);
	bool Create(OBJID id, int nType, int nSort, int nLookType, int nCellX, int nCellY,
		int nLength, int nFat, const char* pszName=NULL);
	void Process(CMapGroup& mapGroup, CUser* pUser);
};

CLASS_SERVER_MSG_BAGIN(MsgNpcInfoEx)
bool Create(CNpc* pNpc);
bool Create(OBJID id, DWORD dwMaxLife, DWORD dwLife, int nType, int nSort,
			int nLookType, int nCellX, int nCellY, int nLength, int nFat, const char* pszName=NULL);
CLASS_SERVER_MSG_END

class CAnnounceData;
class CMsgAnnounceInfo :public MsgAnnounceInfo 
{
public:
	bool Create(CAnnounceData *pData,int type);
	bool Create(int level,int teacher_level,const char* name,int profession,int type);
	void Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc);
};

class CMsgAnnounceList :public MsgAnnounceList  
{
public:
	bool	Create(USHORT usType, USHORT usAmount = 0, ST_ANNOUNCE_TITLE* setAnnounce = NULL);
	bool	Append(OBJID idAnnounce,  const char* pszTitle,int type = 1);
	void	Process	(CMapGroup& mapGroup, CUser* pUser);
};

class CMsgAuction : public MsgAuction
{
public:
	bool Create(/*OBJID idUser*/char* szName ,OBJID idItem,DWORD dwValue,int nAction);
	bool Create(OBJID dwData,int dwValue,int nAction);
	void Process(CMapGroup& mapGroup, CUser* pUser);
};

class SynAttrInfoStruct;
class CSyndicate;
class CMsgSynAttrInfo : public MsgSynAttrInfo
{
public:
	bool Create(SynAttrInfoStruct* pInfo, CSyndicate* pSyn);
};

class CSyndicate;
class CMsgSynInfo : public MsgSynInfo
{
public:
	bool Create	(CSyndicate* pSyn);
};


class CMsgSynMemberInfo : public MsgSynMemberInfo
{
public:
	bool Create	(class CUser* pUser);
	void Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc);
};

class IRole;
class CMsgPlayer : public MsgPlayer
{
public:
	//	bool			Create		(CBooth* pBooth);
	bool Create(IRole* pRole);
	void Process(CUser* pUser);
};

class CMsgUserInfo : public MsgUserInfo
{
public:
	bool Create(CUser* pUser);
};

class CMsgMagicEffect : public MsgMagicEffect
{
public:
	bool	Create(OBJID idUser, int nType, int nLevel, OBJID idTarget, UCHAR ucDir);
	bool	Create(OBJID idUser, int nType, int nLevel, OBJID idTarget, DWORD dwData, UCHAR ucDir);
	bool	CreateByPos(OBJID idUser, int nType, int nLevel, int x, int y, UCHAR ucDir, OBJID idTarget=ID_NONE, DWORD dwPower=0);		// 成片攻击都传坐标
	bool	CreateCollide(OBJID idUser, int nType, int nLevel, OBJID idTarget, DWORD dwData, int nCollideDir, UCHAR ucDir);
	bool	AppendRole(OBJID idRole, DWORD dwData);
};

class CMapItem;
class CMapTrap;
class CMsgMapItem : public MsgMapItem
{
public:
	bool Create(int nAction, CMapItem* pMapItem);
	bool Create(int nAction, CMapTrap* pTrap);
	//	bool Create(int nAction, OBJID idUser);
	bool Create(OBJID id, int nPosX, int nPosY, int nAction=MSGMAPITEM_PICK);
	void Process(CMapGroup& mapGroup, CUser* pUser, SOCKET_ID idSocket);
};

class CItem;
#include "Auction.h"
class CMsgItemInfo : public MsgItemInfo
{
public:
	bool Create(CItem* pItem, int nAction = ITEMINFO_ADDITEM, OBJID idUser=ID_NONE);
	bool Create(CGameAuctionSystemData* pData, int nAction);	
};

class CItem;
class CMsgItemInfoEx : public MsgItemInfoEx 
{
public:
	bool Create(CItem* pItem, OBJID idOwner, int nCost, int nAction = ITEMINFOEX_BOOTH);
};

class CMercenaryTaskData;
class CMsgPlayerTask : public MsgPlayerTask
{
public:
	bool Create(CMercenaryTaskData* pData);
	void Process(CMapGroup& mapGroup, CUser* pUser);
};

class CMsgPackage : public MsgPackage
{
public:
	// modified by zlong 2003-11-24 ---- Create函数添加ucType参数指定类别
	bool Create(int nAction, OBJID id, const MsgPackageItemInfo* buf, int nSize, UCHAR ucType);
	bool Create(int nAction, OBJID id, OBJID idItem, UCHAR ucType);
	void Process(CUser* pUser);
};

class CMsgSyndicate : public MsgSyndicate
{
public:
	bool Create(unsigned short usAction, OBJID idTarget, OBJID idFealty=ID_NONE);
	void Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc);
};

class CMsgInteract : public MsgInteract
{
public:
	bool Create(USHORT unType, OBJID idSender, OBJID idTarget, USHORT unPosX, USHORT unPosY, USHORT usMagicType, USHORT usMagicLev);
	void Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc);
};

class CMsgDataArray : public MsgDataArray
{
protected:
	int RateSuccForQuality(CItem* pEquipItem);
	int RateSuccForGhostLevel(CItem* pEquipItem);
	void SetSynDress(CUser* pUser);
public:
	bool EmbedGemToEquip(CUser* pUser);
	bool UpEquipSuperAddition(CUser* pUser);
	bool UpEquipQuality(CUser* pUser);
	int RateSuccForEquipLevel(CItem* pEquipItem);
	bool UpEquipLevel(CUser* pUser);
	void Process(CUser* pUser);
};