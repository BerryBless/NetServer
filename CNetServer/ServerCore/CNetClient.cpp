#include "pch.h"
#include "CNetClient.h"

#define dfEXIT_CODE 0xFFFFFFFF // GQCS()에서 이게 오면 종료

CNetClient::CNetClient() {
	setlocale(LC_ALL, "korean");
	_wsetlocale(LC_ALL, L"korean");
	timeBeginPeriod(1);


	InitializeSRWLock(&_lock);
	ResetMonitor();
	_client._IOcount = 0x80000000;
	_client._sock = INVALID_SOCKET;
	_isNagle = false;
	SetThreadNum(1, 1);
}

CNetClient::~CNetClient() {
	WSACleanup();
}

bool CNetClient::Connect(const WCHAR *serverIP, USHORT serverPort) {
	int ret;
	while (ret = InterlockedCompareExchange(&_client._IOcount, 0, 0x80000000) != 0x80000000) {
		//CRASH();
	}

	wcscpy_s(_serverIP, _countof(_serverIP), serverIP);
	_serverPort = serverPort;

	//InterlockedIncrement(&_client._IOcount);
	//InterlockedAnd((long *) &_client._IOcount, 0x7fffffff);

	IncrementIOCount(300);
	Lock();
	InterlockedExchange(&_client._IOFlag, FALSE);
	InterlockedExchange(&_client._isAlive, TRUE);
	_client._sock = INVALID_SOCKET;
	_client._sendPacketCnt = 0;
	_client._recvQueue.ClearBuffer();
	_client._sendQueue.Clear();
	Unlock();

	if (ConnectServer()) {
		OnEnterJoinServer();
	} else {
		return false;
	}

	DecrementIOCount(300);
	InterlockedExchange(&_isRunning, TRUE);
	return true;
}

bool CNetClient::Disconnect() {
	if (InterlockedOr(&_isRunning, FALSE) == FALSE) return false;
	bool ret = true;
	IncrementIOCount(33);
	InterlockedExchange(&_client._isAlive, FALSE);
	ret = CancelIoEx((HANDLE) _client._sock, nullptr);
	//closesocket( _client._sock);
	DecrementIOCount(33);

	//Quit();
	return ret;
}

bool CNetClient::SendPacket(Packet *pPacket) {
	//---------------------------
	 // 페킷 포인터를 센드큐에
	 //---------------------------
	pPacket->SetNetHeader();
	pPacket->AddRef();

	_client._sendQueue.enqueue(pPacket);
	//---------------------------
	// monitor
	//---------------------------
	InterlockedIncrement(&_sendPacketCalc);
	return SendPost();
}

void CNetClient::SetThreadNum(BYTE worker, BYTE active) {
	_workerThreadCount = worker;
	_maxRunThreadCount = active;
}

bool CNetClient::Start() {
	if (InterlockedOr(&_isRunning, FALSE) != FALSE)
		return false;
	Init();

	return true;
}

void CNetClient::Quit() {
	//_isRunning = false;
	PostQueuedCompletionStatus(_hIOCP, dfEXIT_CODE, dfEXIT_CODE, NULL);

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

bool CNetClient::ConnectServer() {
	SOCKADDR_IN	addr;
	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 200000;
	int test = 0;
	int err = 0;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_serverPort);
	InetPton(AF_INET, _serverIP, &addr.sin_addr);

	if (_client._sock == INVALID_SOCKET) {
		CreateSocket();
	}

	int connectRet = connect(_client._sock, (SOCKADDR *) &addr, sizeof(addr));

	if (connectRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK) {
			FD_ZERO(&_client._wset);
			FD_ZERO(&_client._errset);

			FD_SET(_client._sock, &_client._wset);
			FD_SET(_client._sock, &_client._errset);


			// 연결 실패시 한번더 대기
			int retval = select(0, nullptr, &_client._wset, &_client._errset, &tval);

			if (retval > 0) {
				if (FD_ISSET(_client._sock, &_client._wset)) {
					return RegisterIocp();
				} else if (FD_ISSET(_client._sock, &_client._errset)) {
					return false;
				}
			} else {
				closesocket(_client._sock);
			}

			return false;
		}

		if (err != WSAEISCONN) {
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Unusual Connect Error %d", err);
		}
	}


	return RegisterIocp();
}


