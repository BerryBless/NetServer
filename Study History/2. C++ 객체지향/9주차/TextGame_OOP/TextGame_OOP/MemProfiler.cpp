#include "MemProfiler.h"
#include <iostream>
#include <cstring>
#include <ctime>
#undef new

#pragma region MemProfiler

// 싱글톤

MemProfiler& MemProfiler::Instance() {
	static MemProfiler instance;
	return instance;
}

MemProfiler::MemProfiler() {
	// 싱글톤 생성자
}
MemProfiler::~MemProfiler() {
	// 싱글톤 소멸자
	this->_list.PrintInfo();
}
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
	_list.Push_back(info);
}

#pragma region List


MemProfiler::InfoList::InfoList() {
	// 리스트 초기화
	_head = nullptr;
	_count = 0;
}
void MemProfiler::InfoList::Push_back(MemAllocInfo info) {
	// 리스트 끝에 추가하기
	Node* newNode = new Node;	// 새 노드 할당
	// 새 노드에 정보넣기
	newNode->Info = info;		
	newNode->Tail = nullptr;
	// 처음부터면
	if (_head == nullptr) {
		_head = newNode;
	}
	else {
		// 꼬리에 달기
		Node* cur = _head;
		while (cur->Tail != nullptr) {
			cur = cur->Tail;
		}
		cur->Tail = newNode;
	}
	_count++;
}

MemProfiler::AllocState MemProfiler::InfoList::TryDelete(void* ptr) {
	if (_head == nullptr) { printf_s("\nMemProfiler::InfoList::TryDelete :: Head == nullptr\n"); return AllocState::ERROR; }
	// 끝까지 순회하며
	Node* itr = _head;
	while (itr != nullptr) {
		// 오류 내용에 맞게 처리
		if (itr->Info.AllocPtr == ptr) {
			if (itr->Info.State == AllocState::Free) {
				// 이미 초기화된걸 다시 초시화 시도할때 에러
				// TODO delete한 파일과 라인 가져오기
				itr->Info.State = AllocState::Freed;
				return AllocState::Freed;
			}
			// 정상적으로 들어옴
			itr->Info.State = AllocState::Free;
			return AllocState::Free;
		}
		else if (itr->Info.AllocPtr == (int*)ptr - 1) {
			// new[] 로 호출하고 delete 로 지울때
			itr->Info.State = AllocState::Array;
			return AllocState::Array;
		}
		else if (itr->Info.AllocPtr == (int*)ptr + 1) {
			// new 로 호출하고 delete[] 로 지울때
			itr->Info.State = AllocState::nArray;
			return AllocState::nArray;
		}
		itr = itr->Tail;
	}
	// 애초에 이상한거 들어옴
	return AllocState::ERROR;
}
// 얼록정보 출력
void MemProfiler::InfoList::PrintInfo() {
	if (_head == nullptr) { printf_s("\MemProfiler::InfoList::PrintInfo :: Head == nullptr\n"); return; }

	FILE* fp = stdout;
	//* 파일 입출력
	// Alloc_YYYYMMDD_HHMMSS.log
	char filename[26];	// 저장할파일 이름

	// 시간구하기
	time_t now = time(0);
	struct tm tstruct;

	localtime_s(&tstruct, &now);
	strftime(filename, sizeof(filename), "Alloc_%Y%m%e_%H%M%S.log", &tstruct); // (Alloc_20210828_173840.log)

	// 파일이 있으면 뒤에 추가
	fopen_s(&fp, filename, "a");
	if (fp == NULL) {
		//CRASH
		int *p = nullptr;
		*p = 10;
		return;
	}
	fseek(fp, 0, SEEK_END);
	//*/
	Node* itr = _head;
	while (itr != nullptr) {
		switch (itr->Info.State)
		{
		case AllocState::Alloc:		// delete 호출하지 않음!
			fprintf_s(fp, "LEAK    [0x%p] [%6Iu] %s : %d\n", itr->Info.AllocPtr, itr->Info.Size, itr->Info.File, itr->Info.Line);
			break;
		case AllocState::Free:		// 정상작동!
			fprintf_s(fp, "NOALLOC [0x%p]\n", itr->Info.AllocPtr);
			break;
		case AllocState::Freed:		// delete 2번이상 호출!
			// TODO 에러메시지
			fprintf_s(fp, "Freed   [0x%p] [%6Iu] %s : %d\n", itr->Info.AllocPtr, itr->Info.Size, itr->Info.File, itr->Info.Line);
			break;
		case AllocState::Array:		// new[] 로 호출하고 delete 로 지울때
			fprintf_s(fp, "ARRAY   [0x%p] [%6Iu] %s : %d\n", itr->Info.AllocPtr, itr->Info.Size, itr->Info.File, itr->Info.Line);
			break;
		case AllocState::nArray:	// new 로 호출하고 delete[] 로 지울때
			fprintf_s(fp, "nArray  [0x%p] [%6Iu] %s : %d\n", itr->Info.AllocPtr, itr->Info.Size, itr->Info.File, itr->Info.Line);
			break;

		default:
			break;
		}
		itr = itr->Tail;
	}
	fclose(fp);
}
#pragma endregion
// 얼록정보 빼기
void MemProfiler::FreeInfo(void* aPtr) {
	AllocState state = _list.TryDelete(aPtr);
	if (state != AllocState::Free) {
		// TODO  오류수정
		Instance()._list.PrintInfo();
	}
}


#pragma endregion


#pragma region new overloading

// 동적할당

// const 안붙이면 에러 (2019)
void* operator new (size_t size, const char* file, int line) {
	void* p = malloc(size);

	MemProfiler::Instance().AllocInfo(p, size, file, line);
	return p;
}

void operator delete (void* p, const char* file, int line) {}
void operator delete (void* p) {
	MemProfiler::Instance().FreeInfo(p);
	free(p);
}


// 배열로 할당
void* operator new[](size_t size, const char* file, int line) {
	void* p = malloc(size);
	MemProfiler::Instance().AllocInfo(p, size, file, line);
	return p;
}

void operator delete[](void* p, const char* file, int line) {}
void operator delete[](void* p) {
	MemProfiler::Instance().FreeInfo(p);
	free(p);
}



#pragma endregion