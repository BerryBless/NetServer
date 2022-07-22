#include "pch.h"
#include "CLanServer.h"
// cmd code
#define dfEXIT_CODE 0xFFFFFFFF // GQCS()에서 이게 오면 종료

// CS
#define SESSION_LOCK(pSession)		SessionLock(pSession)
#define SESSION_UNLOCK(pSession)	SessionUnlock(pSession)

#define dfMIN(a,b) a>b?b:a
#define dfMAX(a,b) a<b?b:a


CLanServer::CLanServer() {
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
	// 모니터링 변수 초기화
	//---------------------------
	ZeroMemory(&_monitor, sizeof(MONITOR));

	//---------------------------
	// 세션 컨테이너 락 초기화
	//---------------------------
	InitializeSRWLock(&_sessionContainerLock);
}

CLanServer::~CLanServer() {
	WSACleanup();
}

bool CLanServer::Start(u_long IP, u_short prot, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	if (_isRunning == true) {
		//---------------------------
		// 이미 실행중일
		//---------------------------
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running\n");
		OnError(111, L"Network is already running");
		return false;
	}
	_isRunning = true;
	//---------------------------
	// 로거 초기화
	//---------------------------
	CLogger::Initialize();
	CLogger::SetDirectory(L"serverlog");
	CLogger::SetLogLevel(dfLOG_LEVEL_ERROR);


	//---------------------------
	// 서버 정보 넣기
	//---------------------------
	_Port = prot;
	_bindIP = IP;
	_workerThreadCount = workerThreadCount;
	_maxRunThreadCount = dfMIN(workerThreadCount, maxRunThreadCount);
	_isNagle = nagle;
	_maxConnection = maxConnection;

	

	//---------------------------
	// 세팅된 정보로 리슨 만들기
	//---------------------------
	if (CreateListenSocket() == false) {
		//---------------------------
		// 리슨 소켓 생성 실패
		//---------------------------
		return false;
	}

	//---------------------------
	// 세션 컨테이너 생성
	//---------------------------
	_sessionContainer = new SESSION[_maxConnection + 1];

	InitializeIndex();
	//---------------------------
	// 스레드 핸들 배열 생성
	// _workerThreadCount + acceptThread + monitorThread
	//---------------------------
	_NumThreads = _workerThreadCount + 2;
	_hThreads = new HANDLE[(long long)_NumThreads];

	//---------------------------
	// 스레드 실행
	//---------------------------
	BeginThreads();



	return _isRunning;
}


bool CLanServer::Start(wchar_t* wsConfigPath) {
	// TODO 파일 패치로 파싱해서 실행
	return Start(INADDR_ANY, 10101, 24, 12, true, 3000);
}


void CLanServer::Quit() {
	//---------------------------
	// 서버 종료
	// _isRunning = false로 모니터링 스레드 종료
	// 종료코드를 완료통지에 넣어 워커 스레드 종료
	// _listensock을 닫아 어셉트 스레드 종료
	//---------------------------
	_isRunning = false;
	PostQueuedCompletionStatus(_hIOCP, dfEXIT_CODE, dfEXIT_CODE, NULL);
	closesocket(_listensock);
}

void CLanServer::WaitForThreadsFin() {
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

int CLanServer::GetSessionCount() {
	//---------------------------
	// 현재 연결된 세션 카운터
	//---------------------------
	int count;

	return count;
}

bool CLanServer::Disconnect(SESSION_ID SessionID) {
	//---------------------------
	// 세션 끊기?
	//---------------------------
	SESSION* pSession = FindSession(SessionID);
	if (pSession == NULL) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"//Disconnect ERROR :: can not find session..");
		OnError(dfLOGIC_DISCONNECT, L"Disconnect ERROR :: can not find session..");
		return false;
	}
	CancelIo((HANDLE)pSession->_sock);
	return true;
}

