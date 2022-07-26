#pragma once
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib" )

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CRingBuffer.h"

// WSAAsyncSelect 경고 끄기
#pragma warning(disable:4996)


// 유저메시지
#define UM_NETWORK WM_USER+1

// 서버 정보
#define dfSERVERFORT 25000

// =========================
// 프로토콜
// 사용포트 25000
// 2 byte - 패킷길이 16
// 4 byte - Start X
// 4 byte - Start Y
// 4 byte - End X
// 4 byte - End Y
// 패킷의 총 사이즈는 18바이트
// # 자기 자신의 그리기도 서버로 패킷을 받아서 그림
// =========================

// 헤더
struct stHEADER {
	unsigned short Len;
};

// 패킷
struct st_DRAW_PACKET {
	int		iStartX;
	int		iStartY;
	int		iEndX;
	int		iEndY;
};


BOOL NetworkInitClient(HWND hWnd, const WCHAR *SERVERIP); // 서버를 찾고 CONNECT까지 초기화
void ReadEvent(HWND hWnd);	// 서버에서 받은 정보로 그림그리기
void WriteEvent();	// 드레그한 좌표를 서버에 보내기
BOOL SendPacket(char *chpPacket, int iSize); // 서버에 패킷을 전송
void NetworkCloseClient(); // 정리