#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include "Profiler.h"
#include "CCrashDump.h"
#include "ObjectPool.hpp"
#include "ObjectPool_TLS.h"

#include <time.h>
#include <strsafe.h>
#include <locale.h>

#define dfTEST_THREAD_POOLING_CNT 10000
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
#else
CObjectPool<TEST_NODE> g_pool;
#endif // dfTLSPOOL

unsigned int __stdcall PoolingTestThread(LPVOID arg);

#define dfNEW_DELETE 0
#define dfPOOL 1

int main() {
	int threadCnt;
	printf_s("num of thread >> ");
	scanf_s("%d", &threadCnt);
	PRO_INIT(threadCnt * 2);
	CLogger::Initialize();
	CLogger::SetDirectory(L"LOG");
	HANDLE *hThreads = new HANDLE[threadCnt * 2];
	TEST_NODE **g_ltestnodes = new TEST_NODE * [dfTEST_THREAD_POOLING_CNT * threadCnt];

	for (int i = 0; i < dfTEST_THREAD_POOLING_CNT * threadCnt; i++) {
		g_ltestnodes[i] = g_pool.Alloc();
	}


	for (int i = 0; i < dfTEST_THREAD_POOLING_CNT * threadCnt; i++) {
		g_pool.Free(g_ltestnodes[i]);
	}
	//CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Capacity[%d] UseCount[%d]\n", g_lfpool.GetCapacity(), g_lfpool.GetSize());



	for (int i = 0; i < threadCnt; i++) {
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, PoolingTestThread, dfNEW_DELETE, 0, nullptr);
	}
	for (int i = 0; i < threadCnt; i++) {
		hThreads[i + threadCnt] = (HANDLE)_beginthreadex(nullptr, 0, PoolingTestThread, (LPVOID)dfPOOL, 0, nullptr);
	}

	WCHAR FILENAME[50] = L"";
	// timestemp
	tm t;
	time_t now;

	int printTick = 0;
	while (true) {
		Sleep(1000);
		CLogger::_Log(dfLOG_LEVEL_NOTICE, L"\nThreadCount [%d]\tCapacity[%d] UseCount[%d]", threadCnt,g_pool.GetCapacity(), g_pool.GetSize());

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
			case dfPOOL:
				PRO_BEGIN(L"POOL_ALLOC");
				testnodes[i] = g_pool.Alloc();
				PRO_END(L"POOL_ALLOC");
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
			case dfPOOL:
				PRO_BEGIN(L"POOL_FREE");
				g_pool.Free(testnodes[i]);
				PRO_END(L"POOL_FREE");
				break;
			default:
				CRASH();
				break;
			}

		}

		int spin = 0;
		while (spin < 300) {
			YieldProcessor();
			++spin;
		}
		//if (g_lfpool.GetSize() == 0) {
		//	if (g_lfpool.GetCapacity() != dfTEST_MAX_POOLING_NODE)
		//		CRASH();
		//}
	}

	return 0;
}
