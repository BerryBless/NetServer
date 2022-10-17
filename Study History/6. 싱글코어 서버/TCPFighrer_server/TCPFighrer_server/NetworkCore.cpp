#include "pch.h"
#include "NetworkCore.h"
#include "PacketProcess.h"
#include "CCalcFPSModule.h"
// =========================
// 전역변수 - 서버
// g_wsaData
// g_listenAddr
// g_listenSock : 리슨 소켓
// =========================

WSADATA g_wsaData;		// 윈속
SOCKADDR_IN g_listenAddr;	// 리슨 소켓 정보
SOCKET g_listenSock;	// 리슨 소켓

// 전역변수 - 관리
std::map<SOCKET, CSession *> g_sessionMap; // 세션맵
std::queue<SOCKET> g_delResvQ; // 삭제할 큐
extern std::list<CCharacter *> g_sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X]; // 섹터
int g_IDGenerator = 0;
extern int g_syncCnt;

// 모니터링
CObjectPool <CSession> g_sessionPool(0, true);
CCalcFPSModule g_sendTPS;
CCalcFPSModule g_recvTPS;

BOOL NetworkInitServer() {
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkInitServer()..");
	int setRet;// 소켓 연결에 나오는 리턴

		// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &g_wsaData) != 0) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// WSAStartup() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"WSAStartup OK..");

	// 소켓 기본정보
	memset(&g_listenAddr, 0, sizeof(g_listenAddr));
	g_listenAddr.sin_family = AF_INET;
	g_listenAddr.sin_port = htons(dfNETWORK_PORT);
	InetPton(AF_INET, L"0.0.0.0", &g_listenAddr.sin_addr);
	_LOG(dfLOG_LEVEL_ERROR, L"PORT [%d] g_listenAddr.sin_addr [%x] ", dfNETWORK_PORT, g_listenAddr.sin_addr);

	// socket()
	g_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_listenSock == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// socket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"g_listenSock OK [%lld]..", g_listenSock);

	// bind()
	setRet = bind(g_listenSock, (SOCKADDR *) &g_listenAddr, sizeof(g_listenAddr));
	if (setRet == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// bind() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"bind() OK [%d] ..", setRet);

	// listen()
	setRet = listen(g_listenSock, SOMAXCONN);
	if (setRet == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// listen() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"listen() OK [%d] ..", setRet);

	// 비동기 키기
	u_long on = 1;
	setRet = ioctlsocket(g_listenSock, FIONBIO, &on);
	if (setRet == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// ioctlsocket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"FIONBIO ON [%d] ..", setRet);
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkInitServer() OK..\n");
	return TRUE;
}

BOOL NetworkPorc() {
	_LOG(dfLOG_LEVEL_DEBUG, L"NetworkPorc()..");

	CSession *pSession;
	SOCKET UserTable_SOCKET[FD_SETSIZE];
	int iSockCnt = 0;
	//g_syncCnt = 0;

	//---------------------------
	// 64개씩 나눠서 하기!
	//---------------------------
	FD_SET ReadSet;
	FD_SET WriteSet;
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
	_LOG(dfLOG_LEVEL_DEBUG, L"INIT SET OK..");

	//---------------------------
	// 리슨소켓 넣기
	//---------------------------
	FD_SET(g_listenSock, &ReadSet);
	UserTable_SOCKET[iSockCnt] = g_listenSock;
	iSockCnt++;
	_LOG(dfLOG_LEVEL_DEBUG, L"INSERT LISTEN SOCK OK..");

	//---------------------------
	// 그외 세션 넣기
	//---------------------------
	_LOG(dfLOG_LEVEL_DEBUG, L"SELECT START..");
	for (auto seIter = g_sessionMap.begin(); seIter != g_sessionMap.end();) {
		pSession = seIter->second;
		++seIter;
		// 세션 소켓 등록
		UserTable_SOCKET[iSockCnt] = pSession->_sock;

		FD_SET(pSession->_sock, &ReadSet);
		if (pSession->_sendQ.GetUseSize() > 0)
			FD_SET(pSession->_sock, &WriteSet);

		iSockCnt++;
		//---------------------------
		// select 처리가능한 최대치 도달
		//---------------------------
		if (FD_SETSIZE <= iSockCnt) {
			// select()
			_LOG(dfLOG_LEVEL_DEBUG, L"CALL SelectProc() iSockCnt[%d] ", iSockCnt);
			SelectProc(UserTable_SOCKET, &ReadSet, &WriteSet);
			_LOG(dfLOG_LEVEL_DEBUG, L"SelectProc() iSockCnt[%d] OK..", iSockCnt);

			// 초기화
			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);
			memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
			iSockCnt = 0;
			_LOG(dfLOG_LEVEL_DEBUG, L"RESET SET OK..");

			// 리슨소켓 넣기
			FD_SET(g_listenSock, &ReadSet);
			UserTable_SOCKET[iSockCnt] = g_listenSock;
			iSockCnt++;
			_LOG(dfLOG_LEVEL_DEBUG, L"INSERT LISTEN SOCK OK..");
		}
	}
	//---------------------------
	// 남아있는 세션도 챙겨주기
	//---------------------------
	if (iSockCnt > 0) {
		_LOG(dfLOG_LEVEL_DEBUG, L"CALL SelectProc() iSockCnt[%d] ", iSockCnt);
		SelectProc(UserTable_SOCKET, &ReadSet, &WriteSet);
		_LOG(dfLOG_LEVEL_DEBUG, L"SelectProc() iSockCnt[%d] OK..", iSockCnt);
	}
	_LOG(dfLOG_LEVEL_DEBUG, L"SELECT OK..");

	//---------------------------
	// 에러가나서 지워야할걸 담아두었다가 이때 모두 삭제
	//---------------------------
	while (g_delResvQ.empty() == false) {
		SOCKET delsock = g_delResvQ.front();
		g_delResvQ.pop();
		Disconnect(delsock);
	}


	return TRUE;
}

BOOL SelectProc(SOCKET *pTableSocket, FD_SET *pReadSet, FD_SET *pWriteSet) {
	if (pTableSocket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// SelectProc : pTableSocket IS NULL.");
		CRASH();
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"\nSelectProc()..\n");

	//---------------------------
	// 지역변수
	//---------------------------
	CSession *pSession;
	timeval timeout;

	//---------------------------
	// 대기시간설장
	//---------------------------
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	//---------------------------
	// select()
	//---------------------------
	int selRet = select(0, pReadSet, pWriteSet, NULL, &timeout);
	if (selRet > 0) {
		//---------------------------
		// 0번째는 무조건 리슨체크
		// 백로그 큐에 뭔가 왔다! - Accept
		//---------------------------
		if (FD_ISSET(g_listenSock, pReadSet)) {
			if (AcceptProc()) {
				_LOG(dfLOG_LEVEL_DEBUG, L"Accept Ok..");
			} else {
				_LOG(dfLOG_LEVEL_ERROR, L"Accept FAIL..");
			}
		}
		//---------------------------
		// 클라이언트 체크
		//---------------------------
		for (int i = 1; i < FD_SETSIZE; i++) {
			if (pTableSocket[i] == INVALID_SOCKET) continue;

			//---------------------------
			// 읽기
			//---------------------------
			if (FD_ISSET(pTableSocket[i], pReadSet)) {
				pSession = FindSession(pTableSocket[i]);
				if (pSession == NULL) {
					_LOG(dfLOG_LEVEL_ERROR, L"///// pReadSet) COULD NOT FIND SESSION");
					CRASH();
				}
				_LOG(dfLOG_LEVEL_DEBUG, L"SID[%d] CALL RECV()", pSession->_SID);
				if (RecvProc(pSession)) {
					_LOG(dfLOG_LEVEL_DEBUG, L"SID[%d] RECV() OK..", pSession->_SID);
				} else {
					_LOG(dfLOG_LEVEL_WARNING, L"///// SID[%d] RECV() ERROR", pSession->_SID);
				}
			}
			//---------------------------
			// 쓰기
			//---------------------------
			if (FD_ISSET(pTableSocket[i], pWriteSet)) {
				pSession = FindSession(pTableSocket[i]);
				if (pSession == NULL) {
					_LOG(dfLOG_LEVEL_ERROR, L"///// pWriteSet) COULD NOT FIND SESSION");
					CRASH();
				}

				_LOG(dfLOG_LEVEL_DEBUG, L"SID[%d] CALL SEND()", pSession->_SID);
				if (SendProc(pSession)) {
					_LOG(dfLOG_LEVEL_DEBUG, L"SID[%d] SEND() OK..", pSession->_SID);
				} else {
					_LOG(dfLOG_LEVEL_WARNING, L"///// SID[%d] SEND() ERROR", pSession->_SID);
				}
			}
		}
	} else {
		//---------------------------
		// 할게 없음
		//---------------------------
		return FALSE;
	}

	return TRUE;
}

BOOL AcceptProc() {
	_LOG(dfLOG_LEVEL_DEBUG, L"\nAcceptProc()..");

	//---------------------------
	// 	   지역변수
	//---------------------------
	// TODO 클라이언트
	SOCKET sock;// 임시 소켓
	SOCKADDR_IN addr; // 임시 주소
	int addrlen = sizeof(addr);

	//---------------------------
	// accept()
	//---------------------------
	_LOG(dfLOG_LEVEL_WARNING, L"CALL ACCEPT() :: ID [%d]", g_IDGenerator);
	sock = accept(g_listenSock, (SOCKADDR *) &addr, &addrlen);
	if (sock == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// ACCEPT() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	//---------------------------
	//  캐릭터 생성
	//---------------------------
	_LOG(dfLOG_LEVEL_DEBUG, L"Create Session\n");
	CSession *pSession = CreateSession(sock);
	g_IDGenerator++;
	pSession->_SID = g_IDGenerator;
	pSession->_sock = sock;
	pSession->_recvQ.ClearBuffer();
	pSession->_sendQ.ClearBuffer();
	pSession->_LastRecvTime = timeGetTime();
	if (InstantiateCharacter(pSession)) {

		_LOG(dfLOG_LEVEL_DEBUG, L"Create Session OK..\n");
	} else {
		_LOG(dfLOG_LEVEL_ERROR, L"Create Session FAILE..\n");
		return FALSE;
	}
	return TRUE;
}

#pragma region SEND


BOOL SendProc(CSession *pSession) {
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"//////SendProc() pSession IS NULL");
		CRASH();
	}
	_LOG(dfLOG_LEVEL_DEBUG, L"\nSendProc(%d)..", pSession->_SID);
	g_sendTPS.Check();
	//---------------------------
	// 	   지역변수
	//---------------------------
	int sendRet;

	//---------------------------
	// 	   send()
	//---------------------------
	sendRet = send(pSession->_sock, pSession->_sendQ.GetFrontBufferPtr(), pSession->_sendQ.DirectDequeueSize(), 0);
	if (sendRet == SOCKET_ERROR) {
		DWORD errcode = GetLastError();
		if (errcode != WSAEWOULDBLOCK) {
			switch (errcode) {
			case WSAEOPNOTSUPP:
			case WSAECONNRESET:
				_LOG(dfLOG_LEVEL_WARNING, L"////// recv() errcode[%d]\n", errcode);
				break;
			default:
				_LOG(dfLOG_LEVEL_ERROR, L"////// recv() errcode[%d]\n", errcode);
				break;
			}
			//---------------------------
			// 큐에 넣어놨다가 나중에 지우기
			//---------------------------
			g_delResvQ.push(pSession->_sock);
			return FALSE;
		}
	}
	//---------------------------
	// 	   보낸만큼 sendQ 뒤로...
	//---------------------------
	pSession->_sendQ.MoveFront(sendRet);
	_LOG(dfLOG_LEVEL_DEBUG, L"SEND [%d byte]\n", sendRet);
	return TRUE;
}

void SendUnicast(CSession *pSession, st_PACKET_HEADER *pHeader, CPacket *pPacket) {
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// SendUnicast() :: pSession IS NULL");
		return;
	}
	//---------------------------
	// pSession의 send 링버퍼에 메시지 넣기
	//---------------------------
	int enq;
	enq = pSession->_sendQ.Enqueue((char *) pHeader, sizeof(st_PACKET_HEADER));
	enq += pSession->_sendQ.Enqueue(pPacket->GetBufferPtr(), pPacket->GetDataSize());
	if (sizeof(st_PACKET_HEADER) + pPacket->GetDataSize() > enq) {
		_LOG(dfLOG_LEVEL_ERROR, L"//////[%d]._sendQ.Enqueue [%d byte] ", pSession->_SID, enq);
		_LOG(dfLOG_LEVEL_ERROR, L"\tDisconnect SID :: [%d]", pSession->_SID);
		CCharacter *pCharacter = FindCharacter(pSession->_SID);
		if (pCharacter != NULL) {
			_LOG(dfLOG_LEVEL_ERROR, L"\tSector Client Count [%d]", g_sector[pCharacter->_curSecPos._Y][pCharacter->_curSecPos._X].size());
			DisconnectClient(pCharacter);
		}
		else
			Disconnect(pSession->_sock);

	}
	_LOG(dfLOG_LEVEL_DEBUG, L"[%d]._sendQ.Enqueue [%d byte] ", pSession->_SID, enq);
	_LOG(dfLOG_LEVEL_DEBUG, L"SendUnicast() OK..\n");
}

