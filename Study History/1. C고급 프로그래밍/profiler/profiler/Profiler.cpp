#include "Profiler.h"

#ifdef dfPROFILER
// ================================
// 멀티바이트 문자열 처리 함수
// 필요한것만 구현
// Strlen : 문자열 길이
// Strcpy : 문자열 복사
// Strcmp : 문자열 비교 (같으면 return 0)
// ================================
int Strlen(const WCHAR* str) {
	int len = 0;
	while (*(str + len) != '\0') {
		len++;
	}
	return len;
}
void Strcpy(WCHAR* strDest, const WCHAR* strSrc) {
	int len = 0;
	while (*(strSrc + len) != '\0') {
		*(strDest + len) = *(strSrc + len);
		len++;
	}
}
int Strcmp(const WCHAR* str1, const WCHAR* str2) {
	int len = Strlen(str1);
	for (int i = 0; i < len; i++) {
		if (*(str1 + i) != *(str2 + i))
			return *(str1 + i) - *(str2 + i);
	}
	return *(str1 + len) - *(str2 + len);
}


// ================================
// g_profileCount : 현재 프로파일링 하는 구간 카운트
// g_profileSample : 현재 프로파일링 하는 데이터 샘플
// ================================
int g_profileCount = 0;
PROFILE_SAMPLE g_profileSample[dfPROFILECOUNT];


// ================================
// ProfileBegin(const WCHAR* name)
// g_profileSample에서 name을 찾아 시작 시간을 저장합니다.
// 1. g_profileSample에서 name의 이름으로된 샘플이 있는지 확인합니다.
//	1-2. name샘플이 없다면 g_profileCount을 하나 증가시키고 그번호에 name을 등록합니다.
// 2. 현재 시간을 g_profileSample[name]에 기록합니다.
// ================================

void ProfileBegin(const WCHAR* name) {
	// 프로 파일 시작
	int index = -1;
	// 기록 하던것이 있나 확인
	for (int i = 0; i < dfPROFILECOUNT; i++) {
		if (Strcmp(name, g_profileSample[i].Name) == 0) {
			// 찾음!
			index = i;
		}
	}
	// 항목에 없음
	if (index < 0 && g_profileCount < dfPROFILECOUNT) {
		// 목록 추가
		index = g_profileCount;
		g_profileCount++;
		// 초기값 지정
		Strcpy(g_profileSample[index].Name, name);
		g_profileSample[index].StartTime.QuadPart = 0;
		g_profileSample[index].TotalTime = 0;
		g_profileSample[index].Max = MININT64;
		g_profileSample[index].Min = MAXINT64;
		g_profileSample[index].Call = 0;
	}
	// end를 실행하지 않고 begin했을때
	if (g_profileSample[index].StartTime.QuadPart != 0) {
		printf_s("\n\nPRO_END 가 실행되지 않음!\n\n");
		CRASH();
	}
	// 시작시간 기록
	QueryPerformanceCounter(&g_profileSample[index].StartTime);
}

// ================================
// ProfileEnd(const WCHAR* name)
// g_profileSample에서 name을 찾아 구간의 시간을 계산후 통계를 냅니다.
// 1. g_profileSample에서 name의 이름으로된 샘플이 있는지 확인합니다.
// 2. 시간의 차를 구합니다.
// 3. 계산한 정보를 저장합니다.
// ================================

