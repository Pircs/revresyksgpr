#include "MessagePort.h"
#include "typedef.h"
#include "protocol.h"
#include "common.h"
#include "IniFile.h"
#include "LogFile.h"
#include "I_Shell.h"
#include "SocketThread.h"
#include "WorldThread.h"
#include "MapGroupThread.h"

#include "P_ServerManager.h"
#include "basetype.h"
#include "array.h"
#include "mystack.h"
#include "I_MessagePort.h"
#include "TimerThread.h"

#include <iostream>


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


CTimerThread::CTimerThread() 
		: CThreadBase()
{
	m_pSocketThread	= NULL;
	m_pWorldThread	= NULL;

	m_nAllPlayers	= 0;
	m_nMaxPlayers	= 0;

	/////////////////////////////////
	m_nState		= SHELLSTATE_NONE;
	m_nTextLines	= 0;
	m_pMsgPort		= NULL;
	m_pInterPort		= NULL;

	m_hMutexThread	= NULL;
}

CTimerThread::~CTimerThread()
{
}

// 该线程类没有外部消息进入，没有共享冲突
void	CTimerThread::OnInit()
{
	LOCK_THREAD;
	try{
		OnInitDialog();

	}catch(...) { LOGCATCH("登录线程初始化异常退出"); }
}

bool CTimerThread::ProcessMsg(OBJID idPacket, void* pMsg, int nType, int nSource)
{
	switch(idPacket)
	{
	case	SHELL_PRINTTEXT:
		{
			//PrintText((char*)pMsg);
			std::cout<<pMsg<<endl;
		}
		break;
	case	SHELL_KERNELSTATE:
		{
			m_sKernelState = (char*)pMsg;
			//UpdateData(false);
		}
	default:
		LOGERROR("Shell process invalid msg [%d].", idPacket);
		return false;
	}
	return true;
}

bool CTimerThread::ProcessInterMsg()
{
	if(!m_pInterPort)
		return false;

	char			buf[MAX_MESSAGESIZE];
	CMessageStatus	cStatus;

	DEBUG_TRY	// VVVVVVVVVVVVVV
		if(!m_pInterPort->IsOpen())
		{
			if(!m_pInterPort->Open())
				return false;
		}

		while(m_pInterPort->Recv(PORT_ANY, PACKET_ANY, STRUCT_TYPE(buf), buf, &cStatus))
		{
			switch(cStatus.m_nPacket)
			{
			case	QUERY_STATUS:
				{
					m_pInterPort->Send(cStatus.m_nPortFrom, ACK_TITLE, STRING_TYPE(m_szServer.c_str()), m_szServer.c_str());
					m_pInterPort->Send(cStatus.m_nPortFrom, ACK_SHELLMSG, STRING_TYPE(m_sShellState.c_str()), m_sShellState.c_str());
					m_pInterPort->Send(cStatus.m_nPortFrom, ACK_KERNELMSG, STRING_TYPE(m_sKernelState.c_str()), m_sKernelState.c_str());
					m_pInterPort->Send(cStatus.m_nPortFrom, ACK_TEXT, STRING_TYPE(m_sText.c_str()), m_sText.c_str());
				}
				break;
			case	QUERY_COMMOND:
				{
					LPCTSTR pStr = (LPCTSTR)buf;
					std::string str= "【remote】";
					str+=pStr;
					PrintText(str);
					m_pMsgPort->Send(MSGPORT_WORLD, WORLD_SHELLTALK, STRING_TYPE(pStr), pStr);
				}
				break;
			default:
				PrintText("CMsgServerDlg::ProcessInterMsg()");
				return false;
			}
		}
		DEBUG_CATCH("CMsgServerDlg::Process()")	// AAAAAAAAAAA

			return true;
}

bool CTimerThread::Process()
{
	char			buf[MAX_MESSAGESIZE];
	CMessageStatus	cStatus;

	DEBUG_TRY	// VVVVVVVVVVVVVV
		while(m_pMsgPort->Recv(PORT_ANY, PACKET_ANY, STRUCT_TYPE(buf), buf, &cStatus))
		{
			if(ProcessMsg(cStatus.m_nPacket, buf, cStatus.m_nVarType, cStatus.m_nPortFrom))
				return true;

			PrintText("ERROR: CMsgServerDlg::Process() ProcessMsg()");
			return false;
		}
		DEBUG_CATCH("CMsgServerDlg::Process()")	// AAAAAAAAAAA

			return true;
}


