/*
RingBuffer

	맴버변수로 SRWLOCK 추가
	맴버함수로 Lock, Unlock 추가

	Enqueue,Dequeue 함수 내부에서 알아서 Lock Unlock 을 해주는 방법도 있지만
	사용자에게 맡기는 편이 성능면이나 확장성이 용이함.

	단 사용자가 실수해서 개판이 될 수 있음.


- 과제

1. RingBuffer - Lock, Unlock 추가.

2. RingBuffer 를 사용하여 잡 메시지 생성 스레드, 메시지 처리 스레드 테스트 구현

- 메시지 구조

struct st_MSG_HEAD
{
	short shType;
	short shStrLen;
};


// shType 이 0 (AddStr) 인 경우는 헤더 뒤에 shStrLen 길이만큼 문자열이 들어감.
// shType 이 1,2,3 인 경우는 헤더 뒤에 문자열이 없어도 됨.


- 메시지 타입

#define dfTYPE_ADD_STR		0
#define dfTYPE_DEL_STR		1
#define dfTYPE_PRINT_LIST	2
#define dfTYPE_QUIT		3

//-----------------------------------------------
// 컨텐츠 부, 문자열 리스트
//-----------------------------------------------
list<wstring>		g_List;

//-----------------------------------------------
// 스레드 메시지 큐 (사이즈 넉넉하게 크게 4~5만 바이트)
//-----------------------------------------------
CRingBuffer 		g_msgQ;



@ 메인스레드

- 워커스레드 3개 이상 생성.
- 고정 문자열 하나를 둔다. ex)"PROCADEMY"

{  (반복)

	1. 메시지 생성

	st_MSG_HEAD.shType = 랜덤하게 메시지 생성 (0 ~ 2)
	st_MSG_HEAD.String = 문자열을 랜덤하게 입력
	"문자열"  < 위 고정 문자열의 범위 내에서 랜덤하게 입력.

	이런 데이터를 생성.


	2. 스페이스키가 눌렸다면, 1번 무시하고 종료 메시지 (TYPE_QUIT) 넣기.

	3. 위에서 만든 메시지 큐에 넣고 워커스레드 깨우기

}

% 메인 스레드는 모든 워커스레드가 끝났다면 멈춘다.
% 메인 스레드의 루프는 50ms 대기시간 으로 개발
% 50ms 대기시간에 이상이 없으면 0ms 대기시간으로 테스트.




@ 워커스레드 (3개 이상)

- 메인스레드 에서 메시지를 넣지 않았다면 쉬고 있어야 함.

스레드 시작로그 - ID 출력

{ (반복)

	1. 메시지 큐에서 메시지 뽑음.

	2. 헤더 타입에 맞게끔

		ADD_STR - 메시지로 온 문자열을 g_List 에 추가.
		DEL_STR - g_List 에서 노드 하나 삭제
		PRINT_LIST - g_List 화면 출력 또는 저장 (또는 무언가 느린 작업)
		QUIT - 스레드 종료.

}


@ 실행결과 예시

List:
List:[PRO] [PROCA] [PROCADE]
List:[PRO] [PROCA] [PROCADE]
List:[PRO] [PROCA] [PROCADE]
List:[PROCAD] [PRO]
List:[PROCADEMY] [PROCAD] [PRO]
List:[PROCADEM] [PROCADEMY] [PROCAD]
List:[PROCAD] [PROC] [PROCADEM] [PROCADE]
List:[PROCADEMY] [PROCAD] [PROC] [PROCADEM]
List:[PRO] [PRO] [PROCADEMY] [PROCAD]
List:[PRO] [PRO] [PROCADEMY] [PROCAD]
List:[PROCADE] [PRO] [PRO] [PROCADEMY]
List:[PROCADEM] [PROCADE] [PRO] [PRO] [PROCADEMY]
List:[PROCADEM] [PROCADE] [PRO] [PRO] [PROCADEMY]
List:[PROCAD] [PROC] [PROC] [PROCADEM] [PROCADE] [PRO]
List:[PROCAD] [PRO] [PROCAD] [PROC] [PROC]
List:[PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD] [PROC] [PROC]
List:[PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD] [PROC] [PROC]
List:[PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD]
List:[PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD]
List:[PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD]
List:[PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD]
List:[PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD]
List:[PROC] [PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD]
List:[PROC] [PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD]
List:[PRO] [PROC] [PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD] [PRO] [PROCAD]
List:[PROCA] [PRO] [PROC] [PROCADEMY] [PRO] [PROC] [PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD] [PRO]
List:[PROCA] [PRO] [PROC] [PROCADEMY] [PRO] [PROC] [PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD] [PRO]
List:[PROCADEM] [PRO] [PRO] [PROCA] [PRO] [PROC] [PROCADEMY] [PRO] [PROC] [PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM] [PROCAD]
List:[PROCADEM] [PRO] [PRO] [PROCA] [PRO] [PROC] [PROCADEMY] [PRO] [PROC] [PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM]
List:[PROCADEM] [PRO] [PRO] [PROCA] [PRO] [PROC] [PROCADEMY] [PRO] [PROC] [PROCAD] [PROCADEM] [PROCA] [PROC] [PROCADEM]
List:[PROC] [PROCADEM] [PROCADEM] [PRO] [PRO] [PROCA] [PRO] [PROC] [PROCADEMY] [PRO] [PROC] [PROCAD] [PROCADEM] [PROCA] [PROC]
List:[PROC] [PROCAD] [PROC] [PROC] [PROCADEM] [PROCADEM] [PRO] [PRO] [PROCA] [PRO] [PROC] [PROCADEMY] [PRO] [PROC] [PROCAD]


# PROCADEMY  문자열 내에서 3자 ~ 9자 까지 랜덤하게 입력
# 메인 스레드는 문자열 리스트 추가 / 리스트에서 삭제 / 리스트 출력 메시지를 생성.
# 3개의 워커스레드는 메시지를 처리.




# 메인스레드는  1초마다 메시지 큐의 빈 공간 / 사용중인 공간 을 출력 / 워커 스레드의 TPS 출력

*/
#include <string>
#include <list>
#include <iostream>
#include <conio.h>
#include "CRingBuffer.h"

