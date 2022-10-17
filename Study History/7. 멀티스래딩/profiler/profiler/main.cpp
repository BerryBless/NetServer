
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include "Profiler.h"

int g_Data = 0;
int g_Connect = 0;
bool g_Shutdown = false;

CRITICAL_SECTION g_updateLock;
void Lock() { EnterCriticalSection(&g_updateLock); }
void Unlock() { LeaveCriticalSection(&g_updateLock); }

SRWLOCK g_srwlock;


unsigned int __stdcall AcceptThread(PVOID pData);
unsigned int __stdcall DisconnectThread(PVOID pData);
unsigned int __stdcall UpdateThread(PVOID pData);


int main() {
	PRO_INIT();

	//----------------------- 
	// 스레드 핸들
	// 	   0 : AcceptThread
	// 	   1 : DisconnectThread
	// 	   2 : UpdateThread
	// 	   3 : UpdateThread
	// 	   4 : UpdateThread
	//----------------------- 
	HANDLE hThread[5];
	clock_t startTick;	// 20초후 종료
	clock_t curTick;	// 현재시간

	//----------------------- 
	// 스레드 등록
	//----------------------- 
	hThread[0] = (HANDLE) _beginthreadex(NULL, 0, &AcceptThread, 0, 0, NULL);
	hThread[1] = (HANDLE) _beginthreadex(NULL, 0, &DisconnectThread, 0, 0, NULL);
	hThread[2] = (HANDLE) _beginthreadex(NULL, 0, &UpdateThread, 0, 0, NULL);
	hThread[3] = (HANDLE) _beginthreadex(NULL, 0, &UpdateThread, 0, 0, NULL);
	hThread[4] = (HANDLE) _beginthreadex(NULL, 0, &UpdateThread, 0, 0, NULL);
	wprintf_s(L"Created Thread!\n");

	InitializeCriticalSection(&g_updateLock);
	// SRWlock 초기화
	InitializeSRWLock(&g_srwlock);

#pragma region MainLoop
	wprintf_s(L"Entering the main loop..\n");
	startTick = clock();
	curTick = startTick;
	while (curTick - startTick < 20000) {
		curTick = clock();
		wprintf_s(L"g_Connect\t[%d]\n", g_Connect);
		Sleep(1000);
		PRO_PRINT(L"log.log");
	}
#pragma endregion

	// 종료
	g_Shutdown = true;
	wprintf_s(L"WaitForMultipleObjects ...\n");
	WaitForMultipleObjects(5, hThread, TRUE, INFINITE);
	wprintf_s(L"WaitForMultipleObjects Doen!\n");

	// 크리티컬 섹션 삭제
	DeleteCriticalSection(&g_updateLock);

	wprintf_s(L"g_Connect [%d], g_Data [%d]", g_Connect, g_Data);

	PRO_PRINT(L"profile.log");
	return 0;
}



unsigned int __stdcall AcceptThread(PVOID pData) {
	wprintf_s(L"[ID : %d] AcceptThread Start.\n", GetCurrentThreadId());
	int cnt = 0;
	int rSleep;
	// srand 값넣기
	srand(456);
	while (!g_Shutdown) {
		PRO_BEGIN(L"ACCEPT");
		cnt++;

		// 100 ~ 1000ms 마다 랜덤하게
		rSleep = (rand() % 901) + 100;
		//wprintf_s(L"AcceptThread rSleep [%d]\n", rSleep);

		//g_Connect++;
		InterlockedIncrement((LONG *) &g_Connect);

		Sleep(rSleep);
		PRO_END(L"ACCEPT");
	}
	wprintf_s(L"[ID : %d] AcceptThread loop cnt [%d]\n", GetCurrentThreadId(), cnt);

	return 0;
}
unsigned int __stdcall DisconnectThread(PVOID pData) {
	wprintf_s(L"[ID : %d] DisconnectThread Start.\n", GetCurrentThreadId());
	int cnt = 0;
	int rSleep;

	// srand 값넣기
	srand(123);
	while (!g_Shutdown) {
		PRO_BEGIN(L"DISCONNECT");
		cnt++;

		// 100 ~ 1000ms 마다 랜덤하게
		rSleep = (rand() % 901) + 100;
		//wprintf_s(L"DisconnectThread rSleep [%d]\n", rSleep);

		//g_Connect--;
		InterlockedDecrement((LONG *) &g_Connect);

		Sleep(rSleep);
		PRO_END(L"DISCONNECT");
	}
	wprintf_s(L"[ID : %d] DisconnectThread loop cnt [%d]\n", GetCurrentThreadId(), cnt);
	return 0;
}
#define dfINTERLOCK
//#define dfSRWLOCK
//#define dfCRITCALSECTION
unsigned int __stdcall UpdateThread(PVOID pData) {
	wprintf_s(L"[ID : %d] UpdateThread Start.\n", GetCurrentThreadId());
	int cnt = 0;
	while (!g_Shutdown) {
		PRO_BEGIN(L"UPDATE");
		cnt++;
#pragma region CriticalSection
#ifdef dfINTERLOCK
		// 인터락으로 해결!
		PRO_BEGIN(L"INTERLOCK");
		int ret = InterlockedIncrement((LONG *) &g_Data);
		if (ret % 1000 == 0) {
			wprintf_s(L"//g_Data\t[%d]\n", ret);
		}
		PRO_END(L"INTERLOCK");
#endif // dfINTERLOCK
#ifdef dfSRWLOCK
		// SRW
		AcquireSRWLockExclusive(&g_srwlock);
		g_Data++;
		ReleaseSRWLockExclusive(&g_srwlock);
		AcquireSRWLockShared(&g_srwlock);
		if (g_Data % 1000 == 0) {
			wprintf_s(L"//g_Data\t[%d]\n", g_Data);
		}
		ReleaseSRWLockShared(&g_srwlock);
#endif // dfSRWLOCK
#ifdef dfCRITCALSECTION
		// 크리티컬 섹션!
		Lock();
		g_Data++;
		if (g_Data % 1000 == 0) {
			wprintf_s(L"//g_Data\t[%d]\n", g_Data);
		}
		Unlock();
#endif  // dfCRITCALSECTION
#pragma endregion

		Sleep(10);
		PRO_END(L"UPDATE");
	}

	wprintf_s(L"[ID : %d] UpdateThread loop cnt [%d]\n", GetCurrentThreadId(), cnt);

	return 0;
}
