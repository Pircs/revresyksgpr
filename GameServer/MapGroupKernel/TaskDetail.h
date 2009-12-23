
#pragma once

#pragma warning(disable:4786)
#include "define.h"
#include <vector>
#include "TaskDetailData.h"
class IDatabase;
class IRole;
class CUser;
class CTaskDetail  
{
public:
	CTaskDetail();
	virtual ~CTaskDetail();

	static CTaskDetail* CreateNew(){return new CTaskDetail;}
	bool	CreateNewTaskDetailData(ST_TASKDETAILDATA * pInfo);
public:
	bool	Create(PROCESS_ID idProcess,CUser *pUser);
	bool	Release()	{delete this; return true;}
public:
	CTaskDetailData* QueryData(OBJID idData);
	CTaskDetailData* QueryData(OBJID idUser,OBJID idTask);
	bool	FindNextData(int &nIter);
	bool	FindData(OBJID  idTask,int nPhase,int nCompleteNum, int &nIter);
	bool	FindNextData(OBJID idUser,int& nIter);
	bool	DeleteData(OBJID idUser,OBJID idTask);

protected:
	typedef std::vector<CTaskDetailData*>	TASKDETAILDATA_SET;
	TASKDETAILDATA_SET	m_setData;
	PROCESS_ID		m_idProcess;

	MYHEAP_DECLARATION(s_heap)
};


