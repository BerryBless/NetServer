#pragma once

#include "CList.h"

template <typename T>
class CQueue {
private:
	CList<T> _queue;
public:
	CQueue() {}
	~CQueue(){}

public:
	void push(T data) {
		_queue.push_back(data);
	}
	void pop() {
		_queue.pop_front();
	}
	T front() {
		return *(_queue.begin());
	}
	void clear() {
		_queue.clear();
	}
	bool empty() {
		return _queue.empty();
	}
};

