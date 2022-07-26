#pragma once
#include "CList.h"
#include "Global.h"
class CDrawLine {

public:
	// abs(sX - eX), abs(sY - eY) 더 긴것을 기준으로
	inline int abs(int a) { if (a > 0) return a; return -a; }
	bool GetLine(int sX, int sY, int eX, int eY, CList <struct  stPOS> &path);
};

