#include "AllMsg.h"
#include "MapGroup.h"
#pragma	warning(disable:4786)

void CMsgFriend::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgFriend, id:%u", m_id);
#endif

	CUser* pUser = mapGroup.GetUserManager()->GetUser(idSocket, idNpc);
	if (!pUser)
	{
		if(idSocket == SOCKET_NONE)//IsNpcMsg())
			return;

		// TransmitMsg
		switch(m_ucAction)
		{
		case _FRIEND_GETINFO:
			{
				OBJID idFriend = m_idFriend;
				CUser* pTarget = mapGroup.GetUserManager()->GetUser(idFriend);
				if(pTarget)
				{
					MsgFriend msg;
					IF_OK(Create(_FRIEND_ONLINE, idFriend, pTarget->GetName(), ONLINE_STATUS, pTarget->GetLookFace(), pTarget->GetLev()))
						mapGroup.QueryIntraMsg()->SendClientMsg(idSocket, &msg);
				}
			}
			break;
		case _ENEMY_ADD:
			{
				OBJID idFriend = m_idFriend;
				CUser* pTarget = mapGroup.GetUserManager()->GetUser(idFriend);
				if(pTarget)
				{
					MsgFriend msg;
					IF_OK(Create(_ENEMY_ONLINE, idFriend, m_szName, ONLINE_STATUS, pTarget->GetLookFace(), pTarget->GetLev()))
						mapGroup.QueryIntraMsg()->SendClientMsg(idSocket, &msg);
				}
			}
			break;
		case _FRIEND_BREAK:
			{
				OBJID idFriend = m_idFriend;
				CUser* pTarget = mapGroup.GetUserManager()->GetUser(idFriend);
				if(pTarget)
				{
					pTarget->DelFriend(m_idTransmit);
				}
			}
			break;
		}

		return;
	}

	if (!pUser->IsAlive())
	{
		pUser->SendSysMsg(STR_DIE);
		return;
	}

	switch(m_ucAction)
	{
	case _FRIEND_APPLY:
		{
			CUser* pTarget = NULL;
			if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_USER, m_idFriend, IPP_OF(pTarget)))
				break;

			if(pUser->GetFriendAmount() >= _MAX_USERFRIENDSIZE)
			{
				pUser->SendSysMsg(STR_FRIEND_LIST_FULL);
				break;
			}

			if(pUser->GetFriend(m_idFriend))
			{
				pUser->SendSysMsg(STR_YOUR_FRIEND_ALREADY);
				break;
			}

			if(pTarget->FetchApply(CUser::APPLY_FRIEND) != pUser->GetID())
			{
				// 如果对方未曾向自己发出过加入好友请求
				pUser->SetApply(CUser::APPLY_FRIEND, m_idFriend);
				IF_OK(Create(_FRIEND_APPLY, pUser->GetID(), pUser->GetName(), true))
					pTarget->SendMsg(this);

				pTarget->SendSysMsg(STR_TO_MAKE_FRIEND, pUser->GetName());
				pUser->SendSysMsg(STR_MAKE_FRIEND_SENT);
			}
			else
			{
				// 如果对方已经向自己发出过请求，直接为双方加好友
				if (pUser->AddFriend(m_idFriend, m_szName) &&
						pTarget->AddFriend(pUser->GetID(), pUser->GetName()))
				{
					if(this->Create(_FRIEND_GETINFO, pTarget->GetID(), pTarget->GetName(), ONLINE_STATUS, pTarget->GetLookFace(), pTarget->GetLev()))
						pUser->SendMsg(this);

					if(this->Create(_FRIEND_GETINFO, pUser->GetID(), pUser->GetName(), ONLINE_STATUS, pUser->GetLookFace(), pUser->GetLev()))
						pTarget->SendMsg(this);

					MSGBUF	szMsg;
					sprintf(szMsg, STR_MAKE_FRIEND, pUser->GetName(), pTarget->GetName());
					pUser->BroadcastRoomMsg(szMsg, true);
				}
				else
				{
					::LogSave("Error: add friend failed");
				}
			}
		}
		break;

	case _FRIEND_ONLINE:
		{
			ASSERT(!"_FRIEND_ONLINE");
		}
		break;
	case _FRIEND_OFFLINE:
		{
			ASSERT(!"_FRIEND_OFFLINE");
		}
		break;

	case _FRIEND_BREAK:
		{
			CUser* pTarget = mapGroup.GetUserManager()->GetUser(m_idFriend);		// 可能为NULL

			CFriend* pFriend = pUser->GetFriend(m_idFriend);
			if (!pFriend)
			{
				if(pTarget)
					pUser->SendSysMsg(STR_NOT_YOUR_FRIEND, pTarget->GetName());
				break;
			}

			NAMESTR	szFriend = "";
			SafeCopy(szFriend, pFriend->GetFriendName(), _MAX_NAMESIZE);
			if (pUser->DelFriend(m_idFriend))
			{
				MSGBUF	szMsg;
				sprintf(szMsg, STR_BREAK_FRIEND, pUser->GetName(), szFriend);
				pUser->BroadcastRoomMsg(szMsg, true);

				pUser->SendMsg(this);
			}

			if (pTarget)
			{
				if(pTarget->DelFriend(pUser->GetID()))
				{
					if(this->Create(_FRIEND_BREAK, pUser->GetID(), pUser->GetName()))
						pTarget->SendMsg(this);
				}
			}
			else	// 对方不在线
			{
				SQLBUF	szSQL;
				sprintf(szSQL, "DELETE FROM %s WHERE userid=%u && friend=%u", _TBL_FRIEND, m_idFriend, pUser->GetID());
				mapGroup.GetDatabase()->ExecuteSQL(szSQL);

				m_idTransmit	= pUser->GetID();
				mapGroup.QueryIntraMsg()->TransmitMsg(this, idSocket, idNpc);
			}
		}
		break;

	case _FRIEND_GETINFO:
		{
			ASSERT(!"_FRIEND_GETINFO");
		}
		break;
	case _ENEMY_DEL:
		{
			pUser->QueryEnemy()->Del(m_idFriend, UPDATE_TRUE);
		}
		break;
	}
}
