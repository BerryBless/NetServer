#pragma once

enum SERVER_TYPE {
	MONITORING_SERVER = 0,
	GAME_SERVER = 1,
	CHAT_SERVER = 2,
	LOGIN_SERVER = 3,
	SERVER_MAX = 4,
};

enum GAME_SERVER_MONITORING_TYPE {
	GAME_SERVER_ON_OFF = 1,
	GAME_SERVER_CPU_USAGE = 2,
	GAME_SERVER_PRIVATE_BYTES = 3,
	GAME_SERVER_SESSION_COUNTS = 4,
	GAME_SERVER_AUTH_PLAYER = 5,
	GAME_SERVER_GAME_PLAYER = 6,
	GAME_SERVER_ACCEPT_TPS = 7,
	GAME_SERVER_PACKET_RECV_TPS = 8,
	GAME_SERVER_PACKET_SEND_TPS = 9,
	GAME_SERVER_DB_WRITE_TPS = 10,
	GAME_SERVER_DB_MSG_QUEUE_SIZE = 11,
	GAME_SERVER_AUTH_THREAD_FPS = 12,
	GAME_SERVER_GAME_THREAD_FPS = 13,
	GAME_SERVER_PACKET_POOL_USAGE = 14,
	GAME_SERVER_MAX = 15
};

enum CHAT_SERVER_MONITORING_TYPE {
	CHAT_SERVER_ON_OFF = 1,
	CHAT_SERVER_CPU_USAGE = 2,
	CHAT_SERVER_RECEIVE_PACKET_COUNT = 3,
	CHAT_SERVER_SEND_PACKET_COUNT = 4,
	CHAT_SERVER_SESSION_COUNTS = 5,
	CHAT_SERVER_PLAYER_COUNTS = 6,
	CHAT_SERVER_UPDATE_TPS = 7,
	CHAT_SERVER_PACKET_POOL_USAGE = 8,
	CHAT_SERVER_UPDATE_MSG_QUEUE_SIZE = 9,
	CHAT_SERVER_MAX = 9
};
enum LOGIN_SERVER_MONITORING_TYPE {
	LOGIN_SERVER_ON_OFF = 1,
	LOGIN_SERVER_CPU_USAGE = 2,
	LOGIN_SERVER_PRIVATE_BYTES = 3,
	LOGIN_SERVER_SESSION_COUNTS = 4,
	LOGIN_SERVER_AUTH_TPS = 5,
	LOGIN_SERVER_PACKET_POOL_USAGE = 6,
	LOGIN_SERVER_MAX = 7
};