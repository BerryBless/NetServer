#include "NetworkCore.h"
#include "PacketProcess.h" // 패킷 처리용
#include "CMonitoring.h"
#include <queue>
#include <map>
#include <set>

// =========================
// 전역변수 - 서버
// g_wsaData
// g_listenAddr
// g_listenSock : 리슨 소켓
// =========================


WSADATA g_wsaData;		// 윈속
SOCKADDR_IN g_listenAddr;	// 리슨 소켓 정보
SOCKET g_listenSock;	// 리슨 소켓

// =========================
// 전역변수 - 서버
// g_clients 클라관리
// g_rooms 룸관리
// g_qDelID 연결 끊을 클라
// g_ClinetCnt	아이디 생성기
// =========================
std::map<DWORD, CClient *> g_clients; // ID로 클라관리
std::map<DWORD, CChatRoom *> g_rooms; // ID로 룸관리
std::set<std::wstring> g_UserNameSet; // 중복 체크용!
std::set<std::wstring> g_RoomNameSet; // 중복 체크용!


std::queue<DWORD> g_qDelID;// 지울 ID큐
int g_ClinetCnt = 0;// 클라이언트 고유 ID생성기
int g_RoomCnt = 0;// 룸 고유 ID생성기

// 모니터링
CMonitoring g_MAccept; // accept() TPS 계산
CMonitoring g_MDisconnect; // disconnect() TPS 계산
CMonitoring g_MSend; // send() TPS 계산
CMonitoring g_MRecv; // recv() TPS 계산
CMonitoring g_MFPS; // FPS 계산



