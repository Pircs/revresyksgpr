

#pragma once

#include "ThreadBase.h"
#include "ClientSocket.h"
#include "I_Shell.h"

class IShell;
class CKernelThread : public CThreadBase, IShell
{
public:
	CKernelThread();
	virtual ~CKernelThread();

protected: // IShell
	virtual void	PrintText(LPCTSTR szMsg)			{ m_pDialog->PrintText(szMsg); }
	virtual void	CloseAll()							{ m_pDialog->CloseAll(); }
	virtual void	SetState(LPCTSTR szState)			{ m_pDialog->SetState(szState); }
	virtual	void	ChangeEncrypt(DWORD nKey)			{ m_pDialog->ChangeEncrypt(nKey); }
protected:
	virtual bool	SendPacket(const char* pack, int nLen, bool bFlush = false)
				{ return m_sockClient.SendPacket(pack, nLen, bFlush); }
	virtual bool	IsConnectOK()
				{ return m_sockClient.IsOpen(); }

public: // dynamic link
	virtual	void	OnInit();
	virtual bool	OnProcess();
	virtual	void	OnDestroy();

public: // static link
	bool	CreateThread(IDialog* pDialog, bool bRun = true);		// false: 暂不运行，用 ResumeThread() 运行

protected:
	IDialog*		m_pDialog;
	CClientSocket	m_sockClient;
	IKernel*		m_pKernel;

	clock_t			m_tNextClock;
};


