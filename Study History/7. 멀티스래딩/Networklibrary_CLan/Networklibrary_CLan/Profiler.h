/*
* 싱글스레드 전용!
* 멀티스레드 에서는 TagName찾을때 경합이 일어납니다!!
* 사용법

* 시간측정
void f2() {
	PRO_START(L"F2");	// 시작할곳에 PRO_START(TagName) 넣기
	Sleep(80);
}
void f3() {
	PRO_BEGIN(L"F3");	// PRO_BEGIN(TagName)
	Sleep(1000);
	PRO_END(L"F3");		// PRO_END(TagName)
}

* 출력
	PRO_PRINT((WCHAR *)L"log.log"); // 로그파일문자열(WCHAR *) 파일이름

* 일고리즘
*	TagName을 지정해 TagName사이의 시간, 실행 횟수를 측정합니다.
*/
#pragma once
#include <stdio.h>
#include <Windows.h>

#define CRASH() do{int* p=nullptr; *p=0; }while(0)

#define dfPROFILER // 프로파일링을 끄고싶다면 주석처리

#ifdef dfPROFILER

// ================================
// 멀티바이트 문자열 처리 함수
// 필요한것만 구현
// Strlen : 문자열 길이
// Strcpy : 문자열 복사
// Strcmp : 문자열 비교 (같으면 return 0)
// ================================

int Strlen(const WCHAR *str);
void Strcpy(WCHAR *strDest, const WCHAR *strSrc);
int Strcmp(const WCHAR *str1, const WCHAR *str2);


#define TICK_TO_MICSEC 1000000	// QueryPerformanceFrequency tick을 마이크로초 단위로 바꿉니다
#define dfPROFILE_SAMPLE_COUNT 20		// 프로파일링할 샘플의 최대 숫자

// ================================
// 매크로함수로 사용
// 프로파일링을 끄고 싶을때는 #define dfPROFILER 주석처리 하기
// ================================
#define PRO_INIT(cnt)			CProfiler::InitProfiler(cnt)
#define PRO_DESTROY()			CProfiler::DestroyProfiler()
#define PRO_RESET()				CProfiler::Reset()
#define PRO_BEGIN(TagName)		CProfiler::Begin(TagName)			// 프로파일링을 시작할 곳의 TagName을 지정합니다.
#define PRO_END(TagName)		CProfiler::End(TagName)			// 프로파일링을 끝낼 곳의 TagName을 지정합니다.
#define PRO_PRINT(LogFile)		CProfiler::Print(LogFile)			// 프로파일링 데이터를 LogFile로 출력합니다.


class CProfiler {
private:
	struct PROFILE_SAMPLE {
		bool			isCheack;			// 이 샘플 데이터가 사용중인지?
		WCHAR			Tag[64];			// 프로파일 샘플 이름.

		LARGE_INTEGER	StartTime;			// 프로파일 샘플 실행 시간.

		__int64			TotalTime;			// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
		__int64			Min[2];				// 최소 사용시간 카운터 Time. [0] : 전체 최소 [1] : [0]보다 크지만 가장 작은값	
		__int64			Max[2];				// 최대 사용시간 카운터 Time. [0] : 전체 최대 [1] : [0]보다 작지만 가장 큰값	

		__int64			Call;				// 누적 호출 횟수.

	};
private :

	DWORD _threadID;
	PROFILE_SAMPLE _samples[dfPROFILE_SAMPLE_COUNT];

public:
	static DWORD _tlsIdx;	// 
	static SRWLOCK _printLock; //  
	static CProfiler **_profiler; // 프로파일러
	static int _profilerIdx; // 프로파일러
	static int _profilerCount; // 프로파일러
private:
	int TagToIndex(const wchar_t *tag); // 음수면 실패
public:
	CProfiler();
	~CProfiler();

	void SetThreadId();

	void ProfileBegin(const wchar_t *tag);
	void ProfileEnd(const wchar_t *tag);
	void ProfilePrintFile(const wchar_t *filename);
	void ProfileReset();


	// TLS에서 this를 찾아 각각 메소드 호출
	// TODO 이객체를 어떻게 생성, 초기화 할것인가?
	static void InitProfiler(int threadCount);
	static void DestroyProfiler();
	static void Reset();
	static void Begin(const wchar_t *tag);
	static void End(const wchar_t *tag);
	static void Print(const wchar_t *filename);


};


// ================================
// 프로파일링 샘플 구조체
// ================================



// ================================
// ProfileBegin : name의 시작시간을 체크합니다
// ProfileEnd : name의 끝시간을 체크하고, 걸린시간을 계산하여 통계를 냅니다.
// ProfileDataOutText : fileName으로 파일출력을 합니다.
// ProfileReset : 프로파일링데이터를 초기화 합니다.
// ================================

void ProfileBegin(const WCHAR *name);
void ProfileEnd(const WCHAR *name);
void ProfileDataOutText(const WCHAR *fileName);
void ProfileReset(void);

//*/
#else
// ================================
// 프로파일링 작동 안함
// ================================
#define PRO_INIT(cnt)				
#define PRO_DESTROY()		
#define PRO_RESET()			
#define PRO_BEGIN(TagName)		
#define PRO_END(TagName)		
#define PRO_PRINT(LogFile)		
#endif
