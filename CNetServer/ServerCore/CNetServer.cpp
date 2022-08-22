#include "pch.h"
#include "CNetServer.h"

#include "Profiler.h"


// cmd code
#define dfEXIT_CODE 0xFFFFFFFF // GQCS()에서 이게 오면 종료

// CS
#define SESSION_LOCK(pSession)		SessionLock(pSession)
#define SESSION_UNLOCK(pSession)	SessionUnlock(pSession)

#define dfMIN(a,b) a>b?b:a
#define dfMAX(a,b) a<b?b:a


CNetServer::CNetServer() {



	//---------------------------
	// 세션 컨테이너 락 초기화
	//---------------------------
	InitializeSRWLock(&_sessionContainerLock);

	ResetMonitor();


	//---------------------------
	// 변수 초기화
	//---------------------------
	_Port = 0;
	_bindIP = 0;
	_listensock = INVALID_SOCKET;
	_maxRunThreadCount = 0;
	_workerThreadCount = 0;
	_maxConnection = 0;
	_isNagle = false;
	_timeoutMillisec = 40000;

	_isRunning = false;
	_NumThreads = 0;

	_hIOCP = INVALID_HANDLE_VALUE;
	_hThreads = nullptr;

	_sessionContainer = nullptr;
	_emptyIndex.Clear();

	_IDGenerater = 1;

}

CNetServer::~CNetServer() {
	CloseHandle(_hIOCP);
	WSACleanup();
}

bool CNetServer::Start(u_long IP, u_short prot, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	if (_isRunning == true) {
		//---------------------------
		// 이미 실행중일
		//---------------------------
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running\n");
		OnError(111, L"Network is already running");
		return false;
	}



	//---------------------------
	// 서버 정보 넣기
	//---------------------------
	_Port = prot;
	_bindIP = IP;
	_workerThreadCount = workerThreadCount;
	_maxRunThreadCount = dfMIN(workerThreadCount, maxRunThreadCount);
	_isNagle = nagle;
	_maxConnection = maxConnection;


	Startup();

	_isRunning = true;

	return _isRunning;
}


bool CNetServer::Start(wchar_t *wsConfigPath) {
	// TODO 파일 패치로 파싱해서 실행
	return Start(INADDR_ANY, 10101, 24, 12, true, 3000);
}


void CNetServer::Quit() {
	//---------------------------
	// 서버 종료
	// _isRunning = false로 모니터링 스레드 종료
	// 종료코드를 완료통지에 넣어 워커 스레드 종료
	// _listensock을 닫아 어셉트 스레드 종료
	//---------------------------
	_isRunning = false;
	PostQueuedCompletionStatus(_hIOCP, dfEXIT_CODE, dfEXIT_CODE, NULL);
	closesocket(_listensock);

	//---------------------------
	// 서버 종료 기다리기
	//---------------------------
	DWORD retval = WaitForMultipleObjects(_NumThreads, _hThreads, TRUE, INFINITE);
	switch (retval) {
	case WAIT_FAILED:
		break;
	case WAIT_TIMEOUT:
		break;
	default:
		break;
	}
}

bool CNetServer::Disconnect(SESSION_ID SessionID) {
	//---------------------------
	// 세션 끊기
	//---------------------------
	bool ret = false;
	SESSION *pSession = AcquireSession(SessionID, dfLOGIC_DISCONNECT);
	if (pSession == NULL) {
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"//Disconnect ERROR :: can not find session..");
		OnError(dfLOGIC_DISCONNECT, L"Disconnect ERROR :: can not find session..");
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"//Disconnect id[%d]", SessionID);

	InterlockedExchange(&pSession->_isAlive, FALSE);
	ret = CancelIoEx((HANDLE) pSession->_sock, nullptr);

	//closesocket(pSession->_sock);
	ReturnSession(pSession, dfLOGIC_DISCONNECT);
	return ret;
}

bool CNetServer::SendPacket(SESSION_ID SessionID, CPacket *pPacket) {
	if (pPacket == nullptr)
		return false;
	pPacket->AddRef();
	//---------------------------
	// 세션찾기
	//---------------------------
	SESSION *pSession = AcquireSession(SessionID, 664466);
	if (pSession == NULL) {
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"//SendPacket ERROR :: can not find session..");
		OnError(dfLOGIC_SEND_PACKET, L"SendPacket ERROR :: can not find session..");
		pPacket->SubRef();
		return false;
	}
	//---------------------------
	// 지워진(끊어진) 세션
	//---------------------------
	if (!InterlockedOr((LONG *) &pSession->_isAlive, 0)) {
		CRASH();
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"//SendPacket ERROR :: Session is colsed..");
		OnError(dfLOGIC_SEND_PACKET, L"SendPacket ERROR :: Session is colsed..");
		pPacket->SubRef();
		return false;
	}
	PRO_BEGIN(L"SendPacket");
	//---------------------------
	// 페킷 포인터를 센드큐에
	//---------------------------
	pPacket->SetNetHeader();
	pSession->_sendQueue.Enqueue(pPacket);
	//---------------------------
	// monitor
	//---------------------------
	InterlockedIncrement(&_sendPacketCalc);
#ifndef df_SENDTHREAD
	SendPost(pSession, dfLOGIC_SEND_PACKET);
#endif // !df_SENDTHREAD
	ReturnSession(pSession, 446644);
	PRO_END(L"SendPacket");
	return true;
}

