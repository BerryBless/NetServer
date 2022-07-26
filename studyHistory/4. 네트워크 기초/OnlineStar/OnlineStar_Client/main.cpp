#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib" )

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <locale.h> // wchar 한글
#include <conio.h>//_kbhit
#include "Profiler.h"

#define dfSERVERIP L"192.168.30.13"
#define dfSERVERPORT 3000

#define dfMAXPLAYER 1000
#define dfMAXWITDH 80
#define dfMAXHEIGHT 23

//#define ScreenBuffer
#define Print
#ifdef Print
void gotoxy(int x, int y);
#endif // Print
/*
ID할당(0)	Type(4Byte) | ID(4Byte) | 안씀(4Byte)	| 안씀(4Byte)
별생성(1)	Type(4Byte) | ID(4Byte) | X(4Byte)		| Y(4Byte)
별삭제(2)	Type(4Byte) | ID(4Byte) | 안씀(4Byte)	| 안씀(4Byte)
이동(3)		Type(4Byte) | ID(4Byte) | X(4Byte)		| Y(4Byte)
*/

typedef int msgHEADER;
struct msgIDSET {
	int ID;
	int TEMP[2];
};
struct msgSTARCREATE {
	int ID;
	int X;
	int Y;
};
struct msgSTARDELETE {
	int ID;
	int TEMP[2];
};
struct msgMOVE {
	int ID;
	int X;
	int Y;
};
struct stPLAYER {
	int ID;
	int X;
	int Y;
};

int main() {
	// 한글설정
	setlocale(LC_ALL, "KOREAN");

	// 변수
	int sRet;	// sned return
	int rRet;	// recv return
	char buf[16]; // recv 버퍼
	DWORD err;	// 겟라스트에러

	int iVAxis = 0; // Vertical
	int iHAxis = 0; // Horizon

	int packetCount = 0;

	stPLAYER stPlayers[dfMAXPLAYER]; // 별 객체
	stPLAYER *myPlayer;				// 내 별!
	int myID;						// 내 ID
	int playerCount;

	// 변수 초기화
	memset(stPlayers, 0xFF, sizeof(stPLAYER) * dfMAXPLAYER);
	myPlayer = NULL;
	myID = -1;
	playerCount = 0;

	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wprintf_s(L"WSAStartup() errcode[%d]\n", WSAGetLastError());
		return 1;
	}

	// 소켓 기본정보
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0xDD, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(dfSERVERPORT);
#ifdef dfSERVERIP
	InetPton(AF_INET, dfSERVERIP, &sockAddr.sin_addr);
#else
	WCHAR serverIP[16];
	wprintf_s(L"접속할 IP 주소 : ");
	wscanf_s(L"%ls", serverIP);
	InetPton(AF_INET, serverIP, &sockAddr.sin_addr);
