#include "Profiler.h"


#pragma region StringFunc

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
#pragma endregion

#ifdef PROFILE
int _profileMethodCount = 0;					// 현재 카운팅되는 메소드 수
PROFILE_SAMPLE _profileSample[PROFILECOUNT];	// 프로파일 데이터를 저장할 수
#define CRASH() int* ptr = nullptr; *ptr = 1



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
void Profile_Begin(const WCHAR* name) {
	// 프로 파일 시작
	int index = -1;
	// 기록 하던것이 있나 확인
	for (int i = 0; i < PROFILECOUNT; i++) {
		if (Strcmp(name, _profileSample[i].Name) == 0) {
			// 찾음!
			index = i;
		}
	}
	// 항목에 없음
	if (index < 0 && _profileMethodCount < PROFILECOUNT) {
		// 목록 추가
		index = _profileMethodCount;
		_profileMethodCount++;
		// 초기값 지정
		Strcpy(_profileSample[index].Name, name);
		_profileSample[index].StartTime.QuadPart = 0;
		_profileSample[index].TotalTime = 0;
		_profileSample[index].Max = MININT64;
		_profileSample[index].Min = MAXINT64;
		_profileSample[index].Call = 0;
	}
	// end를 실행하지 않고 begin했을때
	if (_profileSample[index].StartTime.QuadPart != 0) {
		printf_s("\n\nPRO_END 가 실행되지 않음!\n\n");
		CRASH();
	}
	// 시작시간 기록
	QueryPerformanceCounter(&_profileSample[index].StartTime);
}

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
void Profile_End(const WCHAR* name) {
	// 프로파일 시작
	int index = -1;
	for (int i = 0; i < PROFILECOUNT; i++) {
		if (Strcmp(name, _profileSample[i].Name) == 0) {
			// 찾음!
			index = i;
		}
	}
	// 찾는 정보가 없음!
	if (index < 0) {
		printf_s("\n\nPROFILE 정보가 없음!\n\n");
		CRASH();
	}
	// 프로파일 정보 저장
	LARGE_INTEGER endtime;
	QueryPerformanceCounter(&endtime);
	__int64 temp = endtime.QuadPart - _profileSample[index].StartTime.QuadPart;	// 시간의 차
	_profileSample[index].TotalTime += temp;									// 누적하기
	_profileSample[index].Call++;												// 콜한거 누적!
	_profileSample[index].Max = max(_profileSample[index].Max, temp);			// min 구하기
	_profileSample[index].Min = min(_profileSample[index].Min, temp);			// max 구하기
	_profileSample[index].StartTime.QuadPart = 0;								// 제대로 end가 실행됨!
}


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
void Profile_DataOutText(const CHAR* fileName) {
	// 출력하기
	FILE* pOutputFile = stdout;
	fopen_s(&pOutputFile, fileName,"w");
	if (pOutputFile == NULL) {
		return;
	}
	// 주파수 얻기
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	// 출력 부분
	printf_s("PRINT LOG : %s\n", fileName);
	fprintf_s(pOutputFile, "----------------------------------------------------------------------------------------------------\n");
	fprintf_s(pOutputFile, "            Name  |           Average  |               Min  |               Max  |             Call \n");
	fprintf_s(pOutputFile, "----------------------------------------------------------------------------------------------------\n");
	for (int i = 0; i < _profileMethodCount; i++) {
		// 평균 구하기 필요없는 최대 최소값은 뻄
		__int64 total = _profileSample[i].TotalTime;
		total -= _profileSample[i].Max;
		total -= _profileSample[i].Min;
		DOUBLE avg = (DOUBLE)total / (DOUBLE)(_profileSample[i].Call - 2) / (DOUBLE)freq.QuadPart * TICK_TO_MICSEC;

		// 출력
		fprintf_s(pOutputFile, "%17ls |%17.4llf\xC2\xB5s |%17.4llf\xC2\xB5s |%17.4llf\xC2\xB5s |%17lld\n",
			_profileSample[i].Name,												// 이름
			avg,																// 평균
			_profileSample[i].Min / (DOUBLE)freq.QuadPart * TICK_TO_MICSEC,		// 최소
			_profileSample[i].Max / (DOUBLE)freq.QuadPart * TICK_TO_MICSEC,		// 최대
			_profileSample[i].Call);											// 호출 횟수
	}
	fprintf_s(pOutputFile, "----------------------------------------------------------------------------------------------------\n");

	fclose(pOutputFile);
}

// ============================================================================
//						프로파일링 리셋
// ---------------------------------------------------------------------------
// 프로파일중인 데이터를 저장하는 _profileSample의 전체를 순회합니다.
// 순회를 하며 모든 맴버변수의 값을 초기값으로 되돌립니다.
// ============================================================================
void Profile_Reset(void) {
	// 구조체 배열 초기화
	for (int i = 0; i < PROFILECOUNT; i++) {
		*_profileSample[i].Name = '\0';
		_profileSample[i].StartTime.QuadPart = 0;
		_profileSample[i].TotalTime = 0;
		_profileSample[i].Max = MININT64;
		_profileSample[i].Min = MAXINT64;
		_profileSample[i].Call = 0;
	}
	_profileMethodCount = 0;
}


// ============================================================================
//						프로파일링 자동끝맺음
// ---------------------------------------------------------------------------
// class의 생성, 소멸자를 이용하여 함수를 이탈하면 자동으로 Profile_End를 실행하게 합니다.
// 직접 지정하는 것 보다 매우 느립니다.
// ============================================================================
cProfiler::cProfiler(const WCHAR* tagName) {
	tag = tagName;
	Profile_Begin(tag);
}
cProfiler ::~cProfiler() {
	Profile_End(tag);
}
#endif
