#include "pch.h"
#include "CClient.h"

#define dfEXIT_CODE 0xFFFFFFFF // GQCS()에서 이게 오면 종료
#define dfLEAVE_CODE 0xFFFFFF00 // GQCS()에서 이게 오면 OnClientLeave 호출

CClient::CClient(bool isEncryption) {
	InitializeSRWLock(&_lock);

	//ResetMonitor();

	_maxRunThreadCount = 0;
	_workerThreadCount = 0;
	_maxConnection = 0;
	_isNagle = false;

	_isRunning = false;
	_numThreads = 0;

	_hIOCP = INVALID_HANDLE_VALUE;
	_tWorkers = nullptr;
	_sessionContainer = nullptr;
	_emptyIndex.Clear();

	_IDGenerater = 1;
	_isEncryptionPacket = isEncryption;
}

CClient::~CClient() {
	if (_pConfigData != nullptr)
		delete _pConfigData;
	CloseHandle(_hIOCP);
	WSACleanup();
	_LOG(dfLOG_LEVEL_NOTICE, L"===============================END CLIENT===============================");
}

bool CClient::Start(BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	_workerThreadCount = workerThreadCount;
	_maxRunThreadCount = max(workerThreadCount, maxRunThreadCount);
	_isNagle = nagle;
	_maxConnection = maxConnection;

	Startup();
	_LOG(dfLOG_LEVEL_NOTICE, L"///////////// Client Ready");

	return true;
}


void CClient::Quit() {
	//---------------------------
	// 서버 종료
	// _isRunning = false로 모니터링 스레드 종료
	// 종료코드를 완료통지에 넣어 워커 스레드 종료
	//---------------------------
	_LOG(dfLOG_LEVEL_NOTICE, L"CNetServer Quit Start");
	_isRunning = false;
	PostQueuedCompletionStatus(_hIOCP, dfEXIT_CODE, dfEXIT_CODE, NULL);
	delete[] _tWorkers;
}

bool CClient::Connect(const WCHAR *serverIP, USHORT serverPort) {
	SOCKET sock = CreateSocket();
	SOCKADDR_IN	addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverPort);
	InetPton(AF_INET, serverIP, &addr.sin_addr);

	if (TryConnectServer(sock, addr) == false) {
		// 실패
		_LOG(dfLOG_LEVEL_ERROR, L"Fail Connect Server");
		closesocket(sock);
		return false;
	}

	// 연결 완료
	SESSION *pSession = CreateSession(sock, addr);
	if (pSession == nullptr) {
		_LOG(dfLOG_LEVEL_ERROR, L"Full Connection!! [%d]", CClient::GetSessionCount());
		return false;
	}
	OnEnterServer(pSession->_ID);
	RecvPost(pSession);
	DecrementIOCount(pSession);

	InterlockedIncrement(&_totalConnectSession);
	//InterlockedIncrement(&_connectCalc);

	return true;
}

bool CClient::DisconnectSession(SESSION_ID sessionID) {
	//---------------------------
	// 세션 끊기
	//---------------------------
	bool ret = false;
	SESSION *pSession = AcquireSession(sessionID);
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_DEBUG, L"//Disconnect ERROR :: can not find session..");
		// TODO ERRORCODE
		OnError(111, L"Disconnect ERROR :: can not find session..");
		return false;
	}
	_LOG(dfLOG_LEVEL_DEBUG, L"//Disconnect id[%d]", sessionID);

	InterlockedExchange(&pSession->_isAlive, FALSE);
	ret = CancelIoEx((HANDLE) pSession->_sock, nullptr);

	ReturnSession(pSession);
	return ret;
}

