#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "CRedBlackTree.h"
#include "CMemory.h"
#include "CList.h"
#include "CPriorityQueue.h"

#define dfMAPSIZE 1000
#define dfWAY 8

struct stPQNode {
	int F;
	// H = F - G
	int G;

	int X;
	int Y;

	int CompareTo(stPQNode other) {
		if (F == other.F)
			return 0;
		return F < other.F ? 1 : -1;
	}

	long long GetHash() {
		return (long long) Y << 32 | X;
	}
};

struct  stPath {
	stPath *pParent;

	int X;
	int Y;
};

int GetH(stPQNode s, stPQNode e) {
	int dX = (e.X - s.X);
	int dY = (e.Y - s.Y);

	if (dX < 0) dX *= -1;
	if (dY < 0) dY *= -1;

	return dX + dY;
}


int _objectTile[dfMAPSIZE][dfMAPSIZE];
// LL부터 시계방향 8방향
int _dX[dfWAY] = {-1, 0,+1,+1,+1, 0,-1,-1};
int _dY[dfWAY] = {-1,-1,-1, 0,+1,+1,+1, 0};
stPath *_pPathParent[dfMAPSIZE][dfMAPSIZE];