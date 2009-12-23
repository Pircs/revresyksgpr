#include "AllMsg.h"
#include "MercenaryTask.h"
#include "MapGroup.h"
#include "UserManager.h"
#include "User.h"
#pragma	warning(disable:4786)

bool CMsgPlayerTask::Create(CMercenaryTaskData* pData)
{
	CHECKF (pData);

	m_dwMoney		= pData->GetInt(MTASKDATA_PRIZE_MONEY);
	m_idTask			= pData->GetID();
	m_dwDurability	= pData->GetInt(MTASKDATA_DURA);
	m_ucAddition		= pData->GetInt(MTASKDATA_ADDITION);
	m_ucDegree		= pData->GetInt(MTASKDATA_DEGREE);
	m_ucType			= pData->GetInt(MTASKDATA_TYPE);
	
	int nIdx = 0;
	for (int i=MTASKDATA_PRIZE_ITEMID0; i<=MTASKDATA_PRIZE_ITEMID4; i++)
	{
		OBJID idItem = pData->GetInt((MTASKDATA)i);
		CItem* pItem = pData->GetItem(idItem);
		if (pItem)
		{
			CHECKF (nIdx < _MAX_PRIZE);
			m_infoItem[nIdx].idItemType		= pItem->GetInt(ITEMDATA_TYPE);
			m_infoItem[nIdx].usDurability	= pItem->GetInt(ITEMDATA_AMOUNT);

			if (pItem->IsEudemon() || pItem->IsEudemonEgg())
			{
				m_infoItem[nIdx].ucAddition		= pItem->GetInt(ITEMDATA_GROWTH);
				m_infoItem[nIdx].ucDegree		= pItem->GetEudemonLevel();
			}
			else
			{
				m_infoItem[nIdx].ucAddition		= pItem->GetInt(ITEMDATA_ADDITION);
				m_infoItem[nIdx].ucDegree		= pItem->GetInt(ITEMDATA_LEVEL);
			}
			++nIdx;
		}
	}
	m_StrPacker.AddString(pData->GetStr(MTASKDATA_TITLE));
	m_StrPacker.AddString(pData->GetStr(MTASKDATA_DETAIL));
	m_StrPacker.AddString(pData->GetStr(MTASKDATA_TARGET_NAME));
	UPDATE_MSG_SIZE;
	return true;
}

void CMsgPlayerTask::Process(CMapGroup& mapGroup, CUser* pUser)
{
	//CUser* pUser = mapGroup.GetUserManager()->GetUser(this);
	if (!pUser)
		return;
	
	if (!pUser->IsAlive())
	{
		pUser->SendSysMsg(STR_DIE);
		return ;
	}
	
	if (mapGroup.GetMercenaryTask()->QueryTaskDataByOwner(pUser->GetID()))
	{
		pUser->SendSysMsg(STR_PUBLISH_ONE_TASK_ONLY);
		return;
	}

	if (m_StrPacker.GetCount() < 3)
		return;

	// 奖金检查
	if (pUser->GetMoney() < m_dwMoney)
	{
		pUser->SendSysMsg(STR_NOT_SO_MUCH_MONEY);
		return;
	}
	// 奖品检查
	int i;
	for (i=0; i<_MAX_PRIZE; i++)
	{
		if (m_infoItem[i].idItem != ID_NONE)
		{
			CItem* pItem = pUser->GetItem(m_infoItem[i].idItem);
			if (!pItem)
			{
				pUser->SendSysMsg(STR_ITEM_INEXIST);
				return;
			}
			if (!pItem->IsExchangeEnable())
			{
				pUser->SendSysMsg(STR_NOT_FOR_TASK_PRIZE);
				return;
			}
		}
	}

	// 先取出奖品，防止作弊
	for (i=0; i<_MAX_PRIZE; i++)
	{
		if (m_infoItem[i].idItem != ID_NONE)
		{
			CItem* pItem = pUser->GetItem(m_infoItem[i].idItem);
			if (!pItem)
			{
				pUser->SendSysMsg(STR_ERROR);
				// 出错处理...
				return;
			}
			if (pItem->IsEudemon())
			{
				pUser->CallBackEudemon(pItem->GetID());
				pUser->DetachEudemon(pItem);
			}

			// 简单一点，仅仅改变position，不改变物品的owner_id
			pItem->SetInt(ITEMDATA_POSITION, ITEMPOSITION_PLAYERTASK, true);
			pUser->DelItem(m_infoItem[i].idItem, true);
		}
	}
	if (!pUser->SpendMoney(m_dwMoney, true))
	{
		pUser->SendSysMsg(STR_ERROR);
		return;
	}

	// fill task data
	ST_MTASK_DATA	tTaskData;
	memset(&tTaskData, 0L, sizeof(ST_MTASK_DATA));

	tTaskData.idUser	= pUser->GetID();

	::SafeCopy(tTaskData.szUserName, pUser->GetName(), _MAX_NAMESIZE);
	m_StrPacker.GetString(0, tTaskData.szTitle, 32);
	m_StrPacker.GetString(1, tTaskData.szDetail, 128);
	m_StrPacker.GetString(2, tTaskData.szTargetName, _MAX_NAMESIZE);

	tTaskData.ucType		= m_ucType;
	tTaskData.ucAddition	= m_ucAddition;
	tTaskData.ucDegree		= m_ucDegree;
	tTaskData.dwDura		= m_dwDurability;
	tTaskData.ucRankReq		= m_ucRankReq;
	
	tTaskData.dwPrizeMoney	= m_dwMoney;
	for (i=0; i<_MAX_PRIZE; i++)
		tTaskData.idPrizeItems[i]	= m_infoItem[i].idItem;

	// create new mercenary task
	if (!mapGroup.GetMercenaryTask()->CreateNewTask(&tTaskData))
	{
		pUser->SendSysMsg(STR_ERROR);
		return;
	}

	pUser->SendSysMsg(STR_MERCENARY_TASK_OK);
}