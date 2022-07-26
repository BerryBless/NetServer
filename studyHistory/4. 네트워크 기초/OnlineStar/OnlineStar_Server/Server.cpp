#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib" )

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <locale.h> // wchar 한글
#include <conio.h>//_kbhit

// 나의 라이브러리
#include "Profiler.h"
#include "CQueue.h"
#include "CRingBuffer.h"

#define dfSERVERPORT 3000
#define dfMAXPLAYER 1000
#define dfMAXWITDH 80
#define dfMAXHEIGHT 23
#define dfBUFFERSIZE 1000

//#define PRINTACTION
//#define RENDER

using namespace std;

typedef int msgHEADER;
struct msgIDSET {
	msgHEADER HEADER;
	int ID;
	int TEMP[2];
};
struct msgSTARCREATE {
	msgHEADER HEADER;
	int ID;
	int X;
	int Y;
};
struct msgSTARDELETE {
	msgHEADER HEADER;
	int ID;
	int TEMP[2];
};
struct msgMOVE {
	msgHEADER HEADER;
	int ID;
	int X;
	int Y;
};


struct stPLAYER {
	SOCKET sock;
	CRingBuffer recvQ;
	CRingBuffer sendQ;
	int ID;
	int X;
	int Y;
};

// 전역변수
stPLAYER g_players[dfMAXPLAYER]; // 플레이어
SOCKET g_listenSock; // 리슨소켓
int g_genID = 0; // 아이디 생성 카운터
int g_playerCnt = 0; // 플레이어 현재 접속자
fd_set g_rset, g_wset; // 읽기, 쓰기 셋트
CQueue <stPLAYER *> g_delQueue; // 모아서 disconnect

// 함수
BOOL AcceptProc();
BOOL RecvProc(stPLAYER *pPlayer);	// recvQ에 있는것을 처리
BOOL SendProc(stPLAYER *pPlayer);	// sendQ에 있는것을 처리
BOOL SendUnicast(stPLAYER *pPlayer, char *msg, int size = 16);	// pPlayer에게만 보내기
BOOL SendBroadcast(stPLAYER *pPlayerExc, char *msg, int size = 16); // pPlayerExc를 제외한 모든 플레이어에게 보내기
void Disconnect(stPLAYER *pPlayer); // 플레이어 연결 끊기

#ifdef RENDER
void Render();
void gotoxy(int x, int y);
#endif // RENDER