BOOL CNetServer::DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr) {
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

void CNetServer::Startup() {
	if (_isRunning == true) {
		CLogger::_Log(dfLOG_LEVEL_NOTICE, L"///// Server Already Running");
		return;
	}
	int _ = _wmkdir(L"ServerLog");
	_ = _wmkdir(L"ServerLog\\LibraryLog");
	_ = _wmkdir(L"ServerLog\\MonitorLog");
#ifdef dfPROFILER
	_ = _wmkdir(L"ServerLog\\Profile");
#endif // dfPROFILER
	//---------------------------
	// 문자열 로컬 세팅
	//---------------------------
	setlocale(LC_ALL, "korean");
	_wsetlocale(LC_ALL, L"korean");

	//---------------------------
	// 정말 1초로 카운팅
	//---------------------------
	timeBeginPeriod(1);

	//---------------------------
	// 로거 초기화
	//---------------------------
	CLogger::Initialize();
	CLogger::SetDirectory(L"ServerLog\\LibraryLog");
	CLogger::SetLogLevel(dfLOG_LEVEL_ERROR);


	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"==============================START SERVER==============================");


	//---------------------------
	// 세팅된 정보로 리슨 만들기
	//---------------------------
	if (CreateListenSocket() == false) {
		//---------------------------
		// 리슨 소켓 생성 실패
		//---------------------------
		CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Failed Create Listen Socket");
		return;
	}


	//---------------------------
	// 세션 컨테이너 생성
	//---------------------------
	_sessionContainer = new SESSION[(int)(_maxConnection + (u_short) 1)];

	InitializeIndex();


	//---------------------------
	// 스레드 핸들 배열 생성
	// _workerThreadCount + acceptThread + monitorThread + SendThread
	//---------------------------
	_NumThreads = _workerThreadCount + 4;
	_hThreads = new HANDLE[(long long) _NumThreads];


	//---------------------------
	// 스레드 실행
	//---------------------------
	BeginThreads();
}

bool CNetServer::CreateListenSocket() {
	WSADATA wsa;
	//---------------------------
	// 윈속 초기화
	//---------------------------
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// WSAStartup() errcode[%d]\n", WSAGetLastError());
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"WSAStartup OK..");

	//---------------------------
	// socket()
	//---------------------------
	_listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_listensock == INVALID_SOCKET) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// socket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"socket() OK ..");


	//---------------------------
	// setsockopt()
	//---------------------------
	LINGER l;
	l.l_onoff = 1;
	l.l_linger = 0;
	setsockopt(this->_listensock, SOL_SOCKET, SO_LINGER, (const char *) &l, sizeof(l));
	CLogger::_Log(dfLOG_LEVEL_NOTICE,L"Linger Option Off");


	BOOL keepAliveFlag = 0;
	setsockopt(this->_listensock, SOL_SOCKET, SO_KEEPALIVE, (const char *) &keepAliveFlag, sizeof(BOOL));
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Keep Alive Option Off");


	DWORD sendBufferSize = 1024 * 64;
	setsockopt(this->_listensock, SOL_SOCKET, SO_SNDBUF, (const char *) &sendBufferSize, sizeof(DWORD));
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Send Buffer Zero");


	setsockopt(this->_listensock, IPPROTO_TCP, TCP_NODELAY, (const char *) &_isNagle, sizeof(_isNagle));
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Nagle Option Off");


	//---------------------------
	// bind()
	//---------------------------
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_Port);
	addr.sin_addr.S_un.S_addr = htonl(_bindIP);

	int bindRet = bind(_listensock, (SOCKADDR *) &addr, sizeof(addr));
	if (bindRet == SOCKET_ERROR) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// bind() errcode[%d]", WSAGetLastError());
		closesocket(_listensock);
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"bind() OK ..");

	//---------------------------
	// listen()
	//---------------------------
	int listenRet = listen(_listensock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// listen() errcode[%d]", WSAGetLastError());
		closesocket(_listensock);
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"listen() OK ..");


	//---------------------------
	// CreateIoCompletionPort()
	//---------------------------
	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _maxRunThreadCount);
	if (_hIOCP == NULL) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// CreateIoCompletionPort() errcode[%d]", WSAGetLastError());
		closesocket(_listensock);
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"CreateIoCompletionPort() OK ..");
	return true;
}

void CNetServer::BeginThreads() {
	//---------------------------
	// n : _workerThreadCount
	// 0~n-1 : worker
	// n : accept
	// n+1 : Monitor
	//---------------------------
	int i = 0;

	//---------------------------
	// 워커스레드 만들기
	//---------------------------
	for (; i < _workerThreadCount; i++) {
		_hThreads[i] = (HANDLE) _beginthreadex(nullptr, 0, WorkerThread, this, 0, nullptr);
	}
	//---------------------------
	//  accept 스레드 생성
	//---------------------------
	_hThreads[i++] = (HANDLE) _beginthreadex(nullptr, 0, AcceptThread, this, 0, nullptr);
	//---------------------------
	// 모니터링 스레드 생성
	//---------------------------
	_hThreads[i++] = (HANDLE) _beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);

	_hThreads[i++] = (HANDLE) _beginthreadex(nullptr, 0, TimeOutThread, this, 0, nullptr);

#ifdef df_SENDTHREAD
	_hThreads[i++] = (HANDLE) _beginthreadex(nullptr, 0, SendThread, this, 0, nullptr);
#endif // df_SENDTHREAD

}