BOOL NetworkInitServer() {
	wprintf_s(L"Init Server..\n");
	int setRet;// 소켓 연결에 나오는 리턴

		// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &g_wsaData) != 0) {
		wprintf_s(L"////// WSAStartup() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// 소켓 기본정보
	memset(&g_listenAddr, 0, sizeof(g_listenAddr));
	g_listenAddr.sin_family = AF_INET;
	g_listenAddr.sin_port = htons(dfNETWORK_PORT);
	InetPton(AF_INET, L"0.0.0.0", &g_listenAddr.sin_addr);

	// socket()
	g_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_listenSock == INVALID_SOCKET) {
		wprintf_s(L"////// socket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// bind()
	setRet = bind(g_listenSock, (SOCKADDR *) &g_listenAddr, sizeof(g_listenAddr));
	if (setRet == SOCKET_ERROR) {
		wprintf_s(L"////// bind() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// listen()
	setRet = listen(g_listenSock, SOMAXCONN);
	if (setRet == SOCKET_ERROR) {
		wprintf_s(L"////// listen() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}

	// 비동기 키기
	u_long on = 1;
	setRet = ioctlsocket(g_listenSock, FIONBIO, &on);
	if (setRet == SOCKET_ERROR) {
		wprintf_s(L"////// ioctlsocket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	wprintf_s(L"Done..\n");
	return TRUE;
}

BOOL NetworkPorc() {
	CClient *pClient;
	DWORD UserTable_ID[FD_SETSIZE];
	SOCKET UserTable_SOCKET[FD_SETSIZE];
	int iSockCnt = 0;

	//---------------------------
	// 64개씩 나눠서 하기!
	//---------------------------
	FD_SET ReadSet;
	FD_SET WriteSet;
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	memset(UserTable_ID, -1, sizeof(DWORD) * FD_SETSIZE);
	memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

	//---------------------------
	// 리슨소켓 넣기
	//---------------------------
	FD_SET(g_listenSock, &ReadSet);
	UserTable_ID[iSockCnt] = 0;
	UserTable_SOCKET[iSockCnt] = g_listenSock;
	iSockCnt++;
	//---------------------------
	// 그외 클라이언트도 넣기
	//---------------------------
	for (auto clIter = g_clients.begin(); clIter != g_clients.end();) {
		pClient = clIter->second;
		++clIter;
		// 클라이언트 소켓 등록
		UserTable_ID[iSockCnt] = pClient->_ID;
		UserTable_SOCKET[iSockCnt] = pClient->_sock;

		FD_SET(pClient->_sock, &ReadSet);
		if (pClient->_sendQ.GetUseSize() > 0)
			FD_SET(pClient->_sock, &WriteSet);

		iSockCnt++;
		//---------------------------
		// select 처리가능한 최대치 도달
		//---------------------------
		if (FD_SETSIZE <= iSockCnt) {
			// select()
			SelectProc(UserTable_ID, UserTable_SOCKET, &ReadSet, &WriteSet);

			// 초기화
			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);
			memset(UserTable_ID, -1, sizeof(DWORD) * FD_SETSIZE);
			memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
			iSockCnt = 0;

			// 리슨소켓 넣기
			FD_SET(g_listenSock, &ReadSet);
			UserTable_ID[iSockCnt] = 0;
			UserTable_SOCKET[iSockCnt] = g_listenSock;
			iSockCnt++;
		}
	}
	//---------------------------
	// 남아있는 클라도 챙겨주기
	//---------------------------
	if (iSockCnt > 0) {
		SelectProc(UserTable_ID, UserTable_SOCKET, &ReadSet, &WriteSet);
	}

	//---------------------------
	// 에러가나서 지워야할걸 담아두었다가 이때 모두 삭제
	//---------------------------
	while (g_qDelID.empty() == false) {
		DWORD dwID = g_qDelID.front();
		g_qDelID.pop();
		Disconnect(dwID);
	}

	if (g_MFPS.Check()) {
		// 1초마다 모니터링 출력
		wprintf_s(L"-----------------------\n");
		wprintf_s(L"FPS [%d]\n", g_MFPS.GetTPS());
		wprintf_s(L"accept\t\tTPS [%d] TOTAL[%lld]\n", g_MAccept.GetTPS(), g_MAccept.GetTotal());
		wprintf_s(L"disconnect\tTPS [%d] TOTAL[%lld]\n", g_MDisconnect.GetTPS(), g_MDisconnect.GetTotal());
		wprintf_s(L"send\t\tTPS [%d]\n", g_MSend.GetTPS());
		wprintf_s(L"recv\t\tTPS [%d]\n", g_MRecv.GetTPS());
		wprintf_s(L"-----------------------\n");
	}

	return true;
}

BOOL SelectProc(DWORD *dwpTableID, SOCKET *pTableSocket, FD_SET *pReadSet, FD_SET *pWriteSet) {
	//---------------------------
	// 대기시간설장
	//---------------------------
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	//---------------------------
	// select()
	//---------------------------
	int selRet = select(0, pReadSet, pWriteSet, NULL, &timeout);
	CClient *pClient;
	if (selRet > 0) {
		//---------------------------
		// 0번째는 무조건 리슨체크
		// 백로그 큐에 뭔가 왔다! - Accept
		//---------------------------
		if (FD_ISSET(g_listenSock, pReadSet)) {
			AcceptProc();
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
				pClient = FindClient(dwpTableID[i]);
				if (pClient == NULL) {
					wprintf_s(L"///// pReadSet) pClient is NULL \n");
				}
				RecvProc(pClient);
			}
			//---------------------------
			// 쓰기
			//---------------------------
			if (FD_ISSET(pTableSocket[i], pWriteSet)) {
				pClient = FindClient(dwpTableID[i]);
				if (pClient == NULL) {
					wprintf_s(L"///// pWriteSet) pClient is NULL \n");
				}
				SendProc(pClient);
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
#ifdef PRINTSTEP
	wprintf_s(L"Accept...\n");
#endif // PRINTSTEP
	//---------------------------
	// accept TPS 모니터링
	//---------------------------
	g_MAccept.Check();

	// 플레이어 카운트 추가
	++g_ClinetCnt;

	CClient *pClient;
	SOCKET sock;// 임시 소켓
	SOCKADDR_IN addr; // 임시 주소
	int addrlen = sizeof(addr);

	//---------------------------
	// 백로그큐에 있는 소켓을 accept하기
	// accept()
	//---------------------------
	sock = accept(g_listenSock, (SOCKADDR *) &addr, &addrlen);
	if (sock == INVALID_SOCKET) {
		// 에러
		wprintf_s(L"////// accept() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	//---------------------------
	// 클라이언트 가입 시도
	//---------------------------
	pClient = FindClient(g_ClinetCnt);
	if (pClient != NULL) {
		// 이미 있는 ID
		wprintf_s(L"////// accept() 이미 있는 클라이언트[%d]\n", g_ClinetCnt);
		return FALSE;
	}
	//---------------------------
	// 클라 할당
	// 컨테이너 삽입
	//---------------------------
	pClient = InsertClient(g_ClinetCnt);
	if (pClient == NULL) {
		// 생성실
		wprintf_s(L"////// accept() 클라이언트 생성 실패[%d]\n", g_ClinetCnt);
		return FALSE;
	}
	pClient->Init(g_ClinetCnt, sock, addr);
	return TRUE;
}
#pragma region SEND

void SendProc(CClient *pClient) {
	//---------------------------
	// sendQ에 있는 send할껄 send()
	//---------------------------

	// 변수
	int sendRet; // send()리턴값
	//---------------------------
	// send() TPS 체크
	//---------------------------
	g_MSend.Check();
	sendRet = send(pClient->_sock, pClient->_sendQ.GetFrontBufferPtr(), pClient->_sendQ.DirectDequeueSize(), 0);
	if (sendRet == SOCKET_ERROR) {
		DWORD errcode = GetLastError();
		if (errcode != WSAEWOULDBLOCK) {
			// disconnect
			wprintf_s(L"////// SendProc() errorcode[%d]\n", errcode);
			//---------------------------
			// 큐에 넣어놨다가 나중에 지우기
			//---------------------------
			g_qDelID.push(pClient->_ID);
		}
	} else {
		// 보낸만큼 링버퍼 뒤로 밀어주기
		pClient->_sendQ.MoveFront(sendRet);
	}
}

void SendUnicast(CClient *pClient, st_PACKET_HEADER *pHeader, CPacket *pPacket) {
	//---------------------------
	// pClient에게만 패킷 보내기
	//---------------------------
	if (pClient == NULL) {
		wprintf_s(L"////// SendUnicast() USER NULL\n");
		return;
	}
	//---------------------------
	// pClient의 send 링버퍼에 메시지 넣기
	//---------------------------
	pClient->_sendQ.Enqueue((char *) pHeader, sizeof(st_PACKET_HEADER));
	pClient->_sendQ.Enqueue(pPacket->GetBufferPtr(), pPacket->GetDataSize());

}

void SendBroadcast(CClient *pClientx, st_PACKET_HEADER *pHeader, CPacket *pPacket) {
	//---------------------------
	// pClientx를 제외항 모든 대상에게
	//---------------------------
	CClient *pClient;
	for (auto iter = g_clients.begin(); iter != g_clients.end(); ++iter) {
		pClient = iter->second;
		if (pClient != pClientx)
			SendUnicast(pClient, pHeader, pPacket);
	}
}

void SendBroadcastRoom(CChatRoom *pRoom, CClient *pClientx, st_PACKET_HEADER *pHeader, CPacket *pPacket) {
	//---------------------------
	// pRoom에 속한 클라중 pClientx를 제외항 모든 대상에게
	//---------------------------
	CClient *pClient;
	for (auto iter = pRoom->_userList.begin(); iter != pRoom->_userList.end(); ++iter) {
		pClient = FindClient(*iter);
		if (pClient == NULL) {
			CRASH();
		}
		if (pClient != pClientx)
			SendUnicast(pClient, pHeader, pPacket);
	}

}

#pragma endregion

#pragma region RECV

void RecvProc(CClient *pClient) {
	//---------------------------
	// 	   Recv()
	//---------------------------
	if (pClient == NULL) return;

	int recvRet;
	g_MRecv.Check();
	//---------------------------
	// 직접 링버퍼에 넣기
	//---------------------------
	recvRet = recv(pClient->_sock, pClient->_recvQ.GetRearBufferPtr(), pClient->_recvQ.DirectEnqueueSize(), 0);
	if (recvRet == SOCKET_ERROR || recvRet == 0) {
		DWORD errcode = GetLastError();
		if (errcode != WSAEWOULDBLOCK) {
			//---------------------------
			// 큐에 넣어놨다가 나중에 지우기
			//---------------------------
			wprintf_s(L"////// recv() errorcode[%d]\n", errcode);
			g_qDelID.push(pClient->_ID);
			return;
		}
	}
	// 받은만큼 Rear땡기기
	pClient->_recvQ.MoveRear(recvRet);

	//---------------------------
	// 패킷 처리 함수 호출
	//---------------------------
	int packProcRet = RecvPacket(pClient);
	if (packProcRet < 0) {
		wprintf_s(L"////// RecvProc ERROR  UID[%d]", pClient->_ID);
		return;
	}
}

int RecvPacket(CClient *pClient) {
	//---------------------------
	// 큐처리
	// 헤더 확인하고
	// 내용물 직렬화로
	//---------------------------
	while (true) {

		st_PACKET_HEADER header; // 마샬링용
		int iRecvQsize = pClient->_recvQ.GetUseSize();
		if (sizeof(st_PACKET_HEADER) > iRecvQsize) {
			//---------------------------
			// 헤더보다 적게있어서 할것이 없음
			//---------------------------
			return 1;
		}

		//---------------------------
		// 패킷검사
		// 1. 우리 헤다가 맞는지?
		//---------------------------
		pClient->_recvQ.Peek((char *) &header, sizeof(st_PACKET_HEADER));

		if (dfPACKET_CODE != header.byCode)
			// 이상한 헤더
			return 0xFF;

		//---------------------------
		// 2. payload가 사이즈만큼 왔는지?
		//---------------------------
		if (header.wPayloadSize + sizeof(st_PACKET_HEADER) > iRecvQsize) {
			// 덜왔다
			return 1;
		}

		//---------------------------
		// 일단 페이로드는 잘왔음
		// 헤더는 알고있으니 리시브큐 뒤로 밀어주기
		//---------------------------
		pClient->_recvQ.MoveFront(sizeof(st_PACKET_HEADER));

		//---------------------------
		// 직렬화 버퍼에 복사해주기
		//---------------------------
		CPacket clPacket; // 직렬화 버퍼
		if (header.wPayloadSize !=
			pClient->_recvQ.Dequeue(clPacket.GetBufferPtr(), header.wPayloadSize)) {
			// 덜빼옴!
			return -1;
		}
		//---------------------------
		// 포인터에 직접 복사했으니 크기만큼 쓰는 포지션 변경
		//---------------------------
		clPacket.MoveWritePos(header.wPayloadSize);

		//---------------------------
		// 체크썸 확인
		//---------------------------
		BYTE checksum = MakeChecksum(&clPacket, header.wMsgType);
		if (header.byCheckSum != checksum) {
			// 이상한 데이터
			return -1;
		}
		//---------------------------
		// 패킷 처리함수 (컨텐츠) 호출
		//---------------------------
		if (PacketProc(&clPacket, header.wMsgType, pClient) == false) {
			// 패킷처리 못함
			return -1;
		}
	}

}
#pragma endregion

void Disconnect(DWORD dwUID) {
	//---------------------------
	// 큐에 있는걸 하나씩 빼서 삭제
	//---------------------------
#ifdef PRINTSTEP
	wprintf_s(L"Disconnect UID[%d]", dwUID);
#endif
	CClient *pClient = FindClient(dwUID);
	if (pClient == NULL) {
#ifdef PRINTSTEP
		wprintf_s(L"Disconnect 이미삭제된", dwUID);
#endif
		return;
	}
	//---------------------------
	// 연결끊기
	//---------------------------
	closesocket(pClient->_sock);

	//---------------------------
	// 클라 관리에서 지우기
	//---------------------------
	EraseClient(dwUID);


	//---------------------------
	// TPS 모니터링
	//---------------------------
	g_MDisconnect.Check();
}

void NetworkCloseServer() {
	//---------------------------
	// 서버정리
	//---------------------------
	wprintf_s(L"CloseServer\n");
	WSACleanup();
	closesocket(g_listenSock);
}



#pragma region WRAPPING
// 클라이언트
CClient *InsertClient(DWORD dwUID) {
	//---------------------------
	// 클라이언트 생성후 삽입
	//---------------------------
	auto clIter = g_clients.find(dwUID);
	if (clIter != g_clients.end())
		return NULL;

	CClient *pClient = new CClient;
	g_clients.insert(std::make_pair(dwUID, pClient));
	return pClient;
}
// 검색
CClient *FindClient(DWORD dwUID) {
	//---------------------------
	// 클라이언트 검색
	//---------------------------
	auto clIter = g_clients.find(dwUID);
	if (clIter == g_clients.end())
		return FALSE;
	return clIter->second;
}
// 삭제
BOOL EraseClient(DWORD dwUID) {
	//---------------------------
	// 클라이언트 삭제
	//---------------------------
	auto clIter = g_clients.find(dwUID);
	if (clIter == g_clients.end())
		return FALSE;

	// 이름삭제
	EraseUsername(clIter->second->_username);
	// 클라삭제
	delete clIter->second;
	// 맵삭제
	g_clients.erase(clIter);

	return TRUE;
}

BOOL IsUsername(std::wstring wsUsername) {
	//---------------------------
	// 클라이언트 중복닉 확인
	//---------------------------
	auto iter = g_UserNameSet.find(wsUsername);

	if (iter == g_UserNameSet.end())
		return FALSE;
	return TRUE;
}

BOOL InsertUsername(std::wstring wsUsername, DWORD dwUID) {
	//---------------------------
	// 클라이언트 닉 등록
	//---------------------------

	// 없으면 안됨
	auto clIter = g_clients.find(dwUID);
	if (clIter == g_clients.end())
		return FALSE;
	// 있으면 안됨
	auto wsIter = g_UserNameSet.find(wsUsername);
	if (wsIter != g_UserNameSet.end())
		return FALSE;

	// 값넣기
	g_UserNameSet.insert(wsUsername);
	clIter->second->_username = wsUsername.c_str();
	return TRUE;
}

BOOL EraseUsername(std::wstring wsUsername) {
	//---------------------------
	// 클라이언트 닉 해지
	//---------------------------

	// 없으면 안됨
	auto wsIter = g_UserNameSet.find(wsUsername);
	if (wsIter == g_UserNameSet.end())
		return FALSE;

	// 값빼기
	g_UserNameSet.erase(wsUsername);
	return TRUE;
}

// 채팅방
// 삽입
CChatRoom *InsertRoom(DWORD dwRID) {
	//---------------------------
	// 룸 생성후 컨테이너에 삽입
	//---------------------------
	auto roIter = g_rooms.find(dwRID);
	if (roIter != g_rooms.end())
		return NULL;

	// 룸생성
	CChatRoom *pRoom = new CChatRoom;
	pRoom->_dwRoomID = dwRID;
	// 컨테이너에 삽입
	g_rooms.insert(std::make_pair(dwRID, pRoom));
	return pRoom;
}
// 검색
CChatRoom *FindRoom(DWORD dwRID) {
	//---------------------------
	// 룸 번호로 찾기
	//---------------------------
	auto roIter = g_rooms.find(dwRID);
	if (roIter == g_rooms.end())
		return FALSE;
	return roIter->second;
}
// 삭제
BOOL EraseRoom(DWORD dwRID) {
	//---------------------------
	// 룸 번호로 찾기
	//---------------------------
	auto roIter = g_rooms.find(dwRID);
	if (roIter == g_rooms.end())
		return FALSE;
	// 이름삭제
	EraseRoomname(roIter->second->_wsTitle);
	// 방삭제
	delete roIter->second;
	// 맵삭제
	g_rooms.erase(roIter);

	return TRUE;
}

BOOL IsRoomname(std::wstring wsRoomname) {
	//---------------------------
	// 방제 중복 확인
	//---------------------------
	auto iter = g_RoomNameSet.find(wsRoomname);
	if (iter == g_RoomNameSet.end())
		return FALSE;
	return TRUE;
}

BOOL InsertRoomname(std::wstring wsRoomname, DWORD dwRID) {
	//---------------------------
	// 방제 등록
	//---------------------------

	// 없으면 안됨
	auto rmIter = g_rooms.find(dwRID);
	if (rmIter == g_rooms.end())
		return FALSE;
	// 있으면 안됨
	auto wsIter = g_RoomNameSet.find(wsRoomname);
	if (wsIter != g_RoomNameSet.end())
		return FALSE;

	// 값넣기
	g_RoomNameSet.insert(wsRoomname);
	rmIter->second->_wsTitle = wsRoomname.c_str();
	return  TRUE;
}

BOOL EraseRoomname(std::wstring wsRoomname) {
	//---------------------------
	// 방제 해지
	//---------------------------

	// 없으면 안됨
	auto wsIter = g_RoomNameSet.find(wsRoomname);
	if (wsIter == g_RoomNameSet.end())
		return FALSE;

	// 값빼기
	g_RoomNameSet.erase(wsRoomname);
	return TRUE;
}



BOOL RoomEnter(CClient *pClient, CChatRoom *pRoom) {
	//---------------------------
	// 	   쿨라가 룸에 들어감
	//---------------------------
	if (pClient == NULL)return FALSE;
	if (pRoom == NULL)return FALSE;
	if (pClient->_EnterRoomID != df_LOBBY) return FALSE;
	pRoom->_userList.push_back(pClient->_ID);
	pClient->_EnterRoomID = pRoom->_dwRoomID;
	return TRUE;
}

BOOL RoomLeave(CClient *pClient, CChatRoom *pRoom) {
	//---------------------------
	// 	   클라가 룸에서 나감
	//---------------------------
	if (pClient == NULL)return FALSE;
	if (pRoom == NULL)return FALSE;
	pRoom->_userList.remove(pClient->_ID);
	pClient->_EnterRoomID = df_LOBBY;
	return TRUE;
}
#pragma endregion
