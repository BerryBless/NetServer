#pragma once

#include <Windows.h>
#include "CObjectPool_TLS.hpp"

template <typename T>
class Stack {
private:
	struct NODE {
		T		_data;
		NODE *_next;
	};

public:
	Stack();
	~Stack();

	bool isEmpty() { return _pTop == nullptr; }
	long GetSize() { return _size; }

	void Push(T data);
	bool Pop(T *out);
	void Clear();

private:
	NODE *_pTop;
	long _size;

	ObjectPool_TLS<NODE> _nodePool;

	SRWLOCK _lock;
};

template<typename T>
inline Stack<T>::Stack()
{
	InitializeSRWLock(&_lock);
	_size = 0;
	_pTop = nullptr;
}

template<typename T>
inline Stack<T>::~Stack()
{
	this->Clear();
}

template<typename T>
inline void Stack<T>::Push(T data)
{
	AcquireSRWLockExclusive(&_lock);
	NODE *pNode = _nodePool.Alloc();
	pNode->_data = data;
	pNode->_next = _pTop;
	_pTop = pNode;
	++_size;
	ReleaseSRWLockExclusive(&_lock);
}

template<typename T>
inline bool Stack<T>::Pop(T *out)
{
	AcquireSRWLockExclusive(&_lock);
	NODE *tempTop = _pTop;
	*out = _pTop->_data;
	_pTop = _pTop->_next;
	_nodePool.Free(tempTop);
	--_size;
	ReleaseSRWLockExclusive(&_lock);
	return true;
}

template<typename T>
inline void Stack<T>::Clear()
{
	AcquireSRWLockExclusive(&_lock);
	while (_pTop != nullptr) {
		NODE *pTemp = _pTop;
		_pTop = _pTop->_next;
		_nodePool.Free(pTemp);
	}
	_size = 0;
	ReleaseSRWLockExclusive(&_lock);
}