//============================================================
// 스래드 함수
// 매개변수로 this포인터를 받아 this포인터로 오브젝트를 불러 사용
//============================================================
#pragma region Thread Function
unsigned int __stdcall CNetServer::WorkerThread(LPVOID arg) {
	CNetServer *pServer = (CNetServer *) arg;
	while (true) {
		if (pServer->OnGQCS() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- Worker Thread IS Closed..\n");
	return 0;
}

unsigned int __stdcall CNetServer::AcceptThread(LPVOID arg) {
	CNetServer *pServer = (CNetServer *) arg;
	while (true) {
		if (pServer->AcceptProc() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- Accept Thread IS Closed..\n");
	return 0;
}

unsigned int __stdcall CNetServer::MonitorThread(LPVOID arg) {
	CNetServer *pServer = (CNetServer *) arg;
	while (true) {
		if (pServer->NetMonitorProc() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- Monitor Thread IS Closed..\n");
	return 0;
}
unsigned int __stdcall CNetServer::TimeOutThread(LPVOID arg) {
	CNetServer *pServer = (CNetServer *) arg;
	while (true) {
		if (pServer->TimeOutProc() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- TimeOutThread IS Closed..\n");
	return 0;
}
#ifdef df_SENDTHREAD
unsigned int __stdcall CNetServer::SendThread(LPVOID arg) {
	CNetServer *pServer = (CNetServer *) arg;
	while (true) {
		if (pServer->SendThreadProc() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- SendThread IS Closed..\n");
	return 0;
}
#endif // df_SENDTHREAD
#pragma endregion

bool CNetServer::OnGQCS() {
	DWORD transferredSize = 0;
	SESSION_ID completionKey = 0;
	WSAOVERLAPPED *pOverlapped = nullptr;
	SESSION *pSession = nullptr;

	//---------------------------
	// GQCS()
	//---------------------------
	BOOL GQCSRet = GetQueuedCompletionStatus(_hIOCP, &transferredSize, (PULONG_PTR) &completionKey, &pOverlapped, INFINITE);

	//---------------------------
	// 완료통지 확인
	//---------------------------
	if (pOverlapped == NULL) {
		if (transferredSize == dfEXIT_CODE && (long long) completionKey == dfEXIT_CODE) {
			//---------------------------
			// 종료코드로 정상 종료
			//---------------------------
			PostQueuedCompletionStatus(_hIOCP, dfEXIT_CODE, dfEXIT_CODE, NULL);
			return false;
		}
		//---------------------------
		// 완료통지 실패
		//---------------------------
		DWORD err = WSAGetLastError();
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"overlapped is NULL ERROR CODE [%d]", err);
		OnError(err, L"IOCP ERROR :: overlapped is NULL");
		return true;
	} else if (pOverlapped == (OVERLAPPED *) 0xffffffff) {
		return true;
	}

	//---------------------------
	// completionKey(ID) 로 세션포인터 찾기
	//---------------------------
	pSession = FindSession(completionKey);
	if (pSession == NULL) {
		return true;
	}

	//SESSION_LOCK(pSession);
	do {
		if (InterlockedOr64((LONG64 *) &pSession->_ID, 0) == 0) {
			//---------------------------
			// 이미 지워진 세션
			//---------------------------
			CLogger::_Log(dfLOG_LEVEL_DEBUG, L"OnGQCS :: Deleted Session\n");
			break;
		}
		if (transferredSize > 0 && GQCSRet == TRUE) {
			//---------------------------
			// WSARecv가 완료됨
			//---------------------------
			if (pOverlapped == &pSession->_recvOverlapped) {
				RecvProc(pSession, transferredSize);
			}
			//---------------------------
			// WSASend가 완료됨
			//---------------------------
			if (pOverlapped == &pSession->_sendOverlapped) {
				SendProc(pSession, transferredSize);
			}
		}
	} while (0);
	//SESSION_UNLOCK(pSession);
	//---------------------------
	// 	   IOCount --
	//---------------------------
	DecrementIOCount(pSession, dfLOGIC_WORKER + dfLOGIC_DECREMENT_IO);
	//if (InterlockedDecrement(&pSession->_IOcount) == 0)
	//	ReleaseSession(pSession, dfLOGIC_WORKER + dfLOGIC_DECREMENT_IO);

	return true;
}

bool CNetServer::SendProc(SESSION *pSession, DWORD transferredSize) {
	if (pSession == NULL) {
		CRASH();
	}
	//---------------------------
	// 완료통지 온 패킷 지우기
	//---------------------------
	CPacket *pPacket;
	DWORD sendedPacketCnt = InterlockedExchange(&pSession->_sendPacketCnt,0);

	for (int i = 0; i < sendedPacketCnt; ++i) {
		pPacket = pSession->_pSendPacketBufs[i];
		pPacket->SubRef();
		pPacket = nullptr;
	}

	//---------------------------
	// 	   Send가 끝났다
	//---------------------------
	InterlockedExchange(&pSession->_IOFlag, FALSE);
	InterlockedAdd64(&_totalProcessedBytes, transferredSize);

#ifndef df_SENDTHREAD
	//---------------------------
	// SendQ에 보낼것이 남아있으면 Send
	//---------------------------
	SendPost(pSession, dfLOGIC_CPMPLETE_SEND);
#endif
	return true;
}

bool CNetServer::RecvProc(SESSION *pSession, DWORD transferredSize) {
	//---------------------------
	// recv신호가 옴
	//---------------------------

	//---------------------------
	// 온만큼 링버퍼 Rear빼주기
	//---------------------------
	int movRet = pSession->_recvQueue.MoveRear(transferredSize);
	if (transferredSize != movRet) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L" ID[%lld] :: transferredSize[%d] != movRet[%d]", pSession->_ID, transferredSize, movRet);
		CRASH();
	}
	DWORD msgByte = 0; // 메시지 처리된 바이트 수

	// return val
	int headerPeekRet;
	int headerMoveRet;
	int payloadDeqRet;
	PACKET_NET_HEADER header;

	InterlockedExchange(&pSession->_lastRecvdTime, timeGetTime());

	//---------------------------
	// 	   반복문 돌며 패킷 처리
	//---------------------------
	while (msgByte < transferredSize) {
		if (pSession->_ID == 0) {
			return false;
		}


		//---------------------------
		// 모니터링
		//---------------------------
		InterlockedIncrement(&_totalPacket);
		InterlockedIncrement(&_recvPacketCalc);

		if (pSession->_recvQueue.GetUseSize() <= PACKET_NET_HEADER_SIZE) {
			//---------------------------
			// 헤더보다 적음
			//---------------------------
			break;
		}

		//---------------------------
		// 헤더는 읽을 수 있음
		//---------------------------
		headerPeekRet = pSession->_recvQueue.Peek((unsigned char *) &header, PACKET_NET_HEADER_SIZE);
		if (headerPeekRet != PACKET_NET_HEADER_SIZE) {
			// 무결성 검사
			CRASH();
		}

		if (pSession->_recvQueue.GetUseSize() < (int) (PACKET_NET_HEADER_SIZE + header.len)) {
			break;
		}
		if (header.code != CPacket::PACKET_CODE) {
			CRASH();
		}

		//---------------------------
		// 패킷 풀에서 하나 꺼내기
		//---------------------------
		PRO_BEGIN(L"RecvPost_Packet");
		CPacket *pPacket = CPacket::AllocAddRef();
		if (pPacket == NULL) {
			CRASH();
		}
		pPacket->Clear();

		payloadDeqRet = pSession->_recvQueue.Dequeue(pPacket->GetBufferPtr(), header.len + PACKET_NET_HEADER_SIZE);
		if (payloadDeqRet != header.len + PACKET_NET_HEADER_SIZE) {
			CRASH();
		}

		int MoveWritePosRet = pPacket->MoveWritePos(header.len);
		if (MoveWritePosRet != header.len)
			CRASH();


		if (pPacket->Decode() == false) {
			pPacket->PrintPacket();
			CRASH();
			return false;
		}

		OnRecv(pSession->_ID, pPacket);

		msgByte += payloadDeqRet;

		//---------------------------
		// 참조카운트 하나감소
		//---------------------------
		pPacket->SubRef(5000);
		PRO_END(L"RecvPost_Packet");
	}

	//---------------------------
	// RecvPost
	//---------------------------
	return RecvPost(pSession, dfLOGIC_CPMPLETE_RECV);
}

bool CNetServer::TryAccept(SOCKET &clientSocket, sockaddr_in &clientAddr) {
	int addrlen = sizeof(clientAddr);
	SOCKET sock = accept(_listensock, (SOCKADDR *) &clientAddr, &addrlen);;
	if (sock == INVALID_SOCKET) {
		int err = WSAGetLastError();
		if (err == WSAEINTR || err == WSAENOTSOCK || err == WSAEINVAL) {
			//---------------------------
			// 리슨소켓 닫음
			// 어셉트 스레드 종료
			//---------------------------
			PostQueuedCompletionStatus(_hIOCP, dfEXIT_CODE, dfEXIT_CODE, NULL);
			return false;
		}
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"TryAccept [Error: %d]", err);
		OnError(err, L"Socket Accept");
	}

	clientSocket = sock;
	if (_emptyIndex.GetSize() == 0) {
		closesocket(sock);
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"TryAccept _emptyIndex.GetSize() == 0");
		OnError(001, L"Socket Accept _emptyIndex.GetSize() == 0"); // TODO errorcode
	}
	return true;
}

bool CNetServer::AcceptProc() {
	//---------------------------
	// Accept처리
	//---------------------------
	SOCKADDR_IN clientaddr;
	SOCKET clientsock;

	//---------------------------
	// 동기 accept()
	//---------------------------
	if (TryAccept(clientsock, clientaddr) == false) {
		return false;
	}

	//---------------------------
	// 컨텐츠에서 처리할것 블랙,화이트 리스트, 지역차단 등등
	//---------------------------
	WCHAR IPStr[20];
	GetStringIP(IPStr, clientaddr);
	if (OnConnectionRequest(IPStr, ntohl(clientaddr.sin_addr.S_un.S_addr), ntohs(clientaddr.sin_port)) == false) {
		//---------------------------
		// 컨탠츠에서 차단한 접속처리
		//---------------------------
		WCHAR IP[16] = { 0, };
		InetNtop(AF_INET, &clientaddr.sin_addr.S_un.S_addr, IP, 16);
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Accept Denied IP[%s] Port[%u]\n", IP, ntohs(clientaddr.sin_port));
		closesocket(clientsock);
	}
	//---------------------------
	// 네이글 옵션 켜기
	//---------------------------
	if (_isNagle == true) {
		BOOL optval = _isNagle;
		int optRet = setsockopt(clientsock, IPPROTO_TCP, TCP_NODELAY, (char *) &optval, sizeof(optval));
		if (optRet == SOCKET_ERROR) {
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Nagle Error[%d]\n", WSAGetLastError());
			closesocket(clientsock);
			return false;
		}
	}

	//---------------------------
	// 세션 만들기
	//---------------------------
	SESSION *pSession = CreateSession(clientsock, clientaddr);
	if (pSession == nullptr) {
		::closesocket(clientsock);
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Accept :: Cannot Create Session");
		return true;
	}

	//---------------------------
	// 컨탠츠에서 클라이언트가 완료됐을때 처리 할 가상함수
	//---------------------------
	OnClientJoin(pSession->_IPStr, pSession->_IP, pSession->_port, pSession->_ID);

	//---------------------------
	// WSARecv걸어주기
	//---------------------------
	RecvPost(pSession, dfLOGIC_ACCEPT);


	// 세션 만들때 _IOcount --
	DecrementIOCount(pSession, dfLOGIC_ACCEPT);
	//if (InterlockedDecrement(&pSession->_IOcount) == 0)
	//	ReleaseSession(pSession, dfLOGIC_ACCEPT);


	//---------------------------
	// 모니터링
	//---------------------------
	InterlockedIncrement(&_acceptCalc);
	InterlockedIncrement(&_curSessionCount);
	InterlockedIncrement(&_totalAcceptSession);

	return _isRunning;
}

bool CNetServer::NetMonitorProc() {
	//---------------------------
	// 1초마다 TPS계산
	//---------------------------
	Sleep(1000);
	CalcTPS();

	return _isRunning;
}
#ifdef df_SENDTHREAD
bool CNetServer::SendThreadProc() {
	while (_isRunning) {
		for (int i = 1; i <= this->_maxConnection; ++i) {
			if (InterlockedOr((LONG *) &_sessionContainer[i]._isAlive, FALSE) == FALSE)
				continue;
			SESSION *pSession = AcquireSession(_sessionContainer[i]._ID, 889988);
			if (pSession == nullptr)
				continue;
			if (pSession->_sendQueue.GetSize() > 0)
				SendPost(pSession, 123321);
			ReturnSession(pSession, 998899);
		}
	}
	return false;
}

#endif // df_SENDTHREAD

bool CNetServer::TimeOutProc() {
	DWORD timeoutTime;
	while (_isRunning) {
		timeoutTime = timeGetTime();
		Sleep(_timeoutMillisec);
		for (int i = 1; i <= this->_maxConnection; ++i) {
			if (InterlockedOr((LONG *) &_sessionContainer[i]._IOcount,0) & 0x80000000 != 0) continue;
			if (InterlockedOr((LONG *) &_sessionContainer[i]._isAlive, FALSE) == FALSE) continue;

			if (_sessionContainer[i]._lastRecvdTime >= timeoutTime) continue;
			this->OnTimeout(_sessionContainer[i]._ID);
		}
	}
	return false;
}
bool CNetServer::SendPost(SESSION *pSession, int logic) {
	if (pSession == NULL) {
		CRASH();
	}
	if (InterlockedOr((long *) &pSession->_isAlive, FALSE) == FALSE) {
		return false;
	}
	if (pSession->_ID == 0) {
		return false;
	}
	//---------------------------
	// 	   Send중인지 확인
	//---------------------------
	BOOL isSend = InterlockedExchange(&pSession->_IOFlag, TRUE);
	if (isSend == TRUE) {
		return FALSE;
	}
	//---------------------------
	// 	   Send가능한데 보낼게 없는지 확인
	//---------------------------
	if (pSession->_sendQueue.GetSize() == 0) {
		if (InterlockedExchange(&pSession->_IOFlag, FALSE) == FALSE) {
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendPost(%d) _IOFlag Exchange false to false", logic);
			CRASH();
		}
		return FALSE;
	}
	//---------------------------
	// IOCount ++
	//---------------------------
	IncrementIOCount(pSession, logic + dfLOGIC_INCREMENT_IO);
	//InterlockedIncrement(&pSession->_IOcount);

	//---------------------------
	// 보낼게 있음
	// 
	// WSABUF 셋팅
	//---------------------------
	WSABUF bufferSet[dfSESSION_SEND_PACKER_BUFFER_SIZE]={0};

	SetWSABuffer(bufferSet, pSession, FALSE, logic + dfLOGIC_SET_BUFFER);



	//---------------------------
	// 오버랩 초기화
	//---------------------------
	memset(&pSession->_sendOverlapped, 0, sizeof(pSession->_sendOverlapped));
	//---------------------------
	// WSASend()
	//---------------------------
	SOCKET sock = InterlockedOr64((LONG64 *)&pSession->_sock, 0);
	LONG sendPacketCnt = InterlockedOr((LONG *)&pSession->_sendPacketCnt, 0); // TODO ?
	int sendRet = WSASend(sock, bufferSet, sendPacketCnt, nullptr, 0, &pSession->_sendOverlapped, nullptr);
	if (sendRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			CancelIoEx((HANDLE) pSession->_sock, nullptr);
			if (err != 10053 && err != 10054 && err != 10064 && err != 10038) {
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"//// WSASend(%d) ERROR [%d]", logic, err);
				//CRASH();
			}
			//---------------------------
			//	Error Fail WSASend
			//  IOCount --
			//---------------------------
			DecrementIOCount(pSession, logic + dfLOGIC_DECREMENT_IO);
			//if (InterlockedDecrement(&pSession->_IOcount) == 0)
			//	ReleaseSession(pSession, logic + dfLOGIC_DECREMENT_IO);

			return false;
		}
		//CLogger::_Log(dfLOG_LEVEL_ERROR, L"//// WSASend WSA_IO_PENDING [%d]", err);
	}

	return true;
}

bool CNetServer::RecvPost(SESSION *pSession, int logic) {
	if (pSession == NULL) {
		CRASH();
	}
	if (InterlockedOr((long *) &pSession->_isAlive, FALSE) == FALSE) {
		return false;
	}
	if (InterlockedOr64((LONG64 *) &pSession->_ID, 0) == 0) {
		CRASH();
	}
	//---------------------------
	// IOCount ++
	//---------------------------
	IncrementIOCount(pSession, logic + dfLOGIC_INCREMENT_IO);
	//InterlockedIncrement(&pSession->_IOcount);

	//---------------------------
	// WSABUF 셋팅
	// 0 : 현재 RecvQ.rear, 1 : RecvQ 의 시작지점
	//---------------------------
	WSABUF bufferSet[2];
	DWORD flag = 0;
	DWORD byteRecvs;

	SetWSABuffer(bufferSet, pSession, TRUE, logic + dfLOGIC_SET_BUFFER);

	//---------------------------
	// 오버랩 초기화
	//---------------------------
	memset(&pSession->_recvOverlapped, 0, sizeof(pSession->_recvOverlapped));

	//---------------------------
	//WSARecv()
	//---------------------------
	SOCKET sock = InterlockedOr64((LONG64 *) &pSession->_sock, 0);
	int recvRet = WSARecv(sock, bufferSet, 2, &byteRecvs, &flag, &pSession->_recvOverlapped, nullptr);
	if (recvRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			CancelIoEx((HANDLE) pSession->_sock, nullptr);
			if (err != 10053 && err != 10054 && err != 10064 && err != 10038) {
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"//// %d :: WSARecv ERROR [%d]\n", logic, err);
				CRASH();

			}
			//---------------------------
			//	Error : Fail WSARecv
			//  IOCount --
			//---------------------------
			DecrementIOCount(pSession, logic + dfLOGIC_DECREMENT_IO);
			//if (InterlockedDecrement(&pSession->_IOcount) == 0)
			//	ReleaseSession(pSession, logic + dfLOGIC_DECREMENT_IO);
			return false;
		}
	}
	return true;
}

bool CNetServer::SetWSABuffer(WSABUF *BufSets, SESSION *pSession, bool isRecv, int logic) {
	if (isRecv) {
		//---------------------------
		// recv버퍼에 넣기
		//---------------------------
		char *pBuf = pSession->_recvQueue.GetBufferPtr();
		char *pRear = pSession->_recvQueue.GetRearBufferPtr();
		int  enSize = pSession->_recvQueue.DirectEnqueueSize();
		int  frSize = pSession->_recvQueue.GetFreeSize();
		BufSets[0].buf = pRear;
		BufSets[0].len = enSize;
		BufSets[1].buf = pBuf;
		BufSets[1].len = frSize - enSize;

		//---------------------------
		// 무결성 검사
		//---------------------------
		if (BufSets[0].len + BufSets[1].len != frSize) {
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"SetWSABuffer(%d) :: BufSets[0].len + BufSets[1].len != FreeSize", logic);
			CRASH();
		}
	} else {
		//---------------------------
		// SendQ의 패킷을 WSABUF에 등록
		//---------------------------
		//---------------------------
		// 패킷을 얼마나 보낼지
		//---------------------------
		int numOfPacket = pSession->_sendQueue.GetSize();

		DWORD snapSize = min(dfSESSION_SEND_PACKER_BUFFER_SIZE, numOfPacket + pSession->_sendPacketCnt);
		CPacket *pPacket = nullptr;


		//---------------------------
		// wsabuffer에 등록
		//---------------------------
		for (int i = pSession->_sendPacketCnt; i < snapSize; i++) {
			pPacket = nullptr;
			pSession->_sendQueue.Dequeue(&pPacket);
			pSession->_pSendPacketBufs[i] = pPacket;
			BufSets[i].buf =(char*) pPacket->GetSendPtr();
			BufSets[i].len = pPacket->GetSendSize();
		}

		//---------------------------
		// 처리한만큼 개수 저장
		//---------------------------
		InterlockedExchange(&pSession->_sendPacketCnt, snapSize);
	}

	return true;
}

