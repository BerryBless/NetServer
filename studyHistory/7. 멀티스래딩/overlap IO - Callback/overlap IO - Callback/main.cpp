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

SOCKET g_clientsock; // 일단 책에 있는 예제 따라가기
//TODO 큐로 바꾸기

HANDLE g_hReadEvent, g_hWriteEvent;

BOOL NetworkInitServer();		// 네트워크 초기화
void NetworkCloseServer();		// 네트워크 종료


unsigned int __stdcall WorkerThread(LPVOID arg);// 워커스레드
void CALLBACK CompletionRoutine(DWORD Error, DWORD readn, LPWSAOVERLAPPED overlapped, DWORD lnFlags); // 완료시 호출할 루틴

int main() {

	NetworkInitServer();

	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	DWORD retval, recvbyte, flags;


	HANDLE hThread[5];

	// 이벤트 객체 생성

	g_hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (g_hReadEvent == NULL)
		return -1;

	g_hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (g_hWriteEvent == NULL)
		return -1;

	hThread[0] = (HANDLE) _beginthreadex(NULL, 0, &WorkerThread, 0, 0, NULL);

	while (true) {
		// accept()
		addrlen = sizeof(clientAddr);
		g_clientsock = accept(g_listenSock, (SOCKADDR *) &clientAddr, &addrlen);
		if (g_clientsock == INVALID_SOCKET) {
			_LOG(dfLOG_LEVEL_ERROR, L"////// accept error :: INVALID_SOCKET");
			return -1;
		}
		SetEvent(g_hWriteEvent);
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
bool g_flag = true; // 테스트용
unsigned int __stdcall WorkerThread(LPVOID arg) {
	DWORD retval;
	while (1) {
		while (1) {
			// alertable wait
			DWORD retval = WaitForSingleObjectEx(g_hWriteEvent, INFINITE, TRUE);
			if (retval == WAIT_OBJECT_0)
				break;
			if (retval != WAIT_IO_COMPLETION)
				return -1;
		}


		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(g_clientsock, (SOCKADDR *) &clientaddr, &addrlen);


		// 소켓 정보 구조체 할당과 초기화
		SOCKETINFO *pInfo = new SOCKETINFO;
		if (pInfo == NULL) {
			return -1;
		}
		memset(&(pInfo->_overlapped), 0, sizeof(pInfo->_overlapped));
		pInfo->_sock = g_clientsock;
		pInfo->_recvbytes = 0;
		pInfo->_sendbytes = 0;
		pInfo->_wsabuf.buf = pInfo->_buf;
		pInfo->_wsabuf.len = dfBUFSIZE;



		// 비동기 입출력 시작
		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(pInfo->_sock, &(pInfo->_wsabuf), 1, &recvbytes, &flags, &(pInfo->_overlapped), CompletionRoutine);
		while (g_flag) {
			printf_s(".");
			SleepEx(10, true);
		}
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				return -1;

			}

		}

	}
	return 0;
}

void CALLBACK CompletionRoutine(DWORD Error, DWORD readn, LPWSAOVERLAPPED overlapped, DWORD lnFlags) {
	int retval;
	// 클라이언트 정보 얻기
	SOCKETINFO *pInfo = (SOCKETINFO *) overlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(pInfo->_sock, (SOCKADDR *) &clientaddr, &addrlen);



	// 비동기 입출력 결과 확인
	if (Error != 0 || readn == 0) {
		if (Error != 0)
			_LOG(dfLOG_LEVEL_BUG, L"////// WSARecv() err[%d]", Error);
		closesocket(pInfo->_sock);
		delete pInfo;
		return;

	}
	g_flag = false;


	// 데이터 전송량 갱신
	if (pInfo->_recvbytes == 0) {
		pInfo->_recvbytes = readn;
		pInfo->_sendbytes = 0;
		// 받은 데이터 출력
		pInfo->_buf[pInfo->_recvbytes] = '\0';

	}

	else {
		pInfo->_sendbytes += readn;

	}



	if (pInfo->_recvbytes > pInfo->_sendbytes) {

		// 데이터 보내기

		memset(&(pInfo->_overlapped), 0, sizeof(pInfo->_overlapped));
		pInfo->_wsabuf.buf = pInfo->_buf + pInfo->_sendbytes;
		pInfo->_wsabuf.len = pInfo->_recvbytes - pInfo->_sendbytes;


		DWORD sendbytes;
		retval = WSASend(pInfo->_sock, &(pInfo->_wsabuf), 1, &sendbytes, 0, &(pInfo->_overlapped), CompletionRoutine);
		if (retval == SOCKET_ERROR) {
			Error = WSAGetLastError();
			if (Error != WSA_IO_PENDING) {
				_LOG(dfLOG_LEVEL_BUG, L"////// WSASend() err[%d]", Error);
				return;
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
		retval = WSARecv(pInfo->_sock, &(pInfo->_wsabuf), 1, &recvbytes, &flags, &(pInfo->_overlapped), CompletionRoutine);
		if (retval == SOCKET_ERROR) {
			Error = WSAGetLastError();
			if (Error != WSA_IO_PENDING) {
				_LOG(dfLOG_LEVEL_BUG, L"////// WSASend() err[%d]", Error);
				return;
			}
		}
	}
}
