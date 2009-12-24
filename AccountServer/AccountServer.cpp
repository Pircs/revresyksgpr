#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <tchar.h>

#include "allheads.h"

void SetColor(unsigned short ForeColor=7,unsigned short BackGroundColor=0) 
{ 
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE); 
	SetConsoleTextAttribute(hCon,ForeColor|BackGroundColor); 
};
#include <iostream>
#include <windows.h>

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

	LOGMSG(		"\n\n\n================================================================================\n"
		"=== %s\n"
		"=== Start server time is %s\n"
		"================================================================================\n\n"
		, SERVERTITLE
		, bufStart );
	LOGSERVER(	"\n\n\n================================================================================\n"
		"=== %s\n"
		"=== Start server time is %s\n"
		"================================================================================\n\n"
		, SERVERTITLE
		, bufStart );
	LOGACCOUNT(	"\n\n\n================================================================================\n"
		"=== %s\n"
		"=== Start server time is %s\n"
		"================================================================================\n\n"
		, SERVERTITLE
		, bufStart );
	LOGPOINT(	"\n\n\n================================================================================\n"
		"=== %s\n"
		"=== Start server time is %s\n"
		"================================================================================\n\n"
		, SERVERTITLE
		, bufStart );

	LOGMSG("***************************************************************");
	LOGMSG("** 帐号服务器开始启动");
	LOGMSG("***************************************************************");

	SetColor(FOREGROUND_GREEN); 
	std::cout<<"==============================================================================="<<endl;
	std::cout<<"=== "<<SERVERTITLE<<endl;
	std::cout<<"=== Start server time is ";
	SetColor(); 
	std::cout<<bufStart<<endl;
	SetColor(FOREGROUND_GREEN); 
	std::cout<<"==============================================================================="<<endl;
	SetColor(FOREGROUND_RED,BACKGROUND_BLUE); 
	std::cout<<"***************************************************************"<<endl;
	std::cout<<"**                  Account Server started                   **"<<endl;
	std::cout<<"***************************************************************"<<endl;
	

	if(!GetConfig())	// 装入配置文件
	{
		LOGERROR("无法读取config.ini文件，程序退出");
		SetColor(FOREGROUND_RED); 
		std::cout<<getCurrTimeString()<<"无法读取config.ini文件"<<endl;
		return true;
	}
	LOGMSG("读取INI完毕!");
	SetColor(FOREGROUND_GREEN); 
	std::cout<<getCurrTimeString()<<"读取INI完毕!"<<endl;

	// reset title
	if(strlen(SERVER_TITLE))
		SetWindowText(NULL,SERVER_TITLE);
	std::cout<<getCurrTimeString()<<SERVER_TITLE<<endl;
	/*
	// 初始化网络
	WSADATA		wsaData;
	int	err;
	if((err = WSAStartup(0x0002, &wsaData)) != 0)
	{
	::MessageBox(NULL, "Init WSAStartup() failed.", "Error",MB_OK|MB_ICONERROR);
	return true;
	}
	// 检查版本
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
		LOGERROR("无法创建数据库内部互斥对象，程序退出");
		::MessageBox(NULL, "g_hDbMutex产生失败。", "Error",MB_OK|MB_ICONERROR);
		return true;
		}
		//*/
		g_xDatabase	=::CreateMutex(NULL, false, "Database");
		if (!g_xDatabase)
		{
			LOGERROR("无法创建数据库互斥对象，程序退出");
			printf("Error:g_xDatabase产生失败!\n");
			getch();
			return true;
		}
	}
	else
	{
		LOGERROR("无法打开数据库");
		printf("Error:无法打开数据库!\n");
		getch();
		return true;
	}
	LOGMSG("初始化核心完毕!");

	char szSQL[256];
	sprintf(szSQL, "select * from %s", POINTTABLE);
	IRecordset* pRes = g_db.GetInterface()->CreateNewRecordset(szSQL);	
	if(!pRes)
	{
		LOGERROR("不存在玩家点数表");
		::MessageBox(NULL, "无法打开指定的玩家点数表", "Error",MB_OK|MB_ICONERROR);
		return true;
	}
	pRes->Release();

	g_pOnlineTable = new COnlineTable(ONLINETABLESIZE);
	SetColor(FOREGROUND_GREEN); 
	std::cout<<getCurrTimeString()<<"OnlineTable 对象创建成功!"<<endl;
	LOGMSG("OnlineTable 对象创建成功!");
	g_pPointThread = new CPointThread(POINTLISTENPORT, POINTSOCKETSNDBUF);
	std::cout<<getCurrTimeString()<<"PointThread 对象创建成功!"<<endl;
	LOGMSG("PointThread 对象创建成功!");
	g_pLoginThread = new CLoginThread(LOGINLISTENPORT);
	std::cout<<getCurrTimeString()<<"LoginThread 对象创建成功!"<<endl;
	LOGMSG("LoginThread 对象创建成功!");

	g_pTimerThread = new CTimerThread();
	std::cout<<getCurrTimeString()<<"TimerThread 对象创建成功!"<<endl;

	int err2 = 0;
	err2 += !g_pPointThread->CreateThread(false);
	std::cout<<getCurrTimeString()<<"PointThread 线程创建完毕!"<<endl;
	LOGMSG("PointThread 线程创建完毕!");
	err2 += !g_pLoginThread->CreateThread(false);
	std::cout<<getCurrTimeString()<<"LoginThread 线程创建完毕!"<<endl;
	LOGMSG("LoginThread 线程创建完毕!");

	err2 += !g_pTimerThread->CreateThread(false);
	std::cout<<getCurrTimeString()<<"TimerThread 线程创建完毕!"<<endl;
	LOGMSG("TimerThread 线程创建完毕!");

	// GetServerAccount();

	if(!err2)
	{
		err2 += !g_pPointThread->ResumeThread();		// 先启动计点线程
		err2 += !g_pLoginThread->ResumeThread();
		err2 += !g_pTimerThread->ResumeThread();
		if(err2)
		{
			LOGERROR("ResumeThread()线程出错。程序无法启动");
			::MessageBox(NULL, "启动子线程出错。","",0);
			return true;
		}
	}
	else
	{
		LOGERROR("无法启动子线程，程序退出");
		::MessageBox(NULL, "无法启动子线程","",0);
		return true;
	}

	//SetTimer(NULL,1, 1000, NULL);
	//LOGMSG("SetTimer 启动完毕!");
	//std::cout<<getCurrTimeString()<<"SetTimer 启动完毕!"<<endl;

	LOGMSG("启动完毕，程序正常运行中-------------------------------------");
	std::cout<<getCurrTimeString()<<"启动完毕，程序正常运行中-------------------------------------"<<endl;

	if(g_bEnableLogin)
	{
		PrintText("服务器正常运行中 . . .");
		std::cout<<getCurrTimeString()<<"服务器正常运行中 . . ."<<endl;
	}
	else
	{
		PrintText("暂停玩家登录 %d 秒, 等待游戏服务器同步 . . .", ENABLELOGINDELAY);
		std::cout<<getCurrTimeString()<<"暂停玩家登录 "<<ENABLELOGINDELAY<<" 秒, 等待游戏服务器同步 . . ."<<endl;
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	//////////////////////////////////////////////////////////////////////////////////////////////////////

}

