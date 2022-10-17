#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib" )

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <locale.h> // wchar 한글
#include <conio.h>//_kbhit

#define dfSERVERIP L"127.0.0.1"
#define dfSERVERPORT 10605

char g_string[] = "Hellow?";

struct SESSION {
	SOCKET _sock;
	char _recvbuf[1000];
	char _sendbuf[1000];
};

int main() {
	// 한글설정
	setlocale(LC_ALL, "KOREAN");

	// 변수
	int sRet;	// sned return
	int rRet;	// recv return
	DWORD err;	// 겟라스트에러
	SESSION sessions[4];

	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wprintf_s(L"WSAStartup() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// 소켓 기본정보
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0xDD, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(dfSERVERPORT);
#ifdef dfSERVERIP
	InetPton(AF_INET, dfSERVERIP, &sockAddr.sin_addr);
#else
	WCHAR serverIP[16];
	wprintf_s(L"접속할 IP 주소 : ");
	wscanf_s(L"%ls", serverIP);
	InetPton(AF_INET, serverIP, &sockAddr.sin_addr);
#endif // DEBUG
	for (int i = 0; i < 4; i++) {
		// socket()
		sessions[i]._sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sessions[i]._sock == INVALID_SOCKET) {
			wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// connect()
		sRet = connect(sessions[i]._sock, (SOCKADDR *) &sockAddr, sizeof(sockAddr));
		if (sRet == SOCKET_ERROR) {
			wprintf_s(L"connect() errcode[%d]\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		u_long on = 1;
		sRet = ioctlsocket(sessions[i]._sock, FIONBIO, &on);
		if (sRet == SOCKET_ERROR) {
			wprintf_s(L"ioctlsocket() errcode[%d]\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
	}
	while (true) {

#pragma region NETWORK
		// select 모델
		fd_set rSet, wSet;


		// 소켓 셋 받기로 초기화

		FD_ZERO(&rSet);
		FD_ZERO(&wSet);
		for (int i = 0; i < 4; i++) {
			FD_SET(sessions[i]._sock, &rSet);
			FD_SET(sessions[i]._sock, &wSet);
		}
		timeval time;


		// 비동기 최대 대기시간 설정
		time.tv_sec = 0;
		time.tv_usec = 1;

		rRet = select(0, &rSet, &wSet, NULL, &time);
		if (rRet == SOCKET_ERROR) {
			err = GetLastError();
			if (err == WSAEWOULDBLOCK) {
				// 들어오지 않음 = 에러가아님
				wprintf_s(L"select() WSAEWOULDBLOCK errcode[%d]\n", err);

			} else {
				// 진짜로 에러!
				wprintf_s(L"select() errcode[%d]\n", err);
			}
		}

		// 뭔가 왔음!
		// 그 뭔가를 버퍼에서 긁어오기
		// 1프레임당 1메시지 처리가아니라 1프레임에 버퍼에있는 모든 메시지 처리하기
		for (int i = 0; i < 4; i++) {
			if (FD_ISSET(sessions[i]._sock, &rSet)) {
				rRet = recv(sessions[i]._sock, sessions[i]._recvbuf, sizeof(g_string), 0);
				if (rRet == SOCKET_ERROR) {
					err = WSAGetLastError();
					if (err == WSAEWOULDBLOCK) {
						// 버퍼가 비었다!
						break;
					} else {
						wprintf_s(L"recv() errcode[%d]\n", err);
						WSACleanup();
						closesocket(sessions[i]._sock);
						return 1;
					}
				} else if (rRet > 0) {

				}
			}
		}
#pragma endregion

#pragma region INPUT
		//*
		// 1. 입력 받아서
		// 2. msgMOVE 메시지를 만들고
		// 3. 보내기
		//

		// 보낼 소캣이 충분하다
		for (int i = 0; i < 4; i++) {
			if (FD_ISSET(sessions[i]._sock, &wSet)) {
				sRet = send(sessions[i]._sock, g_string, sizeof(g_string), 0);

			}
		}//*/
#pragma endregion

	}

	WSACleanup();
	return 0;
}
