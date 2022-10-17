#include "CObjectPool.hpp"
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include "Profiler.h"
#include "CCrashDump.h"
#include "CObjectPool_TLS.h"

#include <time.h>
#include <strsafe.h>
#include <locale.h>

#define dfTEST_THREAD 4
#define dfTEST_THREAD_POOLING_CNT 10000
#define dfTEST_MAX_POOLING_NODE (dfTEST_THREAD * dfTEST_THREAD_POOLING_CNT)
#define dfTLSPOOL


class TEST_NODE {
public:
	TEST_NODE()
	{
		_data = 11;
	}
	UINT64 _data;
	UINT64 _check;

};

#ifdef dfTLSPOOL
ObjectPool_TLS<TEST_NODE> g_pool(true);
LF_ObjectPool_TLS<TEST_NODE> g_lfpool(true);
#else
CObjectPool<TEST_NODE> g_pool;
CLF_ObjectPool<TEST_NODE> g_lfpool;
#endif // dfTLSPOOL

unsigned int __stdcall PoolingTestThread(LPVOID arg);
TEST_NODE* g_ltestnodes[dfTEST_MAX_POOLING_NODE] = { 0 };
TEST_NODE* g_lftestnodes[dfTEST_MAX_POOLING_NODE] = { 0 };

#define dfNEW_DELETE 0
#define dfLOCKED_POOL 1
#define dfLOCK_FREE_POOL 2

int main() {
	PRO_INIT(dfTEST_THREAD * 3);
	CLogger::Initialize();
	CLogger::SetDirectory(L"LOG");

	HANDLE hThreads[dfTEST_THREAD * 3];


	for (int i = 0; i < dfTEST_MAX_POOLING_NODE; i++) {
		g_ltestnodes[i] = g_pool.Alloc();
		g_lftestnodes[i] = g_lfpool.Alloc();
	}


	for (int i = 0; i < dfTEST_MAX_POOLING_NODE; i++) {
		g_pool.Free(g_ltestnodes[i]);
		g_lfpool.Free(g_lftestnodes[i]);
	}
	//CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Capacity[%d] UseCount[%d]\n", g_lfpool.GetCapacity(), g_lfpool.GetSize());



	for (int i = 0; i < dfTEST_THREAD; i++) {
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, PoolingTestThread, dfNEW_DELETE, 0, nullptr);
	}
	for (int i = 0; i < dfTEST_THREAD; i++) {
		hThreads[i + dfTEST_THREAD] = (HANDLE)_beginthreadex(nullptr, 0, PoolingTestThread, (LPVOID)dfLOCKED_POOL, 0, nullptr);
	}
	for (int i = 0; i < dfTEST_THREAD; i++) {
		hThreads[i + (dfTEST_THREAD * 2)] = (HANDLE)_beginthreadex(nullptr, 0, PoolingTestThread, (LPVOID)dfLOCK_FREE_POOL, 0, nullptr);
	}

	WCHAR FILENAME[50] = L"";
	// timestemp
	tm t;
	time_t now;

	int printTick = 0;
	while (true) {
		Sleep(1000);
		CLogger::_Log(dfLOG_LEVEL_NOTICE, L"\nL\t:\tCapacity[%d] UseCount[%d]\nLF\t:\tCapacity[%d] UseCount[%d]\n", g_pool.GetCapacity(), g_pool.GetSize(),g_lfpool.GetCapacity(), g_lfpool.GetSize());

		if (printTick >= 60) {
			// timestemp
			time(&now);
			localtime_s(&t, &now);

			swprintf_s(FILENAME, 50, L"LOG/%02d%02d%02d_%02d%02d%02d_PROFILE.log",
				t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
				t.tm_hour, t.tm_min, t.tm_sec);
			PRO_PRINT(FILENAME);
			printTick = 0;
		}
		printTick++;
	}
	return 0;
}

unsigned int __stdcall PoolingTestThread(LPVOID arg) {
	int tCase = ((int)arg);
	INT64 loopcnt = 0;
	DWORD ID = GetCurrentThreadId();
	DWORD tID = GetCurrentThreadId();
	TEST_NODE* testnodes[dfTEST_THREAD_POOLING_CNT] = { 0 };
	while (true) {
		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			switch (tCase)
			{
			case dfNEW_DELETE:
				PRO_BEGIN(L"NEW");
				testnodes[i] = new TEST_NODE;
				PRO_END(L"NEW");
				break;
			case dfLOCKED_POOL:
				PRO_BEGIN(L"POOL_ALLOC");
				testnodes[i] = g_pool.Alloc();
				PRO_END(L"POOL_ALLOC");
				break;
			case dfLOCK_FREE_POOL:
				PRO_BEGIN(L"LFPOOL_ALLOC");
				testnodes[i] = g_lfpool.Alloc();
				PRO_END(L"LFPOOL_ALLOC");
				break;
			default:
				CRASH();
				break;
			}


			testnodes[i]->_data = ID;
			testnodes[i]->_check = 10000;
		}

		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			if (testnodes[i]->_data != ID)
				CRASH();
			testnodes[i]->_check += ID;
		}

		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			if (testnodes[i]->_check != (10000 + ID))
				CRASH();
		}

		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {

			switch (tCase)
			{
			case dfNEW_DELETE:
				PRO_BEGIN(L"DELETE");
				delete testnodes[i];
				PRO_END(L"DELETE");
				break;
			case dfLOCKED_POOL:
				PRO_BEGIN(L"POOL_FREE");
				g_pool.Free(testnodes[i]);
				PRO_END(L"POOL_FREE");
				break;
			case dfLOCK_FREE_POOL:
				PRO_BEGIN(L"LFPOOL_FREE");
				g_lfpool.Free(testnodes[i]);
				PRO_END(L"LFPOOL_FREE");
				break;
			default:
				CRASH();
				break;
			}

		}

		int spin = 0;
		while (spin < 300) {
			Sleep(0);
			YieldProcessor();
			++spin;
		}
		//if (g_lfpool.GetSize() == 0) {
		//	//printf_s("/////////USE COUNT is 0\n");
		//	if (g_lfpool.GetCapacity() != dfTEST_MAX_POOLING_NODE)
		//		CRASH();
		//}
	}

	return 0;
}