void SendSectorOne(int secX, int secY, CSession *pSessionEX, st_PACKET_HEADER *pHeader, CPacket *pPacket) {

	std::list<CCharacter *> *pSectorList; // 섹터 리스트
	CSession *pSession;
	pSectorList = &g_sector[secY][secX]; // 좌표로 리스트 얻기

	// 그섹터의 플레이어 순회
	for (auto iter = pSectorList->begin(); iter != pSectorList->end(); ++iter) {
		pSession = (*iter)->_pSession;
		// 예외
		if (pSession == pSessionEX) continue;
		SendUnicast(pSession, pHeader, pPacket);
	}



	_LOG(dfLOG_LEVEL_DEBUG, L"SendSectorOne(%d, %d) OK..\n", secX, secY);
}

void SendSectorAround(CCharacter *pCharacter, st_PACKET_HEADER *pHeader, CPacket *pPacket, BOOL SendMe) {
	std::list<CCharacter *> *pSectorList; // 섹터 리스트
	st_SECTOR_AROUND around; // 주변섹터 확인
	// 주변 얻기
	GetSectorAround(pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y, &around);
	// 주변 순회
	for (int i = 0; i < around._count; i++) {
		if (SendMe) {
			// 나포함
			SendSectorOne(around._around[i]._X, around._around[i]._Y, NULL, pHeader, pPacket);
		} else {
			// 나 제외
			SendSectorOne(around._around[i]._X, around._around[i]._Y, pCharacter->_pSession, pHeader, pPacket);
		}
	}
	_LOG(dfLOG_LEVEL_DEBUG, L"SendSectorAround(sid[%d] secPos[%d, %d]) OK..\n", pCharacter->_SID, pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y);
}

