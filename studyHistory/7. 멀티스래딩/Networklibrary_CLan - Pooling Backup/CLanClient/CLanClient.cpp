#include "CLanClient.h"
#include "Logger.h"


// cmd code
#define dfEXIT_CODE 0xFFFFFFFF // GQCS()에서 이게 오면 종료


CLanClient::CLanClient() {
	ZeroMemory(&_monitor, sizeof(MONITOR));
}

CLanClient::~CLanClient() {
	closesocket(_sock);
}

bool CLanClient::Connect(ULONG serverIP, USHORT port, BYTE workerThreadCount, bool isNagle) {
	return false;
}

bool CLanClient::Disconnect() {
	return false;
}

bool CLanClient::SendPacket(CPacket *pPacket) {
	return false;
}

void CLanClient::Quit() {
	PostQueuedCompletionStatus(_hIOCP, dfEXIT_CODE, dfEXIT_CODE, NULL); // 워커 하나 종료하라고 알려줌
	_isRunning = false; // 종료!
}

bool CLanClient::CreateSocket() {
	return false;
}

void CLanClient::BeginThreads() {
}

unsigned int __stdcall CLanClient::WorkerThread(LPVOID arg) {
	CLanClient *pClient = (CLanClient *) arg;
	while (true) {
		// static 이니까.. 오브젝트 내에서 실행하게 만들어주기
		if (pClient->OnGQCS() == false) {
			break;
		}
	}
	Sleep(1);
	_LOG(dfLOG_LEVEL_ERROR, L"-- Monitor Thread IS Closed..\n");
	return 0;
}

unsigned int __stdcall CLanClient::MonitorThread(LPVOID arg) {
	CLanClient *pClient = (CLanClient *) arg;
	while (true) {
		// static 이니까.. 오브젝트 내에서 실행하게 만들어주기
		if (pClient->NetMonitorProc() == false) {
			break;
		}
	}
	Sleep(1);
	_LOG(dfLOG_LEVEL_ERROR, L"-- Monitor Thread IS Closed..\n");
	return 0;
}

bool CLanClient::OnGQCS() {
	DWORD transferredSize = 0;
	PULONG_PTR completionKey = 0;
	WSAOVERLAPPED *pOverlapped = nullptr;
	SESSION *pSession = nullptr;

	BOOL GQCSRet = GetQueuedCompletionStatus(_hIOCP, &transferredSize, completionKey, &pOverlapped, INFINITE);

	if (pOverlapped == NULL) {
		// 스레드 종료 
		if (transferredSize == dfEXIT_CODE && (long long) completionKey == dfEXIT_CODE) {
			PostQueuedCompletionStatus(_hIOCP, dfEXIT_CODE, dfEXIT_CODE, NULL); // 워커 하나 종료하라고 알려줌
			return false;
		}
		_LOG(dfLOG_LEVEL_ERROR, L"overlapped is NULL ERROR CODE [%d]", WSAGetLastError());
		OnError(dfLOGIC_FROM_WORKER + 1, L"IOCP ERROR :: overlapped is NULL");
		return true;
	}

	if (transferredSize > 0 && GQCSRet == TRUE) {
		if (pOverlapped == &_session._recvOverlapped) {
			RecvProc(transferredSize);
		}
		if (pOverlapped == &_session._sendOverlapped) {
			SendProc(transferredSize);
		}
	}

	return true;
}

bool CLanClient::SendProc(DWORD transferredSize) {
	_session._sendQueue.MoveFront(transferredSize);

	//InterlockedExchange8((CHAR *) &_session._isSend, FALSE);
	_session._isSend = false;

	SendPost(dfLOGIC_FROM_CPMPLETE_SEND + 100);
	return true;
}

bool CLanClient::RecvProc(DWORD transferredSize) {
	_session._recvQueue.MoveRear(transferredSize);

	DWORD msgByte = 0; // 메시지 처리
	CPacket packet;	 // 나중에 pool로 바꿀수 있게 코딩
	CPacket *pPacket;  // TODO 풀로 받아오기

	// return val
	int headerPeekRet;
	int headerMoveRet;
	int payloadDeqRet;
	USHORT header; // 페이로드 사이즈 2바이트

	while (msgByte < transferredSize) {
		if (_session._isAlive == false) {
			return false;
		}
		pPacket = &packet;// TODO pool로 받아오기 
		pPacket->Clear();
		// TODO 모니터링 체크
		InterlockedIncrement(&_monitor._recvPacketCalc);

		if (_session._recvQueue.GetUseSize() <= sizeof(USHORT)) {
			// 헤더보다 적음
			break;
		}

		// 헤더는 읽을 수 있음
		headerPeekRet = _session._recvQueue.Peek((char *) &header, sizeof(USHORT));
		if (headerPeekRet != sizeof(USHORT)) {
			// 무결성 검사
			CRASH();
		}

		if (_session._recvQueue.GetUseSize() < (int) (sizeof(USHORT) + header)) {
			// 패킷 전체크기보다 적음
			break;
		}

		// 온전한 패킷이 온걸 확인
		// 이미 알고있는 정보는 넘어가기
		headerMoveRet = _session._recvQueue.MoveFront(sizeof(USHORT));
		if (headerMoveRet != sizeof(USHORT)) {
			// 무결성 검사
			CRASH();
		}

		// 페이로드 크기를 알았으니 헤더는 제 역할을 다함
		// 페이로드만 페킷에 넣어주기
		payloadDeqRet = _session._recvQueue.Dequeue(pPacket->GetWritePtr(), (int) header);
		if (payloadDeqRet != header) {
			// 무결성 검사
			CRASH();
		}
		pPacket->MoveWritePos(payloadDeqRet);
		OnRecv(pPacket);

		msgByte += (payloadDeqRet + sizeof(USHORT));

	}

	return RecvPost( dfLOGIC_FROM_CPMPLETE_RECV + 200);
}

