#pragma once
//#ifndef __GODDAMNBUG_ONLINE_PROTOCOL__
//#define __GODDAMNBUG_ONLINE_PROTOCOL__



enum PACKET_TYPE {

//------------------------------------------------------
// Monitor Server Protocol
//------------------------------------------------------


////////////////////////////////////////////////////////
//
//   MonitorServer & MonitorTool Protocol / 응답을 받지 않음.
//
////////////////////////////////////////////////////////

//------------------------------------------------------
// Monitor Server  Protocol
//------------------------------------------------------
	en_PACKET_SS_MONITOR = 20000,
	//------------------------------------------------------
	// Server -> Monitor Protocol
	//------------------------------------------------------
	//------------------------------------------------------------
	// LoginServer, GameServer , ChatServer  가 모니터링 서버에 로그인 함
	//
	// 
	//	{
	//		WORD	Type
	//
	//		int		ServerNo		//  각 서버마다 고유 번호를 부여하여 사용
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_MONITOR_LOGIN,

	//------------------------------------------------------------
	// 서버가 모니터링서버로 데이터 전송
	// 각 서버는 자신이 모니터링중인 수치를 1초마다 모니터링 서버로 전송.
	//
	// 서버의 다운 및 기타 이유로 모니터링 데이터가 전달되지 못할떄를 대비하여 TimeStamp 를 전달한다.
	// 이는 모니터링 클라이언트에서 계산,비교 사용한다.
	// 
	//	{
	//		WORD	Type
	//
	//		BYTE	DataType				// 모니터링 데이터 Type 하단 Define 됨.
	//		int		DataValue				// 해당 데이터 수치.
	//		int		TimeStamp				// 해당 데이터를 얻은 시간 TIMESTAMP  (time() 함수)
	//										// 본래 time 함수는 time_t 타입변수이나 64bit 로 낭비스러우니
	//										// int 로 캐스팅하여 전송. 그래서 2038년 까지만 사용가능
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_MONITOR_DATA_UPDATE,


	en_PACKET_CS_MONITOR = 25000,
	//------------------------------------------------------
	// Monitor -> Monitor Tool Protocol  (Client <-> Server 프로토콜)
	//------------------------------------------------------
	//------------------------------------------------------------
	// 모니터링 클라이언트(툴) 이 모니터링 서버로 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		char	LoginSessionKey[32]		// 로그인 인증 키. (이는 모니터링 서버에 고정값으로 보유)
	//										// 각 모니터링 툴은 같은 키를 가지고 들어와야 함
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN,

	//------------------------------------------------------------
	// 모니터링 클라이언트(툴) 모니터링 서버로 로그인 응답
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	Status					// 로그인 결과 0 / 1 / 2 ... 하단 Define
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_MONITOR_TOOL_RES_LOGIN,

	//------------------------------------------------------------
	// 모니터링 서버가 모니터링 클라이언트(툴) 에게 모니터링 데이터 전송
	// 
	// 통합 모니터링 방식을 사용 중이므로, 모니터링 서버는 모든 모니터링 클라이언트에게
	// 수집되는 모든 데이터를 바로 전송시켜 준다.
	// 
	//
	// 데이터를 절약하기 위해서는 초단위로 모든 데이터를 묶어서 30~40개의 모니터링 데이터를 하나의 패킷으로 만드는게
	// 좋으나  여러가지 생각할 문제가 많으므로 그냥 각각의 모니터링 데이터를 개별적으로 전송처리 한다.
	//
	//	{
	//		WORD	Type
	//		
	//		BYTE	ServerNo				// 서버 No
	//		BYTE	DataType				// 모니터링 데이터 Type 하단 Define 됨.
	//		int		DataValue				// 해당 데이터 수치.
	//		int		TimeStamp				// 해당 데이터를 얻은 시간 TIMESTAMP  (time() 함수)
	//										// 본래 time 함수는 time_t 타입변수이나 64bit 로 낭비스러우니
	//										// int 로 캐스팅하여 전송. 그래서 2038년 까지만 사용가능
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE,
};




enum en_PACKET_SS_HEARTBEAT {
	dfTHREAD_TYPE_WORKER = 1,
	dfTHREAD_TYPE_DB = 2,
	dfTHREAD_TYPE_GAME = 3,
};
enum en_PACKET_CS_MONITOR_TOOL_SERVER_CONTROL {
	dfMONITOR_SERVER_TYPE_LOGIN = 1,
	dfMONITOR_SERVER_TYPE_GAME = 2,
	dfMONITOR_SERVER_TYPE_CHAT = 3,
	dfMONITOR_SERVER_TYPE_AGENT = 4,

