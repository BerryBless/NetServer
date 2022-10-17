#pragma once
class CMonitoring {
private:
	std::map<unsigned long, int> _threadCountMap;	// <ID, count> 지금까지 스레드 ID마다 깨어난 수
	SRWLOCK _threadCountLock;	// _threadCountMap 전용
	
	DWORD _sendCount;								// PostSend TPS
	DWORD _recvCount;								// PostRecv TPS
	DWORD _sendMsgCount;
	DWORD _recvMsgCount;
	DWORD _connectSessionCount;

public:
	CMonitoring();
	void SendCheck();
	void RecvCheck();
	void SendMsgCheck();
	void RecvMsgCheck();
	void IncrementSessionCount();
	void DecrementSessionCount();
	void SendMsgAdd(DWORD count);
	void RecvMsgAdd(DWORD count);
	void ThreadRegistr(unsigned long tID);	// _threadCountMap 에 등록
	void ThreadCheck(unsigned long tID);	// _threadCountMap 에 체크

	void PrintMonitoring();
};

extern CMonitoring g_monitoring; // 모니터링
