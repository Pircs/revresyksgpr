#include "MessagePort.h"
#include "allmsg.h"
#include "protocol.h"
#include "MapList.h"
#include "WorldKernel.h"
//using namespace world_kernel;

bool CMsgAiNpcInfo::Create(ST_CREATENEWNPC* pInfo)
{
	CHECKF(pInfo->usAction == MSGAINPCINFO_CREATENEW);

	m_id			= pInfo->id;
	m_usAction		= pInfo->usAction;
	m_usType		= pInfo->usType;
	m_idMap			= pInfo->idMap;
	m_usPosX		= pInfo->usPosX;
	m_usPosY		= pInfo->usPosY;
	m_idGen			= pInfo->idData;
	m_idOwner		= pInfo->idOwner;
	m_ucOwnerType	= pInfo->ucOwnerType;
	m_nData			= pInfo->nData;
	return true;
}

void CMsgAiNpcInfo::Process()
{
#ifdef _MYDEBUG
	::LogSave("Process CMsgAiNpcInfo: ID:0x:%x, Type:%d, LookType:%d, CellX:%d, CellY:%d, Name:%s",
				m_id	, m_usType,
				m_usLook, m_usCellX, 
				m_usCellY, m_szName);
#endif
	OBJID idMap				= m_idMap;
	PROCESS_ID idProcess	= MapList()->GetMapProcessID(idMap);
	if(idProcess == PROCESS_NONE)
		return;

	ST_CREATENEWNPC	buf;
	memset(&buf, 0, sizeof(ST_CREATENEWNPC));
	buf.id			= m_id;
	buf.usAction	= m_usAction;
	buf.usType		= m_usType;
	buf.idMap		= m_idMap;
	buf.usPosX		= m_usPosX;
	buf.usPosY		= m_usPosY;
	buf.idData		= m_idGen;
	buf.idOwner		= m_idOwner;
	buf.ucOwnerType	= m_ucOwnerType;
	buf.nData		= m_nData;

	IMessagePort* pMsgPort = GameWorld()->GetMsgPort();
	pMsgPort->Send(idProcess, MAPGROUP_CREATENEWNPC, STRUCT_TYPE(ST_CREATENEWNPC), &buf);
}