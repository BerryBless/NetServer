#pragma comment (lib, "ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h> //InetPton
#include <stdio.h>
#include <stdlib.h>


#define SERVERPORT 9000
#define BUFSIZE 512

// 소켓함수 오류 출력후 종료
void err_quit(const WCHAR *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR) lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const WCHAR *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, NULL);
	wprintf_s(L"[%s] %s", msg, (WCHAR *) lpMsgBuf);
	LocalFree(lpMsgBuf);
}


int main() {
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;// 초기화 실패!

	// socet()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0); //IP4, TCP소켓!
	if (listen_sock == INVALID_SOCKET) err_quit(L"socket()");


	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);	// 0.0.0.0에 연결하겠다
	InetPton(AF_INET, L"0.0.0.0",&serveraddr.sin_addr);	// 권장하는 방법
	serveraddr.sin_port = htons(SERVERPORT);		// 포트는 9000번
	retval = bind(listen_sock, (SOCKADDR *) &serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit(L"bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit(L"listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	WCHAR szClientIP[16] = {0};

	while (true) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *) &clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display(L"accept()");
			break;
		}
		// 접속한 클라 정보 출력
		InetNtop(AF_INET, &clientaddr.sin_addr, szClientIP, 16);
		wprintf_s(L"\n[TCP SERVER] Clinet connet: %ls:%d\n", szClientIP, ntohs(clientaddr.sin_port));


		// 클라이언트와 데이터 통신
		while (true) {
			// 데이터 받기
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display(L"recv()");
				break;
			} else if (retval == 0) {
				break;
			}

			// 받은 데이터 출력 
			buf[retval] = '\0';
			wprintf_s(L"[TCP/%s:%d] %s\n", szClientIP, ntohs(clientaddr.sin_port), buf);

			// 데이터 보내기
			retval = send(client_sock, buf, retval, 0);
			if (retval == SOCKET_ERROR) {
				err_display(L"send()");
				break;
			}

		}
		// closesocket()
		closesocket(client_sock);
		wprintf_s(L"[TCP 서버] 클라이언트 종료 : %ls:%d\n", szClientIP, ntohs(clientaddr.sin_port));
	}
	// closesoket()
	closesocket(listen_sock);

	//윈속 종료
	WSACleanup();
	return 0;
}