#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <conio.h>
#include "CCrashDump.h"
#include "CLogger.h"
#include "LFQueue.hpp"
#include "Profiler.h"

#define dfTEST_THREAD 7
#define dfTEST_THREAD_POOLING_CNT 5
#define dfTEST_MAX_POOLING_NODE (dfTEST_THREAD * dfTEST_THREAD_POOLING_CNT)

struct TEST_NODE {
	UINT64 _data;
	UINT64 _check;
};


LFQueue<TEST_NODE> g_lfqueue;
long long g_cnt = 0;
unsigned int __stdcall TestThread(LPVOID arg);
bool g_run = true;
int main() {
	PRO_INIT(30);
	CLogger::Initialize();
	CLogger::SetDirectory(L"LOG");
	CLogger::SetLogLevel(dfLOG_LEVEL_ERROR);

	for (int i = 0; i < dfTEST_MAX_POOLING_NODE; i++) {
		TEST_NODE temp;
		temp._data = 0x12341234ABCDABCD;
		temp._check = 0;
		g_lfqueue.Enqueue(temp);
	}

	HANDLE hThreads[dfTEST_THREAD];

	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Queue size [%d]", g_lfqueue.GetSize());

	for (int i = 0; i < dfTEST_THREAD; i++) {
		hThreads[i] = (HANDLE) _beginthreadex(nullptr, 0, TestThread, 0, 0, nullptr);
	}

	while (true) {
		Sleep(1000);
		CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Queue size [%d]", g_lfqueue.GetSize());
		if (_kbhit()) {
			char cmd = _getch();
			PRO_PRINT(L"LFqueue.profile");
		}
	}

	PRO_DESTROY();
	return 0;
}

unsigned int __stdcall TestThread(LPVOID arg) {
	int id = GetCurrentThreadId();
	TEST_NODE testNode[dfTEST_THREAD_POOLING_CNT];
	printf_s("THREAD start : ID[%d]\n", id);
	while (g_run) {

		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			PRO_BEGIN(L"Dequeue");
			if (g_lfqueue.Dequeue(&testNode[i]) == false) CRASH();
			PRO_END(L"Dequeue");
		}
		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {

			testNode[i]._data = InterlockedIncrement64(&g_cnt);
			testNode[i]._check += id;
		}

		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			if (testNode[i]._check != id)
				CRASH();
		}
		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			testNode[i]._check = 0;
		}
		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			PRO_BEGIN(L"Enqueue");
			g_lfqueue.Enqueue(testNode[i]);
			PRO_END(L"Enqueue");
		}
	}

	return 0;
}
