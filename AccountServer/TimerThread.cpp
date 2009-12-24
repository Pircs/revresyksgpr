#include "AllHeads.h"
#include "TimerThread.h"

#include <iostream>

CTimerThread::CTimerThread() 
		: CThreadBase()
{
	m_nLoginAcceptP5= 0;
	m_nPointFeeP5= 0;
}

CTimerThread::~CTimerThread()
{
}

// ���߳���û���ⲿ��Ϣ���룬û�й����ͻ
void	CTimerThread::OnInit()
{
	LOCKTHREAD;
	try{

	}catch(...) { LOGCATCH("��¼�̳߳�ʼ���쳣�˳�"); }
}

bool	CTimerThread::OnProcess()
{
	LOCKTHREAD;

	try{
		for(;;)
		{
			Sleep(1000);
			// �������߱�
			g_pOnlineTable->RemoveOvertime();

			int		nMovieCount = 6;
			//		char	bufLoginChar[]	= " .oOUv .oOUv .oOUv .oOUv .oOUv";
			char	bufPointChar[]	= "-<(|)>-<(|)>-<(|)>-<(|)>-<(|)>";
			char	bufLoginChar[]	= "-<(|)>-<(|)>-<(|)>-<(|)>-<(|)>";

			//		char	bufScale[21]	= "....................";
			char	bufScale[21]	= "--------------------";
			char	bufScaleArrow[21]	= "===================>";
			int		nLoginCount	= InterlockedExchange(&s_nLoginCount, 0);
			int		nCurrPlayers	= g_pOnlineTable->GetPlayerCount();

			// ��������
			static int	st_nMaxPlayers = 0;
			static char	st_szMaxPlayersTime[20] = "0000-00-00 00:00:00";
			if(st_nMaxPlayers < nCurrPlayers)
			{
				st_nMaxPlayers	= nCurrPlayers;
				DateTime(st_szMaxPlayersTime, time(NULL));
			}
			/*
			// ÿСʱ��������
			static int	st_nMaxPlayersPerHour = 0;
			time_t	tCurr = time(NULL);
			tm * pTm = localtime(&tCurr);
			if(pTm->tm_min + pTm->tm_sec == 0)
			{
			if(st_nMaxPlayersPerHour)
			PrintText("����������%d", st_nMaxPlayersPerHour);
			st_nMaxPlayersPerHour = 0;
			}
			if(st_nMaxPlayersPerHour < nCurrPlayers)
			st_nMaxPlayersPerHour	= nCurrPlayers;
			//*/
			int		nMaxServerTime	= InterlockedExchange(&s_nMaxServerTime, 0);
			int		nAvgServerTime	= -1;
			if(nLoginCount)
				nAvgServerTime = InterlockedExchange(&s_nAvgServerTime, 0) / nLoginCount;
			int		nPointCount = InterlockedExchange(&s_nPointCount, 0);
			int		nLoginScale = 20 - (20*nLoginCount*LOGINLOOPDELAY/1000) % 21;
			int		nPointScale = 20 - (20*nPointCount*POINTLOOPDELAY/1000) % 21;
			int		nSocketCount	= InterlockedExchangeAdd(&s_nSocketCount, 0);
			int		nDatabaseTime	= InterlockedExchange(&s_nDatabaseTimeSum, 0);
			int		nAllTime	= InterlockedExchange(&s_nAllTimeSum, 0);
			if(!nAllTime)
				nAllTime = 20;
			int		nDatabaseScale = 20 - ((nDatabaseTime*20 / nAllTime) % 21);
			if(nSocketCount < 0)
				nSocketCount = 0;
			static int	st_nLoginPos = 0;		// rand() % nMovieCount;
			static int	st_nPointPos = 0;		// rand() % nMovieCount;
			if(nLoginCount)
			{
				st_nLoginPos = (st_nLoginPos + 1) % nMovieCount;
			}
			if(nPointCount)
			{
				st_nPointPos = (st_nPointPos + 1) % nMovieCount;
			}

			ASSERT(MAXGAMESERVERS <= 100);			//??? ÿһ�з�����״̬���֧��20��������
			char	bufServerState[100];
			memset(bufServerState, c_flagNone, 100);
#define	BSS bufServerState
			::GetServerState(bufServerState);

			// 5���Ӹ���һ��
			const int	nMinutes_per_times	= 300;
			const int	nFirstLog			= 0;
			static int	nSaveLog = 0;
			nSaveLog	= (++nSaveLog) % nMinutes_per_times;		// ÿ5���Ӵ���һ��
			bool	bSaveLog = (nSaveLog == nFirstLog);		// ?���ʼ��һ�δ���
			if(bSaveLog)
			{
				m_nLoginAcceptP5 = InterlockedExchange(&s_nLoginAccept, 0);
				m_nPointFeeP5  = InterlockedExchange(&s_nPointFee, 0);
			}
			int	nMinute5Scale = 20 - (((nSaveLog+nMinutes_per_times-nFirstLog)%nMinutes_per_times+1)*20/nMinutes_per_times) % 21;

			// �������
			time_t	tCurrTime;
			time( &tCurrTime );
			tm *	pTmFile = localtime(& g_tStartServerTime);
			int	nSec = (int)difftime(tCurrTime, g_tStartServerTime);
			/*m_sState.Format(//"%s\n\n"
			"����ʱ�䣺%04d-%02d-%02d  %02d:%02d\n"
			"����ʱ�䣺%d day(s)  %02d:%02d:%02d\n"
			"\n"
			"[%c]�Ƶ��̣߳�[%-20s]\n"
			"[%c]��¼�̣߳�[%-20s]\n"
			"\n"
			"������״̬��[%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c]\n"		// ״̬��Ϊ20��������
			"������״̬��[%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c]\n"
			"������״̬��[%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c]\n"
			"������״̬��[%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c]\n"
			"������״̬��[%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c]\n"
			"\n"
			"����������  [%-20s]\n"
			"���ݿ��ʱ��[%-20s]\n"
			// ---------------------------------------------
			//, SERVERTITLE
			, pTmFile->tm_year+1900, pTmFile->tm_mon+1, pTmFile->tm_mday
			, pTmFile->tm_hour, pTmFile->tm_min
			, nSec / 86400, (nSec/3600)%24, (nSec/60)%60, nSec%60
			, bufPointChar[st_nPointPos], bufScale + nPointScale
			, bufLoginChar[st_nLoginPos], bufScale + nLoginScale
			, ' ',BSS[1],BSS[2],BSS[3],BSS[4],BSS[5],BSS[6],BSS[7],BSS[8],BSS[9]
			, BSS[10],BSS[11],BSS[12],BSS[13],BSS[14],BSS[15],BSS[16],BSS[17],BSS[18],BSS[19]
			, BSS[20],BSS[21],BSS[22],BSS[23],BSS[24],BSS[25],BSS[26],BSS[27],BSS[28],BSS[29]
			, BSS[30],BSS[31],BSS[32],BSS[33],BSS[34],BSS[35],BSS[36],BSS[37],BSS[38],BSS[39]
			, BSS[40],BSS[41],BSS[42],BSS[43],BSS[44],BSS[45],BSS[46],BSS[47],BSS[48],BSS[49]
			, BSS[50],BSS[51],BSS[52],BSS[53],BSS[54],BSS[55],BSS[56],BSS[57],BSS[58],BSS[59]
			, BSS[60],BSS[61],BSS[62],BSS[63],BSS[64],BSS[65],BSS[66],BSS[67],BSS[68],BSS[69]
			, BSS[70],BSS[71],BSS[72],BSS[73],BSS[74],BSS[75],BSS[76],BSS[77],BSS[78],BSS[79]
			, BSS[80],BSS[81],BSS[82],BSS[83],BSS[84],BSS[85],BSS[86],BSS[87],BSS[88],BSS[89]
			, BSS[90],BSS[91],BSS[92],BSS[93],BSS[94],BSS[95],BSS[96],BSS[97],BSS[98],BSS[99]
			, bufScaleArrow + 20 - (nSocketCount*20/MAXCONNECTS)%21			//? ��ֻ��һ�����ʱ��Ӧ��ʾһ��
			, bufScaleArrow + nDatabaseScale
			);*/
			/*m_sState2.Format(//"%s\n\n"
			"[%-20s]\n"
			"5���ӵ�¼������%3d\n"
			"5���ӼƵ�������%3d\n"
			"�ۼƼƵ�������%4d\n"
			"\n"
			"������ұ�%d\n"
			"�����������%d\t%s\n"
			"����������max %dms, avg %dms\n"
			//, SERVERTITLE
			, bufScale + nMinute5Scale
			, m_nLoginAcceptP5
			, m_nPointFeeP5
			, InterlockedExchangeAdd(&s_nPointSum, 0)
			, nCurrPlayers
			, st_nMaxPlayers, st_szMaxPlayersTime
			, nMaxServerTime, nAvgServerTime
			);*/
			char bufText[4096];
			if(LockedGetText(bufText))
			{
				m_sText = bufText;
				//std::cout<<bufText<<endl;
			}
			//UpdateData(false);

			// 5���Ӵ���һ��
			if(bSaveLog)
			{
				LOGMSG(std::string(std::string("������״̬��\n") + m_sState + "\n" + m_sState2 + "\n" + bufText).c_str());
			}

			static int	nOnlineCount = 0;
			if(++nOnlineCount >= OUTPUTONLINESECS)
			{
				nOnlineCount = 0;

				// ͬʱ����������ͳ����
				FILE *	fOnline = fopen(ONLINEFILE, "w");
				if(fOnline)
				{
					char	bufTime[20];
					DateTime(bufTime, time(NULL));
					fprintf(fOnline, SERVERTITLE "\n");
					fprintf(fOnline, "%s\n", bufTime);
					int nAmount = g_pOnlineTable->GetPlayerCount();
					for(int i = 1; i < g_nServerAccount; i++)	// 1: ��1��ʼ��
					{
						LPCTSTR		szServerName = g_aServerAccount[i].m_szServerName;
						int			nCount = g_pOnlineTable->GetPlayerCount(szServerName);
						int			nState = g_pPointThread->GetServerState(szServerName);
						const char*		pState = "";
						if(nState == CPointThread::STATE_OFFLINE)
						{
							nAmount -= nCount;
							nCount = 0;
							pState = "(�ѶϿ�)";
						}
						else if(nState == CPointThread::STATE_BUSY)
							pState = "(æ)";
						else if(nState == CPointThread::STATE_FULL)
							pState = "(����)";
						//					fprintf(fOnline, "%2d(%2d). %-8s\t��%4d %s\n", 
						fprintf(fOnline, "%2d. %-4s\t��%4d %s\n", 
							i, 
							//								g_pPointThread->GetServerIndex(szServerName), 
							szServerName, 
							nCount, 
							pState
							);
					}
					fprintf(fOnline, "������������%6d\n", nAmount);
					fprintf(fOnline, ".\n");
					fprintf(fOnline, "������״̬��\n%s\n��ʱͳ�ƣ�\n%s\n�����Ϣ��\n%s\n", m_sState, m_sState2, bufText);
					fclose(fOnline);
				}
			}

			// ������������
			static int	nHeartbeatCount = 0;
			if(++nHeartbeatCount >= HEARTBEATINTERVALSECS / 5)		// 5���ӿ���Ƶ�ʣ��Ա�֤��ʱ�����ߡ�
			{
				g_pPointThread->CheckHeartbeatAll();
				nHeartbeatCount = 0;
			}

			// ����¼�ӳ�ʱ��
			static bool	bLogOK = false;
			if(!g_bEnableLogin && (clock()/CLOCKS_PER_SEC) >= ENABLELOGINDELAY)
			{
				g_bEnableLogin = true;
			}
			if(!bLogOK && g_bEnableLogin)
			{
				bLogOK = true;
				LOGMSG("�ʺŷ�������ʼ������ҵ�¼");
				PrintText("������ҵ�¼");
				std::cout<<getCurrTimeString()<<"������ҵ�¼"<<endl;
			}

			// дCOUNT.TXT
			static int	nCountCount = 0;
			if(++nCountCount >= COUNTFILESECS)
			{
				try{
					nCountCount = 0;
					g_pPointThread->LogCount();

					if(STAT_SERIAL)
						g_pPointThread->LogSerialCount();
				}catch(...){ LOGCATCH("дCOUNT.TXT"); }
			}
		}

		return true;
	}catch(...) { 
		LOGCATCH("��¼�߳���ѭ���쳣���Լ�������С�"); 
		PrintText("��¼�߳���ѭ�������Լ��������...");
		return true; 
	}
}

void	CTimerThread::OnDestroy()
{
	try{
		LOGMSG("��¼�߳������ر�");

	}catch(...) { LOGCATCH("��¼�̹߳ر�ʱ�쳣�˳�"); }
}













