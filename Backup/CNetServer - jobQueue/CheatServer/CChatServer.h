#pragma once
#include "Player.h"
#include "CNetServer.h"
#include "JobMessage.h"


#define INVALID_PLAYER_SECTOR	51
#define SECTOR_X_SIZE			50
#define SECTOR_Y_SIZE			50
#define ID_MAX_LEN				20
#define ID_MAX_SIZE				40
#define NICK_NAME_MAX_LEN		20
#define NICK_NAME_MAX_SIZE		40
#define TOKEN_KEY_SIZE			64
#define MASSAGE_MAX_LEN			512
#define MASSAGE_MAX_SIZE		1024


class CChatServer : public CNetServer {

private:
	struct SECTOR {
		::unordered_set<Player *> _playerSet;
		SRWLOCK _lock;
		SECTOR() {
			_playerSet.clear();
			InitializeSRWLock(&_lock);
		}
	};

public:
	CChatServer();
	~CChatServer();

	void BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	void CloseServer();
	bool isRunning() {
		return _isRunning;
	}

	void CommandWait();
private:
	// Thtread

	static unsigned int __stdcall UpdateThread(LPVOID arg);
	static unsigned int __stdcall MonitoringThread(LPVOID arg);

	bool UpdateProc();
	bool MonitoringProc();

	void BeginThread();

private:
	// virtual
	virtual bool OnConnectionRequest(WCHAR *IPStr, u_long IP, u_short Port); //< accept 직후
	virtual void OnClientJoin(SESSION_ID SessionID); //< Accept 후 접속처리 완료 후 호출.
	virtual void OnClientLeave(SESSION_ID SessionID); //< Release 후 호출
	virtual void OnRecv(SESSION_ID SessionID, CPacket *packet); //< 패킷 수신 완료 후
	virtual void OnError(int errorcode, const WCHAR *log); // 에러 발생시 유저한테 알려줄곳
	virtual void OnTimeout(SESSION_ID SessionID);

	void PacketProc(CPacket *pPacket, SESSION_ID sessionID, WORD type);

	void PacketProcRequestLogin(CPacket *pPacket, SESSION_ID sessionID);
	void PacketProcMoveSector(CPacket *pPacket, SESSION_ID sessionID);
	void PacketProcChatRequire(CPacket *pPacket, SESSION_ID sessionID);
	//void PacketProcHeartBeat(CPacket *pPacket, SESSION_ID sessionID);

	void MakePacketResponseLogin(CPacket *pPacket, ACCOUNT_NO account_no, BYTE status);
	void MakePacketResponseSectorMove(CPacket *pPacket, ACCOUNT_NO account_no, WORD sectorX, WORD sectorY);
	void MakePacketResponseMessage(CPacket *pPacket, ACCOUNT_NO account_no, const WCHAR *ID, const WCHAR *nickName, WORD msgLen, const WCHAR *message);

private:
	void BroadcastSector(CPacket *pPacket, WORD sectorX, WORD sectorY, Player *ex );
	void BroadcastSectorAround(CPacket *pPacket, WORD sectorX, WORD sectorY, Player *ex );


private:
	void InsertPlayer(ULONGLONG sessionID, Player *pPlayer);
	void RemovePlayer(ULONGLONG sessionID);
	Player *FindPlayer(ULONGLONG sessionID);

	inline void PlayerMapLock() { AcquireSRWLockExclusive(&this->_playerMapLock); }
	inline void PlayerMapUnlock() { ReleaseSRWLockExclusive(&this->_playerMapLock); }
	inline void SectorLock(WORD x, WORD y) { AcquireSRWLockExclusive(&this->_sector[y][x]._lock); }
	inline void SectorUnlock(WORD x, WORD y) { ReleaseSRWLockExclusive(&this->_sector[y][x]._lock); }

private:
	void PrintMonitor(FILE *fp);
	void PrintFileMonitor();

private:
	DWORD									_isRunning;

	HANDLE									_hThread[2];

	Queue <JobMessage*>						_jobQueue;
	ObjectPool_TLS <JobMessage>				_jobMsgPool;


	SECTOR **_sector;
	unordered_map<ULONGLONG, Player *>		_playerMap;
	SRWLOCK									_playerMapLock;
	ObjectPool<Player>						_playerPool;

private:
	// server start timestemp
	tm _timeFormet;
	time_t _startTime;


	HardWareMoniter							_hardMoniter;
	ProcessMoniter							_procMonitor;

	ULONGLONG								_SectorMoveCalc;
	ULONGLONG								_SectorMoveTPS;
	ULONGLONG								_ChatRecvCalc;
	ULONGLONG								_ChatRecvTPS;
	ULONGLONG								_ChatSendCalc;
	ULONGLONG								_ChatSendTPS;
	ULONGLONG								_LoginCalc;
	ULONGLONG								_LoginTPS;
	ULONGLONG								_TotalUpdateCount;

};

