#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <locale.h> // wchar 한글
#include <conio.h>//_kbhit

#define DOMAIN L"procademyserver.iptime.org"

#define SERVERIP L"61.77.85.58"
#define BUFSIZE 1000

#define FILENAME L"MHWIB_wallpaper.jpg"
#define SERVERPORT 9997

#define IMAGEFILE	// 이미지 파일 보내기
//#define PROCADEMY // 학원서버에 보내기
// 헤더
struct st_PACKET_HEADER {
	DWORD dwPacketCode; // 0x11223344 우리의 패킷확인 고정값

	WCHAR szName[32]; // 본인이름, 유니코드 NULL 문자 끝
	WCHAR szFileName[128]; // 파일이름, 유니코드 NULL 문자 끝
	int iFileSize;
};


BOOL DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr) {
	ADDRINFOW *pAddrInfo;	// IP정보
	SOCKADDR_IN *pSockAddr;

	// pAddrInfo 에 도메인을 IP로 변환한것을 "리스트로 변환 해줌" (이중포인터)
	// 외부에서 반드시 해재 해줘야함!
	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0) {
		return FALSE;
	}
	pSockAddr = (SOCKADDR_IN *) pAddrInfo->ai_addr;
	*pAddr = pSockAddr->sin_addr;

	FreeAddrInfo(pAddrInfo); // pAddrInfo 리스트 할당 해재!!!!!!
	return TRUE;
}

int main() {
	// 한글설정
	setlocale(LC_ALL, "KOREAN");
	// 변수들
	int ret; // 리턴값
	char buf[BUFSIZE + 1]; // TCP로 보낼 버퍼!
	int iMesSize = 0; // 메시지 사이즈
	int iDataSize = 0; // 메시지에 들어갈 데이터 사이즈

	st_PACKET_HEADER stHeader;// 헤더

	FILE *fpIn; // 파일포인터
	BYTE *FileBuffer; // 파일데이터 버퍼
	int iFileSize;

	BOOL bSend = true;


#ifdef IMAGEFILE


	// 파일 열기/

	_wfopen_s(&fpIn, FILENAME, L"rb");
	if (fpIn == NULL) {
		wprintf_s(L"파일오픈에러");
		return 1;
	}


	// 파일사이즈 구하기
	fseek(fpIn, 0, SEEK_END);
	iFileSize = ftell(fpIn);
	fseek(fpIn, 0, SEEK_SET);

	FileBuffer = new BYTE[iFileSize];

	// 파일버퍼에 데이터 넣고 파일포인터 닫기
	if (fread_s(FileBuffer, iFileSize, iFileSize, 1, fpIn) < 1) {
		wprintf_s(L"파일읽기에러");
		return 1;
	}
	fclose(fpIn);//*/

#else
	iFileSize = 2000;
	FileBuffer = new BYTE[iFileSize];
	memset(FileBuffer, 0xAA, iFileSize);

#endif // IMAGEFILE

	// 헤더 초기화
	memset(&stHeader, 0, sizeof(st_PACKET_HEADER));
	stHeader.dwPacketCode = 0x11223344;
	wcscpy_s(stHeader.szName, L"최우종");
	wcscpy_s(stHeader.szFileName, FILENAME);
	stHeader.iFileSize = iFileSize;

	// 헤더를 메시지에 넣기
	memcpy_s(buf, BUFSIZE, &stHeader, sizeof(st_PACKET_HEADER));
	iMesSize = sizeof(st_PACKET_HEADER);

#pragma region NETWORK SETTING
	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wprintf_s(L"WSAStartup() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

#ifdef PROCADEMY
	// 도메인으로 아이피 얻어오기
	IN_ADDR addr;
	if (DomainToIP(DOMAIN, &addr) == FALSE) {
		return 1;
	}

	// 소켓 기본정보
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0xDD, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr = addr;
	sockAddr.sin_port = htons(SERVERPORT);
#else

	// 소켓 기본정보
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0xDD, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(SERVERPORT);
	InetPton(AF_INET, SERVERIP, &sockAddr.sin_addr);

#endif // PROCADEMY

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// connect()
	ret = connect(sock, (SOCKADDR *) &sockAddr, sizeof(sockAddr));
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"connect() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}



#pragma endregion



			// 헤더 보내기
	ret = send(sock, buf, iMesSize, 0);
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"connect() errcode[%d]\n", WSAGetLastError());
	}
	wprintf_s(L"--------------------------------------------------------------------------------\n");
	wprintf_s(L"헤더\n dwPacketCode( 0x%x ), szName( %s ), szFileName( %s ), iFileSize( %d )\n",
		stHeader.dwPacketCode, stHeader.szName, stHeader.szFileName, stHeader.iFileSize);

	printf_s("[클라이언트] %d바이트를 보냈습니다.\n", ret);




	// 사진 데이터 보내기
	wprintf_s(L"------------------------\n");
	int index = 0;
	bSend = TRUE;
	while (bSend) {
		// 파일 데이터 버퍼에 싣기
		if (index + BUFSIZE < iFileSize) {
			iDataSize = BUFSIZE;
		} else {
			iDataSize = iFileSize - index;
			bSend = FALSE;
		}
		// 사진데이터를 메시지에 넣기
		memcpy_s(buf, BUFSIZE, FileBuffer + index, iDataSize);
		index += iDataSize;

		// 데이터 보내기
		ret = send(sock, buf, iDataSize, 0);
		if (ret == SOCKET_ERROR) {
			wprintf_s(L"connect() errcode[%d]\n", WSAGetLastError());
		}
		printf_s("[클라이언트] %d바이트를 보냈습니다.\n", ret);
	}
	wprintf_s(L"------------------------\n");

	// 데이터 받기 
	memset(buf, 0, BUFSIZE);
	ret = recv(sock, buf, ret, 0);
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"connect() errcode[%d]\n", WSAGetLastError());
	}

	printf_s("%d 바이트를 받았습니다\n", ret);
	printf_s("[받은 데이터] 0x%x\n", *((int *) buf));
	wprintf_s(L"------------------------\n");


	delete[] FileBuffer;
	WSACleanup();
	closesocket(sock);

	return 0;
}
