#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#include <process.h>

#pragma comment(lib,"WS2_32")

WSADATA g_wsaData;		// 윈속
SOCKADDR_IN g_listenAddr;	// 리슨 소켓 정보
SOCKET g_listenSock;	// 리슨 소켓

#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_WARNING 1
#define dfLOG_LEVEL_BUG 2
#define dfLOG_LEVEL_ERROR 3


#define dfNETWORK_PORT 9000

#define _LOG(LogLevel, fmt, ...)	\
	do{	\
			wprintf_s(fmt,##__VA_ARGS__);\
		\
	} while (0) 
#define dfBUFSIZE 1000
// 소켓 정보 저장을 위한 구조체
struct SOCKETINFO {
	WSAOVERLAPPED _overlapped;
	SOCKET _sock;
	char _buf[dfBUFSIZE + 1];
	int _recvbytes;
	int _sendbytes;
	WSABUF _wsabuf;
};



BOOL NetworkInitServer();		// 네트워크 초기화
void NetworkCloseServer();		// 네트워크 종료

unsigned int __stdcall WorkerThread(LPVOID arg);// 워커스레드

int main() {

	NetworkInitServer();
	SOCKET clientsock;
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	DWORD retval, recvbyte, flags;

	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"hcp == NULL");
		return 1;
	}

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	HANDLE hThread;

	for (int i = 0; i < (int) si.dwNumberOfProcessors * 2; i++) {
		hThread = (HANDLE) _beginthreadex(NULL, 0, &WorkerThread, hcp, 0, NULL);
		if (hThread == NULL)return 1;
		CloseHandle(hThread);
	}



	while (true) {
		// accept()
		addrlen = sizeof(clientAddr);
		clientsock = accept(g_listenSock, (SOCKADDR *) &clientAddr, &addrlen);
		if (clientsock == INVALID_SOCKET) {
			_LOG(dfLOG_LEVEL_ERROR, L"////// accept error :: INVALID_SOCKET");
			return -1;
		}
		
			// 소켓과 입출력 완료 포트 연결
		CreateIoCompletionPort((HANDLE) clientsock, hcp, clientsock, 0);

		SOCKETINFO *pInfo = new SOCKETINFO;

		if (pInfo == NULL) {
			_LOG(dfLOG_LEVEL_ERROR, L"////// BADALLOC :: pInfo");
			break;
		}

		// send버퍼 0으로
		// 스레드 하나 더먹?
		int bufSize = 0;
		socklen_t len = sizeof(bufSize);
		setsockopt(clientsock, SOL_SOCKET, SO_SNDBUF, (const char*)&bufSize, sizeof(bufSize));

		memset(&(pInfo->_overlapped), 0, sizeof(pInfo->_overlapped));
		pInfo->_sock = clientsock;
		pInfo->_recvbytes = 0;
		pInfo->_sendbytes = 0;
		pInfo->_wsabuf.buf = pInfo->_buf;
		pInfo->_wsabuf.len = dfBUFSIZE;


		// 비동기 입출력 시작
		flags = 0;
		retval = WSARecv(pInfo->_sock, &(pInfo->_wsabuf), 1, &recvbyte, &flags, &(pInfo->_overlapped), NULL);
		if (retval == SOCKET_ERROR) {
			DWORD Error = WSAGetLastError();
			if (Error != WSA_IO_PENDING) {
				_LOG(dfLOG_LEVEL_BUG, L"////// WSARecv() err[%d]", Error);
				// 종료
				closesocket(pInfo->_sock);
				delete pInfo;
				continue;
			}
		}
	}
	NetworkCloseServer();
	return 0;
}

