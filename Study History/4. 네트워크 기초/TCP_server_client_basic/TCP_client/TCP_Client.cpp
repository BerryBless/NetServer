#pragma comment (lib, "ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h> //InetPton
#include <stdio.h>
#include <stdlib.h>

#define SERVERIP L"127.0.0.1"
#define SERVERPORT 6000
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
// 사용자정의 데이터 수신 함수
// 상대방이 몇바이트 보낼지 모르면 못씀
int recvn(SOCKET s, char *buf, int len, int flags) {
	int received;
	char *ptr = buf;
	int left = len;

	// 내가 원하는 크기를 채울때 까지
	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return  SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}

int main() {
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;// 초기화 실패!

	// socet()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); //IP4, TCP소켓!
	if (sock == INVALID_SOCKET) err_quit(L"socket");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//serveraddr.sin_addr.s_addr = inet_addr(SERVERIP); // 루프백에 커넥트!
	InetPton(AF_INET, SERVERIP, &serveraddr.sin_addr);	// 권장하는 IP주소를 IN_ADDR로 바꾸기
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *) &serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit(L"connect()");

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE + 1];
	int len;

	// 서버와 통신
	while (true) {
		// 데이터 입력
		printf_s("\n [보낼 데이터] ");
		if (gets_s(buf, BUFSIZE) == NULL)
			break;

		// '\n' 문자 제거
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		// 아무 문자 없으면 연결 끊기
		if (strlen(buf) == 0)
			break;

		// 데이터 보내기
		retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display(L"send()");
			break;
		}

		printf_s("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);

		// 데이터 받기 
		retval = recvn(sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display(L"recvn()");
			break;
		} else if (retval == 0) {
			break;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf_s("[TCP 클라이언트] %d 바이트를 받았습니다\n", retval);
		printf_s("[받은 데이터] %s\n", buf);
	}

	// closesocket()
	closesocket(sock);

	//윈속 종료
	WSACleanup();
	return 0;
}