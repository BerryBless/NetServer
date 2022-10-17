#include "Profiler.h"

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

#ifdef PROFILE
int _profileMethodCount = 0;
PROFILE_SAMPLE _profileSample[PROFILECOUNT];
void ProfileBegin(const WCHAR* name) {
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
		CHRASH();
	}
	// 시작시간 기록
	QueryPerformanceCounter(&_profileSample[index].StartTime);
}
void ProfileEnd(const WCHAR* name) {
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
		CHRASH();
	}
	// 프로파일 정보 저장
	LARGE_INTEGER endtime;
	QueryPerformanceCounter(&endtime);
	__int64 temp = endtime.QuadPart - _profileSample[index].StartTime.QuadPart;	// 시간의 차
	_profileSample[index].TotalTime += temp;									// 누적하기
	_profileSample[index].Call++;												// 콜한거 누적!
	_profileSample[index].Max = MAX(_profileSample[index].Max, temp);			// min 구하기
	_profileSample[index].Min = MIN(_profileSample[index].Min, temp);			// max 구하기
	_profileSample[index].StartTime.QuadPart = 0;								// 제대로 end가 실행됨!
}
void ProfileDataOutText(const CHAR* fileName) {
	// 출력하기
	FILE* pOutputFile = stdout;
	fopen_s(&pOutputFile, fileName,"w");

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
void ProfileReset(void) {
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
// 생성자, 소멸자를 이용해서 자동으로 끝냄 (느림)
cProfiler::cProfiler(const WCHAR* tagName) {
	tag = tagName;
	ProfileBegin(tag);
}
cProfiler ::~cProfiler() {
	ProfileEnd(tag);
}
#endif