bool CLanServer::SendPacket(SESSION_ID SessionID, CPacket* pPacket) {
	//---------------------------
	// 세션찾기
	//---------------------------
	SESSION* pSession = FindSession(SessionID);
	if (pSession == NULL) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"//SendPacket ERROR :: can not find session..");
		OnError(dfLOGIC_SEND_PACKET, L"SendPacket ERROR :: can not find session..");
		return false;
	}
	//---------------------------
	// 지워진(끊어진) 세션
	//---------------------------
	if (pSession->_ID == 0) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"//SendPacket ERROR :: Session is colsed..");
		OnError(dfLOGIC_SEND_PACKET, L"SendPacket ERROR :: Session is colsed..");
		return false;
	}

	//---------------------------
	// 페킷 포인터를 센드큐에
	//---------------------------
	pPacket->SetHeader();
	pPacket->AddRef();

	pSession->_sendQueue.Enqueue(pPacket);
	//---------------------------
	// monitor
	//---------------------------
	InterlockedIncrement(&_monitor._sendPacketCalc);
	return SendPost(pSession, dfLOGIC_SEND_PACKET);
}

BOOL CLanServer::DomainToIP(const WCHAR* szDomain, IN_ADDR* pAddr)
{
	ADDRINFOW* pAddrInfo;	// IP정보
	SOCKADDR_IN* pSockAddr;

	// pAddrInfo 에 도메인을 IP로 변환한것을 "리스트로 변환 해줌" (이중포인터)
	// 외부에서 반드시 해재 해줘야함!
	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0) {
		return FALSE;
	}
	pSockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
	*pAddr = pSockAddr->sin_addr;

	FreeAddrInfo(pAddrInfo); // pAddrInfo 리스트 할당 해재!!!!!!
	return TRUE;
}

bool CLanServer::CreateListenSocket() {
	WSADATA wsa;
	//---------------------------
	// 윈속 초기화
	//---------------------------
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// WSAStartup() errcode[%d]\n", WSAGetLastError());
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"WSAStartup OK..\n");

	//---------------------------
	// socket()
	//---------------------------
	_listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_listensock == INVALID_SOCKET) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// socket() errcode[%d]\n", WSAGetLastError());
		return FALSE;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"socket() OK ..\n");

	//---------------------------
	// bind()
	//---------------------------
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_Port);
	addr.sin_addr.S_un.S_addr = htonl(_bindIP);

	int bindRet = bind(_listensock, (SOCKADDR*)&addr, sizeof(addr));
	if (bindRet == SOCKET_ERROR) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// bind() errcode[%d]\n", WSAGetLastError());
		closesocket(_listensock);
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"bind() OK ..\n");

	//---------------------------
	// listen()
	//---------------------------
	int listenRet = listen(_listensock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// listen() errcode[%d]\n", WSAGetLastError());
		closesocket(_listensock);
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"listen() OK ..\n");


	//---------------------------
	// CreateIoCompletionPort()
	//---------------------------
	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _maxRunThreadCount);
	if (_hIOCP == NULL) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"\n////// CreateIoCompletionPort() errcode[%d]\n", WSAGetLastError());
		closesocket(_listensock);
		return false;
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"CreateIoCompletionPort() OK ..\n");
	return true;
}

void CLanServer::BeginThreads() {
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
		_hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, this, 0, nullptr);
	}
	//---------------------------
	//  accept 스레드 생성
	//---------------------------
	_hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, AcceptThread, this, 0, nullptr);
	++i;
	//---------------------------
	// 모니터링 스레드 생성
	//---------------------------
	_hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
}