bool CClient::SendPacket(SESSION_ID sessionID, Packet *pPacket) {
	if (pPacket == nullptr)
		return false;
	pPacket->AddRef();
	//---------------------------
	// 세션찾기
	//---------------------------
	SESSION *pSession = AcquireSession(sessionID);
	if (pSession == NULL) {
		//_LOG(dfLOG_LEVEL_DEBUG, L"//SendPacket ERROR :: can not find session..");
		OnError(123, L"SendPacket ERROR :: can not find session..");
		pPacket->SubRef();
		return false;
	}
	//---------------------------
	// 지워진(끊어진) 세션
	//---------------------------
	if (!InterlockedOr((LONG *) &pSession->_isAlive, 0)) {
		//_LOG(dfLOG_LEVEL_DEBUG, L"//SendPacket ERROR :: Session is colsed..");
		OnError(321, L"SendPacket ERROR :: Session is colsed..");
		pPacket->SubRef();
		return false;
	}
	//PRO_BEGIN(L"SendPacket");
	//---------------------------
	// 페킷 포인터를 센드큐에
	//---------------------------
	if (_isEncryptionPacket)
		pPacket->SetNetHeader();
	else
		pPacket->SetLanHeader();
	pSession->_sendQueue.enqueue(pPacket);
	//---------------------------
	// monitor
	//---------------------------
	//InterlockedIncrement(&_sendPacketCalc);
	SendPost(pSession);
	ReturnSession(pSession);
	//PRO_END(L"SendPacket");
	return true;
}

BOOL CClient::DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr) {
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

void CClient::Startup() {
	if (_isRunning == true) {
		_LOG(dfLOG_LEVEL_NOTICE, L"///// Server Already Running");
		return;
	}
	_isRunning = true;
	//---------------------------
	// 문자열 로컬 세팅
	//---------------------------
	setlocale(LC_ALL, "korean");
	_wsetlocale(LC_ALL, L"korean");

	//---------------------------
	// 정말 1ms로 카운팅
	//---------------------------
	timeBeginPeriod(1);



	_LOG(dfLOG_LEVEL_NOTICE, L"==============================START CLIENT==============================");

	//---------------------------
	// 네트워크 초기화 
	//---------------------------
	SetWSAStartUp();
	_LOG(dfLOG_LEVEL_NOTICE, L"///// SetWSAStartUp() Ok..");
	CreateIOCP();
	_LOG(dfLOG_LEVEL_NOTICE, L"///// CreateIOCP() Ok..");


	//---------------------------
	// 세션 컨테이너 생성
	//---------------------------
	_sessionContainer = new SESSION[(int) (_maxConnection + (u_short) 1)];

	InitializeIndex();
	_LOG(dfLOG_LEVEL_NOTICE, L"///// Create Session Container Ok..");

	//---------------------------
	// 스레드 실행
	//---------------------------
	BeginThreads();
	_LOG(dfLOG_LEVEL_NOTICE, L"///// BeginThreads() Ok..");
}

void CClient::BeginThreads() {
	// WORKER
	_tWorkers = new CThread[_workerThreadCount]();
	for (int i = 0; i < _workerThreadCount; i++) {
		_tWorkers[i].SetThreadName(L"NetServer Worker Thread");
		_tWorkers[i].Launch(
			[](LPVOID arg) {
				CClient *pClient = (CClient *) arg;
				pClient->OnGQCS();
			},
			this);
	}

	// MONITORING
	/*_tMonitoring.Launch(
		[](LPVOID arg) {
			CClient *pClient = (CClient *) arg;
			pClient->NetMonitorProc();
		},
		this);*/
}

bool CClient::OnGQCS() {
	DWORD transferredSize = 0;
	SESSION_ID completionKey = 0;
	WSAOVERLAPPED *pOverlapped = nullptr;
	SESSION *pSession = nullptr;
	while (!_isRunning) YieldProcessor;
	while (_isRunning) {
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
				break;
			}
			//---------------------------
			// 완료통지 실패
			//---------------------------
			DWORD err = WSAGetLastError();
			_LOG(dfLOG_LEVEL_ERROR, L"overlapped is NULL ERROR CODE [%d]", err);
			OnError(err, L"IOCP ERROR :: overlapped is NULL");
			return true;
		} else if (pOverlapped == (OVERLAPPED *) dfLEAVE_CODE) {
			OnLeaveServer(completionKey);
			continue;
		}

		//---------------------------
		// completionKey(ID) 로 세션포인터 찾기
		//---------------------------
		pSession = FindSession(completionKey);
		if (pSession == NULL) {
			continue;
		}

		//SESSION_LOCK(pSession);
		do {
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
				else { //if (pOverlapped == &pSession->_sendOverlapped) {
					SendProc(pSession, transferredSize);
				}
			}
		} while (0);
		//SESSION_UNLOCK(pSession);
		//---------------------------
		// 	   IOCount --
		//---------------------------
		DecrementIOCount(pSession);
		//if (InterlockedDecrement(&pSession->_IOcount) == 0)
		//	ReleaseSession(pSession, dfLOGIC_WORKER + dfLOGIC_DECREMENT_IO);

	}
	return _isRunning;
}

