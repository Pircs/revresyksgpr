// 所有头文件
// 仙剑修，2001.11.19
#pragma once
#include <windows.h>
#include <assert.h>

#include "Typedef.h"
#ifdef	ENCRYPT
	#include "..\GameServer\ServerSocket\EncryptServer.h"
#endif
#include "IniFile.h"
#include "LogFile.h"
#include "ThreadBase.h"
#include "ServerSocket.h"
#include "ListenSocket.h"
#include "LoginThread.h"
#include "PointThread.h"
#include "OnlineTable.h"
#include "TimerThread.h"

#include "Account.h"

#include "Global.h"
#include "SharedBaseFunc.h"