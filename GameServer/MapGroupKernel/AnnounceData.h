
#pragma once

#pragma warning(disable:4786)
#include "define.h"
#include "GameData.h"
#include "GameObj.h"
#include "MyHeap.h"
//#include "MsgPlayerTask.h"
#include <vector>


enum ANNOUNCEDATA {
	ANNOUNCEDATA_ID	= 0,	     // ����ID
	ANNOUNCEDATA_USER_ID,		 // ����������ID			
					
	ANNOUNCEDATA_USER_NAME,	     // ��������������
	ANNOUNCEDATA_LEVEL,          // ���������˵ĵȼ�
	ANNOUNCEDATA_TEACHER_LEVEL,	 // ��ʦ�ȼ�
	ANNOUNCEDATA_PROFESSION,	 // ��ʦְҵ
	ANNOUNCEDATA_TITLE,			 // �������
	ANNOUNCEDATA_CONTENT,		 // ��������
};

struct ST_ANNOUNCE_DATA
{
	int id;
	OBJID user_id;
	char name[16];
	int level;
	int teacherlevel;
	int profession;
	//UCHAR profession;
	char title[32];
	char content[128];
};

char	szAnnounceTable[];
typedef CGameData<ANNOUNCEDATA, szAnnounceTable, szID>		CGameMAnnounceData;


class IDatabase;
class CItem;
class CAnnounceData  : public CGameObj 
{
public:
	CAnnounceData();
	virtual ~CAnnounceData();
	static CAnnounceData* CreateNew()		{ return new CAnnounceData; }
	ULONG	Release()				{ delete this; return 0; }
	ULONG	ReleaseByOwner()		{ delete this; return 0; }
public:
	bool Create	(OBJID idTask, IDatabase* pDb);
	bool Create	(IRecordset* pRes, IDatabase* pDb);

	bool	CreateRecord(const ST_ANNOUNCE_DATA* pInfo, IDatabase* pDb);
	bool	DeleteRecord();
public:// get&set
	OBJID	GetID()			{ return m_pData->GetKey(); }

	int		GetInt(ANNOUNCEDATA idx)		{ return m_pData->GetInt(idx); }
	LPCTSTR	GetStr(ANNOUNCEDATA idx)		{ return m_pData->GetStr(idx); }
	void	SetInt(ANNOUNCEDATA idx, int nData, bool bUpdate = false)					{ m_pData->SetInt(idx, nData); if (bUpdate) m_pData->Update(); }
	void	SetStr(ANNOUNCEDATA idx, LPCTSTR szData, int nSize, bool bUpdate = false)	{ m_pData->SetStr(idx, szData, nSize); if (bUpdate) m_pData->Update(); }
protected:
	CGameMAnnounceData*	m_pData;
	MYHEAP_DECLARATION(s_heap)
};


