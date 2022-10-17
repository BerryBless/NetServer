#include "CJumpPoint.h"
#include "CFramework.h"
#include "Profiler.h"

CJumpPoint::CJumpPoint() {
	_pq.clear();
	memset(_openList, 0, sizeof(_openList));
	_endPos.X = 0;
	_endPos.Y = 0;
	_stepCount = 0;
}

CJumpPoint::~CJumpPoint() {
	clear();
}



bool CJumpPoint::PathFind(stPOS start, stPOS end, CList<stPOS> &path) {
	stPQNODE pqNow;
	stPQNODE pqNext;
	stPATH *pPathNode = (stPATH *) g_memoryPool.Alloc(sizeof(stPATH));
	PRO_BEGIN(L"JPS");
	clear(); // 오픈리스트 초기화
	_endPos = end;	// 목적지 저장

	// 시작지점 노드 생성
	pqNow.X = start.X;
	pqNow.Y = start.Y;
	pqNow.G = 0;
	pqNow.F = GetH(start, end);
	pqNow.dir = -1;

	// 오픈리스트에 넣기
	_openList[pqNow.Y][pqNow.X] = pPathNode;

	pPathNode->pParent = NULL;
	pPathNode->X = pqNow.X;
	pPathNode->Y = pqNow.Y;
	pPathNode->G = pqNow.G;
	pPathNode->F = pqNow.F;

	// 우선순위 큐에 푸쉬
	_pq.Push(pqNow);

	while (_pq.size() > 0) {
		// 하나 pop
		pqNow = _pq.Pop();
		// 뽑은게 목적지면..
		if (pqNow.X == end.X && pqNow.Y == end.Y) {
			break;
		}

		if (pqNow.dir < 0) {
			// 첫시작시 8방향 다보기
			for (int i = 0; i < 8; i++) {
				pqNow.dir = i;
				if (i % 2 == 0) {
					// 짝수면 대각선 방향 (0LU 2RU 4RD 6LD)
					JumpDiagonal(pqNow);
				} else {
					// 홀수면 직선 방향 (1UU 3RR 5DD 7LL)
					JumpStraight(pqNow);
				}
			}

		} else {

			// 큐에 들어있는 방향만 보기
			if (pqNow.dir % 2 == 0) {
				// 짝수면 대각선 방향 (LU RU RD LD)
				JumpDiagonal(pqNow);
			} else {
				// 홀수면 직선 방향 (UU RR DD LL)
				JumpStraight(pqNow);
			}
		}
	}

	// 경로 넣기
	// 목적지에 도달 못함
	if (_openList[end.Y][end.X] == NULL) {
		PRO_END(L"JPS");
		return false;
	}

	// 경로 초기화
	path.clear();

	// 부모를 넣고
	pPathNode = _openList[end.Y][end.X];
	stPOS pos;
	while (pPathNode != NULL) {
		// 내 좌표를 넣고
		pos.X = pPathNode->X;
		pos.Y = pPathNode->Y;
		path.push_front(pos);
		// 부모노드로
		pPathNode = pPathNode->pParent;
	}
	PRO_END(L"JPS");
	return true;
}

inline int CJumpPoint::GetH(stPOS start, stPOS end) {
	int x = start.X - end.X;
	int y = start.Y - end.Y;

	// 절대값
	if (x < 0) x *= -1;
	if (y < 0) y *= -1;

	return (x + y) * 10;
}

inline int CJumpPoint::GetH(int sX, int sY, int eX, int eY) {
	int x = sX - eX;
	int y = sY - eY;

	// 절대값
	if (x < 0) x *= -1;
	if (y < 0) y *= -1;

	return (x + y) * 10;
}

void CJumpPoint::clear() {
	// 우선순위큐 클리어
	_pq.clear();
	// 오픈리스트 클리어
	for (int i = 0; i < dfMAPHEIGHT; i++) {
		for (int j = 0; j < dfMAPWIDTH; j++) {
			if (_openList[i][j] != NULL) {
				g_memoryPool.Free(_openList[i][j]);
				_openList[i][j] = NULL;
			}
		}
	}
}