bool CNetServer::IncrementIOCount(SESSION *pSession, int logic) {
	//---------------------------
	// IOCount++
	//---------------------------
	if (pSession == NULL) CRASH();
	//DWORD retval = InterlockedIncrement(&pSession->_IOcount);
	if ((InterlockedIncrement(&pSession->_IOcount) & 0x80000000) != 0) {
		if (InterlockedDecrement(&pSession->_IOcount) == 0)
			this->ReleaseSession(pSession, logic);
		return false;
	}

#ifdef df_LOGGING_SESSION_LOGIC
	int idx = InterlockedIncrement(&pSession->_IncIndex);
	if (idx >= df_LOGGING_SESSION_LOGIC) pSession->_IncIndex = 0;
	pSession->_IncLog[idx] = logic;
#endif // df_LOGGING_SESSION_LOGIC

	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"IncrementIOCount() ID[%lld] :: logic[%d]", pSession->_ID, logic);
	return true;
}

bool CNetServer::DecrementIOCount(SESSION *pSession, int logic) {
	//---------------------------
	// IOCount--
	//---------------------------
	if (pSession == NULL) CRASH();
	DWORD retval = InterlockedDecrement(&pSession->_IOcount);
#ifdef df_LOGGING_SESSION_LOGIC
	int idx = InterlockedIncrement(&pSession->_DecIndex);
	if (idx >= df_LOGGING_SESSION_LOGIC) pSession->_DecIndex = 0;
	pSession->_DecLog[idx] = logic;
#endif // df_LOGGING_SESSION_LOGIC
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"DecrementIOCount() ID[%lld] :: logic[%d], IOCount[%d]", pSession->_ID, logic, retval);

	if (retval == 0) {
		//---------------------------
		// disconnect
		//---------------------------
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"DecrementIOCount() ID[%lld] :: logic[%d], IOCount[%d]\t call ReleaseSession()", pSession->_ID, logic, retval);
		ReleaseSession(pSession, logic + dfLOGIC_RELEASE_SESSION);
		return false;
	}
	if (retval < 0) {
		//---------------------------
		// ERROR
		//---------------------------
		CLogger::_Log(dfLOG_LEVEL_ERROR,
			L"DecrementIOCount(%d) pSession->_IOcount [%d]\n\
SOCK[%d] :: recv [%d]byte, send [%d]byte, IOCount[%d], _IOFlag [%d]",
logic, pSession->_IOcount, pSession->_sock, pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetSize(), pSession->_IOcount, pSession->_IOFlag);
		CRASH();
	}
	return true;
}



