#pragma once
#include "CServer.h"
#include "DBConnectionPool.h"
#include "CommonProtocol.h"
#include "SS_MoniteringProtocol.h"
#include "CMonitoringServer.h"
#include <set>
#include <vector>

// 외부에서 전용 클라이언트로 접속

class CMonitorToolServer : public CServer {
	friend class CMonitoringServer;

	struct MonitorClient {
		SESSION_ID _id = 0;
		bool _isLogin = false;
	};
public:
	CMonitorToolServer();
	~CMonitorToolServer();

	void BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	void BeginServer(const WCHAR *szConfigFile);
	void CloseServer();
	bool isRunning() {
		return _isRunning;
	}
	void CommandWait();

	void BroadcastPacket(Packet *pPacket);
private:
	virtual bool OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port); // TODO IP주소 string
	virtual void OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID);
	virtual void OnClientLeave(SESSION_ID sessionID);
	virtual void OnRecv(SESSION_ID sessionID, Packet *pPacket);
	virtual void OnSend(SESSION_ID sessionID);
	virtual void OnError(int errorcode, const WCHAR *log);
	virtual void OnTimeout(SESSION_ID sessionID);
	virtual void OnMonitoringPerSec(); // 1 초마다 갱신되는 모니터링


	void PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type);
	void PacketProcMonitorToolReqLogin(Packet *packet, SESSION_ID sessionID);
	void MakePacketMonitorToolReqLogin(Packet *packet, BYTE state);

private:
	char	_loginSessionKey[MONITOR_LOGIN_SESSION_KEY_SIZE + 1]{ "ajfw@!cv980dSZ[fje#@fdj123948djf" };
private:
	void InsertClient(SESSION_ID sessionID, MonitorClient *pClient);
	void RemoveClient(SESSION_ID sessionID);
	MonitorClient *FindClient(SESSION_ID sessionID);

	unordered_map<SESSION_ID, MonitorClient *> _clientMap;

private:
	bool _isRunning;
};

