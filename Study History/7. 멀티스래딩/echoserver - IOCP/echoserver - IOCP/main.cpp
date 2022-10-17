// TODO
// 락걸어주기

#include "pch.h"
#include "NetworkCore.h"
#include <conio.h>


#define dfTHREAD_NUM 5

int main() {
	InitializeSRWLock(&g_sessionContainerLock);

	HANDLE hThread[dfTHREAD_NUM];
	NetworkInitServer();

	hThread[0] = (HANDLE) _beginthreadex(NULL, 0, AcceptThread, NULL, 0, NULL);

	for (int i = 1; i < dfTHREAD_NUM; i++) {
		hThread[i] = (HANDLE) _beginthreadex(NULL, 0, WorkerThread, NULL, 0, NULL);
	}

	while (true) {
		//TODO 스레드제어
		Sleep(1000);

		if (_kbhit()) {
			char cmd = _getch();
			if (cmd == 'Q' || cmd == 'q') {
				PostQueuedCompletionStatus(g_hIOCP, dfEXIT_KEY, dfEXIT_KEY, NULL); // 워커 하나 종료하라고 알려줌
				break;
			}
			if (cmd == 'W' || cmd == 'w') {
				for (auto iter = g_sessions.begin(); iter != g_sessions.end(); ++iter) {
					int recvSize = (iter->second)-> _recvQueue.GetUseSize();
					int sendSize = (iter->second)->_sendQueue.GetUseSize();

					_LOG_NONTIMESTAMP(dfLOG_LEVEL_ERROR, L"SOCK[%d] :: recv [%d]byte, send [%d]byte, IOCount[%d], isSend [%d]", 
						(iter->second)->_sock,recvSize,sendSize, (iter->second)->_IOcount, (iter->second)->_isSend);
				}
			}
			if (cmd == 'P' || cmd == 'p') {
				g_monitoring.PrintMonitoring();
			}
		}
		g_monitoring.PrintMonitoring();
	}

	closesocket(g_listenSock); // 리슨소켓 닫아서 Accept스레드 종료

	DWORD retval = WaitForMultipleObjects(dfTHREAD_NUM, hThread, TRUE, INFINITE);
	switch (retval) {
	case WAIT_FAILED:
		break;
	case WAIT_TIMEOUT:
		break;
	default:
		break;
	}
	NetworkCloseServer();
	return 0;
}