CNetServer::SESSION *CNetServer::AcquireSession(SESSION_ID sessionID, int logic) {
	SESSION *pSession = FindSession(sessionID);
#ifndef df_LOGGING_SESSION_LOGIC
	if ((InterlockedIncrement(&pSession->_IOcount) & 0x80000000) != 0) {
		if (InterlockedDecrement(&pSession->_IOcount) == 0)
			this->ReleaseSession(pSession, logic);
		return nullptr;
	}
#else
	IncrementIOCount(pSession, logic);
#endif // !df_LOGGING_SESSION_LOGIC

	if (pSession->_ID != sessionID) {
		if (InterlockedDecrement(&pSession->_IOcount) == 0)
			this->ReleaseSession(pSession, logic);
		return nullptr;
	}

	return pSession;
}

void CNetServer::ReturnSession(SESSION *pSession, int logic) {
	if (pSession == NULL) CRASH();
#ifndef df_LOGGING_SESSION_LOGIC
	//---------------------------
	// IOCount--
	//---------------------------
	DWORD IOcount = InterlockedDecrement(&pSession->_IOcount);
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"DecrementIOCount() ID[%lld] :: logic[%d], IOCount[%d]", pSession->_ID, logic, IOcount);

	if (IOcount == 0) {
		//---------------------------
		// disconnect
		//---------------------------
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"DecrementIOCount() ID[%lld] :: logic[%d], IOCount[%d]\t call ReleaseSession()", pSession->_ID, logic, IOcount);
		ReleaseSession(pSession, logic + dfLOGIC_RELEASE_SESSION);
	}
	if (IOcount < 0) {
		//---------------------------
		// ERROR
		//---------------------------
		CLogger::_Log(dfLOG_LEVEL_ERROR,
			L"DecrementIOCount(%d) pSession->_IOcount [%d]\n\
SOCK[%d] :: recv [%d]byte, send [%d]byte, IOCount[%d], _IOFlag [%d]",
logic, pSession->_IOcount, pSession->_sock, pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetSize(), pSession->_IOcount, pSession->_IOFlag);
		CRASH();
	}