void SendBroadcast(CSession *pSessionEX, st_PACKET_HEADER *pHeader, CPacket *pPacket) {
	_LOG(dfLOG_LEVEL_DEBUG, L"SendBroadcast()..");
	//---------------------------
	// pSessionEX를 제외한 모든 대상에게
	//---------------------------
	CSession *pSession;
	for (auto iter = g_sessionMap.begin(); iter != g_sessionMap.end(); ++iter) {
		pSession = iter->second;
		if (pSession != pSessionEX)
			SendUnicast(pSession, pHeader, pPacket);
	}
}
#pragma endregion


#pragma region RECV
BOOL RecvProc(CSession *pSession) {
	_LOG(dfLOG_LEVEL_DEBUG, L"nRecvProc()..");

	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// RecvProc() :: pSession IS NULL");
		return FALSE;
	}
	g_recvTPS.Check();
	//---------------------------
	// 	   지역변수
	//---------------------------
	int recvRet;

	//---------------------------
	// 	   recv()
	//---------------------------
	recvRet = recv(pSession->_sock, pSession->_recvQ.GetRearBufferPtr(), pSession->_recvQ.DirectEnqueueSize(), 0);
	if (recvRet == SOCKET_ERROR || recvRet == 0) {
		DWORD errcode = GetLastError();
		if (errcode != WSAEWOULDBLOCK) {
			switch (errcode) {
			case WSAEOPNOTSUPP:
			case WSAECONNRESET:
				_LOG(dfLOG_LEVEL_WARNING, L"////// recv() errcode[%d]\n", errcode);
				break;
			default:
				_LOG(dfLOG_LEVEL_ERROR, L"////// recv() errcode[%d]\n", errcode);
				break;
			}
			//---------------------------
			// 큐에 넣어놨다가 나중에 지우기
			//---------------------------
			g_delResvQ.push(pSession->_sock);
			return FALSE;
		}
	}
	//---------------------------
	// 	  받은만큼 Rear땡기기
	//---------------------------
	pSession->_recvQ.MoveRear(recvRet);
	_LOG(dfLOG_LEVEL_DEBUG, L"RECV [%d byte]\n", recvRet);

	//---------------------------
	// 	  패킷 처리함수 호출
	//---------------------------
	int packRet = RecvPacket(pSession);
	if (packRet > 0) {
		_LOG(dfLOG_LEVEL_DEBUG, L"Packet Process [%d]..\n", packRet);
	} else {
		_LOG(dfLOG_LEVEL_ERROR, L"Packet Process ERROR [%d]..\n", packRet);
	}

	pSession->_LastRecvTime = timeGetTime();

	return TRUE;
}