void OnEnableLogin() 
{
	g_bEnableLogin = true;
}

void OnClose()
{
	static bool	bFlag = false;
	if(!bFlag)	// 运行一次
	{
		g_bEnableLogin = false;		// 停止登录
		bFlag = true;
		LOGMSG("服务器开始退出----------------");
		std::cout<<getCurrTimeString()<<"服务器退出中 . . ."<<endl;
	}
	if(g_pTimerThread->CloseThread(200))		// 先关闭
	{
		if(g_pLoginThread->CloseThread(200))		// 先关闭
		{
			if(g_pPointThread->CloseThread(200))
			{
				// 释放全局变量
				delete	g_pLoginThread;
				delete	g_pPointThread;
				delete	g_pOnlineTable;

				// 关闭数据库
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
				LOGMSG("**  服务器正常关闭");
				LOGMSG("***************************************************************\n\n\n");

				std::cout<<"***************************************************************"<<endl;
				std::cout<<"**  服务器正常关闭"<<endl;
				std::cout<<"***************************************************************\n\n\n"<<endl;
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



int main(int argc, char *argv[])   //主线程运行结束，辅助线程也结束。
{

	//return 0; 
	//HWND hwnd;
	//HDC hdc;
	//printf("There are some words in console window!\n在控制台窗口中绘图!\n");
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