BOOL CNetClient::DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr) {
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

void CNetClient::BeginThreads() {
	BYTE i = 0;

	_hThreads[i++] = (HANDLE) _beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);

	for (; i <= _workerThreadCount; ++i) {
		_hThreads[i] = (HANDLE) _beginthreadex(nullptr, 0, WorkerThread, this, 0, nullptr);
	}

	_NumThreads = i;

}

unsigned int __stdcall CNetClient::WorkerThread(LPVOID arg) {
	CNetClient *pClient = (CNetClient *) arg;
	while (true) {
		if (pClient->OnGQCS() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- Worker Thread IS Closed..\n");
	return 0;
}

unsigned int __stdcall CNetClient::MonitorThread(LPVOID arg) {
	CNetClient *pClient = (CNetClient *) arg;
	while (true) {
		if (pClient->NetMonitorProc() == false) {
			break;
		}
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"-- Monitor Thread IS Closed..\n");
	return 0;
}

bool CNetClient::OnGQCS() {
	DWORD transferredSize = 0;
	SESSION *completionKey = 0;
	WSAOVERLAPPED *pOverlapped = nullptr;

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
	}


	int log = 0;
	if (transferredSize > 0 && GQCSRet == TRUE) {
		//---------------------------
		// WSARecv가 완료됨
		//---------------------------
		if (pOverlapped == &_client._recvOverlapped) {
			RecvProc(transferredSize);
			log = 10;
		}
		//---------------------------
		// WSASend가 완료됨
		//---------------------------
		if (pOverlapped == &_client._sendOverlapped) {
			SendProc(transferredSize);
			log = 30;
		}
	}
	//---------------------------
	// 	   IOCount --
	//---------------------------
	DecrementIOCount(3000 + log);

	return true;
}

bool CNetClient::SendProc(DWORD transferredSize) {
	//OnSend(transferredSize);
	//---------------------------
	// 완료통지 온 패킷 지우기
	//---------------------------
	Packet *pPacket;
	int sendedPacketCnt = _client._sendPacketCnt;

	for (int i = 0; i < sendedPacketCnt; ++i) {
		_client._sendQueue.dequeue(pPacket);
		pPacket->SubRef(4);
		pPacket = nullptr;
	}

	_client._sendPacketCnt = 0;

	//---------------------------
	// 	   Send가 끝났다
	//---------------------------
	//InterlockedExchange8((CHAR *) &_client._IOFlag, FALSE);
	_client._IOFlag = false;


	//---------------------------
	// SendQ에 보낼것이 남아있으면 Send
	//---------------------------
	SendPost();
	return true;
}

bool CNetClient::RecvProc(DWORD transferredSize) {
	//---------------------------
	// recv신호가 옴
	//---------------------------

	//---------------------------
	// 온만큼 링버퍼 Rear빼주기
	//---------------------------
	int movRet = _client._recvQueue.MoveRear(transferredSize);
	if (transferredSize != movRet) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L" RecvProc() :: transferredSize[%d] != movRet[%d]", transferredSize, movRet);
		CRASH();
	}
	DWORD msgByte = 0; // 메시지 처리된 바이트 수

	// return val
	int headerPeekRet;
	int headerMoveRet;
	int payloadDeqRet;
	PACKET_NET_HEADER header;

	//---------------------------
	// 	   반복문 돌며 패킷 처리
	//---------------------------
	while (msgByte < transferredSize) {

		//---------------------------
		// 모니터링
		//---------------------------
		InterlockedIncrement(&_recvPacketCalc);

		if (_client._recvQueue.GetUseSize() <= PACKET_NET_HEADER_SIZE) {
			//---------------------------
			// 헤더보다 적음
			//---------------------------
			break;
		}

		//---------------------------
		// 헤더는 읽을 수 있음
		//---------------------------
		headerPeekRet = _client._recvQueue.Peek((unsigned char *) &header, PACKET_NET_HEADER_SIZE);
		if (headerPeekRet != PACKET_NET_HEADER_SIZE) {
			// 무결성 검사
			CRASH();
		}

		if (_client._recvQueue.GetUseSize() < (int) (PACKET_NET_HEADER_SIZE + header.len)) {
			break;
		}
		if (header.code != Packet::PACKET_CODE) {
			CRASH();
		}

		//---------------------------
		// 패킷 풀에서 하나 꺼내기
		//---------------------------
		Packet *pPacket = Packet::AllocAddRef();
		if (pPacket == NULL) {
			CRASH();
		}
		pPacket->Clear();

		//---------------------------
		// 페이로드 크기를 알았으니 헤더는 제 역할을 다함
		// 페이로드만 페킷에 넣어주기
		//---------------------------
		payloadDeqRet = _client._recvQueue.Dequeue(pPacket->GetBufferPtr(), (int) header.len + PACKET_NET_HEADER_SIZE);
		if (payloadDeqRet != header.len + PACKET_NET_HEADER_SIZE) {
			CRASH();
		}

		int MoveWritePosRet = pPacket->MoveWritePos(header.len);
		if (MoveWritePosRet != header.len)
			CRASH();
		if (pPacket->Decode() == false) {
			CRASH();
			return false;
		}
		OnRecv(pPacket);

		msgByte += payloadDeqRet;

		//---------------------------
		// 참조카운트 하나감소
		//---------------------------
		pPacket->SubRef(5000);
	}

	//---------------------------
	// RecvPost
	//---------------------------
	return RecvPost();
}