int RecvPacket(CSession *pSession) {
	_LOG(dfLOG_LEVEL_DEBUG, L"RecvPacket()..");
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// RecvPacket() :: pSession IS NULL");
		return -1;
	}
	//---------------------------
	// 큐처리
	// 헤더 확인하고
	// 내용물 직렬화버퍼로
	//---------------------------

	while (true) {

		st_PACKET_HEADER header; // 마샬링용
		int iRecvQsize = pSession->_recvQ.GetUseSize();
		if (sizeof(st_PACKET_HEADER) > iRecvQsize) {
			//---------------------------
			// 헤더보다 적게있어서 할것이 없음
			//---------------------------
			if (iRecvQsize != 0)
				_LOG(dfLOG_LEVEL_WARNING, L"_recvQ.GetUseSize() Less than the header [%d byte] :: sid[%d]\n", iRecvQsize, pSession->_SID);
			return 1;
		}

		//---------------------------
		// 패킷검사
		// 1. 우리 헤다가 맞는지?
		//---------------------------
		pSession->_recvQ.Peek((char *) &header, sizeof(st_PACKET_HEADER));
		_LOG(dfLOG_LEVEL_DEBUG, L"header byCode[%hhx] bySize[%hhu] byType[%hhx]\n",
			header.byCode, header.bySize, header.byType);

		if (dfPACKET_CODE != header.byCode) {
			// 이상한 헤더
			_LOG(dfLOG_LEVEL_ERROR, L"Invalid header code [%hhx]\n", header.byCode);
			g_delResvQ.push(pSession->_sock); // 연결 끊기
			return 0xFF;
		}

		//---------------------------
		// 2. payload가 사이즈만큼 왔는지?
		//---------------------------
		if (header.bySize + sizeof(st_PACKET_HEADER) > iRecvQsize) {
			// 덜왔다
			_LOG(dfLOG_LEVEL_WARNING, L"Heather that's not all here SID[%d] \n\tpacket size[%hhu byte] actual payload[%d byte]\n", pSession->_SID,
				header.bySize + (BYTE) sizeof(st_PACKET_HEADER), iRecvQsize);
			return 1;
		}

		//---------------------------
		// 일단 페이로드는 잘왔음
		// 헤더는 알고있으니 리시브큐 뒤로 밀어주기
		//---------------------------
		_LOG(dfLOG_LEVEL_DEBUG, L"recvQ USED [%d byte]\n", iRecvQsize);
		pSession->_recvQ.MoveFront(sizeof(st_PACKET_HEADER));

		//---------------------------
		// 직렬화 버퍼에 복사해주기
		//---------------------------
		_LOG(dfLOG_LEVEL_DEBUG, L"Copy to serialization buffer...\n", );
		CPacket clPacket; // 직렬화 버퍼
		iRecvQsize = pSession->_recvQ.Dequeue(clPacket.GetBufferPtr(), header.bySize);
		if (header.bySize != iRecvQsize) {
			// 덜빼옴!
			_LOG(dfLOG_LEVEL_ERROR, L"Failed to copy the size [%hhu] [%d]\n", header.byCode, iRecvQsize);
			return -1;
		}
		//---------------------------
		// 포인터에 직접 복사했으니 크기만큼 쓰는 포지션 변경
		//---------------------------
		clPacket.MoveWritePos(header.bySize);
		_LOG(dfLOG_LEVEL_DEBUG, L"Copy completed...\n", );


		//---------------------------
		// 패킷 처리함수 (네트워크 - 컨텐츠 연결부) 호출
		//---------------------------
		_LOG(dfLOG_LEVEL_DEBUG, L"Call PacketProc()...\n", );
		if (PacketProc(pSession, &clPacket, header.byType) == FALSE) {
			// 패킷처리 못함
			_LOG(dfLOG_LEVEL_ERROR, L"PacketError ..\n", );
			return -1;
		}
		_LOG(dfLOG_LEVEL_DEBUG, L"PacketProc() OK..\n", );
	}

	return 0;
}

