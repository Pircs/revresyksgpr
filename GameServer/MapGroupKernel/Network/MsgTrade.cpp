#include <windows.h>
#include "AllMsg.h"
#include "User.h"
#include "SquareDeal.h"
#include "MapGroup.h"

void CMsgTrade::Process(CUser* pUser)
{
#ifdef _MYDEBUG
	::LogSave("Process CMsgTrade, id:%u, action:%u", 
		m_id,
		m_usAction);
#endif
//	DEBUG_PROCESSMSG("TRADE",m_id,m_usAction,"",m_nData,0);

	if (!pUser)
		return;

/*	if(pUser->GetLockKey())
	{
		pUser->SendSysMsg("请先打开保护锁。");
		return;
	}

	if (pUser->IsFighting())
	{
		pUser->SendSysMsg("错误：战斗中不能执行交易指令！");
		return;
	}

	if (pUser->GetTeam())
	{
		pUser->SendSysMsg("错误：组队状态不能执行交易指令！");
		return;
	}
*/
	DEBUG_TRY		// VVVVVVVVVVVVVVVVVVVVVVVVVVV
	switch(m_usAction)
	{
	case _TRADE_APPLY:
		{
			if (m_id == ID_NONE)
				return;

			OBJID	idTarget = m_id;
			CUser* pTarget = NULL;
			if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_USER, idTarget, IPP_OF(pTarget)))
			{
				pUser->SendSysMsg(STR_NO_TRADE_TARGET);
				return;
			}

			ISquareDeal* pSquareDeal = pUser->QuerySquareDeal();
			if(pSquareDeal)
			{
				LOGWARNING("玩家重复交易!");
				pSquareDeal->Release();
				return ;
			}

			if(pTarget->QuerySquareDeal())
			{
				pUser->SendSysMsg(STR_TARGET_TRADING);
				return ;
			}

			if (!pTarget->FetchApply(CUser::APPLY_TRADE, pUser->GetID()))
			{
				pUser->SetApply(CUser::APPLY_TRADE, idTarget);
				IF_NOT(this->Create(_TRADE_APPLY, pUser->GetID()))
					return;
				pTarget->SendMsg(this);
				pUser->SendSysMsg(STR_TRADING_REQEST_SENT);
				return;		// ok
			}

			if(!pUser->CreateSquareDeal(pTarget))		// 同时发送消息
			{
				pUser->SendSysMsg(STR_TARGET_TRADING);
				return;
			}
		}
		break;

	case _TRADE_QUIT:
		{
			ISquareDeal* pSquareDeal =	pUser->QuerySquareDeal();
			if(pSquareDeal)
			{
				// inform target
				CUser* pTarget	= pSquareDeal->GetTarget();
				if(pTarget)
				{
//					CHECK(this->Create(_TRADE_FALSE, 0));
//					pTarget->SendMsg(this);
				}

				// feed back
				{
//					CHECK(this->Create(_TRADE_FALSE, 0));
//					pUser->SendMsg(this);
				}

				pSquareDeal->Release();
			}
		}
		break;

	case _TRADE_OPEN:
		{
			ASSERT(!"_TRADE_OPEN");
		}
		break;

	case _TRADE_SUCCESS:
		{
			ASSERT(!"_TRADE_SUCCESS");
		}
		break;

	case _TRADE_FALSE:
		{
			ASSERT(!"_TRADE_FALSE");
		}
		break;

	case _TRADE_ADDITEM:
		{
			ISquareDeal* pSquareDeal =	pUser->QuerySquareDeal();
			if(!pSquareDeal)
				return;

			CItem* pItem	= pUser->GetItem(m_id);
			if (!pItem || !pItem->IsExchangeEnable())
			{
				pUser->SendSysMsg(STR_NOT_FOR_TRADE);

				IF_OK(this->Create(_TRADE_ADDITEMFAIL, m_id))
					pUser->SendMsg(this);
				return;
			}
			if (pItem->IsEudemon())
			{
				pUser->CallBackEudemon(pItem->GetID());
				pUser->DetachEudemon(pItem);
			}

			if(!pSquareDeal->AddItem(pItem))
			{
				IF_OK(this->Create(_TRADE_ADDITEMFAIL, m_id))
					pUser->SendMsg(this);
				return;
			}
		}
		break;

	case _TRADE_ADDMONEY:
		{
			ISquareDeal* pSquareDeal =	pUser->QuerySquareDeal();
			if(!pSquareDeal)
				return;

			int	nAllMoney = pSquareDeal->AddMoney(m_nData);		// return allmoney
			CHECK(this->Create(_TRADE_SELFMONEYALL, nAllMoney));
			pUser->SendMsg(this);

			if(nAllMoney <= 0)
				pUser->AddAttrib(_USERATTRIB_MONEY, 0, SYNCHRO_TRUE);		// recruit money(note: because set money once, so no conflict
		}
		break;

	case _TRADE_OK:
		{
			ISquareDeal* pSquareDeal =	pUser->QuerySquareDeal();
			if(!pSquareDeal)
				return;

			if(pSquareDeal->ClickOK())		// return true: 交易完成
				pSquareDeal->Release();
		}
		break;

	default:
		break;
	}
	DEBUG_CATCH2("switch(MSGTRADE) [%d]", m_usAction)		// AAAAAAAAAAAAAAAA
}