using namespace std;
//-----------------------------------------------
// 메시지 링버퍼 사이즈
//-----------------------------------------------
#define dfTYPE_ADD_STR		0
#define dfTYPE_DEL_STR		1
#define dfTYPE_PRINT_LIST	2
#define dfTYPE_QUIT		3

#define dfMAX_RINGBUFFER_SIZE 400000
#define dfWORKER_THREAD_CNT 26
#define dfWAIT_TIME 0

//-----------------------------------------------
// 스레드 메시지 구조체
//-----------------------------------------------
struct st_MSG_HEAD {
	short shType;
	short shStrLen;
};
//===============================================
// 전역변수
//===============================================
// 스레드 메시지 큐 (사이즈 넉넉하게 크게 4~5만 바이트)
CRingBuffer 		g_msgQ(dfMAX_RINGBUFFER_SIZE);
// 스레드 제어 이벤트
HANDLE g_event;
// srwlock
SRWLOCK g_srwlockList;

//===============================================
// 컨텐츠 부
// 문자열 리스트
//===============================================
list<wstring>		g_List;

wstring g_msg = L"PROCADEMY";

FILE *g_fp;

//===============================================
// 프로파일링
//===============================================
DWORD g_oldtime;
int g_enqcnt = 0;
int g_deqcnt = 0;
int g_insertcnt = 0;
int g_deletecnt = 0;
int g_printcnt = 0;

void PrintProfile();

unsigned int __stdcall WorkerThread(PVOID pParam);

int main() {

	HANDLE hThread[dfWORKER_THREAD_CNT];
	srand(50);
	InitializeSRWLock(&g_srwlockList); // srwlock 초기화

	fopen_s(&g_fp, "ringbuffer.log", "w");

	g_event = CreateEvent(nullptr, false, false, nullptr);// 오토리셋 이벤트

	// 갯수만큼 워커스레드 생성
	for (int i = 0; i < dfWORKER_THREAD_CNT; i++) {
		hThread[i] = (HANDLE) _beginthreadex(nullptr, 0, WorkerThread, 0, 0, nullptr);
	}




	bool run = true;	// 루프 도는지
	st_MSG_HEAD header;	// 메시지헤더
	DWORD err;// 에러코드 뽑아내기용도
	g_oldtime = clock();
	while (run) {
		header.shType = rand() % 3;		// 0 ~ 2
		header.shStrLen = rand() % 8 + 2; // 2 ~ 9

		if (_kbhit()) {
			char cmd = _getch();
			if (cmd == 32) {
				// 스페이스바 눌림
				// 종료
				header.shType = dfTYPE_QUIT;
				header.shStrLen = 0xABCD;

			}
		}

		// 큐에 넣기
		int ret = g_msgQ.Enqueue((char *) &header, sizeof(header));
		g_enqcnt++;

		if (ret == 0) {
			wprintf_s(L"///////RINGBUFFER FULL \n");
			g_msgQ.PrintfInfo();
			Sleep(1000);
		}
		PrintProfile();

		// dfWAIT_TIME 만큼 기다려 보기
		DWORD retval = WaitForMultipleObjects(dfWORKER_THREAD_CNT, hThread, true, dfWAIT_TIME);

		switch (retval) {
		case WAIT_FAILED:
			// 대기 실패
			err = GetLastError();
			wprintf_s(L"////// WorkerThread HANDLE ERROR :: WAIT_FAILED :: ERRORCODE [%d]\n", err);
			return 1;
		case WAIT_OBJECT_0:
			// 모든 스레드 종료
			wprintf_s(L"ALL THREAD CLOSED");
			run = false;
			break;
		case WAIT_TIMEOUT:
			// 50ms 지남
			// 시그널!
			SetEvent(g_event);
			break;
		default:
			break;
		}
		//Sleep(dfWAIT_TIME);
	}

	fclose(g_fp);
	CloseHandle(g_event);
	for (int i = 0; i < dfWORKER_THREAD_CNT; i++) {
		CloseHandle(hThread[i]);
	}
	return 0;
}


