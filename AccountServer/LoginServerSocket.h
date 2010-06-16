// huihui 2010.6.16
#pragma once
#include "ThreadBase.h"
#include "Msg.h"

class CLoginServerSocket : public CServerSocket<ACCOUNT_KEY1, ACCOUNT_KEY2>  
{
public:
	bool processMsg(Msg* pMsg);
	bool processMsgClientAccount(MsgC2SAccount& Msg);
	bool processMsgConnect(MsgConnect& Msg);
	void refuseLogin(LPCTSTR szLoginName, int nType, LPCTSTR szText);
	void allowLogin(OBJID idAccount, DWORD nAuthenID, LPCTSTR szServer);
	void addBan(LPCTSTR szAccount);
};