void CJumpPoint::Start(stPOS start, stPOS end) {
	stPQNODE pqNow;
	stPATH *pPathNode = (stPATH *) g_memoryPool.Alloc(sizeof(stPATH));
	_stepCount = 0;
	clear(); // 오픈리스트 초기화
	_bWork = true;	// 실행중입니다
	_endPos = end;	// 목적지 저장

	// 시작지점 노드 생성
	pqNow.X = start.X;
	pqNow.Y = start.Y;
	pqNow.G = 0;
	pqNow.F = GetH(start, end);
	pqNow.dir = -1;

	// 오픈리스트에 넣기
	_openList[pqNow.Y][pqNow.X] = pPathNode;

	pPathNode->pParent = NULL;
	pPathNode->X = pqNow.X;
	pPathNode->Y = pqNow.Y;
	pPathNode->G = pqNow.G;
	pPathNode->F = pqNow.F;

	// 우선순위 큐에 푸쉬
	_pq.Push(pqNow);

}

void CJumpPoint::Step() {
	// 반복문 안!
	_stepCount += 1; // 이번스탭 표시영역 색깔!
	stPQNODE pqNow;
	stPQNODE pqNext;
	stPATH *pPathNode;

	// 큐가 비어있으면
	if (_pq.size() <= 0) {
		_bWork = false;
		return;
	}

	// 하나 pop
	pqNow = _pq.Pop();
	// 뽑은게 목적지면..
	if (pqNow.X == _endPos.X && pqNow.Y == _endPos.Y) {
		g_map[_endPos.Y][_endPos.X] = (short) eMAP::End;
		_bWork = false;
		return;
	}

	if (pqNow.dir < 0) {
		// 첫시작시 8방향 다보기
		for (int i = 0; i < 8; i++) {
			pqNow.dir = i;
			if (i % 2 == 0) {
				// 짝수면 대각선 방향 (0LU 2RU 4RD 6LD)
				JumpDiagonal(pqNow);
			} else {
				// 홀수면 직선 방향 (1UU 3RR 5DD 7LL)
				JumpStraight(pqNow);
			}
		}

	} else {
		g_map[pqNow.Y][pqNow.X] = (short) eMAP::Close; // 맵에 그리기!

		// 큐에 들어있는 방향만 보기
		if (pqNow.dir % 2 == 0) {
			// 짝수면 대각선 방향 (LU RU RD LD)
			JumpDiagonal(pqNow);
		} else {
			// 홀수면 직선 방향 (UU RR DD LL)
			JumpStraight(pqNow);
		}
	}

}

bool CJumpPoint::GetPath(CList<struct stPOS> &path) {
	if (_openList[_endPos.Y][_endPos.X] == NULL) return false;
	path.clear();
	stPATH *pPathNode;

	pPathNode = _openList[_endPos.Y][_endPos.X];
	stPOS pos;
	while (pPathNode != NULL) {
		pos.X = pPathNode->X;
		pos.Y = pPathNode->Y;
		path.push_front(pos);
		pPathNode = pPathNode->pParent;
	}
	return true;
}

inline bool CJumpPoint::CanGo(int iX, int iY) {
	// 범위 벗어남
	if (0 > iX || iX >= dfMAPWIDTH)return false;
	if (0 > iY || iY >= dfMAPHEIGHT)return false;
	// 벽에 막힘
	if (g_map[iY][iX] == (short) eMAP::Wall) return false;
	// 갈 수 있음
	return true;
}

