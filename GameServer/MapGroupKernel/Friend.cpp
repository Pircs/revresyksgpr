#include "AllMsg.h"
#include "Friend.h"
#include "User.h"

MYHEAP_IMPLEMENTATION(CFriend,s_heap)

CFriend::CFriend()
{
}

CFriend::~CFriend()
{
}

CFriend* CFriend::CreateNewFriend(const FriendInfoStruct* pInfo)
{
	CHECKF(pInfo);

	CFriend*	pFriend = CFriend::CreateNew();
	if(pFriend)
	{
		memcpy(&pFriend->m_Info, pInfo, sizeof(FriendInfoStruct));
	}

	return pFriend;
}