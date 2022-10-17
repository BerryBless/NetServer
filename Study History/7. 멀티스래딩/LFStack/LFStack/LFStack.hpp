#pragma once
#include <Windows.h>
#include <winnt.h>
#include "CObjectPool.hpp"

#define dfOBJECTPOOL 
#define dfMAX_LOGING_COUNT 10000 // 최대 로깅카운트
//#define dfLOGGING_STACK_CPY // 로깅할때 그당시 스택을 탑부터 20개 카피
//#define dfLOGGING_LOCK // 로깅할 락


template <typename T>
class LFStack {
private:
	struct NODE {
		T		_data;
		NODE *_next;
	};

	
	struct D_TOP {
		NODE *_pNode = nullptr; // low
		LONG64 _counter =0;		// high
	};


	struct LOG_DATA {
		int _logcnt;		// 로그 ID
		BYTE _logic;		// 로그 발생 로직
		DWORD _tID;			// 실행 스레드 ID
		long _size;			// 로깅할 스택 사이즈
		D_TOP _top;		// _top
		D_TOP _pold_top;		// old_top
		void *_pTarget;		// push : new node | pop : next node
#ifdef dfLOGGING_STACK_CPY
		// 그 당시 스택의 복사본
		// 대략적인 느낌으로 20개만
		// 로깅사이에 스택을 바꾸는 걸 막지못함
		void *_thatTop[20];
		NODE _thatStack[20];
#endif // dfLOGGING_STACK_CPY
	};
	void Logging(BYTE logic, D_TOP top, D_TOP old_top, NODE *target);
public:
	LFStack();
	~LFStack();

	bool isEmpty() { return _top._pNode == nullptr; }
	long GetSize() { return _size; }
	long GetPoolSize() { return _nodePool.GetCapacityCount(); }
	long GetPoolUse() { return _nodePool.GetUseCount(); }

	void Push(T data);
	bool Pop(T *out);
private:

	alignas(16) D_TOP _top;
	alignas(16) long _size;

#ifdef dfOBJECTPOOL
	CLF_ObjectPool <NODE> _nodePool;
#endif // dfOBJECTPOOL

	// DEBUG
	long		_logCount = 0;
	LOG_DATA	_logData[dfMAX_LOGING_COUNT];
#ifdef dfLOGGING_LOCK
	SRWLOCK _lock;
#endif // dfLOGGING_LOCK
};


template<typename T>
inline void LFStack<T>::Logging(BYTE logic, D_TOP top, D_TOP old_top, NODE *target) {
#ifdef dfLOGGING_LOCK
	AcquireSRWLockExclusive(&_lock);
#endif // dfLOGGING_LOCK
	int logIndex;

	// LOGGING
	//logCount = InterlockedIncrement(&_logCount);
	//logIndex = logCount % dfMAX_LOGING_COUNT;
	//_logData[logIndex]._logcnt = logCount;
	logIndex = InterlockedIncrement(&_logCount);
	if (logIndex >= dfMAX_LOGING_COUNT) _logCount = 0;// 이때 로그는 버림
	_logData[logIndex]._logcnt = _logCount;
	_logData[logIndex]._logic = logic;
	_logData[logIndex]._tID = GetCurrentThreadId();
	_logData[logIndex]._size = _size;
	_logData[logIndex]._top = top;
	_logData[logIndex]._pold_top = old_top;
	_logData[logIndex]._pTarget = target;

#ifdef dfLOGGING_STACK_CPY
	ZeroMemory(_logData[logIndex]._thatTop, sizeof(_logData[logIndex]._thatTop));
	ZeroMemory(_logData[logIndex]._thatStack, sizeof(_logData[logIndex]._thatStack));
	NODE *lTop = top._pNode;
	int i = 0;
	while (lTop != nullptr && i < 20) {
		_logData[logIndex]._thatTop[i] = lTop;
		_logData[logIndex]._thatStack[i] = *lTop;
		lTop = lTop->_next;
		i++;
	}
	//if (i >= 20) CRASH();
#endif // dfLOGGING_STACK_CPY
#ifdef dfLOGGING_LOCK
	ReleaseSRWLockExclusive(&_lock);
#endif // dfLOGGING_LOCK
}

template<typename T>
inline LFStack<T>::LFStack() {
	_size = 0;
#ifdef dfLOGGING_LOCK
	InitializeSRWLock(&_lock);
#endif // dfLOGGING_LOCK
}

template<typename T>
inline LFStack<T>::~LFStack() {
	T temp;
	while (_top._pNode->_next != nullptr) {
		Pop(&temp);
	}
}

template<typename T>
inline void LFStack<T>::Push(T data) {
	DWORD tID = GetCurrentThreadId();

	// push
	// 1. new pNode
	// 2. pNode->next = top
	// 3. top = pNode

	// 3번에서 경합이 발생하면 2번부터

	// 1. new pNode
	alignas(16) D_TOP old_top ;// 디버깅용
	old_top._counter = -1;

	NODE *tempTop;
#ifdef dfOBJECTPOOL
	NODE *pNode = _nodePool.Alloc();
#else
	NODE *pNode = new NODE;
#endif // dfOBJECTPOOL
	pNode->_data = data;

	do {

		Logging('1', _top, old_top, pNode);


		// 2. pNode->next = top
		tempTop = _top._pNode;
		old_top._pNode = tempTop; // 디버깅용
		pNode->_next = tempTop;

		Logging('2', _top, old_top, pNode);

		// 3. (ATOMIC) top = pNode
	} while (InterlockedCompareExchangePointer((PVOID *) &_top._pNode, pNode, tempTop) != tempTop);
	Logging('3', _top, old_top, pNode);
	InterlockedIncrement(&_size);
}

template<typename T>
inline bool LFStack<T>::Pop(T *out) {
	if (out == nullptr) return false;

	alignas(16) D_TOP old_top;
	NODE *next = nullptr;
	do {
		Logging('A', _top, old_top, next);

		// 1. old_top = top 
		old_top._counter = _top._counter;
		old_top._pNode = _top._pNode;
		if (old_top._pNode == nullptr) {
			out = nullptr;
			return false;
		}
		// 2. next = old_top->next
		next = old_top._pNode->_next;
		Logging('B', _top, old_top, next);

		// 3. (ATOMIC) top = next
	} while (InterlockedCompareExchange128((LONG64 *) &_top, old_top._counter + 1, (LONG64 ) next, (LONG64 *) &old_top) == 0);

	Logging('C', _top, old_top, next);

	// 4. out = old_top->data
	*out = old_top._pNode->_data;
	// 5. delete old_top
#ifdef dfOBJECTPOOL
	_nodePool.Free(old_top._pNode);
#else
	delete old_top._pNode;
#endif // dfOBJECTPOOL
	InterlockedDecrement(&_size);

	return true;
}