//============================================================
// 스래드 함수
// 매개변수로 this포인터를 받아 this포인터로 오브젝트를 불러 사용
//============================================================
#pragma region Thread Function
unsigned int __stdcall CLanServer::WorkerThread(LPVOID arg) {
	CLanServer* pServer = (CLanServer*)arg;
	while (true) {
		if (pServer->OnGQCS() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- Worker Thread IS Closed..\n");
	return 0;
}

unsigned int __stdcall CLanServer::AcceptThread(LPVOID arg) {
	CLanServer* pServer = (CLanServer*)arg;
	while (true) {
		if (pServer->AcceptProc() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- Accept Thread IS Closed..\n");
	return 0;
}

unsigned int __stdcall CLanServer::MonitorThread(LPVOID arg) {
	CLanServer* pServer = (CLanServer*)arg;
	while (true) {
		if (pServer->NetMonitorProc() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- Monitor Thread IS Closed..\n");
	return 0;
}
#pragma endregion

bool CLanServer::OnGQCS() {
	DWORD transferredSize = 0;
	SESSION_ID completionKey = 0;
	WSAOVERLAPPED* pOverlapped = nullptr;
	SESSION* pSession = nullptr;

	//---------------------------
	// GQCS()
	//---------------------------
	BOOL GQCSRet = GetQueuedCompletionStatus(_hIOCP, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);

	//---------------------------
	// 완료통지 확인
	//---------------------------
	if (pOverlapped == NULL) {
		if (transferredSize == dfEXIT_CODE && (long long)completionKey == dfEXIT_CODE) {
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
	}

	//---------------------------
	// completionKey(ID) 로 세션포인터 찾기
	//---------------------------
	pSession = FindSession(completionKey);
	if (pSession == NULL) {
		return true;
	}

	SESSION_LOCK(pSession);
	do {
		if (pSession->_ID == 0) {
			//---------------------------
			// 이미 지워진 세션
			//---------------------------
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"OnGQCS :: Deleted Session\n");
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
	SESSION_UNLOCK(pSession);
	//---------------------------
	// 	   IOCount --
	//---------------------------
	DecrementIOCount(pSession, dfLOGIC_WORKER + dfLOGIC_DECREMENT_IO);

	return true;
}

bool CLanServer::SendProc(SESSION* pSession, DWORD transferredSize) {
	if (pSession == NULL) {
		CRASH();
	}
	//---------------------------
	// 완료통지 온 패킷 지우기
	//---------------------------
	CPacket* pPacket;
	int sendedPacketCnt = pSession->_sendPacketCnt;

	for (int i = 0; i < sendedPacketCnt; ++i)
	{
		pSession->_sendQueue.Dequeue(&pPacket);

		pPacket->SubRef();
		pPacket = nullptr;
	}

	pSession->_sendPacketCnt = 0;

	//---------------------------
	// 	   Send가 끝났다
	//---------------------------
	InterlockedExchange(&pSession->_IOFlag, FALSE);


	//---------------------------
	// SendQ에 보낼것이 남아있으면 Send
	//---------------------------
	SendPost(pSession, dfLOGIC_CPMPLETE_SEND);
	return true;
}

bool CLanServer::RecvProc(SESSION* pSession, DWORD transferredSize) {
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
	USHORT header;

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
		InterlockedIncrement(&_monitor._recvPacketCalc);

		if (pSession->_recvQueue.GetUseSize() <= sizeof(USHORT)) {
			//---------------------------
			// 헤더보다 적음
			//---------------------------
			break;
		}

		//---------------------------
		// 헤더는 읽을 수 있음
		//---------------------------
		headerPeekRet = pSession->_recvQueue.Peek((char*)&header, sizeof(USHORT));
		if (headerPeekRet != sizeof(USHORT)) {
			// 무결성 검사
			CRASH();
		}

		if (pSession->_recvQueue.GetUseSize() < (int)(sizeof(USHORT) + header)) {
			//---------------------------
			// 패킷 전체크기보다 적게 남아있음
			//---------------------------
			break;
		}

		//---------------------------
		// 온전한 패킷이 온걸 확인
		// 이미 알고있는 정보는 넘어가기
		//---------------------------
		headerMoveRet = pSession->_recvQueue.MoveFront(sizeof(USHORT));
		if (headerMoveRet != sizeof(USHORT)) {
			//---------------------------
			// 무결성 검사
			//---------------------------
			CRASH();
		}

		//---------------------------
		// 패킷 풀에서 하나 꺼내기
		//---------------------------
		CPacket* pPacket = CPacket::AllocAddRef();
		if (pPacket == NULL) {
			CRASH();
		}
		pPacket->Clear();

		//---------------------------
		// 페이로드 크기를 알았으니 헤더는 제 역할을 다함
		// 페이로드만 페킷에 넣어주기
		//---------------------------
		payloadDeqRet = pSession->_recvQueue.Dequeue(pPacket->GetWritePtr(), (int)header);
		if (payloadDeqRet != header) {
			//---------------------------
			// 무결성 검사
			//---------------------------
			CRASH();
		}
		int MoveWritePosRet = pPacket->MoveWritePos(payloadDeqRet);
		if (MoveWritePosRet != payloadDeqRet)
			CRASH();
		OnRecv(pSession->_ID, pPacket);

		msgByte += (payloadDeqRet + sizeof(USHORT));

		//---------------------------
		// 참조카운트 하나감소
		//---------------------------
		pPacket->SubRef(5000);
	}

	//---------------------------
	// RecvPost
	//---------------------------
	return RecvPost(pSession, dfLOGIC_CPMPLETE_RECV);
}

bool CLanServer::AcceptProc() {
	//---------------------------
	// Accept처리
	//---------------------------
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	SOCKET clientsock;
	HANDLE hResult;

	//---------------------------
	// 동기 accept()
	//---------------------------
	clientsock = accept(_listensock, (SOCKADDR*)&clientaddr, &addrlen);
	if (clientsock == INVALID_SOCKET) {
		int err = WSAGetLastError();
		if (err == WSAENOTSOCK || err == WSAEINTR) {
			//---------------------------
			// 리슨소켓 닫음
			// 어셉트 스레드 종료
			//---------------------------
			return false;
		}
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Accept [Error: %d]", err);
		OnError(err, L"Socket Accept");
		return _isRunning;
	}
	//---------------------------
	// 컨텐츠에서 처리할것 블랙,화이트 리스트, 지역차단 등등
	//---------------------------
	if (OnConnectionRequest(clientaddr.sin_addr.S_un.S_addr, clientaddr.sin_port) == false) {
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
		int optRet = setsockopt(clientsock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));
		if (optRet == SOCKET_ERROR) {
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Nagle Error[%d]\n", WSAGetLastError());
			closesocket(clientsock);
			return false;
		}
	}

	//---------------------------
	// 세션 만들기
	//---------------------------
	SESSION* pSession = CreateSession(clientsock, clientaddr);

	//---------------------------
	// IOCP
	//---------------------------
	hResult = CreateIoCompletionPort((HANDLE)clientsock, _hIOCP, (ULONG_PTR)pSession->_ID, NULL);
	if (hResult == NULL) {
		int err = WSAGetLastError();
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"////// Accept CreateIoCompletionPort() errcode[%d]", err);
		OnError(err, L"Accept CreateIoCompletionPort()");
		return false;
	}
	IncrementIOCount(pSession, dfLOGIC_ACCEPT);
	//---------------------------
	// 컨탠츠에서 클라이언트가 완료됐을때 처리 할 가상함수
	//---------------------------
	OnClientJoin(pSession->_ID);

	//---------------------------
	// WSARecv걸어주기
	//---------------------------
	RecvPost(pSession, dfLOGIC_ACCEPT, true);

	DecrementIOCount(pSession, dfLOGIC_ACCEPT);



	//---------------------------
	// 모니터링
	//---------------------------
	InterlockedIncrement(&_monitor._acceptCount);

	return _isRunning;
}

bool CLanServer::NetMonitorProc() {
	//---------------------------
	// 1초마다 TPS계산
	//---------------------------
	Sleep(1000);

	_monitor._sendPacketTPS = _monitor._sendPacketCalc;
	_monitor._sendPacketCalc = 0;

	_monitor._recvPacketTPS = _monitor._recvPacketCalc;
	_monitor._recvPacketCalc = 0;

	return _isRunning;
}

bool CLanServer::SendThreadProc()
{
	for(;;){

	}
	return false;
}

bool CLanServer::SendPost(SESSION* pSession, int logic) {
	if (pSession == NULL) {
		CRASH();
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
	// 보낼게 있음
	// 
	// WSABUF 셋팅
	//---------------------------
	WSABUF bufferSet[100];
	DWORD byteSends;

	SetWSABuffer(bufferSet, pSession, FALSE, logic + dfLOGIC_SET_BUFFER);

	//---------------------------
	// IOCount ++
	//---------------------------
	IncrementIOCount(pSession, logic + dfLOGIC_INCREMENT_IO);

	//---------------------------
	// 오버랩 초기화
	//---------------------------
	memset(&pSession->_sendOverlapped, 0, sizeof(pSession->_sendOverlapped));
	//---------------------------
	// WSASend()
	//---------------------------
	int sendRet = WSASend(pSession->_sock, bufferSet, pSession->_sendPacketCnt, &byteSends, 0, &pSession->_sendOverlapped, nullptr);
	if (sendRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10054 && err != 10053) {
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"//// WSASend(%d) ERROR [%d]", logic, err);
				//CRASH();
			}
			//---------------------------
			//	Error Fail WSASend
			//  IOCount --
			//---------------------------
			DecrementIOCount(pSession, logic + dfLOGIC_DECREMENT_IO);
			return false;
		}
		//CLogger::_Log(dfLOG_LEVEL_ERROR, L"//// WSASend WSA_IO_PENDING [%d]", err);
	}

	return true;
}

bool CLanServer::RecvPost(SESSION* pSession, int logic, bool isAccept) {
	if (pSession == NULL) {
		CRASH();
	}
	if (pSession->_ID == 0) {
		return false;
	}
	//---------------------------
	// IOCount ++
	//---------------------------
	//if(isAccept == false)
	IncrementIOCount(pSession, logic + dfLOGIC_INCREMENT_IO);

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
	int recvRet = WSARecv(pSession->_sock, bufferSet, 2, &byteRecvs, &flag, &pSession->_recvOverlapped, nullptr);
	if (recvRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10054 && err != 10053) {
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"//// %d :: WSARecv ERROR [%d]\n", logic, err);
				//CRASH();

			}
			//---------------------------
			//	Error : Fail WSARecv
			//  IOCount --
			//---------------------------
			DecrementIOCount(pSession, logic + dfLOGIC_DECREMENT_IO);
			return false;
		}
	}
	return true;
}