	dfMONITOR_SERVER_CONTROL_SHUTDOWN = 1,		// 서버 정상종료 (게임서버 전용)
	dfMONITOR_SERVER_CONTROL_TERMINATE = 2,		// 서버 프로세스 강제종료
	dfMONITOR_SERVER_CONTROL_RUN = 3,		// 서버 프로세스 생성 & 실행
};


enum en_PACKET_SS_MONITOR_DATA_UPDATE {
	dfMONITOR_DATA_TYPE_LOGIN_SERVER_RUN = 1,		// 로그인서버 실행여부 ON / OFF
	dfMONITOR_DATA_TYPE_LOGIN_SERVER_CPU = 2,		// 로그인서버 CPU 사용률
	dfMONITOR_DATA_TYPE_LOGIN_SERVER_MEM = 3,		// 로그인서버 메모리 사용 MByte
	dfMONITOR_DATA_TYPE_LOGIN_SESSION = 4,		// 로그인서버 세션 수 (컨넥션 수)
	dfMONITOR_DATA_TYPE_LOGIN_AUTH_TPS = 5,		// 로그인서버 인증 처리 초당 횟수
	dfMONITOR_DATA_TYPE_LOGIN_PACKET_POOL = 6,		// 로그인서버 패킷풀 사용량


	dfMONITOR_DATA_TYPE_GAME_SERVER_RUN = 10,		// GameServer 실행 여부 ON / OFF
	dfMONITOR_DATA_TYPE_GAME_SERVER_CPU = 11,		// GameServer CPU 사용률
	dfMONITOR_DATA_TYPE_GAME_SERVER_MEM = 12,		// GameServer 메모리 사용 MByte
	dfMONITOR_DATA_TYPE_GAME_SESSION = 13,		// 게임서버 세션 수 (컨넥션 수)
	dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER = 14,		// 게임서버 AUTH MODE 플레이어 수
	dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER = 15,		// 게임서버 GAME MODE 플레이어 수
	dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS = 16,		// 게임서버 Accept 처리 초당 횟수
	dfMONITOR_DATA_TYPE_GAME_PACKET_RECV_TPS = 17,		// 게임서버 패킷처리 초당 횟수
	dfMONITOR_DATA_TYPE_GAME_PACKET_SEND_TPS = 18,		// 게임서버 패킷 보내기 초당 완료 횟수
	dfMONITOR_DATA_TYPE_GAME_DB_WRITE_TPS = 19,		// 게임서버 DB 저장 메시지 초당 처리 횟수
	dfMONITOR_DATA_TYPE_GAME_DB_WRITE_MSG = 20,		// 게임서버 DB 저장 메시지 큐 개수 (남은 수)
	dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS = 21,		// 게임서버 AUTH 스레드 초당 프레임 수 (루프 수)
	dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS = 22,		// 게임서버 GAME 스레드 초당 프레임 수 (루프 수)
	dfMONITOR_DATA_TYPE_GAME_PACKET_POOL = 23,		// 게임서버 패킷풀 사용량

	dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN = 30,		// 에이전트 ChatServer 실행 여부 ON / OFF
	dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU = 31,		// 에이전트 ChatServer CPU 사용률
	dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM = 32,		// 에이전트 ChatServer 메모리 사용 MByte
	dfMONITOR_DATA_TYPE_CHAT_SESSION = 33,		// 채팅서버 세션 수 (컨넥션 수)
	dfMONITOR_DATA_TYPE_CHAT_PLAYER = 34,		// 채팅서버 인증성공 사용자 수 (실제 접속자)
	dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS = 35,		// 채팅서버 UPDATE 스레드 초당 초리 횟수
	dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL = 36,		// 채팅서버 패킷풀 사용량
	dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL = 37,		// 채팅서버 UPDATE MSG 풀 사용량


	dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL = 40,		// 서버컴퓨터 CPU 전체 사용률
	dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY = 41,		// 서버컴퓨터 논페이지 메모리 MByte
	dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV = 42,		// 서버컴퓨터 네트워크 수신량 KByte
	dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND = 43,		// 서버컴퓨터 네트워크 송신량 KByte
	dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY = 44,		// 서버컴퓨터 사용가능 메모리
};


enum en_PACKET_CS_MONITOR_TOOL_RES_LOGIN {
	dfMONITOR_TOOL_LOGIN_OK = 1,				// 로그인 성공
	dfMONITOR_TOOL_LOGIN_ERR_NOSERVER = 2,		// 서버이름 오류 (매칭미스)
	dfMONITOR_TOOL_LOGIN_ERR_SESSIONKEY = 3,	// 로그인 세션키 오류
};


//#endif