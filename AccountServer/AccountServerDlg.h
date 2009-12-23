// AccountServerDlg.h : header file
//

#pragma once

///////
// CAccountServerDlg dialog

class CAccountServerDlg : public CDialog
{
// Construction
public:
	CAccountServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAccountServerDlg)
	enum { IDD = IDD_ACCOUNTSERVER_DIALOG };
	CString	m_sText;
	CString	m_sState2;
	CString	m_sState;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAccountServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nLoginAcceptP5;
	long m_nPointFeeP5;
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAccountServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnCloseButtom();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