bool CNetClient::NetMonitorProc() {
	//---------------------------
	// 1초마다 TPS계산
	//---------------------------
	Sleep(1000);

	CalcTPS();

	return true;
}

bool CNetClient::RegisterIocp() {
	if (_client._sock == INVALID_SOCKET) return false;

	HANDLE hResult = CreateIoCompletionPort((HANDLE) _client._sock, _hIOCP, 0, 0);

	if (hResult == NULL) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]", WSAGetLastError());
		closesocket(_client._sock);

		return false;
	}

	IncrementIOCount(1000);
	RecvPost(true);
	DecrementIOCount(1000);
	return true;
}

bool CNetClient::SendPost() {
	if (InterlockedOr((long *) &_client._isAlive, FALSE) == FALSE) {
		return false;
	}

	//---------------------------
	// 	   Send중인지 확인
	//---------------------------
	BOOL isSend = InterlockedExchange8((CHAR *) &_client._IOFlag, TRUE);
	if (isSend == TRUE) {
		return FALSE;
	}
	//---------------------------
	// 	   Send가능한데 보낼게 없는지 확인
	//---------------------------
	if (_client._sendQueue.GetSize() == 0) {
		if (InterlockedExchange8((CHAR *) &_client._IOFlag, FALSE) == FALSE) {
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendPost() _isSend Exchange false to false");
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

	SetWSABuffer(bufferSet, FALSE);

	//---------------------------
	// IOCount ++
	//---------------------------
	IncrementIOCount(4000);

	//---------------------------
	// 오버랩 초기화
	//---------------------------
	memset(&_client._sendOverlapped, 0, sizeof(_client._sendOverlapped));
	//---------------------------
	// WSASend()
	//---------------------------
	int sendRet = WSASend(_client._sock, bufferSet, _client._sendPacketCnt, &byteSends, 0, &_client._sendOverlapped, nullptr);
	if (sendRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10054 && err != 10053) {
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"//// WSASend() ERROR [%d]", err);
				//CRASH();
			}
			//---------------------------
			//	Error Fail WSASend
			//  IOCount --
			//---------------------------
			DecrementIOCount(4000);
			return false;
		}
		//CLogger::_Log(dfLOG_LEVEL_ERROR, L"//// WSASend WSA_IO_PENDING [%d]", err);
	}

	return true;
}

