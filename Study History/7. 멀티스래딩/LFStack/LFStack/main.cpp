#include "LFStack.hpp"
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include "CCrashDump.h"

#define dfTEST_THREAD 16
#define dfTEST_THREAD_POOLING_CNT 20000
#define dfTEST_MAX_POOLING_NODE (dfTEST_THREAD * dfTEST_THREAD_POOLING_CNT)

struct TEST_NODE {
	UINT64 _data;
	UINT64 _check;
};


LFStack<TEST_NODE> g_lfstack;

unsigned int __stdcall TestThread(LPVOID arg);
bool g_run = true;
int main() {
	CLogger::Initialize();
	CLogger::SetDirectory(L"LOG");
	CLogger::SetLogLevel(dfLOG_LEVEL_ERROR);

	for (int i = 0; i < dfTEST_MAX_POOLING_NODE; i++) {
		TEST_NODE temp;
		temp._data = 0x12341234ABCDABCD;
		temp._check = 0;
		g_lfstack.Push(temp);
	}

	HANDLE hThreads[dfTEST_THREAD];

	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Poolsize[%d], PoolUseNode[%d], StackSize[%d]", g_lfstack.GetPoolSize(), g_lfstack.GetPoolUse(), g_lfstack.GetSize());

	for (int i = 0; i < dfTEST_THREAD; i++) {
		hThreads[i] = (HANDLE) _beginthreadex(nullptr, 0, TestThread, 0, 0, nullptr);
	}

	while (true) {
		Sleep(1000);
		CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Poolsize[%d], PoolUseNode[%d], StackSize[%d]", g_lfstack.GetPoolSize(), g_lfstack.GetPoolUse(), g_lfstack.GetSize());
	}

	return 0;
}

unsigned int __stdcall TestThread(LPVOID arg) {
	int id = GetCurrentThreadId();
	TEST_NODE testNode[dfTEST_THREAD_POOLING_CNT];
	printf_s("THREAD start : ID[%d]\n", id);
	while (g_run) {

		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++)
			if(g_lfstack.Pop(&testNode[i]) == false) CRASH() ;

		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			testNode[i]._check += id;
		}

		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			if (testNode[i]._check != id)
				CRASH();
		}
		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++) {
			testNode[i]._check = 0;
		}
		for (int i = 0; i < dfTEST_THREAD_POOLING_CNT; i++)
			g_lfstack.Push(testNode[i]);
	}

	return 0;
}
