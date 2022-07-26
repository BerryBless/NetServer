#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

int main() {
	int ret;
	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wprintf_s(L"WSAStartup() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// 소켓 기본정보
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(9000);
	InetPton(AF_INET, L"127.0.0.1", &sockAddr.sin_addr);

	// 연결시도 클라 카운트
	int conCount = 0;
	while(true){
		// socket()
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET) {
			wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
			wprintf_s(L"maxcount ( %d )\n", conCount);
			WSACleanup();
			return 1;
		}

		// 소켓 설정 (연결시간)
		LINGER lin;
		lin.l_onoff = 1;//1초동안 열림
		lin.l_linger = 0;// closesocket호출후 0초뒤 소멸
		setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &lin, sizeof(lin));

		// connect()
		ret = connect(sock, (SOCKADDR *) &sockAddr, sizeof(sockAddr));
		if (ret == SOCKET_ERROR) {
			wprintf_s(L"connect() errcode[%d]\n", WSAGetLastError());
			wprintf_s(L"maxcount ( %d )\n", conCount);
			WSACleanup();
			return 1;
		}

		// 시도만하고 바로 소캣닫고 카운트 하나 늘리기
		closesocket(sock);
		//shutdown(sock, SD_BOTH);
		++conCount;
	}

	return 0;
}
//======
// https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
// 결과
// errcode[10061]
// 아무지정 안했을때 : 200
// ret = listen(listenSock, SOMAXCONN_HINT(MAXINT)) : 13830
// 무언가 틀렸다!!!!! => 메모리 허용량까지 안감!
// 가설1 소켓이 바로 없어지지 않는다
// 클라이언트의 연결이 유지될 필요가 없다
// setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &lin, sizeof(lin));
// ->65535개 
// 메모리 허용량이 아닌 65535???
// 연결시간 설정후 결과
// 0이면 1개
// 음수면 200개
// 1~65535 그 개수만큼
//======