#endif // DEBUG

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		wprintf_s(L"socket() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// connect()
	sRet = connect(sock, (SOCKADDR *) &sockAddr, sizeof(sockAddr));
	if (sRet == SOCKET_ERROR) {
		wprintf_s(L"connect() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// 비동기 소켓으로 변경
	u_long on = 1;
	sRet = ioctlsocket(sock, FIONBIO, &on);
	if (sRet == SOCKET_ERROR) {
		wprintf_s(L"ioctlsocket() errcode[%d]\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	while (true) {

#pragma region NETWORK
		PRO_BEGIN(L"NETWORK");
		// select 모델
		fd_set rSet, wSet;


		// 소켓 셋 받기로 초기화
		PRO_BEGIN(L"INIT_SELECT");

		FD_ZERO(&rSet);
		FD_ZERO(&wSet);
		FD_SET(sock, &rSet);
		FD_SET(sock, &wSet);
		timeval time;

		packetCount = 0;

		// 비동기 최대 대기시간 설정
		time.tv_sec = 0;
		time.tv_usec = 1;
		PRO_END(L"INIT_SELECT");

		PRO_BEGIN(L"select()");
		rRet = select(0, &rSet, &wSet, NULL, &time);
		if (rRet == SOCKET_ERROR) {
			err = GetLastError();
			if (err == WSAEWOULDBLOCK) {
				// 들어오지 않음 = 에러가아님
				wprintf_s(L"select() WSAEWOULDBLOCK errcode[%d]\n", err);

			} else {
				// 진짜로 에러!
				wprintf_s(L"select() errcode[%d]\n", err);
			}
		}
		PRO_END(L"select()");

		PRO_BEGIN(L"RECV BUFF");
		// 뭔가 왔음!
		// 그 뭔가를 버퍼에서 긁어오기
		// 1프레임당 1메시지 처리가아니라 1프레임에 버퍼에있는 모든 메시지 처리하기

		while (FD_ISSET(sock, &rSet)) {
			rRet = recv(sock, buf, 16, 0);
			if (rRet == SOCKET_ERROR) {
				err = WSAGetLastError();
				if (err == WSAEWOULDBLOCK) {
					// 버퍼가 비었다!
					break;
				} else {
					wprintf_s(L"recv() errcode[%d]\n", err);
					WSACleanup();
					closesocket(sock);
					return 1;
				}
			} else if (rRet > 0) {
				packetCount++;

				msgHEADER *head = (msgHEADER *) buf;

				switch (*head) {
				case 0:
					// ID할당(0)
				{
					msgIDSET *msg = (msgIDSET *) (buf + sizeof(msgHEADER));
					myID = msg->ID;
				}
				break;
				case 1:
					// 별생성(1)
				{
					playerCount++;
					msgSTARCREATE *msg = (msgSTARCREATE *) (buf + sizeof(msgHEADER));
					// ID가  -1 이면 빈칸!
					for (int i = 0; i < dfMAXPLAYER; i++) {
						if (stPlayers[i].ID == -1) {
							// 정보넣기
							stPlayers[i].ID = msg->ID;
							stPlayers[i].X = msg->X;
							stPlayers[i].Y = msg->Y;
							// 그플레이어가 나면 포인터 따로 저장해두기
							if (msg->ID == myID) {
								myPlayer = stPlayers + i;
							}
							break;
						}
					}
				}
				break;
				case 2:
					// 별삭제(2)
				{
					playerCount--;
					msgSTARDELETE *msg = (msgSTARDELETE *) (buf + sizeof(msgHEADER));
					for (int i = 0; i < dfMAXPLAYER; i++) {
						if (stPlayers[i].ID == msg->ID) {
							// 그 ID 찾아서 FF로 밀어주기 -> 2의 보수임으로 ID = -1
							memset(stPlayers + i, 0xFF, sizeof(stPLAYER));
						}
					}
				}
				break;
				case 3:
					// 이동(3)
				{
					msgMOVE *msg = (msgMOVE *) (buf + sizeof(msgHEADER));
					if (msg->ID == myID) break;
					for (int i = 0; i < dfMAXPLAYER; i++) {
						if (stPlayers[i].ID == msg->ID) {
							stPlayers[i].X = msg->X;
							stPlayers[i].Y = msg->Y;
							break;
						}
					}
				}
				break;

				default:
					break;
				}

			} else {
				break;
			}
		}
		PRO_END(L"RECV BUFF");

		PRO_END(L"NETWORK");

#pragma endregion

#pragma region INPUT
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) {
			break;
		}

		PRO_BEGIN(L"INPUT");
		//*
		// 1. 입력 받아서
		// 2. msgMOVE 메시지를 만들고
		// 3. 보내기
		//

		// 보낼 소캣이 충분하다
		if (FD_ISSET(sock, &wSet)) {
			msgHEADER header = 3;
			msgMOVE msg;
			if (myPlayer == NULL) {
				for (int i = 0; i < dfMAXPLAYER; i++) {
					if (stPlayers[i].ID == myID) {
						myPlayer = stPlayers + i;
						break;
					}
				}
			}

			// UP키가 눌려있는 상태
			if (GetAsyncKeyState(VK_UP) & 0x8001) {
				// 위로
				iVAxis += -1;
			}
			// RIGHT키가 눌려있는 상태
			if (GetAsyncKeyState(VK_RIGHT) & 0x8001) {
				// 오른쪽으로
				iHAxis += 1;
			}
			// DOWN키가 눌려있는 상태
			if (GetAsyncKeyState(VK_DOWN) & 0x8001) {
				// 아래로
				iVAxis += 1;
			}
			// LEFT키가 눌려있는 상태
			if (GetAsyncKeyState(VK_LEFT) & 0x8001) {
				// 왼쪽으로
				iHAxis += -1;
			}
			// 움직일 플레이어가 있으면
			if (myPlayer != NULL) {
				// 다음위치 구하고
				int iNextY = myPlayer->Y + iVAxis;
				int iNextX = myPlayer->X + iHAxis;
				bool bMove = false;	// 움직였다!

				// 이동했으면 내꺼 먼저 움직이기
				if (0 <= iNextX && iNextX <= dfMAXWITDH &&
					myPlayer->X != iNextX) {
					myPlayer->X = iNextX;
					bMove = true;
				}
				if (0 <= iNextY && iNextY <= dfMAXHEIGHT &&
					myPlayer->Y != iNextY) {
					myPlayer->Y = iNextY;
					bMove = true;
				}
				// 이동했는지, 범위내인지 확인
				if (bMove) {

					// 메시지만들고
					msg.ID = myID;
					msg.Y = myPlayer->Y;
					msg.X = myPlayer->X;

					// 메시지 싣고
					memcpy_s(buf, 16, &header, sizeof(msgHEADER));
					memcpy_s(buf + sizeof(msgHEADER), 16, &msg, sizeof(msgMOVE));

					// send()
					sRet = send(sock, buf, 16, 0);
					if (sRet == SOCKET_ERROR) {
						err = GetLastError();
						if (err == WSAEWOULDBLOCK) {
							// 비동기, 보낼게없음
						} else {
							wprintf_s(L"send() errorcode[%d]\n", err);
							WSACleanup();
							break;
						}
					}
				}
				// 이동값 초기화
				iVAxis = 0;
				iHAxis = 0;
			}
		}//*/
		PRO_END(L"INPUT");
#pragma endregion

#pragma region RENDER
		PRO_BEGIN(L"RENDER");
#ifdef Print
		system("cls");
		wprintf(L"Connet Clinet(%d) packet (%d)", playerCount, packetCount);
		for (int i = 0; i < dfMAXPLAYER; i++) {
			if (stPlayers[i].ID != -1) {
				gotoxy(stPlayers[i].X, stPlayers[i].Y);
				wprintf_s(L"*");
			}
		}

#endif // Print
		PRO_END(L"RENDER");
#pragma endregion
		Sleep(5);
	}
	PRO_PRINT("LOG.log");

	WSACleanup();
	closesocket(sock);
	return 0;
}


#ifdef Print
void gotoxy(int x, int y) {
	COORD pos = {x,y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
#endif // Print
