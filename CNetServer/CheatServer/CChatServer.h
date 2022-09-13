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

//#define UPDATE_THREAD

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

//	void BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	void BeginServer(const WCHAR *szConfigFile);
	void CloseServer();
	bool isRunning() {
		return _isRunning;
	}

	void CommandWait();
private:
	// Thtread

	bool UpdateProc();
	bool MonitoringProc();

	void BeginThread();

private:
	// virtual
	virtual bool OnConnectionRequest(WCHAR *IPStr, DWORD IP, USHORT Port); //< accept 직후
	virtual void OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, ULONGLONG sessionID); //< Accept 후 접속처리 완료 후 호출.
	virtual void OnClientLeave(SESSION_ID sessionID); //< Release 후 호출
	virtual void OnRecv(SESSION_ID sessionID, Packet *packet); //< 패킷 수신 완료 후
	virtual void OnSend(SESSION_ID sessionID); //< 패킷 수신 완료 후
	virtual void OnError(int errorcode, const WCHAR *log); // 에러 발생시 유저한테 알려줄곳
	virtual void OnTimeout(SESSION_ID sessionID);

	void PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type);

	void PacketProcRequestLogin(Packet *pPacket, SESSION_ID sessionID);
	void PacketProcMoveSector(Packet *pPacket, SESSION_ID sessionID);
	void PacketProcChatRequire(Packet *pPacket, SESSION_ID sessionID);
	void PacketProcHeartBeat(Packet *pPacket, SESSION_ID sessionID);

	void MakePacketResponseLogin(Packet *pPacket, ACCOUNT_NO account_no, BYTE status);
	void MakePacketResponseSectorMove(Packet *pPacket, ACCOUNT_NO account_no, WORD sectorX, WORD sectorY);
	void MakePacketResponseMessage(Packet *pPacket, ACCOUNT_NO account_no, const WCHAR *ID, const WCHAR *nickName, WORD msgLen, const WCHAR *message);

private:
	void BroadcastSector(Packet *pPacket, WORD sectorX, WORD sectorY, Player *exPlayer );
	void BroadcastSectorAround(Packet *pPacket, WORD sectorX, WORD sectorY, Player *exPlayer);

private:
	// Player Management
	void InsertPlayer(ULONGLONG sessionID, Player *pPlayer);
	void RemovePlayer(ULONGLONG sessionID);
	Player *FindPlayer(ULONGLONG sessionID);

private:
	// LOCK
	inline void PlayerMapLock() { AcquireSRWLockExclusive(&this->_playerMapLock); }
	inline void PlayerMapUnlock() { ReleaseSRWLockExclusive(&this->_playerMapLock); }
	inline void SectorLock(WORD x, WORD y) { AcquireSRWLockExclusive(&this->_sector[y][x]._lock); }
	inline void SectorUnlock(WORD x, WORD y) { ReleaseSRWLockExclusive(&this->_sector[y][x]._lock); }
	inline void SectorSLock(WORD x, WORD y) { AcquireSRWLockShared(&this->_sector[y][x]._lock); }
	inline void SectorSUnlock(WORD x, WORD y) { ReleaseSRWLockShared(&this->_sector[y][x]._lock); }
	inline BOOLEAN TrySectorLock(WORD x, WORD y) { return TryAcquireSRWLockExclusive(&this->_sector[y][x]._lock); }

private:
	// Monitoring
	void PrintMonitor(FILE *fp);
	void PrintFileMonitor();

private:
	DWORD									_isRunning;

	//HANDLE									_hThread[2];
	CThread									_moinitorThrad =  CThread(L"Chat Server Monitorting Thread");;

#ifdef UPDATE_THREAD
	CThread									_updateThread = CThread(L"Chat Server Update Thread");
	Queue <JobMessage*>						_jobQueue;
	HANDLE									_DequeueEvent;
	ObjectPool_TLS <JobMessage>				_jobMsgPool;
#endif // UPDATE_THREAD


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

	LONG								_SectorMoveCalc;
	LONG								_SectorMoveTPS;
	LONG								_ChatRecvCalc;
	LONG								_ChatRecvTPS;
	LONG								_ChatSendCalc;
	LONG								_ChatSendTPS;
	LONG								_LoginCalc;
	LONG								_LoginTPS;
	LONG								_LeaveCalc;
	LONG								_LeaveTPS;
	LONG								_UpdateCalc;
	LONG								_UpdateTPS;

	LONGLONG							_SectorAroundCount;
	LONGLONG							_totalSectorAroundSend;
};

