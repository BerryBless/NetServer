
/*
프로젝트 설정
1. General -> OutputD Directory -> $(SolutionDir)Binary\$(Configuration)\
2. pch 적용
3. 최적화 컴파일 끄기
4. VC++ Directories
	Include Directories -> Edit -> New Line -> $(SolutionDir)ServerCore
	Library Directories -> Edit -> New Line -> $(SolutionDir)Libraries
*/



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
#include "RingBuffer.h"
#include "CLogger.h"
#include "ObjectPool.hpp"
#include "CCrashDump.h"
#include "Stack.hpp"
#include "Queue.hpp"
#include "HardWareMoniter.h"
#include "ProcessMoniter.h"
#include "SerializingBuffer.h"
#include "Types.h"
#include "CParser.h"
#include "DBConnectionPool.h"
//#include "MemProfiler.h"
using namespace std;



#ifndef CRASH
#define CRASH() do{\
					CLogger::_Log(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
					CCrashDump::Crash();\
				}while(0)
#endif // !CRASH

#ifndef ASSERT_CRASH
#define ASSERT_CRASH(expr)	do{\
								if (!(expr)) { CRASH();}\
							}while()
#endif // !ASSERT_CRASH


void HelloWorld();