#else
	DecrementIOCount(pSession, logic);
#endif // !df_LOGGING_SESSION_LOGIC
}


bool CNetServer::ReleaseSession(SESSION *pSession, int logic) {
	//---------------------------
	// Release Session
	//---------------------------
	if (pSession == NULL) {
		CRASH();
	}
	SESSION_ID ID = pSession->_ID;
	if (InterlockedCompareExchange(&pSession->_IOcount, 0x80000000, 0) != 0) {
		return false;
	}
	LONG64 idRet = InterlockedExchange64((LONG64 *) &pSession->_ID, 0);
	if (idRet != ID) {
		CRASH();
	}
	InterlockedExchange(&pSession->_isAlive, FALSE);

	closesocket(pSession->_sock);
	//---------------------------
	// Session관리 컨테이너에서 삭제
	//---------------------------
	int sockRet = InterlockedExchange((long *) &pSession->_sock, INVALID_SOCKET);
	if (sockRet == INVALID_SOCKET) {
		CRASH();
	}


	//---------------------------
	// Sendq에 있던거 풀에 다시넣기
	//---------------------------
	DWORD sendedPacketCnt = InterlockedExchange(&pSession->_sendPacketCnt, 0);

	CPacket *pPacket = nullptr;
	for (int i = 0; i < sendedPacketCnt; ++i) {
		pPacket = pSession->_pSendPacketBufs[i];
		pPacket->SubRef();
	}
	while (pSession->_sendQueue.Dequeue(&pPacket))
		pPacket->SubRef();
	pSession->_recvQueue.ClearBuffer();

	InterlockedExchange(&pSession->_IOFlag, FALSE);

	OnClientLeave(ID);
	USHORT idx = SessionIDtoIndex(ID);
	if (idx == 0) CRASH();
	_emptyIndex.Push(idx);

	//---------------------------
	// 	   모니터링
	//---------------------------
	InterlockedIncrement(&_totalDisconnectSession);
	InterlockedDecrement(&_curSessionCount);
	return true;
}


