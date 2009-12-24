#include "TimerThread.h"
#include <iostream>
#include <windows.h>

const char* getCurrTimeString()
{
	static char szCurrTime[60];
	FormatDateTime(szCurrTime, "[%04d-%02d-%02d %02d:%02d:%02d]: ", time(NULL));		// szFormat: 
	return szCurrTime;
}

int main(int argc, char *argv[])   //���߳����н����������߳�Ҳ������
{
	CTimerThread timerThread;
	std::cout<<getCurrTimeString()<<"TimerThread ���󴴽��ɹ�!"<<endl;

	if (timerThread.CreateThread(false))
	{
		std::cout<<getCurrTimeString()<<"TimerThread �̴߳������!"<<endl;
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