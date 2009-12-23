#pragma once

#include "Timeout.h"

const int	DELETECONNECTUSER_SECS		= 20;		// 20秒超时，删除连接表

class CConnectUser  
{
protected:
	CConnectUser();
	virtual ~CConnectUser();
public:
	static CConnectUser* CreateNew()	{ return new CConnectUser; }
	ULONG	ReleaseByOwner()			{ delete this; return 0; }

	OBJID	GetID()						{ return m_idAccount; }
	bool	Create(OBJID id, DWORD dw, LPCTSTR szInfo, SOCKET_ID idSocket = SOCKET_NONE);

public:
	bool	CheckData(DWORD dw)			{ return m_dwCheckData == dw; }
	SOCKET_ID	GetSocketID()			{ return m_idSocket; }
	LPCTSTR	GetInfo()					{ return m_szInfo; }
	bool	IsValid()					{ return !m_tDelete.IsTimeOut(); }

protected:
	OBJID		m_idAccount;
	DWORD		m_dwCheckData;
	NAMESTR		m_szInfo;
	SOCKET_ID	m_idSocket;

	CTimeOut	m_tDelete;

protected: // ctrl
	MYHEAP_DECLARATION(s_heap)
};


