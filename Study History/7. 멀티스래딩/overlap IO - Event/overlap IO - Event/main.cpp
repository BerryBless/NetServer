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


// 배열 인덱스 저장용
int g_totalsock = 0;
// 이벤트 저장을 위한 구조체 배열
WSAEVENT g_eventarr[WSA_MAXIMUM_WAIT_EVENTS];
// 소켓 정보를 저장하기 위한 구조체 배열
SOCKETINFO *g_socketarr[WSA_MAXIMUM_WAIT_EVENTS];
// 크리티컬 섹션
CRITICAL_SECTION g_lock;

BOOL NetworkInitServer();		// 네트워크 초기화
void NetworkCloseServer();		// 네트워크 종료


unsigned int __stdcall WorkerThread(LPVOID arg);// 워커스레드

// 소켓관리 함수
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int ID);

int main() {

	InitializeCriticalSection(&g_lock);
	NetworkInitServer();

	SOCKET clientSock;
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	DWORD retval, recvbyte, flags;


	HANDLE hThread[5];
	hThread[0] = (HANDLE) _beginthreadex(NULL, 0, &WorkerThread, 0, 0, NULL);

	while (true) {
		addrlen = sizeof(clientAddr);
		clientSock = accept(g_listenSock, (SOCKADDR *) &clientAddr, &addrlen);
		if (clientSock == INVALID_SOCKET) {
			_LOG(dfLOG_LEVEL_ERROR, L"////// accept error :: INVALID_SOCKET");
			return -1;
		}

		if (AddSocketInfo(clientSock) == false) {
			closesocket(clientSock);
			_LOG(dfLOG_LEVEL_ERROR, L"////// Create Client error ");
			continue;
		}

		SOCKETINFO *pInfo = g_socketarr[g_totalsock - 1];

		flags = 0;

		retval = WSARecv(pInfo->_sock, &pInfo->_wsabuf, 1, &recvbyte, &flags, &pInfo->_overlapped, NULL); // 이벤트 방식
		if (retval == SOCKET_ERROR) {
			DWORD err = WSAGetLastError();
			if (err != WSA_IO_PENDING) {
				_LOG(dfLOG_LEVEL_ERROR, L"////// WSARecv err[%d] ", err);
				RemoveSocketInfo(g_totalsock);
				continue;
			}
		}

		WSASetEvent(g_eventarr[0]);
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


	AddSocketInfo(g_listenSock);// 0번은 더미 / 리슨 소켓 이벤트 (무시하고 넘어가야함) 이 소켓에 이벤트가 오면 클라가 추가되었다는 뜻

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
unsigned int __stdcall WorkerThread(LPVOID arg) {
	int retval;
	DWORD bytesTransferred; // 결과 확인
	DWORD flags =0; 
	while (true) {
		DWORD index = WSAWaitForMultipleEvents(g_totalsock, g_eventarr, FALSE, WSA_INFINITE, FALSE);

		if (index == WSA_WAIT_FAILED) {
			_LOG(dfLOG_LEVEL_ERROR, L"////// WSARecv err[%d] ", WSAGetLastError());
			continue;
		}
		index -= WSA_WAIT_EVENT_0;
		WSAResetEvent(g_eventarr[index]);
		if (index == 0) continue;

		// 클라 정보 얻기
		SOCKETINFO *pInfo = g_socketarr[index];
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(pInfo->_sock, (SOCKADDR *) &clientaddr, &addrlen);

		// 비동기 입출력 결과 확인
		retval = WSAGetOverlappedResult(pInfo->_sock, &pInfo->_overlapped, &bytesTransferred, FALSE, &flags);
		if (retval == FALSE || bytesTransferred == 0) {
			// 받지 못함, 연결 끊기
			RemoveSocketInfo(index);
		}

		// 데이터 전송량 갱신
		if (pInfo->_recvbytes == 0) {
			pInfo->_recvbytes = bytesTransferred;
			pInfo->_sendbytes = 0;
			// 받은 데이터 출력
			pInfo->_buf[pInfo->_recvbytes] = '\0';
			printf_s("%s\n", pInfo->_buf);
		} else { pInfo->_sendbytes += bytesTransferred; }

		if (pInfo->_recvbytes > pInfo->_sendbytes) {
			// 데이터 보내기
			memset(&pInfo->_overlapped, 0, sizeof(pInfo->_overlapped));
			pInfo->_overlapped.hEvent = g_eventarr[index];
			pInfo->_wsabuf.buf = pInfo->_buf + pInfo->_sendbytes;
			pInfo->_wsabuf.len = pInfo->_recvbytes - pInfo->_sendbytes;

			DWORD sendbytes;
			retval = WSASend(pInfo->_sock, &(pInfo->_wsabuf), 1, &sendbytes, 0, &(pInfo->_overlapped), NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					_LOG(dfLOG_LEVEL_ERROR, L"WSASend\n");
				}
				continue;
			}
		} else {
			pInfo->_recvbytes = 0;

			// 데이터 받기
			memset(&pInfo->_overlapped, 0, sizeof(pInfo->_overlapped));
			pInfo->_overlapped.hEvent = g_eventarr[index];
			pInfo->_wsabuf.buf = pInfo->_buf;
			pInfo->_wsabuf.len = dfBUFSIZE;

			DWORD recvbytes;
			flags = 0;
			retval = WSARecv(pInfo->_sock, &(pInfo->_wsabuf), 1, &recvbytes, &flags, &(pInfo->_overlapped), NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					_LOG(dfLOG_LEVEL_ERROR, L"WSARecv\n");
				}
				continue;
			}
		}
	}

	return 0;
}

BOOL AddSocketInfo(SOCKET sock) {
	EnterCriticalSection(&g_lock);
	if (g_totalsock >= WSA_MAXIMUM_WAIT_EVENTS) {
		LeaveCriticalSection(&g_lock);
		return FALSE;
	}

	SOCKETINFO *pInfo = new SOCKETINFO;
	if (pInfo == NULL) {
		LeaveCriticalSection(&g_lock);
		return FALSE;
	}

	WSAEVENT hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT) {
		LeaveCriticalSection(&g_lock);
		return FALSE;
	}

	memset(&pInfo->_overlapped, 0, sizeof(pInfo->_overlapped));
	pInfo->_overlapped.hEvent = hEvent;
	pInfo->_sock = sock;
	pInfo->_recvbytes = 0;
	pInfo->_sendbytes = 0;
	pInfo->_wsabuf.buf = pInfo->_buf;
	pInfo->_wsabuf.len = dfBUFSIZE;
	g_socketarr[g_totalsock] = pInfo;
	g_eventarr[g_totalsock] = hEvent;
	++g_totalsock;

	LeaveCriticalSection(&g_lock);
	return TRUE;
}

void RemoveSocketInfo(int ID) {
	EnterCriticalSection(&g_lock);


	SOCKETINFO *pInfo = g_socketarr[ID];
	closesocket(pInfo->_sock);
	delete pInfo;
	WSACloseEvent(g_eventarr[ID]);

	LeaveCriticalSection(&g_lock);
}
