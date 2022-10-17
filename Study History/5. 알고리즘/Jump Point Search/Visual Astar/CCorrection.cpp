#include "CCorrection.h"

bool CCorrection::CanGo(int iX, int iY) {
	// 범위 벗어남
	if (0 > iX || iX >= dfMAPWIDTH)return false;
	if (0 > iY || iY >= dfMAPHEIGHT)return false;
	// 벽에 막힘
	if (g_map[iY][iX] == (short) eMAP::Wall) return false;
	// 갈 수 있음
	return true;
}

bool CCorrection::CheckLine(int sX, int sY, int eX, int eY) {
#ifdef dfIImplemented
	int addX, addY;
	int counter = 0;

	int dx = eX - sX;
	int dy = eY - sY;


	//음수 방향으로 진행될 경우
	if (dx < 0) {
		addX = -1;
		dx = -dx;
	} else {
		addX = 1;
	}

	//음수 방향으로 진행될 경우
	if (dy < 0) {
		addY = -1;
		dy = -dy;
	} else {
		addY = 1;
	}

	int x = sX;
	int y = sY;

	//g_map[y][x] = (short) eMAP::Wall;
	if (CanGo(x, y) == false) return false;

	//dx >= dy 일 경우( 기울기 < 1) y의 조건이 만족하면 y를 1증가시킨다
	if (dx >= dy) {
		for (int i = 0; i < dx; i++) {
			//x를  1증가 시키고 dy만큼 Count 증가
			x += addX;
			//x의 증가값만큼 Count 증가
			counter += dy;

			//증가하는 y값 보다 크다면
			if (counter >= dx) {
				// y를 1증가 시키고 Count를 초기화한다
				y += addY;
				counter -= dx;
			}
			if (CanGo(x, y) == false) return false;
		}
	}
	//dx < dy 일 경우( 기울기 > 1) x의 조건이 만족하면 x를 1증가시킨다
	else {
		for (int i = 0; i < dy; i++) {
			//y를  1증가 시키고 dx만큼 Count 증가
			y += addY;
			counter += dx;

			//증가하는 y값 보다 크다면
			if (counter >= dy) {
				// x를 1증가 시키고 Count를 초기화한다
				x += addX;
				counter -= dy;
			}
			//g_map[y][x] = (short) eMAP::Wall;
			if (CanGo(x, y) == false) return false;
		}
	}
#else
	// 위키 방식
	int dx = abs(sX - eX);
	int dy = -abs(sY - eY);

	int addX;
	int addY;

	addX = 1;
	if (sX > eX)
		addX = -1;

	addY = 1;
	if (sY > eY)
		addY = -1;

	int err = dx + dy; // 오차
	int p; // 2배의 오차???

	int x = sX;
	int y = sY;
	while (true) {
		if (CanGo(x, y) == false) return false;
		if (x == eX && y == eY) {
			break;
		}

		p = 2 * err;
		if (p >= dy) {
			// e_xy + e_y > 0
			err += dy;
			x += addX;
		}
		if (p <= dx) {
			// e_xy+e_y < 0 
			err += dx;
			y += addY;
		}

	}
#endif // dfIImplemented

	return true;
}

bool CCorrection::DrawLine(int sX, int sY, int eX, int eY) {
#ifdef dfIImplemented


	int addX, addY;
	int counter = 0;

	int dx = eX - sX;
	int dy = eY - sY;


	//음수 방향으로 진행될 경우
	if (dx < 0) {
		addX = -1;
		dx = -dx;
	} else {
		addX = 1;
	}

	//음수 방향으로 진행될 경우
	if (dy < 0) {
		addY = -1;
		dy = -dy;
	} else {
		addY = 1;
	}

	int x = sX;
	int y = sY;

	// 출력
	g_map[y][x] = (short) eMAP::Wall;

	//dx >= dy 일 경우( 기울기 < 1) y의 조건이 만족하면 y를 1증가시킨다
	if (dx >= dy) {
		for (int i = 0; i < dx; i++) {
			//x를  1증가 시키고 dy만큼 Count 증가
			x += addX;
			//x의 증가값만큼 Count 증가
			counter += dy;

			//증가하는 y값 보다 크다면
			if (counter >= dx) {
				// y를 1증가 시키고 Count를 초기화한다
				y += addY;
				counter -= dx;
			}
			// 출력
			g_map[y][x] = (short) eMAP::Wall;
		}
	}
	//dx < dy 일 경우( 기울기 > 1) x의 조건이 만족하면 x를 1증가시킨다
	else {
		for (int i = 0; i < dy; i++) {
			//y를  1증가 시키고 dx만큼 Count 증가
			y += addY;
			counter += dx;

			//증가하는 y값 보다 크다면
			if (counter >= dy) {
				// x를 1증가 시키고 Count를 초기화한다
				x += addX;
				counter -= dy;
			}
			// 출력
			g_map[y][x] = (short) eMAP::Wall;
		}
	}
#else // dfIImplemented

	// 위키 방식
	int dx = abs(sX - eX);
	int dy = -abs(sY - eY);

	int addX;
	int addY;

	addX = 1;
	if (sX > eX)
		addX = -1;

	addY = 1;
	if (sY > eY)
		addY = -1;

	int err = dx + dy; // 오차
	int p; // 2배의 오차???

	int x = sX;
	int y = sY;
	while (true) {
		// 출력
		g_map[y][x] = (short) eMAP::Wall;
		if (x == eX && y == eY) {
			break;
		}

		p = 2 * err;
		if (p >= dy) {
			// e_xy + e_y > 0
			err += dy;
			x += addX;
		} 
		if(p<=dx) {
			// e_xy+e_y < 0 
			err += dx;
			y += addY;
		}

	}
#endif // dfIImplemented

	return true;
}

void CCorrection::Correction(CList<struct stPOS> &before, CList<struct stPOS> &after) {
	// 비었음
	if (before.empty()) return;
	after.clear();
	CList<stPOS> ::iterator nowIter = before.begin();	// 시작점
	CList<stPOS> ::iterator nextIter;	// 한번 선 그어보는 지점
	// 시작점
	int sx;
	int sy;
	// 비교점
	int ex;
	int ey;

	//보정 시작
	after.push_back(*nowIter);

	int cnt;
	// 엔딩까지
	for (; nowIter != before.end(); ) {
		// 다음 볼곳
		nextIter = nowIter;
		for (cnt = 0; ; cnt ++ ) {
			// 다음에 볼곳
			++nextIter;
			// 다음이 끝이면
			if (nextIter == before.end()) {
				// 경로에 끝 넣고 리턴
				--nextIter;
				after.push_back(*nextIter);
				return;
			}
			// 브레센헴
			sx = (*nowIter).X;
			sy = (*nowIter).Y;
			ex = (*nextIter).X;
			ey = (*nextIter).Y;
			if (CheckLine(sx, sy, ex, ey) == false) {
				// 직선으로 못가는곳이면 경로에 넣기
				nowIter = nextIter;
				--nowIter;
				after.push_back(*nowIter);
				if (cnt == 0) {
					// 바로 다음이 코너면 그냥 다음부터
					++nowIter;
				}
				// 2중 반복문 처음부터
				break;
			}
		}
	}//*/
}

