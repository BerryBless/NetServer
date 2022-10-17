#pragma once
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib" )

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CRingBuffer.h"

// select 모델!

// 서버 정보
#define dfSERVERFORT 25000
#define dfADDRALL L"0.0.0.0"
#define dfMAXPLAYER 60
#define dfBUFFERSIZE 1000

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

struct stPLAYER {
	SOCKET sock;			// 클라이언트 소켓
	CRingBuffer recvQ;	// 수신 링버퍼
	CRingBuffer sendQ;	// 송신 링버퍼
};


BOOL NetworkInitServer(); // 서버를 찾고 CONNECT까지 초기화
void SelectProc();	// select 모델을 돌릴 함수
void AcceptProc();	// Accept() 처리시 처리해야할 일 (플레이어 생성 등)
void SendProc(stPLAYER *stpPlayer); // sendQ에서 메시지 빼서 send()에 보내기
void SendUnicast(stPLAYER *stpPlayer, char *chpBuffer, int iSize); // stpPlayer->snedQ에 chpBuffer넣기
void SendBroadcast(stPLAYER *stpPlayerEx, char *chpBuffer, int iSize);// stpPlayerEx를 제외한 모든 플레이어의 sendQ에 chpBuffer넣기
void RecvProc(stPLAYER *stpPlayer);	// stpPlayer가 select반응이 왔을 넣기
void Disconnect(stPLAYER *stpPlayer); //stpPlayer를 연결끊기 
void NetworkCloseServer(); // 정리