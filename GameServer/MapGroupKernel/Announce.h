
#pragma once

#pragma warning(disable:4786)
#include "define.h"
#include <deque>
#include "AnnounceData.h"
class IDatabase;
class IRole;
class CAnnounce  
{
public:
	CAnnounce()  ;
	virtual ~CAnnounce();
public:
	void	Release()						{ delete this; }
	static CAnnounce*	CreateNew()		{ return new CAnnounce; }
    virtual bool Create(IDatabase* pDb);
	virtual bool Create(IDatabase* pDb,OBJID id);

	bool	CreateNewAnnounce(const ST_ANNOUNCE_DATA* pInfo, IDatabase* pDb);
    virtual bool	DeleteAnnounceByObjID(OBJID idAnnounce);
    virtual bool    DeleteAnnounceByUserID(OBJID id, bool isDelete);
	int		GetAnnounceAmount()					{ return m_setData.size(); }
	CAnnounceData*	QueryAnnounceDataByIndex(int nIdx);
	CAnnounceData*	QueryAnnounceDataByID(OBJID idAnnounce);
	CAnnounceData*	QueryAnnounceDataByOwner(OBJID idUser);

public:
	bool	SendAnnounceList(IRole* pRole, UCHAR usType, int nIndexBegin);
	bool	SendAnnounceInfo(IRole* pRole, OBJID idAnnounce);
	bool    SendAnnounceSelf(IRole* pRole/*, OBJID idAnnounce*/);
protected:
	typedef std::deque<CAnnounceData*>	ANNOUNCEDATA_SET;
	ANNOUNCEDATA_SET	m_setData;
//	PROCESS_ID	m_idProcess;
};





