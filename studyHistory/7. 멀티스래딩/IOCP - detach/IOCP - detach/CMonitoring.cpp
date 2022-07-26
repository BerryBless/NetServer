#include "pch.h"
#include "CMonitoring.h"
#include "CSession.h"

CMonitoring::CMonitoring() {
	_sendCount = 0;
	_recvCount = 0;
	_sendMsgCount = 0;
	_recvMsgCount = 0;
	_connectSessionCount = 0;
	InitializeSRWLock(&_threadCountLock);
}

void CMonitoring::SendCheck() {
	InterlockedIncrement(&_sendCount);
}

void CMonitoring::RecvCheck() {
	InterlockedIncrement(&_recvCount);
}

void CMonitoring::SendMsgCheck() {
	InterlockedIncrement(&_sendMsgCount);
}

void CMonitoring::RecvMsgCheck() {
	InterlockedIncrement(&_recvMsgCount);
}

void CMonitoring::IncrementSessionCount() {
	InterlockedIncrement(&_connectSessionCount);
}

void CMonitoring::DecrementSessionCount() {
	InterlockedDecrement(&_connectSessionCount);
}

void CMonitoring::SendMsgAdd(DWORD count) {
	InterlockedAdd((LONG *) &_sendMsgCount, count);
}

void CMonitoring::RecvMsgAdd(DWORD count) {
	InterlockedAdd((LONG *) &_recvMsgCount, count);
}

void CMonitoring::ThreadRegistr(unsigned long tID) {
	AcquireSRWLockExclusive(&_threadCountLock);
	_threadCountMap.insert(std::make_pair(tID, 0));
	ReleaseSRWLockExclusive(&_threadCountLock);
}

void CMonitoring::ThreadCheck(unsigned long tID) {
	AcquireSRWLockExclusive(&_threadCountLock);
	do {
		auto citer = _threadCountMap.find(tID);
		if (citer == _threadCountMap.end()) {
			_LOG(dfLOG_LEVEL_ERROR, L"이상한 ID [%d]", tID);
			break;
		}
		citer->second++;
	} while (0);
	ReleaseSRWLockExclusive(&_threadCountLock);
}


void CMonitoring::PrintMonitoring() {
	int sendCount = InterlockedExchange(&_sendCount, 0);
	int recvCount = InterlockedExchange(&_recvCount, 0);
	int sendMsgCount = InterlockedExchange(&_sendMsgCount, 0);
	int recvMsgCount = InterlockedExchange(&_recvMsgCount, 0);
	_LOG(dfLOG_LEVEL_ERROR, 
L"\n====================================\n\
send packet TPS[%d]\n\
recv packet TPS[%d]\n\
WSASend TPS[%d]\n\
WSARecv TPS[%d]\n\
connectCount[%d]", 
sendMsgCount, recvMsgCount, sendCount, recvCount, _connectSessionCount);
	_LOG(dfLOG_LEVEL_ERROR, L"------------------------------------");
	AcquireSRWLockExclusive(&_threadCountLock);
	int cnt = 0;
	for (auto iter = _threadCountMap.begin(); iter != _threadCountMap.end(); ++iter) {
		_LOG_NONTIMESTAMP(dfLOG_LEVEL_ERROR, L"%2d Thread [%5d] :: [%d]", ++cnt, iter->first, iter->second);
		iter->second = 0;
	}
	ReleaseSRWLockExclusive(&_threadCountLock);
	_LOG_NONTIMESTAMP(dfLOG_LEVEL_ERROR, L"====================================");
}

CMonitoring g_monitoring; // 모니터링
