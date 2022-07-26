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

unsigned int __stdcall TestThread(LPVOID arg) {
	int id = GetCurrentThreadId();
	int cnt = 0;
	int var1;
	int var2;
	int var3;
	int var4;
	int var5;

	int i =0;
	long size[dfMAX_LOGING_COUNT] = {0};

	printf_s("THREAD start : ID[%d]\n", id);
	while (g_run) {
		var1 = var2 = var3 = var4 = var5 = 0;
		g_lfstack.Push(id | 0x11110000);
		g_lfstack.Push(id | 0x22220000);
		g_lfstack.Push(id | 0x33330000);
		g_lfstack.Push(id | 0x44440000);
		g_lfstack.Push(id | 0x55550000);

		if (!g_lfstack.Pop(&var1))
			CRASH();
		if (!g_lfstack.Pop(&var2))
			CRASH();
		if (!g_lfstack.Pop(&var3))
			CRASH();
		if (!g_lfstack.Pop(&var4))
			CRASH();
		if (!g_lfstack.Pop(&var5))
			CRASH();
		/*if (!g_lfstack.isEmpty())
			CRASH();*/

		i = (i + 1) % dfMAX_LOGING_COUNT;
		size[i] = g_lfstack.GetSize();

		//printf_s("%d : [%d] [%d] [%d] [%d] [%d]\n", cnt, var1, var2, var3, var4, var5);
	}
	return 0;
}
