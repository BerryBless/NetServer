#pragma once
#include "CServer.h"
#include "DBConnectionPool.h"
#include "CommonProtocol.h"
#include "SS_MoniteringProtocol.h"
#include <unordered_map>
#include "CMonitorToolServer.h"
#include "CParser.h"


// 내부에서 데이터를 받을 서버


class CMonitorToolServer;

class CMonitoringServer : public CServer {
	struct ServerConnect {
		SESSION_ID _id = 0;
		int _serverNo = 0;
		bool _isLogin = false;
	};
public:
	CMonitoringServer();
	~CMonitoringServer();

	void BeginServer(u_long IP, u_short monitorSPort, u_short toolSPort, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	void BeginServer(const WCHAR *szConfigFile);
	void CloseServer();
	bool isRunning() {
		return _isRunning;
	}
	void CommandWait();




private:
	virtual bool OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port);
	virtual void OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) ;
	virtual void OnClientLeave(SESSION_ID sessionID);
	virtual void OnRecv(SESSION_ID sessionID, Packet *pPacket) ;
	virtual void OnSend(SESSION_ID sessionID);
	virtual void OnError(int errorcode, const WCHAR *log) ;
	virtual void OnTimeout(SESSION_ID sessionID);
	virtual void OnMonitoringPerSec(); // 1 초마다 갱신되는 모니터링

	void PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type);
	void PacketProcMonitorLogin(Packet *packet, SESSION_ID sessionID);
	void PacketProcMonitorDataUpdate(Packet *packet, SESSION_ID sessionID);
	void MakePacketMonitorDataUpdate(Packet *packet, BYTE serverNo, BYTE dataType, int dataValue, int timeStamp);

	void ResetChatServerData();
	void DataUpdateChatServer(BYTE dataType, int dataValue, int timeStamp);

private:
	/// <summary>
	/// 모니터링 할 서버 관리
	/// </summary>
	void InsertServer(SESSION_ID sessionID, ServerConnect *pConnection);
	void RemoveServer(SESSION_ID sessionID);
	ServerConnect *FindServer(SESSION_ID sessionID);

	unordered_map<SESSION_ID, ServerConnect *>		_serverMap;
private:
	// DB

	void QueryDataBase(int serverNo, const WCHAR *dataType, ULONGLONG total, ULONGLONG min, ULONGLONG max, ULONGLONG tick);

	DBConnectionPool _DBPool;
	WCHAR _QUERY_FORMAT[20][512]{
		L"INSERT INTO LOG_CHAT_%.2d%.2d(LOGTIME, DATATYPE, TOTAL, MIN, MAX, TICK) VALUES(NOW(),\"%s\",%d,%d,%d,%d)",
		L"CREATE TABLE LOG_CHAT_%.2d%.2d(ID INT NOT NULL PRIMARY KEY AUTO_INCREMENT,LOGTIME DATETIME NOT NULL,DATATYPE VARCHAR(32) NOT NULL,TOTAL BIGINT NOT NULL,MIN BIGINT NOT NULL,MAX BIGINT NOT NULL,TICK BIGINT NOT NULL)"
	};

	// 10분마다 db저장
	DWORD			_curLogTimer;
	DWORD			_preLogTimer;

private:
	bool _isRunning;

	// server start timestemp
	tm _timeFormet;
	time_t _startTime;



/// <summary>
/// 모니터링
/// </summary>
private:
	CMonitorToolServer *_pMonitorToolServer; //외부와의 소통을 위한 서버
	CParser *_pConfigData; // 서버 설정 파일
private:
	constexpr static int					MAX_VALUE = 0x7fffffff;
	constexpr static int					MIN_VALUE = 0;
	constexpr static int					TOTAL = 0;
	constexpr static int					MIN = 1;
	constexpr static int					MAX = 2;
	constexpr static int					TICK = 3;
	ULONGLONG								_Template[4] = { /* TOTAL */ 0, /* MIN */MAX_VALUE, /* MAX */MIN_VALUE, /* TICK */ 0};
	//---------------------------------------------------------------------
	//HARD WARE DATA

	ULONGLONG								_H_CPU[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_H_AvailableMemory[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_H_NonpagedMemory[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_H_NetworkSend[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_H_NetworkRecv[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	//---------------------------------------------------------------------

	//---------------------------------------------------------------------
	//CHAT SERVER DATA
	DWORD									_C_RunningFlag = false;
	ULONGLONG								_C_CPU[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_C_PrivateBytes[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_C_PacketPoolSize[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_C_SessionCount[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_C_PlayerCount[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_C_UpdateTPS[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_C_JobQueue[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	//---------------------------------------------------------------------

};