bool	CTimerThread::OnProcess()
{
	Sleep(200);
	LOCK_THREAD;
	try{
#ifdef	DEBUG_MULTITHREAD
		if(::WaitForSingleObject(m_hMutexThread, INFINITE) == WAIT_ABANDONED)
			return ;
#endif
		static	int nLock = 0;
		if(/*nIDEvent*/nLock <= 0)
		{
			nLock++;	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			//!!! Can't use 'return' in this block

			if(m_pMsgPort)
				Process();

			DEBUG_TRY		// VVVVVVVVVVVVVVVVVVVVVVVVVVVV
				if(m_pInterPort)
					ProcessInterMsg();
			DEBUG_CATCH("ProcessInterMsg()")		// AAAAAAAAAAAAAAAAAAAAAAAAAAAAA

				extern long				g_nRestart;
			switch(m_nState)
			{
			case	SHELLSTATE_NONE:
				{
					m_nState	= SHELLSTATE_END;
				}
				break;
			case	SHELLSTATE_INIT:
				{
					m_nState	= SHELLSTATE_CLOSING;		// 失败时退出

					PrintText("1、Init world thread...");
					m_pWorldThread = new CWorldThread(CMessagePort::GetInterface(MSGPORT_WORLD));
					if(!m_pWorldThread)
						break;
					if(!m_pWorldThread->CreateThread(false))
						break;

					PrintText("2、Init map group thread...");
					int i;
					for(i = 0; i < CONFIG.MAPGROUP_SIZE; i++)
					{
						CMapGroupThread* pThread = new CMapGroupThread(CMessagePort::GetInterface(i + MSGPORT_MAPGROUP_FIRST));
						if(!pThread)
							break; // for
						m_setMapGroupThread.push_back(pThread);
						if(!pThread->CreateThread(false))
							break; // for
					}
					if(i < CONFIG.MAPGROUP_SIZE)
						break;

					PrintText("3、Init socket thread...");
					m_pSocketThread = new CSocketThread(CMessagePort::GetInterface(MSGPORT_SOCKET));
					if(!m_pSocketThread)
						break;
					if(!m_pSocketThread->CreateThread(false))
						break;
					PrintText("Init OK。");

					m_pWorldThread->ResumeThread();
					for(int i = 0; i < CONFIG.MAPGROUP_SIZE; i++)
						m_setMapGroupThread[i]->ResumeThread();
					m_pSocketThread->ResumeThread();
					PrintText("All thread start OK。");

					m_nState	= SHELLSTATE_RUNNING;
				}
				break;
			case	SHELLSTATE_RUNNING:
				{
					// 状态刷新
					const int nSecsPerLogin = 5*60;
					if(m_tStat5Min.ToNextTime(nSecsPerLogin))
					{
						m_nLoginPlayers		= InterlockedExchange(&g_stat.nLoginPlayers, 0);
						m_nLogoutPlayers	= InterlockedExchange(&g_stat.nLogoutPlayers, 0);

						LOGMSG("\n-----------------------------------------------\n"
							"%s"
							"-----------------------------\n"
							"%s"
							"-----------------------------------------------\n", 
							m_sShellState, m_sKernelState);
					}
					const int nSecsPerUpdate = 5;
					if(m_tStat.ToNextTime(nSecsPerUpdate))
					{
						long lDatabaseTime	= ::InterlockedExchange(&s_nDatabaseTimeSum, 0);

						memcpy(&m_stat, &g_stat, sizeof(g_stat));

						STAT_STRUCT stat;
						memset(&stat, 0L, sizeof(STAT_STRUCT));
						stat.nAllPlayers	= m_stat.nAllPlayers;
						memcpy(&g_stat, &stat, sizeof(STAT_STRUCT));
						memcpy(&g_stat, &stat, sizeof(STAT_STRUCT));
						//					memset(&g_stat, 0, sizeof(g_stat));
						//					memset(&g_stat, 0, sizeof(g_stat));		// 保证清空:)
						//					g_stat.nAllPlayers	= m_stat.nAllPlayers;

						SafeCopy(m_stat.szStartServer, m_szStartServer, 20);
						DateTime(m_stat.szCurr, time(NULL));
						//					if(m_stat.nAllPlayers)
						m_nAllPlayers = m_stat.nAllPlayers;
						if(m_stat.nMaxPlayers)
							m_nMaxPlayers = m_stat.nMaxPlayers;

						// update
						/*m_sShellState.Format("start server: %s\r\n"
						"current: %s\r\n\r\n"
						"socket thread: %3d(max:%d)\r\n"
						"world thread:  %3d(max:%d)\r\n"
						"map group 0 thread: %3d(max:%d)[%3d]\r\n"
						"map group 1 thread: %3d(max:%d)\r\n"
						"map group 2 thread: %3d(max:%d)\r\n"
						"map group 3 thread: %3d(max:%d)\r\n"
						"map group 4 thread: %3d(max:%d)\r\n"
						"debug [%d][%d][%d][%d][%d]\r\n"
						"OnTimer [%3d] Database [%3d]\r\n"
						, m_stat.szStartServer
						, m_stat.szCurr
						, m_stat.nSocketTimes ? m_stat.nAllSocketMS/m_stat.nSocketTimes : 0
						, m_stat.nMaxSocketMS
						, m_stat.nWorldTimes ? m_stat.nAllWorldMS/m_stat.nWorldTimes : 0
						, m_stat.nMaxWorldMS
						, m_stat.setMapGroupTimes[0] ? m_stat.setAllMapGroupMS[0]/m_stat.setMapGroupTimes[0] : 0
						, m_stat.setMaxMapGroupMS[0]
						, m_stat.nPacketID
						, m_stat.setMapGroupTimes[1] ? m_stat.setAllMapGroupMS[1]/m_stat.setMapGroupTimes[1] : 0
						, m_stat.setMaxMapGroupMS[1]
						, m_stat.setMapGroupTimes[2] ? m_stat.setAllMapGroupMS[2]/m_stat.setMapGroupTimes[2] : 0
						, m_stat.setMaxMapGroupMS[2]
						, m_stat.setMapGroupTimes[3] ? m_stat.setAllMapGroupMS[3]/m_stat.setMapGroupTimes[3] : 0
						, m_stat.setMaxMapGroupMS[3]
						, m_stat.setMapGroupTimes[4] ? m_stat.setAllMapGroupMS[4]/m_stat.setMapGroupTimes[4] : 0
						, m_stat.setMaxMapGroupMS[4]
						, m_stat.setDebug[0], m_stat.setDebug[1], m_stat.setDebug[2], m_stat.setDebug[3], m_stat.setDebug[4]
						, m_stat.setMapGroupTimerMS[0], lDatabaseTime
						);
						m_sKernelState.Format("players: %3d(max:%d)\r\n"
						"login: %d/5min, logout: %d/5min\r\n\r\n"
						"socket bytes: %3d/s (pps: %d)\r\n"
						"npc socket bytes: %3d/s (pps: %d)\r\n"
						, m_nAllPlayers, m_nMaxPlayers
						, m_stat.nLoginPlayers, m_stat.nLogoutPlayers
						, m_stat.nSocketBytes/nSecsPerUpdate, m_stat.nSocketPackets/nSecsPerUpdate
						, m_stat.nNpcSocketBytes/nSecsPerUpdate, m_stat.nNpcSocketPackets/nSecsPerUpdate
						);*/
						//UpdateData(false);
						FILE* pFile = fopen(ONLINE_FILENAME, "wt");
						if(pFile)
						{
							fprintf(pFile, m_sShellState.c_str());
							fprintf(pFile, "\n\n");
							fprintf(pFile, m_sKernelState.c_str());
							fclose(pFile);
						}
					}

					// dead loop
					extern long g_nMessagePortErrorCount;
					if(g_nMessagePortErrorCount > 0 && g_nMessagePortErrorCount%50000 == 0)
					{
						InterlockedExchange(&g_nMessagePortErrorCount, 0);

						PrintText("Message pipe block, server may be slowly!!!");
						LOGERROR("dump all call stack:");
						while(DeadLoopSet()[MSGPORT_MAPGROUP_FIRST].size())
							LOGERROR("* %s", (LPCTSTR)DeadLoopSet()[MSGPORT_MAPGROUP_FIRST].pop());
					}

					// restart server
					extern long g_nRestart;
					if(g_nRestart)
						m_nState = SHELLSTATE_CLOSING;
				}
				break;
			case	SHELLSTATE_CLOSING:
				{
					// 关闭核心
					PrintText("Kernel closing...");

					int i;
					//*
					// 通知线程关闭
					if(m_pSocketThread)
						m_pSocketThread->CloseThread();
					for(int i = 0; i < m_setMapGroupThread.size(); i++)
						m_setMapGroupThread[i]->CloseThread();
					if(m_pWorldThread)
						m_pWorldThread->CloseThread();
					//*/
					// 等待线程关闭
					if(m_pSocketThread)
					{
						if(m_pSocketThread->CloseThread(CLOSETHREAD_MILLISECS))
							PrintText("Close socket OK。");
						else
							PrintText("Close socket failed!");
						S_DEL(m_pSocketThread);
						Process();
					}
					for(int i = 0; i < m_setMapGroupThread.size(); i++)
					{
						char szText[1024];
						if(m_setMapGroupThread[i]->CloseThread(CLOSETHREAD_MILLISECS))
							sprintf(szText, "Close map group thread[%d] OK。", i);
						else
							sprintf(szText, "Close map group thread[%d] failed!", i);
						PrintText(szText);
						S_DEL(m_setMapGroupThread[i]);
						Process();
					}
					if(m_pWorldThread)
					{
						if(m_pWorldThread->CloseThread(CLOSETHREAD_MILLISECS))
							PrintText("Close world thread OK。");
						else
							PrintText("Close world thread failed!");
						S_DEL(m_pWorldThread);
						Process();
					}

					if (m_hMutexServer)
					{
						::WaitForSingleObject(m_hMutexServer, 10*1000);
						::ReleaseMutex(m_hMutexServer);
						::CloseHandle(m_hMutexServer);
						m_hMutexServer = NULL;
					}

					PrintText("Server is over, close all  after 3 second!");
					Process();

					m_pMsgPort->Close();
					CMessagePort::ClearPortSet();
					m_pMsgPort = NULL;

					if(m_pInterPort)
					{
						S_DEL(m_pInterPort);
					}

					m_nState	= SHELLSTATE_END;
				}
				break;
			case	SHELLSTATE_END:
				{
					//	KillTimer(1);
#ifdef	DEBUG_MULTITHREAD
					if(m_hMutexThread)
					{
						::CloseHandle(m_hMutexThread);
						m_hMutexThread = NULL;
					}
#endif
					Sleep(3000);

					//	if(g_nRestart)
					//		::ShellExecute(m_hWnd, NULL, AfxGetAppName(), NULL, NULL, SW_SHOWNORMAL);

					//		CDialog::OnOK();
				}
				break;
			default:
				ASSERT(!"switch(m_nState)");
			}
			nLock--;	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef	DEBUG_MULTITHREAD
			::ReleaseMutex(m_hMutexThread);
#endif
		}
		return true;
	}catch(...) {
		LOGCATCH("登录线程主循环异常，仍坚持运行中。"); 
		PrintText("登录线程主循环出错，仍坚持运行中...");
		return true; 
	}
}

