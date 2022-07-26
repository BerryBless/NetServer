#pragma once

#ifndef __MONITERING_PROTOCOL_DEF__
#define __MONITERING_PROTOCOL_DEF__
enum ServerType {
	GameServer = 1,
	ChatServer = 2,
	LoginServer = 3,
	ServerMax = 4,
};

enum GameServerMoniteringType {
	OnOffFlag = 1,
	CPUUsage = 2,
	PrivateBytes = 3,
	SessionCounts = 4,
	AuthPlayer = 5,
	GamePlayer = 6,
	AcceptTPS = 7,
	PacketRecvTPS = 8,
	PacketSendTPS = 9,
	DBWriteTPS = 10,
	DBMessageQueueSize = 11,
	AuthThreadFPS = 12,
	GameThreadFPS = 13,
	PacketPoolUsage = 14,
};

enum ChatServerMoniteringType {
};

enum LoginServerMoniteringType {
};
#endif // !__MONITERING_PROTOCOL_DEF__
