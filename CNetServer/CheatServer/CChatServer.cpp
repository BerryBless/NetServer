#include "pch.h"
#include "CChatServer.h"
#include "ChatServerProtocol.h"

#define __PLAYER_MAP_LOCK()		this->PlayerMapLock()
#define __PLAYER_MAP_UNLOCK()	this->PlayerMapUnlock()
#define __SECTOR_LOCK(x, y)		this->SectorLock(x, y)
#define __SECTOR_UNLOCK(x, y)	this->SectorUnlock(x, y)

CChatServer::CChatServer() {
	CLogger::Initialize();
	CLogger::SetDirectory(L"serverlog");
	CLogger::SetLogLevel(dfLOG_LEVEL_ERROR);
	_isRunning = false;
}

CChatServer::~CChatServer() {
}

void CChatServer::BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	if (_isRunning == true) return;
	_isRunning = Start(IP, port, workerThreadCount, maxRunThreadCount, nagle, maxConnection);

	time(&_startTime);
	localtime_s(&_timeFormet, &_startTime);
}


void CChatServer::CloseServer() {
	Quit();
}


void CChatServer::CommandWait() {
}

bool CChatServer::OnConnectionRequest(WCHAR *IPStr, u_long IP, u_short Port) {


	return _isRunning;
}

void CChatServer::OnClientJoin(SESSION_ID SessionID) {
}

void CChatServer::OnClientLeave(SESSION_ID SessionID) {
	PacketProc(nullptr, SessionID, CHAT_PACKET_TYPE::ON_CLIENT_LEAVE);
}       


void CChatServer::OnRecv(SESSION_ID SessionID, CPacket *pPacket) {
	pPacket->AddRef();
	// TODO jobQueue

	WORD type;
	(*pPacket) >> type;
	PacketProc(pPacket, SessionID, type);

	pPacket->SubRef();
}

void CChatServer::OnError(int errorcode, const WCHAR *log) {
}

void CChatServer::OnTimeout(SESSION_ID SessionID) {
	PacketProc(nullptr, SessionID, CHAT_PACKET_TYPE::ON_TIME_OUT);
}

void CChatServer::PacketProc(CPacket *pPacket, SESSION_ID SessionID, WORD type) {
	pPacket->AddRef();

	WCHAR errmsg[512];
	switch (type) {
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_LOGIN:
		PacketProcRequestLogin(pPacket, SessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		PacketProcMoveSector(pPacket, SessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_MESSAGE:
		PacketProcChatRequire(pPacket, SessionID);
		break;
		break;
	case CHAT_PACKET_TYPE::ON_CLIENT_LEAVE:
		RemovePlayer(SessionID);
		break;

	case CHAT_PACKET_TYPE::ON_TIME_OUT:
		wsprintf(errmsg, L"ERROR :: Time Out Case SessionID : %I64u", SessionID);
		CLogger::_Log(dfLOG_LEVEL_ERROR, errmsg);
		Disconnect(SessionID);
		break;

	default:
		wsprintf(errmsg, L"ERROR :: Session Default Case SessionID : %I64u", SessionID);
		CLogger::_Log(dfLOG_LEVEL_ERROR, errmsg);
		Disconnect(SessionID);
		break;
	}

	pPacket->SubRef();
}

void CChatServer::PacketProcRequestLogin(CPacket *pPacket, SESSION_ID SessionID) {
	pPacket->AddRef();



	pPacket->SubRef();
}


void CChatServer::PacketProcMoveSector(CPacket *pPacket, SESSION_ID SessionID) {
	pPacket->AddRef();



	pPacket->SubRef();
}

void CChatServer::PacketProcChatRequire(CPacket *pPacket, SESSION_ID SessionID) {
	pPacket->AddRef();



	pPacket->SubRef();
}


// PACKET_SC_CHAT_RES_LOGIN
void CChatServer::MakePacketResponseLogin(CPacket *pPacket, ACCOUNT_NO account_no, BYTE status) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_LOGIN;
	(*pPacket) << type << status << (__int64) account_no;
}

//PACKET_SC_CHAT_RES_SECTOR_MOVE
void CChatServer::MakePacketResponseSectorMove(CPacket *pPacket, ACCOUNT_NO account_no, WORD sectorX, WORD sectorY) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_SECTOR_MOVE;
	(*pPacket) << type << (__int64) account_no << sectorX << sectorY;

}

//PACKET_SC_CHAT_RES_MESSAGE
void CChatServer::MakePacketResponseMessage(CPacket *pPacket, ACCOUNT_NO account_no, const WCHAR *ID, const WCHAR *nickName, const WCHAR *message, WORD msgLen) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_MESSAGE;
	(*pPacket) << type << (__int64) account_no;
	pPacket->PutData((char *) ID, ID_MAX_SIZE);
	pPacket->PutData((char *) nickName, NICK_NAME_MAX_SIZE);
	(*pPacket) << msgLen;
	pPacket->PutData((char *) message, msgLen);

}

void CChatServer::InsertPlayer(ULONGLONG SessionID, Player *pPlayer) {
	_playerMap.emplace(::make_pair(SessionID, pPlayer));
}

void CChatServer::RemovePlayer(ULONGLONG SessionID) {
	__PLAYER_MAP_LOCK();
	auto iter = _playerMap.find(SessionID);
	if (iter == _playerMap.end()) {
		__PLAYER_MAP_UNLOCK();
		return;
	}

	Player *pPlayer = iter->second;
	_playerMap.erase(iter);
	__PLAYER_MAP_UNLOCK();


	pPlayer->_isAlive = false;
	
	constexpr WORD comp = -1;
	WORD sx = pPlayer->_SectorX;
	WORD sy = pPlayer->_SectorY;
	pPlayer->_SectorX = comp;
	pPlayer->_SectorY = comp;
	if (sx == comp || sy == comp) {
		_playerPool.Free(pPlayer);
		return;
	}
	
	__SECTOR_LOCK(sx, sy);
	for (auto iter = _sector[sy][sx]._playerSet.begin(); iter != _sector[sy][sx]._playerSet.end(); ++iter) {
		if ((*iter)->_SessionID == SessionID) {
			_sector[sy][sx]._playerSet.erase(iter);
			break;
		}
	}
	__SECTOR_UNLOCK(sx, sy);

	_playerPool.Free(pPlayer);
}

Player *CChatServer::FindPlayer(ULONGLONG SessionID) {
	__PLAYER_MAP_LOCK();
	auto iter = _playerMap.find(SessionID);
	if (iter == _playerMap.end()) {
		__PLAYER_MAP_UNLOCK();
		return nullptr;
	}
	__PLAYER_MAP_UNLOCK();
	return iter->second;
}