int main() {
	// 한글설정
	setlocale(LC_ALL, "KOREAN");

	// 변수
	int ret; // 연결할때 1회용 return
	int sRet; // select retrun
	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;
	// 플레이어 초기화
	for (int i = 0; i < dfMAXPLAYER; i++) {
		g_players[i].sock = INVALID_SOCKET;
	}

#pragma region INIT_NETWORK
	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wprintf_s(L"WSAStartup() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// 리슨
	g_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_listenSock == INVALID_SOCKET) {
		wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// 이서버 설정 모든IP접근(0.0.0.0) dfSERVERPORT번 포트
	SOCKADDR_IN servAddr;
	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(dfSERVERPORT);
	InetPton(AF_INET, L"0.0.0.0", &servAddr.sin_addr);

	// bind()
	ret = bind(g_listenSock, (SOCKADDR *) &servAddr, sizeof(servAddr));
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"bind() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// listen()
	ret = listen(g_listenSock, SOMAXCONN);
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"listen() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// 논블록킹 소켓으로 전환
	u_long on = 1;
	ret = ioctlsocket(g_listenSock, FIONBIO, &on);
	if (ret == SOCKET_ERROR) {
		wprintf_s(L"ioctlsocket() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

#pragma endregion

	//써버오픈!
	while (true) {
		PRO_BEGIN(L"INIT select set");

		// select 모델 초기화
		FD_ZERO(&g_rset);
		FD_ZERO(&g_wset);
		FD_SET(g_listenSock, &g_rset); // 리슨을 읽기셋에 등록
		// 플레이어를 셋트에 등록
		for (int i = 0; i < dfMAXPLAYER; i++) {
			if (g_players[i].sock != INVALID_SOCKET) {
				// 읽기!
				FD_SET(g_players[i].sock, &g_rset);
				// 쓰기!
				if (g_players[i].sendQ.GetUseSize() > 0) {
					FD_SET(g_players[i].sock, &g_wset);
				}
			}
		}
		PRO_END(L"INIT select set");


		PRO_BEGIN(L"select()");
		// select()
		sRet = select(0, &g_rset, &g_wset, NULL, &tval);
		if (sRet == SOCKET_ERROR) {
			wprintf_s(L"select() errcode[%d]\n", WSAGetLastError());
			// 써버 폐업
			closesocket(g_listenSock);
			WSACleanup();
			return 1;
		}
		PRO_END(L"select()");

		//TEMP
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) {
			//ESC누르면 써버 꺼짐!
			break;
		}

		// send
		for (int i = 0; i < dfMAXPLAYER; i++) {
			if (FD_ISSET(g_players[i].sock, &g_wset)) {
				SendProc(&g_players[i]);
			}
		}

		// 리슨에 받을것이 있으면! accept 처리
		if (FD_ISSET(g_listenSock, &g_rset))
			if (AcceptProc() == FALSE)
				break;

		// recv
		for (int i = 0; i < dfMAXPLAYER; i++) {
			if (FD_ISSET(g_players[i].sock, &g_rset)) {


				RecvProc(&g_players[i]);
			}
		}
		// 큐에 모인거 지우기
		while (!g_delQueue.empty()) {
			stPLAYER *cur = g_delQueue.front();
			g_delQueue.pop();
			Disconnect(cur);
		}

#ifdef RENDER
		Render();
#endif
	}

	// 써버 폐업
	PRO_PRINT("LOG.log");
	closesocket(g_listenSock);
	WSACleanup();
	return 0;
}

BOOL AcceptProc() {
	PRO_BEGIN(L"AcceptProc()");


	// accept 성공하면 플레이어 만들어주기
	msgIDSET msgID;
	msgSTARCREATE msgSC;

	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	SOCKET clientSock = accept(g_listenSock, (SOCKADDR *) &clientAddr, &addrlen);
	if (clientSock == INVALID_SOCKET) {
		wprintf_s(L"accept() errcode[%d]\n", WSAGetLastError());
		PRO_END(L"AcceptProc()");
		return FALSE;
	}
	wprintf_s(L"accept() 성공 player : %d명\n", ++g_playerCnt);

	// TODO 플레이어 만들기
	// 1. ID, 좌표계 설정 - 서버 palyers 등록
	// 2. 생성된 클라에게 ID및 정보 알려주기
	// 3. 이서버에 접속한 클라에게 이 플레이어 정보 (2랑 퉁!)
	// 4. 생성된 클라에게 기존에 있던 플레이어 정보
	for (int i = 0; i < dfMAXPLAYER; i++) {
		if (g_players[i].sock == INVALID_SOCKET) {
			// 1. 플레이어 생성
			g_players[i].ID = g_genID++;
			g_players[i].X = rand() % dfMAXWITDH;
			g_players[i].Y = rand() % dfMAXHEIGHT;
			g_players[i].recvQ.ClearBuffer();
			g_players[i].sendQ.ClearBuffer();
			g_players[i].sock = clientSock;

			// 2.1 ID 지정하는 패킷 보내기
			msgID.HEADER = 0;
			msgID.ID = g_players[i].ID;
			SendUnicast(&g_players[i], (char *) &msgID);

			// 2.2, 3 새로운 별생성 정보 보내기 (나포함 모두에게!)
			msgSC.HEADER = 1;
			msgSC.ID = g_players[i].ID;
			msgSC.X = g_players[i].X;
			msgSC.Y = g_players[i].Y;
			SendBroadcast(NULL, (char *) &msgSC);

			// 4. 접속한 플레이어에게 기존플레이어 정보 보내기 (나한테만! 내정보는 빼고)
			for (int j = 0; j < dfMAXPLAYER; j++) {
				if (g_players[j].sock != INVALID_SOCKET && i != j) {
					// 의문) FD_ISSET 체크 해야하나?
					msgSC.HEADER = 1;
					msgSC.ID = g_players[j].ID;
					msgSC.X = g_players[j].X;
					msgSC.Y = g_players[j].Y;
					// 새로운 플레이어에게 보내기
					SendUnicast(&g_players[i], (char *) &msgSC);
				}
			}
#ifdef PRINTACTION
			wprintf_s(L"accept() : ID( %2d ), X( %2d ), Y( %2d ), sock( %d )\n",
				g_players[i].ID, g_players[i].X, g_players[i].Y, g_players[i].sock);
#endif // PRINTACTION
			PRO_END(L"AcceptProc()");
			return TRUE;
		}
	}
	PRO_END(L"AcceptProc()");
	return FALSE;
		}
BOOL RecvProc(stPLAYER *pPlayer) {
	if (pPlayer == NULL) return FALSE;
	PRO_BEGIN(L"RecvProc()");

	// 큐에 모두 넣기
	char recvBuf[dfBUFFERSIZE];
	int recvRet;
	int enqRet;
	recvRet = recv(pPlayer->sock, recvBuf, dfBUFFERSIZE, 0);
	if (recvRet == SOCKET_ERROR) {
		DWORD err = GetLastError();
		if (err == WSAEWOULDBLOCK) {
			// 에러가 아님
		} else {
			wprintf_s(L"recv() errorcode[%d]\n", err);
			g_delQueue.push(pPlayer);
			PRO_END(L"RecvProc()");
			return FALSE;
		}
	}
	enqRet = pPlayer->recvQ.Enqueue(recvBuf, recvRet);


	// ============================
	// 큐처리
	// ---
	// 1. 헤더만큼 데이터가 있는지
	// 2. 헤더 내용물 확인
	// 3. 헤더에 따른 메시지물 확인
	// 4. 헤더를 버리고 메시지를 뽑기
	// 5. 메시지에 대한 로직
	// ---
	// 하지만 16Byte고정 임으로
	// 1. 16byte이상 왔는지 확인
	// 2. Peek
	// 3. 처리후 MoveFront(16)
	// ============================

	int peekRet;
	char packet[16];
	// 1. 16byte이상 왔는지 확인
	while (pPlayer->recvQ.GetUseSize() >= 16) {

		// 2. Peek
		peekRet = pPlayer->recvQ.Peek(packet, 16);
		// 처리
		msgMOVE *msg = (msgMOVE *) packet;
		if (msg->HEADER != 3) {
			PRO_END(L"RecvProc()");
			return FALSE;
		}
		pPlayer->X = msg->X;
		pPlayer->Y = msg->Y;
		SendBroadcast(pPlayer, (char *) msg);

		// 3. MoveFront
		pPlayer->recvQ.MoveFront(peekRet);
	}

	PRO_END(L"RecvProc()");
	return TRUE;
}

BOOL SendProc(stPLAYER *pPlayer) {
	if (pPlayer->sendQ.GetUseSize() <= 0) {
		return FALSE;
	}

	PRO_BEGIN(L"SendProc()");

	// ============================
	// 1. sendQ내용물 peek
	// 2. send() 시도
	// 3. 보내진 만큼만 큐에서 빼기
	// ============================

	char sendBuf[dfBUFFERSIZE]; // peek를 할 중간 버퍼
	int peekRet;	// peek한 사이즈
	int sendRet;	// send한 사이즈

	// 1. sendQ내용물 peek
	peekRet = pPlayer->sendQ.Peek(sendBuf, dfBUFFERSIZE);
	// 2. send() 시도
	sendRet = send(pPlayer->sock, sendBuf, peekRet, 0);
	if (sendRet == SOCKET_ERROR) {
		DWORD err = GetLastError();
		if (err == WSAEWOULDBLOCK) {
			// 에러가 아님
		} else {
			wprintf_s(L"send() errorcode[%d]\n", err);
			g_delQueue.push(pPlayer);
			PRO_END(L"SendProc()");
			return FALSE;
		}
	}

	// 3. 보내진 만큼만 큐에서 빼기
	pPlayer->sendQ.MoveFront(sendRet);

	PRO_END(L"SendProc()");
	return FALSE;
}

BOOL SendUnicast(stPLAYER *pPlayer, char *msg, int size) {
	PRO_BEGIN(L"SendUnicast()");
	// pPlayer에게만 보내기

	if (pPlayer == NULL || pPlayer->sock == INVALID_SOCKET || size <= 0) {
		// 이상한 데이터
		PRO_END(L"SendUnicast()");
		return FALSE;
	}
	// 큐에 넣기
	pPlayer->sendQ.Enqueue(msg, size);

	PRO_END(L"SendUnicast()");
	return TRUE;
}
BOOL SendBroadcast(stPLAYER *pPlayerExc, char *msg, int size) {
	PRO_BEGIN(L"SendBroadcast()");


	// pPlayerExc를 제외한 모든 플레이어에게 보내기
	// pPlayerExc 가 NULL이면 모든플레이어
	// 의문) FD_ISSET 체크 해야하나?

	BOOL nui;
	BOOL ret = TRUE;
	if (pPlayerExc == NULL) {
		for (int i = 0; i < dfMAXPLAYER; i++) {
			if (g_players[i].sock != INVALID_SOCKET) {
				nui = SendUnicast(&g_players[i], msg, size);
				if (nui == FALSE) ret = FALSE;
			}
		}
		PRO_END(L"SendBroadcast()");
		return ret;
	} else {
		for (int i = 0; i < dfMAXPLAYER; i++) {
			if (g_players[i].sock != INVALID_SOCKET &&
				g_players[i].sock != pPlayerExc->sock) {
				nui = SendUnicast(&g_players[i], msg, size);
				if (nui == FALSE) ret = FALSE;
			}
		}
		PRO_END(L"SendBroadcast()");
		return ret;
	}
}
void Disconnect(stPLAYER *pPlayer) {
	PRO_BEGIN(L"Disconnect()");
	// 플레이어 연결 끊기
	if (pPlayer->sock == INVALID_SOCKET || pPlayer == NULL) {
		PRO_END(L"Disconnect()");
		return;
	}
	g_playerCnt--;
	closesocket(pPlayer->sock);
	pPlayer->sock = INVALID_SOCKET;

	// 연결 끊었다고 브로드캐스팅
	msgSTARDELETE msgSD;
	msgSD.HEADER = 2;
	msgSD.ID = pPlayer->ID;

	SendBroadcast(pPlayer, (char *) &msgSD);
	PRO_END(L"Disconnect()");
	}


#ifdef RENDER
void Render() {
	system("cls");
	for (int i = 0; i < dfMAXPLAYER; i++) {
		if (g_players[i].sock != INVALID_SOCKET) {
			gotoxy(g_players[i].X, g_players[i].Y);
			wprintf_s(L"*");
		}
	}
}

void gotoxy(int x, int y) {
	COORD pos = {x,y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
#endif // RENDER