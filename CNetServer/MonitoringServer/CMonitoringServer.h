#pragma once
#include "CServer.h"
#include "DBConnectionPool.h"
#include "CommonProtocol.h"
#include "SS_MoniteringProtocol.h"
#include <unordered_map>
#include "CMonitorToolServer.h"


// 내부에서 데이터를 받을 서버


class DBConnectionPool;
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

	void BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
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

	void PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type);
	void PacketProcMonitorLogin(Packet *packet, SESSION_ID sessionID);
	void PacketProcMonitorDataUpdate(Packet *packet, SESSION_ID sessionID);
	void MakePacketMonitorDataUpdate(Packet *packet, BYTE serverNo, BYTE dataType, int dataValue, int timeStamp);
private:


	void UpdateChatServerData(BYTE messageType, int data, int timeStamp);

private:
	bool _isRunning;

private:
	/// <summary>
	/// 모니터링 할 서버 관리
	/// </summary>


	void InsertServer(SESSION_ID sessionID, ServerConnect *pServer);
	void RemoveServer(SESSION_ID sessionID);
	ServerConnect *FindServer(SESSION_ID sessionID);

	unordered_map<SESSION_ID, ServerConnect *>		_serverMap;


/// <summary>
/// 모니터링
/// </summary>
private:
	CMonitorToolServer *_pMonitorToolServer; //외부와의 소통을 위한 서버

private:
	constexpr static int MAX_VALUE = 0x7fffffff;
	constexpr static int MIN_VALUE = 0;
	ULONGLONG								_Template[4] = { /* SUM */ 0, /* MAX */MAX_VALUE, /* MIN */MIN_VALUE, /* CALL */ 0};
	//---------------------------------------------------------------------
	//HARD WARE DATA

	ULONGLONG								_H_CPU[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_H_AvailableMemory[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_H_NonpagedMemory[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_H_NetworkSend[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_H_NetworkRecv[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	//---------------------------------------------------------------------

	//---------------------------------------------------------------------
	//GAME SERVER DATA
	DWORD									_G_RunningFlag = false;
	ULONGLONG								_G_CPU[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_PrivateBytes[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_SessionCount[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_AuthCount[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_PlayerCount[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_AcceptTPS[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_RecvTPS[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_SendTPS[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_DBWrite[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_DBQueue[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_AuthFPS[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_GameFPS[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_G_PacketPoolSize[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
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

	//---------------------------------------------------------------------
	//CHAT SERVER DATA
	DWORD									_L_RunningFlag = false;
	ULONGLONG								_L_CPU[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_L_PrivateBytes[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_L_PacketPoolSize[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_L_SessionCount[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	ULONGLONG								_L_AuthTPS[4] = { 0,MAX_VALUE,MIN_VALUE,0 };
	//---------------------------------------------------------------------

};

