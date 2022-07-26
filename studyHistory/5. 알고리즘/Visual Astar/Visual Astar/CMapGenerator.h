#pragma once
#include "Global.h"

// ¸Ê
extern short g_map[dfMAPHEIGHT][dfMAPWIDTH];

class CMapGenerator {

private:
	// »ó ¿ì ÇÏ ÁÂ
	const int _dx[4] = {0,1,0,-1};
	const int _dy[4] = {-1,0,1,0};

	

public:
	CMapGenerator();
	~CMapGenerator();
	// ½ÃÀÛÁ¡, ³¡Á¡µµ ¹Ù²ñ
	void Generate();
	void RecursiveBacktracking(struct stPOS now);

	
	bool RecursiveCanGo(struct stPOS now, int dir);


};

