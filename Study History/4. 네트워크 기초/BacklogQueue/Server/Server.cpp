#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {
	int ret;
	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wprintf_s(L"WSAStartup() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// 리슨 소켓 
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// 서버 바인딩
	SOCKADDR_IN servAddr;
	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(9000);
	InetPton(AF_INET, L"0.0.0.0", &servAddr.sin_addr);	// 권장하는 방법

	// bind()
	ret = bind(listenSock, (SOCKADDR *) &servAddr, sizeof(servAddr));
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"bind() errcode[%d]\n", WSAGetLastError());
	}

	// listen()
	//ret = listen(listenSock, SOMAXCONN_HINT(MAXINT)); // SOMAXCONN_HINT(MAXINT) : ()안의 숫자로 백로그큐 지정
	ret = listen(listenSock, SOMAXCONN); 
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"listen() errcode[%d]\n", WSAGetLastError());
		return 1;
	}
	// 일부로 아무것도 안하고 바쁜대기
	int a=0;
	while (true) {
		a++;
	}

	closesocket(listenSock);
	WSACleanup();

	return 0;
}