CNetServer::SESSION *CNetServer::CreateSession(SOCKET sock, sockaddr_in clientaddr) {
	//---------------------------
	// ID생성
	//---------------------------
	SESSION_ID id = GenerateSessionID();
	if (id == 0) {
		return nullptr;
	}
	//---------------------------
	// 할당
	//---------------------------
	SESSION *pSession = FindSession(id);

	//---------------------------
	// 초기화
	//---------------------------
	InitializeSRWLock(&pSession->_lock);
	InterlockedExchange64((LONG64 *) &pSession->_ID, 0);
	InterlockedIncrement(&pSession->_IOcount);
	InterlockedAnd((long *) &pSession->_IOcount, 0x7fffffff);

	InterlockedExchange(&pSession->_isAlive, TRUE);
	InterlockedExchange(&pSession->_IOFlag, FALSE);
	InterlockedExchange(&pSession->_sendPacketCnt, 0);

	ZeroMemory(&pSession->_recvOverlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&pSession->_sendOverlapped, sizeof(WSAOVERLAPPED));
	pSession->_recvQueue.ClearBuffer();
	pSession->_sendQueue.Clear();

	//---------------------------
	// 정보 셋팅
	//---------------------------
	InterlockedExchange(&pSession->_ID, id);
	InterlockedExchange(&pSession->_sock, sock);
	ZeroMemory(&pSession->_IPStr, sizeof(pSession->_IPStr));
	GetStringIP(pSession->_IPStr, clientaddr);
	pSession->_IP = ntohl(clientaddr.sin_addr.S_un.S_addr);
	pSession->_port = ntohs(clientaddr.sin_port);
	InterlockedExchange(&pSession->_lastRecvdTime, timeGetTime());

	//---------------------------
	// IOCP
	//---------------------------
	HANDLE hResult = CreateIoCompletionPort((HANDLE) sock, _hIOCP, (ULONG_PTR) pSession->_ID, NULL);
	if (hResult == NULL) {
		int err = WSAGetLastError();
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"////// Accept CreateIoCompletionPort() errcode[%d]", err);
		OnError(err, L"Accept CreateIoCompletionPort()");
		return nullptr;
	}

	return pSession;
}

