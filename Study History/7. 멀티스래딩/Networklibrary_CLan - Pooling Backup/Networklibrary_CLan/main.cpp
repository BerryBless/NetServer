/*
CLanServer
CLanClient

CNetServer
CNetClient

# 서버간 통신 모듈 제작.


목적: 클라이언트 유저와 서버간의 통신 모듈이 아닌  내부 네트워크의 서버와 서버간의 통신을 목적으로 함

- 대규모의 접속을 처리하지 않아도 된다.

- 최대한 간단한 프로토콜을 사용하며, 보안은 신경쓰지 않는다.

- 간단한 사용법을 가지도록 만든다.

- 서버간의 통신 이지만 그래도 클라와 서버 개념은 있다.

- 유저간 통신 모듈을 사용하여 유저처럼 서버를 연결하여 유저와 똑같이 처리를 하는 것도 가능은 함.


- 하지만 서버와의 통신은 유저통신과 별도로 분리 시키는 것이 관리면이나 반응면에서 좋음.

  why) 서버간의 통신은 최대한 빠르고 안전하게 유저수에 영향을 받지 않고 영향을 주지 않는게 좋음

  why) 서버간의 통신은 별도의 랜카드를 사용하여 일반 유저접속용 IP 와는 다르게 가는게 일반적.

  why) 유저간 통신 모듈은 덩치가 크며 프로토콜 자체도 무거우므로 서버간 통신은 프로토콜도 개별적으로 감.







# 서버간 통신 모듈 설계

클라역할 - CLanClient

서버역할 - CLanServer



	- 설계 가이드.

	* IOCP 모델을 사용함.

	* 클래스 내부에 워커 스레드를 가짐.

	* 이벤트 함수 (접속완료,받기완료,보내기완료) 는 순수가상 함수로 제작.

	* 실제 사용부는 위 두개의 클래스를 상속 사용하도록 제작.

	* 내부에서 자체적으로 세션 연결을 관리함.



CLanServer


	- bool Start(...) 오픈 IP / 포트 / 워커스레드 수 (생성수, 러닝수) / 나글옵션 / 최대접속자 수
	- void Stop(...)
	- int GetSessionCount(...)

	- bool Disconnect(SessionID) 		/ SESSION_ID / HOST_ID
	- bool SendPacket(SessionID, CPacket *)   / SESSION_ID / HOST_ID

	virtual bool OnConnectionRequest(IP,Port) = 0;     	   < accept 직후
			return false; 시 클라이언트 거부.
			return true; 시 접속 허용

	virtual void OnClientJoin(Client 정보 / SessionID / 기타등등) = 0;   < Accept 후 접속처리 완료 후 호출.
	virtual void OnClientLeave(SessionID) = 0;   	         	   < Release 후 호출


	virtual void OnRecv(SessionID, CPacket *) = 0;              < 패킷 수신 완료 후
//	virtual void OnSend(SessionID, int sendsize) = 0;           < 패킷 송신 완료 후

//	virtual void OnWorkerThreadBegin() = 0;                    < 워커스레드 GQCS 바로 하단에서 호출
//	virtual void OnWorkerThreadEnd() = 0;                      < 워커스레드 1루프 종료 후

	virtual void OnError(int errorcode, wchar *) = 0;
















CLanClient

	- bool Connect	바인딩 IP, 서버IP / 워커스레드 수 / 나글옵션
	- bool Disconnect()
	- bool SendPacket(CPacket *)

	virtual void OnEnterJoinServer() = 0;		< 서버와의 연결 성공 후
	virtual void OnLeaveServer() = 0;		< 서버와의 연결이 끊어졌을 때

	virtual void OnRecv(Packet *) = 0;		< 하나의 패킷 수신 완료 후
	virtual void OnSend(int sendsize) = 0;		< 패킷 송신 완료 후

//	virtual void OnWorkerThreadBegin() = 0;
//	virtual void OnWorkerThreadEnd() = 0;

	virtual void OnError(int errorcode, wchar *) = 0;




*/
#include "CEchoServer.h"

#define dfWORKERTHREAD 6
#define dfRUNTHREAD 4
#define dfPORT 6000
int main() {
	CEchoServer server;

	server.BeginServer(INADDR_ANY, dfPORT, dfWORKERTHREAD, dfRUNTHREAD, false, 200);
	return 0;
}