bool CClient::SendProc(SESSION *pSession, DWORD transferredSize) {
	if (pSession == NULL) {
		CRASH();
	}
	//---------------------------
	// 완료통지 온 패킷 지우기
	//---------------------------
	Packet *pPacket;
	DWORD sendedPacketCnt = InterlockedExchange(&pSession->_sendPacketCnt, 0);

	for (int i = 0; i < sendedPacketCnt; ++i) {
		pPacket = pSession->_pSendPacketBufs[i];
		pPacket->SubRef();
		pPacket = nullptr;
	}
	OnSend(pSession->_ID);
	//---------------------------
	// 	   Send가 끝났다
	//---------------------------
	InterlockedExchange(&pSession->_IOFlag, FALSE);
	//InterlockedAdd64(&_sendedPacketCalc, sendedPacketCnt);
	//InterlockedAdd64(&_sendProcessedBytesCalc, transferredSize);
	//InterlockedAdd64(&_totalProcessedByte, transferredSize);

	//---------------------------
	// SendQ에 보낼것이 남아있으면 Send
	//---------------------------
	SendPost(pSession);
	return true;
}

bool CClient::RecvProc(SESSION *pSession, DWORD transferredSize) {
	//---------------------------
	// recv신호가 옴
	//---------------------------

	//---------------------------
	// 온만큼 링버퍼 Rear빼주기
	//---------------------------
	int movRet = pSession->_recvQueue.MoveRear(transferredSize);
	if (transferredSize != movRet) {
		_LOG(dfLOG_LEVEL_ERROR, L" ID[%lld] :: transferredSize[%d] != movRet[%d]", pSession->_ID, transferredSize, movRet);
		CRASH();
	}
	//---------------------------
	// 	   반복문 돌며 패킷 처리
	//---------------------------
	for (;;) {
		Packet *pPacket = Packet::AllocAddRef();
		if (TryGetRecvPacket(pSession, pPacket)) {
			InterlockedIncrement(&_totalPacket);
			//InterlockedIncrement(&_recvPacketCalc);
			OnRecv(pSession->_ID, pPacket);
		} else {
			pPacket->SubRef();
			break;
		}
		pPacket->SubRef();
	}

	//---------------------------
	// RecvPost
	//---------------------------
	return RecvPost(pSession);
}

bool CClient::TryConnectServer(SOCKET &socket, sockaddr_in &addr) {
	if (socket == INVALID_SOCKET) return false;

	FD_SET wset;
	FD_SET errset;

	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 200000;

	int connectRet = connect(socket, (SOCKADDR *) &addr, sizeof(addr));

	if (connectRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK) {
			FD_ZERO(&wset);
			FD_ZERO(&errset);

			FD_SET(socket, &wset);
			FD_SET(socket, &errset);


			// 연결 실패시 slelect로 한번더 대기
			int retval = select(0, nullptr, &wset, &errset, &tval);

			if (retval > 0) {
				if (FD_ISSET(socket, &wset)) {
					return true;
				} else if (FD_ISSET(socket, &errset)) {
					_LOG(dfLOG_LEVEL_ERROR, L"Select Connect Error %d", err);
					return false;
				}
			}
			_LOG(dfLOG_LEVEL_ERROR, L"Unusual Connect Error %d", err);
			return false;
		}

		if (err != WSAEISCONN) {
			_LOG(dfLOG_LEVEL_ERROR, L"Unusual Connect Error %d", err);
			return false;
		}
	}

	return true;
}

