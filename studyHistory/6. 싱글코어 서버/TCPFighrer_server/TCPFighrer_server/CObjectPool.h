#pragma once
#include <new.h>
#include <stdlib.h>
/*---------------------------------------------------------------

	procademy MemoryPool.

	메모리 풀 클래스 (오브젝트 풀 / 프리리스트)
	특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.

	- 사용법.

	procademy::CObjectPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData 사용

	MemPool.Free(pData);


----------------------------------------------------------------*/

// 일부로 크래쉬내기
#define dfCRASH() do{int *p = nullptr; *p = 10;}while(0)

template <class DATA>
class CObjectPool {
	//////////////////////////////////////////////////////////////////////////
	// 체크섬
	// 유저영역을 가르키는 포인터의 앞 21비트를 안쓴다는것을 응용
	// 체크섬의 기초데이터(포인터)는 this의 포인터
	//////////////////////////////////////////////////////////////////////////
	enum : unsigned long long {
		//USEDMASK	= 0x80000000'00000000,// 이 오브젝트가 메모리 풀 안에 있는가?
		POINTERMASK = 0x0000FFFF'FFFFFFFF,// this포인터를 저장할곳!
	};
private:
	//////////////////////////////////////////////////////////////////////////
	// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
	//
	// ----------------- <- _pFreeNode가 의미하는 주소
	// | st_BLOCK_NODE |
	// ----------------- <- Alloc() 이 리턴하는 주소
	// |               |	이 부분은 그냥 주소로 관리
	// | <class DATA>  |	노드에 있는것이 아님,
	// |               |	노드 포인터에 노드사이즈 더하면 나오는 주소
	// |               |
	// |               |
	// -----------------
	// 
	// 체크섬의 앞1바이트의 데이터가 
	// - USEDMASK다 -> 이 오브젝트는 오브젝트 풀 안에 있음 
	// - 00 이다 -> 사용중
	// - XOR(^)연산으로 토글하면서 사용
	//////////////////////////////////////////////////////////////////////////
	struct st_BLOCK_NODE {
		alignas(8) unsigned long long	CHECKSUM;					// 체크섬
		alignas(8) st_BLOCK_NODE		*pNEXT;						// 다음노드
	};
private:
	st_BLOCK_NODE	*_pFreeNode;								// 메모리풀 스택의 top
	int				_iCapacity;									// 현재 확보된 블럭수
	int				_iUseCount;									// 현재 사용중인 블럭수
	int				_iBlockSize;								// DATA + NODE의 사이즈
	bool			_bPlacementNew;								// 생성자 호출 유무 (true :호출)
public:

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 초기 블럭 개수.
	//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CObjectPool(int iBlockNum, bool bPlacementNew = false);

	virtual	~CObjectPool();


	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.  
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA *Alloc(void);

	//////////////////////////////////////////////////////////////////////////
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters: (DATA *) 블럭 포인터.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool	Free(DATA *pData);

	//////////////////////////////////////////////////////////////////////////
	// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	//
	// Parameters: 없음.
	// Return: (int) 메모리 풀 내부 전체 개수
	//////////////////////////////////////////////////////////////////////////
	int		GetCapacityCount(void) { return _iCapacity; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) { return _iCapacity; }
};

template<class DATA>
inline CObjectPool<DATA>::CObjectPool(int iBlockNum, bool bPlacementNew) {
	// 초기값 넣기
	_iCapacity = iBlockNum;									// 확보된 블럭수
	_iUseCount = 0;											// 사용중인 블록수
	_bPlacementNew = bPlacementNew;							// 생성자를 호출하냐?
	_iBlockSize = sizeof(DATA) + sizeof(st_BLOCK_NODE);		// 블럭 사이즈 계산
	// 메모리 확보 시도
	_pFreeNode = nullptr;
	for (int i = 0; i < iBlockNum; i++) {
		// 노드 한개
		st_BLOCK_NODE *pNode = (st_BLOCK_NODE *) malloc(_iBlockSize);
		pNode->pNEXT = _pFreeNode;									// 현재 top과 연결
		pNode->CHECKSUM = (unsigned long long)this;
		//pNode->CHECKSUM ^= USEDMASK;								// 할당 되어있지 않다!
		_pFreeNode = pNode;									// top를 맨앞의 블록으로!
		if (_bPlacementNew == true) {
			// placement new
			new (pNode + 1) DATA();
		}
	}
}

