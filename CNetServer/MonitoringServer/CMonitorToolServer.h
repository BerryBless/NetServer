#pragma once
#include "CServer.h"
#include "DBConnectionPool.h"
#include "CommonProtocol.h"
#include "SS_MoniteringProtocol.h"
#include "CMonitoringServer.h"
#include <set>
#include <vector>

class CMonitorToolServer : public CServer {
	friend class CMonitoringServer;

	struct MonitorClient {
		SESSION_ID _id=0;
		bool _isLogin=false;
	};
public:
	CMonitorToolServer();
	~CMonitorToolServer();
private:
	virtual bool OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port); // TODO IP주소 string
	virtual void OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) ;
	virtual void OnClientLeave(SESSION_ID sessionID) ;
	virtual void OnRecv(SESSION_ID sessionID, Packet *pPacket) ;
	virtual void OnSend(SESSION_ID sessionID);
	virtual void OnError(int errorcode, const WCHAR *log);
	virtual void OnTimeout(SESSION_ID sessionID) ;

private:
	void InsertClient(SESSION_ID sessionID, MonitorClient *pClient);
	void RemoveClient(SESSION_ID sessionID);
	MonitorClient *FindClient(SESSION_ID sessionID);

	unordered_map<SESSION_ID, MonitorClient *> _clientMap;

};