void PrintProfile() {
	DWORD curtime = clock();
	if (curtime - g_oldtime > 1000) {
		wprintf_s(L"-------------------------------------------------\n");
		wprintf_s(L"EnqTPS [%d] | DeqTPS [%d]\nInsertTPS [%d] | DeleteTPS [%d] | PrintTPS [%d]\n", g_enqcnt, g_deqcnt, g_insertcnt, g_deletecnt, g_printcnt);
		g_msgQ.PrintfInfo();
		wprintf_s(L"-------------------------------------------------\n");

		g_enqcnt = 0;
		g_deqcnt = 0;
		g_insertcnt = 0;
		g_deletecnt = 0;
		g_printcnt = 0;

		g_oldtime = curtime;
	}
}

unsigned int __stdcall WorkerThread(PVOID pParam) {
	DWORD err;
	st_MSG_HEAD header;
	while (true) {
		// 이벤트 대기
		DWORD retval = WaitForSingleObject(g_event, INFINITE);

		switch (retval) {
		case WAIT_FAILED:
		case WAIT_TIMEOUT:
			err = GetLastError();
			wprintf_s(L"////// WorkerThread HANDLE ERROR :: WAIT_FAILED :: ERRORCODE [%d]\n", err);

			return 1;
		default:
			break;
		}

		// dequeue 해가기
		g_msgQ.Lock();
		if (g_msgQ.GetUseSize() < sizeof(header)) {
			g_msgQ.Unlock();
			continue;
		}

		g_msgQ.Dequeue((char *) &header, sizeof(header));
		g_deqcnt++;


		// 처리할꺼 남았으면 다른스레드도!
		if (g_msgQ.GetUseSize() > 0) {
			SetEvent(g_event);
		}
		g_msgQ.Unlock();
		switch (header.shType) {
		case dfTYPE_ADD_STR:
			// 독점
			AcquireSRWLockExclusive(&g_srwlockList);
			g_List.push_back(g_msg.substr(0, header.shStrLen));
			g_insertcnt++;
			ReleaseSRWLockExclusive(&g_srwlockList);
			break;
		case dfTYPE_DEL_STR:
			// 독점
			AcquireSRWLockExclusive(&g_srwlockList);
			if (g_List.size() > 0) {
				g_List.pop_front();
				g_deletecnt++;
			}
			ReleaseSRWLockExclusive(&g_srwlockList);
			break;
		case dfTYPE_PRINT_LIST: {
			InterlockedIncrement((long *) &g_printcnt);
			wstring buf;

			// 공유
			AcquireSRWLockShared(&g_srwlockList);
			{
				for (auto iter = g_List.begin(); iter != g_List.end(); ++iter) {
					buf += L"[";
					buf += *iter;
					buf += L"]";
					//fwprintf_s(g_fp, L"[%s] ", iter->c_str());
				}

				/*int a = 0;
				while (a < 240000) {
					a++;
					YieldProcessor();
				}*/
				//Sleep(1);
			}
			ReleaseSRWLockShared(&g_srwlockList);
			fwprintf_s(g_fp, L"[%5d]List: [%s]\n", GetCurrentThreadId(), buf.c_str());
		}
							  break;
		case dfTYPE_QUIT:
			// TEMP ::다른 스레드도 종료
		{
			int ret;
			do {
				header.shType = dfTYPE_QUIT;
				header.shStrLen = 0xABCD;
				ret = g_msgQ.Enqueue((char *) &header, sizeof(header));
			} while (ret == 0);
		}
		return 0;
		default:
			break;
		}


	}

}

