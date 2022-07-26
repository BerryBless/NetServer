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
#define dfPROFILECOUNT 10		// 프로파일링할 샘플의 최대 숫자

// ================================
// 매크로함수로 사용
// 프로파일링을 끄고 싶을때는 #define dfPROFILER 주석처리 하기
// ================================
#define PRO_START(TagName)	cProfiler profile (TagName)		// 클래스의 생성 / 소멸을 이용한 프로파일링 (느림)
#define PRO_BEGIN(TagName)	ProfileBegin(TagName)			// 프로파일링을 시작할 곳의 TagName을 지정합니다.
#define PRO_END(TagName)	ProfileEnd(TagName)				// 프로파일링을 끝낼 곳의 TagName을 지정합니다.
#define PRO_PRINT(LogFile)	ProfileDataOutText(LogFile)		// 프로파일링 데이터를 LogFile로 출력합니다.


// ================================
// 프로파일링 샘플 구조체
// ================================
struct PROFILE_SAMPLE {
	WCHAR			Name[64];			// 프로파일 샘플 이름.

	LARGE_INTEGER	StartTime;			// 프로파일 샘플 실행 시간.

	__int64			TotalTime;			// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
	__int64			Min;				// 최소 사용시간 카운터 Time.	
	__int64			Max;				// 최대 사용시간 카운터 Time.

	__int64			Call;				// 누적 호출 횟수.

} ;

// ================================
// ProfileBegin : name의 시작시간을 체크합니다
// ProfileEnd : name의 끝시간을 체크하고, 걸린시간을 계산하여 통계를 냅니다.
// ProfileDataOutText : fileName으로 파일출력을 합니다.
// ProfileReset : 프로파일링데이터를 초기화 합니다.
// ================================

void ProfileBegin(const WCHAR* name);
void ProfileEnd(const WCHAR* name);
void ProfileDataOutText(char* fileName);
void ProfileReset(void);
//* 생성자, 소멸자를 이용해서 자동으로 끝냄 (느림)
class cProfiler {
private:
	const WCHAR* tag;
public:
	cProfiler(const WCHAR* tagName);
	~cProfiler();
	
};
//*/
#else
// ================================
// 프로파일링 작동 안함
// ================================
#define PROFILER(TagName)	// 프로파일링 꺼짐, 프로파일링을 키고싶다면 dfPROFILER를 정의하세요!
#define PRO_BEGIN(TagName)	// 프로파일링 꺼짐, 프로파일링을 키고싶다면 dfPROFILER를 정의하세요!
#define PRO_END(TagName)	// 프로파일링 꺼짐, 프로파일링을 키고싶다면 dfPROFILER를 정의하세요!
#define PRO_PRINT(LogFile)	// 프로파일링 꺼짐, 프로파일링을 키고싶다면 dfPROFILER를 정의하세요!
#endif
