#include "pch.h"
#include "CEchoClient.h"

#define dfDUMMY_CNT 5
 
extern LONG g_sendCalc;
extern LONG g_recvCalc;

static unsigned int __stdcall ClientThread(LPVOID arg);

int main() {
	HANDLE hDummyThread[dfDUMMY_CNT];

	for (int i = 0; i < dfDUMMY_CNT; ++i) {
		hDummyThread[i] = (HANDLE) _beginthreadex(nullptr, 0, ClientThread,/*arg*/ 0, 0, nullptr);
	}

	LONG sendTPS;
	LONG recvTPS;

	for (;;) {
		Sleep(1000);

		sendTPS = InterlockedExchange(&g_sendCalc, 0);
		recvTPS = InterlockedExchange(&g_recvCalc, 0);

		printf_s("\nsendTPS[%d] recvTPS[%d]\n", sendTPS, recvTPS);

	}

	DWORD retval = WaitForMultipleObjects(dfDUMMY_CNT, hDummyThread, TRUE, INFINITE);
	switch (retval) {
	case WAIT_FAILED:
		break;
	case WAIT_TIMEOUT:
		break;
	default:
		break;
	}

	return 0;
}

unsigned int __stdcall ClientThread(LPVOID arg) {
		CEchoClient *pDummy = new CEchoClient;
	for (;;) {
		if (pDummy->Connect(L"127.0.0.1", 6000))
			pDummy->Test();
		else
			CRASH();
		pDummy->Disconnect();
	}
		delete pDummy;
	return 0;
}
