#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#include <locale.h> // wchar 한글
#include <conio.h>//_kbhit

#define SERVERPORT 9997
#define BUFSIZE 1000

struct st_PACKET_HEADER {
	DWORD dwPacketCode; // 0x11223344 우리의 패킷확인 고정값

	WCHAR szName[32]; // 본인이름, 유니코드 NULL 문자 끝
	WCHAR szFileName[128]; // 파일이름, 유니코드 NULL 문자 끝
	int iFileSize;
};


int main() {
	// 한글설정
	setlocale(LC_ALL, "KOREAN");

	int ret;
	st_PACKET_HEADER stHeader;

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
	servAddr.sin_port = htons(SERVERPORT);
	InetPton(AF_INET, L"0.0.0.0", &servAddr.sin_addr);	// 권장하는 방법

	// bind()
	ret = bind(listenSock, (SOCKADDR *) &servAddr, sizeof(servAddr));
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"bind() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// listen()
	ret = listen(listenSock, SOMAXCONN);
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"listen() errcode[%d]\n", WSAGetLastError());
		return 1;
	}
	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	BYTE *filebuf;
	WCHAR szClientIP[16] = {0};
	char retVal[4];
	memset(retVal, 0xDD, 4);

	FILE *out;

	WCHAR path[160];

	while (true) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listenSock, (SOCKADDR *) &clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			wprintf_s(L"accept() errcode[%d]\n", WSAGetLastError());
			break;
		}
		// 접속한 클라 정보 출력
		InetNtop(AF_INET, &clientaddr.sin_addr, szClientIP, 16);
		wprintf_s(L"\n[TCP SERVER] Clinet connet: %ls:%d\n", szClientIP, ntohs(clientaddr.sin_port));


		// 클라이언트와 데이터 통신

		// 헤더 받기
		ret = recv(client_sock, buf, BUFSIZE, 0);
		if (ret == SOCKET_ERROR) {
			wprintf_s(L"recv() errcode[%d]\n", WSAGetLastError());
			break;
		} else if (ret == 0) {
			wprintf_s(L"받은데이터가 없습니다");

			// closesocket()
			closesocket(client_sock);
			wprintf_s(L"[TCP 서버] 클라이언트 종료 : %ls:%d\n", szClientIP, ntohs(clientaddr.sin_port));

			continue;
		}

		// 받은 헤더 출력 
		wprintf_s(L"--------------------------------------------------------------------------------\n");
		wprintf_s(L"받은 데이터 %d 바이트\n", ret);
		memcpy_s(&stHeader, sizeof(stHeader), buf, sizeof(stHeader));
		wprintf_s(L"헤더..\ndwPacketCode(%x)\n", stHeader.dwPacketCode);
		if (stHeader.dwPacketCode == 0x11223344) {
			wprintf_s(L"szName(%s)\n", stHeader.szName);
			wprintf_s(L"szFileName(%s)\n", stHeader.szFileName);
			wprintf_s(L"iFileSize(%d)\n", stHeader.iFileSize);
		} else {
			wprintf_s(L"잘못된 헤더\n");

			// closesocket()
			closesocket(client_sock);
			wprintf_s(L"[TCP 서버] 클라이언트 종료 : %ls:%d\n", szClientIP, ntohs(clientaddr.sin_port));
			continue;
		}
		int index = 0;
		filebuf = new BYTE[stHeader.iFileSize];

		// 헤더 뒤에 데이터 있으면.. 그것도 넣어주기
		if (ret > sizeof(st_PACKET_HEADER)) {
			memcpy_s(filebuf, stHeader.iFileSize,
				buf + sizeof(st_PACKET_HEADER), ret - sizeof(st_PACKET_HEADER));
			index += ret - sizeof(st_PACKET_HEADER);
		}

		wprintf_s(L"--------------------\n");
		wprintf_s(L"파일 데이터 받기\n");
		// 파일버퍼에 다넣기!
		while (index < stHeader.iFileSize) {

			ret = recv(client_sock, buf, BUFSIZE, 0);
			if (ret == SOCKET_ERROR) {
				wprintf_s(L"recv() errcode[%d]\n", WSAGetLastError());
				break;
			}
			wprintf_s(L"받은 데이터 %d 바이트\n", ret);

			memcpy_s(filebuf + index, stHeader.iFileSize,
				buf, ret);
			index += ret;
		}
		wprintf_s(L"--------------------\n");
		wprintf_s(L"파일에 쓰기!\n");

		// 파일버퍼에 있는거 파일에 모두 쓰기
		swprintf_s(path, L"%s_%s", stHeader.szName, stHeader.szFileName);
		_wfopen_s(&out, path, L"wb");
		fwrite((void *) filebuf, stHeader.iFileSize, 1, out);
		fclose(out);
		// 파일버퍼 할당해지
		delete[] filebuf;

		// 응답 보내기
		wprintf_s(L"--------------------\n");

		ret = send(client_sock, retVal, 4, 0);
		if (ret == SOCKET_ERROR) {
			wprintf_s(L"send() errcode[%d]\n", WSAGetLastError());
			break;
		}
		wprintf_s(L"DDDDDDDD 보내기 완료!\n");
		
		// closesocket()
		closesocket(client_sock);
		wprintf_s(L"[TCP SERVER] 클라이언트 종료 : %ls:%d\n", szClientIP, ntohs(clientaddr.sin_port));
	}

	closesocket(listenSock);
	WSACleanup();

	return 0;
}