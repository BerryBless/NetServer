/*
1, WaitForMultipleObjects 를 사용해서 여러개의 스레드 종료 대기.

2.
- 전역변수 -

int g_Data = 0;
int g_Connect = 0;
bool g_Shutdown = false;


g_Connect 는 접속자 수를 가상으로 표현.
g_Data 는 가상의 데이터 처리를 표현.


# AcceptThread

100 ~ 1000ms 마다 랜덤하게 g_Connect 를 + 1

# DisconnectThread

100 ~ 1000ms 마다 랜덤하게 g_Connect 를 - 1


# UpdateThread x 3

10ms 마다 g_Data 를 + 1

그리고 g_Data 가 1000 단위가 될 때 마다 이를 출력


# main

1초마다 g_Connect 를 출력
20초 후 g_Shutdown = true 로 다른 스레드들 종료.

스레드 종료 확인 후 main 종료.



+ beginthreadex 를 사용

+ 모든 변수 및 출력은 동기화가 이루어지도록 안전하게 되어야 함.
+ interlocked 로 해결되는 경우는 interlocked 사용.
+ interlocked 로 해결이 안되는 경우는 critical_section 사용.
+ critical_section 사용 후 srw 으로 변경 테스트
+ 모든 스레드는 할일이 없는경우 block 상태로 쉬도록 합니다.

+ 모든 스레드는 g_Shutdown 이 true 가 되면 종료됨.

+ main 스레드는 모든 스레드가 종료 된 후에 종료처리.

+ 메인에서 스레드의 종료 확인은 WaitForMultipleObjects 사용.
*/

#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>

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

	return 0;
}



unsigned int __stdcall AcceptThread(PVOID pData) {
	wprintf_s(L"[ID : %d] AcceptThread Start.\n", GetCurrentThreadId());
	int cnt = 0;
	int rSleep;
	// srand 값넣기
	srand(456);
	while (!g_Shutdown) {
		cnt++;

		// 100 ~ 1000ms 마다 랜덤하게
		rSleep = (rand() % 901) + 100;
		//wprintf_s(L"AcceptThread rSleep [%d]\n", rSleep);

		//g_Connect++;
		InterlockedIncrement((LONG *) &g_Connect);

		Sleep(rSleep);
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
		cnt++;

		// 100 ~ 1000ms 마다 랜덤하게
		rSleep = (rand() % 901) + 100;
		//wprintf_s(L"DisconnectThread rSleep [%d]\n", rSleep);

		//g_Connect--;
		InterlockedDecrement((LONG *) &g_Connect);

		Sleep(rSleep);
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
		cnt++;
#pragma region CriticalSection
#ifdef dfINTERLOCK
		// 인터락으로 해결!
		int ret = InterlockedIncrement((LONG *) &g_Data);
		if (ret % 1000 == 0) {
			wprintf_s(L"//g_Data\t[%d]\n", ret);
	}
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
	}

	wprintf_s(L"[ID : %d] UpdateThread loop cnt [%d]\n", GetCurrentThreadId(), cnt);

	return 0;
}
