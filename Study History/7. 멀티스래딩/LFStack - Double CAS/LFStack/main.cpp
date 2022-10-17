#include "LFStack.hpp"
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include "CCrashDump.h"

#define dfTEST_THREAD_COUNT 2

LFStack<int> g_lfstack;
unsigned int __stdcall TestThread(LPVOID arg);
bool g_run = true;
int main() {
	HANDLE hThreads[dfTEST_THREAD_COUNT];

	for (int i = 0; i < dfTEST_THREAD_COUNT; i++) {
		hThreads[i] = (HANDLE) _beginthreadex(nullptr, 0, TestThread, 0, 0, nullptr);
	}

	Sleep(INFINITE);

	return 0;
}
#define TESTCASE 100000

unsigned int __stdcall TestThread(LPVOID arg) {
	int id = GetCurrentThreadId();
	__int64 cnt = 0;

	printf_s("THREAD start : ID[%d]\n", id);
	while (g_run) {
		for (int i = 0; i < TESTCASE; i++)
			g_lfstack.Push(i);
		int t;
		for (int i = 0; i < TESTCASE; i++)
			if(g_lfstack.Pop(&t) == false) CRASH() ;

		if (cnt % 10 == 0) {
			printf_s("ID[%d] %lld loops clear\n", id, cnt);
		}
		cnt++;
	}

	return 0;
}
