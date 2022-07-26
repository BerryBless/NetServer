#include <Windows.h>
#include <stdio.h>
#include <process.h>

#define dfTRY 100'000'000

#pragma region Peterson
int g_turn = 0;
bool g_flags[2] = {false, false};

void Lock(int myId) {
	int otherId = 1 - myId;

	g_flags[myId] = true;
	g_turn = myId;


	// ¿©±â¼­ ÆÒ-½º
	//_mm_mfence();
	//__faststorefence();

	//Sleep(0);
	//for (int i = 0; i <20; i++);
	//YieldProcessor();

	while (g_turn == myId && g_flags[otherId] ) {
		YieldProcessor();
	}
}

void Unlock(int myId) {
	g_flags[myId] = false;
}
#pragma endregion

int g_sum = 0;
unsigned __stdcall threadFunc(PVOID pData) {
	int threadId = (int) pData;
	for (int i = 0; i < dfTRY; ++i) {
		Lock(threadId);
		g_sum ++;
		Unlock(threadId);
	}
	return 0;
}

int main() {
	HANDLE hThread[2];
	int cnt = 0;
	while (true) {
		++cnt;
		hThread[0] = (HANDLE) _beginthreadex(NULL, 0, &threadFunc, (void *) 0, 0, NULL);
		hThread[1] = (HANDLE) _beginthreadex(NULL, 0, &threadFunc, (void *) 1, 0, NULL);

		WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

		CloseHandle(hThread[0]);
		CloseHandle(hThread[1]);

		printf_s("%d:%d\n", cnt,g_sum);
		g_sum = 0;
	}

	return 0;
}