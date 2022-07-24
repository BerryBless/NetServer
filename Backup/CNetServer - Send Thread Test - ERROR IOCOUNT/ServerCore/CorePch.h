#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment (lib, "winmm")

#include "Types.h"
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <locale.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <timeapi.h>
#include "CLogger.h"
#include "CCrashDump.h"
using namespace std;



#define CRASH() do{\
					CLogger::_Log(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
					CCrashDump::Crash();\
				}while(0)




void HelloWorld();