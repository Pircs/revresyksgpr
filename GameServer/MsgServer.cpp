#include "TimerThread.h"
#include <iostream>
#include <windows.h>

const char* getCurrTimeString()
{
	static char szCurrTime[60];
	FormatDateTime(szCurrTime, "[%04d-%02d-%02d %02d:%02d:%02d]: ", time(NULL));		// szFormat: 
	return szCurrTime;
}

int main(int argc, char *argv[])   //主线程运行结束，辅助线程也结束。
{
	CTimerThread timerThread;
	std::cout<<getCurrTimeString()<<"TimerThread 对象创建成功!"<<endl;

	if (timerThread.CreateThread(false))
	{
		std::cout<<getCurrTimeString()<<"TimerThread 线程创建完毕!"<<endl;
	}
	timerThread.ResumeThread();

	for(;;)
	{
		std::string strCmd;
		std::cin>>strCmd;   
		if  (strCmd =="exit") 
		{
			break;
		}
		else
		{
			std::cout<<"Can not understand this cmd: "<<strCmd<<endl;
		}
	}
	//OnClose();
	std::system("pause");

	return 0;
}