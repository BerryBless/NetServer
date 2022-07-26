#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib" )

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <locale.h> // wchar 한글
#include <conio.h>//_kbhit

#define dfSTARTPORT 10001
#define dfENDPORT 10099

int main() {
	// 한글설정
	setlocale(LC_ALL, "KOREAN");


	int sRet; // send 리턴
	int rRet; // recv 리턴
	int ret;  // 연결용 리턴

	// 버퍼
	char sendBuff[10] = {0xff,0xee,0xdd,0xaa,0x00,0x99,0x77,0x55,0x33,0x11};
	char recvBuff[1024];
	int addrLen;

	// select모델 설정 (recv 200ms 대기를 위한)
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000; // 200000 마이크로 = 200 밀리
	fd_set rset;

	// 출력을 위한 버퍼
	WCHAR IP[16] = {0};

	
	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wprintf_s(L"WSAStartup() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

#pragma region SOCKET
	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	// 비동기 소켓으로 변경
	u_long on = 1;
	sRet = ioctlsocket(sock, FIONBIO, &on);
	if (sRet == SOCKET_ERROR) {
		wprintf_s(L"ioctlsocket() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	// 브로드캐스팅 활성화
	BOOL bEnable = TRUE;
	ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
		(char *) &bEnable, sizeof(bEnable));
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"setsockopt() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
#pragma endregion

	for (int curPort = dfSTARTPORT; curPort <= dfENDPORT; curPort++) {
		// 소켓 기본정보
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0xDD, sizeof(sockAddr));
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(curPort);
		InetPton(AF_INET, L"255.255.255.255", &sockAddr.sin_addr);

		// 브로드캐스팅
		sRet = sendto(sock, sendBuff, 10, 0, (SOCKADDR *) &sockAddr, sizeof(sockAddr));
		if (sRet == SOCKET_ERROR) {
			wprintf_s(L"sendto() errcode[%d]\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// select()
		FD_ZERO(&rset);
		FD_SET(sock, &rset);
		select(0, &rset, NULL, NULL, &timeout);

		// recv()
		if (FD_ISSET(sock, &rset)) {
			addrLen = sizeof(sockAddr);
			rRet = recvfrom(sock, recvBuff, sizeof(recvBuff), 0, (SOCKADDR *) &sockAddr, &addrLen);
			//rRet = recv(sock, recvBuff, sizeof(recvBuff), 0);
			if (rRet == SOCKET_ERROR) {
				DWORD err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK) {
					wprintf_s(L"recvfrom() errcode[%d]\n", err);
					return 1;
				}
			}

			// 무언가 왔다!
			if (rRet > 0) {
				*(recvBuff + rRet) = '\0';
				*(recvBuff + rRet + 1) = '\0';
				InetNtop(AF_INET, &sockAddr.sin_addr, IP, 16);
				wprintf_s(L"%s  (%s:%d)\n", (WCHAR *) recvBuff, IP, curPort);
			}
		} else {
			wprintf_s(L".");
		}
	}
	closesocket(sock);
	return 0;
}