#include <stdlib.h>
#include <algorithm>
#include <thread>
#include <time.h>
#include <stdio.h>
#include "Global.h"
#define dfMAXARRAY 1000

unsigned int Erand(int byteSize, bool bPositive = false) {
	unsigned int r = 0;		// 0000 0000
	int mask = 0xFF;		// 1111 1111
	for (int i = 0; i < byteSize; i++) {
		int t = rand();
		t &= mask;
		r = (r << 8); // 1byte 만큼 이동
		r |= t;
		//printf_s("%02d : 0x%08x\n", i, r);
	}
	if (bPositive) r &= ~(1 << byteSize);
	return r;
}


int main() {
	srand(30);
	// 할당

	// 변수
	int randsize; // 랜덤 배열 개수
	CPriorityQueue<stPQNode> pq;

	stPQNode start;
	stPQNode end;
	stPQNode temp;
	stPQNode next;

	CRedBlackTree openlist;
	CRedBlackTree closelist;

	CRedBlackTree::stNODE *opened;	// 예약된 노드 받아오기
	const CRedBlackTree::stNODE *closed;	// 이미 방문한 노드체크
	long long hash;					// 그 좌표의 해쉬

	CList <stPath*> path;

	// 시작지점
	start.X = 0;
	start.Y = 0;
	start.G = 0;
	stPath *pPathNode = (stPath *) g_memoryPool.Alloc(sizeof(stPath)); // 부모등록
	pPathNode->pParent = NULL;
	pPathNode->X = start.X;
	pPathNode->Y = start.Y;

	_pPathParent[start.X][start.Y] = pPathNode;
	// 끝점
	end.X = 20;
	end.Y = 10;

	// 시작점 F 구하기
	start.F = start.G + GetH(start, end);

	int cnt = 1; // 임시로 표시할거

	// 시작
	pq.Push(start);

	// 맵초기화
	memset(_objectTile, -1, sizeof(_objectTile));
	for (int i = 0; i <= 15; i++) {
		_objectTile[10][i] = 0;
	}


	while (pq.size() > 0) {
		temp = pq.Pop();

		// close
		hash = temp.GetHash();
		closed = closelist.Find(hash);
		if (closed != NULL) continue; // 이미 방문한곳
		closelist.InsertNode(hash);

		_objectTile[temp.X][temp.Y] = cnt++;

		


		// 다음방향으로
		for (int i = 0; i < dfWAY; i++) {
			// 다음갈곳 구하기
			next.X = temp.X + _dX[i];
			next.Y = temp.Y + _dY[i];
			next.G = temp.G + 1;
			next.F = next.G + GetH(next, end);

			// 다음좌표 무결성 검사
			if (0 > next.X || next.X >= dfMAPSIZE)continue;
			if (0 > next.Y || next.Y >= dfMAPSIZE)continue;

			// 장애물에 막힘
			if (_objectTile[next.X][next.Y] == 0) continue;

			// 다음 갈곳 확인
			hash = next.GetHash();
			// 이미 방문했는지?
			closed = closelist.Find(hash);
			if (closed != NULL) continue; // 이미 확인을 완료한 곳

			// 예약
			opened = openlist.Find(hash);
			if (opened == NULL) {
				// 한번도 방문하지 않은 곳
				openlist.InsertNode(hash, next.F);
			} else if (opened->Data <= next.F) continue; // 이미 더좋은 경로를 찾아 예약한곳
			else {
				// 방문을 했었지만, 더좋은 경로를 발견
				opened->Data = next.F;
			}

			if (_pPathParent[next.X][next.Y] == NULL) {
				pPathNode = (stPath *) g_memoryPool.Alloc(sizeof(stPath)); // 부모등록
				pPathNode->pParent = _pPathParent[temp.X][temp.Y];
				pPathNode->X = next.X;
				pPathNode->Y = next.Y;

				_pPathParent[next.X][next.Y] = pPathNode;
			}
			pq.Push(next);



			// 목적지 도착
			if (next.X == end.X && next.Y == end.Y) {
				for (int i = 0; i < 30; i++) {
					for (int j = 0; j < 30; j++) {
						printf_s("%4d", _objectTile[j][i]);
					}
					printf_s("\n");
				}
				pPathNode = _pPathParent[end.X][end.Y];
				while (pPathNode != NULL) {
					path.push_front(pPathNode);
					pPathNode = _pPathParent[pPathNode->X][pPathNode->Y]->pParent;
				}

				for (auto iter = path.begin(); iter != path.end(); ++iter) 					{
					printf_s("(%d %d)\n", (*iter)->X, (*iter)->Y);
				}
				return 0;
			}
		}
	}


	/*
	for (int TESTCASE = 0;; TESTCASE++) {
		printf_s("\n ============================================= \n");
		printf_s("TEST[%d]\n", TESTCASE);


		printf_s("Complate!!\n");
		printf_s("\n ============================================= \n");
	}*/

	return 0;
}