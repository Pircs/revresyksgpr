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
#include "TimerThread.h"

#include <iostream>

CTimerThread::CTimerThread() 
		: CThreadBase()
{
	m_pSocketThread	= NULL;
	m_pWorldThread	= NULL;

	m_nAllPlayers	= 0;
	m_nMaxPlayers	= 0;
}

CTimerThread::~CTimerThread()
{
}

// ���߳���û���ⲿ��Ϣ���룬û�й�����ͻ
void	CTimerThread::OnInit()
{
	LOCKTHREAD;
	try{

	}catch(...) { LOGCATCH("��¼�̳߳�ʼ���쳣�˳�"); }
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
					m_pInterPort->Send(cStatus.m_nPortFrom, ACK_TITLE, STRING_TYPE(m_szServer), m_szServer);
					m_pInterPort->Send(cStatus.m_nPortFrom, ACK_SHELLMSG, STRING_TYPE(m_sShellState), m_sShellState);
					m_pInterPort->Send(cStatus.m_nPortFrom, ACK_KERNELMSG, STRING_TYPE(m_sKernelState), m_sKernelState);
					m_pInterPort->Send(cStatus.m_nPortFrom, ACK_TEXT, STRING_TYPE(m_sText), m_sText);
				}
				break;
			case	QUERY_COMMOND:
				{
					LPCTSTR pStr = (LPCTSTR)buf;
					CString str;
					str.Format("��remote��%s", pStr);
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
	LOCKTHREAD;
	try{
		for(;;)
		{
			Sleep(200);
#ifdef	DEBUG_MULTITHREAD
			if(::WaitForSingleObject(m_hMutexThread, INFINITE) == WAIT_ABANDONED)
				return ;
#endif

			static	int nLock = 0;
			if(nIDEvent == 1 && nLock <= 0)
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
						m_nState	= SHELLSTATE_CLOSING;		// ʧ��ʱ�˳�

						PrintText("1��Init world thread...");
						m_pWorldThread = new CWorldThread(CMessagePort::GetInterface(MSGPORT_WORLD));
						if(!m_pWorldThread)
							break;
						if(!m_pWorldThread->CreateThread(false))
							break;

						PrintText("2��Init map group thread...");
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

						PrintText("3��Init socket thread...");
						m_pSocketThread = new CSocketThread(CMessagePort::GetInterface(MSGPORT_SOCKET));
						if(!m_pSocketThread)
							break;
						if(!m_pSocketThread->CreateThread(false))
							break;
						PrintText("Init OK��");

						m_pWorldThread->ResumeThread();
						for(int i = 0; i < CONFIG.MAPGROUP_SIZE; i++)
							m_setMapGroupThread[i]->ResumeThread();
						m_pSocketThread->ResumeThread();
						PrintText("All thread start OK��");

						m_nState	= SHELLSTATE_RUNNING;
					}
					break;
				case	SHELLSTATE_RUNNING:
					{
						// ״̬ˢ��
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
							//					memset(&g_stat, 0, sizeof(g_stat));		// ��֤���:)
							//					g_stat.nAllPlayers	= m_stat.nAllPlayers;

							SafeCopy(m_stat.szStartServer, m_szStartServer, 20);
							DateTime(m_stat.szCurr, time(NULL));
							//					if(m_stat.nAllPlayers)
							m_nAllPlayers = m_stat.nAllPlayers;
							if(m_stat.nMaxPlayers)
								m_nMaxPlayers = m_stat.nMaxPlayers;

							// update
							m_sShellState.Format("start server: %s\r\n"
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
								);
							UpdateData(false);
							FILE* pFile = fopen(ONLINE_FILENAME, "wt");
							if(pFile)
							{
								fprintf(pFile, m_sShellState);
								fprintf(pFile, "\n\n");
								fprintf(pFile, m_sKernelState);
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
						// �رպ���
						PrintText("Kernel closing...");

						int i;
						//*
						// ֪ͨ�̹߳ر�
						if(m_pSocketThread)
							m_pSocketThread->CloseThread();
						for(int i = 0; i < m_setMapGroupThread.size(); i++)
							m_setMapGroupThread[i]->CloseThread();
						if(m_pWorldThread)
							m_pWorldThread->CloseThread();
						//*/
						// �ȴ��̹߳ر�
						if(m_pSocketThread)
						{
							if(m_pSocketThread->CloseThread(CLOSETHREAD_MILLISECS))
								PrintText("Close socket OK��");
							else
								PrintText("Close socket failed!");
							S_DEL(m_pSocketThread);
							Process();
						}
						for(int i = 0; i < m_setMapGroupThread.size(); i++)
						{
							char szText[1024];
							if(m_setMapGroupThread[i]->CloseThread(CLOSETHREAD_MILLISECS))
								sprintf(szText, "Close map group thread[%d] OK��", i);
							else
								sprintf(szText, "Close map group thread[%d] failed!", i);
							PrintText(szText);
							S_DEL(m_setMapGroupThread[i]);
							Process();
						}
						if(m_pWorldThread)
						{
							if(m_pWorldThread->CloseThread(CLOSETHREAD_MILLISECS))
								PrintText("Close world thread OK��");
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
						KillTimer(1);
#ifdef	DEBUG_MULTITHREAD
						if(m_hMutexThread)
						{
							::CloseHandle(m_hMutexThread);
							m_hMutexThread = NULL;
						}
#endif
						Sleep(3000);

						if(g_nRestart)
							::ShellExecute(m_hWnd, NULL, AfxGetAppName(), NULL, NULL, SW_SHOWNORMAL);

						CDialog::OnOK();
					}
					break;
				default:
					ASSERT(!"switch(m_nState)");
				}

				nLock--;	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			}

#ifdef	DEBUG_MULTITHREAD
			::ReleaseMutex(m_hMutexThread);
#endif
		}
		return true;
	}catch(...) { 
		LOGCATCH("��¼�߳���ѭ���쳣���Լ�������С�"); 
		PrintText("��¼�߳���ѭ���������Լ��������...");
		return true; 
	}
}

void	CTimerThread::OnDestroy()
{
	try{
		LOGMSG("��¼�߳������ر�");

	}catch(...) { LOGCATCH("��¼�̹߳ر�ʱ�쳣�˳�"); }
}