SESSION_ID CNetServer::GenerateSessionID() {
	//---------------------------
	// 	   Session ID 생성
	//---------------------------
	SESSION_ID id = 0;
	do {
		if (_emptyIndex.GetSize() == 0)
			break;

		_emptyIndex.Pop((USHORT *) &id);
		if (id == 0) CRASH();
		id = id << (8 * 6);

		id |= _IDGenerater;
		++_IDGenerater;

	} while (0);
	return id;
}

USHORT CNetServer::SessionIDtoIndex(SESSION_ID sessionID) {
	USHORT idx = sessionID >> (8 * 6);
	if (sessionID != 0 && idx == 0) CRASH();
	return idx;
}

void CNetServer::InitializeIndex() {
	for (USHORT i = _maxConnection; i > 0; i--) {
		_emptyIndex.Push(i);
	}
}

CNetServer::SESSION *CNetServer::FindSession(SESSION_ID sessionID) {
	int idx = SessionIDtoIndex(sessionID);
	return &_sessionContainer[idx];
}

#pragma region LOCK
void CNetServer::SessionLock(SESSION *pSession) {
	AcquireSRWLockExclusive(&pSession->_lock);
}

void CNetServer::SessionUnlock(SESSION *pSession) {
	ReleaseSRWLockExclusive(&pSession->_lock);
}

void CNetServer::SessionContainerLock() {
	AcquireSRWLockExclusive(&_sessionContainerLock);
}

void CNetServer::SessionContainerUnlock() {
	ReleaseSRWLockExclusive(&_sessionContainerLock);
}
void CNetServer::CalcTPS() {
	_acceptPerSec = InterlockedExchange(&_acceptCalc, 0);
	_recvPacketPerSec = InterlockedExchange(&_recvPacketCalc, 0);
	_sendPacketPerSec = InterlockedExchange(&_sendPacketCalc, 0);

}

CNetServer::MoniteringInfo CNetServer::GetMoniteringInfo() {
	MoniteringInfo info;
	info._workerThreadCount = _maxRunThreadCount;
	info._runningThreadCount = _workerThreadCount;
	info._acceptPerSec = _acceptPerSec;
	info._recvPacketPerSec = _recvPacketPerSec;
	info._sendPacketPerSec = _sendPacketPerSec;
	info._totalAcceptSession = _totalAcceptSession;
	info._totalPacket = _totalPacket;
	info._totalProecessedBytes = _totalProcessedBytes;
	info._totalReleaseSession = _totalDisconnectSession;
	info._sessionCnt = _curSessionCount;
	//info._stackCapacity = 0;
	info._stackSize = _emptyIndex.GetSize();
	info._queueSize = 0;
	//info._queueCapacity = 0;
	//info._maxCapacity = 0;
	for (int i = 1; i <= _maxConnection; i++)
		info._queueSize += _sessionContainer[i]._sendQueue.GetSize();
	info._queueSizeAvg = info._queueSize / _maxConnection;
	return info;
}
void CNetServer::ResetMonitor() {
	_curSessionCount = 0;
	_totalPacket = 0;
	_recvPacketCalc = 0;
	_recvPacketPerSec = 0;
	_sendPacketCalc = 0;
	_sendPacketPerSec = 0;

	_totalProcessedBytes = 0;

	_acceptCalc = 0;
	_acceptPerSec = 0;
	_totalAcceptSession = 0;
	_totalDisconnectSession = 0;

}
#pragma endregion