bool CNetClient::RecvPost(bool isAccept) {
	if (InterlockedOr((long *) &_client._isAlive, FALSE) == FALSE) {
		return false;
	}

	//---------------------------
	// IOCount ++
	//---------------------------
	//if(isAccept == false)
	IncrementIOCount(5000);

	//---------------------------
	// WSABUF 셋팅
	// 0 : 현재 RecvQ.rear, 1 : RecvQ 의 시작지점
	//---------------------------
	WSABUF bufferSet[2];
	DWORD flag = 0;
	DWORD byteRecvs;

	SetWSABuffer(bufferSet, TRUE);

	//---------------------------
	// 오버랩 초기화
	//---------------------------

	DWORD IOcount = _client._IOcount;
	DWORD sock = _client._sock;

	memset(&_client._recvOverlapped, 0, sizeof(_client._recvOverlapped));
	//---------------------------
	//WSARecv()
	//---------------------------
	int recvRet = WSARecv(_client._sock, bufferSet, 2, &byteRecvs, &flag, &_client._recvOverlapped, nullptr);
	if (recvRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10054 && err != 10053) {
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"////  :: WSARecv ERROR [%d]\n", err);
				//CRASH();

			}
			//---------------------------
			//	Error : Fail WSARecv
			//  IOCount --
			//---------------------------
			DecrementIOCount(5000);
			//return false;
		}
	}
	return true;
}

bool CNetClient::SetWSABuffer(WSABUF *BufSets, bool isRecv) {
	if (isRecv) {
		//---------------------------
		// recv버퍼에 넣기
		//---------------------------

		char *pBuf = _client._recvQueue.GetBufferPtr();
		char *pRear = _client._recvQueue.GetRearBufferPtr();
		int  enSize = _client._recvQueue.DirectEnqueueSize();
		int  frSize = _client._recvQueue.GetFreeSize();
		BufSets[0].buf = pRear;
		BufSets[0].len = enSize;
		BufSets[1].buf = pBuf;
		BufSets[1].len = frSize - enSize;
		//---------------------------
		// 무결성 검사
		//---------------------------
		if (BufSets[0].len + BufSets[1].len != frSize) {
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"SetWSABuffer() :: BufSets[0].len + BufSets[1].len != FreeSize");
			CRASH();
		}
	} else {
		//---------------------------
		// SendQ의 패킷을 WSABUF에 등록
		//---------------------------
		Packet *pPacketBufs[100];
		//---------------------------
		// 패킷을 얼마나 보낼지
		//---------------------------
		DWORD snapSize = _client._sendQueue.Peek(pPacketBufs, 100);


		//---------------------------
		// wsabuffer에 등록
		//---------------------------
		for (int i = 0; i < snapSize; ++i) {

			BufSets[i].buf = (CHAR *)pPacketBufs[i]->GetSendPtr();
			BufSets[i].len = pPacketBufs[i]->GetSendSize();
		}

		//---------------------------
		// 처리한만큼 개수 저장
		//---------------------------
		_client._sendPacketCnt = snapSize;
	}

	return true;
}
long incArr[10000];
long incIdx = 0;
long decArr[10000];
long decIdx = 0;

bool CNetClient::IncrementIOCount(int logic) {
	int idx = InterlockedIncrement(&incIdx);
	if (idx >= 10000) idx = incIdx = 0;
	incArr[idx] = logic;

	if ((InterlockedIncrement(&_client._IOcount) & 0x80000000) != 0) {
		if (InterlockedDecrement(&_client._IOcount) == 0)
			this->ReleaseSessionProc(logic);
		return false;
	}
	return true;
}

bool CNetClient::DecrementIOCount(int logic) {
	int idx = InterlockedIncrement(&decIdx);
	if (idx >= 10000) idx = decIdx = 0;
	decArr[idx] = logic;


	DWORD ret = InterlockedDecrement(&_client._IOcount);
	if (ret < 0)
		CRASH();

	if (ret == 0)
		ReleaseSessionProc(logic + 5000);

	return true;
}