//bool CClient::NetMonitorProc() {
//	//---------------------------
//	// 1초마다 TPS계산
//	//---------------------------
//	while (!_isRunning) YieldProcessor;
//	while (_isRunning) {
//
//		Sleep(1000);
//		if (!_isRunning) break;
//		OnMonitoringPerSec();
//		//CalcTPS();
//	}
//
//	return _isRunning;
//}

bool CClient::SendPost(SESSION *pSession) {
	if (pSession == NULL) {
		CRASH();
	}
	if (InterlockedOr((long *) &pSession->_isAlive, FALSE) == FALSE) {
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
	if (pSession->_sendQueue.GetSize() <= 0) {
		if (InterlockedExchange(&pSession->_IOFlag, FALSE) == FALSE) {
			_LOG(dfLOG_LEVEL_ERROR, L"SendPost() _IOFlag Exchange false to false");
			CRASH();
		}
		return FALSE;
	}
	//PRO_BEGIN(L"SendPost");


	//---------------------------
	// 보낼게 있음
	// 
	// WSABUF 셋팅
	//---------------------------
	WSABUF bufferSet[dfSESSION_SEND_PACKER_BUFFER_SIZE] = { 0 };

	SetWSABuffer(bufferSet, pSession, FALSE);



	//---------------------------
	// 오버랩 초기화
	//---------------------------
	memset(&pSession->_sendOverlapped, 0, sizeof(pSession->_sendOverlapped));

	//---------------------------
	// IOCount ++
	//---------------------------
	IncrementIOCount(pSession);
	//InterlockedIncrement(&pSession->_IOcount);

	//---------------------------
	// WSASend()
	//---------------------------
	SOCKET sock = InterlockedOr64((LONG64 *) &pSession->_sock, 0);
	LONG sendPacketCnt = InterlockedOr((LONG *) &pSession->_sendPacketCnt, 0); // TODO ?
	int sendRet = WSASend(sock, bufferSet, sendPacketCnt, nullptr, 0, &pSession->_sendOverlapped, nullptr);
	if (sendRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10053 && err != 10054 && err != 10064 && err != 10038) {
				_LOG(dfLOG_LEVEL_ERROR, L"//// WSASend() ERROR [%d]", err);
				//CRASH();
			}
			//---------------------------
			//	Error Fail WSASend
			//  IOCount --
			//---------------------------
			DecrementIOCount(pSession);
			//if (InterlockedDecrement(&pSession->_IOcount) == 0)
			//	ReleaseSession(pSession, logic + dfLOGIC_DECREMENT_IO);

			//PRO_END(L"SendPost");
			return false;
		}
		//_LOG(dfLOG_LEVEL_ERROR, L"//// WSASend WSA_IO_PENDING [%d]", err);
	}
	//PRO_END(L"SendPost");

	return true;
}

