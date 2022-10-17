#include "NetworkCore.h"
#include "CQueue.h"
#include"AsyncSelect_NetDraw_Server.h"
#include "CMonitorGraphUnit.h"

// =========================
// 전역변수 - 서버
// g_wsaData
// g_sockAddr
// g_listenSock : 리슨 소켓
// =========================

WSADATA g_wsaData;		// 윈속
SOCKADDR_IN g_sockAddr;	// 소켓 정보
SOCKET g_listenSock;	// 리슨된 소켓
stPLAYER g_players[dfMAXPLAYER];

fd_set g_wset;	// 쓰기셋
fd_set g_rset;	// 읽기셋

CQueue<stPLAYER *> g_delQ; // 지울 플레이어 큐

int g_ClientCount = 0;
int g_PacketCount = 0;
extern CMonitorGraphUnit *g_pClientCount;
extern CMonitorGraphUnit *g_pPacketCount;


BOOL NetworkInitServer() {
	int setRet;// 소켓 연결에 나오는 리턴

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &g_wsaData) != 0) {
		wprintf_s(L"WSAStartup() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// 소켓 기본정보
	memset(&g_sockAddr, 0, sizeof(g_sockAddr));
	g_sockAddr.sin_family = AF_INET;
	g_sockAddr.sin_port = htons(dfSERVERFORT);
	InetPton(AF_INET, dfADDRALL, &g_sockAddr.sin_addr);

	// socket()
	g_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_listenSock == INVALID_SOCKET) {
		wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// bind()
	setRet = bind(g_listenSock, (SOCKADDR *) &g_sockAddr, sizeof(g_sockAddr));
	if (setRet == SOCKET_ERROR) {
		wprintf_s(L"bind() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// listen()
	setRet = listen(g_listenSock, SOMAXCONN);
	if (setRet == SOCKET_ERROR) {
		wprintf_s(L"listen() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// 비동기 키기
	u_long on = 1;
	setRet = ioctlsocket(g_listenSock, FIONBIO, &on);
	if (setRet == SOCKET_ERROR) {
		wprintf_s(L"ioctlsocket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// player 초기화
	for (int i = 0; i < dfMAXPLAYER; i++) {
		g_players[i].sock = INVALID_SOCKET;
	}

	return TRUE;
}

void SelectProc() {
	int selRet;
	// select 초기화
	FD_ZERO(&g_rset);
	FD_SET(g_listenSock, &g_rset);
	for (int i = 0; i < dfMAXPLAYER; i++) {
		if (g_players[i].sock != INVALID_SOCKET) {
			FD_SET(g_players[i].sock, &g_rset);

		}
	}

	// select()
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	selRet = select(0, &g_rset, &g_wset, NULL, &timeout);
	if (selRet == SOCKET_ERROR) {
		DWORD errcode = WSAGetLastError();
		if (errcode == WSAEWOULDBLOCK) {
			// 에러가 아님
		} else {
			wprintf_s(L"select() errcode[%d]\n", errcode);
			return;
		}
	}
	// 패킷모니터링
	g_PacketCount = 0;


	// 백로그 큐에 뭔가 왔다! - Accept
	if (FD_ISSET(g_listenSock, &g_rset)) {
		AcceptProc();
	}

	// recv() - 클라에게서 패킷이 왔다
	for (int i = 0; i < dfMAXPLAYER; i++) {
		if (FD_ISSET(g_players[i].sock, &g_rset)) {
			RecvProc(&g_players[i]);
		}
	}

	FD_ZERO(&g_wset);
	for (int i = 0; i < dfMAXPLAYER; i++) {
		if (g_players[i].sock != INVALID_SOCKET) {
			if (g_players[i].sendQ.GetUseSize() > 0) {
				FD_SET(g_players[i].sock, &g_wset);
			}
		}
	}
	// send() 
	for (int i = 0; i < dfMAXPLAYER; i++) {
		if (FD_ISSET(g_players[i].sock, &g_wset)) {
			SendProc(&g_players[i]);
		}
	}

	// 접속 끊기
	while (g_delQ.empty() == false) {
		stPLAYER *pCur = g_delQ.front();
		Disconnect(pCur);
		g_delQ.pop();
	}

	// 모니터링
	g_pClientCount->InsertData(g_ClientCount);
	g_pPacketCount->InsertData(g_PacketCount);

}

void AcceptProc() {
	// 백로그큐에 있는 소켓을 accept하기
	for (int i = 0; i < dfMAXPLAYER; i++) {
		if (g_players[i].sock == INVALID_SOCKET) {
			// accept()
			int addrlen = sizeof(g_sockAddr);
			g_players[i].sock = accept(g_listenSock, (SOCKADDR *) &g_sockAddr, &addrlen);
			if (g_players[i].sock == INVALID_SOCKET) {
				// 에러
				wprintf_s(L"accept() errcode[%d]\n", WSAGetLastError());
				return;
			}

			// 초기화
			g_players[i].sendQ.ClearBuffer();
			g_players[i].recvQ.ClearBuffer();


			// 플레이어 카운트 추가
			g_ClientCount++;
			return;
		}
	}
}

void SendProc(stPLAYER *stpPlayer) {
	// sendQ에 있는 send할껄 send()
		// 변수
	char buffer[1000];
	int peekRet;
	int sendRet;

	while (stpPlayer->sendQ.GetUseSize() > 0) {
		peekRet = stpPlayer->sendQ.Peek(buffer, 1000);
		sendRet = send(stpPlayer->sock, buffer, peekRet, 0);
		if (sendRet == SOCKET_ERROR) {
			DWORD errcode = GetLastError();
			if (errcode == WSAEWOULDBLOCK) {

				break;
			} else {
				// disconnect
				g_delQ.push(stpPlayer);
				break;
			}
		}
		g_PacketCount++; // 패킷 모니터링
		stpPlayer->sendQ.MoveFront(peekRet);
	}
}

void SendUnicast(stPLAYER *stpPlayer, char *chpBuffer, int iSize) {
	// 무결성 검사
	if (chpBuffer == NULL || iSize < 0) return;
	if (stpPlayer->sock == INVALID_SOCKET)return;
	if (stpPlayer == NULL)return;

	// stpPlayer에게 chpBuffer 보내기
	stpPlayer->sendQ.Enqueue(chpBuffer, iSize);
}

void SendBroadcast(stPLAYER *stpPlayerEx, char *chpBuffer, int iSize) {
	if (chpBuffer == NULL || iSize < 0) return;

	if (stpPlayerEx == NULL) {
		// 모든 플레이어 에게 보내기
		for (int i = 0; i < dfMAXPLAYER; i++) {
			if (g_players[i].sock != INVALID_SOCKET) {
				SendUnicast(&g_players[i], chpBuffer, iSize);
			}
		}
	} else {
		// stpPlayerEx를 제외하고 chpBuffer 보내기
		for (int i = 0; i < dfMAXPLAYER; i++) {
			if (g_players[i].sock != INVALID_SOCKET &&
				g_players[i].sock != stpPlayerEx->sock) {
				SendUnicast(&g_players[i], chpBuffer, iSize);
			}
		}
	}
}


void RecvProc(stPLAYER *stpPlayer) {
	if (stpPlayer == NULL) return;

	// 큐에 모두 넣기
	char buffer[dfBUFFERSIZE];
	int recvRet;
	int enqRet;
	recvRet = recv(stpPlayer->sock, buffer, dfBUFFERSIZE, 0);
	if (recvRet == SOCKET_ERROR) {
		DWORD err = GetLastError();
		if (err == WSAEWOULDBLOCK) {
			// 에러가 아님
		} else {
			// 지우기
			wprintf_s(L"recv() errorcode[%d]\n", err);
			g_delQ.push(stpPlayer);
			return;
		}
	}
	g_PacketCount++; // 패킷 모니터링
	enqRet = stpPlayer->recvQ.Enqueue(buffer, recvRet);

	// ============================
	// 큐처리
	// ---
	// 1. 헤더만큼 데이터가 있는지
	// 2. 헤더 내용물 확인
	// 3. 헤더에 따른 메시지물 확인
	// 4. 헤더를 버리고 메시지를 뽑기
	// 5. 메시지에 대한 로직
	// ============================

	int peekRet;
	stHEADER *stpHeader;		// 마샬링용
	st_DRAW_PACKET *stpDPacekt; // 마샬링용

	while (stpPlayer->recvQ.GetUseSize() >= sizeof(stHEADER) + sizeof(st_DRAW_PACKET)) {

		peekRet = stpPlayer->recvQ.Peek(buffer, sizeof(stHEADER));
		if (peekRet < sizeof(stHEADER)) { return; }

		stpHeader = (stHEADER *) buffer;
		peekRet = stpPlayer->recvQ.Peek(buffer, sizeof(stHEADER) + stpHeader->Len);
		if (peekRet < sizeof(stHEADER) + stpHeader->Len) { return; }

		stpDPacekt = (st_DRAW_PACKET *) (buffer + sizeof(stHEADER));

		stpPlayer->recvQ.MoveFront(peekRet);

		// sendQ에 보낼것 넣기
		SendBroadcast(NULL, buffer, peekRet);
	}
}

void Disconnect(stPLAYER *stpPlayer) {
	if (stpPlayer->sock == INVALID_SOCKET) return;
	if (stpPlayer == NULL) return;

	// 연결끊기
	closesocket(stpPlayer->sock);
	// 초기화
	stpPlayer->sock = INVALID_SOCKET;

	// 모니터링
	// 플레이어 카운트 감소
	g_ClientCount--;

}


void NetworkCloseServer() {
	WSACleanup();
	closesocket(g_listenSock);
}
