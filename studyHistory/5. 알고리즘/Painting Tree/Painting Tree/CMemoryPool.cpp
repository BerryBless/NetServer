#include <new.h>
#include <stdlib.h>
#include "CMemoryPool.h"

CMemoryPool::CMemoryPool(int iBlockSize) {
	// 초기값 넣기
	_iBlockSize = iBlockSize;
	_iCapacity = 0;												// 확보된 블럭수
	_iUseCount = 0;												// 사용중인 블록수
	_pFreeNode = nullptr;										// 따로 확보 안함(프리리스트)

}

CMemoryPool::~CMemoryPool() {
	// 가능하면 모든블럭 free해주기
	st_BLOCK_NODE *pNODE;
	// 노드 하나하나 프리
	while (_pFreeNode != nullptr) {
		pNODE = _pFreeNode;
		_pFreeNode = _pFreeNode->pNEXT;
		_aligned_free(pNODE);
	}
}

void *CMemoryPool::Alloc(void) {

	// 남은공간 여유가 없으면
	if (_pFreeNode == nullptr) {								// 공간이 부족할때
	//////////////////////////////////////////////////////////////////////////
	// 맨 마지막 블록은 기존 _pFreeNode과 연결이 된다
	// 첫번째 블록은 새로운 _pFreeNode가 된다
	// 
	// -----------------  --->  -----------------  --->  -----------------  ---> .... 
	// | st_BLOCK_NODE |        | st_BLOCK_NODE |        | st_BLOCK_NODE |
	// -----------------        -----------------        ----------------- 
	// |               |        |               |        |               |
	// | alloc memory  |        | alloc memory  |        | alloc memory  |
	// |               |        |               |        |               |
	// |               |        |               |        |               |
	// |               |        |               |        |               |
	// -----------------        -----------------        ----------------- 
	//////////////////////////////////////////////////////////////////////////

	// 노드 한개
		st_BLOCK_NODE *pNode = (st_BLOCK_NODE *) _aligned_malloc(_iBlockSize, 8); // 8의 경계 명확하게!
		pNode->pNEXT = _pFreeNode;									// 현재 top과 연결
		pNode->CHECKSUM = (unsigned long long)this;					// 이 풀의 노드라고 표시
		_pFreeNode = pNode;
		_iCapacity += 1;
	}

	// 스택의 Pop()	
	void *pRet = (void *) (_pFreeNode + 1);						// FreeNode 있는곳 할당해주기 
	// 스택에서 나갔다를 표시
	_pFreeNode->CHECKSUM ^= USEDMASK;							// 할당 해지한 데이터 토글(OFF)
	_pFreeNode = _pFreeNode->pNEXT;								// 스택 탑 빼기
	++_iUseCount;												// 하나 들어감

	return pRet;
}

bool CMemoryPool::Free(void *pData) {
	if (pData == nullptr) return false;

	// 이 풀의 범위를 체크해서 정말로 "이 풀의 메모리"가 맞는지 확인
	// 노드정보 확인
	st_BLOCK_NODE *pNode = ((st_BLOCK_NODE *) pData - 1);		// 노드 사이즈만큼 앞으로!
	//*
	if (pNode->CHECKSUM & USEDMASK == 0) {
		// 이미 할당 해지된 데이터
		dfCRASH();
		return false;
	}//*/
	if ((pNode->CHECKSUM & POINTERMASK) != (unsigned long long) this) {
		// 다른 메모리풀에서 들어온 데이터
		dfCRASH();
	}

	// 스택의 Push()
	pNode->pNEXT = _pFreeNode;									// top는 다음노드를 가르키게
	_pFreeNode = pNode;											// 지금 top!

	// 스텍이 들어왔다를 표시
	pNode->CHECKSUM ^= USEDMASK;								// 할당 해지한 데이터 토글(ON)
	--_iUseCount;												// 하나 빠짐

	return true;
}