bool CClient::RecvPost(SESSION *pSession) {
	if (pSession == NULL) {
		CRASH();
	}
	if (InterlockedOr((long *) &pSession->_isAlive, FALSE) == FALSE) {
		return false;
	}

	//PRO_BEGIN(L"RecvPost");


	//---------------------------
	// IOCount ++
	//---------------------------
	IncrementIOCount(pSession);
	//InterlockedIncrement(&pSession->_IOcount);

	//---------------------------
	// WSABUF 셋팅
	// 0 : 현재 RecvQ.rear, 1 : RecvQ 의 시작지점
	//---------------------------
	WSABUF bufferSet[2];
	DWORD flag = 0;
	DWORD byteRecvs;

	SetWSABuffer(bufferSet, pSession, TRUE);

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
			if (err != 10053 && err != 10054 && err != 10064 && err != 10038) {
				_LOG(dfLOG_LEVEL_ERROR, L"//// RecvPost() :: WSARecv ERROR [%d]\n", err);

			}
			//---------------------------
			//	Error : Fail WSARecv
			//  IOCount --
			//---------------------------
			DecrementIOCount(pSession);
			//if (InterlockedDecrement(&pSession->_IOcount) == 0)
			//	ReleaseSession(pSession, logic + dfLOGIC_DECREMENT_IO);
			//PRO_END(L"RecvPost");
			return false;
		}
	}
	//PRO_END(L"RecvPost");
	return true;
}

void CClient::PostClientLeave(SESSION_ID sessionID) {
	PostQueuedCompletionStatus(_hIOCP, 0, sessionID, (LPOVERLAPPED) dfLEAVE_CODE);
}

/*bool CClient::TryGetRecvPacket(SESSION *pSession, Packet *pPacket) {


	return true;
}*/

bool CClient::TryGetRecvPacket(SESSION *pSession, Packet *pPacket) {
	if (_isEncryptionPacket) {
		PACKET_NET_HEADER header;
		pPacket->Clear();

		if (pSession->_recvQueue.GetUseSize() < PACKET_NET_HEADER_SIZE) {
			return false;
		}

		int headerPeekRet = pSession->_recvQueue.Peek((unsigned char *) &header, PACKET_NET_HEADER_SIZE);
		if (headerPeekRet != PACKET_NET_HEADER_SIZE) {
			DisconnectSession(pSession->_ID);
			return false;
		}
		if (pSession->_recvQueue.GetUseSize() < (int) (PACKET_NET_HEADER_SIZE + header.len)) {
			DisconnectSession(pSession->_ID);
			return false;
		}
		if (header.code != Packet::PACKET_CODE) {
			DisconnectSession(pSession->_ID);
			return false;
		}

		int recvPeekRet = pSession->_recvQueue.Dequeue(pPacket->GetBufferPtr(), header.len + PACKET_NET_HEADER_SIZE);
		if (recvPeekRet != header.len + PACKET_NET_HEADER_SIZE) {
			DisconnectSession(pSession->_ID);
			return false;
		}
		int MoveWritePosRet = pPacket->MoveWritePos(header.len);
		if (MoveWritePosRet != header.len) {
			pPacket->PrintPacket();
			DisconnectSession(pSession->_ID);
		}

		if (pPacket->Decode() == false) {
			DisconnectSession(pSession->_ID);
			return false;
		}
	} else {
		PACKET_LAN_HEADER header;
		pPacket->Clear();

		if (pSession->_recvQueue.GetUseSize() < PACKET_LAN_HEADER_SIZE) {
			return false;
		}

		int headerPeekRet = pSession->_recvQueue.Peek((unsigned char *) &header, PACKET_LAN_HEADER_SIZE);
		if (headerPeekRet != PACKET_LAN_HEADER_SIZE) {
			CRASH();
			return false;
		}
		if (pSession->_recvQueue.GetUseSize() < (int) (PACKET_LAN_HEADER_SIZE + header.len)) {
			return false;
		}
		int headMoveFront = pSession->_recvQueue.MoveFront(PACKET_LAN_HEADER_SIZE);
		if (headMoveFront != PACKET_LAN_HEADER_SIZE) {
			CRASH();
		}

		int recvPeekRet = pSession->_recvQueue.Dequeue(pPacket->GetWritePtr(), header.len);
		if (recvPeekRet != header.len) {
			DisconnectSession(pSession->_ID);
			return false;
		}
		int MoveWritePosRet = pPacket->MoveWritePos(header.len);
		if (MoveWritePosRet != header.len) {
			pPacket->PrintPacket();
			CRASH();
			DisconnectSession(pSession->_ID);
		}
	}

	return true;
}

