#pragma once
#include <Windows.h>
#include <winnt.h>

#define dfMAX_LOGING_COUNT 10000 // 최대 로깅카운트
#define dfLOGGING_STACK_CPY // 로깅할때 그당시 스택을 탑부터 20개 카피
//#define dfLOGGING_LOCK // 로깅할 락

#define CRASH() do{\
		int *nptr = nullptr; *nptr = 1;\
	}while(0)


template <typename T>
class LFStack {
private:
	struct NODE {
		T		_data;
		NODE *_next;
	};

	/*struct DELETE_WAIT {
		void *_delPtr;
		DELETE_WAIT *_next;
	};*/



	struct LOG_DATA {
		int _logcnt;		// 로그 ID
		BYTE _logic;		// 로그 발생 로직
		DWORD _tID;			// 실행 스레드 ID
		long _size;			// 로깅할 스택 사이즈
		long _callCheck;	// 실행 스레드 ID
		long _pushRef;		// 로깅할때 push 참조 카운터
		long _popRef;		// 로깅할때 pop 참조 카운터
		void *_pTop;		// _top
		void *_pOldTop;		// oldTop
		void *_pTarget;		// push : new node | pop : next node
#ifdef dfLOGGING_STACK_CPY
		// 그 당시 스택의 복사본
		// 대략적인 느낌으로 20개만
		// 로깅사이에 스택을 바꾸는 걸 막지못함
		void *_thatTop[20];
		NODE _thatStack[20];
#endif // dfLOGGING_STACK_CPY
	};

	bool CAS(NODE **dest, NODE *exch, NODE *comp);
	void Logging(BYTE logic, NODE *top, NODE *oldTop, NODE *target, long callcheck);
public:
	LFStack();
	~LFStack();

	bool isEmpty() { return _top == nullptr; }
	long GetSize() { return _size; }

	void Push(T data);
	bool Pop(T *out);
private:

	NODE *_top;


	long _size;
	
	long _pushRef;
	long _popRef;
	long _callChecker = 0;

	// DEBUG
	long		_logCount = 0;
	LOG_DATA	_logData[dfMAX_LOGING_COUNT];
#ifdef dfLOGGING_LOCK
	SRWLOCK _lock;
#endif // dfLOGGING_LOCK
};

template<typename T>
inline bool LFStack<T>::CAS(NODE **dest, NODE *exch, NODE *comp) {
	/*
	retval = dest
	if (dest == comp)
		dest = exch;
	retrun retval == comp*/
	return InterlockedCompareExchangePointer((PVOID *) dest, (PVOID) exch, (PVOID) comp) == comp;
}

template<typename T>
inline void LFStack<T>::Logging(BYTE logic, NODE *top, NODE *oldTop, NODE *target, long callcheck) {
#ifdef dfLOGGING_LOCK
	AcquireSRWLockExclusive(&_lock);
#endif // dfLOGGING_LOCK
	int logCount;
	int logIndex;

	// LOGGING
	logCount = InterlockedIncrement(&_logCount);
	logIndex = logCount % dfMAX_LOGING_COUNT;
	_logData[logIndex]._logcnt = logCount;
	_logData[logIndex]._logic = logic;
	_logData[logIndex]._tID = GetCurrentThreadId();
	_logData[logIndex]._callCheck = callcheck;
	_logData[logIndex]._size = _size;
	_logData[logIndex]._pushRef = _pushRef;
	_logData[logIndex]._popRef = _popRef;
	_logData[logIndex]._pTop = top;
	_logData[logIndex]._pOldTop = oldTop;
	_logData[logIndex]._pTarget = target;

#ifdef dfLOGGING_STACK_CPY
	ZeroMemory(_logData[logIndex]._thatTop, sizeof(_logData[logIndex]._thatTop));
	ZeroMemory(_logData[logIndex]._thatStack, sizeof(_logData[logIndex]._thatStack));
	int i = 0;
	while (top != nullptr && i < 20) {
		_logData[logIndex]._thatTop[i] = top;
		_logData[logIndex]._thatStack[i] = *top;
		top = top->_next;
		i++;
	}
	if (i >= 20) CRASH();
#endif // dfLOGGING_STACK_CPY
#ifdef dfLOGGING_LOCK
	ReleaseSRWLockExclusive(&_lock);
#endif // dfLOGGING_LOCK
}

template<typename T>
inline LFStack<T>::LFStack() {
	_top = nullptr;
	_size = 0;
	_pushRef = 0;
	_popRef = 0;
#ifdef dfLOGGING_LOCK
	InitializeSRWLock(&_lock);
#endif // dfLOGGING_LOCK
}

template<typename T>
inline LFStack<T>::~LFStack() {
	T temp;
	while (_top->_next != nullptr) {
		Pop(&temp);
	}
}

template<typename T>
inline void LFStack<T>::Push(T data) {
	InterlockedIncrement(&_pushRef);
	long call = InterlockedIncrement(&_callChecker);
	int logCount;
	int logIndex;
	DWORD tID = GetCurrentThreadId();

	// push
	// 1. new pNode
	// 2. pNode->next = top
	// 3. top = pNode

	// 3번에서 경합이 발생하면 2번부터

	// 1. new pNode
	NODE *oldTop = nullptr;
	NODE *pNode = new NODE;

	pNode->_data = data;
	pNode->_next = nullptr;
	do {

		Logging('1', _top, oldTop, pNode, call);


		// 2. pNode->next = top
		oldTop = _top;
		pNode->_next = oldTop;

		Logging('2', _top, oldTop, pNode, call);

		// 3. (ATOMIC) top = pNode
		// if( top == oldtop ){
		//		top = pNode;
		//		return true;
		// } else {
		//		return false;
		// }
	} while (InterlockedCompareExchangePointer((PVOID *) &_top, pNode, oldTop) != oldTop);
	Logging('3', _top, oldTop, pNode, call);

	InterlockedIncrement(&_size);
	InterlockedDecrement(&_pushRef);

	Logging('4', _top, oldTop, pNode, call);
}

template<typename T>
inline bool LFStack<T>::Pop(T *out) {
	if (out == nullptr) return false;
	InterlockedIncrement(&_popRef);
	long call = InterlockedIncrement(&_callChecker);
	int logCount;
	int logIndex;
	DWORD tID = GetCurrentThreadId();

	// pop
	// 1. oldtop = top 
	// 2. next = oldtop->next
	// 3. top = next
	// 4. out = oldtop->data
	// 5. delete oldtop

	NODE *oldTop = nullptr;
	NODE *next = nullptr;

	do {
		Logging('A', _top, oldTop, next, call);

		// 1. oldtop = top 
		oldTop = _top;
		if (oldTop == nullptr) {

			Logging('Z', _top, oldTop, next, call);
			CRASH();
			return false;
		}

		// 2. next = oldtop->next
		next = oldTop->_next;
		Logging('B', _top, oldTop, next, call);


		// 3. (ATOMIC) top = next
	} while (InterlockedCompareExchangePointer((PVOID *) &_top, next, oldTop) != oldTop);

	Logging('C', _top, oldTop, next, call);

	// 4. out = oldtop->data
	*out = oldTop->_data;
	// 5. delete oldtop
	delete oldTop;

	InterlockedDecrement(&_popRef);
	InterlockedDecrement(&_size);
	Logging('D', _top, nullptr, next, call);

	return true;
}
