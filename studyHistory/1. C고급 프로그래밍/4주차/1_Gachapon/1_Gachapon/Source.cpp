#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

struct ITEM {
	char name[30];
	int	 rate;
};

ITEM _itemTable[6] = {
	{"불멸의 용비늘",10},
	{"멸진룡의 견갑각",6},
	{"멸진룡의 재생각",5},
	{"멸진룡의 첨예발톱",3},
	{"멸진룡의 큰뿔",3},
	{"멸진룡의 보옥",1}
};

// 올랜덤 진짜 "확 률"
// 1. 전체 아이템들의 비율 총 합을 구함.
// 2. rand() 함수로 확률을 구함
// 여기서 확률은 1/100 이 아니며, 1/총합비율 임.
// int iRand = 확률구하기;
// 3. 전체 아이템 테이블을 돌면서
// 위에서 구한 Rand 값에 해당 지점의 아이템을 찾는다.

int cnt = 0;

void RandomItem() {
	// 1
	int allRate = 0;

	for (int i = 0; i < 6; i++) {
		allRate += _itemTable[i].rate;
	}
	// 2
	int r = rand() % allRate;
	// 3
	int rating = 0;
	for (int i = 0; i < 6; i++) {
		rating += _itemTable[i].rate;

		if (r < rating) {
			printf_s("%d : 갈무리 : %s | %d / %d\n",++cnt, _itemTable[i].name, _itemTable[i].rate, allRate);
			return;
		}
	}

}


// 테이블
// 강제 당첨
// 1. 전체 아이템들의 비율 총 합을 구함.
// 단, WinTime 이 정해진 아이템은 확률의 대상 자체가 아니기 때문에 제외.
// 2. 본 뽑기 회차에 대한 지정 아이템이 있는지 확인
// WinTime 과 iCount 가 같은 아이템을 찾는다. (WinTime 번째 뽑기에 강제당첨)
// 있다면.. 그 아이템을 뽑고 중단.
// 3. rand() 함수로 확률을 구함
// 여기서 확률은 1/100 이 아니며, 1/총합비율 임.
// int iRand = 확률구하기;
// 4. 전체 아이템 테이블을 돌면서
// 위에서 구한 Rand 값에 해당 지점의 아이템을 찾는다.
// 5. 뽑기 회차를 초기화 해야할지 판단하여 초기화.
// 테이블 미리 작성
// 1. 테이블작성
// 1-1. 전체 아이템들의 비율 총 합을 구함
// 1-2. 합만큼의 배열을 할당후 arr[i] = i
// 1-3. arr 셔플
int _randomItemTable[100000];
void ResetTable() {
	int allRate = 0;
	for (int i = 0; i < 6; i++) {
		allRate += _itemTable[i].rate;
	}
	for (int i = 0; i < allRate; i++) {
		_randomItemTable[i] = i;
	}

	// 셔플
	for (int i = 0; i < allRate; i++) {
		int r = rand() % allRate;
		int t = _randomItemTable[i];
		_randomItemTable[i] = _randomItemTable[r];
		_randomItemTable[r] = t;
	}

}
void RandomTable() {
	int allRate = 0;
	for (int i = 0; i < 6; i++) {
		allRate += _itemTable[i].rate;
	}
	if (cnt % allRate == 0) {
		ResetTable();
		cnt = 0;
	}

	int rating = 0;
	for (int i = 0; i < 6; i++) {
		rating += _itemTable[i].rate;

		if (_randomItemTable[cnt] < rating) {
			printf_s("%d : 갈무리 : %s | %d / %d\n", cnt, _itemTable[i].name, _itemTable[i].rate, allRate);
			cnt++;
			return;
		}
	}
}


// 차감
ITEM _buffer[6];

void Gashapon() {
	int bufferRate = 0;
	for (int i = 0; i < 6; i++) {
		bufferRate += _buffer[i].rate;
	}
	if ( bufferRate <= 0 || cnt == 0) {
		for (int i = 0; i < 6; i++) {
			_buffer[i] = _itemTable[i];
			bufferRate += _buffer[i].rate;
		}
	}

	// 2
	int r = rand() % bufferRate;
	// 3
	int rating = 0;
	for (int i = 0; i < 6; i++) {
		rating += _buffer[i].rate;

		if (r < rating) {
			printf_s("%d : 갈무리 : %s | %d / %d\n", cnt, _itemTable[i].name, _itemTable[i].rate, bufferRate);
			_buffer[i].rate--;
			cnt++;
			return;
		}
	}
}

int main() {
	while (true) {
		char c = _getch();
		RandomTable();
	}
	return 0;
}