void ProfileEnd(const WCHAR* name) {
	// 프로파일 시작
	int index = -1;
	for (int i = 0; i < dfPROFILECOUNT; i++) {
		if (Strcmp(name, g_profileSample[i].Name) == 0) {
			// 찾음!
			index = i;
		}
	}
	// 찾는 정보가 없음!
	if (index < 0) {
		printf_s("\n\nPROFILE 정보가 없음!\n\n");
		CRASH();
	}

	// 시간의 차
	LARGE_INTEGER endtime;
	QueryPerformanceCounter(&endtime);
	__int64 temp = endtime.QuadPart - g_profileSample[index].StartTime.QuadPart;	

	// 프로파일 정보 저장
	g_profileSample[index].TotalTime += temp;									// 누적하기
	g_profileSample[index].Call++;												// 콜한거 누적!
	if (g_profileSample[index].Max < temp) g_profileSample[index].Max = temp;		// max 구하기
	if (g_profileSample[index].Min > temp) g_profileSample[index].Min = temp;		// min 구하기
	g_profileSample[index].StartTime.QuadPart = 0;								// 제대로 end가 실행됨! (시작시간 0)
}


// ================================
// ProfileDataOutText(WCHAR* fileName)
// g_profileSample의 정보를 fileName에 출력합니다.
// 1. fileName이 있다면 해당파일의 끝에, 없다면 fileName을 생성합니다.
// 2. 출력시점의 평균을 구합니다
// 3. g_profileSample의 모든 정보를 출력합니다.
// ================================
void ProfileDataOutText(WCHAR* fileName) {
	// 출력하기
	FILE* fp = stdout;
	_wfopen_s(&fp, fileName, L"a");
	if (fp == NULL) {
		_wfopen_s(&fp, fileName, L"w");
		if (fp == NULL) {
			// 읽지도 생성도 못했으면 결함!
			CRASH();
		}
	} else {
		fseek(fp, 0, SEEK_END);
	}

	// 주파수 얻기
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	// 출력 부분
	wprintf_s(L"PRINT LOG : %s\n", fileName);
	fprintf_s(fp, "PROFILER:\n\
----------------------------------------------------------------------------------------------------\n\
            Name  |           Average  |               Min  |               Max  |             Call \n\
----------------------------------------------------------------------------------------------------\n");
	for (int i = 0; i < g_profileCount; i++) {
		// 평균 구하기 필요없는 최대 최소값은 뻄
		__int64 total = g_profileSample[i].TotalTime;
		total -= g_profileSample[i].Max;
		total -= g_profileSample[i].Min;
		DOUBLE avg = (DOUBLE)total / (DOUBLE)(g_profileSample[i].Call - 2) / (DOUBLE)freq.QuadPart * TICK_TO_MICSEC;

		// 출력
		fprintf_s(fp, "%17ls |%17.4llf\xC2\xB5s |%17.4llf\xC2\xB5s |%17.4llf\xC2\xB5s |%17lld\n",
			g_profileSample[i].Name,												// 이름
			avg,																// 평균
			g_profileSample[i].Min / (DOUBLE)freq.QuadPart * TICK_TO_MICSEC,		// 최소
			g_profileSample[i].Max / (DOUBLE)freq.QuadPart * TICK_TO_MICSEC,		// 최대
			g_profileSample[i].Call);											// 호출 횟수
	}
	fprintf_s(fp, "----------------------------------------------------------------------------------------------------\n");

	fclose(fp);
}


// ================================
// ProfileReset()
// g_profileSample의 정보를 모두 지웁니다.
// ================================

void ProfileReset(void) {
	// 구조체 배열 초기화
	for (int i = 0; i < dfPROFILECOUNT; i++) {
		*g_profileSample[i].Name = '\0';
		g_profileSample[i].StartTime.QuadPart = 0;
		g_profileSample[i].TotalTime = 0;
		g_profileSample[i].Max = MININT64;
		g_profileSample[i].Min = MAXINT64;
		g_profileSample[i].Call = 0;
	}
	g_profileCount = 0;
}

// ================================
// cProfiler
// 생성, 소멸자를 이용해 실수로 end를 호출하지 못해도 end를 처리합니다.
// ================================
cProfiler::cProfiler(const WCHAR* tagName) {
	tag = tagName;
	ProfileBegin(tag);
}
cProfiler ::~cProfiler() {
	ProfileEnd(tag);
}
#endif
