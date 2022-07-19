#pragma once
#include <Windows.h>
#include "CObjectPool_TLS.hpp"


template <typename T>
class Queue {
private:

	struct Node {
		T _data;
		Node *_prev;
		Node *_next;
	};

public:
	Queue();
	~Queue();

	void Enqueue(T data);
	bool Dequeue(T *data);
	DWORD Peek(T arr[], DWORD size);
	long GetSize() { return _size; }
	bool IsEmpty() { return _size == 0; }
	void Clear();
private:

	Node _head;        // 시작노드를 포인트한다.
	Node _tail;        // 마지막노드를 포인트한다.
	long _size;         // 큐 노드 카운트
	ObjectPool_TLS<Node> _nodePool;

	SRWLOCK _lock;
};

template<typename T>
inline Queue<T>::Queue()
{
	InitializeSRWLock(&_lock);
	_size = 0;


	_head._prev = nullptr;
	_head._next = &_tail;

	_tail._prev = &_head;
	_tail._next = nullptr;
}

template<typename T>
inline Queue<T>::~Queue()
{
	this->Clear();
}

template<typename T>
inline void Queue<T>::Enqueue(T data)
{
	AcquireSRWLockExclusive(&_lock);
	Node *pNode = (Node *) _nodePool.Alloc();
	pNode->_data = data;


	pNode->_next = &_tail;
	pNode->_prev = _tail._prev;
	_tail._prev = pNode;
	pNode->_prev->_next = pNode;

	++_size;

	ReleaseSRWLockExclusive(&_lock);
}

template<typename T>
inline bool Queue<T>::Dequeue(T *data)
{
	if (_head._next == &_tail) return false;
	AcquireSRWLockExclusive(&_lock);
	Node *pNode = _head._next;
	*data = pNode->_data;
	pNode->_prev->_next = pNode->_next;
	pNode->_next->_prev = pNode->_prev;

	--_size;

	_nodePool.Free(pNode);
	ReleaseSRWLockExclusive(&_lock);
	return true;
}

template<typename T>
inline DWORD Queue<T>::Peek(T arr[], DWORD size)
{
	int cnt = 0;
	Node *temp = _head._next;
	for (; cnt < size; ++cnt) {
		if (temp == &_tail) break;
		arr[cnt] = temp->_data;
		temp = temp->_next;
	}
	return cnt;
}

template<typename T>
inline void Queue<T>::Clear()
{
	AcquireSRWLockExclusive(&_lock);
	while (_head._next != &_tail)
	{
		Node *pNode = _head._next;

		pNode->_prev->_next = pNode->_next;
		pNode->_next->_prev = pNode->_prev;

		_nodePool.Free(pNode);
	}


	_size = 0;
	ReleaseSRWLockExclusive(&_lock);
}
