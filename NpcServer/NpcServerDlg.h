#pragma once

//#include <afxmt.h>
#include "KernelThread.h"
#include "Timeout.h"
#include "InternetPort.h"

struct STAT_ST
{
	long	nKernelCount;
	long	nKernelSum;
	long	nKernelMax;
};///////
// CNpcServerDlg dialog

class IMessagePort;
class CNpcServerDlg : public CDialog, public IDialog
{
// Construction
public:
	CNpcServerDlg(CWnd* pParent = NULL);	// standard constructor

public: // interface
	bool ProcessInterMsg();
	void SetShellState();
	void ShowText();
	void PrintText(LPCTSTR szMsg);
	void CloseAll();
	void SetState(LPCTSTR szMsg);
	void ChangeEncrypt(DWORD nKey);

// Dialog Data
	//{{AFX_DATA(CNpcServerDlg)
	enum { IDD = IDD_NPCSERVER_DIALOG };
	CEdit	m_ctlText;
	CString	m_sKernel;
	CString	m_sShell;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNpcServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	LOCK_DECLARATION;
	CString			m_sUpdate;
	HANDLE			m_hMutexServer;
	CTimeOut		m_tUpdateStateWin;
	CTimeOut		m_tSaveStateWin;
	IMessagePort*	m_pInterPort;
	char			m_szServerName[16];
	CString			m_sText;
	int				m_nTextLines;

	CKernelThread m_thdKernel;
	enum { STATE_NONE, STATE_STARTING, STATE_NORMAL, STATE_CLOSING, STATE_CLOSED };
	int m_nState;

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNpcServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


