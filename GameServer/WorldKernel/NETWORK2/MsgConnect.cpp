#include "AllMsg.h"
#include "UserList.h"
#include "logfile.h"
//using namespace world_kernel;
#include "WorldKernel.h"

void CMsgConnect::Process(SOCKET_ID idSocket)
{
#ifdef _MYDEBUG_X
	::LogSave("Process CMsgConnect, idAccount:%u, Data:%d, Info:%s",
				m_idAccount,
				m_dwData,
				m_szInfo);
#endif

	if (m_idAccount == ID_NONE)
		return;

	int nVersion = 0;
	if (1 != sscanf(m_szInfo, "%d", &nVersion))
	{
		CMsgTalk msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, STR_ERROR_VERSION, "", _COLOR_WHITE, _TXTATR_ENTRANCE))
			GameWorld()->SendClientMsg(idSocket, &msg);//SendMsg(&msg);
		return;
	}
#ifdef	PALED_DEBUG
#else
	if (nVersion < GAME_VERSION)
	{
		CMsgTalk msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, STR_ERROR_LOWER_VERSION, "", _COLOR_WHITE, _TXTATR_ENTRANCE))
			GameWorld()->SendClientMsg(idSocket, &msg);//SendMsg(&msg);
		return;
	}
#endif
	extern DWORD g_dwMaxPlayer;
	if(GameWorld()->QueryUserList()->GetUserAmount() > g_dwMaxPlayer && m_idAccount > MAX_GM_ID)
	{
		MsgTalk msg;
		if(msg.Create(SYSTEM_NAME, ALLUSERS_NAME, STR_ERROR_SERVER_FULL, "", _COLOR_WHITE, _TXTATR_ENTRANCE))
			GameWorld()->SendClientMsg(idSocket, &msg);//SendMsg(&msg);
		return;
	}

	// 接收完了MSG_CONNECT后，用认证号重置密码。
	DWORD dwEncyptCode = (m_idAccount+m_uData)^0x4321;
	GameWorld()->ChangeCode(idSocket, m_uData^dwEncyptCode);

	if(GameWorld()->ClientConnect(m_idAccount, m_uData, m_szInfo, idSocket))
	{
		CPlayer* pPlayer	= g_UserList.GetPlayerByAccountID(m_idAccount);
		ASSERT(pPlayer);
		if(pPlayer)
			UserList()->LoginToMapGroup(pPlayer);
	}
}