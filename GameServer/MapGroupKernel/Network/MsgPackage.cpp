#include "AllMsg.h"
#include "package.h"
#include "MapGroup.h"
#include "Npc.h"

bool CMsgPackage::Create(int nAction,OBJID id , OBJID idItem, UCHAR ucType)
{
	CHECKF(id != ID_NONE && idItem != ID_NONE);

	m_id			= id;
	m_ucAction	= nAction;
	m_ucType		= ucType;	//MSGPACKAGE_TYPE_NONE;						// ������
	m_idItem		= idItem;

	return true;
}

bool CMsgPackage::Create(int nAction, OBJID id, const MsgPackageItemInfo* buf, int nSize, UCHAR ucType)
{
	CHECKF(buf && nSize >= 0 && nSize <= PACKAGE_LIMIT);

	// fill info now
	//m_unMsgSize		= sizeof(MSG_Info) + (nSize-1) * sizeof(MsgPackageItemInfo);
	//m_unMsgType		= _MSG_PACKAGE;

	m_id			= id;
	m_ucAction	= nAction;
	m_ucType		= ucType;

	for (int i=0; i<nSize; ++i)
	{
		m_setItemInfo.Append(buf[i]);
	}

	return true;
}

void CMsgPackage::Process(CUser* pUser)
{
	DEBUG_TRY	// VVVVVVVVVVVVVVVVVVVVVVVVVV
	//CUser* pUser = UserManager()->GetUser(this);
	CHECK(pUser);

	switch(m_ucType)
	{
	case MSGPACKAGE_TYPE_AUCTION_STORAGE:
		{
			switch(m_ucAction)
			{
			case MSGPACKAGE_QUERYLIST:
				{
				// NPC
				OBJID	idNpc = m_id;
				CNpc* pNpc = NULL;
				if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
					return ;
				if(!pNpc->IsAuctionNpc())
					return ;
                DEBUG_TRY
				pUser->QueryStorage()->SendStorageInfo(idNpc,ITEMPOSITION_AUCTION_STORAGE);
				DEBUG_CATCH("QueryStorage()->SendStorageInfo Error")
			//	pUser->SendStorageInfo(idNpc,ITEMPOSITION_AUCTION_STORAGE);
				}
				break;
			case MSGPACKAGE_CHECKOUT:
				{
				// NPC
				OBJID	idNpc = m_id;
				OBJID	idItem = m_idItem;
				CNpc* pNpc = NULL;
				if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
					return ;
				if(!pNpc->IsAuctionNpc())
					return ;
/*				if(pUser->IsItemFull(pUser->QueryStorage()->GetWeight(idItem),
										pUser->QueryStorage()->GetItemType(idItem),
										pUser->QueryStorage()->GetAmountLimit(idItem)))
					return ;*/

				// ITEM
				DEBUG_TRY	// VVVVVVVVVVVVVV
				CItem* pItem = pUser->QueryStorage()->CheckOut(idNpc, idItem, UPDATE_FALSE,ITEMPOSITION_AUCTION_STORAGE);		// ��ȡ��������
				if(!pItem)
					return ;
				pUser->AddItem(pItem, SYNCHRO_TRUE);
//				pNpc->QueryAuction()->QueryPackage()->CheckOut(idItem);
				DEBUG_CATCH("MSGPACKAGE_AUCTION_CHECKOUT")
				}
				break;
			}
		}
		break;
	case	MSGPACKAGE_TYPE_STORAGE:
		switch(m_ucAction)
		{
		case	MSGPACKAGE_QUERYLIST:
			{
				// NPC
				OBJID	idNpc = m_id;
				CNpc* pNpc = NULL;
				if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
					return ;
				if(!pNpc->IsStorageNpc())
					return ;

				pUser->QueryStorage()->SendStorageInfo(idNpc);
			}
			break;
		case	MSGPACKAGE_CHECKIN:
			{
				// ITEM
				OBJID	idItem = m_idItem;
				CItem* pItem = pUser->GetItem(idItem);
				if(!pItem || !pItem->IsStorageEnable() && !pItem->IsChestItem())	// enable store chest
				{
					pUser->SendSysMsg(STR_CANT_STORAGE);
					return ;
				}

				// NPC
				OBJID	idNpc = m_id;
				CNpc* pNpc = NULL;
				if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
					return ;
				if(!pNpc->IsStorageNpc())
					return ;
				int nSize = pNpc->GetStorageSize();
				if(nSize == 0)
					nSize	= USER_PACKAGE_SIZE;		// �ߴ�
				if(pUser->QueryStorage()->GetStorageCount() >= nSize)
//				if(pUser->QueryStorage()->GetStorageCount() >= pNpc->GetStorageSize())
					return ;

				DEBUG_TRY	// VVVVVVVVVVVVVV
				pUser->PopItem(idItem, SYNCHRO_TRUE, UPDATE_FALSE);				// ��ȡ��������
				pUser->QueryStorage()->CheckIn(idNpc, pItem, UPDATE_TRUE);
				DEBUG_CATCH("MSGPACKAGE_CHECKIN") // AAAAAAAAAAAAAAAAA
			}
			break;
		case	MSGPACKAGE_CHECKOUT:
			{
				// NPC
				OBJID	idNpc = m_id;
				OBJID	idItem = m_idItem;
				CNpc* pNpc = NULL;
				if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
					return ;
				if(!pNpc->IsStorageNpc())
					return ;
				if(pUser->IsItemFull(pUser->QueryStorage()->GetWeight(idItem),
										pUser->QueryStorage()->GetItemType(idItem),
										pUser->QueryStorage()->GetAmountLimit(idItem)))
					return ;

				// ITEM
				DEBUG_TRY	// VVVVVVVVVVVVVV
				CItem* pItem = pUser->QueryStorage()->CheckOut(idNpc, idItem, UPDATE_FALSE);		// ��ȡ��������
				if(!pItem)
					return ;
				pUser->AddItem(pItem, SYNCHRO_TRUE);
				DEBUG_CATCH("MSGPACKAGE_CHECKIN") // AAAAAAAAAAAAAAAAA
			}
			break;
		default:
			ASSERT(!"action");
		}
		break;
	case	MSGPACKAGE_TYPE_TRUNK:
		switch(m_ucAction)
		{
		case	MSGPACKAGE_QUERYLIST:
			{
				// NPC
				OBJID	idNpc = m_id;
				CNpc* pNpc = NULL;
				if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
					return ;
				if(!pNpc->IsTrunkNpc())
					return ;

				pNpc->QueryTrunk()->QueryPackage()->SendInfo(pUser);
			}
			break;
		case	MSGPACKAGE_CHECKIN:
			{
				// ITEM
				OBJID	idItem = m_idItem;
				CItem* pItem = pUser->GetItem(idItem);
				if(!pItem || !pItem->IsExchangeEnable() || !pItem->IsStorageEnable())
				{
					pUser->SendSysMsg(STR_CANT_STORAGE);
					return ;
				}
				if (pItem->IsEudemon())
				{
					pUser->CallBackEudemon(pItem->GetID());
					pUser->DetachEudemon(pItem);
				}

				// NPC
				OBJID	idNpc = m_id;
				CNpc* pNpc = NULL;
				if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
					return ;
				if(!pNpc->IsTrunkNpc())
					return ;
				if(pNpc->QueryTrunk()->IsPackageFull())
					return ;

				DEBUG_TRY	// VVVVVVVVVVVVVV
				pUser->PopItem(idItem, SYNCHRO_TRUE, UPDATE_FALSE);				// ��ȡ��������
				pNpc->QueryTrunk()->QueryPackage()->CheckIn(pUser, pItem, UPDATE_TRUE);
				DEBUG_CATCH("MSGPACKAGE_CHECKIN") // AAAAAAAAAAAAAAAAA

				if(pNpc->IsShelfNpc())
					pNpc->SendSelfToBlock();
			}
			break;
		case	MSGPACKAGE_CHECKOUT:
			{
				// NPC
				OBJID	idNpc = m_id;
				OBJID	idItem = m_idItem;
				CNpc* pNpc = NULL;
				if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
					return ;
				if(!pNpc->IsTrunkNpc())
					return ;
				if(!pNpc->IsOwnerOf(pUser, CNpc::MATE_ENABLE))
					return ;
				if(pUser->IsItemFull(pNpc->QueryTrunk()->QueryPackage()->GetWeight(idItem),
										pNpc->QueryTrunk()->QueryPackage()->GetItemTypeByID(idItem),
										pNpc->QueryTrunk()->QueryPackage()->GetItemAmountLimitByID(idItem)))
					return ;

				// ITEM
				DEBUG_TRY	// VVVVVVVVVVVVVV
				CItem* pItem = pNpc->QueryTrunk()->QueryPackage()->CheckOut(pUser, idItem, UPDATE_FALSE);		// ��ȡ��������
				if(!pItem)
					return ;
				pUser->AddItem(pItem, SYNCHRO_TRUE);

				// log
				if (pItem->IsNonsuchItem())
				{
					::MyLogSave("gmlog/Trunk", "%s(%u) checkout item:[id=%u, type=%u], dur=%d, max_dur=%d", 
							pUser->GetName(),
							pUser->GetID(),
							pItem->GetID(), 
							pItem->GetInt(ITEMDATA_TYPE),
							pItem->GetInt(ITEMDATA_AMOUNT),
							pItem->GetInt(ITEMDATA_AMOUNTLIMIT));
				}
				DEBUG_CATCH("MSGPACKAGE_CHECKOUT") // AAAAAAAAAAAAAAAAA

				if(pNpc->IsShelfNpc())
					pNpc->SendSelfToBlock();
			}
			break;
		default:
			ASSERT(!"action");
		}
		break;
	case	MSGPACKAGE_TYPE_CHEST:
		// ���Ԫ��
		// add by zlong 2003-11-24
		switch(m_ucAction)
		{
		case	MSGPACKAGE_QUERYLIST:
			{
				// CHEST
				OBJID	idChest	= m_id;
				CItem*	pChestItem	= pUser->GetItem(idChest);
				if (!pChestItem || !pChestItem->IsChestItem())
					return ;
				pUser->QueryStorage()->SendStorageInfo(idChest, ITEMPOSITION_CHEST);
			}
			break;
		case	MSGPACKAGE_CHECKIN:
			{
				// ITEM
				OBJID	idItem = m_idItem;
				CItem* pItem = pUser->GetItem(idItem);
				//������Ƕ�����Ԫ�����������Ų��ɽ����򲻿ɴ洢����Ʒ
				if(!pItem || !pItem->IsExchangeEnable() || !pItem->IsStorageEnable() || pItem->IsChestItem())
				{
					pUser->SendSysMsg(STR_CANT_STORAGE);
					return ;
				}
				if (pItem->IsEudemon())
				{
					pUser->CallBackEudemon(pItem->GetID());
					pUser->DetachEudemon(pItem);
				}

				// CHEST
				OBJID	idChest = m_id;
				CItem*	pChestItem	= pUser->GetItem(idChest);
				if (!pChestItem || !pChestItem->IsChestItem())
					return ;

				//���Ԫ������������Ʒtype_id�󶨣����� = (idType % 10 + 1) * 3
				OBJID idType	= pChestItem->GetInt(ITEMDATA_TYPE);
				if(pUser->QueryStorage()->GetStorageCount() >= (idType % 10 +1) * 3)
					return ;

				DEBUG_TRY	// VVVVVVVVVVVVVV
				pUser->PopItem(idItem, SYNCHRO_TRUE, UPDATE_FALSE);				// ��ȡ��������
				pUser->QueryStorage()->CheckIn(idChest, pItem, UPDATE_TRUE, ITEMPOSITION_CHEST);
				DEBUG_CATCH("MSGPACKAGE_CHECKIN") // AAAAAAAAAAAAAAAAA
			}
			break;
		case	MSGPACKAGE_CHECKOUT:
			{
				// CHEST
				OBJID	idChest = m_id;
				OBJID	idItem = m_idItem;
				CItem*	pChestItem	= pUser->GetItem(idChest);
				if (!pChestItem || !pChestItem->IsChestItem())
					return ;
				if(pUser->IsItemFull(pUser->QueryStorage()->GetWeight(idItem),
										pUser->QueryStorage()->GetItemType(idItem),
										pUser->QueryStorage()->GetAmountLimit(idItem)))
					return ;

				// ITEM
				DEBUG_TRY	// VVVVVVVVVVVVVV
				CItem* pItem = pUser->QueryStorage()->CheckOut(idChest, idItem, UPDATE_FALSE, ITEMPOSITION_CHEST);		// ��ȡ��������
				if(!pItem)
					return ;
				pUser->AddItem(pItem, SYNCHRO_TRUE);
				// log
				if (pItem->IsNonsuchItem())
				{
					::MyLogSave("gmlog/Chest", "%s(%u) checkout item:[id=%u, type=%u], dur=%d, max_dur=%d", 
							pUser->GetName(),
							pUser->GetID(),
							pItem->GetID(), 
							pItem->GetInt(ITEMDATA_TYPE),
							pItem->GetInt(ITEMDATA_AMOUNT),
							pItem->GetInt(ITEMDATA_AMOUNTLIMIT));
				}
				DEBUG_CATCH("MSGPACKAGE_CHECKIN") // AAAAAAAAAAAAAAAAA
			}
			break;
		default:
			ASSERT(!"action");
		}
		// GetAmount()
		break;
	// ���޷�����
	case MSGPACKAGE_TYPE_EUDEMON_BROODER:
		{
			switch (m_ucAction)
			{
			case MSGPACKAGE_QUERYLIST:
				{
					// NPC
					OBJID	idNpc = m_id;
					CNpc* pNpc = NULL;
					if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
						return ;
					if(!pNpc->IsEudemonBrooderNpc())
						return ;

					CPackage* pPackage = pNpc->QueryEudemonBrooder()->QueryPackage(pUser->GetID());
					if (!pPackage)
						return;
					// ���޵��ķ���Ӧ����SendInfo֮ǰ���У�
					// ��������Ķ�CPackage�ģ���ʱ���ȸĶ��ɡ�
					pPackage->SendInfo(pUser);
				}
				break;
			case MSGPACKAGE_CHECKIN:
				{
					// ITEM
					OBJID	idItem = m_idItem;
					CItem* pItem = pUser->GetItem(idItem);
					if(!pItem || !pItem->IsStorageEnable() || !pItem->IsEudemonEgg())	// ֻ�л��޵��ſ��ԷŽ���
					{
						pUser->SendSysMsg(STR_CANT_STORAGE);
						return ;
					}
					pUser->CallBackEudemon(pItem->GetID());
					pUser->DetachEudemon(pItem);

					// NPC
					OBJID	idNpc = m_id;
					CNpc* pNpc = NULL;
					if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
						return ;
					if(!pNpc->IsEudemonBrooderNpc())
						return ;
					if(pNpc->QueryEudemonBrooder()->IsPackageFull(pUser->GetID()))
						return ;
					CPackage* pPackage = pNpc->QueryEudemonBrooder()->QueryPackage(pUser->GetID());
					if (!pPackage)
						return;

					// ����availabletime�ֶμ�¼���޵���ʼ������ʱ��
					pItem->SetInt(ITEMDATA_AVAILABLETIME, ::TimeGet(TIME_SECOND), true);
					
					DEBUG_TRY	// VVVVVVVVVVVVVV
					pUser->PopItem(idItem, SYNCHRO_TRUE, UPDATE_FALSE);				// ��ȡ��������
					pPackage->CheckIn(pUser, pItem, UPDATE_TRUE);
					DEBUG_CATCH("MSGPACKAGE_CHECKIN") // AAAAAAAAAAAAAAAAA
				}
				break;
			case MSGPACKAGE_CHECKOUT:
				{
					// NPC
					OBJID	idNpc = m_id;
					OBJID	idItem = m_idItem;
					CNpc* pNpc = NULL;
					if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
						return ;
					if(!pNpc->IsEudemonBrooderNpc())
						return ;
					CPackage* pPackage = pNpc->QueryEudemonBrooder()->QueryPackage(pUser->GetID());
					if (!pPackage)
						return;
					if(pUser->IsItemFull(pPackage->GetWeight(idItem),
											pPackage->GetItemTypeByID(idItem),
											pPackage->GetItemAmountLimitByID(idItem)))
						return ;

					// ITEM
					DEBUG_TRY	// VVVVVVVVVVVVVV
					CItem* pItem = pPackage->CheckOut(pUser, idItem, UPDATE_FALSE);		// ��ȡ��������
					if(!pItem)
						return ;

					// ������޵��Ŀ�ʼ����ʱ���¼
					pItem->SetInt(ITEMDATA_AVAILABLETIME, 0, true);
					pUser->AddItem(pItem, SYNCHRO_TRUE);

					// log
					if (pItem->IsNonsuchItem())
					{
						::MyLogSave("gmlog/Eudemon", "%s(%u) checkout item:[id=%u, type=%u], dur=%d, max_dur=%d", 
								pUser->GetName(),
								pUser->GetID(),
								pItem->GetID(), 
								pItem->GetInt(ITEMDATA_TYPE),
								pItem->GetInt(ITEMDATA_AMOUNT),
								pItem->GetInt(ITEMDATA_AMOUNTLIMIT));
					}
					DEBUG_CATCH("MSGPACKAGE_CHECKOUT") // AAAAAAAAAAAAAAAAA
				}
				break;
			default:
				ASSERT(!"action");
			}
		}
		break;
	// ���޲ֿ�
	case MSGPACKAGE_TYPE_EUDEMON_STORAGE:
		{
			switch (m_ucAction)
			{
			case MSGPACKAGE_QUERYLIST:
				{
					// NPC
					OBJID	idNpc = m_id;
					CNpc* pNpc = NULL;
					if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
						return ;
					if(!pNpc->IsEudemonStorageNpc())
						return ;
					CPackage* pPackage = pNpc->QueryEudemonStorage()->QueryPackage(pUser->GetID());
					if (!pPackage)
						return;

					pPackage->SendInfo(pUser);
				}
				break;
			case MSGPACKAGE_CHECKIN:
				{
					// ITEM
					OBJID	idItem = m_idItem;
					CItem* pItem = pUser->GetItem(idItem);
					if(!pItem || !pItem->IsStorageEnable() 
						|| !(pItem->IsEudemon() || pItem->IsEudemonEgg()))	// ֻ�л��޺ͻ��޵�������Ž���
					{
						pUser->SendSysMsg(STR_CANT_STORAGE);
						return ;
					}
					pUser->CallBackEudemon(pItem->GetID());
					pUser->DetachEudemon(pItem);

					// NPC
					OBJID	idNpc = m_id;
					CNpc* pNpc = NULL;
					if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
						return ;
					if(!pNpc->IsEudemonStorageNpc())
						return ;
					if(pNpc->QueryEudemonStorage()->IsPackageFull(pUser->GetID()))
						return ;
					CPackage* pPackage = pNpc->QueryEudemonStorage()->QueryPackage(pUser->GetID());
					if (!pPackage)
						return;

					DEBUG_TRY	// VVVVVVVVVVVVVV
					pUser->PopItem(idItem, SYNCHRO_TRUE, UPDATE_FALSE);				// ��ȡ��������
					pPackage->CheckIn(pUser, pItem, UPDATE_TRUE);
					DEBUG_CATCH("MSGPACKAGE_CHECKIN") // AAAAAAAAAAAAAAAAA

					if(pNpc->IsShelfNpc())
						pNpc->SendSelfToBlock();
				}
				break;
			case MSGPACKAGE_CHECKOUT:
				{
					// NPC
					OBJID	idNpc = m_id;
					OBJID	idItem = m_idItem;
					CNpc* pNpc = NULL;
					if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
						return ;
					if(!pNpc->IsEudemonStorageNpc())
						return ;
					CPackage* pPackage = pNpc->QueryEudemonStorage()->QueryPackage(pUser->GetID());
					if (!pPackage)
						return;
					if(pUser->IsItemFull(pPackage->GetWeight(idItem),
											pPackage->GetItemTypeByID(idItem),
											pPackage->GetItemAmountLimitByID(idItem)))
						return ;

					// ITEM
					DEBUG_TRY	// VVVVVVVVVVVVVV
					CItem* pItem = pPackage->CheckOut(pUser, idItem, UPDATE_FALSE);		// ��ȡ��������
					if(!pItem)
						return ;
					pUser->AddItem(pItem, SYNCHRO_TRUE);

					// log
					if (pItem->IsNonsuchItem())
					{
						::MyLogSave("gmlog/Eudemon", "%s(%u) checkout item:[id=%u, type=%u], dur=%d, max_dur=%d", 
								pUser->GetName(),
								pUser->GetID(),
								pItem->GetID(), 
								pItem->GetInt(ITEMDATA_TYPE),
								pItem->GetInt(ITEMDATA_AMOUNT),
								pItem->GetInt(ITEMDATA_AMOUNTLIMIT));
					}
					DEBUG_CATCH("MSGPACKAGE_CHECKOUT") // AAAAAAAAAAAAAAAAA

					if(pNpc->IsShelfNpc())
						pNpc->SendSelfToBlock();
				}
				break;
			default:
				ASSERT(!"action");
			}
		}
		break;
	default:
		ASSERT(!"type");
	}
	DEBUG_CATCH3("MsgPackage. type[%d], subtype[%d]", m_ucType, m_ucAction)	// AAAAAAAAAAAAAAA
}