#pragma endregion

BOOL Disconnect(SOCKET sock) {
	if (sock == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_WARNING, L"////// Disconnect() :: sock IS INVALID_SOCKET");
		return FALSE;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"Disconnect()..");
	//---------------------------
	// 큐에 있는걸 하나씩 빼서 삭제
	//---------------------------
	CSession *pSession = FindSession(sock);
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_WARNING, L"////// Disconnect() :: pSession IS NULL");
		return FALSE;
	}
	//---------------------------
	// 연결끊기
	//---------------------------
	closesocket(pSession->_sock);
	_LOG(dfLOG_LEVEL_DEBUG, L"closesocket() OK..\n");

	//---------------------------
	// 클라 관리에서 지우기
	//---------------------------
	DeleteCharacter(pSession->_SID);
	DeleteSession(sock);
	_LOG(dfLOG_LEVEL_DEBUG, L"DeleteSession() OK..\n");

	_LOG(dfLOG_LEVEL_DEBUG, L"Disconnect() OK..\n");
	return TRUE;
}
void NetworkCloseServer() {
	//---------------------------
	// 서버정리
	//---------------------------
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkCloseServer()..");
	WSACleanup();
	_LOG(dfLOG_LEVEL_ERROR, L"WSACleanup OK..");
	closesocket(g_listenSock);
	_LOG(dfLOG_LEVEL_ERROR, L"g_listenSock OK..");
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkCloseServer() OK..\n");
}


