#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <tchar.h>

#include "allheads.h"

#include <iostream>

BOOL OnInitDialog()
{
	SetWindowText(NULL,SERVERTITLE);
	time( &g_tStartServerTime );
	char	bufStart[20];
	DateTime(bufStart, g_tStartServerTime);

	CreateDirectory("!log!", NULL);
	CreateDirectory("!txt!", NULL);
	CreateDirectory("syslog", NULL);

	InitLog(SERVERTITLE, LOGFILE, g_tStartServerTime);
	LOGMSG("\n\n\n"
		"============================================================\n"
		"== %s Start server time is %s\n"
		"============================================================\n\n",
		SERVERTITLE, bufStart );
	LOGSERVER("\n\n\n"
		"============================================================\n"
		"== %s Start server time is %s\n"
		"============================================================\n\n",
		SERVERTITLE, bufStart );
	LOGACCOUNT("\n\n\n"
		"============================================================\n"
		"== %s Start server time is %s\n"
		"============================================================\n\n",
		SERVERTITLE, bufStart );
	LOGPOINT("\n\n\n"
		"============================================================\n"
		"== %s Start server time is %s\n"
		"============================================================\n\n",
		SERVERTITLE, bufStart );

	LOGMSG("\n**************************************************\n"
		"**            Account Server started            **\n"
		"**************************************************\n");

	if(!GetConfig())	// װ�������ļ�
	{
		LOGERROR("�޷���ȡconfig.ini�ļ��������˳�");
		return true;
	}
	LOGMSG("��ȡINI���!");

	// reset title
	if(strlen(SERVER_TITLE))
		SetWindowText(NULL,SERVER_TITLE);
	/*
	// ��ʼ������
	WSADATA		wsaData;
	int	err;
	if((err = WSAStartup(0x0002, &wsaData)) != 0)
	{
	::MessageBox(NULL, "Init WSAStartup() failed.", "Error",MB_OK|MB_ICONERROR);
	return true;
	}
	// ���汾
	if(wsaData.wVersion != 0x0002)
	{
	WSACleanup();
	::MessageBox(NULL, "WSAStartup Version not match 2.0", "Error",MB_OK|MB_ICONERROR);
	return true;
	}
	//*/
	// init db
	if(g_db.Init(DBHOSTNAME, DBUSER, DBPASSWORD, DATABASENAME))	
	{
		/*
		g_hDbMutex	=::CreateMutex(NULL, false, "DBMutex");
		if (!g_hDbMutex)
		{
		LOGERROR("�޷��������ݿ��ڲ�������󣬳����˳�");
		::MessageBox(NULL, "g_hDbMutex����ʧ�ܡ�", "Error",MB_OK|MB_ICONERROR);
		return true;
		}
		//*/
		g_xDatabase	=::CreateMutex(NULL, false, "Database");
		if (!g_xDatabase)
		{
			LOGERROR("�޷��������ݿ⻥����󣬳����˳�");
			printf("Error:g_xDatabase����ʧ��!\n");
			getch();
			return true;
		}
	}
	else
	{
		LOGERROR("�޷������ݿ�");
		return true;
	}
	LOGMSG("��ʼ���������!");

	char szSQL[256];
	sprintf(szSQL, "select * from %s", POINTTABLE);
	IRecordset* pRes = g_db.GetInterface()->CreateNewRecordset(szSQL);	
	if(!pRes)
	{
		LOGERROR("��������ҵ�����");
		return true;
	}
	pRes->Release();

	g_pOnlineTable = new COnlineTable(ONLINETABLESIZE);
	LOGMSG("OnlineTable ���󴴽��ɹ�!");
	g_pPointThread = new CPointThread(POINTLISTENPORT, POINTSOCKETSNDBUF);
	LOGMSG("PointThread ���󴴽��ɹ�!");
	g_pLoginThread = new CLoginThread(LOGINLISTENPORT);
	LOGMSG("LoginThread ���󴴽��ɹ�!");
	g_pTimerThread = new CTimerThread();
	LOGMSG("TimerThread ���󴴽��ɹ�!");

	int err2 = 0;
	err2 += !g_pPointThread->CreateThread(false);
	LOGMSG("PointThread �̴߳������!");
	err2 += !g_pLoginThread->CreateThread(false);
	LOGMSG("LoginThread �̴߳������!");
	err2 += !g_pTimerThread->CreateThread(false);
	LOGMSG("TimerThread �̴߳������!");

	// GetServerAccount();

	if(!err2)
	{
		err2 += !g_pPointThread->ResumeThread();		// �������Ƶ��߳�
		err2 += !g_pLoginThread->ResumeThread();
		err2 += !g_pTimerThread->ResumeThread();
		if(err2)
		{
				LOGERROR("ResumeThread()�̳߳��������޷�����");
			return true;
		}
	}
	else
	{
		LOGERROR("�޷��������̣߳������˳�");
		return true;
	}
	LOGMSG("������ϣ���������������...");

	if(g_bEnableLogin)
	{
		PrintText("����������������...");
	}
	else
	{
		PrintText("��ͣ��ҵ�¼ %d ��, �ȴ���Ϸ������ͬ�� . . .", ENABLELOGINDELAY);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void OnClose()
{
	static bool	bFlag = false;
	if(!bFlag)	// ����һ��
	{
		g_bEnableLogin = false;		// ֹͣ��¼
		bFlag = true;
		LOGMSG("��������ʼ�˳�...");
	}
	if(g_pTimerThread->CloseThread(200))		// �ȹر�
	{
		if(g_pLoginThread->CloseThread(200))		// �ȹر�
		{
			if(g_pPointThread->CloseThread(200))
			{
				// �ͷ�ȫ�ֱ���
				delete	g_pLoginThread;
				delete	g_pPointThread;
				delete	g_pOnlineTable;

				// �ر����ݿ�
				g_db.Destroy();
				/*
				g_db.Close();
				if (g_hDbMutex)
				{
				::CloseHandle(g_hDbMutex);
				g_hDbMutex	=NULL;
				}
				//*/
				if (g_xDatabase)
				{
					::CloseHandle(g_xDatabase);
					g_xDatabase	=NULL;
				}
				LOGMSG("***************************************************************");
				LOGMSG("**  �����������ر�");
				LOGMSG("***************************************************************\n\n\n");
				return;
			}
		}
	}
	Sleep(500);
}

extern "C"
{
	WINBASEAPI HWND WINAPI GetConsoleWindow();
}

int main(int argc, char *argv[])   //���߳����н����������߳�Ҳ������
{

	//return 0; 
	//HWND hwnd;
	//HDC hdc;
	//printf("There are some words in console window!\n�ڿ���̨�����л�ͼ!\n");
	//system("Color 3D");
	//hwnd = GetConsoleWindow();
	//hdc = GetDC(hwnd);
	//LineTo(hdc, 200, 300);
	//Rectangle(hdc, 10, 30, 300, 50);
	//TextOut(hdc, 10, 10, _TEXT("Hello World\nYesNoConcel!"), 20);
	//ReleaseDC(hwnd, hdc);
	OnInitDialog();
	for(;;)
	{
		std::string strCmd;
		std::cin>>strCmd;   
		if  (strCmd =="exit") 
		{
			break;
		}
		if  (strCmd =="login") 
		{
			if(g_bEnableLogin)
			{
				std::cout<<"Login was already opened!"<<endl;
			}
			else
			{
				g_bEnableLogin=true;
				std::cout<<getCurrTimeString()<<"Login opening!"<<endl;
			}
		}
		else
		{
			std::cout<<"Can not understand this cmd: "<<strCmd<<endl;
		}
	}
	OnClose();
	std::system("pause");

	return 0;
}