template<class DATA>
inline CObjectPool<DATA>::~CObjectPool() {
	//  가능하면 모든블럭 free해주기
	st_BLOCK_NODE *pNODE;
	// 노드 하나하나 프리
	while (_pFreeNode != nullptr) {
		pNODE = _pFreeNode;
		_pFreeNode = _pFreeNode->pNEXT;
		free(pNODE);
	}
}

template<class DATA>
inline DATA *CObjectPool<DATA>::Alloc(void) {
	// 남은공간 여유가 없으면
	if (_pFreeNode == nullptr) {							// 공간이 부족할때
		//////////////////////////////////////////////////////////////////////////
		// 맨 마지막 블록은 기존 _pFreeNode과 연결이 된다
		// 첫번째 블록은 새로운 _pFreeNode가 된다
		// 
		// -----------------  --->  -----------------  --->  -----------------  ---> .... 
		// | st_BLOCK_NODE |        | st_BLOCK_NODE |        | st_BLOCK_NODE |
		// -----------------        -----------------        ----------------- 
		// |               |        |               |        |               |
		// | <class DATA>  |        | <class DATA>  |        | <class DATA>  |
		// |               |        |               |        |               |
		// |               |        |               |        |               |
		// |               |        |               |        |               |
		// -----------------        -----------------        ----------------- 
		//
		//////////////////////////////////////////////////////////////////////////
		// 노드 한개
		st_BLOCK_NODE *pNode = (st_BLOCK_NODE *) malloc(_iBlockSize);
		pNode->pNEXT = _pFreeNode;									// 현재 top과 연결
		pNode->CHECKSUM = (unsigned long long)this;
		//pNode->CHECKSUM ^= USEDMASK;								// 할당 되어있지 않다!
		_pFreeNode = pNode;											// top를 맨앞의 블록으로!
		if (_bPlacementNew == true) {
			// placement new
			new (pNode + 1) DATA();
		}
		++_iCapacity;
	}

	// 스택의 Pop()	
	DATA *pRet = (DATA *) (_pFreeNode + 1);						// FreeNode 있는곳 할당해주기 
	// 스택에서 나갔다를 표시
	//_pFreeNode->CHECKSUM ^= USEDMASK;							// // 할당 해지한 데이터 토글(OFF)
	_pFreeNode = _pFreeNode->pNEXT;								// 스택 탑 빼기

	++_iUseCount;												// 하나 들어감

	// 생성자 호출
	if (_bPlacementNew == true) {
		// placement new
		new (pRet) DATA();
	}


	return pRet;
}

template<class DATA>
inline bool CObjectPool<DATA>::Free(DATA *pData) {
	if (pData == nullptr) return false;


	// 노드정보 확인
	// 이 풀의 범위를 체크해서 정말로 "이 풀의 메모리"가 맞는지 확인
	st_BLOCK_NODE *pNode = ((st_BLOCK_NODE *) pData - 1);			// 노드 사이즈만큼 앞으로!

	//if (pNode->CHECKSUM & USEDMASK) {
	//	// 이미 할당 해지된 데이터
	//	dfCRASH();
	//	return false;
	//}
	if ((pNode->CHECKSUM & POINTERMASK) != (unsigned long long) this) {
		// 다른 메모리풀에서 들어온 데이터
		dfCRASH();
	}

	// 소멸자 호출
	if (_bPlacementNew == true) {
		pData->~DATA();
	}
	// 스택의 Push()
	pNode->pNEXT = _pFreeNode;									// top는 다음노드를 가르키게
	_pFreeNode = pNode;											// 지금 top!

	// 스텍이 들어왔다를 표시
	//pNode->CHECKSUM ^= USEDMASK;								// 할당 해지한 데이터 토글(ON)

	--_iUseCount;												// 하나 빠짐

	return true;
}
