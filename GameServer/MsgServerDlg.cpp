#include "typedef.h"
#include "protocol.h"
#include "common.h"
#include "IniFile.h"
#include "LogFile.h"
#include "I_Shell.h"
#include "SocketThread.h"
#include "WorldThread.h"
#include "MapGroupThread.h"
#include "MessagePort.h"
#include "P_ServerManager.h"
#include "basetype.h"
#include "array.h"
#include "mystack.h"
#include "I_MessagePort.h"

#include "MsgServer.h"
#include "MsgServerDlg.h"

typedef	CMyStack<const char*>	DEADLOOP;
typedef	Array<DEADLOOP>		DEADLOOP_SET;
DEADLOOP_SET	g_setDeadLoop;
inline DEADLOOP_SET&	DeadLoopSet() { return g_setDeadLoop; }///////
ST_CONFIG		CONFIG;	
///////
struct STAT_STRUCT	g_stat = {0};
long				g_nRestart		= false;
long				g_nServerClosed = false;

long				s_nDatabaseTimeSum;



CMsgServerDlg::CMsgServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgServerDlg::IDD, pParent)
{
	m_nState		= SHELLSTATE_NONE;
	m_nTextLines	= 0;
	m_pMsgPort		= NULL;
	m_pInterPort		= NULL;


	m_hMutexThread	= NULL;
}

BOOL CMsgServerDlg::OnInitDialog()
{
	memset(&g_stat, 0, sizeof(g_stat));
	DateTime(m_szStartServer, time(NULL));

	m_hMutexServer = ::CreateMutex(NULL, false, "ConquerServer");
	if (m_hMutexServer)
	{
		if (ERROR_ALREADY_EXISTS == ::GetLastError())
		{
			::ReleaseMutex(m_hMutexServer);
			::CloseHandle(m_hMutexServer);
			m_hMutexServer = NULL;
			MessageBox("Repeat run game server!");
			this->EndDialog(-1);
			return false;
		}
	}
	else
	{
		MessageBox("Create mutex failed!");
		this->EndDialog(-1);
		return false;
	}

#ifdef	DEBUG_MULTITHREAD
	m_hMutexThread    =::CreateMutex(NULL, false, "FW_DEBUG_MULTITHREAD");
	if(!m_hMutexThread)
	{
		PrintText("Create mutex handle failed!");
		return false;
	}
#endif
	if(!LoadConfigIni())
		MessageBox("Load config.ini failed!");
	else if(!CMessagePort::InitPortSet(MSGPORT_MAPGROUP_FIRST + CONFIG.MAPGROUP_SIZE))
		MessageBox("Initial intra message port failed!");
	else
	{
		m_pMsgPort = CMessagePort::GetInterface(MSGPORT_SHELL);
		m_pMsgPort->Open();
		m_nState	= SHELLSTATE_INIT;

		if(CONFIG.CURRENT_PORTID)
		{
			m_pInterPort = CInternetPort::CreateNew(CONFIG.CURRENT_PORTID, CONFIG.PORT_SIZE, CONFIG.MASTER_IP, CONFIG.MASTER_PORT, CONFIG.LOGIN_KEY);
			if(m_pInterPort)
				m_pInterPort->Open();
		}
	}

	// dead loop init
	for(int i = 0; i < CONFIG.MAPGROUP_SIZE + MSGPORT_MAPGROUP_FIRST; i++)
		DeadLoopSet().Push(DEADLOOP());

	// get game title
	CIniFile	ini("shell.ini", "AccountServer");
	ini.GetString(m_szServer, "SERVERNAME", _MAX_NAMESIZE);

	// windows title
	CString strTitle;
	strTitle.Format("%s - %s (%s %s)", GAME_TITLE, m_szServer, __DATE__, __TIME__);
	SetWindowText(strTitle);

	// init log file
	CreateDirectory(LOGFILE_DIR, NULL);
	InitLog(strTitle, LOGFILE_FILENAME, time(NULL));
	LOGMSG("\n\n\n=================================================================");
	LOGMSG(strTitle);
	LOGMSG("=================================================================");

	SetTimer(1, 200, NULL);				// 句柄为1，刷新间隔为200MS

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMsgServerDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	PrintText("CLOSE key pressed");
	if(AfxMessageBox("Are you sure?", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK)
	{
		m_nState	= SHELLSTATE_CLOSING;
		PrintText("Server closing...");
	}
}

void CMsgServerDlg::OnSend() 
{
	this->UpdateData(TRUE);
	if(m_pMsgPort)
	{
		m_pMsgPort->Send(MSGPORT_WORLD, WORLD_SHELLTALK, STRING_TYPE((LPCTSTR)m_sTalk), (LPCTSTR)m_sTalk);
	}
}

void CMsgServerDlg::PrintText(LPCTSTR szMsg)
{
	if(m_nTextLines >= TEXTWINDOW_SIZE)
	{
		int nPos = m_sText.Find("\n", 0);
		if(nPos != -1)
			m_sText = m_sText.Mid(nPos + 1);
	}

	char	buf[20];
	DateTime(buf);
	m_sText += buf+11;
	m_sText += "【";
	m_sText += szMsg;
	m_sText += "】";
	m_sText += "\r\n";
	m_nTextLines++;

	UpdateData(false);

	LOGMSG("SHELL: %s", szMsg);
}



bool CMsgServerDlg::LoadConfigIni()
{
	CIniFile	ini(CONFIG_FILENAME, "System");

	// 初值
	CONFIG.MAPGROUP_SIZE	= 0;
	CONFIG.CURRENT_PORTID	= 0;
	CONFIG.PORT_SIZE		= 0;
	CONFIG.MASTER_IP[0]		= 0;
	CONFIG.MASTER_PORT		= 0;
	CONFIG.LOGIN_KEY[0]		= 0;

	// 读入
	CONFIG.MAPGROUP_SIZE	= ini.GetInt("MAPGROUP_SIZE");
	if(CONFIG.MAPGROUP_SIZE == 0)
		return false;

	ini.SetSection("InternetPort");
	CONFIG.CURRENT_PORTID	= ini.GetInt("CURRENT_PORTID");
	CONFIG.PORT_SIZE		= ini.GetInt("PORT_SIZE");
	ini.GetString(CONFIG.MASTER_IP, "MASTER_IP", IPSTR_SIZE);
	CONFIG.MASTER_PORT		= ini.GetInt("MASTER_PORT");
	ini.GetString(CONFIG.LOGIN_KEY, "LOGIN_KEY", 256);
	if(CONFIG.MAPGROUP_SIZE == 0)
		return false;

	return true;
}