bool CLanServer::SetWSABuffer(WSABUF* BufSets, SESSION* pSession, bool isRecv, int logic) {
	if (isRecv) {
		//---------------------------
		// recv버퍼에 넣기
		//---------------------------

		char* pBuf = pSession->_recvQueue.GetBufferPtr();
		char* pRear = pSession->_recvQueue.GetRearBufferPtr();
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
	}
	else {
		//---------------------------
		// SendQ의 패킷을 WSABUF에 등록
		//---------------------------
		CPacket* pPacketBufs[100];
		//---------------------------
		// 패킷을 얼마나 보낼지
		//---------------------------
		DWORD snapSize = pSession->_sendQueue.Peek(pPacketBufs, 100);
		

		//---------------------------
		// wsabuffer에 등록
		//---------------------------
		for (int i = 0; i < snapSize; ++i) {
			
			BufSets[i].buf = pPacketBufs[i]->GetSendPtr();
			BufSets[i].len = pPacketBufs[i]->GetSendSize();
		}

		//---------------------------
		// 처리한만큼 개수 저장
		//---------------------------
		pSession->_sendPacketCnt = snapSize;
	}

	return true;
}

bool CLanServer::IncrementIOCount(SESSION* pSession, int logic) {
	if (pSession == NULL) return false;
	//---------------------------
	// IOCount++
	//---------------------------
	if (pSession == NULL) CRASH();
	DWORD retval = InterlockedIncrement(&pSession->_IOcount);
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"IncrementIOCount() ID[%lld] :: logic[%d]", pSession->_ID, logic);
	return true;
}

