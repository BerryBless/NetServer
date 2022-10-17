#include "Profiler.h"

#ifdef dfPROFILER

SRWLOCK CProfiler::_printLock;
DWORD CProfiler::_tlsIdx;
CProfiler CProfiler::_profiler[dfPROFILE_THREAD_COUNT];
int CProfiler::_profilerIdx = 0;

int CProfiler::TagToIndex(const wchar_t *tag) {
	for (int i = 0; i < dfPROFILE_SAMPLE_COUNT; i++) {
		if (_samples[i].isCheack == false) return i; // 찾지못함, 인덱스가 비어있음
		if (wcscmp(_samples[i].Tag, tag) == 0) return i; // 찾음
	}
	return -1; // 인덱스가 꽉참, 새로은 태그
}

CProfiler::CProfiler() {
	//SetThreadId();
	ProfileReset();
}

CProfiler::~CProfiler() {
	// 이게맞나?
}

void CProfiler::SetThreadId() {
	_threadID = GetCurrentThreadId();
}

void CProfiler::ProfileBegin(const wchar_t *tag) {
	// 프로 파일 시작
	int index = TagToIndex(tag);

	// 항목에 없음
	if (_samples[index].isCheack == false) {
		// 초기화
		_samples[index].isCheack = true;
		wcscat_s(_samples[index].Tag, sizeof(_samples[index].Tag), tag);

		_samples[index].Call = 0;
	}
	// end를 실행하지 않고 begin했을때
	if (_samples[index].StartTime.QuadPart != 0) {
		CRASH();
	}
	// 시작시간 기록
	QueryPerformanceCounter(&_samples[index].StartTime);
}

void CProfiler::ProfileEnd(const wchar_t *tag) {
	// 프로파일 시작
	int index = TagToIndex(tag);

	// 찾는 정보가 없음!
	if (_samples[index].isCheack == false || index < 0) {
		printf_s("\n\nPROFILE 정보가 없음!\n\n");
		CRASH();
	}

	// 시간의 차
	LARGE_INTEGER endtime;
	QueryPerformanceCounter(&endtime);
	__int64 time = endtime.QuadPart - _samples[index].StartTime.QuadPart;

	// 프로파일 정보 저장
	_samples[index].TotalTime += time;									// 누적하기
	_samples[index].Call++;												// 콜한거 누적!

	// MIN / MAX 처리
	if (_samples[index].Max[1] < time) _samples[index].Max[1] = time;		// max 구하기
	if (_samples[index].Min[1] > time) _samples[index].Min[1] = time;		// min 구하기
	if (_samples[index].Max[0] < _samples[index].Max[1]) {
		__int64 temp = _samples[index].Max[1];
		_samples[index].Max[1] = _samples[index].Max[0];
		_samples[index].Max[0] = temp;
	}
	if (_samples[index].Min[0] > _samples[index].Min[1]) {
		__int64 temp = _samples[index].Min[1];
		_samples[index].Min[1] = _samples[index].Min[0];
		_samples[index].Min[0] = temp;
	}

	// 완료
	_samples[index].StartTime.QuadPart = 0;								// 제대로 end가 실행됨! (시작시간 0)
}

void CProfiler::ProfilePrintFile(const wchar_t *filename) {
	AcquireSRWLockExclusive(&_printLock);
	// 출력하기
	FILE *fp = stdout;
	_wfopen_s(&fp, filename, L"a+");
	fseek(fp, 0, SEEK_END);

	// 주파수 얻기
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	// 출력 부분
	wprintf_s(L"PRINT LOG : %s\n", filename);
	fprintf_s(fp, "PROFILER: ThreadID[%d]\n\
----------------------------------------------------------------------------------------------------\n\
            Name  |           Average  |               Min  |               Max  |             Call \n\
----------------------------------------------------------------------------------------------------\n", _threadID);
	for (int i = 0; i < dfPROFILE_SAMPLE_COUNT; i++) {
		if (_samples[i].isCheack == false) break;
		// 평균 구하기 필요없는 최대 최소값은 뻄
		__int64 total = _samples[i].TotalTime;
		total -= _samples[i].Max[0];
		total -= _samples[i].Min[0];
		DOUBLE avg = (DOUBLE) total / (DOUBLE) (_samples[i].Call - 2) / (DOUBLE) freq.QuadPart * TICK_TO_MICSEC;

		// 출력
		fprintf_s(fp, "%17ls |%17.4llf\xC2\xB5s |%17.4llf\xC2\xB5s |%17.4llf\xC2\xB5s |%17lld\n",
			_samples[i].Tag,												// 이름
			avg,																// 평균
			_samples[i].Min[0] / (DOUBLE) freq.QuadPart * TICK_TO_MICSEC,		// 최소
			_samples[i].Max[0] / (DOUBLE) freq.QuadPart * TICK_TO_MICSEC,		// 최대
			_samples[i].Call);											// 호출 횟수
	}
	fprintf_s(fp, "----------------------------------------------------------------------------------------------------\n");

	fclose(fp);
	ReleaseSRWLockExclusive(&_printLock);
}

void CProfiler::ProfileReset() {
	memset(_samples, 0, sizeof(_samples));
	for (int i = 0; i < dfPROFILE_SAMPLE_COUNT; i++) {
		_samples[i].Min[0] = MAXINT64;
		_samples[i].Min[1] = MAXINT64;
	}
}
// TLS
void CProfiler::InitProfiler() {
	_tlsIdx = TlsAlloc();
	InitializeSRWLock(&_printLock);

}



void CProfiler::Reset() {
	CProfiler *pProfiler = (CProfiler *) TlsGetValue(_tlsIdx);
	if (pProfiler == nullptr) {
		CRASH();
	}
	pProfiler->ProfileReset();
}

void CProfiler::Begin(const wchar_t *tag) {
	CProfiler *pProfiler = (CProfiler *) TlsGetValue(_tlsIdx);
	if (pProfiler == nullptr) {
		// 할당?
		pProfiler = &_profiler[_profilerIdx];
		_profilerIdx++;
		TlsSetValue(_tlsIdx, pProfiler);

		pProfiler->SetThreadId();
	}
	pProfiler->ProfileBegin(tag);
}
void CProfiler::End(const wchar_t *tag) {
	CProfiler *pProfiler = (CProfiler *) TlsGetValue(_tlsIdx);
	if (pProfiler == nullptr) {
		CRASH();
	}
	pProfiler->ProfileEnd(tag);
}
void CProfiler::Print(const wchar_t *filename) {
	for (int i = 0; i < _profilerIdx; i++) {
		_profiler[i].ProfilePrintFile(filename);
	}
}
#endif
