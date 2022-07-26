#include "CMemory.h"
#include <stdlib.h>
#include <iostream>
CMemory::CMemory() {
	int size = 0;			// 블록사이즈
	int poolIndex = 0;		// 풀인덱스
	int tableIndex = 0;		// 풀테이블 인덱스

	// 1. size크기의 블럭을 담당하는 풀을 생성하고
	// 2. pools에 해당 포인터를 넣고
	// 3. O(1)접근을 위해 해당 영역에 해당하는 풀을넣는다
	// ~1024까지 32단위
	// 32바이트는 관리하기 위한 사이즈..
	for (size = 64; size <= 1024; size += 32) {
		CMemoryPool *pool = new CMemoryPool(size);
		_pools[poolIndex] = pool;
		poolIndex++;
		while (tableIndex <= size) {
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}
	// ~2048까지 128단위
	for (size = 1152; size <= 2048; size += 128) {
		CMemoryPool *pool = new CMemoryPool(size);
		_pools[poolIndex] = pool;
		poolIndex++;
		while (tableIndex <= size) {
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}
	// ~4096까지 256단위
	for (size = 2304; size <= 4096; size += 256) {
		CMemoryPool *pool = new CMemoryPool(size);
		_pools[poolIndex] = pool;
		poolIndex++;
		while (tableIndex <= size) {
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}
}

CMemory::~CMemory() {
	for (int i = 0; i < POOL_COUNT; i++)
		delete _pools[i];
}

void *CMemory::Alloc(int size) {
	int allocSize = size + NODE_SIZE; // 블록 + 헤더 + 테일(체크섬)
	void *pRet = nullptr;
	CMemoryPool::st_BLOCK_NODE *pNode;
	if (allocSize > MAX_ALLOC_SIZE) {
		// 그냥 할당
		pRet = _aligned_malloc(allocSize, 8);
		pRet = (CMemoryPool::st_BLOCK_NODE *) pRet + 1;
	} else {
		// 관리하는 테이블에서 꺼내오기
		pRet = _poolTable[allocSize]->Alloc();
	}
	pNode = (CMemoryPool::st_BLOCK_NODE *) pRet - 1;
	pNode->iBlockSize = allocSize; // 노드에 총 사이즈 정보 저장
	*((long long *) ((char *) pNode + allocSize - 8)) = 0; // 체크섬 0 으로!

	return pRet;
}

void CMemory::Free(void *ptr) {
	CMemoryPool::st_BLOCK_NODE *pNode = (CMemoryPool::st_BLOCK_NODE *) ptr - 1;

	long long allocSize = pNode->iBlockSize;
	long long checksum = *((long long *) ((char *) pNode + allocSize - 8));
	
	// 무결성 체크
	if (checksum != 0) {
		// 오버플로 났어요
		dfCRASH();
	}
	// TODO 언더플로

	if (allocSize > MAX_ALLOC_SIZE) {
		// 너무 큰 사이즈
		// 그냥 할당 해지
		_aligned_free(pNode);
	} else {
		// 해당 풀에 할당 해지
		_poolTable[allocSize]->Free(ptr);
	}
}

void CMemory::Monitoring(const char *FileName) {
	FILE *fp = stdout;
	
	// 뒤에 추가
	fopen_s(&fp, FileName, "a");
	if (fp == NULL) {
		fopen_s(&fp, FileName, "w");
	}
	fseek(fp, 0, SEEK_END);


	fprintf_s(fp, "\n==============================================================\n");
	fprintf_s(fp, "Monitoring Pools");
	fprintf_s(fp, "\n--------------------------------------------------------------\n");
	for (int i = 0; i < POOL_COUNT; i++) {
		fprintf_s(fp, "POOL[%4d] _iUseCount [%d] / _iCapacity[%d]\n",_pools[i]->GetBlockSize(),_pools[i]->GetUseCount(),_pools[i]->GetCapacityCount());
	}
	
	fprintf_s(fp, "\n==============================================================\n");
	fclose(fp);
}

CMemory g_memoryPool;