bool CNetClient::ReleaseSessionProc(int logic) {
	if (_client._sock == INVALID_SOCKET) {
		return false;
	}
	if (InterlockedCompareExchange(&_client._IOcount, 0x80000000, 0) != 0) {
		return false;
	}

	Lock();
	OnLeaveServer();
	//closesocket(_client._sock);
	_client._sock = INVALID_SOCKET;
	InterlockedExchange(&_client._IOFlag, FALSE);
	Packet *pPacket;
	while (_client._sendQueue.dequeue(pPacket)) {
		pPacket->SubRef(66);
	}
	_client._recvQueue.ClearBuffer();

	if(InterlockedExchange(&_isRunning, FALSE) == FALSE) CRASH();
	Unlock();
	return false;
}

void CNetClient::Init() {
	SetStartUp();

	_hThreads = new HANDLE[(long long) _workerThreadCount + 1];

	CreateIOCP();

	BeginThreads();

}

void CNetClient::CreateIOCP() {
	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, _maxRunThreadCount);
	if (_hIOCP == NULL) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]", WSAGetLastError());

	}
}

bool CNetClient::CreateSocket() {
	_client._sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_client._sock == INVALID_SOCKET) {
		OnError(111, L"Create Socke Error");
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Create Socke Error[CODE : %d]\n", WSAGetLastError());
		return false;
	}
	if (SetStartUp() == false)
		return false;
	if (SetTimeWaitZero() == false)
		return false;

	if (_isNagle)
		SetNagle(_isNagle);
	return true;
}

bool CNetClient::SetStartUp() {
	WSADATA			wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"WSAStartup [Error: %d]", WSAGetLastError());
		return false;
	}

	return true;
}

bool CNetClient::SetTimeWaitZero() {
	LINGER optval;

	optval.l_onoff = 1;
	optval.l_linger = 0;

	int timeOutnRet = setsockopt(_client._sock, SOL_SOCKET, SO_LINGER, (char *) &optval, sizeof(optval));
	if (timeOutnRet == SOCKET_ERROR) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Client Socket Linger [Error: %d]", WSAGetLastError());
		closesocket(_client._sock);
		return false;
	}

	return true;
}

bool CNetClient::SetNonBlockSocket() {
	u_long on = 1;

	int retval = ioctlsocket(_client._sock, FIONBIO, &on);

	if (retval == SOCKET_ERROR) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Set NonBlock Socket [Error: %d]", WSAGetLastError());
		closesocket(_client._sock);
		return false;
	}

	return true;
}

bool CNetClient::SetNagle(bool sw) {
	BOOL optval = sw;

	if (setsockopt(_client._sock, IPPROTO_TCP, TCP_NODELAY, (char *) &optval, sizeof(optval)) == SOCKET_ERROR) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Nagle [Error: %d]", WSAGetLastError());
		closesocket(_client._sock);

		return true;
	}
	return false;
}

void CNetClient::CalcTPS() {
	_recvPacketPerSec = InterlockedExchange(&_recvPacketCalc, 0);
	_sendPacketPerSec = InterlockedExchange(&_sendPacketCalc, 0);
}

CNetClient::MoniteringInfo CNetClient::GetMoniteringInfo() {
	MoniteringInfo info;
	info._workerThreadCount = _maxRunThreadCount;
	info._runningThreadCount = _workerThreadCount;
	info._recvPacketPerSec = _recvPacketPerSec;
	info._sendPacketPerSec = _sendPacketPerSec;
	info._totalPacket = _totalPacket;
	info._totalProecessedBytes = _totalProcessedBytes;
	info._queueSize = 0;
	info._queueSize = _client._sendQueue.GetSize();
	return info;
}

void CNetClient::ResetMonitor() {
	_totalPacket = 0;
	_recvPacketCalc = 0;
	_recvPacketPerSec = 0;
	_sendPacketCalc = 0;
	_sendPacketPerSec = 0;
	_totalProcessedBytes = 0;

}
