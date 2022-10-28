#pragma once
#include <Windows.h>
#include <winnt.h>
#include <new.h>
#include "CCrashDump.h"
#include "CLogger.h"

#define CRASH() do{int*ptr =nullptr; *ptr =100;}while(0)



//#define dfMAX_LOG_COUNT 10000

template <class DATA>
class CLF_ObjectPool {
private:
	struct BLOCK_NODE {
		DATA				_data;
		CLF_ObjectPool<DATA> *_checksum;		// 체크섬
		BLOCK_NODE *_next;						// 다음노드
		DWORD _useFlag = false;
	};

	struct D_TOP {
		BLOCK_NODE *_pNode = nullptr; // low
		LONG64 _counter = 0;		// high
	};

private:
	alignas(16) D_TOP	_freeTop;									// 메모리풀 스택의 top	
	long				_capacity;									// 현재 확보된 블럭수
	long				_useCount;									// 현재 사용중인 블럭수
	bool				_isPlacementNew;							// 생성자 호출 유무 (true :호출)

public:

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 초기 블럭 개수.
	//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CLF_ObjectPool(int size = 0, bool isPlacementNew = false) {
		_useCount = 0;
		_capacity = 0;
		_isPlacementNew = isPlacementNew;
		_freeTop._pNode = nullptr;
		_freeTop._counter = 0;
		AllocMemory();
	}

	virtual	~CLF_ObjectPool() {
		// 소멸자..
		BLOCK_NODE *pNode = _freeTop._pNode;
		BLOCK_NODE *pNext;
		while (pNode != nullptr) {
			pNext = pNode->_next;
			free(pNode);
			pNode = pNext;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.  
	//		Pop(p)
	// 
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA *Alloc(void) {
		// pop()
		DATA *pRet = nullptr;
		BLOCK_NODE *next = nullptr;
		alignas(16) D_TOP old_top;
		do {

			// 1. old_top = top 
			old_top._counter = _freeTop._counter;
			old_top._pNode = _freeTop._pNode;
			if (old_top._pNode == nullptr) {
				pRet = AllocMemory();
				return pRet;
			}
			// 2. next = old_top->next
			next = old_top._pNode->_next;

			// 3. (ATOMIC) top = next
		} while (InterlockedCompareExchange128((LONG64 *) &_freeTop, old_top._counter + 1, (LONG64) next, (LONG64 *) &old_top) == 0);


		// 4. out = old_top->data
		pRet = (DATA *) old_top._pNode;


		//PlacementNew
		if (_isPlacementNew)
			new (pRet) DATA();

		// checksum
		((BLOCK_NODE *) pRet)->_useFlag++;
		InterlockedIncrement(&_useCount);

		return pRet;
	}


	//////////////////////////////////////////////////////////////////////////
	// 사용중이던 블럭을 해제한다.
	// 	   Push(p)
	//
	// Parameters: (DATA *) 블럭 포인터.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool	Free(DATA *pData) {
		// push()
		// 1. new pNode
		BLOCK_NODE *oldTop;
		BLOCK_NODE *pNode = (BLOCK_NODE *) pData;

		// 체크
		if (pNode->_useFlag <= 0) {
			// 2번 free
			CRASH();
		}
		if (pNode->_checksum != this) {
			// 다른 풀에서 들어온 놈 또는 오버플로
			CRASH();
		}
		// PlacementNew
		if (_isPlacementNew)
			pData->~DATA();

		pNode->_useFlag--;

		do {

			// 2. pNode->next = top
			oldTop = _freeTop._pNode;
			pNode->_next = oldTop;

			// 3. (ATOMIC) top = pNode
		} while (InterlockedCompareExchangePointer((PVOID *) &_freeTop._pNode, pNode, oldTop) != oldTop);


		InterlockedDecrement(&_useCount);

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	//
	// Parameters: 없음.
	// Return: (int) 메모리 풀 내부 전체 개수
	//////////////////////////////////////////////////////////////////////////
	int		GetCapacity(void) { return _capacity; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int		GetSize(void) { return _useCount; }
private:
	//////////////////////////////////////////////////////////////////////////
	// 블럭 N개를 메모리에 할당 받는다.  
	//		Push()
	// 
	// Parameters: 할당받을 블럭 수
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	DATA *AllocMemory() {
		// push()

		BLOCK_NODE *oldTop;
		BLOCK_NODE *pNode;



		InterlockedIncrement(&_capacity);

		pNode = (BLOCK_NODE *) malloc(sizeof(BLOCK_NODE));

		// 체크섬
		pNode->_next = nullptr;
		pNode->_checksum = this;
		pNode->_useFlag = 0;

		//PlacementNew
		if (_isPlacementNew == false)
			new ((DATA *) pNode) DATA();

		// 바로 사용
		pNode->_useFlag++;
		InterlockedIncrement(&_useCount);

		return (DATA *) pNode;
	}
};

