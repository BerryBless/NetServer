#pragma once
// lib
#pragma comment(lib,"WS2_32")
#pragma comment(lib, "winmm.lib" )

// common
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <conio.h>
#include <time.h>

// STL
#include <xstring>
#include <queue>
#include <list>
#include <map>
#include <unordered_map>

// 글로벌 디파인, 이넘
#include "GDefine.h" 
#include "Logger.h"
#include "CObjectPool.h"

// network
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <timeapi.h>
#include <Windows.h>


extern bool g_bShutdown;