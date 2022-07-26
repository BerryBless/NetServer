#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment (lib, "winmm")

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
#include "CRingBuffer.h"
#include "CLogger.h"
#include "CObjectPool.hpp"
#include "CCrashDump.h"
#include "Stack.hpp"
#include "Queue.hpp"
#include "HardWareMoniter.h"
#include "ProcessMoniter.h"
#include "CPacket.h"
using namespace std;



#define CRASH() do{\
					CLogger::_Log(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
					CCrashDump::Crash();\
				}while(0)




void HelloWorld();