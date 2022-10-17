#pragma once
#include "Global.h"
#include "CList.h"
class CCorrection {
public:
	inline int abs(int a) { if (a > 0) return a; return -a; }

	bool CanGo(int iX, int iY);		// 갈수있나?
	bool CheckLine(int sX, int sY, int eX, int eY); // s부터 e까지 타일의 직선
	bool DrawLine(int sX, int sY, int eX, int eY); // s부터 e까지 타일의 직선
	void Correction(CList <struct stPOS> &before, CList <struct stPOS> &after);
};