bool CLanClient::NetMonitorProc() {
	// 1초마다 0으로 초기화
	Sleep(1000);
	_monitor._wsasendTPS = _monitor._wsasendCalc;
	_monitor._wsasendCalc = 0;

	_monitor._wsarecvTPS = _monitor._wsarecvCalc;
	_monitor._wsarecvCalc = 0;

	_monitor._sendPacketTPS = _monitor._sendPacketCalc;
	_monitor._sendPacketCalc = 0;

	_monitor._recvPacketTPS = _monitor._recvPacketCalc;
	_monitor._recvPacketCalc = 0;

	return _isRunning;
}

bool CLanClient::SendPost(int logic) {
	if (_session._isAlive == false) {
		return false;
	}

	BOOL isSend = InterlockedExchange8((CHAR *) &_session._isSend, TRUE);
	if (isSend == TRUE) {
		return FALSE;
	}
	if (_session._sendQueue.GetUseSize() == 0) {
		if (InterlockedExchange8((CHAR *) &_session._isSend, FALSE) == FALSE) {
			_LOG(dfLOG_LEVEL_ERROR, L"SendPost(%d) _isSend Exchange false to false", logic);
			CRASH();
		}
		return FALSE;
	}

	WSABUF bufferSet[2]; // 0 : 현재, 1 : 링버퍼 처음
	DWORD byteSends;

	SetWSABuffer(bufferSet,  FALSE, logic + 1);

	// 오버랩 초기화
	memset(&_session._sendOverlapped, 0, sizeof(_session._sendOverlapped));
	// WSASend()
	int sendRet = WSASend(_sock, bufferSet, 2, &byteSends, 0, &_session._sendOverlapped, nullptr);
	if (sendRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10054 && err != 10053)
				_LOG(dfLOG_LEVEL_ERROR, L"//// WSASend(%d) ERROR [%d]", logic, err);
		}
		// 비동기로 잘들어감
		//_LOG(dfLOG_LEVEL_ERROR, L"//// WSASend WSA_IO_PENDING [%d]", err);
	}
	InterlockedIncrement(&_monitor._wsasendCalc);

	return true;
}

bool CLanClient::RecvPost(int logic, bool isAccept) {
	if (_session._isAlive == false) {
		return false;
	}
	WSABUF bufferSet[2]; // 0 : 현재, 1 : 링버퍼 처음
	DWORD flag = 0;
	DWORD byteRecvs;

	SetWSABuffer(bufferSet,  TRUE, logic + 1);

	if (isAccept)
		bufferSet[1].len = 0;

	// 오버랩 초기화
	memset(&_session._recvOverlapped, 0, sizeof(_session._recvOverlapped));
	//WSARecv()
	int recvRet = WSARecv(_sock, bufferSet, 2, &byteRecvs, &flag, &_session._recvOverlapped, nullptr);
	if (recvRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10054 && err != 10053)
				_LOG(dfLOG_LEVEL_ERROR, L"//// WSARecv ERROR [%d]", err);
			return false;
		}
	}
	// 동기로 완료
	InterlockedIncrement(&_monitor._wsarecvCalc);
	return true;
}

bool CLanClient::SetWSABuffer(WSABUF *pBufSet, bool isRecv, int logic) {
	if (isRecv) {
		char *pBuf = _session._recvQueue.GetBufferPtr();
		char *pRear = _session._recvQueue.GetRearBufferPtr();
		int  enSize = _session._recvQueue.DirectEnqueueSize();
		int  frSize = _session._recvQueue.GetFreeSize();
		// recv버퍼에 넣기
		pBufSet[0].buf = pRear;
		pBufSet[0].len = enSize;
		pBufSet[1].buf = pBuf;
		pBufSet[1].len = frSize - enSize;
		if (pBufSet[0].len + pBufSet[1].len > frSize) {
			_LOG(dfLOG_LEVEL_ERROR, L"SetWSABuffer(%d) :: pBufSet[0].len + pBufSet[1].len != FreeSize", logic);
			CRASH();
		}
	} else {
		// send버퍼에서 빼가기
		char *pBuf = _session._recvQueue.GetBufferPtr();
		char *pFront = _session._sendQueue.GetFrontBufferPtr();

		int  deSize = _session._sendQueue.DirectDequeueSize();
		int usSize = _session._sendQueue.GetUseSize();

		pBufSet[0].buf = pFront;
		pBufSet[0].len = deSize;
		pBufSet[1].buf = pBuf;
		pBufSet[1].len = usSize - deSize;
		if (pBufSet[0].len + pBufSet[1].len > usSize) {
			_LOG(dfLOG_LEVEL_ERROR, L"SetWSABuffer(%d) :: pBufSet[0].len + pBufSet[1].len != UseSize", logic);
			CRASH();
		}
	}

	return true;
}