bool CJumpPoint::JumpStraight(stPQNODE pqNow, bool bOpen) {
	stPQNODE pqNext; // 다음 탐색노드

	// next
	int nx = pqNow.X;
	int ny = pqNow.Y;
	int dir = pqNow.dir;

	// 확인용
	// 벽
	int wallx;
	int wally;
	// 코너
	int	cornerx;
	int cornery;

	int calcdir;

	bool opened = false; // 노드를 생성(예약) 했냐 안했냐 true = 했다
	pqNext.G = pqNow.G;
	while (true) {
		// 다음좌표로
		nx += _dx[dir];
		ny += _dy[dir];
		// 다음좌표 무결성 검사
		if (CanGo(nx, ny) == false) return false;

		// 정보넣기
		pqNext.X = nx;
		pqNext.Y = ny;
		pqNext.G += _cost[dir];
		// 목적지 도착
		if (pqNext.X == _endPos.X && pqNext.Y == _endPos.Y) {
			opened = true;
			if (bOpen) {
				pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
				Enqueue(pqNow, pqNext);
			} else {
				return true;
			}
			break;
		}

		// 벽 체크
		// 위쪽이 막혀 있으면서 오른쪽 위는 뚫려있는 경우
		calcdir = (dir + 6) % 8;
		wallx = nx + _dx[calcdir];
		wally = ny + _dy[calcdir];
		if (CanGo(wallx, wally) == false) {
			calcdir = (dir + 7) % 8;
			cornerx = nx + _dx[calcdir];
			cornery = ny + _dy[calcdir];

			if (CanGo(cornerx, cornery) == true) {
				// 정보넣기
				pqNext.dir = calcdir;// 오른쪽 위방향 탐색
				// 찾았다
				opened = true;
				// 큐에 넣기
				if (bOpen) {
					pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
					Enqueue(pqNow, pqNext);
				} else {
					return true;
				}
			}
		}


		// 아래쪽이 막혀 있으면서 오른쪽 아래는 뚫려있는 경우
		calcdir = (dir + 2) % 8;
		wallx = nx + _dx[calcdir];
		wally = ny + _dy[calcdir];
		if (CanGo(wallx, wally) == false) {
			calcdir = (dir + 1) % 8;
			cornerx = nx + _dx[calcdir];
			cornery = ny + _dy[calcdir];

			if (CanGo(cornerx, cornery) == true) {
				pqNext.dir = calcdir; // 오른쪽 아래방향 탐색
				// 찾았다!
				opened = true;
				// 큐에 넣기
				if (bOpen) {
					pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
					Enqueue(pqNow, pqNext);
				} else {
					return true;
				}
			}
		}

		if (opened == true) {
			// 다음좌표로
			nx += _dx[dir];
			ny += _dy[dir];
			// 다음좌표 무결성 검사
			if (CanGo(nx, ny) == false) return true;
			// 현재 방향도 넣어두기?
			pqNext.dir = dir;
			if (bOpen) {
				pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
				Enqueue(pqNow, pqNext);
			}
			return true;
		}

		//*
		// 출력
		if (g_map[ny][nx] == (short) eMAP::Empty ||
			g_map[ny][nx] > (short) eMAP::MAX)
			g_map[ny][nx] = (short) eMAP::MAX + _stepCount;
			//*/
	}
	return true;
}

