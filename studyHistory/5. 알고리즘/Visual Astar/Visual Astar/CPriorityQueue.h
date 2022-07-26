#pragma once
//#include <vector>
#include "Global.h"
/*--------------------------------------------
*
* Compare(a,b)
*  return a < b ? 1 : -1;
* 
struct stPQNode {
	int F;
	int G;
	int X;
	int Y;

	int CompareTo(stPQNode other) {
		if (F == other.F)
			return 0;
		return F < other.F ? 1 : -1;
	}
};
--------------------------------------------*/

struct stPQNODE {
	int F;
	int G;
	int X;
	int Y;

	int CompareTo(stPQNODE other) {
		if (F == other.F)
			return 0;
		return F < other.F ? 1 : -1;
	}
};

class CPriorityQueue {
private:
	stPQNODE _heap[dfMAPWIDTH * dfMAPHEIGHT];
	int _lastIndex; // 데이터가 들어갈 위치 (노드 개수)
	//std::vector<stPQNODE> _heap;	// 완전이진트리 배열로 표현
	// TODO 백터 대체할껄 찾아보기
	// [i]연산을 O(1)에 수행하며 메모리를 동적으로 할당하는 것은 ??
public:
	CPriorityQueue();

	void clear();
	void Push(stPQNODE data);
	stPQNODE Pop();
	stPQNODE Peek();

	int size() { return _lastIndex; }
};
