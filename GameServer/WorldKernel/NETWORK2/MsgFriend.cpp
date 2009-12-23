#include "AllMsg.h"
#include "WorldKernel.h"
//using namespace world_kernel;
#include "userlist.h"

void CMsgFriend::Process(int nTrans)
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgFriend, id:%u", m_id);
#endif

	CPlayer* pUser = UserList()->GetPlayer(nTrans);
	if (!pUser)
		return ;

	switch(m_ucAction)
	{
	case _FRIEND_ONLINE:
		{
			pUser->SendMsg(this);
		}
		break;
	case _ENEMY_ONLINE:
		{
			pUser->SendMsg(this);
		}
		break;
	}
}
