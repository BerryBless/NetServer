#pragma once
#include <stdio.h>
#include <Windows.h>

#define PROFILE


#pragma region StringFunc

// WCHAR스트링의 길이를 구합니다
int Strlen(const WCHAR* str);
// WCHAR스트링을 복사합니다.
void Strcpy(WCHAR* strDest, const WCHAR* strSrc);
// WCHAR스트링의 비교를 합니다
int Strcmp(const WCHAR* str1, const WCHAR* str2);

#pragma endregion

#ifdef PROFILE
#define TICK_TO_MICSEC 1000000	
#define PROFILECOUNT 10

// 언재든 프로파일링을 끌 수 있게 매크로 함수로 호출합니다.
#define PRO_START(TagName)	cProfiler profile (TagName)
#define PRO_BEGIN(TagName)	Profile_Begin(TagName)
#define PRO_END(TagName)	Profile_End(TagName)
#define PRO_PRINT(LogFile)	Profile_DataOutText(LogFile)

typedef struct {
	WCHAR			Name[64];			// 프로파일 샘플 이름.

	LARGE_INTEGER	StartTime;			// 프로파일 샘플 실행 시간.

	__int64			TotalTime;			// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
	__int64			Min;				// 최소 사용시간 카운터 Time.	
	__int64			Max;				// 최대 사용시간 카운터 Time.

	__int64			Call;				// 누적 호출 횟수.

} PROFILE_SAMPLE;
//extern int _profileMethodCount;					// 현재 카운팅되는 메소드 수
extern PROFILE_SAMPLE _profileSample[PROFILECOUNT];	// 프로파일 데이터를 저장할 수


// ============================================================================
//						프로파일링 시작!
// ---------------------------------------------------------------------------
// name으로 기록하고 있던 내역을 찾습니다.
//	없으면 _profileSample의 한곳에 정보를 구조체로 넣어 기록을 시작합니다.
// 
// 만약 이전 기록을 마무리 하기전에 다시 실행했다면 그 표본은 버립니다.
// StartTime이 0이 아니라면 마무리가 안된것 입니다.
// 
// 구조체에 StartTime (Tick)을 저장합니다.
// ============================================================================
void Profile_Begin(const WCHAR* name);


// ============================================================================
//						프로파일링 끝!
// ---------------------------------------------------------------------------
// name으로 기록하고 있던 내역을 찾습니다.
// 내역이 없으면 잘못된것 임으로 프로그램 크래쉬를 냅니다
// 구조체에 다음 정보를 기억합니다
// 1. endTime - StartTime을 구해 TotalTime에 누적합니다.
// 2. call한 횟수를 하나 증가시킵니다.
// 3. min, max값을 구해 저장합니다.
// 4. StartTime을 0으로 초기화 시킵니다.
// ============================================================================
void Profile_End(const WCHAR* name);


// ============================================================================
//						프로파일링 출력
// ---------------------------------------------------------------------------
// 1. 평균을 계산하기전 주파수를 얻기위해 QueryPerformanceFrequency(&freq)를 실행합니다.
// 2. _profileSample을 _profileMethodCount 만큼 순회하며 출력합니다
//	2.1 평균을 구하기 앞서 더정확한 값을 얻기위해 TotalTime에서 min, max값을 빼줍니다.
//  2.2 min, max값이 빠졌으니 call도 2회 빼줍니다
//  2.3 TotalTime / Call 로 평균을 구합니다
//  2.4 시간(Tick)을 주파수(freq)로 나누어 마이크로 세컨드 단위로 변환합니다.
//  2.5 형식에 맞게 출력합니다
// ============================================================================
void Profile_DataOutText(const CHAR* fileName);


// ============================================================================
//						프로파일링 리셋
// ---------------------------------------------------------------------------
// 프로파일중인 데이터를 저장하는 _profileSample의 전체를 순회합니다.
// 순회를 하며 모든 맴버변수의 값을 초기값으로 되돌립니다.
// ============================================================================
void Profile_Reset(void);

// ============================================================================
//						프로파일링 자동끝맺음
// ---------------------------------------------------------------------------
// class의 생성, 소멸자를 이용하여 함수를 이탈하면 자동으로 Profile_End를 실행하게 합니다.
// 직접 지정하는 것 보다 매우 느립니다.
// ============================================================================
class cProfiler {
private:
	const WCHAR* tag;
public:
	cProfiler(const WCHAR* tagName);
	~cProfiler();
	
};
//*/
#else
// 언재든 프로파일링을 끌 수 있게 매크로 함수로 호출합니다.
#define PRO_START(TagName)
#define PRO_BEGIN(TagName)
#define PRO_END(TagName)
#define PRO_PRINT(LogFile)
#endif
