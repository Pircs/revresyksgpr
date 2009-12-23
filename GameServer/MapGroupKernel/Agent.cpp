#include "windows.h"
#include "define.h"
#include "common.h"
#include "Agent.h"
#include "Mapgroup.h"

MYHEAP_IMPLEMENTATION(CAgent,s_heap)

CAgent::CAgent()
{

}

CAgent::~CAgent()
{

}

bool CAgent::Create(PROCESS_ID idProcess, OBJID idAgent)
{
	return CUser::Create(idProcess, SOCKET_NONE, idAgent);
}

bool CAgent::SendMsg(Msg* pMsg)
{
	return MapGroup(PID)->QueryIntraMsg()->SendNpcMsg(GetID(), pMsg);
}

bool CAgent::FullItem(int nItemType, DWORD dwData)
{
	// ������Ҫ����µı���ϵͳ���޸�!!!

	for(int i = 0; i < _MAX_USERITEMSIZE; i++)
	{
		if(this->IsItemFull(CItem::GetWeight(nItemType), nItemType, dwData))
			return true;
		else
			this->AwardItem(nItemType, SYNCHRO_TRUE, CUser::IDENT_OK);
	}
	return true;
}