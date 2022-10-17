#pragma once
#include "CObjectPool.hpp"
#define dfLOGGING_COUNT 100000

#define LOGIC_ENQ 0xAA
#define LOGIC_DEQ 0x11


template <typename T>
class LFQueue {
private:

	struct Node {
		T _data;
		Node *_next;
	};

	struct D_TOP {
		Node *_pNode = nullptr; // low
		LONG64 _counter = -1234;		// high
	};

	struct LOG_DATA {
		BYTE _logic;
		DWORD _tID;
		D_TOP _head;
		D_TOP _tail;
		D_TOP _snap_top;
		Node *_next;
		bool _isHead;
	};

	alignas(16) D_TOP _head;        // 시작노드를 포인트한다.
	alignas(16) D_TOP _tail;        // 마지막노드를 포인트한다.
	long _size;         // 큐 노드 카운트

	CObjectPool<Node> _nodePool;

	long _logcnt = 0;
	LOG_DATA _log[dfLOGGING_COUNT];
public:
	LFQueue();
	~LFQueue();

	void Enqueue(T data);
	bool Dequeue(T *data);
	long GetSize() { return _size; }
	bool IsEmpty() { return _size == 0; }

private:
	void MoveTail(BYTE logic, D_TOP *snap, Node **next);
	void Logging(BYTE logic, D_TOP snap_top, Node *next, bool isHead = false);
};


template<typename T>
inline LFQueue<T>::LFQueue() {
	//Node *dummy = new Node;
	Node* dummy = _nodePool.Alloc();

	dummy->_next = nullptr;
	_head._pNode = dummy;
	_head._counter = 0;
	_tail._pNode = dummy;
	_tail._counter = 0;
}

template<typename T>
inline LFQueue<T>::~LFQueue() {
	Node *node = _head._pNode;
	int cnt = 0;
	while (node != nullptr) {
		Node *t = node;
		node = node->_next;
		cnt++;
		//delete t;
		_nodePool.Free(t);
	}
}

template<typename T>
inline void LFQueue<T>::Enqueue(T data) {
	alignas(16) D_TOP top;
	//Node *node = new Node;
	Node* node = _nodePool.Alloc();
	Node *next;

	node->_data = data;
	node->_next = nullptr;

	Logging(LOGIC_ENQ, top, node);
	while (true) {
		top._counter = _tail._counter;
		top._pNode = _tail._pNode;
		next = top._pNode->_next;

		if (next == nullptr) {
			Node *snap_next = _tail._pNode;
			if (InterlockedCompareExchangePointer((PVOID *) &top._pNode->_next, node, nullptr) == nullptr) {
				if (snap_next == node) {
					int test = 0;
				}
				InterlockedIncrement((DWORD *) &_size);
				Logging(LOGIC_ENQ + 1, top, next);

				// 하나만 옮기고 끝.
				MoveTail(LOGIC_ENQ + 10, &top, &next);

				break;
			}
		}
		else {
			// 급한놈이 꼬리 옮기기
			MoveTail(LOGIC_ENQ + 20, &top, &next);
		}
	}
}

template<typename T>
inline bool LFQueue<T>::Dequeue(T *data) {
	alignas(16) D_TOP top;
	alignas(16) D_TOP tail;
	Node *next;
	Node *tail_next;
	T snap_data;

	InterlockedDecrement((DWORD *) &_size);

	if (_size < 0) {
		InterlockedIncrement((DWORD *) &_size);
		return false;
	}

	while (1) {
		top._counter = _head._counter;
		top._pNode= _head._pNode;
		next = top._pNode->_next;

		tail._counter = _tail._counter;
		tail._pNode = _tail._pNode;
		tail_next = tail._pNode->_next;

		if (next != nullptr) {
			if (top._pNode == tail._pNode) {
				MoveTail(LOGIC_DEQ + 20, &tail, &tail_next);
			} else {
				snap_data = next->_data;
				if (InterlockedCompareExchange128((LONG64 *) &_head, top._counter + 1, (LONG64) next, (LONG64 *) &top)) {
					Logging(LOGIC_DEQ + 50, top, next, true);
					break;
				}
			}
		}
	}

	*data = snap_data;

	//delete top._pNode;
	_nodePool.Free(top._pNode);
	return true;
}

template<typename T>
inline void LFQueue<T>::MoveTail(BYTE logic, D_TOP *snap, Node **next) {
	snap->_counter = _tail._counter;
	snap->_pNode = _tail._pNode;
	*next = snap->_pNode->_next;
	if (*next != nullptr) {
		if (InterlockedCompareExchange128((LONG64 *) &_tail, snap->_counter + 1, (LONG64) *next, (LONG64 *) snap) == 0) {
			// 커밋
			Logging(logic, *snap, nullptr);
		}
		else {
			// 실패
			Logging(logic, *snap, *next);
		}
	}

}

template<typename T>
inline void LFQueue<T>::Logging(BYTE logic, D_TOP snap_top, Node *next, bool isHead ) {
	long idx = InterlockedIncrement(&_logcnt);
	if (idx > dfLOGGING_COUNT) {
		_logcnt = 0;
		idx = 0;
	}
	_log[idx]._logic = logic;
	_log[idx]._tID = GetCurrentThreadId();
	_log[idx]._head = _head;
	_log[idx]._tail = _tail;
	_log[idx]._snap_top = snap_top;
	_log[idx]._next = next;
	_log[idx]._isHead = isHead;

}