bool CClient::SetWSABuffer(WSABUF *BufSets, SESSION *pSession, bool isRecv) {
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
			_LOG(dfLOG_LEVEL_ERROR, L"SetWSABuffer() :: BufSets[0].len + BufSets[1].len != FreeSize");
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
		Packet *pPacket = nullptr;


		//---------------------------
		// wsabuffer에 등록
		//---------------------------
		for (int i = pSession->_sendPacketCnt; i < snapSize; i++) {
			pPacket = nullptr;
			pSession->_sendQueue.dequeue(pPacket);
			pSession->_pSendPacketBufs[i] = pPacket;
			BufSets[i].buf = (char *) pPacket->GetSendPtr();
			BufSets[i].len = pPacket->GetSendSize();
		}

		//---------------------------
		// 처리한만큼 개수 저장
		//---------------------------
		InterlockedExchange(&pSession->_sendPacketCnt, snapSize);
	}

	return true;
}

void CClient::CreateIOCP() {
	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, _maxRunThreadCount);
	if (_hIOCP == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]", WSAGetLastError());
	}
}

bool CClient::SetWSAStartUp() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		_LOG(dfLOG_LEVEL_ERROR, L"\n////// WSAStartup() errcode[%d]\n", WSAGetLastError());
		return false;
	}
	return true;
}

SOCKET CClient::CreateSocket() {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		OnError(111, L"Create Socke Error");
		_LOG(dfLOG_LEVEL_ERROR, L"\n////// socket() errcode[%d]\n", WSAGetLastError());
		return false;
	}

	if (SetTimeWaitZero(sock) == false)
		return false;

	if (_isNagle)
		SetNagle(sock, _isNagle);
	return sock;
}

bool CClient::SetTimeWaitZero(SOCKET sock) {
	LINGER optval;

	optval.l_onoff = 1;
	optval.l_linger = 0;

	int timeOutnRet = setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &optval, sizeof(optval));
	if (timeOutnRet == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"Client Socket Linger [Error: %d]", WSAGetLastError());
		closesocket(sock);
		return false;
	}

	return true;
}

bool CClient::SetNonBlockSocket(SOCKET sock) {
	u_long on = 1;

	int retval = ioctlsocket(sock, FIONBIO, &on);

	if (retval == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"Set NonBlock Socket [Error: %d]", WSAGetLastError());
		closesocket(sock);
		return false;
	}

	return true;
}

bool CClient::SetNagle(SOCKET sock, bool sw) {
	BOOL optval = sw;

	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &optval, sizeof(optval)) == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"Socketopt Nagle [Error: %d]", WSAGetLastError());
		closesocket(sock);

		return true;
	}
	return false;
}

SESSION *CClient::AcquireSession(SESSION_ID sessionID) {
	SESSION *pSession = FindSession(sessionID);
	if ((InterlockedIncrement(&pSession->_IOcount) & 0x80000000) != 0) {
		if (InterlockedDecrement(&pSession->_IOcount) == 0)
			this->ReleaseSession(pSession);
		return nullptr;
	}

	if (pSession->_ID != sessionID) {
		if (InterlockedDecrement(&pSession->_IOcount) == 0)
			this->ReleaseSession(pSession);
		return nullptr;
	}

	return pSession;
}