bool CJumpPoint::JumpDiagonal(stPQNODE pqNow) {
	stPQNODE pqNext; // 다음 탐색노드

	// next
	int nx = pqNow.X;
	int ny = pqNow.Y;
	int dir = pqNow.dir;

	// 벽
	int wallx;
	int wally;
	// 코너
	int	cornerx;
	int cornery;

	int calcdir;

	bool opened = false;

	// G값
	pqNext.G = pqNow.G;
	while (true) {
		// 다음좌표로
		nx += _dx[dir];
		ny += _dy[dir];
		// 다음좌표 무결성 검사
		if (CanGo(nx, ny) == false) return false;

		// 정보넣기
		pqNext.X = nx;
		pqNext.Y = ny;
		pqNext.G += _cost[dir];

		// 목적지 도착
		if (pqNext.X == _endPos.X && pqNext.Y == _endPos.Y) {
			opened = true;
			pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
			Enqueue(pqNow, pqNext);
			break;
		}



		// 벽 체크
		// 왼쪽이 막혀 있으면서 왼쪽 위가 막혀있지 않은 경우
		calcdir = (dir + 5) % 8;
		wallx = nx + _dx[calcdir];
		wally = ny + _dy[calcdir];
		if (CanGo(wallx, wally) == false) {
			calcdir = (dir + 6) % 8;
			cornerx = nx + _dx[calcdir];
			cornery = ny + _dy[calcdir];
			if (CanGo(cornerx, cornery) == true) {
				// 찾았다
				opened = true;
				pqNext.dir = calcdir;// 왼쪽 위방향으로 탐색
				pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
				Enqueue(pqNow, pqNext);
			}
		}

		// 아래가 막혀 있으면서 오른쪽 아래가 안막혔으면
		calcdir = (dir + 3) % 8;
		wallx = nx + _dx[calcdir];
		wally = ny + _dy[calcdir];
		if (CanGo(wallx, wally) == false) {
			calcdir = (dir + 2) % 8;
			cornerx = nx + _dx[calcdir];
			cornery = ny + _dy[calcdir];
			if (CanGo(cornerx, cornery) == true) {
				// 찾았다!
				opened = true;
				pqNext.dir = calcdir;// 왼쪽 아래방향으로 탐색
				pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
				Enqueue(pqNow, pqNext);
			}
		}

		// 직선방향 탐색
		pqNext.dir = (dir + 7) % 8;// 왼 방향으로 탐색
		if (JumpStraight(pqNext, false)) {
			// 왼 직선방향에 무언가 있음
			// 찾았다!
			opened = true;
			pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
			Enqueue(pqNow, pqNext);
		}

		pqNext.dir = (dir + 1) % 8;// 오른쪽 방향으로 탐색
		if (JumpStraight(pqNext, false)) {
			// 오른쪽 직선방향에 무언가 있음
			// 찾았다!
			opened = true;
			pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
			Enqueue(pqNow, pqNext);
		}
		// 노드가 생성었나?
		if (opened == true) {
			// 다음좌표로
			nx += _dx[dir];
			ny += _dy[dir];
			// 다음좌표 무결성 검사
			if (CanGo(nx, ny) == false) return true;
			// 현재 방향도 넣어두기
			pqNext.dir = dir;
			pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);
			Enqueue(pqNow, pqNext);
			return true;
		}
		//*
		// 탐색했다고 표시 색깔내기
		if (g_map[ny][nx] == (short) eMAP::Empty ||
			g_map[ny][nx] > (short) eMAP::MAX)
			g_map[ny][nx] = (short) eMAP::MAX + _stepCount;
			//*/
	}
	return opened;
}
void CJumpPoint::Enqueue(stPQNODE pqNow, stPQNODE pqNext) {
	stPATH *pPathNode;
	int checksum = (1 << pqNext.dir);
	// 예약되었는지 확인
	pPathNode = _openList[pqNext.Y][pqNext.X];

	// 이전에 예약이 됨
	if (pPathNode != NULL) {
		// 이전에 방문한것보다 안좋은 경로
		if (pPathNode->F < pqNext.F)
			return;
		// 이미 확인한 방향
		if (pPathNode->Checksum & checksum)
			return;
		// 두가지 경우가 아니라면 갱신할 가치가 있음
	} else {
		// 방문한적 없음
		pPathNode = (stPATH *) g_memoryPool.Alloc(sizeof(stPATH));
		_openList[pqNext.Y][pqNext.X] = pPathNode;
		pPathNode->Checksum = 0;
	}
	// 그 좌표를 예약하기
	pPathNode->pParent = _openList[pqNow.Y][pqNow.X];
	pPathNode->X = pqNext.X;
	pPathNode->Y = pqNext.Y;
	pPathNode->G = pqNext.G;
	pPathNode->F = pqNext.F;
	pPathNode->Checksum |= checksum; // 큐에 넣을 방향 체크


	// 예약됐다고 그리기
	g_map[pqNext.Y][pqNext.X] = (short) eMAP::Open;

	_pq.Push(pqNext);
}


stPATH *CJumpPoint::GetPathNode(int iX, int iY) {
	if (CanGo(iX, iY) == false)
		// 갈수없는곳이면 널포인터
		return NULL;
	return _openList[iY][iX];
}
