
#pragma once

#include "FriendData.h"
#include "Myheap.h"
#include "TimeOut.h"


const int	LEAVEWORD_INTERVAL_MINS	= 1;							// ¡Ù—‘º‰∏Ù∑÷÷” ˝


class CUser;
class CFriend : public CFriendData  
{
public:
	CFriend();
	virtual ~CFriend();
	ULONG	ReleaseByOwner() { delete this; return 0; }

public:
	static CFriend*	CreateNewFriend		(const FriendInfoStruct* pInfo);

public: // static
	static CFriend*	CreateNew() { return (new CFriend); }

public: // application
	bool	IsLeaveWordEnable()				{ return m_tLeaveWord.ToNextTime(LEAVEWORD_INTERVAL_MINS*60); }

protected:
	CTimeOut	m_tLeaveWord;

protected: // ctrl
	MYHEAP_DECLARATION(s_heap)
};