#pragma region WRAPPING
CSession *CreateSession(SOCKET sock) {
	if (sock == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CreateSession() :: sock IS INVALID_SOCKET");
		return NULL;
	}


	//---------------------------
	// 	   세션 생성
	//---------------------------

	// 이미 있는 세션인지 확인
	auto iter = g_sessionMap.find(sock);
	if (iter != g_sessionMap.end()) {
		_LOG(dfLOG_LEVEL_ERROR, L"////////CreateSession() Session that already exists..");
		return NULL;
	}

	// 오브젝트풀에서 하나 뽑아오기
	CSession *pSession = g_sessionPool.Alloc();

	// 컨테이너에 넣기
	g_sessionMap.insert(std::make_pair(sock, pSession));
	_LOG(dfLOG_LEVEL_DEBUG, L"CreateSession() OK..");
	return pSession;
}

CSession *FindSession(SOCKET sock) {
	if (sock == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_BUG, L"////// FindSession() :: sock IS INVALID_SOCKET");
		return NULL;
	}

	//---------------------------
	// 	   세션 검색
	//---------------------------

	// 이미 있는 세션인지 확인
	auto iter = g_sessionMap.find(sock);
	if (iter == g_sessionMap.end()) {
		// 없음 NULL
		_LOG(dfLOG_LEVEL_WARNING, L"////////FindSession() Session not found..");
		return NULL;
	}
	// 있음 세션
	_LOG(dfLOG_LEVEL_DEBUG, L"FindSession() OK..");
	return iter->second;
}

BOOL DeleteSession(SOCKET sock) {

	if (sock == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_BUG, L"////// DeleteSession() :: sock IS INVALID_SOCKET");
		return FALSE;
	}

	//---------------------------
	// 	   세션 삭제
	//---------------------------

	// 이미 있는 세션인지 확인
	auto iter = g_sessionMap.find(sock);
	if (iter == g_sessionMap.end()) {
		// 존재하지 않는 세션 삭제못함
		_LOG(dfLOG_LEVEL_WARNING, L"////////DeleteSession() Session not found..");
		return FALSE;
	}

	// 오브젝트 풀에 반환
	g_sessionPool.Free(iter->second);
	// 컨테이너에서 삭제
	g_sessionMap.erase(iter);

	_LOG(dfLOG_LEVEL_WARNING, L"DeleteSession() OK..");
	return true;
}
#pragma endregion
