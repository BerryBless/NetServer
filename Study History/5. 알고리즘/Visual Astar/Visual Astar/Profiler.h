#pragma once
#include <stdio.h>
#include <Windows.h>

#define CHRASH() int* p=nullptr; *p=0;

#define MIN(a,b) a>b?b:a
#define MAX(a,b) a>b?a:b

#define PROFILE

int Strlen(const WCHAR* str);
void Strcpy(WCHAR* strDest, const WCHAR* strSrc);
int Strcmp(const WCHAR* str1, const WCHAR* str2);

#ifdef PROFILE
#define TICK_TO_MICSEC 1000000	
#define PROFILECOUNT 10

// 매크로함수로 사용
#define PRO_START(TagName)	cProfiler profile (TagName)
#define PRO_BEGIN(TagName)	ProfileBegin(TagName)
#define PRO_END(TagName)	ProfileEnd(TagName)
#define PRO_PRINT(LogFile)	ProfileDataOutText(LogFile)

typedef struct {
	WCHAR			Name[64];			// 프로파일 샘플 이름.

	LARGE_INTEGER	StartTime;			// 프로파일 샘플 실행 시간.

	__int64			TotalTime;			// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
	__int64			Min;				// 최소 사용시간 카운터 Time.	
	__int64			Max;				// 최대 사용시간 카운터 Time.

	__int64			Call;				// 누적 호출 횟수.

} PROFILE_SAMPLE;
extern int _profileMethodCount;
extern PROFILE_SAMPLE _profileSample[PROFILECOUNT];


void ProfileBegin(const WCHAR* name);
void ProfileEnd(const WCHAR* name);
void ProfileDataOutText(const CHAR* fileName);
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
#define PROFILER(TagName)
#define PRO_BEGIN(TagName)
#define PRO_END(TagName)
#define PRO_PRINT(LogFile)
#endif
