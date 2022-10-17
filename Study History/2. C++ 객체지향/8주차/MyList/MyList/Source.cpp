#include "CList.h"
#include "Profiler.h"
#include "MemProfiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <thread>
#include <time.h>

#define MAX_ARRAY 10'000'000

class CPlayer {
public:
	struct stStat {
		int _MaxHp;
		int _STR;
		int _DEF;
	};

	enum class eState {
		Idle = 0,
		Dead,
	};

public:

	eState _state;// 상태
	stStat _statistics; // 스탯
	int _hp;
	int _kill;
	char _name[16];
public:
	CPlayer(int maxHp, int str, int def, const char *playerName) {
		_statistics._MaxHp = maxHp;
		_statistics._STR = str;
		_statistics._DEF = def;
		_state = eState::Idle;
		_hp = _statistics._MaxHp;
		_kill = 0;
		strcpy_s(_name, playerName);
	}
	~CPlayer() {
		_statistics._MaxHp = 0;
		_statistics._STR = 0;
		_statistics._DEF = 0;
		_state = eState::Dead;
		_hp = 0;
	}
	virtual void Attack(CPlayer *target)=0;
	virtual void Hit(CPlayer *attacker) =0;

	bool isDead() {
		return this->_state == eState::Dead;
	}
};

class CFuse : CPlayer {

};


// rand() 확장
// 1~4Byte 랜덤
unsigned int Erand(int byteSize) {
	unsigned int r = 0;		// 0000 0000
	int mask = 0xFF;		// 1111 1111
	for (int i = 0; i < byteSize; i++) {
		int t = rand();
		t &= mask;
		r = (r << 8); // 1byte 만큼 이동
		r |= t;
		//printf_s("%02d : 0x%08x\n", i, r);
	}
	return r;
}



void SpawnPlayer(CList <CPlayer *> *playerList, int num) {
	char name[16];
	sprintf_s(name, "Player_%d", num);
	int r = rand() % 2;
	if (r) {
		//playerList->push_back(new CPlayer(Erand(2), Erand(2), Erand(2), name));
	} else {
		//playerList->push_front(new CPlayer(Erand(3), Erand(3), Erand(3), name));
	}
}

bool comp(int a, int b) {
	return a > b;
}
bool comp2(int a, int b) {
	return a< b;
}

#pragma region 정렬 검증
// 검증 : 신뢰 가능한 정렬
void SortVerify(int arr[], int size) {
	printf_s("START :: SortVerify()\n");
	time_t start = clock();
	std::sort(arr, arr + size,comp);
	printf_s("END   :: SortVerify() - %04lldms\n", clock() - start);
}
// 테스트용 배열 만들기
void Array_Make(int origin[], int size) {
	printf_s("START :: Array_Make()\n");
	for (int i = 0; i < size; i++) {
		origin[i] = (int) Erand(sizeof(int));
	}
	printf_s("END   :: Array_Make()\n");
}
// 정렬 채점
template<typename T = int>
bool Array_Veri(CList<T> *iList, int origin[], int size) {
	printf_s("START :: Array_Veri()\n");
	time_t start = clock();
	// 정답과 풀이 비교
	bool o = true;
	int i = 0;
	for (auto iter = iList->begin(); iter != iList->end(); ++iter) {
		if (origin[i] != *iter) {
			o = false;
		}
		i++;
	}
	printf_s("END   :: Array_Veri() - %04lldms\n", clock() - start);
	return o;
}
#pragma endregion


int main() {
	int *originArray = (int *) malloc(sizeof(int) * MAX_ARRAY);
	CList<int> iList;
	for (int TESTCASE =3; TESTCASE < 50; TESTCASE+=123) {
		// 재현 가능한 렌덤
		srand(TESTCASE);
		iList.clear();
		int arrSize = TESTCASE % MAX_ARRAY + 1; // 1 ~ MAX_ARRAY
		printf_s("\n==================================\n");
		printf_s("TEST %d : size(%d) start\n\n", TESTCASE, arrSize);
		// 데이터 생성
		Array_Make(originArray, arrSize);
		for (int i = 0; i < arrSize; i++) {
			int r = rand() % 2;
			if (r == 1) {
				iList.push_back(originArray[i]);
			} else {
				iList.push_front(originArray[i]);
			}
		}
		// 답안지 작성
		std::thread originSortThread(SortVerify, originArray, arrSize);
		// 문제집 풀기
		printf_s("START :: Array_Sort()\n");
		time_t start = clock();
		iList.sort(comp);
		printf_s("END   :: Array_Sort() - %04lldms\n", clock() - start);
		// 검증
		originSortThread.join(); // 답안지 작성 끝나기 기다리기
		if (Array_Veri(&iList, originArray, arrSize)) {
			printf_s("\nTEST %d : size(%d) clear\n", TESTCASE, arrSize);
			printf_s("\n==================================\n");
		} else {
			printf_s("\n==================================\n");
			printf_s("\nERROR | TESTCASE :: %d / arrSize :: %d\n", TESTCASE, arrSize);
			printf_s("\n==================================\n");
			for (auto iter = iList.begin(); iter != iList.end(); ++iter) {
				printf_s("%d ", *iter);
			}
			printf_s("\n");
			for (int i = 0; i < arrSize; i++) {
				printf_s("%d ", originArray[i]);
			}
			printf_s("\n");
			break;
		}
		PRO_PRINT("log/list.log");
		iList.clear();
		MEMPROFILER_PRINT();
	}
	free(originArray);
	return 0;
}