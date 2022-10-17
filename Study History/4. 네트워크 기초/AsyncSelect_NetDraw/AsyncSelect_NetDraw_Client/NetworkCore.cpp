#include "NetworkCore.h"

// =========================
// 전역변수 - 클라이언트
// g_wsaData
// g_sockAddr
// g_sock : 소켓
// g_recvQ, g_sendQ : recv(), send() 링버퍼
// g_bSendFlag : 현재 send()를 호출 가능한 상태인지?
// =========================

WSADATA g_wsaData;		// 윈속
SOCKADDR_IN g_sockAddr;	// 소켓 정보
SOCKET g_sock;			// 서버와 connect된 소켓
CRingBuffer g_recvQ;	// 수신 링버퍼
CRingBuffer g_sendQ;	// 송신 링버퍼
BOOL g_bSendFlag;		// 지금 "당장" 보낼 수 있는 지 확인



BOOL NetworkInitClient(HWND hWnd, const WCHAR *SERVERIP) {
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
	InetPton(AF_INET, SERVERIP, &g_sockAddr.sin_addr);

	// socket()
	g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_sock == INVALID_SOCKET) {
		wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// connect()
	setRet = connect(g_sock, (SOCKADDR *) &g_sockAddr, sizeof(g_sockAddr));
	if (setRet == SOCKET_ERROR) {
		wprintf_s(L"connect() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}


	// WSAAsyncSelect()
	setRet = WSAAsyncSelect(g_sock, hWnd, UM_NETWORK,
		FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);


	return TRUE;
}

void ReadEvent(HWND hWnd) {
	// rece버퍼에 무언가 남아있으면 들어옴
	// 1. recvQ에 넣고
	// 2. 적당히 처리
	// 3. 로직

	char buffer[1000];
	int recvRet;
	int enqRet;

	recvRet = recv(g_sock, buffer, 1000, 0);
	if (recvRet == SOCKET_ERROR) {
		DWORD errcode = GetLastError();
		if (errcode == WSAEWOULDBLOCK) {

		} else {
			// TODO disconnect
			return;
		}
	}

	enqRet = g_recvQ.Enqueue(buffer, recvRet);


	// 패킷의 내용대로 그리기
	int peekRet;	// 엿보기 리턴
	char chpHeaderBuf[sizeof(stHEADER)];	// 헤더 버퍼
	char chpPacketBuf[sizeof(stHEADER) + sizeof(st_DRAW_PACKET)]; // 패킷 버퍼 sizeof(stHEADER) + sizeof(st_DRAW_PACKET) 만큼 엿봐야함
	stHEADER *stpHeader; // 마샬링용
	st_DRAW_PACKET *stpPacket; // 마샬링용

	// recvQ에 있는걸 다 빼기
	while (g_recvQ.GetUseSize() >= sizeof(stHEADER) + sizeof(st_DRAW_PACKET)) {

		// 헤더 엿보기
		peekRet = g_recvQ.Peek(chpHeaderBuf, sizeof(stHEADER)); // 헤더 엿보기
		if (peekRet != sizeof(stHEADER)) { 
			// 헤더도 안왔음
			return; }

		// 헤더 내용데로 내용물 엿보기
		stpHeader = (stHEADER *) chpHeaderBuf; // 헤더 마샬링
		peekRet = g_recvQ.Peek(chpPacketBuf, sizeof(stHEADER) + stpHeader->Len); // 헤더와 헤더의 길이만큼 엿보기
		if (peekRet != sizeof(stHEADER) + stpHeader->Len) {
			// 완전한 패킷이 아님
			return; }

		// 정보 다뻇으니 큐에서 빼기
		stpPacket = (st_DRAW_PACKET *) (chpPacketBuf + sizeof(stHEADER));// 마샬링
		g_recvQ.MoveFront(sizeof(stHEADER) + stpHeader->Len);

		// 그림그리기
		HDC hdc = GetDC(hWnd);
		//SelectObject(hdc,  GetStockObject(BLACK_PEN) );
		// 직선 선그리기
		MoveToEx(hdc, stpPacket->iStartX, stpPacket->iStartY, NULL);	// 시작점
		LineTo(hdc, stpPacket->iEndX, stpPacket->iEndY);				// 끝점
		// DC연결 해지
		ReleaseDC(hWnd, hdc);
	}
}

// sendQ에 있는걸 모두 보내기
void WriteEvent() {
	// 이함수가 호출되었으면
	// 연결이 시작되었거나
	// 못보내는 상황이 있다가 보낼 수 있는 상황으로 왔을때
	// g_bSendFlag = TRUE;
	// 	   + 추가적으로 그냥 send할때
	// sendQ에 있는걸 "모두" 보내기

	// 변수
	char buffer[1000];
	int peekRet;
	int sendRet;

	g_bSendFlag = TRUE;

	while (g_sendQ.GetUseSize() > 0) {
		peekRet = g_sendQ.Peek(buffer, 1000);
		sendRet = send(g_sock, buffer, peekRet, 0);
		if (sendRet == SOCKET_ERROR) {
			DWORD errcode = GetLastError();
			if (errcode == WSAEWOULDBLOCK) {
				// 보낼 수 없는 상 황 이 옴
				g_bSendFlag = FALSE;
				break;
			} else {
				// TODO : disconnect
				break;
			}
		}
		g_sendQ.MoveFront(peekRet);
	}
}

BOOL SendPacket(char *chpPacket, int iSize) {
	// 1. 일단 sendQ에 넣는다
	// 2. g_bSendFlag 
	//	- TRUE : 최대한 보낸다 retrun TRUE
	//	- FALSE : 넘긴다 retrun FALSE

	// 1. 일단 sendQ에 넣는다
	int qRet = g_sendQ.Enqueue(chpPacket, iSize);

	if (g_bSendFlag == FALSE) {
		return FALSE;
	}

	// sendQ에 있는걸 최대한 모두 보내기
	WriteEvent();

	return TRUE;
}

void NetworkCloseClient() {
	WSACleanup();
	closesocket(g_sock);
}
