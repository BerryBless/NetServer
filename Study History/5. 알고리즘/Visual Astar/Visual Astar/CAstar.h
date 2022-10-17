#pragma once
#include "CPriorityQueue.h"
#include "Global.h"
#include "CList.h"
#include "CMemory.h"
class CAstar {
private:
	CPriorityQueue _pq;

	struct stPATH *_openList[dfMAPHEIGHT][dfMAPWIDTH];


	bool _bWork; // 찾는중이냐? true = A* 실행중

	// 왼쪽위부터 시계방향
	// LU UU RU RR RD DD LD LL
	const int _dx[8] =	{-1, 0,+1,+1,+1, 0,-1,-1};
	const int _dy[8] =	{-1,-1,-1, 0,+1,+1,+1, 0};
	const int _cost[8] ={15,10,15,10,15,10,15,10};

	struct stPOS _endPos;

public:
	CAstar();
	~CAstar();

	bool Astar(struct stPOS start, struct stPOS end, CList<struct stPOS> &path);
	int GetH(struct stPOS start, struct stPOS end);
	int GetH(int sX, int sY, int eX, int eY);

	void clear();

	void Start(struct stPOS start, struct stPOS end);
	void Step();
	bool GetPath(CList<struct stPOS> &path);

	bool isWorking() { return _bWork; }

	struct stPATH *GetPathNode(int iX, int iY);
};

