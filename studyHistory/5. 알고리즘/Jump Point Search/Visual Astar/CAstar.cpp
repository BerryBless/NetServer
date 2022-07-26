#include "CAstar.h"
#include "CFramework.h"
#include "Profiler.h"
CAstar::CAstar() {
	_pq.clear();
	memset(_openList, 0, sizeof(_openList));
	_endPos.X = 0;
	_endPos.Y = 0;
}

CAstar::~CAstar() {
	clear();
}

bool CAstar::CanGo(int iX, int iY) {
	// 범위 벗어남
	if (0 > iX || iX >= dfMAPWIDTH)return false;
	if (0 > iY || iY >= dfMAPHEIGHT)return false;
	// 벽에 막힘
	if (g_map[iY][iX] == (short) eMAP::Wall) return false;
	// 갈 수 있음
	return true;
}

bool CAstar::PathFind(stPOS start, stPOS end, CList<stPOS> &path) {
	stPQNODE pqNow;
	stPQNODE pqNext;
	stPATH *pPathNode = (stPATH *) g_memoryPool.Alloc(sizeof(stPATH));

	PRO_BEGIN(L"ASTAR");

	clear(); // 오픈리스트 초기화

	// 시작지점 노드 생성
	pqNow.X = start.X;
	pqNow.Y = start.Y;
	pqNow.G = 0;
	pqNow.F = GetH(start, end);


	// 오픈리스트에 넣기
	_openList[pqNow.Y][pqNow.X] = pPathNode;

	pPathNode->pParent = NULL;
	pPathNode->X = pqNow.X;
	pPathNode->Y = pqNow.Y;
	pPathNode->G = pqNow.G;
	pPathNode->F = pqNow.F;


	// 우선순위 큐에 푸쉬
	_pq.Push(pqNow);


	// 탐색 시작!!
	while (_pq.size() >0){
		// 하나 pop
		pqNow = _pq.Pop();

		// 뽑은게 목적지면..
		if (pqNow.X == end.X && pqNow.Y == end.Y) {
			break;
		}

		// 왼쪽위부터 시계방향으로 8방향 검사
		for (int i = 0; i < 8; i++) {
			// 다음위치 정보
			pqNext.X = pqNow.X + _dx[i];
			pqNext.Y = pqNow.Y + _dy[i];
			pqNext.G = pqNow.G + _cost[i];
			pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, end.X, end.Y);

			
			// 갈수있나?
			if (CanGo(pqNext.X, pqNext.Y) == false) continue;

			// 예약되었는지 확인
			pPathNode = _openList[pqNext.Y][pqNext.X];

			// 이전에 예약이 됨
			if (pPathNode != NULL) {
				// 이전에 방문한것보다 안좋은 경로
				if (pPathNode->F <= pqNext.F)
					continue;
			} else {
				// 방문한적 없음
				pPathNode = (stPATH *) g_memoryPool.Alloc(sizeof(stPATH));
				_openList[pqNext.Y][pqNext.X] = pPathNode;
			}

			// 그 좌표를 예약하기
			pPathNode->pParent = _openList[pqNow.Y][pqNow.X];
			pPathNode->X = pqNext.X;
			pPathNode->Y = pqNext.Y;
			pPathNode->G = pqNext.G;
			pPathNode->F = pqNext.F;

			// 우선순위 큐에 넣기
			_pq.Push(pqNext);
		}
	}
	// 경로 넣기
	// 목적지에 도달 못함
	if (_openList[end.Y][end.X] == NULL) {
		PRO_END(L"ASTAR");
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
	PRO_END(L"ASTAR");
	return true;
}

int CAstar::GetH(stPOS start, stPOS end) {
	int x = start.X - end.X;
	int y = start.Y - end.Y;

	// 절대값
	if (x < 0) x *= -1;
	if (y < 0) y *= -1;

	return (x + y) * 10;
}

int CAstar::GetH(int sX, int sY, int eX, int eY) {
	int x = sX - eX;
	int y = sY - eY;

	// 절대값
	if (x < 0) x *= -1;
	if (y < 0) y *= -1;

	return (x + y) * 10;
}

void CAstar::clear() {
	_pq.clear();
	for (int i = 0; i < dfMAPHEIGHT; i++) {
		for (int j = 0; j < dfMAPWIDTH; j++) {
			if (_openList[i][j] != NULL) {
				g_memoryPool.Free(_openList[i][j]);
				_openList[i][j] = NULL;
			}
		}
	}
}

void CAstar::Start(stPOS start, stPOS end) {
	stPQNODE pqNow;
	stPATH *pPathNode = (stPATH *) g_memoryPool.Alloc(sizeof(stPATH));

	clear(); // 오픈리스트 초기화
	_bWork = true;	// 실행중입니다
	_endPos = end;	// 목적지 저장

	// 시작지점 노드 생성
	pqNow.X = start.X;
	pqNow.Y = start.Y;
	pqNow.G = 0;
	pqNow.F = GetH(start, end);


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

void CAstar::Step() {
	// 반복문 안!
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
		_bWork = false;
		return;
	}

	// close 그래픽에 표시
	if (g_map[pqNow.Y][pqNow.X] == (short)eMAP::Open)
		g_map[pqNow.Y][pqNow.X] = (short)eMAP::Close;

	// 왼쪽위부터 시계방향으로 8방향 검사
	for (int i = 0; i < 8; i++) {
		// 다음위치 정보
		pqNext.X = pqNow.X + _dx[i];
		pqNext.Y = pqNow.Y + _dy[i];
		pqNext.G = pqNow.G + _cost[i];
		pqNext.F = pqNext.G + GetH(pqNext.X, pqNext.Y, _endPos.X, _endPos.Y);

		// 갈수있나?
		if (CanGo(pqNext.X, pqNext.Y) == false) continue;

		// 예약되었는지 확인
		pPathNode = _openList[pqNext.Y][pqNext.X];

		// 이전에 예약이 됨
		if (pPathNode != NULL) {
			// 이전에 방문한것보다 안좋은 경로
			if (pPathNode->F <= pqNext.F)
				continue;
		} else {
			// 방문한적 없음
			pPathNode = (stPATH *) g_memoryPool.Alloc(sizeof(stPATH));
			_openList[pqNext.Y][pqNext.X] = pPathNode;
		}
		// 그 좌표를 예약하기
		pPathNode->pParent = _openList[pqNow.Y][pqNow.X];
		pPathNode->X = pqNext.X;
		pPathNode->Y = pqNext.Y;
		pPathNode->G = pqNext.G;
		pPathNode->F = pqNext.F;

		// 예약됐다고 그리기
		if (g_map[pqNext.Y][pqNext.X] == (short)eMAP::Empty)
			g_map[pqNext.Y][pqNext.X] = (short)eMAP::Open;
		
		// 우선순위 큐에 넣기
		_pq.Push(pqNext);
	}
}

bool CAstar::GetPath(CList<struct stPOS> &path) {
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

stPATH *CAstar::GetPathNode(int iX, int iY) {
	return _openList[iY][iX];
}
