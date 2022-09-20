#include "pch.h"
#include "CMonitoringTool.h"
#include "CommonProtocol.h"
#include "SC_MoniteringProtocol.h"

CMonitoringTool::CMonitoringTool(const WCHAR *szConfigFile) : CClient(ENCRYPTED_PACKET) {
	CClient::Start(1, 1, FALSE, 3);
	_pConfigData = new CParser(szConfigFile);
	_pConfigData->SetNamespace(L"MonitorClientConfig");

	// Monitor Server Connect Config
	_pConfigData->SetNamespace(L"MonitorServerConnect");
	_pConfigData->TryGetValue(L"MonitorServerIP", _monitorServerIP);
	_pConfigData->TryGetValue(L"MonitorServerPort", _monitorServerPort);


}

CMonitoringTool::~CMonitoringTool() {
	CClient::Quit();
}

void CMonitoringTool::SetWinHandle(HINSTANCE hInst, HWND hWnd) {
	_hInst = hInst;
	_hWnd = hWnd;

}

bool CMonitoringTool::ConnectMonitor() {
	return CClient::Connect(_monitorServerIP, _monitorServerPort);
}

void CMonitoringTool::OnEnterServer(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_ERROR, L"//OnEnterServer[%lld]", sessionID);
	Packet *pPacket = Packet::AllocAddRef();
	MakePacketMonitorToolResLogin(pPacket, _loginSessionKey);
	SendPacket(sessionID, pPacket);
	pPacket->SubRef();
}

void CMonitoringTool::OnLeaveServer(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_ERROR, L"//OnLeaveServer[%lld]", sessionID);
}

void CMonitoringTool::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	_LOG(dfLOG_LEVEL_DEBUG, L"//OnRecv");
	pPacket->AddRef();
	WORD type;
	(*pPacket) >> type;

	PacketProc(pPacket, sessionID, type);

	pPacket->SubRef();
}

void CMonitoringTool::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
	switch (type) {
	case PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE:
		PacketProcMonitorToolDataUpdate(pPacket, sessionID);
		break;
	case PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_RES_LOGIN:
		PacketProcMonitorToolResLogin(pPacket, sessionID);
		break;
	default:
		_LOG(dfLOG_LEVEL_ERROR, L"Default Case Monitoring Clinet Packet Procsess");
		DisconnectSession(sessionID);
		break;
	}
}

void CMonitoringTool::PacketProcMonitorToolResLogin(Packet *pPacket, SESSION_ID sessionID) {
	BYTE state;
	*pPacket >> state;
	switch (state) {
	case en_PACKET_CS_MONITOR_TOOL_RES_LOGIN::dfMONITOR_TOOL_LOGIN_OK:
	{
		Client *pClient = new Client;
		pClient->_isLogin = false;
		pClient->_data = 0;
		pClient->_ID = sessionID;
		InsertClient(sessionID, pClient);
	}
	break;
	case en_PACKET_CS_MONITOR_TOOL_RES_LOGIN::dfMONITOR_TOOL_LOGIN_ERR_SESSIONKEY:
	{
		DisconnectSession(sessionID);
		this->Quit(); // TODO 종료 함수 만들기
	}
	break;

	default:
		break;
	}
}

void CMonitoringTool::PacketProcMonitorToolDataUpdate(Packet *pPacket, SESSION_ID sessionID) {
	// TODO 그래프로 보여주기

	BYTE serverNo;
	BYTE dataType;
	int dataValue;
	int timeStamp;
	*pPacket >> serverNo >> dataType >> dataValue >> timeStamp;
	printf_s("serverNo[%d], dataType[%d], dataValue[%d] timeStamp[%d]\n", serverNo, dataType, dataValue, timeStamp);
	switch (dataType) {
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_CPU_USAGE:
		_C_CPU->InsertData(dataValue);
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_RECEIVE_PACKET_COUNT:
		_C_RecvPacket->InsertData(dataValue);
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_SEND_PACKET_COUNT:
		_C_SendPacket->InsertData(dataValue);
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_SESSION_COUNTS:
		_C_SessionCount->InsertData(dataValue);
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PLAYER_COUNTS:
		_C_PlayerCount->InsertData(dataValue);
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_UPDATE_TPS:
		_C_UpdateTPS->InsertData(dataValue);
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PACKET_POOL_USAGE:
		_C_PacketPoolSize->InsertData(dataValue);
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_UPDATE_MSG_QUEUE_SIZE:
		_C_JobQueue->InsertData(dataValue);
		break;
	default:
		break;
	}
}

void CMonitoringTool::MakePacketMonitorToolResLogin(Packet *pPacket, const char *loginSessionKey) {
	WORD type = PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN;
	*pPacket << type;
	pPacket->PutData((char *) loginSessionKey, MONITOR_LOGIN_SESSION_KEY_SIZE);
}

void CMonitoringTool::CreateView(HINSTANCE hInst, HWND hWnd) {
	_C_CPU = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"CPU USE",
		/* ux */20, /* uy */ 10, /* xsize */300, /* ysize*/ 200, /* max value */ 100, /* Alert value */85);
	_C_RecvPacket = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"Recv Packet Count",
		/* ux */340, /* uy */ 10, /* xsize */300, /* ysize*/ 200, /* max value */ 30000, /* Alert value */30000);
	_C_SendPacket = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"Send Packet Count",
		/* ux */660, /* uy */ 10, /* xsize */300, /* ysize*/ 200, /* max value */ 1000000, /* Alert value */0);
	_C_SessionCount = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"Session Count",
		/* ux */20, /* uy */ 230, /* xsize */300, /* ysize*/ 200, /* max value */ 20000, /* Alert value */15001);
	_C_PlayerCount = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"Player Count",
		/* ux */340, /* uy */ 230, /* xsize */300, /* ysize*/ 200, /* max value */ 20000, /* Alert value */15001);
	_C_PacketPoolSize = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"Packet Use Count",
		/* ux */660, /* uy */ 230, /* xsize */300, /* ysize*/ 200, /* max value */ 15000, /* Alert value */15000);
	_C_UpdateTPS = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"Update TPS",
		/* ux */20, /* uy */ 460, /* xsize */300, /* ysize*/ 200, /* max value */ 20000, /* Alert value */0);
	_C_JobQueue = new CMonitorGraphUnit(hInst, hWnd, CMonitorGraphUnit::TYPE::LINE_SINGLE, L"JobQueue",
		/* ux */340, /* uy */ 460, /* xsize */300, /* ysize*/ 200, /* max value */ 20000, /* Alert value */0);
}

void CMonitoringTool::InsertClient(SESSION_ID sessionID, Client *pClient) {
	_ClientMap.emplace(::make_pair(sessionID, pClient));
}

void CMonitoringTool::RemoveClient(SESSION_ID sessionID) {
	auto iter = _ClientMap.find(sessionID);
	if (iter == _ClientMap.end()) {
		return;
	}
	Client *pClient = iter->second;
	_ClientMap.erase(iter);

	pClient->_isLogin = false;
	pClient->_data = 0;

	delete pClient;
}

CMonitoringTool::Client *CMonitoringTool::FindClient(SESSION_ID sessionID) {
	auto iter = _ClientMap.find(sessionID);
	if (iter == _ClientMap.end()) {
		return nullptr;
	}
	return iter->second;
}
