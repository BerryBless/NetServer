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
	int dir; // 탐색중인 방향
	//int iDirFlag; // 탐색할 방향
	inline int CompareTo(stPQNODE other) {
		if (F == other.F) {
			if (F - G < other.F - other.G)
				return 1;
			return -1;
		}
		if (F < other.F)
			return 1;
		return -1;
	}
};

class CPriorityQueue {
private:
	stPQNODE _heap[dfMAPWIDTH * dfMAPHEIGHT];
	int _lastIndex; // 데이터가 들어갈 위치 (노드 개수)
public:
	CPriorityQueue();

	void clear();
	void Push(stPQNODE data);
	stPQNODE Pop();
	stPQNODE Peek();

	inline int size() { return _lastIndex; }
};
