#include "NetworkCore.h"
#include "PacketProcess.h" // 패킷 처리용

#define dfDIRECTBUFFER

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
CRingBuffer g_recvQ(1000);	// 수신 링버퍼
CRingBuffer g_sendQ(1000);	// 송신 링버퍼
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
	g_sockAddr.sin_port = htons(dfSERVERPORT);
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

BOOL ReadEvent() {
	// rece버퍼에 무언가 남아있으면 들어옴
	// 1. recvQ에 넣고
	// 2. 적당히 처리
	// 3. 로직

	char buffer[1000];
	int recvRet;
	int enqRet;

#ifndef dfDIRECTBUFFER


	recvRet = recv(g_sock, buffer, 1000, 0);
	if (recvRet == SOCKET_ERROR) {
		DWORD errcode = GetLastError();
		if (errcode == WSAEWOULDBLOCK) {
			return TRUE;
		} else {
			// disconnect
			return FALSE;
		}
	}

	// 링버퍼 확인하기
	// 얼마나? -> 프로토콜 보다 큰 메시지가 왔을때
	// 이유 : 지금 링버퍼가 남아있다면, 이상한 메시지일 가능성이 큼
	// 얼마나 -> 다비워
	if (g_recvQ.GetUseSize() > sizeof(stPACKET_SC_CREATE_MY_CHARACTER) + sizeof(st_PACKET_HEADER))
		g_recvQ.ClearBuffer();

	enqRet = g_recvQ.Enqueue(buffer, recvRet);
#else
	recvRet = recv(g_sock, g_recvQ.GetRearBufferPtr(), g_recvQ.DirectEnqueueSize(), 0);
	if (recvRet == SOCKET_ERROR) {
		DWORD errcode = GetLastError();
		if (errcode == WSAEWOULDBLOCK) {
			return TRUE;
		} else {
			// disconnect
			return FALSE;
		}
	}
	g_recvQ.MoveRear(recvRet);

#endif // !dfDIRECTBUFFER

	CPacket clPacket;

	// 모든 내용을 처리
	while (true) {

		// 패킷 내용 처리
		st_PACKET_HEADER *pHeader; // 마샬링용 헤더

		// 헤더 확인하기
		int peekRet = g_recvQ.Peek(buffer, sizeof(st_PACKET_HEADER));
		if (peekRet < sizeof(st_PACKET_HEADER)) { break; }

		// 메시지 확인하기
		pHeader = (st_PACKET_HEADER *) buffer;
		if (pHeader->byCode != 0x89) {
			// 잘못된 코드! 얼마나 빼줘야하나?
			// 1바이트 빼주고 다시 시도
			g_recvQ.MoveFront(1);
			continue;
		}
		int iMsgSize = pHeader->bySize + sizeof(st_PACKET_HEADER);
		peekRet = g_recvQ.Peek(buffer, iMsgSize);
		if (peekRet < iMsgSize) { return FALSE; }

		clPacket.Clear();
		clPacket.PutData(buffer, iMsgSize);


		// 패킷 처리 함수 호출
		if (SC_PacketProc(&clPacket) == FALSE) {
			// 처리하지 못한 패킷
			// 잘못된 패킷???
			// 버퍼가 꽊찻나?
			break;
		}

		// 일 처리 끝났으면 큐에서 빼기
		g_recvQ.MoveFront(peekRet);

	}
	return TRUE;
}

// sendQ에 있는걸 모두 보내기
BOOL WriteEvent() {
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
#ifndef dfDIRECTBUFFER

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
				// disconnect
				return FALSE;
			}
		}
		g_sendQ.MoveFront(peekRet);
	}
#else
	while (g_sendQ.GetUseSize() > 0) {
		sendRet = send(g_sock, g_sendQ.GetFrontBufferPtr(), g_sendQ.DirectDequeueSize(), 0);
		if (sendRet == SOCKET_ERROR) {
			DWORD errcode = GetLastError();
			if (errcode == WSAEWOULDBLOCK) {
				// 보낼 수 없는 상 황 이 옴
				g_bSendFlag = FALSE;
				break;
			} else {
				// disconnect
				return FALSE;
			}
		}
		g_sendQ.MoveFront(sendRet);
	}
#endif //!dfDIRECTBUFFER
	return TRUE;
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
