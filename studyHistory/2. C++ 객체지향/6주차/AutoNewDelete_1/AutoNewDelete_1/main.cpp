#include <iostream>
#include <cstdlib>
#include <cstring>
#include <list>
#define MEMORYALLOC(type, size) MemoryAlloc<type>( sizeof(type) * size, __LINE__, __FILE__)


struct MemAllocInfo {
	void* allocPtr;
	size_t size;
	int line;
	char filename[256];
};
int _mCount = 0;
int _totalSize = 0;
std::list <MemAllocInfo> _memAllockInfo;
template <typename T>
T* MemoryAlloc(size_t size, int line, const char* filename);
template <typename T>
void MemoryRelease(T* ptr);
void MemoryAllocPrint();
int main() {
	__int64 * p3 = MEMORYALLOC(__int64, 545);
	MemoryRelease(p3);
	MemoryAllocPrint();

	return 0;
}

template <typename T>
T* MemoryAlloc(size_t size, int line, const char* filename) {
	MemAllocInfo info;	// 할당정보 저장할 구조체
	T* buffer;			// 실제 할당 주소
	buffer = (T*)malloc(size); // size만크 메모리 할당
	// info에 정보넣기
	info.allocPtr = (void*)buffer;
	info.size = size;
	info.line = line;
	strcpy_s(info.filename, strlen(filename) + 1, filename);
	// 리스트 끝에 붙이기
	_memAllockInfo.push_back(info);

	// 토탈 구하기
	_totalSize += size;
	_mCount++;
	return buffer;
}
template <typename T>
void MemoryRelease(T* ptr) {
	for (auto itr = _memAllockInfo.begin(); itr != _memAllockInfo.end(); ++itr) {
		if (itr->allocPtr == ptr) {
			itr = _memAllockInfo.erase(itr);
			free(ptr);
			return;
		}

	}
}
void MemoryAllocPrint() {
	printf_s("Total Alloc Size : %d\n", _totalSize);
	printf_s("Total Alloc Count : %d\n", _mCount);
	for (auto itr = _memAllockInfo.begin(); itr != _memAllockInfo.end(); ++itr) {
		printf_s("Not Relase Memory : [0x%p] %d Bytes\n", itr->allocPtr, itr->size);
		printf_s("File : %s :: %d\n", itr->filename, itr->line);
	}
}