BOOL NetworkInitServer() {
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkInitServer()..");
	int setRet;// 소켓 연결에 나오는 리턴

		// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &g_wsaData) != 0) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// WSAStartup() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"WSAStartup OK..");

	// 소켓 기본정보
	memset(&g_listenAddr, 0, sizeof(g_listenAddr));
	g_listenAddr.sin_family = AF_INET;
	g_listenAddr.sin_port = htons(dfNETWORK_PORT);
	InetPton(AF_INET, L"0.0.0.0", &g_listenAddr.sin_addr);
	_LOG(dfLOG_LEVEL_ERROR, L"PORT [%d] g_listenAddr.sin_addr [%x] ", dfNETWORK_PORT, g_listenAddr.sin_addr);

	// socket()
	g_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_listenSock == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// socket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"g_listenSock OK [%lld]..", g_listenSock);

	// bind()
	setRet = bind(g_listenSock, (SOCKADDR *) &g_listenAddr, sizeof(g_listenAddr));
	if (setRet == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// bind() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"bind() OK [%d] ..", setRet);

	// listen()
	setRet = listen(g_listenSock, SOMAXCONN);
	if (setRet == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// listen() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"listen() OK [%d] ..", setRet);

	_LOG(dfLOG_LEVEL_ERROR, L"NetworkInitServer() OK..\n");
	return TRUE;
}
void NetworkCloseServer() {
	//---------------------------
	// 서버정리
	//---------------------------
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkCloseServer()..");
	WSACleanup();
	_LOG(dfLOG_LEVEL_ERROR, L"WSACleanup OK..");
	closesocket(g_listenSock);
	_LOG(dfLOG_LEVEL_ERROR, L"g_listenSock OK..");
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkCloseServer() OK..\n");
}
LONG g_cnt =0;
unsigned int __stdcall WorkerThread(LPVOID arg) {
	int retval;
	HANDLE hcp = (HANDLE) arg;

	while (true) {
		DWORD transferred = 0;
		SOCKET clientsock = 0;
		SOCKETINFO *pInfo;
		retval = GetQueuedCompletionStatus(hcp, &transferred, &clientsock, (LPOVERLAPPED *) &pInfo, INFINITE);

		InterlockedIncrement(&g_cnt);
		_LOG(dfLOG_LEVEL_ERROR, L"ID[%d] wake thread cnt [%d]\n",GetCurrentThreadId(), g_cnt);

		// 클라 정보 얻기
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(pInfo->_sock, (SOCKADDR *) &clientaddr, &addrlen);

		// 결과 확인
		if (retval == 0 || transferred == 0) {
			// 종료
			closesocket(pInfo->_sock);
			delete pInfo;
			InterlockedDecrement(&g_cnt);
			continue;
		}

		// 데이터 전송량 갱신
		if (pInfo->_recvbytes == 0) {
			pInfo->_recvbytes = transferred;
			pInfo->_sendbytes = 0;
			// 받은 데이터 출력
			pInfo->_buf[pInfo->_recvbytes] = '\0';
		}
		else {
			pInfo->_sendbytes += transferred;
		}



		if (pInfo->_recvbytes > pInfo->_sendbytes) {
			// 데이터 보내기
			memset(&(pInfo->_overlapped), 0, sizeof(pInfo->_overlapped));
			pInfo->_wsabuf.buf = pInfo->_buf + pInfo->_sendbytes;
			pInfo->_wsabuf.len = pInfo->_recvbytes - pInfo->_sendbytes;


			DWORD sendbytes;
			_LOG(dfLOG_LEVEL_BUG, L"call WSASend() \n");
			retval = WSASend(pInfo->_sock, &(pInfo->_wsabuf), 1, &sendbytes, 0, &(pInfo->_overlapped), NULL);
			if (retval == SOCKET_ERROR) {
				DWORD Error = WSAGetLastError();
				if (Error != WSA_IO_PENDING) {
					_LOG(dfLOG_LEVEL_BUG, L"////// WSASend() err[%d]", Error);
					// 종료
					closesocket(pInfo->_sock);
					delete pInfo;
					InterlockedDecrement(&g_cnt);
					continue;
					//return -1;
				}
				else {
					_LOG(dfLOG_LEVEL_BUG, L"WSASend() WSA_IO_PENDING \n");
				}
			}
		} else {

			pInfo->_recvbytes = 0;

			// 데이터 받기

			memset(&(pInfo->_overlapped), 0, sizeof(pInfo->_overlapped));
			pInfo->_wsabuf.buf = pInfo->_buf;
			pInfo->_wsabuf.len = dfBUFSIZE;



			DWORD recvbytes;
			DWORD flags = 0;
			_LOG(dfLOG_LEVEL_BUG, L"call WSARecv() \n");
			retval = WSARecv(pInfo->_sock, &(pInfo->_wsabuf), 1, &recvbytes, &flags, &(pInfo->_overlapped), NULL);
			if (retval == SOCKET_ERROR) {
				DWORD Error = WSAGetLastError();
				if (Error != WSA_IO_PENDING) {
					_LOG(dfLOG_LEVEL_BUG, L"////// WSARecv() err[%d]", Error);
					// 종료
					closesocket(pInfo->_sock);
					delete pInfo;
					InterlockedDecrement(&g_cnt);
					continue;
					//return -1;
				}
				else {
					_LOG(dfLOG_LEVEL_BUG, L"WSARecv() WSA_IO_PENDING \n");

				}
			}
		}
		InterlockedDecrement(&g_cnt);
	}
}