void	CTimerThread::OnDestroy()
{
	try{
		m_nState	= SHELLSTATE_CLOSING;
		PrintText("Server closing...");
		LOGMSG("登录线程正常关闭");

	}catch(...) { LOGCATCH("登录线程关闭时异常退出"); }
}

BOOL CTimerThread::OnInitDialog()
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
			std::cout<<"Repeat run game server!"<<endl;
			//MessageBox("Repeat run game server!");
		//	this->EndDialog(-1);
			return false;
		}
	}
	else
	{
		std::cout<<"Create mutex failed!"<<endl;
		//MessageBox("Create mutex failed!");
	//	this->EndDialog(-1);
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
	{
		std::cout<<"Load config.ini failed!"<<endl;
		//MessageBox("Load config.ini failed!");
	}
	else if(!CMessagePort::InitPortSet(MSGPORT_MAPGROUP_FIRST + CONFIG.MAPGROUP_SIZE))
	{
		std::cout<<"Initial intra message port failed!"<<endl;
		//MessageBox("Initial intra message port failed!");
	}
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
	ini.getString(m_szServer, "SERVERNAME");

	// windows title
	//CString strTitle;
	//strTitle.Format("%s - %s (%s %s)", GAME_TITLE, m_szServer, __DATE__, __TIME__);
	//SetWindowText(strTitle);

	// init log file
	CreateDirectory(LOGFILE_DIR, NULL);
	//InitLog(strTitle, LOGFILE_FILENAME, time(NULL));
	LOGMSG("\n\n\n=================================================================");
//	LOGMSG(strTitle);
	LOGMSG("=================================================================");

	return TRUE; 
}

void CTimerThread::Send(const std::string& strCmd) 
{
	if(m_pMsgPort)
	{
		m_sText =strCmd;
		m_pMsgPort->Send(MSGPORT_WORLD, WORLD_SHELLTALK, STRING_TYPE((LPCTSTR)strCmd.c_str()), (LPCTSTR)strCmd.c_str());
	}
}

void CTimerThread::PrintText(const std::string& szMsg)
{
	if(m_nTextLines >= TEXTWINDOW_SIZE)
	{
		//int nPos = m_sText.Find("\n", 0);
		//if(nPos != -1)
			m_sText = "";//m_sText.Mid(nPos + 1);
	}

	char	buf[20];
	DateTime(buf);
	m_sText += buf+11;
	m_sText += "【";
	m_sText += szMsg;
	m_sText += "】";
	m_sText += "\r\n";
	m_nTextLines++;

	static char szCurrTime[60];
	FormatDateTime(szCurrTime, "[%04d-%02d-%02d %02d:%02d:%02d]: ", time(NULL));		// szFormat: 
	std::cout<<szCurrTime<<szMsg<<endl;

	LOGMSG("SHELL: %s", szMsg);
}

bool CTimerThread::LoadConfigIni()
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