void CClient::ReturnSession(SESSION *pSession) {
	if (pSession == NULL) CRASH();
	//---------------------------
	// IOCount--
	//---------------------------
	DWORD IOcount = InterlockedDecrement(&pSession->_IOcount);
	_LOG(dfLOG_LEVEL_DEBUG, L"DecrementIOCount() ID[%lld] :: IOCount[%d]", pSession->_ID, IOcount);

	if (IOcount == 0) {
		//---------------------------
		// disconnect
		//---------------------------
		_LOG(dfLOG_LEVEL_DEBUG, L"DecrementIOCount() ID[%lld] :: IOCount[%d]\t call ReleaseSession()", pSession->_ID, IOcount);
		ReleaseSession(pSession);
	}
	if (IOcount < 0) {
		//---------------------------
		// ERROR
		//---------------------------
		_LOG(dfLOG_LEVEL_ERROR,
			L"DecrementIOCount() pSession->_IOcount [%d]\n\
SOCK[%d] :: recv [%d]byte, send [%d]byte, IOCount[%d], _IOFlag [%d]",
pSession->_IOcount, pSession->_sock, pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetSize(), pSession->_IOcount, pSession->_IOFlag);
		CRASH();
	}
}

inline bool CClient::IncrementIOCount(SESSION *pSession) {
	//---------------------------
	// IOCount++
	//---------------------------
	if (pSession == NULL) CRASH();
	//DWORD retval = InterlockedIncrement(&pSession->_IOcount);
	if ((InterlockedIncrement(&pSession->_IOcount) & 0x80000000) != 0) {
		if (InterlockedDecrement(&pSession->_IOcount) == 0)
			this->ReleaseSession(pSession);
		return false;
	}


	_LOG(dfLOG_LEVEL_DEBUG, L"IncrementIOCount() ID[%lld]", pSession->_ID);
	return true;
}

inline bool CClient::DecrementIOCount(SESSION *pSession) {
	//---------------------------
	// IOCount--
	//---------------------------
	if (pSession == NULL) CRASH();
	DWORD retval = InterlockedDecrement(&pSession->_IOcount);
	_LOG(dfLOG_LEVEL_DEBUG, L"DecrementIOCount() ID[%lld] :: IOCount[%d]", pSession->_ID, retval);

	if (retval == 0) {
		//---------------------------
		// disconnect
		//---------------------------
		_LOG(dfLOG_LEVEL_DEBUG, L"DecrementIOCount() ID[%lld] :: IOCount[%d]\t call ReleaseSession()", pSession->_ID, retval);
		ReleaseSession(pSession);
		return false;
	}
	if (retval < 0) {
		//---------------------------
		// ERROR
		//---------------------------
		_LOG(dfLOG_LEVEL_ERROR,
			L"DecrementIOCount() pSession->_IOcount [%d]\n\
SOCK[%d] :: recv [%d]byte, send [%d]byte, IOCount[%d], _IOFlag [%d]",
pSession->_IOcount, pSession->_sock, pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetSize(), pSession->_IOcount, pSession->_IOFlag);
		CRASH();
	}
	return true;
}

bool CClient::ReleaseSession(SESSION *pSession) {
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

	Packet *pPacket = nullptr;
	for (int i = 0; i < sendedPacketCnt; ++i) {
		pPacket = pSession->_pSendPacketBufs[i];
		pPacket->SubRef();
	}
	while (pSession->_sendQueue.dequeue(pPacket))
		pPacket->SubRef();
	pSession->_recvQueue.ClearBuffer();

	InterlockedExchange(&pSession->_IOFlag, FALSE);

	PostClientLeave(ID);
	//OnLeaveServer(ID);
	USHORT idx = sessionIDtoIndex(ID);
	if (idx == 0) CRASH();
	_emptyIndex.push(idx);

	//---------------------------
	// 	   모니터링
	//---------------------------
	InterlockedIncrement(&_totalDisconnectSession);
	InterlockedDecrement(&_curSessionCount);
	return true;
}

SESSION *CClient::CreateSession(SOCKET sock, sockaddr_in servAddr) {
	//---------------------------
	// ID생성
	//---------------------------
	SESSION_ID id = GeneratesessionID();
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
	GetStringIP(pSession->_IPStr, servAddr);
	pSession->_IP = ntohl(servAddr.sin_addr.S_un.S_addr);
	pSession->_port = ntohs(servAddr.sin_port);

	//---------------------------
	// IOCP
	//---------------------------
	HANDLE hResult = CreateIoCompletionPort((HANDLE) sock, _hIOCP, (ULONG_PTR) pSession->_ID, NULL);
	if (hResult == NULL) {
		int err = WSAGetLastError();
		_LOG(dfLOG_LEVEL_ERROR, L"////// Connect CreateIoCompletionPort() errcode[%d]", err);
		OnError(err, L"Connect CreateIoCompletionPort()");
		return nullptr;
	}

	return pSession;
}

