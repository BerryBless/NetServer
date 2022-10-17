
#include "MemProfiler.h"
#ifdef MEMPROFILER

#include <iostream>
#include <cstring>
#include <ctime>
#undef new

#pragma region MemProfiler

#pragma region Singleton

MemProfiler::MemProfiler() {
	// 싱글톤 생성자
}
MemProfiler::~MemProfiler() {
	_list.clear();
}
#pragma endregion
// 얼록 정보 넣기!
void MemProfiler::AllocInfo(void* aPtr, size_t size, const char* file, int line) {
	MemAllocInfo info;	// MemAllocInfo 구조체
	// 구조체에 정보 넣기
	info.AllocPtr = aPtr;
	info.Size = size;
	strcpy_s(info.File, strlen(file) + 1, file);
	info.Line = line;
	info.State = AllocState::Alloc;
	// 구조체를 리스트에 달기
	_list.push_back(info);
}


// 얼록정보 출력
void MemProfiler::PrintInfo() {
	if (_list.empty()) { 
		printf_s("\MemProfiler::PrintInfo :: _list.empty()\n"); return; }

	FILE* fp = stdout;
	//* 파일 입출력
	// Alloc_YYYYMMDD_HHMMSS.log
	char filename[30];	// 저장할파일 이름

	// 시간구하기
	time_t now = time(0);
	struct tm tstruct;

	localtime_s(&tstruct, &now);
	strftime(filename, sizeof(filename), "log/Alloc_%Y%m%e_%H%M%S.log", &tstruct); // (Alloc_20210828_173840.log)

	// 파일이 있으면 뒤에 추가
	fopen_s(&fp, filename, "a");
	fseek(fp, 0, SEEK_END);
	//*/
	for (auto iter = _list.begin(); iter != _list.end(); ++iter) {
		switch ((*iter).State)
		{
		case AllocState::Alloc:		// delete 호출하지 않음!
			fprintf_s(fp, "LEAK    [0x%p] [%6Iu] %s : %d\n", (*iter).AllocPtr, (*iter).Size, (*iter).File, (*iter).Line);
			break;
		case AllocState::Free:		// 정상작동!
			fprintf_s(fp, "NOALLOC [0x%p]\n", (*iter).AllocPtr);
			break;
		case AllocState::Freed:		// delete 2번이상 호출!
			// TODO 에러메시지
			fprintf_s(fp, "Freed   [0x%p] [%6Iu] %s : %d\n", (*iter).AllocPtr, (*iter).Size, (*iter).File, (*iter).Line);
			break;
		case AllocState::Array:		// new[] 로 호출하고 delete 로 지울때
			fprintf_s(fp, "ARRAY   [0x%p] [%6Iu] %s : %d\n", (*iter).AllocPtr, (*iter).Size, (*iter).File, (*iter).Line);
			break;
		case AllocState::nArray:	// new 로 호출하고 delete[] 로 지울때
			fprintf_s(fp, "nArray  [0x%p] [%6Iu] %s : %d\n", (*iter).AllocPtr, (*iter).Size, (*iter).File, (*iter).Line);
			break;

		default:
			break;
		}
	}
	fclose(fp);
}
// 얼록정보 빼기
void MemProfiler::FreeInfo(void *ptr) {
	if (_list.empty() || ptr == NULL) { return; }
	// 끝까지 순회하며
	for (auto iter = _list.begin(); iter != _list.end(); ++iter) {
		// 오류 내용에 맞게 처리
		if ((*iter).AllocPtr == ptr) {
			if ((*iter).State == AllocState::Free) {
				// 이미 초기화된걸 다시 초시화 시도할때 에러
				// TODO delete한 파일과 라인 가져오기
				(*iter).State = AllocState::Freed;
			}
			// 정상적으로 들어옴
			(*iter).State = AllocState::Free;
		} else if ((*iter).AllocPtr == (int *) ptr - 1) {
			// new[] 로 호출하고 delete 로 지울때
			(*iter).State = AllocState::Array;
		} else if ((*iter).AllocPtr == (int *) ptr + 1) {
			// new 로 호출하고 delete[] 로 지울때
			(*iter).State = AllocState::nArray;
		}
	}
}

#pragma endregion


#pragma region new overloading

// 동적할당

// const 안붙이면 에러 (2019)
void* operator new (size_t size, const char* file, int line) {
	void* p = malloc(size);

	I_MEMPROFILER->AllocInfo(p, size, file, line);
	return p;
}

void operator delete (void* p, const char* file, int line) {}
void operator delete (void* p) {
	I_MEMPROFILER->FreeInfo(p);
	free(p);
}


// 배열로 할당
void* operator new[](size_t size, const char* file, int line) {
	void* p = malloc(size);
	I_MEMPROFILER->AllocInfo(p, size, file, line);
	return p;
}

void operator delete[](void* p, const char* file, int line) {}
void operator delete[](void* p) {
	I_MEMPROFILER->FreeInfo(p);
	free(p);
}



#pragma endregion

#endif // MEMPROFILER
