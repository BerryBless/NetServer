#pragma once
#include "CMemoryPool.h"


//class CMemoryPool;

// 메모리 매니져
class CMemory {
	enum {
		// ~1024까지 32단위, ~2048까지 128단위, ~4096까지 256단위 
		// 하나의 노드 관리에 필요한 공간이 32바이트이라서 첫번째 (32바이트짜리)풀은 못씀
		// 47개의 풀
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256) - 1,
		// 하나의 노드 관리에 필요한 공간 (헤더 + 오버플로 체크섬)
		NODE_SIZE = sizeof(CMemoryPool::st_BLOCK_NODE) + 8,
		// 관리할 수 있는 최대 크기 (한 블럭이 4096바이트 짜리 풀)
		MAX_ALLOC_SIZE = 4096
	};

public:
	CMemory();
	~CMemory();

	void *Alloc(int size);
	void	Free(void *ptr);

	void Monitoring(const char *FileName);

private :
	// 메모리 풀 카운트
	CMemoryPool *_pools[POOL_COUNT];
	CMemoryPool *_poolTable[MAX_ALLOC_SIZE + 1];
};

extern CMemory g_memoryPool;