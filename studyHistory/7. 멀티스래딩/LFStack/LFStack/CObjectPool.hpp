#pragma once
#include <Windows.h>
#include <winnt.h>
#include <new.h>
#include "CCrashDump.h"
#include "CLogger.h"

#define CRASH() do{\
					CLogger::_Log(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
					int *nptr = nullptr; *nptr = 1;\
				}while(0)


//#define dfMAX_LOG_COUNT 10000

template <class DATA>
class CLF_ObjectPool {
	//////////////////////////////////////////////////////////////////////////
	// 체크섬
	// 유저영역을 가르키는 포인터의 앞 21비트를 안쓴다는것을 응용
	// 체크섬의 기초데이터(포인터)는 this의 포인터
	//////////////////////////////////////////////////////////////////////////
	enum : unsigned long long {
		FREE_CHECK = 0xFFDD0000'00000000,// checksum
		POINTER_MASK = 0x0000FFFF'FFFFFFFF,// this포인터를 저장할곳!
	};
private:
	struct BLOCK_NODE {
		DATA				_data;
		unsigned long long	_checksum;					// 체크섬
		BLOCK_NODE *_next;						// 다음노드
	};

	struct D_TOP {
		BLOCK_NODE *_pNode = nullptr; // low
		LONG64 _counter = 0;		// high
	};

private:
	alignas(16) D_TOP	_freeTop;									// 메모리풀 스택의 top	
	long			_capacity;									// 현재 확보된 블럭수
	long			_useCount;									// 현재 사용중인 블럭수
	bool			_isPlacementNew;							// 생성자 호출 유무 (true :호출)

#ifdef dfMAX_LOG_COUNT
private:
	// LOGGING
	struct LOG_DATA {
		WORD _idx;
		BYTE _logic;
		long			_capacity;									// 현재 확보된 블럭수
		long			_useCount;									// 현재 사용중인 블럭수
		BLOCK_NODE	_topNode;
	};
	LOG_DATA		_logData[dfMAX_LOG_COUNT];
	long			_logIndex;
private:
	void Logging(BYTE logic);
#endif // dfMAX_LOG_COUNT


private:
	//////////////////////////////////////////////////////////////////////////
	// 블럭 N개를 메모리에 할당 받는다.  
	//		Push()
	// 
	// Parameters: 할당받을 블럭 수
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void AllocMemory(int size);

public:

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 초기 블럭 개수.
	//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CLF_ObjectPool(int size = 0, bool isPlacementNew = false);

	virtual	~CLF_ObjectPool();


	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.  
	//		Pop(p)
	// 
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA *Alloc(void);


	//////////////////////////////////////////////////////////////////////////
	// 사용중이던 블럭을 해제한다.
	// 	   Push(p)
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
	int		GetCapacityCount(void) { return _capacity; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) { return _useCount; }
};

#ifdef dfMAX_LOG_COUNT
template<class DATA>
inline void CLF_ObjectPool<DATA>::Logging(BYTE logic) {
	long logIndex = InterlockedIncrement(&_logIndex);
	if (logIndex >= dfMAX_LOG_COUNT)_logIndex = 0; // 이때쯤 로그는 버림
	_logData[logIndex]._idx = logIndex;
	_logData[logIndex]._logic = logic;
	_logData[logIndex]._capacity = _capacity;
	_logData[logIndex]._useCount = _useCount;
	if (_freeTop._pNode == nullptr)
		ZeroMemory(_logData[logIndex]._topNode, sizeof(_logData[logIndex]._topNode));
	else
		_logData[logIndex]._topNode = *_freeTop._pNode;
}
#endif // dfMAX_LOG_COUNT

template<class DATA>
inline CLF_ObjectPool<DATA>::CLF_ObjectPool(int size, bool isPlacementNew) {
	_useCount = 0;
	_capacity = 0;
	_isPlacementNew = isPlacementNew;
	_freeTop._pNode = nullptr;
	_freeTop._counter = 0;
	AllocMemory(size);
}

template<class DATA>
inline CLF_ObjectPool<DATA>::~CLF_ObjectPool() {
	// 소멸자..
	BLOCK_NODE *pNode = _freeTop._pNode;
	BLOCK_NODE *pNext;
	while (pNode != nullptr) {
		pNext = pNode->_next;
		free(pNode);
		pNode = pNext;
	}
}

template<class DATA>
inline void CLF_ObjectPool<DATA>::AllocMemory(int size) {
	// push()

	BLOCK_NODE *oldTop;
	BLOCK_NODE *pNode;

	for (int i = 0; i < size; i++) {
		// 1. new pNode
		pNode = (BLOCK_NODE *) malloc(sizeof(BLOCK_NODE));

		//PlacementNew
		if (_isPlacementNew == false)
			new (&pNode->_data) (DATA);


		// 체크섬
		pNode->_checksum = (unsigned long long)this;
		pNode->_checksum ^= FREE_CHECK;


		do {

			// 2. pNode->next = top
			oldTop = _freeTop._pNode;
			pNode->_next = oldTop;


			// 3. (ATOMIC) top = pNode
		} while (InterlockedCompareExchangePointer((PVOID *) &_freeTop._pNode, pNode, oldTop) != oldTop);

		InterlockedIncrement(&_capacity);
	}
}

template<class DATA>
inline DATA *CLF_ObjectPool<DATA>::Alloc(void) {
	// pop()
	DATA *pRet = nullptr;


	alignas(16) D_TOP old_top;
	BLOCK_NODE *next = nullptr;
	do {

		// 1. old_top = top 
		old_top._counter = _freeTop._counter;
		old_top._pNode = _freeTop._pNode;
		if (old_top._pNode == nullptr) {
			AllocMemory(1);
			continue;
		}
		// 2. next = old_top->next
		next = old_top._pNode->_next;

		// 3. (ATOMIC) top = next
	} while (InterlockedCompareExchange128((LONG64 *) &_freeTop, old_top._counter + 1, (LONG64) next, (LONG64 *) &old_top) == 0);

	// checksum
	old_top._pNode->_checksum ^= FREE_CHECK;
	// 4. out = old_top->data
	pRet = (DATA *) old_top._pNode;

	//PlacementNew
	if (_isPlacementNew)
		new (pRet) (DATA);

	InterlockedIncrement(&_useCount);
	return pRet;
}

template<class DATA>
inline bool CLF_ObjectPool<DATA>::Free(DATA *pData) {
	// push()
	// 1. new pNode
	BLOCK_NODE *oldTop;
	BLOCK_NODE *pNode = (BLOCK_NODE *) pData;

	// 체크
	if (pNode->_checksum & FREE_CHECK) {
		// 2번 freed?
		CRASH();
	}
	if ((pNode->_checksum & POINTER_MASK) != (unsigned long long) this) {
		// 다른 풀에서 들어온 놈 또는 오버플로
		CRASH();
	}

	pNode->_checksum ^= FREE_CHECK;

	do {

		// 2. pNode->next = top
		oldTop = _freeTop._pNode;
		pNode->_next = oldTop;

		// 3. (ATOMIC) top = pNode
	} while (InterlockedCompareExchangePointer((PVOID *) &_freeTop._pNode, pNode, oldTop) != oldTop);


	// PlacementNew
	if (_isPlacementNew)
		pData->~DATA();

	InterlockedDecrement(&_useCount);

	return true;
}