SESSION_ID CClient::GeneratesessionID() {
	//---------------------------
	// 	   Session ID 생성
	//---------------------------
	SESSION_ID id = 0;
	USHORT idx;
	do {
		if (_emptyIndex.GetSize() == 0)
			break;

		_emptyIndex.pop(idx);
		id = idx;
		if (id == 0) CRASH();
		id = id << (8 * 6);

		id |= _IDGenerater;
		++_IDGenerater;

	} while (0);
	return id;
}

inline USHORT CClient::sessionIDtoIndex(SESSION_ID sessionID) {
	USHORT idx = sessionID >> (8 * 6);
	if (sessionID != 0 && idx == 0) CRASH();
	return idx;
}

inline SESSION *CClient::FindSession(SESSION_ID sessionID) {
	int idx = sessionIDtoIndex(sessionID);
	return &_sessionContainer[idx];
}

void CClient::InitializeIndex() {
	for (USHORT i = _maxConnection; i > 0; i--) {
		_emptyIndex.push(i);
	}
}

//void CClient::CalcTPS() {
//	_connectPerSec = InterlockedExchange(&_connectCalc, 0);
//	_recvPacketPerSec = InterlockedExchange(&_recvPacketCalc, 0);
//	_sendPacketPerSec = InterlockedExchange(&_sendPacketCalc, 0);
//	_sendProcessedBytesTPS = InterlockedExchange64(&_sendProcessedBytesCalc, 0);
//	_sendedPacketPerSec = InterlockedExchange64(&_sendedPacketCalc, 0);
//}
//
//CClient::MoniteringInfo CClient::GetMoniteringInfo() {
//	MoniteringInfo info;
//	info._workerThreadCount = _maxRunThreadCount;
//	info._runningThreadCount = _workerThreadCount;
//	info._connectPerSec = _connectPerSec;
//	info._recvPacketPerSec = _recvPacketPerSec;
//	info._sendPacketPerSec = _sendPacketPerSec;
//	info._totalConnectSession = _totalConnectSession;
//	info._totalPacket = _totalPacket;
//	info._sendBytePerSec = _sendProcessedBytesTPS;
//	info._sendedPacketPerSec = _sendedPacketPerSec;
//	info._totalProecessedBytes = _totalProcessedByte;
//	info._totalReleaseSession = _totalDisconnectSession;
//	info._sessionCnt = _curSessionCount;
//	//info._stackCapacity = 0;
//	info._stackSize = _emptyIndex.GetSize();
//	info._queueSize = 0;
//	//info._queueCapacity = 0;
//	//info._maxCapacity = 0;
//	for (int i = 1; i <= _maxConnection; i++)
//		info._queueSize += _sessionContainer[i]._sendQueue.GetSize();
//	info._queueSizeAvg = info._queueSize / _maxConnection;
//	return info;
//}
//
//void CClient::ResetMonitor() {
//	_curSessionCount = 0;
//	_totalPacket = 0;
//	_recvPacketCalc = 0;
//	_recvPacketPerSec = 0;
//	_sendPacketCalc = 0;
//	_sendPacketPerSec = 0;
//	_sendedPacketCalc = 0;
//	_sendedPacketPerSec = 0;
//	_sendProcessedBytesCalc = 0;
//	_sendProcessedBytesTPS = 0;
//	_totalProcessedByte = 0;
//
//	_connectCalc = 0;
//	_connectPerSec = 0;
//	_totalConnectSession = 0;
//	_totalDisconnectSession = 0;
//}