bool CLanServer::DecrementIOCount(SESSION* pSession, int logic) {
	if (pSession == NULL) return false;
	//---------------------------
	// IOCount--
	//---------------------------
	DWORD retval = InterlockedDecrement(&pSession->_IOcount);
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

bool CLanServer::ReleaseSession(SESSION* pSession, int logic) {
	//---------------------------
	// Release Session
	//---------------------------
	if (pSession == NULL) {
		CRASH();
	}
	SESSION_ID ID = pSession->_ID;

	if (pSession->_ID != ID) {
		return false;
	}

	if (InterlockedExchange(&pSession->_isAlive, FALSE) == FALSE) {
		return false;
	}

	OnClientLeave(pSession->_ID);
	closesocket(pSession->_sock);

	//---------------------------
	// Session관리 컨테이너에서 삭제
	//---------------------------
	int idRet = InterlockedExchange64((LONG64*)&pSession->_ID, 0);
	if (idRet == 0) {
		CRASH();
	}
	int sockRet = InterlockedExchange((long*)&pSession->_sock, INVALID_SOCKET);
	if (sockRet == INVALID_SOCKET) {
		CRASH();
	}
	//---------------------------
	// Sendq에 있던거 풀에 다시넣기
	//---------------------------
	SESSION_LOCK(pSession);
	for (;;) {
		CPacket *pPacket;
		if (pSession->_sendQueue.Dequeue(&pPacket) == false) {
			break;
		}
		pPacket->SubRef();
	}
	pSession->_recvQueue.ClearBuffer();
	SESSION_UNLOCK(pSession);


	DeleteSessionData(ID);

	//---------------------------
	// 	   모니터링
	//---------------------------
	InterlockedIncrement(&_monitor._disconnectCount);
	return true;
}


CLanServer::SESSION* CLanServer::CreateSession(SOCKET sock, SOCKADDR_IN addr) {
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
	SESSION* pSession = FindSession(id);

	//---------------------------
	// 초기화
	//---------------------------
	InitializeSRWLock(&pSession->_lock);
	InterlockedExchange64 ((LONG64*)&pSession->_ID,0);
	InterlockedExchange(&pSession->_IOcount, 0);
	InterlockedExchange(&pSession->_IOFlag, FALSE);
	InterlockedExchange(&pSession->_sendPacketCnt, 0);
	InterlockedExchange(&pSession->_isAlive, TRUE);

	ZeroMemory(&pSession->_recvOverlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&pSession->_sendOverlapped, sizeof(WSAOVERLAPPED));
	pSession->_recvQueue.ClearBuffer();
	pSession->_sendQueue.Clear();

	//---------------------------
	// 정보 셋팅
	//---------------------------
	pSession->_ID = id;
	pSession->_sock = sock;
	pSession->_IP = addr.sin_addr.S_un.S_addr;
	pSession->_port = addr.sin_port;

	return pSession;
}

CLanServer::SESSION_ID CLanServer::GenerateSessionID() {
	//---------------------------
	// 	   Session ID 생성
	//---------------------------
	SESSION_ID id = 0;
	do {
		if (_emptyIndex.GetSize() == 0)
			break;

		_emptyIndex.Pop((USHORT *)&id);
		if (id == 0) CRASH();
		id = id << (8 * 6);

		id |= _IDGenerater;
		++_IDGenerater;

	} while (0);
	return id;
}

USHORT CLanServer::SessionIDtoIndex(SESSION_ID sessionID) {
	USHORT idx = sessionID >> (8 * 6);
	if (sessionID != 0 && idx == 0) CRASH();
	return idx;
}

void CLanServer::InitializeIndex() {
	for (USHORT i = _maxConnection; i > 0; i--) {
		_emptyIndex.Push(i);
	}
}

CLanServer::SESSION* CLanServer::FindSession(SESSION_ID sessionID) {
	int idx = SessionIDtoIndex(sessionID);
	return &_sessionContainer[idx];
}

void CLanServer::DeleteSessionData(SESSION_ID sessionID) {
	//---------------------------
	//delete pSession;
	//---------------------------
	USHORT idx = SessionIDtoIndex(sessionID);
	_emptyIndex.Push(idx);
}

#pragma region LOCK
void CLanServer::SessionLock(SESSION* pSession) {
	AcquireSRWLockExclusive(&pSession->_lock);
}

void CLanServer::SessionUnlock(SESSION* pSession) {
	ReleaseSRWLockExclusive(&pSession->_lock);
}

void CLanServer::SessionContainerLock() {
	AcquireSRWLockExclusive(&_sessionContainerLock);
}

void CLanServer::SessionContainerUnlock() {
	ReleaseSRWLockExclusive(&_sessionContainerLock);
}
#pragma endregion
