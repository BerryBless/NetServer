#pragma once
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib" )

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CRingBuffer.h"
#include "PacketDefine.h"

// WSAAsyncSelect 경고 끄기
#pragma warning(disable:4996)

// 유저메시지
#define UM_NETWORK WM_USER+1

// 서버 정보
#define dfSERVERPORT 10605
#define dfSERVERIP L"106.245.38.107"
//#define dfSERVERIP L"127.0.0.1"


BOOL NetworkInitClient(HWND hWnd, const WCHAR *SERVERIP); // 서버를 찾고 CONNECT까지 초기화
BOOL ReadEvent();	// 서버에서 받은거 (SC)
BOOL WriteEvent();	// recvQ에 있는걸 모두 보내기, 막혔다가 뚫릴때 (CS)
BOOL SendPacket(char *chpPacket, int iSize); // 서버에 패킷을 전송
void NetworkCloseClient(); // 정리