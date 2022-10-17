#pragma once
#include "CPriorityQueue.h"
#include "Global.h"
#include "CList.h"
#include "CMemory.h"

class CJumpPoint {

private:
	// 우선순위 큐 (1순위 F가 작은것, 2순위 H가 작은것)
	CPriorityQueue _pq;

	// 오픈리스트겸 클로즈리스트 (해당좌표에 포인터가 있다면 한번 거쳐간것 으로 침)
	struct stPATH *_openList[dfMAPHEIGHT][dfMAPWIDTH];

	// 표시영역 색깔을 위한 카운터
	int _stepCount;

	bool _bWork; // 찾는중이냐? true = A* 실행중

	// 왼쪽위부터 시계방향
	// 0LU 1UU 2RU 3RR 4RD 5DD 6LD 7LL
	const int _dx[8] = {-1, 0,+1,+1,+1, 0,-1,-1};
	const int _dy[8] = {-1,-1,-1, 0,+1,+1,+1, 0};
	const int _cost[8] = {14,10,14,10,14,10,14,10};

	// 끝지점
	struct stPOS _endPos;

public:
	CJumpPoint();
	~CJumpPoint();

	bool PathFind(struct stPOS start, struct stPOS end, CList<struct stPOS> &path); // 한번에 찾기!

	//
	// 갈 수 있나?
	//
	inline bool CanGo(int iX, int iY);
	/// 
	/// 휴라스틱
	/// 
	inline int GetH(struct stPOS start, struct stPOS end);
	inline int GetH(int sX, int sY, int eX, int eY);

	/// 
	/// 길찾기 정보 초기화
	/// 
	void clear();

	//
	// 길찾기 시작
	//
	void Start(struct stPOS start, struct stPOS end);
	//
	// 길찾는 중
	//
	void Step();
	//
	// 구한 경로 뱉어내기
	//
	bool GetPath(CList<struct stPOS> &path);


	// pqNow.dir 방향으로 탐색
	// 직선 탐색
	bool JumpStraight(struct stPQNODE pqNow, bool bOpen = true);
	// 대각선 탐색
	bool JumpDiagonal(struct stPQNODE pqNow);
	// 큐에 넣기
	void Enqueue(struct stPQNODE pqNow, struct stPQNODE pqNext);

	// 실행중이냐?
	bool isWorking() { return _bWork; }

	// 디버그용 그 좌표값엔 뭐가 있는지?
	struct stPATH *GetPathNode(int iX, int iY);
};

